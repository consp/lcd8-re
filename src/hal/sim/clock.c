#include <stdio.h>
#include <time.h>
#include "eeprom.h"
#include "clock.h"
#include "gui.h"

// use lvgl for timer
#include "lvgl.h"

extern settings_t settings;
volatile uint32_t timer_counter = 0;

/***
 * LVGL timer functions
 */
uint32_t timer_cb(void) {
    return timer_counter;
}

lv_timer_t *wheelspeed_timer = NULL;
lv_timer_t *tmr_timer = NULL;
void _mstimer_timer(lv_timer_t *timer) {
    timer_counter++;
}
void _wheelspeed_timer(lv_timer_t *timer) {
    gui_increment_trip();
}

uint32_t reset_flag = 0;

void clock_init(void) {
    wheelspeed_timer = lv_timer_create(_wheelspeed_timer, 1000, NULL);
    tmr_timer = lv_timer_create(_mstimer_timer, 1, NULL);
    // pause
    lv_timer_pause(wheelspeed_timer);
}


void clock_get_time(uint32_t *hours, uint32_t *minutes) {
    time_t rt;
    struct tm *tm;

    time(&rt);
    tm = localtime(&rt);
    *hours = tm->tm_hour;
    *minutes = tm->tm_min;
}

void clock_get_all(uint8_t *hours, uint8_t *minutes, uint8_t *sec, uint16_t *year, uint8_t *mon, uint8_t *day) {
    time_t rt;
    struct tm *tm;

    time(&rt);
    tm = localtime(&rt);
    *hours = tm->tm_hour;
    *minutes = tm->tm_min;
    *sec = tm->tm_sec;
    *year = tm->tm_year;
    *mon = tm->tm_mon;
    *day = tm->tm_mday;
}

void clock_set_wheelspeed_timer(uint32_t interval) {
    if (interval > 0) {
        lv_timer_set_period(wheelspeed_timer, interval);
        lv_timer_resume(wheelspeed_timer);
    } else {
        lv_timer_pause(wheelspeed_timer);
    }
}

void clock_set_time(uint32_t hour, uint32_t minute, uint32_t second) {
    // does nothing
}

void clock_set_date(uint32_t year, uint32_t month, uint32_t day, uint32_t week) {
    // does nothing
}

