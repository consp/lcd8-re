/*
 * Read/write to eeprom via i2c bitbang
 */
#include "delay.h"
#include "config.h"
#include "eeprom.h"
#include "crc.h"

#define T_LOW  10
#define T_HIGH 7
#define T_SHORT 1
// this will be optimized away by the compiler if constant
#define SDA(x) if (x != 0) { GPIOC->scr = GPIO_PINS_6;} else {GPIOC->clr = GPIO_PINS_6;}
#define SCL(x) if (x != 0) { GPIOC->scr = GPIO_PINS_7;} else {GPIOC->clr = GPIO_PINS_7;}
#define DELAY_HIGH delay_us(T_HIGH)
#define DELAY_LOW delay_us(T_LOW)
#define DELAY_SHORT delay_us(T_SHORT)

#define I2C_READ 1
#define I2C_WRITE 0

#define GPIO_SET_READ_LOW(x, y)     x->cfglr &= ~(0xF << (y * 4)); x->cfglr |= 0x4 << (y * 4); x->clr = 1 << y;
#define GPIO_SET_WRITE_LOW(x, y)    x->cfglr &= ~(0xF << (y * 4)); x->cfglr |= 0x2 << (y * 4); x->scr = 1 << y;

settings_t settings;

/*
static inline void i2c_start(void) {
    // only one device on bus, no arbitration needed
    SDA(1);
    DELAY_HIGH;
    SDA(0);
    DELAY_HIGH;
    SCL(0);
    DELAY_LOW;
}

static void i2c_stop(void) {
    SDA(0);
    DELAY_LOW;
    SCL(1);
    DELAY_HIGH;
    SDA(1);
    DELAY_LOW;
}

// always use 32bit, anything else is slower
static void i2c_write_bit(uint32_t bit) {
    SDA(bit);
    DELAY_SHORT;
    SCL(1);
    DELAY_HIGH;
    SCL(0);
    DELAY_LOW;
}

static uint32_t i2c_read_bit(void) {
    uint32_t rv = 0;
    SDA(1);
    DELAY_HIGH;
    SCL(1);
    DELAY_HIGH;
    GPIO_SET_READ_LOW(GPIOC, 6);
    rv = (GPIOC->idt & GPIO_PINS_6) != 0 ? 1 : 0;
    SCL(0);
    DELAY_LOW;
    GPIO_SET_WRITE_LOW(GPIOC, 6);
    return rv;
}

static uint32_t i2c_write_byte(uint32_t byte) {
    uint32_t mask = 1 << 7;
    do {
        i2c_write_bit(byte & mask);
    } while(mask >>= 1);
    return i2c_read_bit();
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
}
*/

int i2c_transfer(uint32_t address, uint8_t *values, ssize_t length, uint32_t RWFLAG, uint32_t delay) {
    /*
    int r = 0;
    i2c_start();

    address <<= 1; // shift
    address |= RWFLAG; 
                      
    if ((r = i2c_write_byte(address))) {
        goto stop;
    }

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
    return r;
    */
    return 0;
}

void eeprom_write_address(uint8_t address, uint8_t page) {
    //i2c_transfer(0x50 | ((page & 0x07)), &address, 1, I2C_WRITE, 0);
}

void eeprom_write_byte(uint8_t address, uint8_t byte, uint8_t page) {
    //uint8_t blob[2] = { address, byte };
    //i2c_transfer(0x50 | ((page & 0x07)), blob, 2, I2C_WRITE, 100);
}

void eeprom_write_bytes(uint8_t address, uint8_t page, uint8_t *data, uint32_t length) {
    /*
    uint8_t r = 0;
    while (!r && length) {
        i2c_start();
        r = i2c_write_byte(((0x50 | (page & 0x07)) << 1) | I2C_WRITE);
        if (!r) {
            r = i2c_write_byte(address);
        }
        uint8_t block = 0;
        while(!r && length && block < 8) {
            r = i2c_write_byte(*data++);
            address++;
            block++;
            length--;
        }
        i2c_stop();
        delay_ms(5);
    }*/
}

uint8_t eeprom_read_byte(uint8_t byte, uint8_t page) {
    /*
    uint8_t buffer = byte;
    int r = i2c_transfer(0x50 | ((page & 0x07)), &buffer, 1, I2C_WRITE, 0);
    if (r) return 0x00;
    buffer = 0;
    i2c_transfer(0x50 | ((page & 0x07)), &buffer, 1, I2C_READ, 0);
    return buffer;*/
    return 0;
}

uint8_t eeprom_read_current_byte(uint8_t page) {
    /*
    uint8_t buffer = 0;
    i2c_transfer(0x50 | ((page & 0x07)), &buffer, 1, I2C_READ, 0);
    return buffer;*/
    return 0;
}

void eeprom_read_bytes(uint8_t address, uint8_t page, uint8_t *buffer, uint32_t length) {
    /*
    int r = i2c_transfer(0x50 | ((page & 0x07)), &address, 1, I2C_WRITE, 0);
    if (r) return;
    buffer[0] = 0;
    i2c_transfer(0x50 | ((page & 0x07)), buffer, length, I2C_READ, 0);
    return;*/
}

void eeprom_init(void) {
    /*
    crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);

    GPIOC->clr = (GPIO_PINS_7 | GPIO_PINS_6);
    GPIO_SET_WRITE_LOW(GPIOC, 7);
    GPIO_SET_WRITE_LOW(GPIOC, 6);

    i2c_initialize();
*/
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
    settings.regen_current = REGEN_CURRENT;
    settings.pas_timeout = PAS_TIMEOUT;
    settings.pas_ramp = PAS_RAMP;

    settings.trip_time = 0;
    settings.trip_distance = 0;
    settings.total_distance = 0;
    settings.backlight_level = 100;
#if LEXT_INSTALLED
    // save date and time in case of brownout/powerdown
    settings.time = ERTC->time;
    settings.date = ERTC->date;
#endif
    settings.factory_reset = 0xAA; // mark as factory reset
    eeprom_write_settings();
}

void eeprom_read_settings(void) {
    eeprom_read_bytes(0, 0, (uint8_t *) &settings, sizeof(settings_t));
    uint8_t crc = crc_calc((uint8_t *) &settings, sizeof(settings_t) - 1);
    if (settings.crc != crc || settings.header != 0xCAFEBABE) {
        eeprom_write_defaults();
    } else {
        settings.factory_reset = 0x55;
    }
}

void eeprom_write_settings(void) {
    // calc crc
    settings.header = 0xCAFEBABE;
    settings.crc = crc_calc((uint8_t *) &settings, sizeof(settings_t) - 1);
    eeprom_write_bytes(0, 0, (uint8_t *) &settings, sizeof(settings_t));
}

