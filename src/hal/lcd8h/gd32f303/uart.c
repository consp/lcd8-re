#include "uart.h"
#include "lvgl.h"
#include "lv_conf.h"
#include "crc.h"
#include "gd32f30x.h"
#include <machine/endian.h>

#define USART0_DATA_ADDRESS      ((uint32_t)0x40013804)
uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE] = {0};
uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE] = {0};
volatile int uart_rx_ready = 0;
volatile int uart_tx_ready = 0;
uint32_t read_buffer_length = 0;

#if LV_USE_LOG
void lv_log_callback(const char *c);
#endif
static void uart_init_dma(void);

void uart_init(uint32_t baud)
{

    rcu_periph_clock_enable(RCU_DMA0);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);

    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    usart_deinit(USART0);
    usart_baudrate_set(USART0, baud);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_dma_receive_config(USART0, USART_RECEIVE_DMA_ENABLE);
    usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE);
    usart_enable(USART0);

    nvic_irq_enable(USART0_IRQn, 0, 0); // recv irq
    usart_interrupt_enable(USART0, USART_INT_IDLE);

#if DEBUG && LV_USE_LOG
#pragma message("Using uart to log LV messages")
    lv_log_register_print_cb(lv_log_callback);
#endif
}

static void uart_init_dma(void) {

    dma_parameter_struct dma_init_struct;

    rcu_periph_clock_enable(RCU_DMA0);

    /* deinitialize DMA channel4 (USART0 rx) */
    dma_deinit(DMA0, DMA_CH4);
    dma_init_struct.direction       = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr     = (uint32_t) uart_rx_buffer;
    dma_init_struct.memory_inc      = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width    = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number          = UART_RX_BUFFER_SIZE;
    dma_init_struct.periph_addr     = USART0_DATA_ADDRESS;
    dma_init_struct.periph_inc      = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width    = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority        = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH4, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH4);
    /* enable DMA channel4 */
    dma_channel_enable(DMA0, DMA_CH4);

    dma_deinit(DMA0, DMA_CH3);
    dma_init_struct.direction       = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr     = (uint32_t) uart_tx_buffer;
    dma_init_struct.memory_inc      = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width    = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number          = 0;
    dma_init_struct.periph_addr     = USART0_DATA_ADDRESS;
    dma_init_struct.periph_inc      = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width    = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority        = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH3, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH3);
    /* enable DMA channel4 */
    dma_channel_disable(DMA0, DMA_CH3);

    usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE);
    usart_flag_clear(USART0, USART_FLAG_RBNE);
    usart_dma_receive_config(USART0, USART_RECEIVE_DMA_ENABLE);
}

void USART0_IRQHandler(void)
{
    if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE)){
        /* clear IDLE flag */
        usart_data_receive(USART0);
        /* toggle the LED2 */
        
        dma_channel_disable(DMA0, DMA_CH4);
        /* number of data received */
        //rx_count = 256 - (dma_transfer_number_get(DMA0, DMA_CH4));
        
        dma_transfer_number_config(DMA0, DMA_CH4, UART_RX_BUFFER_SIZE);
        dma_channel_enable(DMA0, DMA_CH4);
    }
}
/*
void USART0_IRQHandler(void)
{
    if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE)){
    {
        usart_flag_clear(USART1, USART_IDLEF_FLAG);
        read_buffer_length = DMA1_CHANNEL5->dtcnt;

        dma_channel_disable(DMA0, DMA_CH4);
        dma_transfer_number_config(DMA0, DMA_CH4, 256);
        dma_channel_enable(DMA0, DMA_CH4);
        
        uart_rx_ready = 1; 

        dma_channel_enable(DMA1_CHANNEL5, FALSE);
        DMA1_CHANNEL5->maddr = (uint32_t) uart_rx_buffer;
        DMA1_CHANNEL5->dtcnt = UART_RX_BUFFER_SIZE;
    } else if (usart_interrupt_flag_get(USART1, USART_TDC_FLAG) != RESET) {
        USART1->sts_bit.tdc = 0;
        uart_tx_ready = 0;
    }

        dma_channel_disable(DMA0, DMA_CH4);
        dma_transfer_number_config(DMA0, DMA_CH4, 256);
        dma_channel_enable(DMA0, DMA_CH4);
}

void DMA1_Channel4_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA1_FDT4_FLAG))
    {
        dma_flag_clear(DMA1_FDT4_FLAG);
        dma_channel_enable(DMA1_CHANNEL4, FALSE);
    }
}

void DMA1_Channel5_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA1_FDT5_FLAG))
    {
        // data full
        uart_rx_ready = 1;
        dma_flag_clear(DMA1_FDT5_FLAG);
        dma_channel_disable(DMA0, DMA_CH5);
    }
}*/
#include "delay.h"
void uart_send(const uint8_t *buffer, ssize_t length, int async) {
    /*
    if (!async) while(uart_tx_ready);
    uart_tx_ready = 1;
    memcpy(uart_tx_buffer, buffer, length);
    DMA1_CHANNEL4->maddr = (uint32_t) uart_tx_buffer;
    DMA1_CHANNEL4->dtcnt = length; 
    dma_channel_enable(DMA1_CHANNEL4, TRUE);*/
}
int32_t tval = 0;

int uart_get_data(uint8_t *data, uint32_t *length) {
    /*
    if (uart_rx_ready) {
        uart_rx_ready = 0;
        *length = read_buffer_length;
        if (*length > UART_RX_BUFFER_SIZE) *length = UART_RX_BUFFER_SIZE;
        memcpy(data, uart_rx_buffer, *length);

        read_buffer_length = 0;
    
        dma_channel_enable(DMA1_CHANNEL5, TRUE);
        return 1;
    } else {
        // make sure
        dma_channel_enable(DMA1_CHANNEL5, TRUE);
    }*/
    return 0;
}


#if LV_USE_LOG
void lv_log_callback(const char *c) {
    uart_send(c, strlen(c), 0);
}
#endif
