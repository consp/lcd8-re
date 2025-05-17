#include "comm.h"
#include "eeprom.h"
#include "uart.h"
#include "clock.h"
#include "crc.h"

#ifdef PLATFORM_SIM 
#include <arpa/inet.h>
#define __ntohs     ntohs
#define __htons     htons
#define __ntohl     ntohl
#define __htonl     htonl
#endif

// externals
//
extern int32_t speed, battery_current, battery_voltage_controller, mot_temperature, con_temperature, wheel_circumfence;
extern uint8_t draw_power_trigger, draw_speed_trigger, draw_temperatures_trigger, brake, controller_mode, draw_battery_voltage_trigger, draw_brake_trigger, draw_controller_mode_trigger;
extern settings_t settings;

extern volatile uint32_t timer_counter;

uint32_t comm_counter = 0;
#if UART_COMM == UART_COMM_VESC
uint32_t comm_counter2 = 0;
#endif
volatile int rv = 0;
volatile uint32_t sel = 0;
volatile int32_t rpm = 0;
void comm_update(void) {
    // parse if available and update variables
    uint8_t data[UART_RX_BUFFER_SIZE];;
    uint32_t buf_len = 0;
    uint32_t tval = 0;
    if (uart_get_data(data, &buf_len)) {
        uint32_t length = 0;
        while (length < buf_len && length < UART_RX_BUFFER_SIZE) {
#if UART_COMM == UART_COMM_EBICS
            msg_controller *msg = (msg_controller *) &data[length];
            if (msg->length + length + 3 > UART_RX_BUFFER_SIZE) break;
            // calidate crc
            uint8_t crc = crc_calc_comm((uint8_t *) msg, msg->length);
            if (msg->crc == crc) {
                switch (msg->tag) {
                    case (MSG_VOLTAGE):
                        battery_voltage_controller = (int32_t) __ntohl(msg->voltage.value);
                        draw_battery_voltage_trigger = 1;
                        break;
                    case (MSG_CURRENT):
                        battery_current = (int32_t) __ntohl(msg->current.value); 
                        draw_power_trigger = 1; 
                        break;
                    case (MSG_ROTATION):
                        tval = __ntohs(msg->rotation.value);
                        if (tval > 0) {
                            // enable
                            clock_set_wheelspeed_timer(tval);
                            speed = (((int32_t)settings.wheel_circumfence) * 36000L) / (tval); 
                        } else {
                            clock_set_wheelspeed_timer(0);
                            speed = 0;
                        }
                        draw_speed_trigger = 1;
                        break;
                    case (MSG_TEMPERATURE):
                        mot_temperature = __ntohs(msg->temperature.engine); // in 0.1C
                        con_temperature = __ntohs(msg->temperature.controller); // in 0.1C 
                        draw_temperatures_trigger = 1;
                        break;
                    case (MSG_CONTROLLER_STATUS):
                        brake = msg->status.brake;
                        controller_mode = msg->status.mode;
                        draw_brake_trigger = 1;
                        draw_controller_mode_trigger = 1;
                        break;
                    default:
                        break;
                }
                length += msg->length + 3;
            } else {
                length++;
            }
#elif UART_COMM == UART_COMM_VESC
            vesc_packet_1 pck = {0};
            rv = comm_vesc_packet_resc(&data[length], buf_len - length, &pck);
            /* printf("Data fail: %02X%02X %lu %d\n", data[length + 0], data[length + 1], buf_len - length, rv); |)}># */
            if (!rv) {
                int indx = 1;
                switch (pck.data[0]) {
                    case COMM_VESC_CMD_GET_VALUES:
                        sel = 0xFFFFFFFF;
                    case COMM_VESC_CMD_GET_VALUES_SELECTIVE:
                        // get value
                        if (pck.data[0] == COMM_VESC_CMD_GET_VALUES_SELECTIVE) {
                            sel = __ntohl(*((uint32_t *) &pck.data[indx]));
                            indx += 4;
                        }
                        
                        if (sel & VESC_GET_FET_TEMP) {
                            // "float16" but actually just int16 * 10
                            con_temperature = __ntohs(*((uint16_t *) &pck.data[indx]));
                            indx += 2;
                            draw_temperatures_trigger = 1;
                        }
                        if (sel & VESC_GET_MOTOR_TEMP) {
                            // "float16" but actually just int16 * 10
                            mot_temperature = __ntohs(*((uint16_t *) &pck.data[indx]));
                            indx += 2;
                            draw_temperatures_trigger = 1;
                        }
                        if (sel & VESC_GET_AVG_MOTOR_CURRENT) {
                            battery_current = __ntohl(*((int32_t *) &pck.data[indx])) * 10; 
                            draw_power_trigger = 1; 
                            indx += 4;
                        }
                        if (sel & VESC_GET_AVG_INPUT_CURRENT) {
                            battery_current = __ntohl(*((int32_t *) &pck.data[indx])) * 10; 
                            draw_power_trigger = 1; 
                            indx += 4;
                        }
                        if (sel & VESC_GET_AVG_ID) indx += 4;
                        if (sel & VESC_GET_AVG_IQ) indx += 4;
                        if (sel & VESC_GET_DUTY_CYCLE) indx += 2;
                        if (sel & VESC_GET_RPM) {
                            rpm = __ntohl(*((int32_t *) &pck.data[indx]));
                            if (rpm == 0) {
                                clock_set_wheelspeed_timer(0);
                                speed = 0;
                            } else {
                                tval = 60000 / rpm; 
                                clock_set_wheelspeed_timer(tval);
                                speed = (6 * settings.wheel_circumfence * rpm) / 100; // cm/h
                            }
                            draw_speed_trigger = 1;
                            indx += 4;
                        }
                        if (sel & VESC_GET_INPUT_VOLTAGE) {
                            battery_voltage_controller = (int32_t) __ntohs(*((uint16_t *) &pck.data[indx])) * 100;
                            draw_battery_voltage_trigger = 1;
                            indx += 2;
                        }
                        if (sel & VESC_GET_AMP_HOURS) indx += 4;
                        if (sel & VESC_GET_AMP_HOURS_CHARGED) indx += 4;
                        if (sel & VESC_GET_WATT_HOURS) indx += 4;
                        if (sel & VESC_GET_WATT_HOURS_CHARGED) indx += 4;
                        if (sel & VESC_GET_TACH) indx += 4;
                        if (sel & VESC_GET_TACH_ABS) indx += 4;
                        if (sel & VESC_GET_FAULT) indx += 1;
                        if (sel & VESC_GET_PID_POS) indx += 4;
                        if (sel & VESC_GET_CONTROLLER_ID) indx += 1;
                        if (sel & VESC_GET_ALL_FET_TEMP) indx += 6;
                        if (sel & VESC_GET_AVG_VD) indx += 4;
                        if (sel & VESC_GET_AVG_VQ) indx += 4;
                        if (sel & VESC_GET_STATUS) indx += 1;
                        break;
                    default:
                        break;
                }
                length += 5 + pck.length;
            } else {
                length++;
            }
#endif
        }
    }

#if UART_COMM == UART_COMM_EBICS
    if (timer_counter - comm_counter > 5000) {
        comm_send_controller_settings();
        comm_send_display_settings();
        comm_send_display_status();
        comm_counter = timer_counter;
    }
#endif

#if UART_COMM == UART_COMM_VESC
    if (timer_counter - comm_counter > COMM_VESC_SLOW_INTERVAL) {
        // standard timer for normal data
        comm_vesc_packet_send_get_data(
                VESC_GET_FET_TEMP |
                VESC_GET_MOTOR_TEMP |
                VESC_GET_INPUT_VOLTAGE |
                VESC_GET_AMP_HOURS | 
                VESC_GET_FAULT |
                VESC_GET_STATUS
        );
        comm_counter = timer_counter;
    }
    if (timer_counter - comm_counter2 > COMM_VESC_FAST_INTERVAL) {
        comm_vesc_packet_send_get_data(
                VESC_GET_AVG_INPUT_CURRENT |
                VESC_GET_RPM // |
                /* VESC_GET_TACH */
        );
        comm_counter2 = timer_counter;
    }
#endif
}

