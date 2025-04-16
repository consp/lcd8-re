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
/* uint32_t cnt = 0; */
/* uint16_t dmadata[64] = { 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000}; */
/* void DMA2_Channel1_IRQHandler(void) */
/* { */
/*     DMA2->clr = DMA2_FDT1_FLAG; */
/*     cnt++; */
/*  */
/*     DMA2_CHANNEL1->maddr = (uint32_t) dmadata; */
/*     DMA2_CHANNEL1->dtcnt = 64; */
/*     DMA2_CHANNEL1->ctrl_bit.chen = 1; */
/*     TMR2->ctrl1_bit.tmren = 1; */
/* } */
int main(void)
{
//    system_clock_config();

    // set all gpio's to input, just to be sure
    /* crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE); */
    /* crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE); */
    /* crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE); */
    /* #<{(| GPIOA->cfglr = 0x44444444; |)}># */
    /* GPIOA->cfghr = 0x44444444; */
    /* GPIOA->odt = 0; */
    /* GPIOA->cfglr = 0x44444444; */
    /* GPIOB->cfghr = 0x44444444; */
    /* GPIOB->odt = 0; */
    /* GPIOC->cfglr = 0x44444444; */
    /* GPIOC->cfghr = 0x44444444; */
    /* GPIOC->odt = 0; */
    /*  */
    /* GPIOC->cfglr = 0x33000000; */
    /* GPIOC->cfghr = 0x00000000; */
    /* crm_periph_clock_enable(CRM_DMA2_PERIPH_CLOCK, TRUE); */
    /* crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE); */
    /* crm_periph_clock_enable(CRM_TMR2_PERIPH_CLOCK, TRUE); */
    /*  */
    /*     delay_ms(1000); */
    /* tmr_base_init(TMR2, 99, 0); */
    /* tmr_clock_source_div_set(TMR2, TMR_CLOCK_DIV4); */
    /* tmr_cnt_dir_set(TMR2, TMR_COUNT_UP); */
    /*  */
    /* #<{(| enable tmr2 overflow dma request |)}># */
    /* tmr_dma_request_enable(TMR2, TMR_OVERFLOW_DMA_REQUEST, TRUE); */
    /*  */
    /* tmr_dma_request_enable(TMR2, TMR_OVERFLOW_DMA_REQUEST, TRUE); */
    /* static dma_init_type dma_init_struct = {0}; */
    /* dma_reset(DMA2_CHANNEL1); */
    /* dma_init_struct.buffer_size = 64; */
    /* dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL; */
    /* dma_init_struct.memory_base_addr = (uint32_t) dmadata; */
    /* dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD; */
    /* dma_init_struct.memory_inc_enable = TRUE; */
    /* dma_init_struct.peripheral_base_addr = (uint32_t)&GPIOC->odt; */
    /* dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD; */
    /* dma_init_struct.peripheral_inc_enable = FALSE; */
    /* dma_init_struct.priority = DMA_PRIORITY_HIGH; */
    /* dma_init_struct.loop_mode_enable = FALSE; */
    /* dma_init(DMA2_CHANNEL1, &dma_init_struct); */
    /* dma_interrupt_enable(DMA2_CHANNEL1, DMA_FDT_INT, TRUE); */
    /* nvic_priority_group_config(NVIC_PRIORITY_GROUP_4); */
    /* nvic_irq_enable(DMA2_Channel1_IRQn, 0, 0); */
    /* dma_flexible_config(DMA2, FLEX_CHANNEL1, DMA_FLEXIBLE_TMR2_OVERFLOW); */
    /* dma_channel_enable(DMA2_CHANNEL1, TRUE); */
    /* tmr_counter_enable(TMR2, TRUE); */
    /* while(1) { */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /*     GPIOC->odt = 0x0000; */
    /*     GPIOC->odt = 0xFFFF; */
    /* } */

    controls_init();                        // init adc and buttons 
    power_enable();
    button_release(BUTTON_ID_POWER, 1250);  // ignore inputs for a while
    crc_init();                             // crc init if HW unit
    eeprom_init();                          // initialize the eeprom for data storage
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
            sprintf(buf, "Free: %ld/%ld, %d%% used, %d%% frag, cpu %d%%\n", mon.free_size, mon.total_size, mon.used_pct, mon.frag_pct);
            uart_send(buf, strlen(buf), 0);
            x = timer_counter;
        }
#endif
    }
}

#if PLATFORM != SIM
void WWDT_IRQHandler(void) {
    NVIC_SystemReset();
}
#endif
