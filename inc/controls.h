#ifndef __CONTROLS_H__
#define __CONTROLS_H__

#include "at32f415_gpio.h"

#define POWER_BUTTON_GPIO   GPIOA
#define POWER_BUTTON_PIN    GPIO_PINS_6
#define POWER_BUTTON_PULL   GPIO_PULL_NONE
#define POWER_BUTTON_MODE   GPIO_MODE_INPUT

#define POWER_LATCH_GPIO    GPIOA
#define POWER_LATCH_PIN     GPIO_PINS_4
#define POWER_LATCH_PULL    GPIO_PULL_NONE
#define POWER_LATCH_MODE    GPIO_MODE_OUTPUT

#define VOLTAGE_DETECT_GPIO GPIOA
#define VOLTAGE_DETECT_PIN  GPIO_PINS_5
#define VOLTAGE_DETECT_PULL GPIO_PULL_NONE
#define VOLTAGE_DETECT_MODE GPIO_MODE_ANALOG
#define VOLTAGE_DETECT_ADC_CH    ADC_CHANNEL_5

#define UP_BUTTON_GPIO      GPIOC
#define UP_BUTTON_PIN       GPIO_PINS_2
#define UP_BUTTON_PULL      GPIO_PULL_NONE
#define UP_BUTTON_MODE      GPIO_MODE_INPUT
#define UP_BUTTON_ADC_CH    ADC_CHANNEL_12

#define DOWN_BUTTON_GPIO      GPIOC
#define DOWN_BUTTON_PIN       GPIO_PINS_1
#define DOWN_BUTTON_PULL      GPIO_PULL_NONE
#define DOWN_BUTTON_MODE      GPIO_MODE_ANALOG
#define DOWN_BUTTON_ADC_CH    ADC_CHANNEL_11

#define NC_BUTTON_GPIO      GPIOC
#define NC_BUTTON_PIN       GPIO_PINS_0
#define NC_BUTTON_PULL      GPIO_PULL_NONE
#define NC_BUTTON_MODE      GPIO_MODE_ANALOG
#define NC_BUTTON_ADC_CH    ADC_CHANNEL_10

typedef struct {
    uint16_t temperature_int;
    uint16_t voltage_mcu;
    uint16_t voltage_battery;
    uint16_t temperature_ext; // DOWN button
    uint16_t nc_button; // can be used as light detector?
    uint16_t up_button; // normally not read
    uint16_t power_button; // normally not read
} adc_data_t;

void controls_init(void);
int power_button(void);
int up_button(void);
int down_button(void);
int32_t int_temp(void);
int32_t ext_temp(void);
int32_t NTC_ADC2Temperature(int16_t adc_value);

#endif // __CONTROLS_H__
