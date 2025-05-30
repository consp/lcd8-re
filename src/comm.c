#include "comm.h"
#include "eeprom.h"
#include "uart.h"
#include "clock.h"
#include "crc.h"
#include "lvgl.h"
#include "config.h"
#include <errno.h>
#include <math.h>
#ifdef CAN_ENABLED
#include "can.h"
#endif

#define min(a,b) (a < b ? a : b)
#ifdef PLATFORM_SIM 
#include <arpa/inet.h>
#define __ntohs     ntohs
#define __htons     htons
#define __ntohl     ntohl
#define __htonl     htonl
#endif

// system is possibly in Os mode
#pragma GCC optimize ("O3")

// externals
extern int32_t erpm, speed, motor_current, battery_current, battery_voltage_controller, mot_temperature, con_temperature, wheel_circumfence, amphours_total, amphours_regen_total, tachometer, tachometer_abs;
extern float wh_left, wh_total, wh_regen_total, distance_total;
extern uint8_t draw_power_trigger, draw_speed_trigger, draw_temperatures_trigger, brake, controller_mode, draw_battery_voltage_trigger, draw_brake_trigger, draw_controller_mode_trigger, controller_id, vesc_status;
extern int16_t fet_temp_1, fet_temp_2, fet_temp_3, duty_cycle;
extern settings_t settings;

extern volatile uint32_t timer_counter;

CRITICAL ssize_t comm_provess_vesc_msg(uint8_t *data, ssize_t length);

uint32_t comm_counter = 0;
int8_t comm_ready = 0;
#if UART_COMM == UART_COMM_VESC
#include <math.h>
uint32_t comm_counter2 = 0;
uint32_t comm_counter_very_slow = 0;
extern vesc_mcconf_temp mcconf;
extern volatile int mconf_actual;
extern volatile int mconf_updated; 
void buffer_append_float32_auto(uint8_t* buffer, float number);
float buffer_get_float32_auto(const uint8_t *buffer, int32_t *index);
#endif

int rv = 0;
uint32_t sel = 0;

#ifndef CAN_ENABLED
void comm_update(void) {
    // parse if available and update variables
    uint8_t data[UART_RX_BUFFER_SIZE];;
    uint32_t buf_len = 0;
    int32_t rpm = 0;
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
                        rpm  = __ntohs(msg->rotation.value);
                        if (rpm  > 0) {
                            // enable
                            clock_set_wheelspeed_timer(60000 / rpm);
                            speed = (((int32_t)settings.wheel_circumfence) * 36000L) / (rpm); 
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
                
                length += 5 + comm_provess_vesc_msg(pck.data, pck.length);
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
                VESC_GET_AMP_HOURS_CHARGED |
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
        /* comm_vesc_packet_send_get_setup( */
        /*         VESC_GET_SETUP_SPEED |  */
        /*         VESC_GET_SETUP_CURRENT_TOT */
        /* ); */
        comm_counter2 = timer_counter;
    }

    if (timer_counter - comm_counter_very_slow > COMM_VESC_VERY_SLOW_INTERVAL) {
        if (!mconf_actual) {
            LV_LOG_INFO("Requesting mcconf");
            comm_send_controller_settings();
        }
        if (mconf_actual && !mconf_updated) {
            comm_send_display_settings();
        }
        comm_vesc_packet_send_get_setup(
                VESC_GET_SETUP_INPUT_VOLTAGE |
                VESC_GET_SETUP_BATTERY_LEVEL |
                VESC_GET_SETUP_AH_TOT | 
                VESC_GET_SETUP_AH_CHARGE_TOT | 
                VESC_GET_SETUP_WH_TOT | 
                VESC_GET_SETUP_WH_CHARGE_TOT |
                VESC_GET_SETUP_WH_LEFT |
                VESC_GET_SETUP_DISTANCE |
                VESC_GET_SETUP_ODO |
                VESC_GET_SETUP_SPEED | 
                VESC_GET_SETUP_SYS_TIME
        );
        comm_counter_very_slow = timer_counter;
    }
#endif
#ifdef BT_UART_ENABLED
    bt_check();
#endif
}
#else
inline void comm_update(void) {
    can_process();
#ifdef BT_UART_ENABLED
    bt_check();
#endif
}
#endif

