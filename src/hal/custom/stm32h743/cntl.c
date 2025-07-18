#include "controls.h"
#include "comm.h"
#include "delay.h"
#include "lcd.h"
#include "hal/custom/stm32h743/cntl.h"

#include "config.h"
#include "eeprom.h"
#include "gui.h"
#include "stm32h743.h"

extern settings_t settings;
typedef struct
{
  __IO uint32_t ISR;   /*!< DMA interrupt status register */
  __IO uint32_t Reserved0;
  __IO uint32_t IFCR;  /*!< DMA interrupt flag clear register */
} DMA_Base_Registers;

extern adc_data_t adc;

typedef struct adc2data_t {
    uint16_t photo;
    uint16_t ntc_internal;
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

int32_t ext_temp_store = 0; // store temperature adc value in case button is pressed

ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc2;
DMA_HandleTypeDef hdma_adc3;
TIM_HandleTypeDef htim3;

extern uint8_t draw_temperatures_trigger_internal;

void DMA1_Stream2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_adc2);
  draw_temperatures_trigger_internal = 1;
}

/**
  * @brief This function handles DMA1 stream3 global interrupt.
  */
void DMA1_Stream3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_adc3);
  draw_temperatures_trigger_internal = 1;
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
      Error_Handler();
    }
    /* Peripheral clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();
    __HAL_RCC_ADC3_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC2 GPIO Configuration
    PC1     ------> ADC2_INP11
    PA5     ------> ADC2_INP19
    PA6     ------> ADC2_INP3
    PB0     ------> ADC2_INP9
    */
    GPIO_InitStruct.Pin = BTN_UP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BTN_UP_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ADC_VBUS_Pin|ADC_PHOTO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
    sConfig.Channel = ADC_CHANNEL_3;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    sConfig.OffsetSignedSaturation = DISABLE;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_9;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
    {
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
        Error_Handler();
    }
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
        Error_Handler();
    }

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_0;
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

    if (HAL_ADC_Start_DMA(&hadc3, (uint32_t*) &adc3, 4) != HAL_OK) {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);

    if (HAL_ADC_Start_DMA(&hadc2, (uint32_t*) &adc2, 3) != HAL_OK) {
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
    htim3.Init.Prescaler = 24000-1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 10000; // once every second
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
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

    HAL_TIM_Base_Start(&htim3);

}

static void gpio_config(void) {

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, BT_RESET_Pin|BT_FUNC_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, PWR_EN_Pin|CNT_PWR_Pin|LCD_CS_MAN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, LED1_Pin|GPIO_PIN_3|LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED1_Pin LED2_Pin LED3_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin|LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

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
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN_PWR_Pin */
  GPIO_InitStruct.Pin = BTN_PWR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BTN_PWR_GPIO_Port, &GPIO_InitStruct);

  /*AnalogSwitch Config */
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA1, SYSCFG_SWITCH_PA1_CLOSE);


  /*AnalogSwitch Config */
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA1, SYSCFG_SWITCH_PA1_CLOSE);
}

/**
 * public functions
 */

void controls_init(void) {
    LV_LOG_INFO("ADC Initialization");
    adc_config();
    LV_LOG_INFO("TMR Initialization");
    tmr_config();
    LV_LOG_INFO("DMA Initialization");
    dma_config();
    LV_LOG_INFO("GPIO Initialization");
    gpio_config();
}

CRITICAL void power_enable(void) {
    HAL_GPIO_WritePin(PWR_EN_GPIO_Port, PWR_EN_Pin, GPIO_PIN_SET); 
    HAL_GPIO_WritePin(CNT_PWR_GPIO_Port, CNT_PWR_Pin, GPIO_PIN_SET); 
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET); 
}

CRITICAL void power_disable(void) { 
    lcd_backlight(0); // "show off"
#if UART_COMM == UART_COMM_VESC
    comm_vesc_packet_send_shutdown();
#endif
    // cap should keep it fed for a while
}

volatile uint32_t xun = 0;
static inline int power_button_measure(void) {
    return (BTN_PWR_GPIO_Port->IDR & (BTN_PWR_Pin)) == 0;
}

static inline int down_button_measure(void) {
    return (BTN_DOWN_GPIO_Port->IDR & (BTN_DOWN_Pin)) == 0;
}

static inline int up_button_measure(void) {
    return adc3.up_button <= 1000;
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
    // data from artery example code converted to return 24.8s fixed point format
    return (int32_t) (256 * ntc_calc(adc2.ntc_internal, 1, 33000.0f));
}

int32_t ext_temp(void) {
    // result is in 24.8s format
    // value is stored for when button is pressed as it pulls to ground
    if (adc.temperature_ext <= 10) {
        return ext_temp_store;
    } else {
        return ext_temp_store = (int32_t) (256 * ntc_calc(adc3.ntc_external, 0, 10000.0f));
    }
}

