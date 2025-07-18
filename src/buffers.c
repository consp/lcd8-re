#include "config.h"

#ifdef PLATFORM_LCD8 
 #if DMA_WRITE
uint8_t RAMHIGH pixelbuffer[PIXEL_BUFFER_SIZE] __attribute__ ((aligned (4)));
 #else
uint8_t RAMHIGH pixelbuffer[PIXEL_BUFFER_SIZE] __attribute__ ((aligned (4)));
 #endif
#elif defined(STM32H743)
uint8_t RAM_D2 pixelbuffer[PIXEL_BUFFER_SIZE] __attribute__ ((aligned (4)));
uint8_t RAM_D2 pixelbuffer2[PIXEL_BUFFER_SIZE] __attribute__ ((aligned (4)));
uint8_t RAMHIGH framebuffer[(COLOR_SIZE * 320 * 480)] __attribute__ ((aligned (4)));
/* uint8_t *pixelbuffer2 = NULL; */
#else
uint8_t RAMHIGH pixelbuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(lv_color_t)];
#endif
