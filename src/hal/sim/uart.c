#include "uart.h"
#include "lvgl.h"
#include "lv_conf.h"
#include "crc.h"

uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE] = {0};
uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE] = {0};
volatile int uart_rx_ready = 0;
volatile int uart_tx_ready = 0;
uint32_t read_buffer_length = 0;

#if LV_USE_LOG
void lv_log_callback(const char *c);
#endif

void uart_init(uint32_t baud)
{
}


void uart_send(const uint8_t *buffer, ssize_t length, int async) {
}

int uart_get_data(uint8_t *data, uint32_t *length) {
}


#ifdef DEBUG
uint32_t divval(void) {
    return 0;
}
#endif

#if LV_USE_LOG
void lv_log_callback(const char *c) {
    uart_send(c, strlen(c), 0);
}
#endif
