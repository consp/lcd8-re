#ifndef __LCD_H__
#define __LCD_H__

#include "config.h"
#include "delay.h"
#include "lvgl.h"

// functions must be implemented in HAL
void lcd_init(void);
void lcd_backlight(int16_t enable);
int lcd_start(void);
#if COLOR_SIZE == 2
CRITICAL void lcd_fill(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint16_t color);
#else
CRITICAL void lcd_fill(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color);
#endif
#if LVGL_VERSION_MAJOR == 9
void lcd_draw_large_text(uint32_t x, uint32_t y, const lv_image_dsc_t *img, lv_color_t color);
void lcd_lvgl_flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixmap);
#else
void lcd_draw_large_text(uint32_t x, uint32_t y, const lv_img_dsc_t *img, lv_color_t color);
void lcd_lvgl_flush(lv_disp_drv_t *display, const lv_area_t *area, lv_color_t *pixmap);
#endif

CRITICAL void read_register(uint8_t command, void *location, uint32_t bits, uint32_t num);

#define ILI_SOFT_RESET                  0x01
#define ILI_SLEEP_OUT                   0x11
#define ILI_SLEEP_IN                    0x10
#define ILI_DISPLAY_INVERSION_ON        0x21
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
#define ILI_DISPLAY_BLANKING_PORCH_CONTROL 0xB5
#define ILI_DISPLAY_FUNCTION_CONTROL    0xB6
#define ILI_POWER_CONTROL_1             0xC0
#define ILI_POWER_CONTROL_2             0xC1
#define ILI_POWER_CONTROL_3             0xC2
#define ILI_VCOM_CONTROL_1              0xC5
#define ILI_CABC_CONTROL_9              0xCF
#define ILI_SET_IMAGE_FUNCTION          0xE9
#define ILI_ADJUST_CONTROL_3            0xF7

#define ILI_COL_ADDR_SET                0x2A
#define ILI_PA_ADDR_SET                 0x2B
#define ILI_MEMORY_WRITE                0x2C

#define ST_CSC                          0xF0
#define ST_ENTRY_MODE_SET               0xB7
#define ST_DISPLAY_OUTPUT_CTRL_ADJUST   0xE8

#if COLOR_SIZE == 2
#define RGB(r, g, b)            ((uint16_t) (((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | b >> 3))
#else
#define RGB(r, g, b)            ((uint32_t) (r << 16 | g << 8 | b))
#endif
/**
 *
 * commands
 */
#ifdef TOUCH_ENABLED
void touch_init(void);

// struct magic is for le/be conversion
// you must read from byte 0, otherwise you will not get the correct data
#pragma pack(push, 1)
typedef struct {
    union {
        struct {
            uint16_t pre;
            uint8_t num;
            union {
                uint16_t x1_raw;
                struct {
                    int16_t x1 : 12;
                    int16_t unknown : 4;
                };
            };
            int16_t y1;
            uint8_t post;
        };
        uint8_t data[8];
    };
    // uint8_t; // always 0x10
    // uint8_t type2; // 0x80 -> press, 0xff
    // int16_t x2; // note: in big endian
    // int16_t y2; // note: in big endian
    // uint8_t; // always 0x10 iff 2 press, otherwise 0xff
} touch_points;
#pragma pack(pop)

int touch_get_pos(int16_t *x, int16_t *y, int32_t *presses);
#endif

#endif // __LCD_H__