CRITICAL ssize_t comm_process_vesc_msg(uint8_t *data, ssize_t length) {
    int indx = 1;
    int32_t rpm;
    switch (data[0]) {
        case COMM_VESC_CMD_GET_VALUES:
            sel = 0xFFFFFFFF;
        case COMM_VESC_CMD_GET_VALUES_SELECTIVE:
            // get value
            if (data[0] == COMM_VESC_CMD_GET_VALUES_SELECTIVE) {
                sel = __ntohl(*((uint32_t *) &data[indx]));
                indx += 4;
            }
            
            if (sel & VESC_GET_FET_TEMP) {
                // "float16" but actually just int16 * 10
                con_temperature = __ntohs(*((uint16_t *) &data[indx]));
                indx += 2;
                draw_temperatures_trigger = 1;
            }
            if (sel & VESC_GET_MOTOR_TEMP) {
                // "float16" but actually just int16 * 10
                mot_temperature = __ntohs(*((uint16_t *) &data[indx]));
                indx += 2;
                draw_temperatures_trigger = 1;
            }
            if (sel & VESC_GET_AVG_MOTOR_CURRENT) {
                motor_current = __ntohl(*((int32_t *) &data[indx])) * 10; 
                draw_power_trigger = 1; 
                indx += 4;
            }
            if (sel & VESC_GET_AVG_INPUT_CURRENT) {
                battery_current = __ntohl(*((int32_t *) &data[indx])) * 10; 
                draw_power_trigger = 1; 
                indx += 4;
            }
            if (sel & VESC_GET_AVG_ID) indx += 4;
            if (sel & VESC_GET_AVG_IQ) indx += 4;
            if (sel & VESC_GET_DUTY_CYCLE) {
                duty_cycle = __ntohs(*((int16_t *) &data[indx]));
                indx += 2;
            }
            if (sel & VESC_GET_RPM) {
                rpm = erpm = __ntohl(*((int32_t *) &data[indx]));
                if (rpm == 0) {
                    clock_set_wheelspeed_timer(0);
                    speed = 0;
                } else {
                    if (mconf_actual) {
                        rpm /= (mcconf.motor_poles >> 1);
                    } else {
                        rpm /= 10; // default
                    }
                    
                    clock_set_wheelspeed_timer(rpm);
                    speed = (6 * settings.wheel_circumfence * rpm) / 10; // cm/h
                }
                draw_speed_trigger = 1;
                indx += 4;
            }
            if (sel & VESC_GET_INPUT_VOLTAGE) {
                battery_voltage_controller = (int32_t) __ntohs(*((uint16_t *) &data[indx])) * 100;
                draw_battery_voltage_trigger = 1;
                indx += 2;
            }
            if (sel & VESC_GET_AMP_HOURS) {
                amphours_total = (int32_t) __ntohl(*((uint32_t *) &data[indx])); // in mah
                indx += 4;
            }
            if (sel & VESC_GET_AMP_HOURS_CHARGED) {
                amphours_regen_total = (int32_t) __ntohl(*((uint32_t *) &data[indx])); // in mah
                indx += 4;
            }
            if (sel & VESC_GET_WATT_HOURS) indx += 4;
            if (sel & VESC_GET_WATT_HOURS_CHARGED) indx += 4;
            if (sel & VESC_GET_TACH) {
                tachometer = __ntohl(*((int32_t *) &data[indx]));
                indx += 4;
            }
            if (sel & VESC_GET_TACH_ABS) { 
                tachometer_abs = __ntohl(*((int32_t *) &data[indx]));
                indx += 4;
            }
            if (sel & VESC_GET_FAULT) indx += 1;
            if (sel & VESC_GET_PID_POS) indx += 4;
            if (sel & VESC_GET_CONTROLLER_ID) {
                controller_id = data[indx];
                indx += 1;
            }
            if (sel & VESC_GET_ALL_FET_TEMP) {
                fet_temp_1 = __ntohs(*((int16_t *) &data[indx]));
                fet_temp_2 = __ntohs(*((int16_t *) &data[indx+2]));
                fet_temp_3 = __ntohs(*((int16_t *) &data[indx+4]));
                indx += 6;
            }
            if (sel & VESC_GET_AVG_VD) indx += 4;
            if (sel & VESC_GET_AVG_VQ) indx += 4;
            if (sel & VESC_GET_STATUS) {
                vesc_status = data[indx];
                indx += 1;
            }
            break;
        case COMM_VESC_CMD_GET_VALUES_SETUP:
            sel = 0xFFFFFFFF;
        case COMM_VESC_CMD_GET_VALUES_SETUP_SELECTIVE:
            // get value
            if (data[0] == COMM_VESC_CMD_GET_VALUES_SETUP_SELECTIVE) {
                sel = __ntohl(*((uint32_t *) &data[indx]));
                indx += 4;
            }
            if (sel & VESC_GET_SETUP_FET_TEMP) {
                indx += 2;
                con_temperature = __ntohs(*((uint16_t *) &data[indx]));
                draw_temperatures_trigger = 1;
            }
            if (sel & VESC_GET_SETUP_MOTOR_TEMP) {
                indx += 2;
                mot_temperature = __ntohs(*((uint16_t *) &data[indx]));
                draw_temperatures_trigger = 1;
            }
            if (sel & VESC_GET_SETUP_CURRENT_TOT) {
                indx += 4;
            }
            if (sel & VESC_GET_SETUP_CURRENT_IN_TOT) indx += 4;
            if (sel & VESC_GET_SETUP_DUTY_CYCLE) indx += 2;
            if (sel & VESC_GET_SETUP_RPM) indx += 4;
            if (sel & VESC_GET_SETUP_SPEED) {
                /* uint32_t sp = (int32_t) __ntohl(*((uint32_t *) &data[indx])); // in mah */
                indx += 4;
            }
            if (sel & VESC_GET_SETUP_INPUT_VOLTAGE) indx += 2;
            if (sel & VESC_GET_SETUP_BATTERY_LEVEL) indx += 2;
            if (sel & VESC_GET_SETUP_AH_TOT) {
                amphours_total = (int32_t) __ntohl(*((uint32_t *) &data[indx])); // in mah
                indx += 4;
            }
            if (sel & VESC_GET_SETUP_AH_CHARGE_TOT) {
                amphours_regen_total = (int32_t) __ntohl(*((uint32_t *) &data[indx])); // in mah
                indx += 4;
            }
            if (sel & VESC_GET_SETUP_WH_TOT) {
                wh_total = (float) __ntohl(*((uint32_t *) &data[indx])) / 10; // in mWh
                indx += 4;
            }
            if (sel & VESC_GET_SETUP_WH_CHARGE_TOT) {
                wh_regen_total = (float) __ntohl(*((uint32_t *) &data[indx])) / 10; // in mWh
                indx += 4;
            }
            if (sel & VESC_GET_SETUP_DISTANCE) {
                distance_total = (float) __ntohl(*((uint32_t *) &data[indx])) / 1000; // in m
                indx += 4;
            }
            if (sel & VESC_GET_SETUP_DISTANCE_ABS) indx += 4;
            if (sel & VESC_GET_SETUP_PID_POS) indx += 4;
            if (sel & VESC_GET_SETUP_FAULT) indx += 1;
            if (sel & VESC_GET_SETUP_VESC_ID) indx += 1;
            if (sel & VESC_GET_SETUP_NUM_VESC) indx += 1;
            if (sel & VESC_GET_SETUP_WH_LEFT) {
                wh_left = (int32_t) __ntohl(*((uint32_t *) &data[indx])); // in mah
                indx += 4;
            }
            if (sel & VESC_GET_SETUP_ODO) indx += 4;
            if (sel & VESC_GET_SETUP_SYS_TIME) {
                indx += 4;
            }
            break;
        case COMM_VESC_CMD_GET_MCCONF_TEMP:
            // get data
            if (length < 49) {
                // ack
                LV_LOG_INFO("Ack message for MCCONF");
                indx += length;
                break;
            }
            memcpy((uint8_t *) &mcconf.current_scale_min, &data[indx], 49); 
            for (int i = 0; i < 40; i+=4) {
                LV_LOG_INFO("MCV: %02X%02X%02X%02X", data[i], data[i+1], data[i+2], data[i+3]); 
            }
            indx += 49;
            mconf_actual = 1;
            LV_LOG_INFO("MCCONF Received");
            // temp convert
            break;
        case COMM_VESC_CMD_SET_MCCONF_TEMP:
            // response from controller
            mconf_updated = 1;
            comm_ready = 1;
            LV_LOG_INFO("Confirm MCCONF set");
            break;
        default:
            LV_LOG_INFO("Unknown command: %02X", data[0]); 
            break;
    }
    return indx;
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
    
    cnt_send(data, 3 + sizeof(msg_display_settings));
#elif UART_COMM == UART_COMM_VESC
    if (!mconf_actual) {
        LV_LOG_INFO("Motor configuration not actual");
        return;
    }
    // set all values in mconf
    float current_max = (float) settings.current_max / 1000.0;
    float wheel_circumfence = (float) settings.wheel_circumfence * 10;
    float max_speed = (((float) settings.speed_assist_max) * 1000000.0 * (mcconf.motor_poles / 2)) / (wheel_circumfence * 60);
    float regen_current = -1.0 * (((float) settings.regen_current) / 1000.0);
    float assist_level = ((float) settings.assist_last) * (1.0/9.0);
    float power_max = ((float) settings.controller_power_max);
    float power_min = ((float) settings.controller_power_min);
    if (assist_level > 0.9) assist_level = 1.0; // fix float being float
    wheel_circumfence /= M_PI;
    // convert values
    LV_LOG_INFO("Values mcconf: %d %d %d %d %d %d %d %d", 
            (int) assist_level, 
            (int) current_max, 
            (int) regen_current,
            (int) power_min,
            (int) power_max,
            (int) wheel_circumfence,
            (int) max_speed,
            (int) mcconf.motor_poles);
    buffer_append_float32_auto((uint8_t *) &mcconf.current_scale_max, assist_level);
    buffer_append_float32_auto((uint8_t *) &mcconf.erpm_max, max_speed);
    buffer_append_float32_auto((uint8_t *) &mcconf.watt_min, power_min);
    buffer_append_float32_auto((uint8_t *) &mcconf.watt_max, power_max);
    buffer_append_float32_auto((uint8_t *) &mcconf.in_current_min, regen_current);
    buffer_append_float32_auto((uint8_t *) &mcconf.in_current_max, current_max);
    buffer_append_float32_auto((uint8_t *) &mcconf.wheel_diameter, wheel_circumfence);

    // send
    comm_vesc_packet_send_mconf_set(&mcconf);
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
    
    cnt_send(data, 3 + sizeof(msg_display_status));
#elif UART_COMM == UART_COMM_VESC
    comm_send_lights();
    comm_send_display_settings(); // there is no easy way to do this
#endif
}

