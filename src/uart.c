#include "uart.h"
#include "at32f415_dma.h"
#include "lvgl.h"
#include "lv_conf.h"
#include "eeprom.h"
#include <machine/endian.h>

uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE] = {0};
uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE] = {0};
volatile int uart_rx_ready = 0;
volatile int uart_tx_ready = 0;
uint32_t read_buffer_length = 0;

// externals
//
extern int32_t speed, battery_current, battery_voltage_controller, mot_temperature, con_temperature, wheel_circumfence;
extern uint8_t draw_power_trigger, draw_speed_trigger, draw_temperatures_trigger, brake, controller_mode, draw_battery_voltage_trigger;
extern settings_t settings;

static void uart_init_dma(void);
#if DEBUG && DEBUG_UART_PRINT
void lv_log_callback(const char *c);
#endif

void uart_init(uint32_t baud)
{
    gpio_init_type gpio_init_struct;
    /* enable the usart and it's io clock */
    crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

    /* set default parameter */
    gpio_default_para_init(&gpio_init_struct);

    /* configure the usart1_tx  pa9 */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = GPIO_PINS_9;
    gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
    gpio_init(GPIOA, &gpio_init_struct);

    /* configure the usart1_rx  pa10 */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = GPIO_PINS_10;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(GPIOA, &gpio_init_struct);

    /*configure usart nvic interrupt */
    nvic_irq_enable(USART1_IRQn, 0, 0);

    /*configure usart param*/
    usart_init(USART1, baud, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_transmitter_enable(USART1, TRUE);
    usart_receiver_enable(USART1, TRUE);
    usart_interrupt_enable(USART1, USART_IDLE_INT, TRUE);
    usart_dma_transmitter_enable(USART1, TRUE);
    usart_dma_receiver_enable(USART1, TRUE);
    usart_enable(USART1, TRUE);

    uart_init_dma();

#if DEBUG && LV_USE_LOG
#pragma message("Using uart to log LV messages")
    lv_log_register_print_cb(lv_log_callback);
#endif
}

static void uart_init_dma(void) {

    dma_init_type dma_init_struct;
    crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);

    dma_reset(DMA1_CHANNEL4);
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = UART_TX_BUFFER_SIZE;
    dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_base_addr = (uint32_t) uart_tx_buffer;
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.peripheral_base_addr = (uint32_t)&USART1->dt;
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
    dma_init_struct.loop_mode_enable = FALSE;
    dma_init(DMA1_CHANNEL4, &dma_init_struct);

    dma_interrupt_enable(DMA1_CHANNEL4, DMA_FDT_INT, TRUE);
    nvic_irq_enable(DMA1_Channel4_IRQn, 0, 0);

    /* dma_flexible_config(DMA1, FLEX_CHANNEL4, DMA_FLEXIBLE_UART1_TX); */

    dma_reset(DMA1_CHANNEL5);
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = UART_RX_BUFFER_SIZE;
    dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_base_addr = (uint32_t) uart_rx_buffer; 
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.peripheral_base_addr = (uint32_t)&USART1->dt;
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
    dma_init_struct.loop_mode_enable = FALSE;
    dma_init(DMA1_CHANNEL5, &dma_init_struct);

    dma_interrupt_enable(DMA1_CHANNEL5, DMA_FDT_INT, TRUE);

    nvic_irq_enable(DMA1_Channel5_IRQn, 0, 0);
    /* dma_flexible_config(DMA1, FLEX_CHANNEL5, DMA_FLEXIBLE_UART1_RX); */

    dma_channel_enable(DMA1_CHANNEL4, FALSE);
    dma_channel_enable(DMA1_CHANNEL5, TRUE);

}


void USART1_IRQHandler(void)
{
    /* if(usart_interrupt_flag_get(USART1, USART_RDBF_FLAG) != RESET) */
    /* { */
    /*     uart_rx_ready = 1;  */
    /* } */
    if(usart_interrupt_flag_get(USART1, USART_IDLEF_FLAG) != RESET)
    {
        usart_flag_clear(USART1, USART_IDLEF_FLAG);
        read_buffer_length = DMA1_CHANNEL5->dtcnt;

        uart_rx_ready = 1; 

        dma_channel_enable(DMA1_CHANNEL5, FALSE);
        DMA1_CHANNEL5->maddr = (uint32_t) uart_rx_buffer;
        DMA1_CHANNEL5->dtcnt = UART_RX_BUFFER_SIZE;
    }
}

