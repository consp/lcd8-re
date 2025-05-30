#ifndef __CAN_H__
#define __CAN_H__

#include <stdint.h>


#ifdef CAN_ENABLED
void can_init(void);
void can_deinit(void);
void can_send(uint32_t id, uint8_t *data, ssize_t length);
void can_vesc_send(uint32_t command, uint8_t *data, ssize_t data_length);
void can_vesc_shutdown(void);
void can_process(void);
void can_vesc_set_current_scale_max(void);
#else
#define can_init(x)
#define can_send(x, y, z)
#endif

#pragma pack(push,1)
typedef struct {
    int32_t rpm;
    int16_t current_motor;
    int16_t duty_cycle;
} vesc_status_1;

typedef struct {
    uint32_t amp_hours_consumed;
    uint32_t amp_hours_regen;
} vesc_status_2;

typedef struct {
    uint32_t watt_hours_consumed;
    uint32_t watt_hours_regen;
} vesc_status_3;

typedef struct {
    int16_t mosfet_temp;
    int16_t motor_temp;
    int16_t current_in;
    int16_t pid_pos;
} vesc_status_4;

typedef struct {
    int32_t tacho;
    int32_t voltage;
} vesc_status_5;

typedef struct {
    // int16_t adc_volt_1;
    // int16_t adc_volt_2;
    // int16_t adc_volt_3;
    // int16_t servo;
    uint32_t watt_hours_left;
    uint32_t distance_total;
} vesc_status_6;

typedef struct {
    uint8_t id;
    uint8_t cmd;
    uint8_t data[6];
    int     length; // calculated from received can
} vesc_short_packet;

#pragma pack(pop)


#endif // __CAN_H__
