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

#ifdef MEMORY_DEBUG
#ifndef SIM
uint8_t *debugbuffer = (uint8_t *) 0x20007E00;
uint16_t *debugbuffer16 = (uint16_t *) 0x20007E00;
uint32_t *debugbuffer32 = (uint32_t *) 0x20007E00;
#else
uint8_t *debugbuffer[2048];
uint16_t *debugbuffer16;
uint32_t *debugbuffer32;
#endif
#endif

extern uint32_t timer;

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
    memset(debugbuffer, 0xAA, 2048);
    debugbuffer16 = (uint16_t *) debugbuffer;
    debugbuffer32 = (uint32_t *) debugbuffer;
#else
    for (int i = 0; i < 512; i++)  debugbuffer[i] = 0xAA;
#endif
#endif
#ifndef SIM
    __IO uint32_t index = 0;

    /* nvic_priority_group_config(NVIC_PRIORITY_GROUP_4); */
    /* nvic_irq_enable(TMR4_GLOBAL_IRQn, 0, 0); */
    system_clock_config();

    // enable gpio clocks
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE); // we use all channels
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE); // we use all channels
    crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE); // we use all channels
  
    lcd_init();    // attempt to initialize the lcd, mine responds to ID4 as a ILI9488
#endif

    eeprom_init(); // initialize the eeprom for data storage
    controls_init(); // adc and buttons 

    lcd_backlight(100);
    lcd_start();

    gui_init();

    uint8_t x = 0;
    /*
     * Target is 30 fps, which is overkill
     *
     * most large items (bar, top line etc) take ~4-5ms
     * graph takes 34ms so maybe two frames
     * large text takes ~6ms
     */
    while(1) {
        volatile uint32_t tm = timer_cb();
        adc_ordinary_software_trigger_enable(ADC1, TRUE);       // trigger adc
        gui_update();                                           // update gui data
        lv_timer_handler();                                     // draw
        x++;
    }
}

