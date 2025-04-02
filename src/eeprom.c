/*
 * Read/write to eeprom via i2c bitbang
 */

#include "delay.h"
#include "eeprom.h"
#include "gui.h"

#ifndef SIM
uint32_t SCL_PIN = GPIO_PINS_7;
uint32_t SDA_PIN = GPIO_PINS_6;
gpio_type *GPIO_BLOCK = GPIOC;

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

static const uint8_t CRC_TABLE[256] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
    0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
    0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
    0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
    0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
    0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
    0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
    0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
    0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
    0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
    0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
    0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

uint8_t crc8ccitt(uint8_t *data, size_t size, uint8_t *preamble, ssize_t preamble_length) {
	uint8_t val = 0;
    if (preamble) {
        while (preamble_length--) val = CRC_TABLE[val ^ *preamble++];
    }

	while (size--) val = CRC_TABLE[val ^ *data++];

	return val;
}

static void crc_init(void) {
    /* crm_periph_clock_enable(CRM_CRC_PERIPH_CLOCK, TRUE); */
    /* CRC->idt = 0x00; // iv 0x00 */
    /* CRC->ctrl_bit.poly_size = CRC_POLY_SIZE_8B; */
    /* CRC->ctrl_bit.revid = 0; */
    /* CRC->ctrl_bit.revod = 0; */
    /* CRC->poly = CRC_POLYNOMIAL; */
    /* CRC->ctrl_bit.rst = 1; */
}

uint8_t crc_calc(uint8_t *input, ssize_t length) {
    /* crc_data_reset(); */
    /* CRC->ctrl_bit.rst = 0x1; */
    /* while(length--) CRC->dt = *input++; */
    return crc8ccitt(input, length, NULL, 0);
}

uint8_t crc_calc_uart(uint8_t *input, ssize_t length) {
    return crc8ccitt(input+3, length, input, 2);
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
    crc_init();

    //

    /* uint8_t iv[5] = { 0xaa, 0xaa, 0xaa, 0xaa, 0xaa }; */
    /* uint8_t result = crc_calc(iv, 5); */
    //
    // read from file?

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
    settings.lights_mode = LIGHTS_MODE_MANUAL;
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


#else
uint8_t eeprom_data[EEPROM_SIZE];

uint8_t eeprom_read_byte(uint32_t byte) {
    return eeprom_data[byte];
}

void eeprom_read_bytes(uint32_t start, uint8_t *buffer, uint32_t length) {
    memcpy(buffer, &eeprom_data[start], length);
}

void eeprom_write_byte(uint8_t address, uint8_t byte) {
    eeprom_data[address] = byte;
}

void eeprom_init(void) {
}


#endif
