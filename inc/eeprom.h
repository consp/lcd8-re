#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "at32f415_gpio.h"
#include "delay.h"

#define T_LOW  5
#define T_HIGH 4
// this will be optimized away by the compiler if constant
#define SDA(x) x != 0 ? gpio_bits_set(GPIO_BLOCK, SDA_PIN) : gpio_bits_reset(GPIO_BLOCK, SDA_PIN)
#define SCL(x) x != 0 ? gpio_bits_set(GPIO_BLOCK, SCL_PIN) : gpio_bits_reset(GPIO_BLOCK, SCL_PIN)
#define DELAY_HIGH delay_us(T_HIGH)
#define DELAY_LOW delay_us(T_LOW)

#define I2C_READ 1
#define I2C_WRITE 0

#define GPIO_SET_READ_LOW(x, y)     x->cfglr &= ~(0xF << (y * 4)); x->cfglr |= 0x4 << (y * 4)
#define GPIO_SET_WRITE_LOW(x, y)    x->cfglr &= ~(0xF << (y * 4)); x->cfglr |= 0x6 << (y * 4)

void i2c_transfer(uint32_t address, uint8_t *values, ssize_t length, uint32_t RWFLAG);
void i2c_initialize(void);
uint8_t eeprom_read_byte(uint32_t byte);
void eeprom_read_bytes(uint32_t start, uint8_t *buffer, uint32_t length);
void eeprom_write_byte(uint8_t address, uint8_t byte);
void eeprom_init(void);

#endif // __EEPROM_H__
