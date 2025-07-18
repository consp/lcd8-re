#include <stdio.h>
#include "eeprom.h"
#include "clock.h"
#include "gui.h"


extern settings_t settings;
volatile uint32_t timer_counter = 0;
volatile uint32_t shutdown_timer = 0;

/***
 * LVGL timer functions
 */
uint32_t timer_cb(void) {
    return HAL_GetTick();
}

RTC_HandleTypeDef hrtc;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim5;

CRITICAL void TIM2_IRQHandler(void)
{
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
    gui_increment_trip();
}


static void clock_rtc_init(void) {

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    LV_LOG_INFO("Initializing RTC");
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Initializes the peripherals clock
    */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();

    /** Initialize RTC Only
    */
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        Error_Handler();
    }

    uint16_t year = 0;
    clock_get_all(NULL, NULL, NULL, &year, NULL, NULL);

    if (year == 0) {
        // reset to something
        clock_set_time(12, 0, 0);
        clock_set_date(70, 1, 1, RTC_WEEKDAY_FRIDAY);
    }
}

static void clock_wheelspeed_init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = ((SystemCoreClock/2) / 10000)-1; // 10000 counts/s e.g. 0.1ms
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);

    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    HAL_TIM_Base_Start(&htim2);

}

void clock_init(void) {
    clock_rtc_init();
    clock_wheelspeed_init();
}

void clock_get_time(uint8_t *hours, uint8_t *minutes) {
    clock_get_all(hours, minutes, NULL, NULL, NULL, NULL);
}


void clock_get_all(uint8_t *hours, uint8_t *minutes, uint8_t *sec, uint16_t *year, uint8_t *mon, uint8_t *day) {
    RTC_TimeTypeDef time = {0};
    HAL_StatusTypeDef rv = 0;
    if ((rv = HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN))) {
        LV_LOG_WARN("Failed to read time from RTC: %08X", rv);        
        return;
    }

    LV_LOG_TRACE("Time is %02d:%02d.%02d %lu", time.Hours, time.Minutes, time.Seconds, time.DayLightSaving);

    RTC_DateTypeDef date = {0};
    if ((rv = HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN))) {
        LV_LOG_WARN("Failed to get date");
        return;
    }

    LV_LOG_TRACE("Date is %02d-%02d-20%02d", date.Date, date.Month, date.Year);
    if (year) *year = date.Year;
    if (mon) *mon = date.Month;
    if (day) *day = date.Date;
    if (hours) *hours = time.Hours;
    if (minutes) *minutes = time.Minutes;
    if (sec) *sec = time.Seconds;
}

CRITICAL void clock_set_wheelspeed_timer(int32_t rpm) {
    if (rpm < 0) rpm = 0;
    if (rpm == 0)  TIM2->ARR = 0;
    else TIM2->ARR = (600000 / rpm) - 1; // 32bit required
}

void clock_set_time(uint32_t hour, uint32_t minute, uint32_t second) {
    RTC_TimeTypeDef sTime = {0};
    sTime.Hours = hour;
    sTime.Minutes = minute;
    sTime.Seconds = second;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        LV_LOG_WARN("Failed to set time");
    }
    LV_LOG_INFO("Time set");
}

void clock_set_date(uint32_t year, uint32_t month, uint32_t day, uint32_t week) {
    RTC_DateTypeDef sDate = {0};

    sDate.WeekDay = week;
    sDate.Month = (uint8_t) month;
    sDate.Date = (uint8_t) day;
    sDate.Year = (uint8_t) year;

    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        LV_LOG_WARN("Failed to set date");
    }
    LV_LOG_INFO("Date set");
}

#ifdef PROFILE
#pragma message("TIM5 enabled as us clock")
void clock_setup_us_source(void) {

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    __HAL_RCC_TIM5_CLK_ENABLE();

    htim5.Instance = TIM5;
    htim5.Init.Prescaler = (SystemCoreClock / 1000000) - 1; // 1000 counts/s e.g. 1ms
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 0xFFFFFFFF; // maximum
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /* __HAL_TIM_CLEAR_FLAG(&htim5, TIM_FLAG_UPDATE); */
    /* __HAL_TIM_ENABLE_IT(&htim5, TIM_IT_UPDATE); */

    /* HAL_NVIC_SetPriority(TIM5_IRQn, 0, 0); */
    /* HAL_NVIC_EnableIRQ(TIM5_IRQn); */

    HAL_TIM_Base_Start(&htim5);

}

CRITICAL uint64_t clock_us(void) { return (uint64_t) TIM5->CNT; };
#endif
