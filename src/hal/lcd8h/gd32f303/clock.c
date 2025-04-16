#include <stdio.h>
#include "eeprom.h"
#include "clock.h"
#include "gui.h"


extern settings_t settings;
volatile uint32_t timer_counter = 0;

/***
 * LVGL timer functions
 */
uint32_t timer_cb(void) {
    return timer_counter;
}

//ertc_time_type time;
//tmr_output_config_type tmr_output_struct;
uint32_t reset_flag = 0;
void clock_init(void) {
#if LEXT_INSTALLED == 1
    crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);
    /* enable battery powered domain access */

    pwc_battery_powered_domain_access(TRUE);
    if (!reset_flag) {
        if(CRM->bpdc_bit.ertcsel != CRM_ERTC_CLOCK_LEXT)
        {
            /* reset battery powered domain */
            crm_battery_powered_domain_reset(TRUE);
            crm_battery_powered_domain_reset(FALSE);

            /* config ertc clock source */
            crm_ertc_clock_select(CRM_ERTC_CLOCK_LEXT);
        }

        /* check lext enabled or not */
        if(crm_flag_get(CRM_LEXT_STABLE_FLAG) == RESET)
        {
            crm_lext_bypass(FALSE);

            crm_clock_source_enable(CRM_CLOCK_SOURCE_LEXT, TRUE);
            while(crm_flag_get(CRM_LEXT_STABLE_FLAG) == RESET);
        }

        crm_ertc_clock_enable(TRUE);

        ertc_reset();

        ertc_wait_update();

        ertc_direct_read_enable(FALSE);

        ertc_divider_set(127, 255); // for 1Hz @ 32768 khz crystal per reference manual
        ertc_hour_mode_set(ERTC_HOUR_MODE_24);

        /* ertc_alarm_mask_set(ERTC_ALA, ERTC_ALARM_MASK_DATE_WEEK); */
        /* ertc_alarm_week_date_select(ERTC_ALA, ERTC_SLECT_DATE); */
        /* ertc_alarm_set(ERTC_ALA, 31, 5, 20, 30, ERTC_AM); */

        /* ertc_interrupt_enable(ERTC_ALA_INT, TRUE); */
        ertc_alarm_enable(ERTC_ALA, FALSE);
        ertc_flag_clear(ERTC_ALAF_FLAG);

            // cold boot, set time
        if (settings.time == 0 && settings.date == 0) {
            clock_set_time(12, 0, 0);
            clock_set_date(25, 5, 14, 1);
            settings.time = ERTC->time;
            settings.date = ERTC->date;
        } else {
            ERTC->time = settings.time;
            ERTC->date = settings.date;
        }

        ertc_bpr_data_write(ERTC_DT1, 0x1234);

        ertc_timestamp_get(&time);

    } else {
        // hot boot, recover time
        settings.time = ERTC->time;
        settings.date = ERTC->date;
    }
#endif
/*
    // general ms timer via general timer and wheelspeed timer 
    crm_periph_clock_enable(CRM_TMR4_PERIPH_CLOCK, TRUE);
    tmr_base_init(TMR4, 10000-1, TIMER_FREQ(1000000)-1); // 1khz
    tmr_clock_source_div_set(TMR4, TMR_CLOCK_DIV1);
    tmr_cnt_dir_set(TMR4, TMR_COUNT_UP);
    tmr_interrupt_enable(TMR4, TMR_OVF_INT, TRUE); // trap on overflow

    tmr_output_enable(TMR4, FALSE);
    tmr_counter_enable(TMR4, TRUE);

    crm_periph_clock_enable(CRM_TMR5_PERIPH_CLOCK, TRUE);
    tmr_base_init(TMR5, 0, TIMER_FREQ(10000)-1); // 1khz
    tmr_clock_source_div_set(TMR5, TMR_CLOCK_DIV1);
    tmr_cnt_dir_set(TMR5, TMR_COUNT_UP);
    tmr_interrupt_enable(TMR5, TMR_OVF_INT, TRUE); // trap on overflow
    
    tmr_32_bit_function_enable(TMR5, 1);

    tmr_output_enable(TMR5, FALSE);
    tmr_counter_enable(TMR5, TRUE);

    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_irq_enable(TMR4_GLOBAL_IRQn, 0, 0);

    nvic_irq_enable(TMR5_GLOBAL_IRQn, 0, 0);
*/
}

void clock_get_time(uint32_t *hours, uint32_t *minutes) {
#if LEXT_INSTALLED
    ertc_calendar_get(&time);
    *hours = time.hour;
    *minutes = time.min;
#else 
    *hours = 0;
    *minutes = 0;
#endif
}


void clock_get_all(uint8_t *hours, uint8_t *minutes, uint8_t *sec, uint16_t *year, uint8_t *mon, uint8_t *day) {
#if LEXT_INSTALLED
    ertc_calendar_get(&time);
    *hours = time.hour;
    *minutes = time.min;
    *sec = time.sec;
    *year = 2000 + time.year;
    *mon = time.month;
    *day = time.day;
#endif
}

void TMR4_GLOBAL_IRQHandler(void)
{
    /*
    if(tmr_interrupt_flag_get(TMR4, TMR_OVF_FLAG) != RESET)
    {
        timer_counter++;
        tmr_flag_clear(TMR4, TMR_OVF_FLAG);
    }
    */
}

void TMR5_GLOBAL_IRQHandler(void)
{
    /*
    if(tmr_interrupt_flag_get(TMR5, TMR_OVF_FLAG) != RESET)
    {
        tmr_flag_clear(TMR5, TMR_OVF_FLAG);
        gui_increment_trip();
    }
    */
}

void clock_set_wheelspeed_timer(uint32_t interval) {
    /*
    if (interval == 0) TMR5->pr = 0;
    else TMR5->pr = (interval * 100)-1;
    */
}

void clock_set_time(uint32_t hour, uint32_t minute, uint32_t second) {
#if LEXT_INSTALLED
    ertc_time_set(hour, minute, second, ERTC_24H);
#endif
}

void clock_set_date(uint32_t year, uint32_t month, uint32_t day, uint32_t week) {
#if LEXT_INSTALLED
    ertc_date_set(year, month, day, week);
#endif
}

