#include "controls.h"
#include "comm.h"
#include "delay.h"
#include "lcd.h"
#include "hal/custom/stm32h743/cntl.h"
#include "stm32h7xx_ll_pwr.h"

#include "config.h"
#include "eeprom.h"
#include "gui.h"
#include "clock.h"
#include "stm32h743.h"
#include "lcd.h"
#include "uart.h"

extern settings_t settings;
extern uint8_t draw_lights_trigger;
typedef struct
{
  __IO uint32_t ISR;   /*!< DMA interrupt status register */
  __IO uint32_t Reserved0;
  __IO uint32_t IFCR;  /*!< DMA interrupt flag clear register */
} DMA_Base_Registers;

extern adc_data_t adc;

typedef struct adc2data_t {
    uint16_t ntc_internal;
    uint16_t photo;
    uint16_t vbus;
} adc2data;

typedef struct adc3data_t {
    union {
        uint16_t ntc_external;
        uint16_t up_button;
    };
    uint16_t internal_temp;
    uint16_t vbat;
    uint16_t vref;
} adc3data;

RAM_D2 adc2data adc2 = {0};
RAM_D2 adc3data adc3 = {0};

extern uint8_t power_button_state ;
extern uint8_t up_button_state ;
extern uint8_t down_button_state ;
extern uint8_t nc_button_state ;
extern uint8_t light_state ;

extern uint32_t power_button_start;
extern uint32_t up_button_start;
extern uint32_t down_button_start;
extern uint32_t nc_button_start;
extern uint32_t button_backoff, button_backoff_start;

int32_t lights_timeout = 0;

int32_t ext_temp_store = 0; // store temperature adc value in case button is pressed

ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc2;
DMA_HandleTypeDef hdma_adc3;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim12;

static CRITICAL inline void measure_buttons(void);
extern uint8_t draw_temperatures_trigger_internal;

CRITICAL int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    int32_t rv = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    if (rv < out_min) rv = out_min;
    else if (rv > out_max) rv = out_max;
    return rv;
}

volatile int adc_update1 = 0;
volatile int adc_update2 = 0;

CRITICAL void DMA1_Stream2_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_adc2);
    if (adc_update1++ >= 10) {
        draw_temperatures_trigger_internal = 1;
        adc_update1 = 0;
        /* LV_LOG_INFO("ADC2: %hu %hu %hu %hu", adc2.photo, adc2.ntc_internal, adc2.vbus, adc2.pwr_button); */
    }
}

/**
  * @brief This function handles DMA1 stream3 global interrupt.
  */
CRITICAL void DMA1_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_adc3);
    if (adc_update2++ >= 10) {
        draw_temperatures_trigger_internal = 1;
        adc_update2 = 0;
        /* LV_LOG_INFO("ADC3: %hu %hu %hu %hu", adc3.up_button, adc3.internal_temp, adc3.vbat, adc3.vref); */
    }
}



/**
 * static functions
 */
