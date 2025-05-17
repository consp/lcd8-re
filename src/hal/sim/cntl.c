#include "controls.h"
#include "config.h"
#include "lvgl.h"
#include "gui.h"
#if LVGL_VERSION_MAJOR == 8
#include "gtkdrv.h"
#endif

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

lv_timer_t *lp_timer = NULL;
int32_t ext_temp_store = 0; // store temperature adc value in case button is pressed

/**
 * static functions
 */

/**
 * public functions
 */
uint32_t lpcnt = 0;

static void cb_handler(lv_event_t * e) {
#if LVGL_VERSION_MAJOR == 8
    lv_indev_t *i = lv_indev_get_act();
    uint32_t key = i->proc.types.keypad.last_key;
    uint32_t state = i->proc.types.keypad.last_state;

    // state is always pressed, lvgl 8.x is crap with keys
    // you can only capture release of "enter"
    switch (key) {
        case 113:
            up_button_state = BUTTON_PRESSED;
            break;
        case 119:
            up_button_state = BUTTON_LONG_PRESSED;
            lpcnt = timer_counter;
            break;
        case 97: 
            power_button_state = BUTTON_PRESSED;
            break;
        case 115:
            power_button_state = BUTTON_LONG_PRESSED;
            lpcnt = timer_counter;
            break;
        case 122:
            down_button_state = BUTTON_PRESSED;
            break;
        case 120:
            down_button_state = BUTTON_LONG_PRESSED;
            lpcnt = timer_counter;
            break;
        case 100:
            up_button_state = BUTTON_LONG_PRESSED;
            down_button_state = BUTTON_LONG_PRESSED;
            lpcnt = timer_counter;
            break;
    }
#endif
}

static void _lp_timer(lv_timer_t *t) {
    if (timer_counter - lpcnt > 1000 && lpcnt != 0) {
        up_button_state = BUTTON_RELEASED;
        down_button_state = BUTTON_RELEASED;
        power_button_state = BUTTON_RELEASED;
        lpcnt = 0;
    }
}

#if LVGL_VERSION_MAJOR == 8
lv_indev_drv_t indev_drv_kb;
#endif
lv_indev_t *indev = NULL;
lv_group_t * g = NULL;

void controls_init(void) {
#if LVGL_VERSION_MAJOR == 8
    lv_indev_drv_init(&indev_drv_kb);
    indev_drv_kb.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv_kb.read_cb = gtkdrv_keyboard_read_cb;
    indev = lv_indev_drv_register(&indev_drv_kb);

    // register keypresses
    g = lv_group_create();
    lv_group_set_default(g);
    /* lv_label_t *tmplabel = lv_label_create(lv_screen_active()); */
    /* lv_obj_set_pos(tmplabel, 0, 0); */
    /* lv_obj_set_size(tmplabel, 1, 1); */
    lv_obj_add_event_cb(lv_screen_active(), cb_handler, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(lv_screen_active(), cb_handler, LV_EVENT_RELEASED, NULL);
    lv_group_add_obj(lv_group_get_default(), lv_screen_active());
    /* lv_group_focus_obj(lv_screen_active()); */
    lv_indev_set_group(indev, lv_group_get_default());

    lv_group_focus_obj(lv_screen_active());
    LV_LOG_INFO("%08x, %08x", lv_group_get_default(), indev);

    
    lp_timer = lv_timer_create(_lp_timer, 100, NULL);
#endif
}

void power_enable(void) {  }
void power_disable(void) { 
    abort();
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

