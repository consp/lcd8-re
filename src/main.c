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
void system_clock_config(void);
/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
    system_clock_config();

    delay_init();
    lv_init();
#ifdef PLATFORM_SIM
    gtkdrv_init();
    gui_init();
#endif

    controls_init();                        // init adc and buttons 
    power_enable();
    button_release(BUTTON_ID_POWER, 1250);  // ignore inputs for a while
                                            //
    eeprom_init();                          // initialize the eeprom for data storage
    clock_init();                           // clouck source (if available)
                                            //
                                            //
    crc_init();                             // crc init if HW unit
    lcd_init();                             // attempt to initialize the lcd peripherals
    uart_init(57600);                       // initialize comms
    
    
    lcd_backlight(100);
    lcd_start();                            // start lcd init sequence
#ifndef PLATFORM_SIM
    gui_init();                             // start lvgl and setup screen
#endif
        
    comm_send_display_settings();  
    comm_send_display_status();  
    comm_send_controller_settings();
/* #if MONITOR && DEBUG */
    uint32_t x = 0, y = 0;
/* #endif */

    gui_update();                                           // update gui data
    while(1) {
        gui_update();                                           // update gui data
#if MONITOR && DEBUG
        uint32_t m = lv_timer_handler();                                     // draw
        /* delay_ms(m); */
#else
        lv_timer_handler();
#endif
        comm_update();
        button_presses();

/* #if MONITOR & LVGL_LOG */
        if (timer_counter - x >= 1000) {
            lv_mem_monitor_t mon;
            lv_mem_monitor(&mon);
            char buf[64];
            LV_LOG_INFO("Free: %ld/%ld, %d%% used, %d%% frag, cpu %d%%\n", mon.free_size, mon.total_size, mon.used_pct, mon.frag_pct);
            x = timer_counter;
        }
/* #endif */
    }
}

#if !defined(PLATFORM_SIM)
void WWDT_IRQHandler(void) {
    NVIC_SystemReset();
}
#endif
