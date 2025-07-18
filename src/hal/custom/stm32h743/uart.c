#include <string.h>
#include <machine/endian.h>
#include "uart.h"
#include "lvgl.h"
#include "lv_conf.h"
#include "crc.h"
#include "delay.h"


uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];
uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE];
volatile int uart_rx_ready = 0;
volatile int uart_tx_ready = 0;
uint32_t read_buffer_length = 0;
uint8_t *uart_rx_start = NULL;
uint8_t *uart_rx_end = NULL;

#if LV_USE_LOG
#pragma message("Providing log callback, do not attach to a motor")
#if LVGL_VERSION_MAJOR == 9
void lv_log_callback(lv_log_level_t lvl, const char *c);
#else
void lv_log_callback(const char *c);
#endif
#endif
static void uart_init_dma(void);

void uart_init(uint32_t baud)
{
    LV_LOG_INFO("UART Initialization");
    uart_init_dma();
    uart_rx_ready = 0;

#if (DEBUG && LV_USE_LOG && !defined(SEMIHOSTING) && !defined(SEGGER_RTT))
#pragma message("Using uart to log LV messages")
    lv_log_register_print_cb(lv_log_callback);
#endif
}

static void uart_init_dma(void) {
}


CRITICAL void USART1_IRQHandler(void)
{
    /* if(usart_interrupt_flag_get(USART1, USART_RDBF_FLAG) != RESET) */
    /* { */
    /*     uart_rx_ready = 1;  */
    /* } */
    /* if(usart_interrupt_flag_get(USART1, USART_IDLEF_FLAG) != RESET) */
    /* { */
    /*     usart_flag_clear(USART1, USART_IDLEF_FLAG); */
    /*     #<{(| read_buffer_length += (UART_RX_BUFFER_SIZE - DMA1_CHANNEL5->dtcnt); |)}># */
    /*     #<{(| if (read_buffer_length >= UART_RX_BUFFER_SIZE) read_buffer_length = UART_RX_BUFFER_SIZE - 1; |)}># */
    /*     uart_rx_end = uart_rx_buffer + (UART_RX_BUFFER_SIZE - DMA1_CHANNEL5->dtcnt); */
    /*     #<{(| uart_rx_end = uart_rx_start + read_buffer_length; |)}># */
    /*     #<{(| if (uart_rx_end > uart_rx_buffer + UART_RX_BUFFER_SIZE) uart_rx_end = uart_rx_buffer + (UART_RX_BUFFER_SIZE - (uart_rx_start - uart_rx_buffer)); |)}># */
    /*     read_buffer_length = (uart_rx_end - uart_rx_start) % UART_RX_BUFFER_SIZE; */
    /*     #<{(| printf("%08X, %08X, %08X, %ld\n", (int) uart_rx_buffer, (int) uart_rx_start, (int) uart_rx_end, read_buffer_length); |)}># */
    /*     if (read_buffer_length > 0) uart_rx_ready = 1;  */
    /*     if (read_buffer_length > UART_RX_BUFFER_SIZE - 16) { */
    /*         // assume no message will fit, disable dma */
    /*         dma_channel_enable(DMA1_CHANNEL5, FALSE); */
    /*         DMA1_CHANNEL5->maddr = (uint32_t) uart_rx_buffer; */
    /*         DMA1_CHANNEL5->dtcnt = UART_RX_BUFFER_SIZE; */
    /*     } */
    /*  */
    /* } else if (usart_interrupt_flag_get(USART1, USART_TDC_FLAG) != RESET) { */
    /*     usart_flag_clear(USART1, USART_TDC_FLAG); */
    /*     USART1->sts_bit.tdc = 0; */
    /*     uart_tx_ready = 0; */
    /* } */
}

CRITICAL void uart_send(const uint8_t *buffer, ssize_t length, int async) {
    /* while(uart_tx_ready); */
    /* uart_tx_ready = 1; */
    /* memcpy(uart_tx_buffer, buffer, length); */
    /* DMA1_CHANNEL4->maddr = (uint32_t) uart_tx_buffer; */
    /* DMA1_CHANNEL4->dtcnt = length;  */
    /* dma_channel_enable(DMA1_CHANNEL4, TRUE); */
}
int32_t tval = 0;

CRITICAL int uart_get_data(uint8_t *data, uint32_t *length) {
    /* if (uart_rx_ready) { */
    /*     uart_rx_ready = 0; */
    /*     *length = read_buffer_length; */
    /*     if (*length > UART_RX_BUFFER_SIZE) *length = UART_RX_BUFFER_SIZE; */
    /*     if (uart_rx_start == uart_rx_end) { */
    /*         return 0; */
    /*     } else if (uart_rx_end < uart_rx_start) { */
    /*         memcpy(data, uart_rx_start, UART_RX_BUFFER_SIZE - (uart_rx_start - uart_rx_buffer)); */
    /*         memcpy(&data[UART_RX_BUFFER_SIZE - (uart_rx_start - uart_rx_buffer)], uart_rx_buffer, uart_rx_end - uart_rx_buffer); */
    /*     } else { */
    /*         memcpy(data, uart_rx_start, uart_rx_end - uart_rx_start); */
    /*     } */
    /*     uart_rx_start = uart_rx_end; */
    /*     read_buffer_length = 0; */
    /*     #<{(| char bla[65] = {0}; |)}># */
    /*     #<{(| for (int i = 0; i < *length && i < 32; i++) { |)}># */
    /*     #<{(|     sprintf(&bla[i*2], "%02X", data[i]); |)}># */
    /*     #<{(| } |)}># */
    /*     #<{(| printf("%lu %s\n", *length, bla); |)}># */
    /*  */
    /*     dma_channel_enable(DMA1_CHANNEL5, TRUE); */
    /*     return 1; */
    /* } else { */
    /*     // make sure */
    /*     dma_channel_enable(DMA1_CHANNEL5, TRUE); */
    /* } */
    return 0;
}


#ifdef DEBUG
uint32_t divval(void) {
    /* return USART1->baudr_bit.div; */
}
#endif

#if (LV_USE_LOG && !defined(SEMIHOSTING) && !defined(SEGGER_RTT)) || defined(SWO)
#if LVGL_VERSION_MAJOR == 9
void lv_log_callback(lv_log_level_t lvl, const char *c) {
#else
void lv_log_callback(const char *c) {
#endif
#if defined(SWO)
    int l = strlen(c);
    while(l--) ITM_SendChar(*c++);
#else
    uart_send(c, strlen(c), 0);
#endif
}
#endif
