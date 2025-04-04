#ifndef __COMM_H__
#define __COMM_H__

#include <stdint.h>
#include <unistd.h>
#include <string.h>

#define MSG_VOLTAGE                 0x10        // voltage as reported by controller in mV
#define MSG_CURRENT                 0x11        // current as reported by controller in mA
#define MSG_ROTATION                0x12        // rotation period in ms
#define MSG_TEMPERATURE             0x13
#define MSG_CONTROLLER_STATUS       0x14
#define MSG_CONTROLLER_SETTINGS     0x15

#define MSG_DISPLAY_STATUS          0x80        
#define MSG_DISPLAY_SETTINGS        0x81

void comm_send_display_settings(void);
void comm_send_display_status(void);
void comm_send_controller_settings(void);
void comm_update(void);

#pragma pack(push,1)
typedef struct {
    uint32_t    value;              // in mV
} msg_voltage;

typedef struct {
    uint32_t    value;              // in mA
} msg_current;

typedef struct {
    uint16_t    value;              // in ms
    uint8_t     direction;          // 0 == forward, != 0 is backwards
} msg_rotation;

typedef struct {
    int16_t    engine;              // in 0.1c 
    int16_t    controller;          // in 0.1c
} msg_temperature;

typedef struct {
    uint8_t     brake : 1;          // 1 == brake
    uint8_t     mode : 7;         // controller mode
    uint8_t     error;              // error message
} msg_controller_status;

typedef struct {
} msg_controller_settings;          // RFU

typedef struct {
    uint8_t     assist_level;       // Assist level setting
    uint8_t     lights;             // Enable/disable lights
} msg_display_status;

typedef struct {
    uint16_t    max_current;        // in mA
    uint8_t     max_speed;          // in km/h for assist
    uint16_t    wheel_circumfence;  // in mm
    uint8_t     assist_levels;
} msg_display_settings;

typedef struct {
    uint8_t tag;
    uint8_t length;
    uint8_t crc;
    union {
        msg_voltage                 voltage;
        msg_current                 current;
        msg_rotation                rotation;
        msg_temperature             temperature;
        msg_controller_status       status;
    };
} msg_controller;

typedef struct {
    uint8_t tag;
    uint8_t length;
    uint8_t crc;
    union {
        msg_display_settings        settings;
        msg_display_status          status;
    };
} msg_display;
#pragma pack(pop)

#endif // __COMM_H__
