#ifndef __CONFIG_H__
#define __CONFIG_H__

#define C24_8(c, d)         ((c << 8) | (d * 256))

#define DISPLAY_WIDTH   320
#define DISPLAY_HEIGHT  480

#define TEMPERATURE_FILTER_SHIFT 3
#define SPEED_FILTER_SHIFT 2
#define POWER_FILTER_SHIFT 2

#ifdef DEBUG
#define PIXEL_BUFFER_LINES 14
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
#define WHEEL_CIRCUMFENCE 223
// change to 1 if lext installed
#define LEXT_INSTALLED 1

// change to not draw controller mode
#define DRAW_CONTROLLER_MODE

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
#define BAUD_MULTIPLIER      1                  //  
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
#define BAUD_MULTIPLIER      1                
#define EFFECTIVE_CLOCK      (4000000 / CLOCK_OFFSET)
#endif

#define TIMER_FREQ(x)        (\
            (\
                (2 * (EFFECTIVE_CLOCK) * (1 + PLL_MULTIPL)) / ((1 + (APB2_DIVIDER >> 2)) * (1 + (AHB_DIVIDER >> 3)) * 10) \
            ) / x)


/*
 * Experimental settings
 *
 * Here be dragons!
 */
// #define DMA_WRITE 1

#endif // __CONFIG_H__
