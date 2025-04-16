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

uint32_t reset_flag = 0;
void clock_init(void) {
}


void clock_get_time(uint32_t *hours, uint32_t *minutes) {
}

void clock_get_all(uint8_t *hours, uint8_t *minutes, uint8_t *sec, uint16_t *year, uint8_t *mon, uint8_t *day) {
}

void clock_set_wheelspeed_timer(uint32_t interval) {
}

void clock_set_time(uint32_t hour, uint32_t minute, uint32_t second) {
}

void clock_set_date(uint32_t year, uint32_t month, uint32_t day, uint32_t week) {
}

