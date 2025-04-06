#include <stdio.h>
#include "clock.h"
#include "gui.h"

volatile uint32_t timer_counter = 0;

/***
 * LVGL timer functions
 */
uint32_t timer_cb(void) {
    return timer_counter;
}

tmr_output_config_type tmr_output_struct;

void clock_init(void) {
#if LEXT_INSTALLED == 1
    crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);

    pwc_battery_powered_domain_access(TRUE);
    ertc_write_protect_disable();
    crm_battery_powered_domain_reset(TRUE);
    crm_battery_powered_domain_reset(FALSE);


    crm_clock_source_enable(CRM_CLOCK_SOURCE_LEXT, TRUE);

    while(crm_flag_get(CRM_LEXT_STABLE_FLAG) == RESET)
    {
    }

    crm_ertc_clock_select(CRM_ERTC_CLOCK_LEXT);
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

    ertc_date_set(25, 3, 31, 14);
    ertc_time_set(12, 0, 0, ERTC_24H);

    ertc_bpr_data_write(ERTC_DT1, 0x1234);
#endif

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

}

ertc_time_type time;
void clock_get_time(void) {
    ertc_timestamp_get(&time);
}

void TMR4_GLOBAL_IRQHandler(void)
{
    if(tmr_interrupt_flag_get(TMR4, TMR_OVF_FLAG) != RESET)
    {
        timer_counter++;
        tmr_flag_clear(TMR4, TMR_OVF_FLAG);
    }
}

void TMR5_GLOBAL_IRQHandler(void)
{
    if(tmr_interrupt_flag_get(TMR5, TMR_OVF_FLAG) != RESET)
    {
        tmr_flag_clear(TMR5, TMR_OVF_FLAG);
        gui_increment_trip();
    }
}

void clock_set_wheelspeed_timer(uint32_t interval) {
    if (interval == 0) TMR5->pr = 0;
    else TMR5->pr = (interval * 100)-1;
}
