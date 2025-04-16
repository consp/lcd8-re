#include "at32f415_gpio.h"
#include "controls.h"

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
#define UP_BUTTON_PIN       GPIO_PINS_1
#define UP_BUTTON_PULL      GPIO_PULL_NONE
#define UP_BUTTON_MODE      GPIO_MODE_ANALOG
#define UP_BUTTON_ADC_CH    ADC_CHANNEL_11

#define DOWN_BUTTON_GPIO      GPIOC
#define DOWN_BUTTON_PIN       GPIO_PINS_2
#define DOWN_BUTTON_PULL      GPIO_PULL_NONE
#define DOWN_BUTTON_MODE      GPIO_MODE_INPUT 
#define DOWN_BUTTON_ADC_CH    ADC_CHANNEL_12

#define NC_BUTTON_GPIO      GPIOC
#define NC_BUTTON_PIN       GPIO_PINS_0
#define NC_BUTTON_PULL      GPIO_PULL_NONE
#define NC_BUTTON_MODE      GPIO_MODE_OUTPUT 
#define NC_BUTTON_ADC_CH    ADC_CHANNEL_10

#define EXT_TEMP_ADC_CH UP_BUTTON_ADC_CH
int power_button_press(void);
int up_button_press(void);
int down_button_press(void);
int nc_button_press(void);
int power_button_hold(void);
int up_button_hold(void);
int down_button_hold(void);
int nc_button_hold(void);
int32_t int_temp(void);
int32_t ext_temp(void);
int32_t NTC_ADC2Temperature(int16_t adc_value);

int32_t voltage_ebat(void);
int32_t voltage_mcu(void);

void power_enable(void);
void power_disable(void);

