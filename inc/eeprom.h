#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <stdint.h>

void eeprom_init(void);
void eeprom_read_settings(void);
void eeprom_write_settings(void);

#pragma pack(push, 1)
typedef struct __attribute__((packed)) {
    uint32_t header;
    // stored data
    uint32_t trip_time;
    uint32_t trip_distance;
    uint32_t total_distance;

    // display setting values
    uint16_t    battery_voltage_min;
    uint16_t    battery_voltage_max;
    uint8_t     graph_duration;
    uint8_t     graph_shift;
    uint8_t     graph_max;
    uint8_t     assist_levels;
    uint8_t     assist_last;
    uint8_t     speed_max;
    uint8_t     speed_redline;
    int16_t     power_min;
    int16_t     power_max;
    uint8_t     battery_voltage_from_controller;
    uint8_t     backlight_level;
    
    // motor configuration
    uint8_t     speed_assist_max;
    uint16_t    wheel_circumfence;
    uint16_t    current_max;
    uint8_t     lights_enabled;
    uint8_t     lights_mode;
    uint16_t    regen_current;
    uint16_t    pas_timeout;
    uint16_t    pas_ramp;

    uint32_t    date;
    uint32_t    time;

    uint8_t     factory_reset;
    uint8_t     crc;
} settings_t;
#pragma pack(pop)


#endif // __EEPROM_H__