void DMA1_Channel4_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA1_FDT4_FLAG))
    {
        uart_tx_ready = 0;
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
        dma_channel_enable(DMA1_CHANNEL5, FALSE);
    }
}

void uart_send(const uint8_t *buffer, ssize_t length, int async) {
    if (!async) while(uart_tx_ready); // wait for available
    uart_tx_ready = 1;
    memcpy(uart_tx_buffer, buffer, length);
    DMA1_CHANNEL4->maddr = (uint32_t) uart_tx_buffer;
    DMA1_CHANNEL4->dtcnt = length; 
    dma_channel_enable(DMA1_CHANNEL4, TRUE);
}
int32_t tval = 0;

void uart_update(void) {
    // parse if available and update variables
    if (uart_rx_ready) {
        ssize_t length = 0;
        uart_rx_ready = 0;
        while (length < read_buffer_length) {
            msg_controller *msg = (msg_controller *) &uart_rx_buffer[length];
            // calidate crc
            uint8_t crc = crc_calc_uart((uint8_t *) msg, msg->length);
            if (msg->crc == crc) {
                switch (msg->tag) {
                    case (MSG_VOLTAGE):
                        battery_voltage_controller = (int32_t) __ntohl(msg->voltage.value);
                        draw_battery_voltage_trigger = 1;
                        break;
                    case (MSG_CURRENT):
                        battery_current = (int32_t) __ntohl(msg->current.value); 
                        draw_power_trigger = 1; 
                        break;
                    case (MSG_ROTATION):
                        tval = __ntohs(msg->rotation.value);
                        if (tval > 0) {
                            // enable
                            TMR5->pr = (tval * 100)-1;
                            speed = (((int32_t)settings.wheel_circumfence) * 36000L) / (tval); 
                        } else {
                            TMR5->pr = 0;
                            speed = 0;
                        }
                        draw_speed_trigger = 1;
                        break;
                    case (MSG_TEMPERATURE):
                        mot_temperature = __ntohs(msg->temperature.engine); // in 0.1C
                        con_temperature = __ntohs(msg->temperature.controller); // in 0.1C 
                        draw_temperatures_trigger = 1;
                        break;
                    case (MSG_CONTROLLER_STATUS):
                        brake = msg->status.brake;
                        controller_mode = msg->status.mode;
                        break;
                    default:
                        break;
                }
                length += msg->length + 3;
            } else {
                length += 3;
            }
        }
        read_buffer_length = 0;
        // reset dma
    }

    dma_channel_enable(DMA1_CHANNEL5, TRUE);
}

void uart_send_display_settings(void) {
    uint8_t data[3 + sizeof(msg_display_settings)] = { MSG_DISPLAY_SETTINGS };
    msg_display *msg = (msg_display *) data;
    msg->length = sizeof(msg_display_settings); 
   
    msg->settings.max_current = __htons(settings.current_max);
    msg->settings.wheel_circumfence = __htons(settings.wheel_circumfence);
    msg->settings.max_speed = settings.speed_assist_max;
    msg->settings.assist_levels = settings.assist_levels;

    msg->crc = crc_calc_uart((uint8_t *) msg, sizeof(msg_display_settings));
    
    uart_send(data, 3 + sizeof(msg_display_settings), 0);
}

void uart_send_display_status(void) {
    uint8_t data[3 + sizeof(msg_display_status)] = { MSG_DISPLAY_STATUS };
    msg_display *msg = (msg_display *) data;
    msg->length = sizeof(msg_display_status);
    msg->status.assist_level = settings.assist_last;
    msg->status.lights = settings.lights_enabled;
    msg->crc = crc_calc_uart((uint8_t *) msg, sizeof(msg_display_status));
    
    uart_send(data, 3 + sizeof(msg_display_status), 0);
}

void uart_send_controller_settings(void) {
}

/* int get_msg(uint8_t *buffer, ssize_t *length) { */
/*     if (!uart_tx_ready) return 0; */
/*     if (NULL == buffer) return 0; */
/*     *length = DMA1_CHANNEL5->dtcnt; */
/*     #<{(| memcpy(buffer, uart_rx_buffer, *length); |)}># */
/*     DMA1_CHANNEL5->dtcnt = 0; */
/*     DMA1_CHANNEL5->maddr = (uint32_t ) uart_rx_buffer; */
/*     uart_rx_ready = 0; */
/* } */

#if DEBUG && LV_USE_LOG
void lv_log_callback(const char *c) {
    uart_send(c, strlen(c), 0);
}
#endif

#ifdef DEBUG
uint32_t divval(void) {
    return USART1->baudr_bit.div;
}
#endif
