#ifndef __LCD_H__
#define __LCD_H__

#include "config.h"
#include "delay.h"
#include "lvgl.h"

// functions must be implemented in HAL
void lcd_init(void);
void lcd_backlight(uint32_t enable);
int lcd_start(void);
#if LVGL_VERSION_MAJOR == 9
void lcd_draw_large_text(uint32_t x, uint32_t y, const lv_image_dsc_t *img, lv_color_t color);
void lcd_lvgl_flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixmap);
#else
void lcd_draw_large_text(uint32_t x, uint32_t y, const lv_img_dsc_t *img, lv_color_t color);
void lcd_lvgl_flush(lv_disp_drv_t *display, const lv_area_t *area, lv_color_t *pixmap);
#endif



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


#endif // __LCD_H__
