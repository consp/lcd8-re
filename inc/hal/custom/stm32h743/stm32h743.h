#ifndef __STM32H743_H__
#define __STM32H743_H__
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_conf.h"


void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

void system_clock_config(void);
void Error_Handler(void);

#define TP_INT_EXTI_IRQn EXTI15_10_IRQn

#define BTN_M_Pin GPIO_PIN_2
#define BTN_M_GPIO_Port GPIOC
#define BTN_DOWN_Pin GPIO_PIN_1
#define BTN_DOWN_GPIO_Port GPIOC
#define BTN_UP_Pin GPIO_PIN_0
#define BTN_UP_GPIO_Port GPIOC
#define PWR_SENSE_Pin GPIO_PIN_0
#define PWR_SENSE_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_1
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_2
#define LED2_GPIO_Port GPIOA
#define LED3_Pin GPIO_PIN_3
#define LED3_GPIO_Port GPIOA
#define ADC_VBUS_Pin GPIO_PIN_5
#define ADC_VBUS_GPIO_Port GPIOA
#define ADC_PHOTO_Pin GPIO_PIN_6
#define ADC_PHOTO_GPIO_Port GPIOA
#define ADC_NTC_Pin GPIO_PIN_0
#define ADC_NTC_GPIO_Port GPIOB
#define LCD_D4_Pin GPIO_PIN_7
#define LCD_D4_GPIO_Port GPIOE
#define LCD_D5_Pin GPIO_PIN_8
#define LCD_D5_GPIO_Port GPIOE
#define LCD_D6_Pin GPIO_PIN_9
#define LCD_D6_GPIO_Port GPIOE
#define LCD_D7_Pin GPIO_PIN_10
#define LCD_D7_GPIO_Port GPIOE
#define LCD_D8_Pin GPIO_PIN_11
#define LCD_D8_GPIO_Port GPIOE
#define LCD_D9_Pin GPIO_PIN_12
#define LCD_D9_GPIO_Port GPIOE
#define LCD_D10_Pin GPIO_PIN_13
#define LCD_D10_GPIO_Port GPIOE
#define LCD_D11_Pin GPIO_PIN_14
#define LCD_D11_GPIO_Port GPIOE
#define LCD_D12_Pin GPIO_PIN_15
#define LCD_D12_GPIO_Port GPIOE
#define TOUCH_SCL_Pin GPIO_PIN_10
#define TOUCH_SCL_GPIO_Port GPIOB
#define TOUCH_SDA_Pin GPIO_PIN_11
#define TOUCH_SDA_GPIO_Port GPIOB
#define TP_RESET_Pin GPIO_PIN_12
#define TP_RESET_GPIO_Port GPIOB
#define TP_INT_Pin GPIO_PIN_13
#define TP_INT_GPIO_Port GPIOB
#define LCD_D13_Pin GPIO_PIN_8
#define LCD_D13_GPIO_Port GPIOD
#define LCD_D14_Pin GPIO_PIN_9
#define LCD_D14_GPIO_Port GPIOD
#define LCD_D15_Pin GPIO_PIN_10
#define LCD_D15_GPIO_Port GPIOD
#define LCD_CD_Pin GPIO_PIN_11
#define LCD_CD_GPIO_Port GPIOD
#define EEPROM_SCL_Pin GPIO_PIN_12
#define EEPROM_SCL_GPIO_Port GPIOD
#define EEPROM_SDA_Pin GPIO_PIN_13
#define EEPROM_SDA_GPIO_Port GPIOD
#define LCD_D0_Pin GPIO_PIN_14
#define LCD_D0_GPIO_Port GPIOD
#define LCD_D1_Pin GPIO_PIN_15
#define LCD_D1_GPIO_Port GPIOD
#define BT_UART_TX_Pin GPIO_PIN_6
#define BT_UART_TX_GPIO_Port GPIOC
#define BT_UART_RX_Pin GPIO_PIN_7
#define BT_UART_RX_GPIO_Port GPIOC
#define BT_RESET_Pin GPIO_PIN_8
#define BT_RESET_GPIO_Port GPIOC
#define BT_FUNC_Pin GPIO_PIN_9
#define BT_FUNC_GPIO_Port GPIOC
#define LCD_BACKLIGHT_Pin GPIO_PIN_11
#define LCD_BACKLIGHT_GPIO_Port GPIOA
#define LCD_D2_Pin GPIO_PIN_0
#define LCD_D2_GPIO_Port GPIOD
#define LCD_D3_Pin GPIO_PIN_1
#define LCD_D3_GPIO_Port GPIOD
#define LCD_RD_Pin GPIO_PIN_4
#define LCD_RD_GPIO_Port GPIOD
#define LCD_WR_Pin GPIO_PIN_5
#define LCD_WR_GPIO_Port GPIOD
#define TEAR_INT_Pin GPIO_PIN_6
#define TEAR_INT_GPIO_Port GPIOD
#define LCD_CS_FCM_Pin GPIO_PIN_7
#define LCD_CS_FCM_GPIO_Port GPIOD
#define LCD_CS_MAN_Pin GPIO_PIN_4
#define LCD_CS_MAN_GPIO_Port GPIOB
#define BTN_PWR_Pin GPIO_PIN_7
#define BTN_PWR_GPIO_Port GPIOB
#define CNT_PWR_Pin GPIO_PIN_8
#define CNT_PWR_GPIO_Port GPIOB
#define PWR_EN_Pin GPIO_PIN_9
#define PWR_EN_GPIO_Port GPIOB
#define CONTROLLER_UART_RX_Pin GPIO_PIN_0
#define CONTROLLER_UART_RX_GPIO_Port GPIOE
#define CONTROLLER_UART_TX_Pin GPIO_PIN_1
#define CONTROLLER_UART_TX_GPIO_Port GPIOE
#endif // __STM32H743_H__
