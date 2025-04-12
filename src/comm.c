#include "comm.h"
#include "eeprom.h"
#include "uart.h"
#include "clock.h"
#include "crc.h"

// externals
//
extern int32_t speed, battery_current, battery_voltage_controller, mot_temperature, con_temperature, wheel_circumfence;
extern uint8_t draw_power_trigger, draw_speed_trigger, draw_temperatures_trigger, brake, controller_mode, draw_battery_voltage_trigger, draw_brake_trigger, draw_controller_mode_trigger;
extern settings_t settings;

extern volatile uint32_t timer_counter;
uint32_t comm_counter = 0;

void comm_update(void) {
    // parse if available and update variables
    uint8_t data[UART_RX_BUFFER_SIZE];;
    uint32_t buf_len = 0;
    uint32_t tval = 0;
    if (uart_get_data(data, &buf_len)) {
        uint32_t length = 0;
        while (length < buf_len && length < UART_RX_BUFFER_SIZE) {
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
        }
    }

    if (timer_counter - comm_counter > 5000) {
        /* comm_send_controller_settings(); */
        /* comm_send_display_settings(); */
        /* comm_send_display_status(); */
        comm_counter = timer_counter;
    }
}

void comm_send_display_settings(void) {
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
}

void comm_send_display_status(void) {
    uint8_t data[3 + sizeof(msg_display_status)] = { MSG_DISPLAY_STATUS };
    msg_display *msg = (msg_display *) data;
    msg->length = sizeof(msg_display_status);
    msg->status.assist_level = settings.assist_last;
    msg->status.lights = settings.lights_enabled;
    msg->crc = crc_calc_comm((uint8_t *) msg, sizeof(msg_display_status));
    
    uart_send(data, 3 + sizeof(msg_display_status), 0);
}

void comm_send_controller_settings(void) {
}

/* int get_msg(uint8_t *buffer, ssize_t *length) { */
/*     if (!comm_tx_ready) return 0; */
/*     if (NULL == buffer) return 0; */
/*     *length = DMA1_CHANNEL5->dtcnt; */
/*     #<{(| memcpy(buffer, comm_rx_buffer, *length); |)}># */
/*     DMA1_CHANNEL5->dtcnt = 0; */
/*     DMA1_CHANNEL5->maddr = (uint32_t ) comm_rx_buffer; */
/*     comm_rx_ready = 0; */
/* } */