void comm_send_lights(void) {
    LV_LOG_INFO("Lights: %d", settings.lights_enabled);
    comm_vesc_packet_send_lights(settings.lights_enabled ? 0x03 : 0x00);
}


void comm_send_controller_settings(void) {
#if UART_COMM == UART_COMM_VESC
    LV_LOG_INFO("Requesting mcconf temp settings");
    comm_vesc_packet_send_mconf_get();
#endif
}

#if UART_COMM == UART_COMM_VESC
#pragma message("VESC UART Communication")


// for now copy them, should be externally included since GPL is shit
void buffer_append_float32_auto(uint8_t* buffer, float number) {
	// Set subnormal numbers to 0 as they are not handled properly
	// using this method.
	if (fabsf(number) < 1.5e-38) {
		number = 0.0;
	}

	int e = 0;
	float sig = frexpf(number, &e);
	float sig_abs = fabsf(sig);
	uint32_t sig_i = 0;

	if (sig_abs >= 0.5) {
		sig_i = (uint32_t)((sig_abs - 0.5f) * 2.0f * 8388608.0f);
		e += 126;
	}

	uint32_t res = ((e & 0xFF) << 23) | (sig_i & 0x7FFFFF);
	if (sig < 0) {
		res |= 1U << 31;
	}

    /* res = __htonl(res); // convert to network */
    /* *buffer = res; */
    buffer[0] = res >> 24U;
    buffer[1] = res >> 16U;
    buffer[2] = res >> 8u;
    buffer[3] = res;
}

