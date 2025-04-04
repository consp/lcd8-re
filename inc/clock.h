#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "config.h"
#include "at32f415_ertc.h"

extern volatile uint32_t timer_counter;
/*
 * The clock must implement a ms timer and a wheelspeed timer
 * The wheelspeed timer should be a ms timer as well with variable period
 */

/*
 * @brief Called on start to initialize timers
 */
void clock_init(void);

/*
 * @brief Get the current time (if RTC available)
 */
void clock_get_time(void);

/*
 * LVGL Callback funtion for ms timer
 */
uint32_t timer_cb(void);

void clock_set_wheelspeed_timer(uint32_t interval);
#endif // __CLOCK_H__
