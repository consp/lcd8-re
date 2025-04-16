#include "controls.h"
#include "config.h"

extern adc_data_t adc;
extern volatile uint32_t timer_counter;

extern uint8_t power_button_state ;
extern uint8_t up_button_state ;
extern uint8_t down_button_state ;
extern uint8_t nc_button_state ;

extern uint32_t power_button_start;
extern uint32_t up_button_start;
extern uint32_t down_button_start;
extern uint32_t nc_button_start;
extern uint32_t button_backoff, button_backoff_start;

int32_t ext_temp_store = 0; // store temperature adc value in case button is pressed

/**
 * static functions
 */

/**
 * public functions
 */

void controls_init(void) {
}

void power_enable(void) {  }
void power_disable(void) { 
}


static inline int power_button_measure(void) {
    return 0;
}

static inline int down_button_measure(void) {
    return 0;
}

static inline int up_button_measure(void) {
    return 0;
}

static inline int nc_button_measure(void) {
    return 0;
}

int32_t int_temp(void) {
    return 25 << 8;
}

int32_t ext_temp(void) {
    return 25 << 8;
}

static inline void measure_buttons(void) {
    // button press addition
    if (timer_counter < button_backoff + button_backoff_start) return;
    if (up_button_measure()) {
        if (up_button_state == BUTTON_RELEASED) {
            up_button_start = timer_counter;
            up_button_state = BUTTON_DOWN;
        } else if (up_button_state == BUTTON_DOWN && timer_counter - up_button_start > BUTTON_LONG_PRESS_TIME) {
            up_button_state = BUTTON_LONG_PRESSED;
        }
    } else {
        if (up_button_state == BUTTON_DOWN && timer_counter - up_button_start > BUTTON_DEBOUNCE_TIME) {
            up_button_state = up_button_state == BUTTON_DOWN ? BUTTON_PRESSED : BUTTON_RELEASED;
            up_button_start = 0;
        } else if (up_button_state != BUTTON_PRESSED) {
            up_button_state = BUTTON_RELEASED;
            up_button_start = 0;
        }
    }

    if (down_button_measure()) {
        if (down_button_state == BUTTON_RELEASED) {
            down_button_start = timer_counter;
            down_button_state = BUTTON_DOWN;
        } else if (down_button_state == BUTTON_DOWN && timer_counter - down_button_start > BUTTON_LONG_PRESS_TIME) {
            down_button_state = BUTTON_LONG_PRESSED;
        }
    } else {
        if (down_button_state == BUTTON_DOWN && timer_counter - down_button_start > BUTTON_DEBOUNCE_TIME) {
            down_button_state = down_button_state == BUTTON_DOWN ? BUTTON_PRESSED : BUTTON_RELEASED;
            down_button_start = 0;
        } else if (down_button_state != BUTTON_PRESSED) {
            down_button_state = BUTTON_RELEASED;
            down_button_start = 0;
        }
    }

    if (power_button_measure()) {
        if (power_button_state == BUTTON_RELEASED) {
            power_button_start = timer_counter;
            power_button_state = BUTTON_DOWN;
        } else if (power_button_state == BUTTON_DOWN && timer_counter - power_button_start > BUTTON_LONG_PRESS_TIME) {
            power_button_state = BUTTON_LONG_PRESSED;
        }
    } else {
        if (power_button_state == BUTTON_DOWN && timer_counter - power_button_start > BUTTON_DEBOUNCE_TIME) {
            power_button_state = power_button_state == BUTTON_DOWN ? BUTTON_PRESSED : BUTTON_RELEASED;
            power_button_start = 0;
        } else if (power_button_state != BUTTON_PRESSED) {
            power_button_state = BUTTON_RELEASED;
            power_button_start = 0;
        }
    }
    if (nc_button_measure()) {
        if (nc_button_state == BUTTON_RELEASED) {
            nc_button_start = timer_counter;
            nc_button_state = BUTTON_DOWN;
        } else if (timer_counter - nc_button_start > BUTTON_LONG_PRESS_TIME) {
            nc_button_state = BUTTON_LONG_PRESSED;
        }
    } else {
        if (nc_button_state == BUTTON_DOWN && timer_counter - nc_button_start > BUTTON_DEBOUNCE_TIME) {
            nc_button_state = nc_button_state == BUTTON_DOWN ? BUTTON_PRESSED : BUTTON_RELEASED;
            nc_button_start = 0;
        } else if (nc_button_state != BUTTON_PRESSED) {
            nc_button_state = BUTTON_RELEASED;
            nc_button_start = 0;
        }
    }
}


int32_t voltage_mcu(void) {
    return 0;
}

int32_t voltage_ebat(void) { // in mv
    return 24000; 
}