void comm_send_display_settings(void) {
#if UART_COMM == UART_COMM_EBICS
    uint8_t data[3 + sizeof(msg_display_settings)] = { MSG_DISPLAY_SETTINGS };
    msg_display *msg = (msg_display *) data;
    msg->length = sizeof(msg_display_settings); 
   
    msg->settings.max_current = __htons(settings.current_max);
    msg->settings.wheel_circumfence = __htons(settings.wheel_circumfence);
    msg->settings.max_speed = settings.speed_assist_max;
    msg->settings.assist_levels = settings.assist_levels;
    msg->settings.pas_ramp = __htons(settings.pas_ramp);
    msg->settings.pas_timeout = __htons(settings.pas_timeout);
    msg->settings.regen_current = __htons(settings.regen_current);

    msg->crc = crc_calc_comm((uint8_t *) msg, sizeof(msg_display_settings));
    
    uart_send(data, 3 + sizeof(msg_display_settings), 0);
#elif UART_COMM == UART_COMM_VESC
#endif
}

void comm_send_display_status(void) {
#if UART_COMM == UART_COMM_EBICS
    uint8_t data[3 + sizeof(msg_display_status)] = { MSG_DISPLAY_STATUS };
    msg_display *msg = (msg_display *) data;
    msg->length = sizeof(msg_display_status);
    msg->status.assist_level = settings.assist_last;
    msg->status.lights = settings.lights_enabled;
    msg->crc = crc_calc_comm((uint8_t *) msg, sizeof(msg_display_status));
    
    uart_send(data, 3 + sizeof(msg_display_status), 0);
#elif UART_COMM == UART_COMM_VESC
#endif
}

