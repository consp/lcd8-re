#include "delay.h"
#include <time.h>

void delay_init(void) {};

void delay_sec(uint16_t sec) {
    struct timespec timer = { sec, 0 };
    nanosleep(&timer, NULL);
}

void delay_10ns(uint32_t ns) {
    struct timespec timer = { 0, ns * 10 };
    nanosleep(&timer, NULL);
}
void delay_ms(uint16_t ms) { delay_10ns(ms * 100000); };
void delay_us(uint32_t us) { delay_10ns(us * 100); };
