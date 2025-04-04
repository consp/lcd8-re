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
#ifdef PLATFORM_LCD8
#include "at32f415_clock.h"
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

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
    system_clock_config();
  
    clock_init(); // clouck source
    lcd_init();    // attempt to initialize the lcd peripherals

    controls_init();                        // init adc and buttons 
    
    button_release(BUTTON_ID_POWER, 500);   // ignore inputs for a while
    power_enable();                         // keep power on
    crc_init();                             // crc init if HW unit
    eeprom_init();                          // initialize the eeprom for data storage
    uart_init(BAUD(57600));      

    lcd_start(); // start lcd init sequence

    gui_init(); // start lvgl
    /*
     * Target is 30 fps, which is overkill
     *
     * most large items (bar, top line etc) take ~4-5ms
     * graph takes 34ms so maybe two frames
     * large text takes ~6ms
     */
    gui_update();
    comm_send_display_settings(); // 
#ifdef DEBUG
    uint32_t x = 0, y = 0;
#endif
    while(1) {
        gui_update();                                           // update gui data
        lv_timer_handler();                                     // draw
        comm_update();
        button_presses();
#ifdef DEBUG
        if (timer_counter - x >= 1000) {
            lv_mem_monitor_t mon;
            lv_mem_monitor(&mon);
            LV_LOG_INFO("Free: %ld/%ld, %d%% used, %d%% frag", mon.free_size, mon.total_size, mon.used_pct, mon.frag_pct);
            x = timer_counter;
        }
        if (timer_counter - y >= 250) {
            extern uint8_t power_button_state, up_button_state, down_button_state;
            LV_LOG_INFO("%d %d %d", power_button_state, up_button_state, down_button_state);
            y = timer_counter;
        }
#endif
    }
}