static void adc_config(void) {

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_CLKP;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        LV_LOG_ERROR("Cannot init perpheral clock");
        Error_Handler();
    }
    /* Peripheral clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();
    __HAL_RCC_ADC3_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**ADC2 GPIO Configuration
    PA5     ------> ADC2_INP19
    PA6     ------> ADC2_INP3
    PB0     ------> ADC2_INP9
    */
    GPIO_InitStruct.Pin = BTN_UP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BTN_UP_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : BTN_PWR_Pin */
    GPIO_InitStruct.Pin = BTN_PWR_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BTN_PWR_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ADC_VBUS_Pin|ADC_PHOTO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ADC_VBUS_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ADC_NTC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ADC_NTC_GPIO_Port, &GPIO_InitStruct);


    ADC_ChannelConfTypeDef sConfig = {0};

    /** Common config
    */
    hadc2.Instance = ADC2;
    hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV64;
    hadc2.Init.Resolution = ADC_RESOLUTION_16B;
    hadc2.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc2.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    hadc2.Init.LowPowerAutoWait = DISABLE;
    hadc2.Init.ContinuousConvMode = DISABLE;
    hadc2.Init.NbrOfConversion = 3;
    hadc2.Init.DiscontinuousConvMode = DISABLE;
    hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T3_TRGO;
    hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc2.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc2.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
    hadc2.Init.OversamplingMode = DISABLE;
    hadc2.Init.Oversampling.Ratio = 1;
    unsigned int rv = 0;
    if ((rv = HAL_ADC_Init(&hadc2)) != HAL_OK)
    {
        LV_LOG_ERROR("Failed to init adc2: %08X", rv);
        Error_Handler();
    }

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_9;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    sConfig.OffsetSignedSaturation = DISABLE;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
    {
        LV_LOG_ERROR("Failed to init adc2 channel 9");
        Error_Handler();
    }

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_3;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
    {
        LV_LOG_ERROR("Failed to init adc2 channel 3");
        Error_Handler();
    }

    /* #<{(|* Configure Regular Channel */
    /* |)}># */
    /* sConfig.Channel = ADC_CHANNEL_11; */
    /* sConfig.Rank = ADC_REGULAR_RANK_3; */
    /* if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) */
    /* { */
    /*     Error_Handler(); */
    /* } */

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_19;
    sConfig.Rank = ADC_REGULAR_RANK_3;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
    {
        LV_LOG_ERROR("Failed to init adc2 channel 19");
        Error_Handler();
    }

    /* sConfig.Channel = ADC_CHANNEL_19; */
    /* sConfig.Rank = ADC_REGULAR_RANK_4; */
    /* if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) */
    /* { */
    /*     Error_Handler(); */
    /* } */
    /* if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK) */
    /* { */
    /*     Error_Handler(); */
    /* } */
  

    /** Common config
    */
    hadc3.Instance = ADC3;
    hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV64;
    hadc3.Init.Resolution = ADC_RESOLUTION_16B;
    hadc3.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc3.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    hadc3.Init.LowPowerAutoWait = DISABLE;
    hadc3.Init.ContinuousConvMode = DISABLE;
    hadc3.Init.NbrOfConversion = 4;
    hadc3.Init.DiscontinuousConvMode = DISABLE;
    hadc3.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T3_TRGO;
    hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
    hadc3.Init.OversamplingMode = DISABLE;
    hadc3.Init.Oversampling.Ratio = 1;
    if (HAL_ADC_Init(&hadc3) != HAL_OK)
    {
        LV_LOG_ERROR("Failed to init adc3");
        Error_Handler();
    }

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_10;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    sConfig.OffsetSignedSaturation = DISABLE;
    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }


    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_VBAT;
    sConfig.Rank = ADC_REGULAR_RANK_3;
    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_VREFINT;
    sConfig.Rank = ADC_REGULAR_RANK_4;
    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK) */
    /* { */
    /*     Error_Handler(); */
    /* } */
}

static void dma_config(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();
    /* ADC2 DMA Init */
    /* ADC2 Init */
    hdma_adc2.Instance = DMA1_Stream2;
    hdma_adc2.Init.Request = DMA_REQUEST_ADC2;
    hdma_adc2.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc2.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc2.Init.Mode = DMA_CIRCULAR;
    hdma_adc2.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_adc2.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_adc2) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(&hadc2,DMA_Handle,hdma_adc2);

    hdma_adc3.Instance = DMA1_Stream3;
    hdma_adc3.Init.Request = DMA_REQUEST_ADC3;
    hdma_adc3.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc3.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc3.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc3.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc3.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc3.Init.Mode = DMA_CIRCULAR;
    hdma_adc3.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_adc3.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_adc3) != HAL_OK)
    {
      Error_Handler();
    }


    __HAL_LINKDMA(&hadc3,DMA_Handle,hdma_adc3);

    HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

    if (HAL_ADC_Start_DMA(&hadc3, (uint32_t*) &adc3, sizeof(adc3data) / sizeof(uint16_t)) != HAL_OK) {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);

    if (HAL_ADC_Start_DMA(&hadc2, (uint32_t*) &adc2, sizeof(adc2data) / sizeof(uint16_t)) != HAL_OK) {
        Error_Handler();
    }
    /* ADC_STATE_CLR_SET(hadc3.State, */
    /*                       HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC | HAL_ADC_STATE_REG_OVR | HAL_ADC_STATE_REG_EOSMP, */
    /*                       HAL_ADC_STATE_REG_BUSY); */
    /* __HAL_ADC_CLEAR_FLAG(&hadc3, (ADC_FLAG_EOC | ADC_FLAG_EOS | ADC_FLAG_OVR)); */
    /* LL_ADC_REG_SetDataTransferMode(hadc3.Instance, ADC_CFGR_DMACONTREQ((uint32_t)hadc3.Init.ConversionDataManagement)); */
    /* LL_ADC_REG_StartConversion(hadc3.Instance); */
    /* DMA_Base_Registers  *regs_dma  = (DMA_Base_Registers *)hdma_adc3.StreamBaseAddress; */
    /*  */
    /* regs_dma->IFCR = 0x3FUL << (hdma_adc3.StreamIndex & 0x1FU); */
    /* DMA1_Stream3->CR &= (uint32_t)(~DMA_SxCR_DBM); */
    /* DMA1_Stream3->NDTR = 4; */
    /* DMA1_Stream3->PAR = (uint32_t)&hadc3.Instance->DR; */
    /* DMA1_Stream3->M0AR = (uint32_t) &adc3; */
    /*  */
    /* __HAL_DMA_ENABLE_IT(&hdma_adc3, DMA_IT_TC);  */
    /* __HAL_DMA_ENABLE(&hdma_adc3); */

    DMA1_Stream3->CR &= ~(DMA_IT_TE | DMA_IT_HT | DMA_IT_FE | DMA_IT_DME);
    DMA1_Stream2->CR &= ~(DMA_IT_TE | DMA_IT_HT | DMA_IT_FE | DMA_IT_DME);
}

