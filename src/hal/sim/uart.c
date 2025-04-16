#include "uart.h"
#include "lvgl.h"
#include "lv_conf.h"
#include "crc.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>  
#include <termios.h>
#include <signal.h>
#include <sys/select.h>

uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE] = {0};
uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE] = {0};
volatile int uart_rx_ready = 0;
volatile int uart_tx_ready = 0;
uint32_t read_buffer_length = 0;

#if LV_USE_LOG
void lv_log_callback(const char *c);
#endif

void sig_handler(int sig) {
    struct termios attr;
    switch (sig) {
        case SIGSEGV:
        case SIGKILL:
            /* tcgetattr(STDIN_FILENO, &attr); */
            /* attr.c_lflag |= (ICANON | ECHO); */
            /* tcsetattr(STDIN_FILENO, TCSANOW, &attr); */
            abort();
            break;
        default:
            /* tcgetattr(STDIN_FILENO, &attr); */
            /* attr.c_lflag |= (ICANON | ECHO); */
            /* tcsetattr(STDIN_FILENO, TCSANOW, &attr); */
            /* abort(); */
            break;
    }
}

void uart_init(uint32_t baud)
{   
    /* struct termios attr; */
    /* tcgetattr(STDIN_FILENO, &attr); */
    /* attr.c_lflag &= ~(ICANON | ECHO); */
    /* tcsetattr(STDIN_FILENO, TCSANOW, &attr); */
    /*  */
    setvbuf(stdout, (char *)NULL, _IONBF, 0); 
    /*  */
    signal(SIGSEGV, sig_handler);
}


void uart_send(const uint8_t *buffer, ssize_t length, int async) {
    fwrite(buffer, length, sizeof(uint8_t), stdout);
    fflush(stdout);
}

int uart_get_data(uint8_t *data, uint32_t *length) {
    // the assumption the length of the data buffer is equal to the UART_RX_BUFFER_SIZE
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    if (FD_ISSET(0, &fds)) {
        ssize_t rv = read(STDIN_FILENO, uart_rx_buffer, UART_RX_BUFFER_SIZE); 
        /* ssize_t rv = fread(uart_rx_buffer, sizeof(uint8_t), UART_RX_BUFFER_SIZE, stdin);  */
        if (rv > 0) {
            *length = rv;
            memcpy(data, uart_rx_buffer, *length);
            return 1;
        }
    }
    return 0;
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
