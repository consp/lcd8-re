#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>
#include <unistd.h>
#include "config.h"

#ifdef DEBUG
#define UART_TX_BUFFER_SIZE     128
#define UART_RX_BUFFER_SIZE     128
#else
#define UART_TX_BUFFER_SIZE     32 
#define UART_RX_BUFFER_SIZE     32 
#endif

void uart_init(uint32_t baud);
void uart_send(const uint8_t *buffer, ssize_t length, int async);
int uart_get_data(uint8_t *data, uint32_t *length);

#endif // __UART_H__
