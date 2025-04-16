#include "lcd.h"
#include "uart.h"
#include "gtkdrv.h"

/* #define MEMORY_DEBUG */
uint16_t dummy = 0;


void lcd_init(void) {
}

void lcd_backlight(uint32_t value) { // 0-100
}

int lcd_start(void) {
    return 1;
}

extern lv_disp_drv_t disp_drv;

#if LVGL_VERSION_MAJOR == 9
void lcd_draw_large_text(uint32_t x, uint32_t y, const lv_image_dsc_t *img, lv_color_t color) {
#else
void lcd_draw_large_text(uint32_t x, uint32_t y, const lv_img_dsc_t *img, lv_color_t color) {
#endif
    uint8_t *data = (uint8_t *) img->data;
    data += 8;
    lv_color_t tbuffer[((img->header.h*DISPLAY_WIDTH) + img->header.w) * sizeof(lv_color_t)];
    memset(tbuffer, 0x00, sizeof(tbuffer));
    lv_color_t *pb = tbuffer;
    int n = 0;
    for (int j = 0; j < img->header.h; j++) {
        for (int i = 0; i < img->header.w; i+=8) {
            uint8_t vdata = *data;
            for (int z = 0; z < 8; z++) {
                if (vdata & 0x80) {
                    tbuffer[n++] = (lv_color_t) { .full = 0x00000000 };
                } else {
                    tbuffer[n++] = color;
                }
                vdata <<= 1;
                pb++;
            }
            data++;
        }
    }
    lv_area_t area = {0};
    area.x1 = x; area.y1 = y;
    area.x2 = x + img->header.w - 1;
    area.y2 = y + img->header.h - 1;
    gtkdrv_flush_cb(&disp_drv, &area, tbuffer);
}
