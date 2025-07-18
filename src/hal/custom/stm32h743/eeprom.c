/*
 * Read/write to eeprom via i2c bitbang
 */
#include <string.h>
#include "delay.h"
#include "config.h"
#include "eeprom.h"
#include "crc.h"
#include "lvgl.h"

settings_t settings;
settings_t settings_backup;
I2C_HandleTypeDef hi2c2;

#define ADDR(x) (0xA0 | ((x & 0x7) << 1))

static inline void i2c_start(void) {
    // only one device on bus, no arbitration needed
}

static void i2c_initialize(void) {

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Initializes the peripherals clock
    */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
    PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C2 GPIO Configuration
    PB10     ------> I2C2_SCL
    PB11     ------> I2C2_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_I2C2_CLK_ENABLE();

    // setup port
    //
    hi2c2.Instance = I2C2;
    hi2c2.Init.Timing = 0x307075B1;
    hi2c2.Init.OwnAddress1 = 1;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c2) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Analogue filter
    */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Digital filter
    */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
    {
        Error_Handler();
    }
}


void eeprom_write_bytes(uint8_t address, uint8_t page, uint8_t *data, uint32_t length) {
    unsigned int rv = 0;
    uint8_t databuffer[9] = {0};
    while(length) {
        databuffer[0] = address;
        memcpy(&databuffer[1], data, length > 8 ? 8 : length);
        if ((rv = HAL_I2C_Master_Transmit(&hi2c2, ADDR(page), databuffer, length > 8 ? 9 : length + 1, 1000)) != HAL_I2C_ERROR_NONE) {
            LV_LOG_WARN("EEPROM Transmit error: %08X", rv);
            return;
        }
        length -= length > 8 ? 8 : length;
        address += 8;
        data += 8;
        HAL_Delay(5);
    }
}

void eeprom_write_byte(uint8_t address, uint8_t byte, uint8_t page) {
    eeprom_write_bytes(address, page, &byte, 1);
}

uint8_t eeprom_read_byte(uint8_t byte, uint8_t page) {
    uint8_t result = 0;
    unsigned int rv = 0;
    if ((rv = HAL_I2C_Master_Transmit(&hi2c2, ADDR(page), &byte, 1, 1000)) != HAL_I2C_ERROR_NONE) {
        LV_LOG_WARN("EEPROM Transmit error: %08X", rv);
        return result;
    }
    if ((rv = HAL_I2C_Master_Receive(&hi2c2, ADDR(page), &result, 1, 1000)) != HAL_I2C_ERROR_NONE) {
        LV_LOG_WARN("EEPROM Receive error: %08X", rv);
    }
    return result;
}

uint8_t eeprom_read_current_byte(uint8_t page) {
    uint8_t buffer = 0;
    unsigned int rv = 0;
    if ((rv = HAL_I2C_Master_Receive(&hi2c2, ADDR(page), &buffer, 1, 1000)) != HAL_I2C_ERROR_NONE) {
        LV_LOG_WARN("EEPROM Receive error: %08X", rv);
    }
    /* i2c_transfer(0x50 | ((page & 0x07)), &buffer, 1, I2C_READ, 0); */
    return buffer;
}

int eeprom_read_bytes(uint8_t address, uint8_t page, uint8_t *buffer, uint32_t length) {
    unsigned int rv = 0;
    if ((rv = HAL_I2C_Master_Transmit(&hi2c2, ADDR(page), &address, 1, 1000)) != HAL_I2C_ERROR_NONE) {
        LV_LOG_WARN("EEPROM Transmit error: %08X", rv);
        return 1;
    }
    if ((rv = HAL_I2C_Master_Receive(&hi2c2, ADDR(page), buffer, length, 1000)) != HAL_I2C_ERROR_NONE) {
        LV_LOG_WARN("EEPROM Receive error: %08X", rv);
        return 1;
    }
    return 0;
}

void eeprom_init(void) {
    LV_LOG_INFO("EEPROM Initialize");
    i2c_initialize();

    // read settings from eeprom
    eeprom_read_settings();
}

void eeprom_write_defaults(void) {
    settings.power_max = POWER_MAX;
    settings.power_redline = POWER_REDLINE;
    settings.power_min = POWER_MIN;
    settings.speed_max = SPEED_MAX;
    settings.current_max = 25000;
    settings.battery_voltage_min = BATTERY_MIN;
    settings.battery_voltage_max = BATTERY_MAX;
    settings.graph_duration = GRAPH_DURATION;
    settings.graph_field = GRAPH_FIELD_SPEED_POWER_AVG;
    settings.graph_max = GRAPH_MAX;
    settings.assist_levels = ASSIST_LEVELS;
    settings.assist_last = ASSIST_DEFAULT;
    settings.wheel_circumfence = WHEEL_CIRCUMFENCE;
    settings.speed_redline = SPEED_REDLINE;
    settings.speed_assist_max = SPEED_MAX_ASSIST;
    settings.lights_enabled = 0;
    settings.lights_mode = 0;
    settings.battery_voltage_from_controller = 0;
    settings.regen_current = REGEN_CURRENT;
    settings.pas_timeout = PAS_TIMEOUT;
    settings.pas_ramp = PAS_RAMP;
    settings.shutdown_timer = SHUTDOWN_TIMER_DEFAULT;

    settings.trip_time = 0;
    settings.trip_distance = 0;
    settings.total_distance = 0;
    settings.backlight_level = 100;
#if LEXT_INSTALLED
    // save date and time in case of brownout/powerdown
    settings.time = 0; 
    settings.date = 0; 
#endif

    settings.light_sensitivity = 2700;
    settings.factory_reset = 0xAA; // mark as defaults loaded 
}