int32_t light_level(void) {
    /* return adc.nc_button; */
    return (uint16_t) adc2.photo; // raw value
}

void auto_lights() {
    if (settings.lights_mode == LIGHTS_MODE_AUTOMATIC) {
        if (light_level() > settings.light_sensitivity && !settings.lights_enabled) {
            settings.lights_enabled = 1;
            comm_send_display_status();
        } else if (light_level() < settings.light_sensitivity && settings.lights_enabled) {
            settings.lights_enabled = 0;
            comm_send_display_status();
        }
    }
}

static inline void measure_buttons(void) {
    // trigger adc
    /* adc_ordinary_software_trigger_enable(ADC1, TRUE); */
    // button press addition
    if (HAL_GetTick() < button_backoff + button_backoff_start) return;
    if (up_button_measure()) {
        if (up_button_state == BUTTON_RELEASED) {
            up_button_start = HAL_GetTick();
            up_button_state = BUTTON_DOWN;
        } else if (up_button_state == BUTTON_DOWN && HAL_GetTick() - up_button_start > BUTTON_LONG_PRESS_TIME) {
            up_button_state = BUTTON_LONG_PRESSED;
        }
    } else {
        if (up_button_state == BUTTON_DOWN && HAL_GetTick() - up_button_start > BUTTON_DEBOUNCE_TIME) {
            up_button_state = up_button_state == BUTTON_DOWN ? BUTTON_PRESSED : BUTTON_RELEASED;
            up_button_start = 0;
        } else if (up_button_state != BUTTON_PRESSED) {
            up_button_state = BUTTON_RELEASED;
            up_button_start = 0;
        }
    }

    if (down_button_measure()) {
        if (down_button_state == BUTTON_RELEASED) {
            down_button_start = HAL_GetTick();
            down_button_state = BUTTON_DOWN;
        } else if (down_button_state == BUTTON_DOWN && HAL_GetTick() - down_button_start > BUTTON_LONG_PRESS_TIME) {
            down_button_state = BUTTON_LONG_PRESSED;
        }
    } else {
        if (down_button_state == BUTTON_DOWN && HAL_GetTick() - down_button_start > BUTTON_DEBOUNCE_TIME) {
            down_button_state = down_button_state == BUTTON_DOWN ? BUTTON_PRESSED : BUTTON_RELEASED;
            down_button_start = 0;
        } else if (down_button_state != BUTTON_PRESSED) {
            down_button_state = BUTTON_RELEASED;
            down_button_start = 0;
        }
    }

    if (power_button_measure()) {
        if (power_button_state == BUTTON_RELEASED) {
            power_button_start = HAL_GetTick();
            power_button_state = BUTTON_DOWN;
        } else if (power_button_state == BUTTON_DOWN && HAL_GetTick() - power_button_start > BUTTON_LONG_PRESS_TIME) {
            power_button_state = BUTTON_LONG_PRESSED;
        }
    } else {
        if (power_button_state == BUTTON_DOWN && HAL_GetTick() - power_button_start > BUTTON_DEBOUNCE_TIME) {
            power_button_state = power_button_state == BUTTON_DOWN ? BUTTON_PRESSED : BUTTON_RELEASED;
            power_button_start = 0;
        } else if (power_button_state != BUTTON_PRESSED) {
            power_button_state = BUTTON_RELEASED;
            power_button_start = 0;
        }
    }
    if (nc_button_measure()) {
        if (nc_button_state == BUTTON_RELEASED) {
            nc_button_start = HAL_GetTick();
            nc_button_state = BUTTON_DOWN;
        } else if (HAL_GetTick() - nc_button_start > BUTTON_LONG_PRESS_TIME) {
            nc_button_state = BUTTON_LONG_PRESSED;
        }
    } else {
        if (nc_button_state == BUTTON_DOWN && HAL_GetTick() - nc_button_start > BUTTON_DEBOUNCE_TIME) {
            nc_button_state = nc_button_state == BUTTON_DOWN ? BUTTON_PRESSED : BUTTON_RELEASED;
            nc_button_start = 0;
        } else if (nc_button_state != BUTTON_PRESSED) {
            nc_button_state = BUTTON_RELEASED;
            nc_button_start = 0;
        }
    }
}


int32_t voltage_mcu(void) {
    return (adc3.vref << 8) / 19859;
}

int32_t voltage_bat(void) {
    return (adc3.vbat << 8) / 19859;
}

int32_t voltage_ebat(void) { // in mv
    /* extern int32_t battery_voltage; */
    /* return battery_voltage; */
    return (adc2.vbus << 15) / 81205;
}

