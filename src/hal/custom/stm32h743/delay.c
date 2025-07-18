#include "config.h"
#include "delay.h"

/* delay macros */
#define STEP_DELAY_MS                    50
/* delay variable */

/**
  * @brief  inserts a delay time.
  * @param  nus: specifies the delay time length, in microsecond.
  * @retval none
  */
CRITICAL void delay_10ns(uint32_t ns)
{
}


void delay_init()
{
}

/**
  * @brief  inserts a delay time.
  * @param  nus: specifies the delay time length, in microsecond.
  * @retval none
  */
CRITICAL void delay_us(uint32_t nus)
{
    HAL_Delay(nus/1000);
}

/**
  * @brief  inserts a delay time.
  * @param  nms: specifies the delay time length, in milliseconds.
  * @retval none
  */
CRITICAL void delay_ms(uint16_t nms)
{
    HAL_Delay(nms);
}

/**
  * @brief  inserts a delay time.
  * @param  sec: specifies the delay time, in seconds.
  * @retval none
  */
CRITICAL void delay_sec(uint16_t sec)
{
    HAL_Delay(sec * 1000);
}
