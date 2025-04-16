#ifndef __CONFIG_H__
#define __CONFIG_H__

#define C24_8(c, d)         ((c << 8) | (d * 256))

#define DISPLAY_WIDTH   320
#define DISPLAY_HEIGHT  480

#define TEMPERATURE_FILTER_SHIFT 4
#define VOLTAGE_FILTER_SHIFT 4
#define SPEED_FILTER_SHIFT 2
#define POWER_FILTER_SHIFT 2

#ifdef DEBUG
#define PIXEL_BUFFER_LINES 20
#else
#ifdef PLATFORM_LCD8
#define PIXEL_BUFFER_LINES 22 // lower if more mem is required for lvgl
#else
#define PIXEL_BUFFER_LINES 32 // reduces tearing if multiple of 32
#endif
#endif
#define PIXEL_BUFFER_SIZE (PIXEL_BUFFER_LINES * 2 * DISPLAY_WIDTH)

#define REFRESH_INTERVAL 33 // refresh interval in us

#define EEPROM_SIZE 512 // device has a 2 page eeprom

/**
 * default values
 */
#define POWER_MAX 250
#define POWER_MIN -250
#define SPEED_MAX 25

#define BATTERY_MIN 21000
#define BATTERY_MAX 29400

#define GRAPH_DURATION 1
#define GRAPH_SHIFT 1
#define GRAPH_MAX 30

#define ASSIST_LEVELS 9
#define ASSIST_DEFAULT 0
#define WHEEL_CIRCUMFENCE 216
#define PAS_TIMEOUT 8000
#define PAS_RAMP 1600
#define REGEN_CURRENT 5000

/*
 * if you install a external 32khz crystal the ertc can work
 * I chose a bog standard one with 2 * 5.6pF caps.
 * to keep it running the battery will drain.
 * Adding a tiny battery will not work as the drain current is ~10mA and it's neigh impossible to
 * detect the power signal in the comperator on PA4 due to it being in the single digit mV range.
 * I've tried changing the resistors but this does not result in significant changes. I probably
 * do not understand the circuit and since no device is ever sold with an RTC I guess the original
 * developers didn't either.
 */
// #define LEXT_INSTALLED 1

// change to not draw controller mode
#define DRAW_CONTROLLER_MODE

// archtecture specifics

#if defined(AT32F415)
/*
 * OC will use 200mhz and an external crystal. 
 * I added an 8MHz crystal and two 17pF caps.
 */
#define OC // overclock to 200mhz, which is the maximum. Higher values will result in 200mhz but evert calculation is broken

#ifdef OC
#define CLOCK_SOURCE         CRM_CLOCK_SOURCE_HEXT  // you need a stable crystal
#define CLOCK_SOURCE_DIV     CRM_PLL_SOURCE_HEXT 
#define CLOCK_SOURCE_FLAG    CRM_HEXT_STABLE_FLAG
#define FLASH_WAIT_CYCLES    FLASH_WAIT_CYCLE_3 // 3 might work, ymmv
#define PLL_MULTIPL          CRM_PLL_MULT_25    // 200mhz
#define AHB_DIVIDER          CRM_AHB_DIV_1      // 200mhz
#define APB2_DIVIDER         CRM_APB2_DIV_2     // 100mhz
#define APB1_DIVIDER         CRM_APB1_DIV_2     // 100mhz
#define CLOCK_OFFSET         1                  // ext clock should be stable
#define EFFECTIVE_CLOCK      (8000000 / CLOCK_OFFSET)

#else
#define CLOCK_SOURCE         CRM_CLOCK_SOURCE_HICK
#define CLOCK_SOURCE_DIV     CRM_PLL_SOURCE_HICK
#define CLOCK_SOURCE_FLAG    CRM_HICK_STABLE_FLAG
#define FLASH_WAIT_CYCLES    FLASH_WAIT_CYCLE_4 // 
#define PLL_MULTIPL          CRM_PLL_MULT_36    // 144mhz
#define AHB_DIVIDER          CRM_AHB_DIV_1      // @cpu
#define APB2_DIVIDER         CRM_APB2_DIV_2     // 72mhz
#define APB1_DIVIDER         CRM_APB1_DIV_2     // 72mhz
#define CLOCK_OFFSET         1
#define EFFECTIVE_CLOCK      (4000000 / CLOCK_OFFSET)
#endif

#define TIMER_FREQ(x)        (\
            (\
                (2 * (EFFECTIVE_CLOCK) * (1 + PLL_MULTIPL)) / ((1 + (APB2_DIVIDER >> 2)) * (1 + (AHB_DIVIDER >> 3)) * 10) \
            ) / x)

#include "at32f415.h"
#elif defined(GD32F303)
#include "gd32f30x.h"
#endif



/*
 * Experimental settings
 *
 * Here be dragons!
 *
 * You have been warned.
 */

/* 
 * enabeling dma might work or not, depending on the quality
 * of the signal and other "if's"
 * Due to how the timer works it always overshoots the dma requests
 * AFAIK there is no way to stop the timer at the end of the 
 * dma requests in time at speeds over ~5MHz.
 * Currently it's set to output around 12.5MHz signal rate which is the limit at which misses start to occur.
 * Due to being unable to stop the timer on time the final pixels (2) will need to be redrawn, costing an additional 840ns in the interrupt at X_BACKOFF = 2.
 * The advantage being you still het the 1000us of spare time per bigger draw to play with.
 *
 * With the small buffers used you will not achieve any speedup (Since SW bitbanging runs about 25%-30% faster). If you somehow become calculation limited it might be worth it.
 *
 * Change X_BACKOFF to determine how many pixels will be redrawn. If you see random pixels on the next line or missing black ones on the right edge, use this to fix that issue
 * This is due to the "ILI9488" not ignoring pixls outside it's range set Page/Column range like per spec. Might be due to it being a knockoff.
 */
// #define DMA_WRITE 1
#define X_BACKOFF 3 

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#endif // __CONFIG_H__
