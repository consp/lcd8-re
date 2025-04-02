#ifndef __LCD_H__
#define __LCD_H__

#include "config.h"
#include "at32f415_gpio.h"
#include "delay.h"
#include "lvgl.h"

// #define USE_TMR_INT

void lcd_init(void);
void lcd_backlight(uint32_t enable);

/* ugui accelerators */
#if 0
void lcd_draw(UG_S16 x, UG_S16 y, UG_COLOR color);
UG_RESULT lcd_draw_line(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR color);
UG_RESULT lcd_fill(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR color);
void (*lcd_fill_area(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2))(uint32_t, UG_COLOR);
UG_RESULT lcd_draw_bmp(UG_S16 x, UG_S16 y, UG_BMP *bmp);
#endif
void lcd_fill(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint16_t color);
void lcd_update(void);

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

#define WRITE_DATA(x)       GPIOB->odt = x
#define CLEAR_DATA()        GPIOB->odt = 0

// #define WRITE_STROBE()      DELAY_NOP_50; GPIOC->scr = PIN_WRITE; DELAY_NOP_50
// #define READ_STROBE()       GPIOC->clr = PIN_READ; DELAY_NOP_200; GPIOC->scr = PIN_RoEAD; DELAY_NOP_200;

#define SET_READ()          GPIOB->odt = 0; GPIOB->cfglr = 0x44444444; GPIOB->cfghr = 0x44444444; 
#define SET_WRITE()         GPIOB->odt = 0; GPIOB->cfglr = 0x33333333; GPIOB->cfghr = 0x33333333;

#define WRITE_STROBE_TMR()        GPIOC->cfghr &= 0xFFFFFF0F; GPIOC->cfglr |= 0xFFFFFFBF
#define WRITE_STROBE_GPIO()       GPIOC->cfghr &= 0xFFFFFF0F; GPIOC->cfglr |= 0xFFFFFF3f

#define CS_IDLE             GPIOC->scr = PIN_CS
#define CS_ACTIVE           GPIOC->clr = PIN_CS

#define READ_IDLE           GPIOC->scr = PIN_READ
#define READ_ACTIVE         GPIOC->clr = PIN_READ

#define COMMAND             GPIOC->clr = PIN_CD
#define DATA                GPIOC->scr = PIN_CD

#define WRITE_IDLE          GPIOC->scr = PIN_WRITE
#define WRITE_ACTIVE        GPIOC->clr = PIN_WRITE

#define WRITE_8BIT(x)       WRITE_DATA(0x00FF & x)
#define WRITE_16BIT(x)      WRITE_DATA(x)
#define READ()              ((uint16_t)(GPIOB->idt & 0x0000FFFF))


#define ILI_SOFT_RESET                  0x01
#define ILI_SLEEP_OUT                   0x11
#define ILI_SLEEP_IN                    0x10
#define ILI_DISPLAY_ON                  0x29
#define ILI_DISPLAY_OFF                 0x28
#define ILI_MEMORY_ACCESS_CONTROL       0x36
#define ILI_INTERFACE_PIXEL_FORMAT      0x3A
#define ILI_WRITE_CTRL_DISPLAY          0x53
#define ILI_WRITE_DISPLAY_BRIGHTNESS    0x51
#define ILI_POSITIVE_GAMMA_CORRECTION   0xE0
#define ILI_NEGATIVE_GAMMA_CORRECTION   0xE1
#define ILI_INTERFACE_MODE_CONTROL      0xB0
#define ILI_FRAME_RATE_CONTROL_NORMAL   0xB1
#define ILI_DISPLAY_INVERSION_CONTROL   0xB4
#define ILI_DISPLAY_FUNCTION_CONTROL    0xB6
#define ILI_POWER_CONTROL_1             0xC0
#define ILI_POWER_CONTROL_2             0xC1
#define ILI_VCOM_CONTROL_1              0xC5
#define ILI_CABC_CONTROL_9              0xCF
#define ILI_SET_IMAGE_FUNCTION          0xE9
#define ILI_ADJUST_CONTROL_3            0xF7

#define ILI_COL_ADDR_SET                0x2A
#define ILI_PA_ADDR_SET                 0x2B
#define ILI_MEMORY_WRITE                0x2C

#define RGB(r, g, b)            ((uint16_t) (((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | b >> 3))
/**
 *
 * commands
 */


void set_address_window(uint32_t x, uint32_t y, uint32_t x2, uint32_t y2);
void fill(uint32_t color, uint32_t len);
int lcd_start(void);
void lcd_test(void);

void lcd_command(uint8_t cmd);


#if LVGL_VERSION_MAJOR == 9
void lcd_draw_large_text(uint32_t x, uint32_t y, const lv_image_dsc_t *img, uint16_t color);
void lcd_lvgl_flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixmap);
#else
void lcd_draw_large_text(uint32_t x, uint32_t y, const lv_img_dsc_t *img, uint16_t color);
void lcd_lvgl_flush(lv_disp_drv_t *display, const lv_area_t *area, lv_color_t *pixmap);
#endif
#endif // __LCD_H__
