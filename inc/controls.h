#ifndef __CONTROLS_H__
#define __CONTROLS_H__

#include "lvgl.h"


#define BUTTON_MEASURE_PERIOD   50 // measure every 100ms
#define BUTTON_ACTIVE_PERIOD    200 // active period 300ms
#define BUTTON_HOLD_PERIOD      1000 // active period 300ms
#define BUTTON_LONG_PRESS_TIME  1000
#define BUTTON_DEBOUNCE_TIME    BUTTON_MEASURE_PERIOD + 10

#define BUTTON_COUNT (BUTTON_ACTIVE_PERIOD / BUTTON_MEASURE_PERIOD)
#define BUTTON_HOLD (BUTTON_HOLD_PERIOD / BUTTON_MEASURE_PERIOD)

typedef struct {
    uint16_t temperature_int;
    uint16_t voltage_mcu;
    uint16_t voltage_battery;
    union {
        uint16_t temperature_ext;
        uint16_t up_button;
    };
    uint16_t nc_button; // can be used as light detector?
    uint16_t down_button; // normally not read
    uint16_t power_button; // normally not read
} adc_data_t;

void controls_init(void);
void button_release(uint8_t id, uint32_t backoff);
uint8_t buttons_pressed(void);

int32_t voltage_ebat(void);
int32_t voltage_mcu(void);

int32_t int_temp(void);
int32_t ext_temp(void);

void power_enable(void);
void power_disable(void);

#define BUTTON_ID_UP        0
#define BUTTON_ID_DOWN      2 
#define BUTTON_ID_POWER     4
#define BUTTON_ID_NC        6

#define BUTTON_RELEASED     0
#define BUTTON_DOWN         1 
#define BUTTON_PRESSED      2
#define BUTTON_LONG_PRESSED 3

#define BUTTON_STATE(x, y)     (y << x)
#define BUTTON_MASK(x)         (0x3 << x)

#endif // __CONTROLS_H__