void comm_send_controller_settings(void) {
}

#if UART_COMM == UART_COMM_VESC
#pragma message("VESC UART Communication")
int comm_vesc_packet_resc(uint8_t *data, uint32_t length, vesc_packet_1 *pack) {
    // check for min length, payload is 1 byte at least
    if ( 
            (data[0] == 2 && length < 6) || 
            (data[0] == 3 && length < 7) ||
            (data[0] == 4 && length < 8)) {
        return -1;
    }
    // translate length
    pack->type = data[0];
    uint32_t ind = 1;
    switch(pack->type) {
        default:
            return -2;
        case 2:
            pack->length = (uint32_t) data[1];
            ind++;
            break;
        case 3:
            pack->length = (data[1] << 8) | data[2];
            ind += 2;
            break;
        case 4:
            pack->length = (data[1] << 16) | data[2] << 8 | data[3];
            ind += 3;
            break;
    }

    // check length
    if (pack->length + ind + 3 > length) return -3;

    pack->data = &data[ind];
    pack->crc = data[ind + pack->length] << 8 | data[ind + pack->length + 1];

    if (data[ind + pack->length + 2] != 3) return -4;

    uint16_t crc = crc16(pack->data, pack->length);

    if (crc != pack->crc) return -5;

    return 0;
}

int comm_vesc_packet_send(uint8_t *data, uint32_t length) {
    // only support 8bit packet length for now
    uint8_t buffer[64];
    buffer[0] = 2;
    buffer[1] = (uint8_t) length;
    memcpy(buffer+2, data, length);
    uint16_t crc = crc16(data, length);
    buffer[length + 2] = crc >> 8;
    buffer[length + 3] = crc & 0xFF;
    buffer[length + 4] = 3;
    uart_send(buffer, length + 5, 1);
    return 1;
}

int comm_vesc_packet_send_get_data(uint32_t mask) {
    uint8_t buffer[5];
    mask = __htonl(mask);
    buffer[0] = COMM_VESC_CMD_GET_VALUES_SELECTIVE;
    memcpy(buffer+1, (uint8_t *) &mask, sizeof(uint32_t));
    comm_vesc_packet_send(buffer, 5);
    return 1;
}
#endif
