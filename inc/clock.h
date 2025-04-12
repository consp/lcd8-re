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
void clock_get_time(uint32_t *hours, uint32_t *minutes);

/*
 * LVGL Callback funtion for ms timer
 */
uint32_t timer_cb(void);

void clock_set_wheelspeed_timer(uint32_t interval);

void clock_set_time(uint32_t hour, uint32_t minute, uint32_t second);
void clock_set_date(uint32_t year, uint32_t month, uint32_t day, uint32_t week);
void clock_get_all(uint8_t *hours, uint8_t *minutes, uint8_t *sec, uint16_t *year, uint8_t *mon, uint8_t *day);

#endif // __CLOCK_H__
