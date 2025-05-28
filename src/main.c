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

#include <string.h>
#include <stdio.h>
#include "config.h"
#ifndef PLATFORM_SIM
#if defined(AT32F415)
#include "at32f415.h"
#include "at32f415_clock.h"
#elif defined(AT32F435)
#include "at32f435_437.h"
#endif
#include <cmsis/core/core_cm4.h>
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

void system_clock_config(void);
extern lv_display_t *display;
/**
  * @brief  main function.
  * @param  none
  * @retval none
  */

#if defined(SEMIHOSTING) && !defined(PLATFORM_SIM)
extern void initialise_monitor_handles(void);
#endif

CRITICAL int main(void)
{
    system_clock_config();
#if defined(SEMIHOSTING) && !defined(PLATFORM_SIM)
  	initialise_monitor_handles();
    printf("Semihosting enabled");
#endif

    delay_init();
    lv_init();
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
    wdt_divider_set(WDT_CLK_DIV_32);
    wdt_reload_value_set(1250); // 1s
    wdt_counter_reload();
    wdt_enable();
#endif
    while(1) {
#if PLATFORM_SIM && LVGL_VERSION_MAJOR == 9
        SDL_Delay(5);
#endif
        comm_update();
        button_presses();
        gui_update();                                           // update gui data
        auto_lights();
        uint32_t m = lv_timer_handler();
        delay_ms(m > CYCLE_DELAY_LIMIT ? CYCLE_DELAY_LIMIT : m);
#if (!defined(DEBUG) || DEBUG == 0) && !defined(PLATFORM_SIM)
        wdt_counter_reload();
#endif
    }
}

#if !defined(PLATFORM_SIM)
CRITICAL void WWDT_IRQHandler(void) {
    NVIC_SystemReset();
}
#endif
