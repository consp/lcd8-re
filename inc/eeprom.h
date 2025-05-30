#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <stdint.h>

void eeprom_init(void);
void eeprom_read_settings(void);
void eeprom_write_settings(void);
void eeprom_factory_reset(void);

typedef enum graph_field_et {
    GRAPH_FIELD_SHIFT = 0,
    GRAPH_FIELD_RUN = 1,
    GRAPH_FIELD_SPEED_POWER_AVG = 2,
    GRAPH_FIELD_SPEED_AVG = 3
} graph_field;

typedef enum {
    ERROR_UART_TRANSMIT =   0x00000001,
    ERROR_UART_RECEIVE =    0x00000002,
    ERROR_EEPROM_READ =     0x00000004,
    ERROR_EEPROM_WRITE =    0x00000008
} error_state;

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
    uint8_t     graph_field;
    uint8_t     graph_max;
    uint8_t     assist_levels;
    uint8_t     assist_last;
    uint8_t     speed_max;
    uint8_t     speed_redline;
    int16_t     power_min;
    int16_t     power_redline;
    int16_t     power_max;
    uint8_t     battery_voltage_from_controller;
    int8_t      backlight_level;
    int8_t      backlight_sensitivity;
    
    // motor configuration
    uint8_t     speed_assist_max;
    uint16_t    wheel_circumfence;
    uint16_t    current_max;
    uint8_t     lights_enabled;
    uint8_t     lights_mode;
    uint16_t    regen_current;
    uint16_t    pas_timeout;
    uint16_t    pas_ramp;
    int16_t     controller_power_min;
    int16_t     controller_power_max;

    uint32_t    date;
    uint32_t    time;
    uint8_t     shutdown_timer;

    uint16_t    light_sensitivity;
    uint32_t    notification_timeout;

    uint8_t     factory_reset;
    uint8_t     crc;
} settings_t;
#pragma pack(pop)


#endif // __EEPROM_H__