float uint32_to_float32_auto(void *buffer) {
    uint32_t res = *((uint32_t *) buffer);
    res = __ntohl(res);
	int e = (res >> 23) & 0xFF;
	uint32_t sig_i = res & 0x7FFFFF;
	int neg = res & (1U << 31);

	float sig = 0.0;
	if (e != 0 || sig_i != 0) {
		sig = (float)sig_i / (8388608.0 * 2.0) + 0.5;
		e -= 126;
	}

	if (neg) {
		sig = -sig;
	}
	return ldexpf(sig, e);
}

float buffer_get_float32_auto(const uint8_t *buffer, int32_t *index) {
	uint32_t res = __ntohl(*((uint32_t *) &buffer[*index]));
    *index += 4;

	int e = (res >> 23) & 0xFF;
	uint32_t sig_i = res & 0x7FFFFF;
	int neg = res & (1U << 31);

	float sig = 0.0;
	if (e != 0 || sig_i != 0) {
		sig = (float)sig_i / (8388608.0 * 2.0) + 0.5;
		e -= 126;
	}

	if (neg) {
		sig = -sig;
	}

	return ldexpf(sig, e);
}

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

CRITICAL int comm_vesc_packet_send(uint8_t *data, uint32_t length, int async) {
    // only support 8bit packet length for now
    static uint8_t buffer[64];
    uint16_t crc = crc16(data, length);
#ifdef CAN_ENABLED
    buffer[0] = VESC_OWN_ID;
    buffer[1] = 0;
    buffer[2] = (uint8_t) (length & 0xFF00) >> 8;
    buffer[3] = (uint8_t) length & 0x00FF;
    buffer[4] = crc >> 8;
    buffer[5] = crc & 0xFF;
    int indx = 0;
    if (length > 7) {
        while(indx < length) {
            uint8_t tbuf[8];
            memset(tbuf, 0x00, 8);
            tbuf[0] = indx;
            memcpy(tbuf + 1, data + indx, min(length - indx, 7));
            can_vesc_send(VESC_CAN_CMD_FILL_RX_BUFFER, tbuf, min(1 + (length - indx), 8));
            indx += 7;
        }
        buffer[6] = 0x00;
        buffer[7] = 0x00;
        can_vesc_send(VESC_CAN_CMD_PROCESS_RX_BUFFER, buffer, 8);
        can_vesc_send(0, NULL, 0);
    } else {
        memcpy(buffer+2, data, length);
        can_vesc_send(VESC_CAN_CMD_PROCESS_RX_BUFFER_SHORT, buffer, 2 + length); 
    }
#else
    buffer[0] = 2;
    buffer[1] = (uint8_t) length & 0x00FF;
    buffer[length + 2] = crc >> 8;
    buffer[length + 3] = crc & 0xFF;
    buffer[length + 4] = 3;

    cnt_send(buffer, length + 5);
#endif
    return 1;
}

