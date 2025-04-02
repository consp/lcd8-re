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

#ifndef SIM
#include "at32f415_clock.h"
#include "at32f415_dma.h"
#include "lcd.h"
#endif
#include "delay.h"
#include "eeprom.h"
#include "controls.h"
#include "gui.h"
#include "uart.h"
#include "clock.h"
#include <string.h>
#include <stdio.h>

/** @addtogroup AT32F415_periph_examples
  * @{
  */

/** @addtogroup 415_I2S_spii2s_switch_halfduplex_polling I2S_spii2s_switch_halfduplex_polling
  * @{
  */

#ifndef SIM
__IO uint32_t tx_index = 0, rx_index = 0;
volatile error_status transfer_status1 = ERROR, transfer_status2 = ERROR, transfer_status3 = ERROR;
#endif

extern uint32_t timer;
uint8_t ding[256];
extern adc_data_t adc;
/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
#ifdef MEMORY_DEBUG
#ifdef SIM
    memset(debugbuffer, 0x55, 2048);
    debugbuffer16 = (uint16_t *) debugbuffer;
    debugbuffer32 = (uint32_t *) debugbuffer;
#else
    for (int i = 0; i < 512; i++)  debugbuffer[i] = 0x55;
#endif
#endif
#ifndef SIM
    system_clock_config();

    // enable gpio clocks
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE); // we use all channels
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE); // we use all channels
    crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE); // we use all channels
  
    clock_init(); // clouck source
    lcd_init();    // attempt to initialize the lcd peripherals
#endif

    controls_init(); // adc and buttons 
    // ignore inputs for a while
    button_release(BUTTON_ID_POWER, 2000);
    power_enable();
    eeprom_init(); // initialize the eeprom for data storage
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
    uart_send_display_settings();
#if DEBUG
    uint32_t x = 0, y = 0;
    extern volatile uint32_t timer_counter;
#endif
    while(1) {
        gui_update();                                           // update gui data
        lv_timer_handler();                                     // draw
        uart_update();
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

