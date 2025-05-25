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

#ifdef SEMIHOSTING
extern void initialise_monitor_handles(void);
#endif

CRITICAL int main(void)
{
    system_clock_config();
#ifdef SEMIHOSTING
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
    
    crc_init();                             // crc init if HW unit
    eeprom_init();                          // initialize the eeprom for data storage
    clock_init();                           // clouck source (if available)
    lcd_init();                             // attempt to initialize the lcd peripherals
    
    button_release(BUTTON_ID_POWER, 500);   // ignore inputs for a while
    button_release(BUTTON_ID_DOWN, 500);   // ignore inputs for a while
    button_release(BUTTON_ID_UP, 500);   // ignore inputs for a while
    
    lcd_backlight(100);
    lcd_start();                            // start lcd init sequence
#ifndef PLATFORM_SIM
    uart_init(57600);                       // initialize comms
    gui_init();                             // start lvgl and setup screen
#endif
    comm_send_display_settings();  
    comm_send_display_status();  
    comm_send_controller_settings();

#if MONITOR && DEBUG
    uint32_t x = 0, y = 0;
    /* uint32_t tmp = 0; */
    /* extern int32_t battery_current; */
    /* extern int32_t battery_voltage; */
    /* extern int32_t speed; */
    /* extern uint8_t draw_power_trigger, draw_speed_trigger; */
#endif
    lv_timer_handler();

    while(1) {
#if PLATFORM_SIM && LVGL_VERSION_MAJOR == 9
        SDL_Delay(5);
#endif
        comm_update();
        button_presses();
        gui_update();                                           // update gui data

        /* if (timer_counter - tmp >= 100) { */
        /*     tmp = timer_counter; */
        /*     battery_voltage = 27000; */
        /*     battery_current+=100; */
        /*     draw_power_trigger = 1; */
        /*     draw_speed_trigger = 1; */
        /*     speed += 250; */
        /*     if (battery_current > 20000) battery_current = -10000; */
        /*     if (speed > 35000) speed = 0; */
        /* } */
#if MONITOR && DEBUG
        uint32_t m = lv_timer_handler();                                     // draw
        delay_ms(m);
#else
        uint32_t m = lv_timer_handler();
        delay_ms(m);
#endif
    }
}

#if !defined(PLATFORM_SIM)
CRITICAL void WWDT_IRQHandler(void) {
    NVIC_SystemReset();
}
#endif