int comm_vesc_packet_send_get_setup(uint32_t mask) {
    uint8_t buffer[5];
    mask = __htonl(mask);
    buffer[0] = COMM_VESC_CMD_GET_VALUES_SETUP_SELECTIVE;
    memcpy(buffer+1, (uint8_t *) &mask, sizeof(uint32_t));
    comm_vesc_packet_send(buffer, 5, TRUE);
    return 1;
}

int comm_vesc_packet_send_get_data(uint32_t mask) {
    uint8_t buffer[5];
    mask = __htonl(mask);
    buffer[0] = COMM_VESC_CMD_GET_VALUES_SELECTIVE;
    memcpy(buffer+1, (uint8_t *) &mask, sizeof(uint32_t));
    comm_vesc_packet_send(buffer, 5, TRUE);
    return 1;
}

void comm_vesc_packet_send_shutdown(void) {
    uint8_t buffer[3];
    buffer[0] = COMM_VESC_CMD_SHUTDOWN;
    buffer[1] = 1; // force 1
    buffer[2] = 0;
#ifdef CAN_ENABLED
    can_vesc_shutdown();
#endif
    comm_vesc_packet_send(buffer, 3, FALSE);
}

void comm_vesc_packet_send_mconf_get(void) {
    uint8_t buffer[3];
    buffer[0] = COMM_VESC_CMD_GET_MCCONF_TEMP;
    comm_vesc_packet_send(buffer, 1, FALSE);
}

void comm_vesc_packet_send_mconf_set(vesc_mcconf_temp *mcconf) {
    /*
     * 4 cur min scal, 4 cur max scale
     * 4 min erpm, 4 max erpm
     * 4 min duty, 4 max duty 
     * 4 min watt, 4 max watt
     * 4 in curr min, 4 in curr max
     * 1 motor poles
     * 4 gear ratio
     * 4 wheel diameter
     * = 49 bytes
     */
    // mconf is assumed to be 50 bytes long
    LV_LOG_INFO("Sending mcconf");
    mconf_updated = 0;
    mcconf->cmd = COMM_VESC_CMD_SET_MCCONF_TEMP;
    mcconf->store = 1;
    mcconf->forward_can = 0;
    mcconf->ack = 1;
    mcconf->divide_by_controllers = 0;
    comm_vesc_packet_send((uint8_t*) mcconf, sizeof(vesc_mcconf_temp), FALSE);
}

void comm_vesc_packet_send_lights(uint8_t val) {
#ifdef CAN_ENABLED
    can_send(VESC_LIGHTS_ID | (0x02 << 8), &val, 1);
#else
    uint8_t buffer[4];
    buffer[0] = COMM_VESC_CMD_FORWARD_CAN;
    buffer[1] = VESC_LIGHTS_ID;
    buffer[2] = 0x02; // set
    buffer[3] = val;
    comm_vesc_packet_send(buffer, 4, TRUE);
#endif
}
#endif

#ifdef CAN_ENABLED
void comm_vesc_ping(void) {
    uint8_t packet[1] = { VESC_OWN_ID };
    can_send(VESC_CAN_ID | (VESC_CAN_CMD_PING << 8), packet, 1); 
}

void comm_vesc_pong(uint8_t id) {
    uint8_t packet[2] = { VESC_OWN_ID, 2 };
    can_send(id | (VESC_CAN_CMD_PONG << 8), packet, 2); 
}
#endif
