/*
 * Read/write to eeprom via i2c bitbang
 */
#include "at32f415_gpio.h"
#include "delay.h"
#include "config.h"
#include "eeprom.h"
#include "crc.h"

#define T_LOW  10
#define T_HIGH 9
// this will be optimized away by the compiler if constant
#define SDA(x) x != 0 ? gpio_bits_set(GPIOC, GPIO_PINS_6) : gpio_bits_reset(GPIOC, GPIO_PINS_6)
#define SCL(x) x != 0 ? gpio_bits_set(GPIOC, GPIO_PINS_7) : gpio_bits_reset(GPIOC, GPIO_PINS_7)
#define DELAY_HIGH delay_us(T_HIGH)
#define DELAY_LOW delay_us(T_LOW)

#define I2C_READ 1
#define I2C_WRITE 0

#define GPIO_SET_READ_LOW(x, y)     x->cfglr &= ~(0xF << (y * 4)); x->cfglr |= 0x4 << (y * 4)
#define GPIO_SET_WRITE_LOW(x, y)    x->cfglr &= ~(0xF << (y * 4)); x->cfglr |= 0x6 << (y * 4)

settings_t settings;

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
    GPIO_SET_READ_LOW(GPIOC, 6);
    SCL(0);
    rv = (gpio_input_data_read(GPIOC) & GPIO_PINS_6) != 0 ? 1 : 0;
    DELAY_LOW;
    GPIO_SET_WRITE_LOW(GPIOC, 6);
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

static void i2c_initialize(void) {
    i2c_stop();
}

void i2c_transfer(uint32_t address, uint8_t *values, ssize_t length, uint32_t RWFLAG, uint32_t delay) {
    i2c_start();

    address <<= 1; // shift
    address |= RWFLAG; 
                      
    if (!i2c_write_byte(address)) goto stop;

    if (!RWFLAG) {
        // write
        while (length--) {
            i2c_write_byte(*values++);
            if (delay) delay_us(delay);
        }
    } else {
        // read
        while(length--) {
            *values = i2c_read_byte();
            values++;
            if (length != 0) i2c_write_bit(0);
            else i2c_write_bit(1);
            if (delay) delay_us(delay);
        }
    }

stop:
    i2c_stop(); 
}

void eeprom_write_address(uint8_t address, uint8_t page) {
    i2c_transfer(0x50 | ((page & 0x07)), &address, 1, I2C_WRITE, 0);
}

void eeprom_write_byte(uint8_t address, uint8_t byte, uint8_t page) {
    uint8_t blob[2] = { address, byte };
    delay_ms(1);
    i2c_transfer(0x50 | ((page & 0x07)), blob, 2, I2C_WRITE, 10000);
}

void eeprom_write_bytes(uint8_t address, uint8_t page, uint8_t *data, uint32_t length) {
    uint8_t r = 1;
    /* while (length--) { */
    /*     eeprom_write_byte(address++, *data++, 0); */
    /* } */
    while (r && length) {
        i2c_start();
        r = 0;
        while(!r) r = i2c_write_byte((0x50 | (page & 0x07)) << 1 | I2C_WRITE);
        if (r) {
            r = i2c_write_byte(address);
        }
        uint8_t block = 0;
        while(r && length && block < 8) {
            r = i2c_write_byte(*data++);
            address++;
            block++;
            length--;
        }
        i2c_stop();
        delay_ms(5);
    }
}

uint8_t eeprom_read_byte(uint32_t byte, uint8_t page) {
    uint8_t buffer = byte & 0x000000ff;
    i2c_transfer(0x50 | ((page & 0x07)), &buffer, 1, I2C_WRITE, 0);
    buffer = 0;
    i2c_transfer(0x50 | ((page & 0x07)), &buffer, 1, I2C_READ, 0);
    return buffer;
}

uint8_t eeprom_read_current_byte(uint8_t page) {
    uint8_t buffer = 0;
    i2c_transfer(0x50 | ((page & 0x07)), &buffer, 1, I2C_READ, 0);
    return buffer;
}

void eeprom_read_bytes(uint32_t start, uint8_t page, uint8_t *buffer, uint32_t length) {
    /* for (uint32_t i = length; i > 0; i--) buffer[i] = 0; */
    uint8_t address = start & 0x000000FF;
    i2c_transfer(0x50 | ((page & 0x07)), &address, 1, I2C_WRITE, 0);
    buffer[0] = 0;
    i2c_transfer(0x50 | ((page & 0x07)), buffer, length, I2C_READ, 0);
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
    

    i2c_initialize();

    // read settings from eeprom
    eeprom_read_settings();
}

void eeprom_write_defaults(void) {
    settings.power_max = POWER_MAX;
    settings.power_min = POWER_MIN;
    settings.speed_max = SPEED_MAX;
    settings.current_max = 10000;
    settings.battery_voltage_min = BATTERY_MIN;
    settings.battery_voltage_max = BATTERY_MAX;
    settings.graph_duration = GRAPH_DURATION;
    settings.graph_shift = GRAPH_SHIFT;
    settings.graph_max = GRAPH_MAX;
    settings.assist_levels = ASSIST_LEVELS;
    settings.assist_last = ASSIST_DEFAULT;
    settings.wheel_circumfence = WHEEL_CIRCUMFENCE;
    settings.speed_redline = SPEED_MAX;
    settings.speed_assist_max = SPEED_MAX;
    settings.lights_enabled = 0;
    settings.lights_mode = 0;
    settings.battery_voltage_from_controller = 0;

    settings.trip_time = 0;
    settings.trip_distance = 0;
    settings.total_distance = 0;
    settings.backlight_level = 100;
    eeprom_write_settings();
}

void eeprom_read_settings(void) {
    eeprom_read_bytes(0, 0, (uint8_t *) &settings, sizeof(settings_t));
    uint8_t crc = crc_calc((uint8_t *) &settings, sizeof(settings_t) - 1);
    if (settings.crc != crc || settings.header != 0xCAFEBABE) {
        // set defaults
        eeprom_write_defaults();
    }
}

void eeprom_write_settings(void) {
    // calc crc
    settings.header = 0xCAFEBABE;
    settings.crc = crc_calc((uint8_t *) &settings, sizeof(settings_t) - 1);
    eeprom_write_bytes(0, 0, (uint8_t *) &settings, sizeof(settings_t));
}

