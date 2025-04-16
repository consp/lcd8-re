#include "config.h"
#include "controls.h"

// common data

adc_data_t adc;
uint8_t power_button_state = BUTTON_RELEASED;
uint8_t up_button_state = BUTTON_RELEASED;
uint8_t down_button_state = BUTTON_RELEASED;
uint8_t nc_button_state = BUTTON_RELEASED;

extern volatile uint32_t timer_counter;

uint32_t power_button_start = 0;
uint32_t up_button_start = 0;
uint32_t down_button_start = 0;
uint32_t nc_button_start = 0;
uint32_t button_backoff = 0, button_backoff_start = 0;

void button_release(uint8_t id, uint32_t backoff) {
    button_backoff_start = timer_counter;
    button_backoff = backoff;
    switch(id) {
        case BUTTON_ID_UP:
            up_button_state = BUTTON_RELEASED;
            up_button_start = 0;
            break;
        case BUTTON_ID_DOWN:
            down_button_state = BUTTON_RELEASED;
            down_button_start = 0;
            break;
        case BUTTON_ID_NC:
            nc_button_state = BUTTON_RELEASED;
            nc_button_start = 0;
            break;
        case BUTTON_ID_POWER:
            power_button_state = BUTTON_RELEASED;
            power_button_start = 0;
            break;
    }
}


uint8_t buttons_pressed(void) {
    uint8_t bt = (down_button_state << BUTTON_ID_DOWN) | (up_button_state << BUTTON_ID_UP) | (nc_button_state << BUTTON_ID_NC) | (power_button_state << BUTTON_ID_POWER);
    return bt;
}
