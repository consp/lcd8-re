/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */
#define LOOP 1
#include <string.h>
#include <stdio.h>
#include "config.h"
#ifndef PLATFORM_SIM
 #if defined(AT32F415)
  #include "at32f415.h"
  #include "at32f415_clock.h"
  #include <cmsis/core/core_cm4.h>
 #elif defined(AT32F435)
  #include "at32f435_437.h"
  #include <cmsis/core/core_cm4.h>
 #elif defined(STM32H743)
  #include "stm32h7xx.h"
  #include <cmsis/core/core_cm7.h>
 #endif
#endif
#ifdef PLATFORM_SIM
#include <SDL_timer.h>
#endif

#include "lcd.h"
#include "delay.h"
#include "eeprom.h"
#include "controls.h"
#include "gui.h"
#include "uart.h"
#include "clock.h"
#include "crc.h"
#include "comm.h"
#include "uart.h"

#ifdef SEGGER_RTT
#include "rtt/SEGGER_RTT.h"
#endif

extern lv_display_t *display;
extern volatile uint32_t shutdown_timer;
extern settings_t settings;
/**
  * @brief  main function.
  * @param  none
  * @retval none
  */

#ifdef PLATFORM_LCD8 
 #if DMA_WRITE
uint8_t RAMHIGH pixelbuffer[PIXEL_BUFFER_SIZE] __attribute__ ((aligned (4)));
 #else
uint8_t RAMHIGH pixelbuffer[PIXEL_BUFFER_SIZE] __attribute__ ((aligned (4)));
 #endif
#elif defined(STM32H743)
uint8_t RAMHIGH pixelbuffer[PIXEL_BUFFER_SIZE] __attribute__ ((aligned (4)));
uint8_t RAMHIGH pixelbuffer2[PIXEL_BUFFER_SIZE] __attribute__ ((aligned (4)));
uint8_t RAMHIGH framebuffer[(COLOR_SIZE * 320 * 480)] __attribute__ ((aligned (4)));
/* uint8_t *pixelbuffer2 = NULL; */
#else
 #if LVGL_VERSION_MAJOR == 8
  #include "gtkdrv.h"
 #endif
uint8_t RAMHIGH pixelbuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(lv_color_t)];
#endif

#if defined(SEMIHOSTING) && !defined(PLATFORM_SIM)
extern void initialise_monitor_handles(void);
#endif

#if defined(SWO)
extern int __io_getchar(void) __attribute__((weak));
int __io_putchar(int ch)
{
    ITM_SendChar(ch);
    return(ch);
}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
  (void)file;
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    *ptr++ = __io_getchar();
  }

  return len;
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
  (void)file;
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    __io_putchar(*ptr++);
  }
  return len;
}
void ITM_enable(void) {
#define WWORD(x) *((__IO uint32_t *)(x))
#define SWO_BASE (0x5C003000UL)
#define SWTF_BASE (0x5C004000UL)
    uint32_t SWOSpeed = 7500000; /* [Hz] we have 2 Mbps SWO speed in ST-Link SWV viewer if we select 400000000 Hz core clock */
    uint32_t SWOPrescaler = (SystemCoreClock / SWOSpeed) - 1; /* divider value */

    //enable debug clocks
    DBGMCU->CR = 0x00700000; //enable debug clocks

    //UNLOCK FUNNEL
    //SWTF->LAR unlock

    //SWO->LAR unlock
    WWORD(SWO_BASE + 0xFB0) = 0xC5ACCE55; //unlock SWO
    
    if (WWORD(SWO_BASE + 0xFB4) & 0x00000002) {
        while(1); // blocked from access
    }
    //SWO divider setting
    //This divider value (0x000000C7) corresponds to 400Mhz core clock
    //SWO->CODR = PRESCALER[12:0]
    WWORD(SWO_BASE + 0x010) = (WWORD(SWO_BASE + 0x010) & 0xFFFFF000) | SWOPrescaler; //clock divider
    WWORD(SWO_BASE + 0x0F0) = (WWORD(SWO_BASE + 0x0F0) & 0xFFFFFFF0) | 0x00000002; //set to NRZ
    /* WWORD(SWO_BASE + 0x300) = (WWORD(SWO_BASE + 0x300) & 0xFFFFFFF0) | 0x00000008; // */

    WWORD(SWTF_BASE + 0xFB0) = 0xC5ACCE55; //unlock SWTF
    WWORD(SWTF_BASE + 0x000) = 0x00000003; //enable port 0 and 1

    // enable poer
    WWORD(0x580244E0) |= 0x00000002;
    WWORD(0x58020400) = (WWORD(0x58020400) & 0xffffff3f) | 0x00000080;
    WWORD(0x58020408) |= 0x00000080;
    WWORD(0x58020420) &= 0xffff0fff;

}