static void tmr_config(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = (SystemCoreClock / 10000) - 1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 500; // 20 times every second 
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);
    __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);

    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    HAL_TIM_Base_Start(&htim3);

    // button measure timer

    /* __HAL_RCC_TIM12_CLK_ENABLE(); */
    /*  */
    /* htim12.Instance = TIM12; */
    /* htim12.Init.Prescaler = (SystemCoreClock / 10000) - 1; */
    /* htim12.Init.CounterMode = TIM_COUNTERMODE_UP; */
    /* htim12.Init.Period = 1000; //10 hz  */
    /* htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; */
    /* htim12.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; */
    /* if (HAL_TIM_Base_Init(&htim12) != HAL_OK) */
    /* { */
    /*     Error_Handler(); */
    /* } */
    /* sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL; */
    /* if (HAL_TIM_ConfigClockSource(&htim12, &sClockSourceConfig) != HAL_OK) */
    /* { */
    /*     Error_Handler(); */
    /* } */
    /* sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE; */
    /* sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET; */
    /* sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE; */
    /* if (HAL_TIMEx_MasterConfigSynchronization(&htim12, &sMasterConfig) != HAL_OK) */
    /* { */
    /*     Error_Handler(); */
    /* } */
    /*  */
    /* __HAL_TIM_CLEAR_FLAG(&htim12, TIM_FLAG_UPDATE); */
    /* __HAL_TIM_ENABLE_IT(&htim12, TIM_IT_UPDATE); */
    /*  */
    /* HAL_NVIC_SetPriority(TIM8_BRK_TIM12_IRQn, 0, 0); */
    /* HAL_NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn); */
    /*  */
    /* HAL_TIM_Base_Start(&htim12); */

}

CRITICAL void TIM3_IRQHandler(void)
{
    __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);
    measure_buttons();
}

static void gpio_config(void) {

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOC, BT_RESET_Pin|BT_FUNC_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, PWR_EN_Pin|CNT_PWR_Pin|LCD_CS_MAN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, LED1_Pin|LED2_Pin|LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED1_Pin LED2_Pin LED3_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin|LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN_M_Pin */
  GPIO_InitStruct.Pin = BTN_M_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BTN_M_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN_DOWN_Pin */
  GPIO_InitStruct.Pin = BTN_DOWN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BTN_DOWN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BT_RESET_Pin BT_FUNC_Pin */
  GPIO_InitStruct.Pin = BT_RESET_Pin|BT_FUNC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PWR_SENSE_Pin */
  GPIO_InitStruct.Pin = PWR_SENSE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PWR_SENSE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PWR_EN_Pin CNT_PWR_Pin LCD_CS_MAN_Pin */
  GPIO_InitStruct.Pin = PWR_EN_Pin|CNT_PWR_Pin|LCD_CS_MAN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*AnalogSwitch Config */
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC2, SYSCFG_SWITCH_PC2_CLOSE);

  led_green(0);
  led_blue(0);
  led_red(0);

}

void pwr_config(void) {
    LL_PWR_SetBattChargResistor(LL_PWR_BATT_CHARGRESISTOR_1_5K);
    LL_PWR_EnableBatteryCharging();
}

