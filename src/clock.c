#include <stdio.h>
#include "clock.h"
#include "uart.h"



/*
 * ertc is present but no external crystal 
 * resuting in variable clocked ertc which will not be
 * stable enough to be reliable
 *
 * Install a a 
 */
int clock_available(void) {
    return LEXT_INSTALLED;
}

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
}

ertc_time_type time;
void clock_get_time(void) {
    ertc_timestamp_get(&time);
}


