#include "config.h"
#include "gd32f30x_gpio.h"
#include "delay.h"
#include "lvgl.h"

#define PIN_READ    GPIO_PIN_8 
#define PIN_WRITE   GPIO_PIN_9 
#define PIN_CD      GPIO_PIN_10
#define PIN_CS      GPIO_PIN_11

/*
 * Timings
 *
 * Fmclk max 110Mhz, pw ~10ns
 * 500ns per r/w cycle assumes 2MHz, which should be achievable
 *
 * Write:
 * Pull READ high
 * Pull CD to C or D (no setup)
 * Pull CS# low, min 20ns delay
 * Setup DATA
 * Pull Write strobe low, keep low for at least 50ns
 * Pull Write strobe high for at least 50ns
 * Pull CS# high before next cycle
 *
 * Read:
 * Pull WRITE high
 * Pull CD to D (no setup)
 * Pull CS# low, min 20ns delay
 */



#define COLOR_565
//#define COLOR_666

/** 
 * Color conversions
 */
#ifdef COLOR_565
#define ILI_RGB(r, g, b) (((r >> 3) << 11) | ((g >> 2) << 5) | ((b >> 3)))
#endif

/*
 * on continuous draw, this will take ~15ms per frame which is faster than the tear refresh rate resulting in tear free redraws 
 */
#define DELAY_NOP_6        asm(".syntax unified; .rept 1 ; nop ; .endr");
#define DELAY_NOP_20        asm(".syntax unified; .rept 3 ; nop ; .endr");
#define DELAY_NOP_50        asm(".syntax unified; .rept 7 ; nop ; .endr");
#define DELAY_NOP_200       asm(".syntax unified; .rept 24 ; nop ; .endr");
#define DELAY_NOP_400       delay_10ns(40);
#define DELAY_NOP_500       delay_10ns(50);
// #define DELAY_NOP_400       asm(".syntax unified; .rept 59 ; nop ; .endr");
// #define DELAY_NOP_500       asm(".syntax unified; .rept 74 ; nop ; .endr");
//

// #define DELAY_NOP_6         delay_10ns(1);
// #define DELAY_NOP_20        delay_10ns(2);
// #define DELAY_NOP_50        delay_10ns(5);
// #define DELAY_NOP_200       delay_10ns(20);
// #define DELAY_NOP_400       delay_10ns(40);
// #define DELAY_NOP_500       delay_10ns(50);
/*
 * 1MHz
 */
// #define DELAY_NOP_6        asm(".rept 2 ; nop ; .endr");
// #define DELAY_NOP_20        asm(".rept 6 ; nop ; .endr");
// #define DELAY_NOP_50        asm(".rept 14 ; nop ; .endr");
// #define DELAY_NOP_200       asm(".rept 48 ; nop ; .endr");
// #define DELAY_NOP_400       asm(".rept 118 ; nop ; .endr");
// #define DELAY_NOP_500       asm(".rept 148 ; nop ; .endr");

// #define DELAY_NOP_6        asm(".rept 2 ; nop ; .endr"); ; 
// #define DELAY_NOP_20        delay_10ns(1);
// #define DELAY_NOP_50        delay_10ns(2);
// #define DELAY_NOP_200       delay_10ns(4);
// #define DELAY_NOP_400       delay_10ns(8);
// #define DELAY_NOP_500       delay_10ns(10);

#define WRITE_DATA(x)       GPIO_BOP(GPIOB) = x
#define CLEAR_DATA()        GPIO_BC(GPIOB) = 0xFFFF; 

#define SET_READ()          GPIO_BC(GPIOB) = 0; GPIO_CTL0(GPIOB) = 0x44444444; GPIO_CTL1(GPIOB) = 0x44444444; 
#define SET_WRITE()         GPIO_BC(GPIOB) = 0; GPIO_CTL0(GPIOB) = 0x33333333; GPIO_CTL1(GPIOB) = 0x33333333;

#define CS_IDLE             GPIO_BOP(GPIOC) = PIN_CS
#define CS_ACTIVE           GPIO_BC(GPIOC) = PIN_CS

#define READ_IDLE           GPIO_BOP(GPIOC) = PIN_READ
#define READ_ACTIVE         GPIO_BC(GPIOC) = PIN_READ

#define COMMAND             GPIO_BC(GPIOC) = PIN_CD
#define DATA                GPIO_BOP(GPIOC) = PIN_CD

#define WRITE_IDLE          GPIO_BOP(GPIOC) = PIN_WRITE
#define WRITE_ACTIVE        GPIO_BC(GPIOC) = PIN_WRITE

#define WRITE_8BIT(x)       WRITE_DATA(0x00FF & x)
#define WRITE_16BIT(x)      WRITE_DATA(x)
#define READ()              ((uint16_t)(GPIO_ISTAT(GPIOB) & 0x0000FFFF))
