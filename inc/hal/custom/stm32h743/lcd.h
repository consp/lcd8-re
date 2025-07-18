#ifndef __LCD_H_435__
#define __LCD_H_435__
#include "config.h"
#include "delay.h"
#include "lvgl.h"

#define PIN_READ    GPIO_PINS_8 
#define PIN_WRITE   GPIO_PINS_9 
#define PIN_CD      GPIO_PINS_10
#define PIN_CS      GPIO_PINS_11

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

/*
 * on continuous draw, this will take ~15ms per frame which is faster than the tear refresh rate resulting in tear free redraws 
 */

// #define DELAY_NOP_6        
// #define DELAY_NOP_20       
// #define DELAY_NOP_50        asm(".syntax unified; .rept 7 ; nop ; .endr");
// #define DELAY_NOP_200       asm(".syntax unified; .rept 24 ; nop ; .endr");
// #define DELAY_NOP_400       delay_10ns(40);
// #define DELAY_NOP_500       delay_10ns(50);
#define DELAY_NOP_6         asm(".syntax unified; .rept 1 ; nop ; .endr");
#define DELAY_NOP_10        asm(".syntax unified; .rept 3 ; nop ; .endr");
#define DELAY_NOP_15        asm(".syntax unified; .rept 5 ; nop ; .endr");
#define DELAY_NOP_20        asm(".syntax unified; .rept 6 ; nop ; .endr");
#define DELAY_NOP_50        asm(".syntax unified; .rept 15 ; nop ; .endr");
#define DELAY_NOP_200       asm(".syntax unified; .rept 57 ; nop ; .endr");
// #define DELAY_NOP_400       delay_10ns(40);
// #define DELAY_NOP_500       delay_10ns(50);
// #define DELAY_NOP_400       asm(".syntax unified; .rept 59 ; nop ; .endr");
// #define DELAY_NOP_500       asm(".syntax unified; .rept 74 ; nop ; .endr");
//

// #define DELAY_NOP_6         asm(".syntax unified; .rept 2 ; nop ; .endr");
// #define DELAY_NOP_10        asm(".syntax unified; .rept 6 ; nop ; .endr");
// #define DELAY_NOP_15        asm(".syntax unified; .rept 10; nop ; .endr");
// #define DELAY_NOP_20        asm(".syntax unified; .rept 12; nop ; .endr");
// #define DELAY_NOP_50        asm(".syntax unified; .rept 30 ; nop ; .endr");
// #define DELAY_NOP_200       asm(".syntax unified; .rept 114; nop ; .endr");
#define DELAY_NOP_400       delay_10ns(40);
#define DELAY_NOP_500       delay_10ns(50);
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

#define WRITE_DATA(x)
#define CLEAR_DATA()

#define SET_READ()
#define SET_WRITE()

#define CS_IDLE
#define CS_ACTIVE

#define READ_IDLE
#define READ_ACTIVE

#define COMMAND
#define DATA

#define WRITE_IDLE
#define WRITE_ACTIVE

#define WRITE_8BIT(x)
#define WRITE_16BIT(x)
#define READ()

#endif