void eeprom_print_values(void) {
    LV_LOG_INFO("------------- EEPROM SETTINGS ---------------");
    LV_LOG_INFO("Header:                    %08lX",  settings.header);
    LV_LOG_INFO("Factory settings:          %02X", settings.factory_reset);
    LV_LOG_INFO("---------------------------------------------");
    LV_LOG_INFO("Trip time:                 %lu", settings.trip_time);
    LV_LOG_INFO("Trip distance:             %lu", settings.trip_distance);
    LV_LOG_INFO("Total distance:            %lu", settings.total_distance);
    LV_LOG_INFO("---------------------------------------------");
    LV_LOG_INFO("Battery voltage min:       %hu", settings.battery_voltage_min);
    LV_LOG_INFO("Battery voltage max:       %hu", settings.battery_voltage_max);
    LV_LOG_INFO("Graph duration:            %u",  settings.graph_duration);
    LV_LOG_INFO("Graph shift:               %u",  settings.graph_field);
    LV_LOG_INFO("Graph max:                 %u",  settings.graph_max);
    LV_LOG_INFO("Assist levels:             %u",  settings.assist_levels);
    LV_LOG_INFO("Assist last:               %u",  settings.assist_last);
    LV_LOG_INFO("Speed max:                 %u",  settings.speed_max);
    LV_LOG_INFO("Speed redline:             %u",  settings.speed_redline);
    LV_LOG_INFO("Power min:                 %hd", settings.power_min);
    LV_LOG_INFO("Power redline:             %hd", settings.power_redline);
    LV_LOG_INFO("Power max:                 %hd", settings.power_max);
    LV_LOG_INFO("Battery volt from cont:    %u",  settings.battery_voltage_from_controller);
    LV_LOG_INFO("Backlight level:           %u",  settings.backlight_level);
    LV_LOG_INFO("---------------------------------------------");
}

volatile int readfail = 0;
volatile uint8_t crc = 0;

void eeprom_read_settings(void) {
    LV_LOG_INFO("Reading settings from EEPROM");
    int counter = 0;
    while(counter < 3) {
        if (!eeprom_read_bytes(0, 0, (uint8_t *) &settings, sizeof(settings_t))) {
            crc = crc_calc((uint8_t *) &settings, sizeof(settings_t) - 1);
            if (settings.crc != crc || settings.header != 0xCAFEBABE) {
                // attempt reading backup
                LV_LOG_WARN("CRC or header fail: %02X vs %02X and %08X vs %08X", crc, settings.crc, 0xCAFEBABE, (unsigned int) settings.header);
                LV_LOG_WARN("Reading backup");
                if (!eeprom_read_bytes(0, 1, (uint8_t *) &settings_backup, sizeof(settings_t))) {
                    if (memcmp(&settings, &settings_backup, sizeof(settings_t)) == 0) {
                        eeprom_factory_reset();
                    } else {
                        // main possibly broken
                        crc = crc_calc((uint8_t *) &settings_backup, sizeof(settings_t) - 1);
                        if (settings_backup.crc == crc || settings_backup.header == 0xCAFEBABE) {
                            memcpy(&settings, &settings_backup, sizeof(settings_t));
                        } else {
                            eeprom_factory_reset();
                        }
                    }
                } else {
                    LV_LOG_WARN("Reading backup failed");
                }
                readfail++;
                delay_ms(100);
            } else {
                LV_LOG_INFO("EEPROM settings read");
                break;
            }
        }
        counter++;
    }
    eeprom_print_values();
}


void eeprom_factory_reset(void) {
    eeprom_write_defaults();
    settings.factory_reset = 0xAA;
    eeprom_write_settings();
}


void eeprom_write_settings(void) {
    // calc crc
    LV_LOG_INFO("Writing settings to EEPROM");
    settings.header = 0xCAFEBABE;
    settings.crc = crc_calc((uint8_t *) &settings, sizeof(settings_t) - 1);
    eeprom_write_bytes(0, 0, (uint8_t *) &settings, sizeof(settings_t));
    // check result
    uint8_t tmp[sizeof(settings_t)];

    if (eeprom_read_bytes(0, 0, tmp, sizeof(settings_t))) {
        LV_LOG_WARN("Failed to read back written settings, aborting");
        return;
    }
    if (memcmp(tmp, &settings, sizeof(settings_t)) != 0) {
        LV_LOG_WARN("Written settings do not match, aborting");
        return;
    }
    LV_LOG_INFO("Writing backup");
    eeprom_write_bytes(0, 1, (uint8_t *) &settings, sizeof(settings_t));
    HAL_Delay(5);
}

