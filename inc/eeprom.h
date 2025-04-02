#ifndef __EEPROM_H__
#define __EEPROM_H__

#ifndef SIM
#include "at32f415_gpio.h"
#else
#include <string.h>
#endif
#include "delay.h"
#include "config.h"

#define T_LOW  10
#define T_HIGH 9
// this will be optimized away by the compiler if constant
#define SDA(x) x != 0 ? gpio_bits_set(GPIO_BLOCK, SDA_PIN) : gpio_bits_reset(GPIO_BLOCK, SDA_PIN)
#define SCL(x) x != 0 ? gpio_bits_set(GPIO_BLOCK, SCL_PIN) : gpio_bits_reset(GPIO_BLOCK, SCL_PIN)
#define DELAY_HIGH delay_us(T_HIGH)
#define DELAY_LOW delay_us(T_LOW)

#define I2C_READ 1
#define I2C_WRITE 0

#define GPIO_SET_READ_LOW(x, y)     x->cfglr &= ~(0xF << (y * 4)); x->cfglr |= 0x4 << (y * 4)
#define GPIO_SET_WRITE_LOW(x, y)    x->cfglr &= ~(0xF << (y * 4)); x->cfglr |= 0x6 << (y * 4)

void i2c_transfer(uint32_t address, uint8_t *values, ssize_t length, uint32_t RWFLAG, uint32_t delay);
uint8_t eeprom_read_byte(uint32_t byte, uint8_t page);
void eeprom_read_bytes(uint32_t start, uint8_t page, uint8_t *buffer, uint32_t length);
void eeprom_write_byte(uint8_t address, uint8_t page, uint8_t byte);
void eeprom_init(void);

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

    uint8_t     crc;
} settings_t;
#pragma pack(pop)

#define CRC_POLYNOMIAL 0x07  // Bluetooth crc8

void eeprom_read_settings(void);
void eeprom_write_settings(void);
uint8_t crc_calc(uint8_t *data, ssize_t length);
uint8_t crc_calc_uart(uint8_t *input, ssize_t length);
#endif // __EEPROM_H__
