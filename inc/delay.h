#include "stdio.h"
#include "at32f415.h"

#define NS_CALC(s) (s / 10)

void delay_init(void);
void delay_10ns(uint32_t ns);
void delay_us(uint32_t nus);
void delay_ms(uint16_t nms);
void delay_sec(uint16_t sec);
