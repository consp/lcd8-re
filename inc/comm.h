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
#define COMM_VESC_LIGHTS_ID                         0x89

#define COMM_VESC_FAST_INTERVAL                     100
#define COMM_VESC_SLOW_INTERVAL                     1000
#define COMM_VESC_VERY_SLOW_INTERVAL                5000

#define COMM_VESC_CMD_FORWARD_CAN                   34
#define COMM_VESC_CMD_GET_VALUES                    4
#define COMM_VESC_CMD_GET_VALUES_SETUP              47
#define COMM_VESC_CMD_GET_VALUES_SELECTIVE          50
#define COMM_VESC_CMD_GET_VALUES_SETUP_SELECTIVE    51
#define COMM_VESC_CMD_SHUTDOWN                      156
#define COMM_VESC_CMD_MCONF_GET                     91
#define COMM_VESC_CMD_MCONF_SET                     48

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

#define VESC_GET_SETUP_FET_TEMP                     (1 << 0)
#define VESC_GET_SETUP_MOTOR_TEMP                   (1 << 1)
#define VESC_GET_SETUP_CURRENT_TOT                  (1 << 2)
#define VESC_GET_SETUP_CURRENT_IN_TOT               (1 << 3)
#define VESC_GET_SETUP_DUTY_CYCLE                   (1 << 4)
#define VESC_GET_SETUP_RPM                          (1 << 5)
#define VESC_GET_SETUP_SPEED                        (1 << 6)
#define VESC_GET_SETUP_INPUT_VOLTAGE                (1 << 7)
#define VESC_GET_SETUP_BATTERY_LEVEL                (1 << 8)
#define VESC_GET_SETUP_AH_TOT                       (1 << 9)
#define VESC_GET_SETUP_AH_CHARGE_TOT                (1 << 10)
#define VESC_GET_SETUP_WH_TOT                       (1 << 11)
#define VESC_GET_SETUP_WH_CHARGE_TOT                (1 << 12)
#define VESC_GET_SETUP_DISTANCE                     (1 << 13)
#define VESC_GET_SETUP_DISTANCE_ABS                 (1 << 14)
#define VESC_GET_SETUP_PID_POS                      (1 << 15)
#define VESC_GET_SETUP_FAULT                        (1 << 16)
#define VESC_GET_SETUP_VESC_ID                      (1 << 17)
#define VESC_GET_SETUP_NUM_VESC                     (1 << 18)
#define VESC_GET_SETUP_WH_LEFT                      (1 << 19)
#define VESC_GET_SETUP_ODO                          (1 << 20)
#define VESC_GET_SETUP_SYS_TIME                     (1 << 21)

#define VESC_GET_STATS_SPEED_AVG                    (1 << 0)
#define VESC_GET_STATS_SPEED_MAX                    (1 << 1)
#define VESC_GET_STATS_POWER_AVG                    (1 << 2)
#define VESC_GET_STATS_POWER_MAX                    (1 << 3)
#define VESC_GET_STATS_CURRENT_AVG                  (1 << 4)
#define VESC_GET_STATS_CURRENT_MAX                  (1 << 5)
#define VESC_GET_STATS_TEMP_MOSFET_AVG              (1 << 6)
#define VESC_GET_STATS_TEMP_MOSFET_MAX              (1 << 7)
#define VESC_GET_STATS_TEMP_MOTOR_AVG               (1 << 8)
#define VESC_GET_STATS_TEMP_MOTOR_MAX               (1 << 9)


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
int comm_vesc_packet_send(uint8_t *data, uint32_t length, int async);
void comm_vesc_packet_send_shutdown(void);
int comm_vesc_packet_send_get_data(uint32_t mask);
int comm_vesc_packet_send_get_setup(uint32_t mask);
void comm_vesc_packet_send_lights(uint8_t val);
void comm_vesc_packet_send_mconf_get(void);
void comm_vesc_packet_send_mconf_set(uint8_t *mconf);
#endif

#endif // __COMM_H__
