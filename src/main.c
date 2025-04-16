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
#if  defined(PLATFORM_LCD8) && defined(AT32F415)
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


    controls_init();                        // init adc and buttons 
    power_enable();
    button_release(BUTTON_ID_POWER, 1250);  // ignore inputs for a while
    crc_init();                             // crc init if HW unit
    eeprom_init();                          // initialize the eeprom for data storage
    
    lv_init();
    clock_init();                           // clouck source (if available)
                                            


    lcd_init();                             // attempt to initialize the lcd peripherals
    
    lcd_backlight(100);
    lcd_start();                            // start lcd init sequence

    gui_init();                             // start lvgl and setup screen
    uart_init(57600);                 // initialize comms
        
    comm_send_display_settings();  
    comm_send_display_status();  
    comm_send_controller_settings();
#if MONITOR && DEBUG
    uint32_t x = 0, y = 0;
#endif

    gui_update();                                           // update gui data
    while(1) {
        gui_update();                                           // update gui data
#if MONITOR && DEBUG
        uint32_t m = lv_timer_handler();                                     // draw
        delay_ms(m);
#else
        lv_timer_handler();
#endif
        comm_update();
        button_presses();

#if MONITOR  && DEBUG
        if (timer_counter - x >= 1000) {
            lv_mem_monitor_t mon;
            lv_mem_monitor(&mon);
            char buf[64];
            /* sprintf(buf, "Free: %ld/%ld, %d%% used, %d%% frag, cpu %d%%\n", mon.free_size, mon.total_size, mon.used_pct, mon.frag_pct); */
            /* uart_send(buf, strlen(buf), 0); */
            x = timer_counter;
        }
#endif
    }
}

#if !defined(PLATFORM_SIM)
void WWDT_IRQHandler(void) {
    NVIC_SystemReset();
}
#endif