/**
 * public functions
 */

void controls_init(void) {
    LV_LOG_INFO("PWR Initialization");
    pwr_config();
    LV_LOG_INFO("ADC Initialization");
    adc_config();
    LV_LOG_INFO("TMR Initialization");
    tmr_config();
    LV_LOG_INFO("DMA Initialization");
    dma_config();
    LV_LOG_INFO("GPIO Initialization");
    gpio_config();
    LV_LOG_INFO("Finished controls");
}

CRITICAL void power_enable(void) {
    HAL_GPIO_WritePin(PWR_EN_GPIO_Port, PWR_EN_Pin, GPIO_PIN_SET); 
    HAL_GPIO_WritePin(CNT_PWR_GPIO_Port, CNT_PWR_Pin, GPIO_PIN_SET);
    led_green(1);
}

CRITICAL void led_blue(int state) {
    if (state) HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
    else HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
}

CRITICAL void led_red(int state) {
    if (state) HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    else HAL_GPIO_WritePin(LED3_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
}

CRITICAL void led_green(int state) {
    if (state) HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    else HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
}

CRITICAL int led_blue_state(void) {
    volatile uint32_t odr = LED1_GPIO_Port->ODR & LED3_Pin;
    return odr;
}

CRITICAL int led_red_state(void) {
    volatile uint32_t odr = LED2_GPIO_Port->ODR & LED1_Pin;
    return odr;
}

CRITICAL int led_green_state(void) {
    volatile uint32_t odr = LED3_GPIO_Port->ODR & LED2_Pin;
    return odr;
}

CRITICAL void power_disable(void) { 
    LV_LOG_WARN("Shutdown triggered");
    eeprom_write_settings();
#if UART_COMM == UART_COMM_VESC
    comm_vesc_packet_send_shutdown();
#endif
    // cap should keep it fed for a while
    led_green(1);
    led_blue(1);
    led_red(1);
    lcd_backlight(80);
    HAL_Delay(100);
    HAL_GPIO_WritePin(PWR_EN_GPIO_Port, PWR_EN_Pin, GPIO_PIN_RESET); 
    /* HAL_PWR_EnterSTANDBYMode(); */
    while(1) {
        // keep feeding the WD, releasing the button will kill the cpu and reset it
        HAL_Delay(10);
    };
}

volatile uint32_t xun = 0;
// the power button does not work properly when vbus is below 5v
static inline int power_button_measure(void) {
    /* return adc2.pwr_button >= 32000 && adc2.vbus >= 6000; // without some bus power the button will not work */
    return (BTN_PWR_GPIO_Port->IDR & (BTN_PWR_Pin)) == 0; // && adc2.vbus >= 6000; // 6000 is about 9v which is the minimum
}

static inline int down_button_measure(void) {
    return (BTN_DOWN_GPIO_Port->IDR & (BTN_DOWN_Pin)) == 0;
}

static inline int up_button_measure(void) {
    return adc3.up_button <= 4000 && adc3.up_button > 0;
}

static inline int nc_button_measure(void) {
    return (BTN_M_GPIO_Port->IDR & (BTN_M_Pin)) == 0;
}

float ntc_calc(uint32_t value, int highside, float rvalue) {
    float temp = 0.0f;
    float rntc = 0.0f;
    if (highside) {
        rntc = ((65535.0f / value) - 1.0f) * rvalue; 
    } else {
        rntc = (value * rvalue) / (65535.0f - value);
    }

    temp = rntc / (float)10000.0f; 
    temp = logf(temp);
    temp /= (float)3950.0f;
    temp += 1.0f / ((float)25.0f + 273.15f);
    temp = 1.0f / temp;
    temp -= 273.15f;
    return temp;
}

int32_t int_temp(void) {
    return (int32_t) (1000 * ntc_calc(adc2.ntc_internal, 1, 10000.0f));
}

int32_t ext_temp(void) {
    if (adc3.ntc_external <= 2000) {
        return ext_temp_store;
    } else {
        return ext_temp_store = (int32_t) (1000 * ntc_calc(adc3.ntc_external, 0, 10000.0f));
    }
}

int32_t light_level(void) {
    /* return adc.nc_button; */
    return (uint16_t) adc2.photo; // raw value
}

void auto_lights() {
    int timeout = 0, send = 0;
    if (settings.lights_mode == LIGHTS_MODE_AUTOMATIC) {
        if (HAL_GetTick() - lights_timeout < LIGHTS_TIMEOUT && lights_timeout != 0) {
            timeout = 1;
        }

        if (light_level() < settings.light_sensitivity && !settings.lights_enabled && !timeout) {
            settings.lights_enabled = 1;
            draw_lights_trigger = 1;
            lights_timeout = HAL_GetTick();
            send = 1;
        } else if (light_level() >= (settings.light_sensitivity + (settings.light_sensitivity >> 2)) && settings.lights_enabled && !timeout) {
            settings.lights_enabled = 0;
            draw_lights_trigger = 1;
            lights_timeout = HAL_GetTick(); 
            send = 1;
        }

        if (!timeout && send) comm_send_lights();
    }
#ifdef LIGHT_SENSOR_ENABLED
    if (settings.backlight_sensitivity > 0) {
        lcd_backlight((int16_t) map((int32_t) light_level(), ((65000 / 10) * (10 - ((int32_t)settings.backlight_sensitivity - 1))), 65535, 35, 100));
    }
#endif
}

static inline void button_switch(int measure(void), uint8_t *state, uint32_t *start, uint32_t now) { 
    if (measure()) { // button is pressed
        if (*state == BUTTON_RELEASED) {
            *start = now;
            *state = BUTTON_DOWN;
        } else if (*state == BUTTON_DOWN && now - *start > BUTTON_LONG_PRESS_TIME) {
            *state = BUTTON_LONG_PRESSED;
        }
    } else {
        if (*state == BUTTON_DOWN && now - *start > BUTTON_DEBOUNCE_TIME) {
            *state = *state == BUTTON_DOWN ? BUTTON_PRESSED : BUTTON_RELEASED;
            *start = 0;
        } else if (*state != BUTTON_PRESSED) {
            *state = BUTTON_RELEASED;
            *start = 0;
        } else if (now - *start > BUTTON_TIMEOUT) {
            *start = 0;
            *state = 0;
        }
    }
}

static CRITICAL inline void measure_buttons(void) {
    // trigger adc
    // button press addition
    volatile uint32_t ctime = HAL_GetTick();
    if (ctime < button_backoff + button_backoff_start) return;
    button_switch(up_button_measure, &up_button_state, &up_button_start, ctime);
    button_switch(down_button_measure, &down_button_state, &down_button_start, ctime);
    button_switch(power_button_measure, &power_button_state, &power_button_start, ctime);
    button_switch(nc_button_measure, &nc_button_state, &nc_button_start, ctime);

#ifdef TOUCH_ENABLED
    int16_t x, y;
    int32_t num;
    int touched = touch_get_pos(&x, &y, &num);
    if (touched) {
#ifdef BT_UART_ENABLED 
        // top left
        if (x < DISPLAY_WIDTH / 2 && y < DISPLAY_HEIGHT / 2) {
            bt_send_cmd(0x01);
        } else if (x > DISPLAY_WIDTH / 2 && y < DISPLAY_HEIGHT / 2) {
            bt_send_cmd(0x04);
        } else if (x < DISPLAY_WIDTH / 2 && y > DISPLAY_HEIGHT / 2) {
            bt_send_cmd(0x02);
        } else if (x > DISPLAY_WIDTH / 2 && y > DISPLAY_HEIGHT / 2) {
            bt_send_cmd(0x03);
        }
#endif
    }
#endif
}

CRITICAL void reset_all_buttons(void) {
    LV_LOG_INFO("Reset all buttons");
    up_button_state = up_button_start = 0;
    down_button_state = down_button_start = 0;
    power_button_state = power_button_start = 0;
    nc_button_state = nc_button_start = 0;
}


int32_t voltage_mcu(void) {
    return (adc3.vref * 1000) / 19859; // ref
}

int32_t voltage_bat(void) { // in mv
    return ((adc3.vbat * 4000) / 19859); // vbat is /4
}

int32_t voltage_ebat(void) { // in mv
    float vbus = (float) adc2.vbus;
    float v = (103300.0f / 65536.0f) * vbus;
    return (int32_t) (v); // in mv
}

int32_t internal_temperature(void) {
    return __HAL_ADC_CALC_TEMPERATURE_TYP_PARAMS(2000, 620, 30, 3300, adc3.internal_temp, LL_ADC_RESOLUTION_16B);
}

