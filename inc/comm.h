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

/*
 * VESC values
 */

#define COMM_VESC_FAST_INTERVAL                     100
#define COMM_VESC_SLOW_INTERVAL                     1000

#define COMM_VESC_CMD_GET_VALUES                    4
#define COMM_VESC_CMD_GET_VALUES_SETUP              47
#define COMM_VESC_CMD_GET_VALUES_SELECTIVE          50
#define COMM_VESC_CMD_GET_VALUES_SETUP_SELECTIVE    51

// maskss for get values
#define VESC_GET_FET_TEMP                           (1 << 0)
#define VESC_GET_MOTOR_TEMP                         (1 << 1)
#define VESC_GET_AVG_MOTOR_CURRENT                  (1 << 2)
#define VESC_GET_AVG_INPUT_CURRENT                  (1 << 3)
#define VESC_GET_AVG_ID                             (1 << 4)
#define VESC_GET_AVG_IQ                             (1 << 5)
#define VESC_GET_DUTY_CYCLE                         (1 << 6)
#define VESC_GET_RPM                                (1 << 7)
#define VESC_GET_INPUT_VOLTAGE                      (1 << 8)
#define VESC_GET_AMP_HOURS                          (1 << 9)
#define VESC_GET_AMP_HOURS_CHARGED                  (1 << 10)
#define VESC_GET_WATT_HOURS                         (1 << 11)
#define VESC_GET_WATT_HOURS_CHARGED                 (1 << 12)
#define VESC_GET_TACH                               (1 << 13)
#define VESC_GET_TACH_ABS                           (1 << 14)
#define VESC_GET_FAULT                              (1 << 15)
#define VESC_GET_PID_POS                            (1 << 16)
#define VESC_GET_CONTROLLER_ID                      (1 << 17)
#define VESC_GET_ALL_FET_TEMP                       (1 << 18)
#define VESC_GET_AVG_VD                             (1 << 19)
#define VESC_GET_AVG_VQ                             (1 << 20)
#define VESC_GET_STATUS                             (1 << 21)

typedef struct vesc_packet_1_t {
    uint8_t type;
    uint32_t length;
    uint8_t *data;
    uint16_t crc;
} vesc_packet_1;

/*
 * Custom EBICS values
 */

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
    uint16_t    regen_current;
    uint16_t    pas_timeout;
    uint16_t    pas_ramp;
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

#if UART_COMM == UART_COMM_VESC
int comm_vesc_packet_resc(uint8_t *data, uint32_t length, vesc_packet_1 *pack);
int comm_vesc_packet_send(uint8_t *data, uint32_t length);
int comm_vesc_packet_send_get_data(uint32_t mask);
#endif

#endif // __COMM_H__
