/*
 * Read/write to eeprom via i2c bitbang
 */
#include "delay.h"
#include "config.h"
#include "eeprom.h"
#include "crc.h"

settings_t settings;


void eeprom_init(void) {
    eeprom_read_settings();
}

void eeprom_write_defaults(void) {
    settings.power_max = POWER_MAX;
    settings.power_min = POWER_MIN;
    settings.speed_max = SPEED_MAX;
    settings.current_max = 10000;
    settings.battery_voltage_min = BATTERY_MIN;
    settings.battery_voltage_max = BATTERY_MAX;
    settings.graph_duration = GRAPH_DURATION;
    settings.graph_shift = GRAPH_SHIFT;
    settings.graph_max = GRAPH_MAX;
    settings.assist_levels = ASSIST_LEVELS;
    settings.assist_last = ASSIST_DEFAULT;
    settings.wheel_circumfence = WHEEL_CIRCUMFENCE;
    settings.speed_redline = SPEED_MAX;
    settings.speed_assist_max = SPEED_MAX;
    settings.lights_enabled = 0;
    settings.lights_mode = 0;
    settings.battery_voltage_from_controller = 0;
    settings.regen_current = REGEN_CURRENT;
    settings.pas_timeout = PAS_TIMEOUT;
    settings.pas_ramp = PAS_RAMP;

    settings.trip_time = 0;
    settings.trip_distance = 0;
    settings.total_distance = 0;
    settings.backlight_level = 100;
#if LEXT_INSTALLED
    // save date and time in case of brownout/powerdown
    settings.time = 0;
    settings.date = 0;
#endif
    settings.factory_reset = 0xAA; // mark as factory reset
    eeprom_write_settings();
}

void eeprom_read_settings(void) {
    eeprom_write_defaults();
}

void eeprom_write_settings(void) {
}

