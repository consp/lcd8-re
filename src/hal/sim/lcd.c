#include "lcd.h"
#include "uart.h"

/* #define MEMORY_DEBUG */
uint16_t dummy = 0;


void lcd_init(void) {
}

void lcd_backlight(uint32_t value) { // 0-100
}

int lcd_start(void) {
}

#if LVGL_VERSION_MAJOR == 9
void lcd_draw_large_text(uint32_t x, uint32_t y, const lv_image_dsc_t *img, uint16_t color) {
#else
void lcd_draw_large_text(uint32_t x, uint32_t y, const lv_img_dsc_t *img, uint16_t color) {
#endif
}