void DWT_enable(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;    
}
#endif

static void rounder_cb(lv_event_t *e)
{
    lv_area_t *area = lv_event_get_param(e);

    /* Round the height to the nearest multiple of 8 */
    area->x1 = (area->x1 & ~0x1);
    area->x2 = (area->x2 | 0x1);
}

CRITICAL void lv_setup(void) {
#if LVGL_VERSION_MAJOR == 8
    {
    lv_disp_drv_init(&disp_drv);
 #ifdef PLATFORM_LCD8
  #if DMA_WRITE
        // double buff
        lv_disp_draw_buf_init(&draw_buf, pixelbuffer, pixelbuffer2, (PIXEL_BUFFER_LINES * DISPLAY_WIDTH) >> 1);
  #else
        lv_disp_draw_buf_init(&draw_buf, pixelbuffer, NULL, PIXEL_BUFFER_LINES * DISPLAY_WIDTH);
  #endif
 #else
        lv_disp_draw_buf_init(&draw_buf, pixelbuffer, NULL, DISPLAY_HEIGHT * DISPLAY_WIDTH);
 #endif

        disp_drv.draw_buf = &draw_buf;
 #ifdef PLATFORM_SIM
        disp_drv.flush_cb = gtkdrv_flush_cb;
 #else
        disp_drv.flush_cb = lcd_lvgl_flush;    /*Set your driver function*/
 #endif
        disp_drv.set_px_cb = NULL;
        disp_drv.antialiasing = TRUE;
        disp_drv.hor_res = DISPLAY_WIDTH;   /*Set the horizontal resolution of the disp_drv*/
        disp_drv.ver_res = DISPLAY_HEIGHT;   /*Set the vertical resolution of the disp_drv*/
        display = lv_disp_drv_register(&disp_drv);
    }
#else
    {
 #ifdef PLATFORM_SIM
        display = lv_sdl_window_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
 #else
        display = lv_display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
  #if PIXEL_BUFFER_SIZE == (DISPLAY_WIDTH * COLOR_SIZE * DISPLAY_HEIGHT)
        lv_display_set_buffers(display, pixelbuffer, NULL, PIXEL_BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_DIRECT);
  #else
        lv_display_set_buffers(display, pixelbuffer, pixelbuffer2, PIXEL_BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  #endif
        lv_display_set_flush_cb(display, lcd_lvgl_flush);
 #endif

        lv_display_add_event_cb(display, rounder_cb, LV_EVENT_INVALIDATE_AREA, display);
        // ticks
#if !defined(STM32H743)
        lv_tick_set_cb(&timer_cb);
#endif
    }
#endif

}
#ifdef PROFILE
// send on itm port 1
void flush_cb(const char *buffer) {
    int l = strlen(buffer);
    while(l--) {
        while(ITM->PORT[1U].u32 == 0UL) __NOP();
        ITM->PORT[1U].u8 = (uint8_t)*buffer++;
    }
}
void SetupProfiler(void) {
    clock_setup_us_source();
    lv_profiler_builtin_config_t config;
    lv_profiler_builtin_config_init(&config);
    config.tick_per_sec = 1000000; /* One second is equal to 1000000000 nanoseconds */
    config.tick_get_cb = clock_us;
    config.flush_cb = flush_cb;
    lv_profiler_builtin_init(&config);
}
#endif
extern uint32_t frames;

int CRITICAL main(void)
{
    /* memcpy(&_sitcmram, &_siitcmram, (uint32_t)&_eitcmram - (uint32_t)&_sitcmram);  */
    system_clock_config();

#if defined(SWO)
    // enable swo
    ITM_enable();
    printf("---------------------------------------- LOG START ----------------------------------------------\n");
#endif

#if defined(SEMIHOSTING) && !defined(PLATFORM_SIM) && !defined(SWO)
  	initialise_monitor_handles();
#endif
    delay_init();
    lv_init();
#ifdef PROFILE
    SetupProfiler();
#endif
    lv_setup();
#ifdef PLATFORM_SIM
#if LV_VER == 8
    gtkdrv_init();
#endif
    eeprom_init();                          // initialize the eeprom for data storage
    uart_init(57600);                       // initialize comms
    gui_init();
#endif
    
    controls_init();                        // init adc and buttons 
    power_enable();
    
    lcd_init();                             // attempt to initialize the lcd peripherals
    lcd_backlight(100);
    lcd_start();                            // start lcd init sequence
    crc_init();                             // crc init if HW unit
    eeprom_init();                          // initialize the eeprom for data storage
    clock_init();                           // clouck source (if available)
    
    button_release(BUTTON_ID_POWER, 500);   // ignore inputs for a while
    button_release(BUTTON_ID_DOWN, 500);   // ignore inputs for a while
    button_release(BUTTON_ID_UP, 500);   // ignore inputs for a while
    
#ifndef PLATFORM_SIM
    uart_init(57600);                       // initialize comms
    gui_init();                             // start lvgl and setup screen
#endif
    comm_send_display_settings();  
    comm_send_display_status();  
    comm_send_controller_settings();

#if MONITOR && DEBUG
#endif
    lv_timer_handler();

#if (!defined(DEBUG) || DEBUG == 0) && !defined(PLATFORM_SIM)
#if defined(STM32H743)
    
#else
    wdt_divider_set(WDT_CLK_DIV_32);
    wdt_reload_value_set(1250); // 1s
    wdt_counter_reload();
    wdt_enable();
#endif
#endif
#ifdef LOOP
    uint32_t cntr = 0;
    LV_LOG_INFO("Starting main loop");
    clock_set_wheelspeed_timer(60);
#endif
    while(1) {
#if PLATFORM_SIM && LVGL_VERSION_MAJOR == 9
        SDL_Delay(5);
#endif
        comm_update();
        button_presses();
        gui_update();                                           // update gui data
        auto_lights();
        lv_timer_handler();
        /* uint32_t m = lv_timer_handler(); */
        /* delay_ms(m > CYCLE_DELAY_LIMIT ? CYCLE_DELAY_LIMIT : m); */
#ifdef LOOP
        extern int32_t speed, power_value, battery_current;
        extern uint8_t draw_power_trigger, draw_speed_trigger, draw_battery_voltage_trigger;
        speed += 1;
        power_value += 1;
        battery_current += 1;

        if (speed > 30000) speed = 0;
        if (power_value > 100000) power_value = 0;
        if (battery_current > 20000) battery_current = -10000;


        cntr = cntr + 1;
        if (cntr > 5) 
        {

            draw_power_trigger = 1;
            draw_speed_trigger = 1;
            draw_battery_voltage_trigger = 1;
        }
#endif
#if (!defined(DEBUG) || DEBUG == 0) && !defined(PLATFORM_SIM)
#if defined(STM32H743)
    
#else
        wdt_counter_reload();
#endif
#endif
        if (timer_counter - shutdown_timer > (((uint32_t) settings.shutdown_timer) * 60000)) {
            power_disable();
        }
    }
}

#if !defined(PLATFORM_SIM)
#if defined(STM32H743)
#else
CRITICAL void WWDT_IRQHandler(void) {
    NVIC_SystemReset();
}
#endif
#endif
