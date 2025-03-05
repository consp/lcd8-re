/*
 * Read/write to eeprom via i2c bitbang
 */

#include "at32f415_gpio.h"
#include "delay.h"
#include "eeprom.h"

uint32_t SCL_PIN = GPIO_PINS_7;
uint32_t SDA_PIN = GPIO_PINS_6;
gpio_type *GPIO_BLOCK = GPIOC;


static void i2c_start(void) {
    // only one device on bus, no arbitration needed
    SDA(0);
    DELAY_HIGH;
    SCL(0);
    DELAY_LOW;
}

static void i2c_stop(void) {
    SDA(0);
    DELAY_LOW;
    SCL(1);
    DELAY_HIGH; DELAY_HIGH;
    SDA(1);
    DELAY_LOW;
}

// always use 32bit, anything else is slower
static void i2c_write_bit(uint32_t bit) {
    SDA(bit);
    SCL(1);
    DELAY_HIGH;
    SCL(0);
    DELAY_LOW;
}

static uint32_t i2c_read_bit(void) {
    uint32_t rv = 0;
    SDA(1);
    SCL(1);
    DELAY_HIGH;
    GPIO_SET_READ_LOW(GPIO_BLOCK, 6);
    SCL(0);
    rv = (gpio_input_data_read(GPIO_BLOCK) & SDA_PIN) != 0 ? 1 : 0;
    DELAY_LOW;
    GPIO_SET_WRITE_LOW(GPIO_BLOCK, 6);
    return rv;
}

static uint32_t i2c_write_byte(uint32_t byte) {
    uint32_t mask = 1 << 7;
    do {
        i2c_write_bit(byte & mask);
    } while(mask >>= 1);
    return !i2c_read_bit();
}

static uint8_t i2c_read_byte(void) {
    uint32_t rv = 1;
    do {
        rv <<= 1;
        rv |= i2c_read_bit();
    } while(!(rv & 0x100));
    return rv & 0x000000FF;
}

void i2c_initialize(void) {
    i2c_stop();
}

void i2c_transfer(uint32_t address, uint8_t *values, ssize_t length, uint32_t RWFLAG) {
    i2c_start();

    address <<= 1; // shift
    address |= RWFLAG; 
                      
    if (!i2c_write_byte(address)) goto stop;

    if (!RWFLAG) {
        // write
        while (length > 0 && i2c_write_byte(*values++)) length--;
    } else {
        // read
        while(length--) {
            *values = i2c_read_byte();
            values++;
            if (length != 0) i2c_write_bit(0);
            else i2c_write_bit(1);
        }
    }

stop:
    i2c_stop(); 
}

void eeprom_write_address(uint8_t address) {
    i2c_transfer(0x50, &address, 1, I2C_WRITE);
}

void eeprom_write_byte(uint8_t address, uint8_t byte) {
    uint8_t blob[2] = { address, byte };
    i2c_transfer(0x50, blob, 2, I2C_WRITE);
    // write takes about 1000us
    delay_us(1000);
}

uint8_t eeprom_read_byte(uint32_t byte) {
    uint8_t buffer = byte & 0x000000ff;
    i2c_transfer(0x50, &buffer, 1, I2C_WRITE);
    buffer = 0;
    i2c_transfer(0x50, &buffer, 1, I2C_READ);
    return buffer;
}

uint8_t eeprom_read_current_byte(void) {
    uint8_t buffer = 0;
    i2c_transfer(0x50, &buffer, 1, I2C_READ);
    return buffer;
}

void eeprom_read_bytes(uint32_t start, uint8_t *buffer, uint32_t length) {
    /* for (uint32_t i = length; i > 0; i--) buffer[i] = 0; */
    uint8_t address = start & 0x000000FF;
    i2c_transfer(0x50, &address, 1, I2C_WRITE);
    buffer[0] = 0;
    i2c_transfer(0x50, buffer, length, I2C_READ);
    return;
}

void eeprom_init(void) {
  gpio_init_type gpio_initstructure;
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);

  /* SDA pin PC6 */
  gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_OPEN_DRAIN;
  gpio_initstructure.gpio_pull           = GPIO_PULL_NONE; // external pullup
  gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
  gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_initstructure.gpio_pins           = GPIO_PINS_6;
  gpio_init(GPIOC, &gpio_initstructure);

  /* SCL pin PC7 */
  gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_OPEN_DRAIN;
  gpio_initstructure.gpio_pull           = GPIO_PULL_NONE; // external pullup
  gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
  gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_initstructure.gpio_pins           = GPIO_PINS_7;
  gpio_init(GPIOC, &gpio_initstructure);
}

