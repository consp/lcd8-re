#include <string.h>
#include <stdio.h>
#include "lvgl.h"
#include "stm32h7xx_ll_dma2d.h"
#include "draw/dma2d/lv_draw_dma2d.h"
#include "config.h"
#include "lcd.h"
#include "hal/custom/stm32h743/lcd.h"
#include "uart.h"
#include "cntl.h"

// system is possibly in Os mode
/* #pragma GCC optimize ("O3") */

extern uint8_t framebuffer[];

/* #define MEMORY_DEBUG */
uint16_t dummy = 0;
volatile int touch_flag = 0;
touch_points tp = {0};

#define LCD_TIMING_ADDR_HOLD        1  //1
#define LCD_TIMING_DATA_SETUP       3  //4 
/* #define LCD_TIMING_ADDR_HOLD        15  //1 */
/* #define LCD_TIMING_DATA_SETUP       15  //4  */
#define LCD_TIMING_TURN_AROUND_TIME 0 
#define LCD_CMD_ADDR	*(volatile uint16_t*)(0x60000000)
#define LCD_DATA_ADDR	*(volatile uint16_t*)(0x60020000)

SRAM_HandleTypeDef hsram1;
lv_display_t *remotedisp = NULL;

#ifdef TOUCH_ENABLED
void touch_init(void);
#endif

typedef struct ili_cmd_t {
    uint8_t cmd;
    uint8_t data[16];
    uint32_t data_length;
} ili_cmd;

#define STARTUP_COMMAND_LENGTH_ILI9488 14
const ili_cmd ili_startup_cmds_ili9488[STARTUP_COMMAND_LENGTH_ILI9488] = {
		{ILI_POSITIVE_GAMMA_CORRECTION,     {0x00, 0x09, 0x16, 0x09, 0x17, 0x0A, 0x3F, 0x78, 0x4B, 0x07, 0x0E, 0x0F, 0x18, 0x1A, 0x0F}, 15}, // panel specific
		{ILI_NEGATIVE_GAMMA_CORRECTION,     {0x00, 0x1D, 0x1F, 0x05, 0x0F, 0x05, 0x33, 0x34, 0x43, 0x02, 0x0A, 0x08, 0x2E, 0x33, 0x0F}, 15}, // panel specific
        {ILI_FRAME_RATE_CONTROL_NORMAL,     {0xA0}, 1}, // Fosc,78.13hz
        {ILI_DISPLAY_INVERSION_CONTROL,     {0x00}, 1}, // 2 dot inversion 
        {ILI_POWER_CONTROL_1,               {0x12, 0x12}, 2}, // +/- 4.6895
        {ILI_POWER_CONTROL_2,               {0x41}, 1}, // VCI*6/VCI*4
        {ILI_VCOM_CONTROL_1,                {0x00, 0x25, 0x80}, 3}, // [ignore NV byte] [-1.35938] [read from vcomreg] last byte is NV memory programmed and ignored
		{ILI_DISPLAY_FUNCTION_CONTROL,      {0x02}, 1}, // if correct only sets first byte to non display area AGND 
		{ILI_DISPLAY_FUNCTION_CONTROL,      {0x02, 0x02, 0x3B}, 3},
#if COLOR_SIZE == 2
		{ILI_MEMORY_ACCESS_CONTROL,         {0x48}, 1}, // Column inverted, BGR filters
		{ILI_INTERFACE_PIXEL_FORMAT,        {0x55}, 1}, // RGB565
#else 
		{ILI_MEMORY_ACCESS_CONTROL,         {0x48}, 1}, // Column inverted, BGR filters
		{ILI_INTERFACE_PIXEL_FORMAT,        {0x66}, 1}, // RGB666
#endif
        {ILI_SET_IMAGE_FUNCTION,            {0x00}, 1}, // Disable 24bit mode
        {ILI_ADJUST_CONTROL_3,              {0xA9, 0x51, 0x2C, 0x82}, 4},// default (only one bit can be changed)
		{ILI_WRITE_DISPLAY_BRIGHTNESS,      {0x7F}, 1}
        //

        /* {ILI_POWER_CONTROL_2,               {0x44}, 1}, */
		/* {ILI_VCOM_CONTROL_1,                {0x00, 0x22, 0x80, 0x44}, 4}, */
		/* {ILI_MEMORY_ACCESS_CONTROL,         {(0x20 | 0x08)}, 1}, */
		/* {ILI_INTERFACE_PIXEL_FORMAT,        {0x55}, 1}, */
		/* {ILI_INTERFACE_MODE_CONTROL,        {0x00}, 1}, */
		/* {ILI_FRAME_RATE_CONTROL_NORMAL,     {0xB0, 0x11}, 2}, */
		/* {ILI_DISPLAY_INVERSION_CONTROL,     {0x02}, 1}, */
		/* {ILI_DISPLAY_FUNCTION_CONTROL,      {0x02, 0x02, 0x3B}, 3}, */
		/* {ILI_SET_IMAGE_FUNCTION,            {0x00}, 1}, */
		/* {ILI_WRITE_CTRL_DISPLAY,            {0x28}, 1}, */
		/* {ILI_WRITE_DISPLAY_BRIGHTNESS,      {0x7F}, 1}, */
};
#define STARTUP_COMMAND_LENGTH_ST7796 12
const ili_cmd ili_startup_cmds_st7796[STARTUP_COMMAND_LENGTH_ST7796] = {
        {ST_CSC,                            {0xC3}, 1},
        {ST_CSC,                            {0x96}, 1}, 
#if COLOR_SIZE == 2
		{ILI_MEMORY_ACCESS_CONTROL,         {0x48}, 1}, // Column inverted, BGR filters
		{ILI_INTERFACE_PIXEL_FORMAT,        {0x05}, 1}, // RGB565
#else 
		{ILI_MEMORY_ACCESS_CONTROL,         {0x48}, 1}, // Column inverted, BGR filters
		{ILI_INTERFACE_PIXEL_FORMAT,        {0x06}, 1}, // RGB666
#endif
		{ILI_DISPLAY_FUNCTION_CONTROL,      {0x02, 0x02}, 2},
        {ILI_DISPLAY_BLANKING_PORCH_CONTROL,{0x02, 0x03, 0x00, 0x04}, 4},
        {ILI_DISPLAY_INVERSION_CONTROL,     {0x02}, 1}, // 2 dot inversion 
        {ILI_FRAME_RATE_CONTROL_NORMAL,     {0x80, 0x10}, 2}, // 
        {ST_ENTRY_MODE_SET,                 {0xC6}, 1},
        {ILI_VCOM_CONTROL_1,                {0x24}, 1},  
        /* {ST_DISPLAY_OUTPUT_CTRL_ADJUST,     {0x40, 0x8a, 0x00, 0x00, 0x29, 0x19, 0xa5, 0x33}, 8}, */
        {ILI_POWER_CONTROL_3,               {0x0F}, 1}, // 


		/* {ILI_POSITIVE_GAMMA_CORRECTION,     {0xF0, 0x09, 0x13, 0x12, 0x12, 0x2b, 0x3c, 0x44, 0x4b, 0x1b, 0x18, 0x17, 0x1d, 0x21}, 14}, // panel specific */
		/* {ILI_NEGATIVE_GAMMA_CORRECTION,     {0xf0, 0x09, 0x13, 0x0c, 0x0d, 0x27, 0x3b, 0x44, 0x4d, 0x0b, 0x17, 0x17, 0x1d, 0x21}, 14}, // panel specific */
        /* {ILI_POWER_CONTROL_1,               {0x12, 0x12}, 2}, // +/- 4.6895 */
        /* {ILI_POWER_CONTROL_2,               {0x41}, 1}, // VCI*6/VCI*4 */
        /* {ILI_SET_IMAGE_FUNCTION,            {0x00}, 1}, // Disable 24bit mode */
        /* {ILI_ADJUST_CONTROL_3,              {0xA9, 0x51, 0x2C, 0x82}, 4},// default (only one bit can be changed) */
		{ILI_WRITE_DISPLAY_BRIGHTNESS,      {0xFF}, 1}
        //

        /* {ILI_POWER_CONTROL_2,               {0x44}, 1}, */
		/* {ILI_VCOM_CONTROL_1,                {0x00, 0x22, 0x80, 0x44}, 4}, */
		/* {ILI_MEMORY_ACCESS_CONTROL,         {(0x20 | 0x08)}, 1}, */
		/* {ILI_INTERFACE_PIXEL_FORMAT,        {0x55}, 1}, */
		/* {ILI_INTERFACE_MODE_CONTROL,        {0x00}, 1}, */
		/* {ILI_FRAME_RATE_CONTROL_NORMAL,     {0xB0, 0x11}, 2}, */
		/* {ILI_DISPLAY_INVERSION_CONTROL,     {0x02}, 1}, */
		/* {ILI_DISPLAY_FUNCTION_CONTROL,      {0x02, 0x02, 0x3B}, 3}, */
		/* {ILI_SET_IMAGE_FUNCTION,            {0x00}, 1}, */
		/* {ILI_WRITE_CTRL_DISPLAY,            {0x28}, 1}, */
		/* {ILI_WRITE_DISPLAY_BRIGHTNESS,      {0x7F}, 1}, */
};


static void write_data_8bit(uint8_t data);
static void write_cmd(uint8_t data);
static void write_data(uint16_t data);
static void lcd_set_address_window(uint32_t x, uint32_t y, uint32_t x2, uint32_t y2);

TIM_HandleTypeDef htim1;
void lcd_backlight_init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    __HAL_RCC_TIM1_CLK_ENABLE();

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 12000 - 1; //100hz
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 100;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 50;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
    {
        Error_Handler();
    }
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 0;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.BreakFilter = 0;
    sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
    sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
    sBreakDeadTimeConfig.Break2Filter = 0;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
    {
        Error_Handler();
    }
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM1 GPIO Configuration
    PA11    ------> TIM1_CH4
    */
    GPIO_InitStruct.Pin = LCD_BACKLIGHT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    /* GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; */
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(LCD_BACKLIGHT_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, LCD_BACKLIGHT_Pin, GPIO_PIN_RESET);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

    lcd_backlight(100);
}

static uint32_t FMC_Initialized = 0;
void lcd_sram_init(void) {

    GPIO_InitTypeDef GPIO_InitStruct ={0};
    if (FMC_Initialized) {
        return;
    }
    FMC_Initialized = 1;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();



    HAL_GPIO_WritePin(GPIOB, LCD_CS_MAN_Pin, GPIO_PIN_RESET);
    /*Configure GPIO pins : PWR_EN_Pin CNT_PWR_Pin LCD_CS_MAN_Pin */
    GPIO_InitStruct.Pin = LCD_CS_MAN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, LCD_CS_MAN_Pin, GPIO_PIN_RESET);
    /** Initializes the peripherals clock
    */
    /* PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FMC; */
    /* PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_D1HCLK; */
    /*  */
    /* if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) */
    /* { */
    /*     Error_Handler(); */
    /* } */
    FMC_Initialized = 1;

    /** Initializes the peripherals clock
    */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FMC;
    PeriphClkInitStruct.PLL2.PLL2M = 4;
    PeriphClkInitStruct.PLL2.PLL2N = 225;
    PeriphClkInitStruct.PLL2.PLL2P = 2;
    PeriphClkInitStruct.PLL2.PLL2Q = 2;
    PeriphClkInitStruct.PLL2.PLL2R = 3;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /* Peripheral clock enable */
    __HAL_RCC_FMC_CLK_ENABLE();

    /** FMC GPIO Configuration
        PE7   ------> FMC_D4
        PE8   ------> FMC_D5
        PE9   ------> FMC_D6
        PE10   ------> FMC_D7
        PE11   ------> FMC_D8
        PE12   ------> FMC_D9
        PE13   ------> FMC_D10
        PE14   ------> FMC_D11
        PE15   ------> FMC_D12
        PD8   ------> FMC_D13
        PD9   ------> FMC_D14
        PD10   ------> FMC_D15
        PD11   ------> FMC_A16
        PD14   ------> FMC_D0
        PD15   ------> FMC_D1
        PC7   ------> FMC_NE1
        PD0   ------> FMC_D2
        PD1   ------> FMC_D3
        PD4   ------> FMC_NOE
        PD5   ------> FMC_NWE
  */
    GPIO_InitStruct.Pin = LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin
                            |LCD_D8_Pin|LCD_D9_Pin|LCD_D10_Pin|LCD_D11_Pin
                            |LCD_D12_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_D13_Pin|LCD_D14_Pin|LCD_D15_Pin|LCD_CD_Pin
                            |LCD_D0_Pin|LCD_D1_Pin|LCD_D2_Pin|LCD_D3_Pin
                            |LCD_RD_Pin|LCD_WR_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_CS_FCM_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(LCD_CS_FCM_GPIO_Port, &GPIO_InitStruct);

    FMC_NORSRAM_TimingTypeDef Timing = {0};

    /* USER CODE BEGIN FMC_Init 1 */

    /* USER CODE END FMC_Init 1 */

    /** Perform the SRAM1 memory initialization sequence
    */
    hsram1.Instance = FMC_NORSRAM_DEVICE;
    hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    /* hsram1.Init */
    hsram1.Init.NSBank = FMC_NORSRAM_BANK1;
    hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
    hsram1.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
    hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
    hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
    hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
    hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
    hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
    hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
    hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
    hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
    hsram1.Init.WriteFifo = FMC_WRITE_FIFO_ENABLE;
    hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;
    /* Timing */
    Timing.AddressSetupTime = 0;
    Timing.AddressHoldTime = LCD_TIMING_ADDR_HOLD; // 2
    Timing.DataSetupTime = LCD_TIMING_DATA_SETUP; // 3
    Timing.BusTurnAroundDuration = LCD_TIMING_TURN_AROUND_TIME;
    Timing.CLKDivision = 15;
    Timing.DataLatency = 15;
    Timing.AccessMode = FMC_ACCESS_MODE_A;
    /* ExtTiming */

    if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
    {
        Error_Handler( );
    }
}
#ifndef DIRECT
DMA2D_HandleTypeDef hdma2d;
#endif
void lcd_dma2d_init(void) {
#ifndef DIRECT
    __HAL_RCC_DMA2D_CLK_ENABLE();
    // make sure it's initialized
    hdma2d.Instance = DMA2D;
    hdma2d.Init.Mode = DMA2D_M2M_PFC;
    hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
    hdma2d.Init.OutputOffset = 0;
    hdma2d.LayerCfg[1].InputOffset = 0;
    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputAlpha = 0;
    hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
    hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
    hdma2d.LayerCfg[1].ChromaSubSampling = DMA2D_NO_CSS;
    if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
    {
        Error_Handler();
    }
    // no rtos so disable for now
    __HAL_DMA2D_ENABLE_IT(&hdma2d, DMA2D_IT_TC);
    /*  */
    HAL_NVIC_SetPriority(DMA2D_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2D_IRQn);
#endif
}

MDMA_HandleTypeDef hmdma_mdma_channel1_sw_0;
void lcd_mdma_fcm_init(void) {
    __HAL_RCC_MDMA_CLK_ENABLE();
    /* Local variables */

    /* Configure MDMA channel MDMA_Channel1 */
    /* Configure MDMA request hmdma_mdma_channel1_sw_0 on MDMA_Channel1 */
    hmdma_mdma_channel1_sw_0.Instance = MDMA_Channel1;
    hmdma_mdma_channel1_sw_0.Init.Request = MDMA_REQUEST_SW;
    hmdma_mdma_channel1_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    hmdma_mdma_channel1_sw_0.Init.Priority = MDMA_PRIORITY_LOW;
    hmdma_mdma_channel1_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_mdma_channel1_sw_0.Init.SourceInc = MDMA_SRC_INC_HALFWORD;
    hmdma_mdma_channel1_sw_0.Init.DestinationInc = MDMA_DEST_INC_DISABLE;
    hmdma_mdma_channel1_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_HALFWORD;
    hmdma_mdma_channel1_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_HALFWORD;
    hmdma_mdma_channel1_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_mdma_channel1_sw_0.Init.BufferTransferLength = 6400;
    hmdma_mdma_channel1_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    hmdma_mdma_channel1_sw_0.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    hmdma_mdma_channel1_sw_0.Init.SourceBlockAddressOffset = 0;
    hmdma_mdma_channel1_sw_0.Init.DestBlockAddressOffset = 0;
    __HAL_LINKDMA(&hsram1, hmdma, hmdma_mdma_channel1_sw_0);
    __HAL_MDMA_ENABLE_IT(&hmdma_mdma_channel1_sw_0, MDMA_IT_CTC);
    if (HAL_MDMA_Init(&hmdma_mdma_channel1_sw_0) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_MDMA_DeInit(&hmdma_mdma_channel1_sw_0);
    // enable
    HAL_MDMA_Init(&hmdma_mdma_channel1_sw_0);
    
    /* MDMA interrupt initialization */
    /* MDMA_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(MDMA_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(MDMA_IRQn);
}

volatile int mdmaflag = 0;
uint32_t frames = 0;
volatile uint32_t fpstimeout = 0;
// iff the loop is auto restarted at 480mhz it can be done at 210fps, which is a bit overkill.
CRITICAL void MDMA_IRQHandler(void)
{
    HAL_MDMA_IRQHandler(&hmdma_mdma_channel1_sw_0);
    /* LV_LOG_INFO("MDMA Int"); */
    mdmaflag = 0;
    MDMA_Channel1->CIFCR |= 0x1F;
    frames++;
    /* LV_LOG_INFO("FT: %lums", HAL_GetTick() - fpstimeout); */
    lv_display_flush_ready(remotedisp);
}

CRITICAL void DMA2D_IRQHandler(void) {
#ifndef DIRECT
    HAL_DMA2D_IRQHandler(&hdma2d);
    DMA2D->IFCR |= LL_DMA2D_FLAG_TCIF;
    /* LV_LOG_INFO("DMA2D Int: %08X", DMA2D->ISR); */
    if (DMA2D->ISR & LL_DMA2D_FLAG_CEIF || DMA2D->ISR & LL_DMA2D_FLAG_TEIF) {

        DMA2D->IFCR |= LL_DMA2D_FLAG_CEIF | LL_DMA2D_FLAG_TCIF;
        LV_LOG_INFO("%08lX", DMA2D->NLR);
        LV_LOG_INFO("%08lX %08lX %08lX", DMA2D->FGPFCCR, DMA2D->FGMAR, DMA2D->FGOR);
        LV_LOG_INFO("%08lX %08lX %08lX", DMA2D->OPFCCR, DMA2D->OMAR, DMA2D->OOR);
    }
    if (!mdmaflag && HAL_GetTick() - fpstimeout >= 16) {
        fpstimeout = HAL_GetTick();
        mdmaflag = 1;
        lcd_set_address_window(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
        write_cmd(ILI_MEMORY_WRITE);
        HAL_MDMA_Start_IT(&hmdma_mdma_channel1_sw_0, (uintptr_t) framebuffer, (uintptr_t) 0x60020000, DISPLAY_WIDTH * COLOR_SIZE, DISPLAY_HEIGHT);
    }
    lv_display_flush_ready(remotedisp);
#endif
}

void lcd_init(void) {
    LV_LOG_INFO("LCD Initialization");
    lcd_backlight_init();
    lcd_sram_init();
    lcd_dma2d_init();
    lcd_mdma_fcm_init();

#ifdef TEAR_INT_ENABLED
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = TEAR_INT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(TEAR_INT_GPIO_Port, &GPIO_InitStruct);
#endif
}


CRITICAL void lcd_backlight(int16_t value) { // 0-100
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (value) {
        if (value != htim1.Instance->CCR4) {
            GPIO_InitStruct.Pin = LCD_BACKLIGHT_Pin;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
            HAL_GPIO_Init(LCD_BACKLIGHT_GPIO_Port, &GPIO_InitStruct);
            htim1.Instance->CCR4 = value;
            HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
        }
    } else {
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
        GPIO_InitStruct.Pin = LCD_BACKLIGHT_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
        HAL_GPIO_Init(LCD_BACKLIGHT_GPIO_Port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(GPIOA, LCD_BACKLIGHT_Pin, GPIO_PIN_RESET);
    }
}

static inline void write_cmd(uint8_t command) {
    LCD_CMD_ADDR = (uint8_t) command;
}

static inline void write_data(uint16_t data) {
    LCD_DATA_ADDR = (uint16_t) data;
}

static inline void write_data_8bit(uint8_t data) {
    LCD_DATA_ADDR = data;
}

static inline void read_data_16bit(uint16_t *data) {
    *data = LCD_DATA_ADDR;
}

static inline void read_data_8bit(uint8_t *data) {
    *data = LCD_DATA_ADDR;
}

CRITICAL void read_register(uint8_t command, void *location, uint32_t bits, uint32_t num) {
    write_cmd(command);

    // loosen timings
    FMC_NORSRAM_TimingTypeDef Timing = {0};
    Timing.AddressSetupTime = 15;
    Timing.AddressHoldTime = 15;
    Timing.DataSetupTime = 255;
    Timing.BusTurnAroundDuration = 5;
    Timing.CLKDivision = 16;
    Timing.DataLatency = 17;
    Timing.AccessMode = FMC_ACCESS_MODE_A;
    FMC_NORSRAM_Timing_Init(FMC_NORSRAM_DEVICE, &Timing, FMC_NORSRAM_BANK1);

    uint16_t *location16 = (uint16_t *) location;
    uint8_t *location8 = (uint8_t *) location;
    if (bits) while(num--) { read_data_16bit(location16++); }
    else while(num--) { read_data_8bit(location8++); }
    // restore timings
    Timing.AddressSetupTime = 0; // 0
    Timing.AddressHoldTime = LCD_TIMING_ADDR_HOLD; // 2
    Timing.DataSetupTime = LCD_TIMING_DATA_SETUP; // 3
    Timing.BusTurnAroundDuration = LCD_TIMING_TURN_AROUND_TIME;
    Timing.CLKDivision = 16;
    Timing.DataLatency = 17;
    Timing.AccessMode = FMC_ACCESS_MODE_A;
    FMC_NORSRAM_Timing_Init(FMC_NORSRAM_DEVICE, &Timing, FMC_NORSRAM_BANK1);
}


CRITICAL static void lcd_set_address_window(uint32_t x, uint32_t y, uint32_t x2, uint32_t y2) {
    write_cmd(ILI_COL_ADDR_SET);
    write_data_8bit(x >> 8);
    write_data_8bit(x);
    write_data_8bit(x2 >> 8);
    write_data_8bit(x2);

    write_cmd(ILI_PA_ADDR_SET);
    write_data_8bit(y >> 8);
    write_data_8bit(y);
    write_data_8bit(y2 >> 8);
    write_data_8bit(y2);
}

CRITICAL void lcd_command(uint8_t cmd) {
    write_cmd(cmd);
}

CRITICAL void lcd_draw_data(uint16_t *data, ssize_t length) {
    write_cmd(ILI_MEMORY_WRITE);
    while (length--) write_data(*data++);
}

CRITICAL int detect_lcd(void) {
    // ili9488 has D3 register
    uint8_t data[4] = {0};
    LV_LOG_INFO("Testing ILI9488");
    read_register(0xD3, &data, 0, 4);
    if (data[1] == 0x00 && data[2] == 0x94 && data[3] == 0x88) {
        LV_LOG_INFO("ILI9488 detected");
        return 1;
    }
    read_register(0xD3, &data, 0, 4);
    LV_LOG_INFO("Testing ST7796");
    // st7796
    if (data[1] == 0x00 && data[2] == 0x77 && data[3] == 0x96) {
        LV_LOG_INFO("ST7796 detected");
        return 2;
    }
    LV_LOG_INFO("Not compatible: %02X%02X%02X", data[1], data[2], data[3]);
    return 0;
}

#if COLOR_SIZE == 2
CRITICAL void lcd_fill_pixels(int32_t length, uint16_t color) {
    while(length--) {
        write_data(color);
    }

}
#else
CRITICAL void lcd_fill_pixels(int32_t length, uint32_t color) {
    while(length > 0) {
        write_data((uint16_t) (color >> 8));
        write_data((uint16_t) ((color << 8) | (color >> 16)));
        write_data((uint16_t) (color & 0x0000FFFF));
        length-=2;
    }

}
#endif

#if COLOR_SIZE == 2
CRITICAL void lcd_fill(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint16_t color) {
#else
CRITICAL void lcd_fill(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color) {
#endif
   lcd_set_address_window(x1, y1, x2, y2);
   write_cmd(ILI_MEMORY_WRITE);
   lcd_fill_pixels(((x2-x1) + 1) * ((y2-y1) + 1), color);
}

// does nothing, we do not have a frame buffer
void lcd_update(void) {
}


CRITICAL int lcd_start(void) {
    LV_LOG_INFO("LCD Start of configuration");
    /* write_cmd(0x00); */
    /* write_data(0x0001); */
    /* write_cmd(ILI_SLEEP_OUT); */
    /* HAL_Delay(170); */
    /* HAL_Delay(2); */
    /* write_cmd(0xFF); */
    /* write_cmd(0xaa); */
    /* write_cmd(0x55); */
    /* write_cmd(0xFF); */
    /* write_data(0xffff); */
    /* write_data(0x0000); */
    /* write_data(0xaaaa); */
    /* write_data(0x5555); */
    /* HAL_Delay(2); */
    int dsp = 0;
    if ((dsp = detect_lcd()) == 0) {
        LV_LOG_ERROR("Invalid display detected");
        return 0;
    }

    if (dsp == 1) {
        for (int i = 0; i < STARTUP_COMMAND_LENGTH_ILI9488; i++) {
            write_cmd(ili_startup_cmds_ili9488[i].cmd);
            for (int x = 0; x < ili_startup_cmds_ili9488[i].data_length; x++) {
                write_data_8bit(ili_startup_cmds_ili9488[i].data[x]);
            }
        }
    } else if (dsp == 2) {
        for (int i = 0; i < STARTUP_COMMAND_LENGTH_ST7796; i++) {
            write_cmd(ili_startup_cmds_st7796[i].cmd);
            for (int x = 0; x < ili_startup_cmds_st7796[i].data_length; x++) {
                write_data_8bit(ili_startup_cmds_st7796[i].data[x]);
            }
        }
    }
    write_cmd(ILI_SLEEP_OUT);
    HAL_Delay(10);
    write_cmd(ILI_DISPLAY_ON);
    /* HAL_Delay(100); // 186? */
  
    write_cmd(ILI_DISPLAY_INVERSION_ON);
#ifdef DEBUG
    /* for (int i = 0; i < 480; i++) { */
    /*     lcd_fill(0, i, 320, i+1, RGB(i % 256, i % 256, i % 256)); */
    /* } */
    lcd_fill(0, 0, 320, 160, RGB(255, 0, 0));
    lcd_fill(0, 160, 320, 320, RGB(0, 255, 0));
    lcd_fill(0, 320, 320, 480, RGB(0, 0, 255));

    uint8_t display_info[16];
    read_register(0x0C, &display_info, 0, 2);
    LV_LOG_INFO("Display pixel format: %02X", display_info[1]);
    read_register(0x0E, &display_info, 0, 2);
    LV_LOG_INFO("Display signal mode: %02X", display_info[1]);
    read_register(0x0A, &display_info, 0, 2);
    LV_LOG_INFO("Display power mode: %02X", display_info[1]);
    read_register(0x09, &display_info, 0, 4);
    LV_LOG_INFO("Display voltage booster: %s", ((display_info[1] & 0x80) >> 7) == 1 ? "On" : "Off");
    LV_LOG_INFO("Display row access order: %s", ((display_info[1] & 0x40) >> 6) == 1 ? "Decrement" : "Increment");
    LV_LOG_INFO("Display column access order: %s", ((display_info[1] & 0x20) >> 5) == 1 ? "Decrement" : "Increment");
    LV_LOG_INFO("Display row/column exchange: %s", ((display_info[1] & 0x10) >> 4) == 1 ? "R/C exchange" : "Normal");
    LV_LOG_INFO("Display scan address order: %s", ((display_info[1] & 0x08) >> 3) == 0 ? "Decrement" : "Increment");
    LV_LOG_INFO("Display rgb/bgr: %s", ((display_info[1] & 0x04) >> 2) == 1 ? "BGR" : "RGB");
    LV_LOG_INFO("Display interface color format: %02X", (display_info[2] & 0x70) >> 4);
    LV_LOG_INFO("Display idle mode: %s", ((display_info[2] & 0x08) >> 3) == 1 ? "On" : "Off");
    LV_LOG_INFO("Display partial mode: %s", ((display_info[2] & 0x04) >> 2) == 1 ? "On" : "Off");
    LV_LOG_INFO("Display sleep mode: %s", ((display_info[2] & 0x02) >> 1) == 1 ? "On" : "Off");
    LV_LOG_INFO("Display normal mode: %s", ((display_info[2] & 0x01)) == 1 ? "On" : "Off");
    LV_LOG_INFO("Display vertical scrolling: %s", ((display_info[3] & 0x80) >> 7) == 1 ? "On" : "Off");
    LV_LOG_INFO("Display inversion: %s", ((display_info[3] & 0x20) >> 5) == 1 ? "On" : "Off");
    LV_LOG_INFO("Display display: %s", ((display_info[3] & 0x04) >> 2) == 1 ? "On" : "Off");
    LV_LOG_INFO("Display tearing effect line: %s", ((display_info[3] & 0x02) >> 1) == 1 ? "On" : "Off");
    LV_LOG_INFO("Display gamma curve: %02X", ((display_info[3] & 0x01) << 2) | ((display_info[4] & 0xC0) >> 6));
    LV_LOG_INFO("Display tearing line mode: %s", ((display_info[4] & 0x02) >> 5) == 1 ? "mode 1" : "mode 2");

#else
    lcd_fill(0, 0, 320, 480, RGB(0, 0, 0));
#endif
    return 1;
}

void lcd_test(void) {

}


CRITICAL void lcd_lvgl_flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixmap) {
#ifndef DIRECT
    uint32_t *pixels = (uint32_t *) (pixmap);
    uint16_t length =((area->y2 - area->y1) + 1);
    uint16_t width = ((area->x2 - area->x1) + 1);
    uint32_t fb = (uint32_t) framebuffer;
/* #if PIXEL_BUFFER_SIZE == (DISPLAY_WIDTH * COLOR_SIZE  * DISPLAY_HEIGHT) */
/*     // direct, always draw whole lines */
/*     pixels += (DISPLAY_WIDTH * area->y1); */
/*     lcd_set_address_window(0, area->y1, DISPLAY_WIDTH - 1, area->y2); */
/*     width = DISPLAY_WIDTH; */
/* #else */
/*     lcd_set_address_window(area->x1, area->y1, area->x2, area->y2); */
/* #endif */
    /* LV_LOG_INFO("Starting flush: %3dx%3dx%3dx%3d :: %5d :: %08X", area->x1, area->y1, area->x2, area->y2, length, (uint32_t) pixmap); */
    /* write_cmd(ILI_MEMORY_WRITE); */
    remotedisp = display; // store display for irq
    /* SCB_CleanInvalidateDCache(); // cache cleanup not needed */
    DMA2D->CR = LL_DMA2D_MODE_M2M_PFC | LL_DMA2D_IT_TCIE; // int enable, set m2m pfc mode
    DMA2D->FGMAR = (uint32_t) pixels;
    DMA2D->FGOR = 0;
    DMA2D->FGPFCCR = LL_DMA2D_INPUT_MODE_ARGB8888;
    DMA2D->OPFCCR = LL_DMA2D_OUTPUT_MODE_RGB888 | DMA2D_OPFCCR_RBS | DMA2D_OPFCCR_SB; // Convert endianess by dual swapping
    DMA2D->OMAR = (uint32_t) (fb) + (((area->y1 * 320) + area->x1) * COLOR_SIZE);
    DMA2D->OOR = DISPLAY_WIDTH - width;
    DMA2D->NLR = (width << 16) | (length);
    /* LV_LOG_INFO("%d %d %d %d %08X %08X %08X %08X", area->x1, area->y1, area->x2, area->y2, DMA2D->OMAR, DMA2D->FGMAR, DMA2D->NLR, pixels);  */
    DMA2D->CR |= 1;
    /* HAL_MDMA_PollForTransfer(&hmdma_mdma_channel1_sw_0, HAL_MDMA_FULL_TRANSFER, 100); */
    /* if (display != NULL) lv_display_flush_ready(display); */

    // redraw entire buffer
    /* lcd_set_address_window(area->x1, area->y2, DISPLAY_WIDTH, DISPLAY_HEIGHT); */
    /* write_cmd(ILI_MEMORY_WRITE); */
    /* HAL_MDMA_Start_IT(&hmdma_mdma_channel1_sw_0, (uintptr_t) pixmap, (uintptr_t) 0x60020000, width * COLOR_SIZE, length); */
#else
    // direct, assume format is live, flush cache and draw

    // 18 bit mode results in 24bit transfers so no need to mess around
    // start dma transfer
#ifndef PARTIAL
    if(!mdmaflag) {
        lv_area_t *inv = display->inv_areas;
        int32_t start = DISPLAY_HEIGHT, stop = 0;
        for (int i = 0; i < display->inv_p; i++) {
            if (inv[i].y1 < start) start = inv[i].y1;
            if (inv[i].y2 > stop) stop = inv[i].y2;
        }
        LV_LOG_INFO("INV: %ld %ld", start, stop);
        SCB_CleanInvalidateDCache();
        fpstimeout = HAL_GetTick();
        mdmaflag = 1;
        lcd_set_address_window(0, start, DISPLAY_WIDTH, stop + 1);
        write_cmd(ILI_MEMORY_WRITE);
        HAL_MDMA_Start_IT(&hmdma_mdma_channel1_sw_0, (uintptr_t) &framebuffer[start * DISPLAY_WIDTH * COLOR_SIZE], (uintptr_t) 0x60020000, DISPLAY_WIDTH * COLOR_SIZE, stop - start + 1);
    }
#else
        SCB_CleanInvalidateDCache();
        fpstimeout = HAL_GetTick();
        int32_t pixels = ((area->y2 - area->y1) + 1) * ((area->x2 - area->x1) + 1) * COLOR_SIZE;
        int divider = 512;
        for (; pixels % divider != 0 && divider > 2; divider >>= 1); 
        lcd_set_address_window(area->x1, area->y1, area->x2, area->y2);
        write_cmd(ILI_MEMORY_WRITE);
        /* HAL_MDMA_Start_IT(&hmdma_mdma_channel1_sw_0, (uintptr_t) pixmap, (uintptr_t) 0x60020000, pixels / divider, divider); */
        HAL_MDMA_Start_IT(&hmdma_mdma_channel1_sw_0, (uintptr_t) pixmap, (uintptr_t) 0x60020000, ((area->x2 - area->x1) + 1) * COLOR_SIZE, ((area->y2 - area->y1) + 1));
        /* LV_LOG_INFO("Partial render: %3ldx%3ld to %3ldx%3ld at %08lX %5ld %3ld", area->x1, area->y1, area->x2, area->y2, pixmap, pixels, divider); */
#endif
    
    remotedisp = display;
#endif
}


#if LVGL_VERSION_MAJOR == 9
CRITICAL void lcd_draw_large_text(uint32_t x, uint32_t y, const lv_image_dsc_t *img, lv_color_t clr) {
    uint16_t color = lv_color_to_u16(clr);
    if (img->header.cf == LV_COLOR_FORMAT_I1) {
#else
CRITICAL void lcd_draw_large_text(uint32_t x, uint32_t y, const lv_img_dsc_t *img, lv_color_t color) {
    if (img->header.cf == LV_IMG_CF_INDEXED_1BIT) {
#endif
        // images stored as 1bpp, need to iterate over and draw raw
#ifdef DMA_WRITE
        // items are 148*88, so 37 cycles
#define BLOB_LINES 4 
#define BLOB_SIZE (88*BLOB_LINES)
        uint16_t blob[BLOB_SIZE]; // expand and dma draw
        uint16_t blob2[BLOB_SIZE]; // expand and dma draw
        uint16_t *cb = NULL;
        uint8_t *data = (uint8_t *) img->data;
        data += 8;
        int cycles = 0;
        cb = (cb == blob) ? blob2 : blob ;
        while(cycles < 148 / BLOB_LINES) {
            // expand
            int32_t i = 0;
            while(i < BLOB_SIZE) {
                register uint32_t vdata = *data;
#if LVGL_VERSION_MAJOR == 8
                cb[i++] = (vdata & 0x80) ? 0x0000 : color.full;
                cb[i++] = (vdata & 0x40) ? 0x0000 : color.full;
                cb[i++] = (vdata & 0x20) ? 0x0000 : color.full;
                cb[i++] = (vdata & 0x10) ? 0x0000 : color.full;
                cb[i++] = (vdata & 0x08) ? 0x0000 : color.full;
                cb[i++] = (vdata & 0x04) ? 0x0000 : color.full;
                cb[i++] = (vdata & 0x02) ? 0x0000 : color.full;
                cb[i++] = (vdata & 0x01) ? 0x0000 : color.full;
#else 
                cb[i++] = (vdata & 0x80) ? 0x0000 : color;
                cb[i++] = (vdata & 0x40) ? 0x0000 : color;
                cb[i++] = (vdata & 0x20) ? 0x0000 : color;
                cb[i++] = (vdata & 0x10) ? 0x0000 : color;
                cb[i++] = (vdata & 0x08) ? 0x0000 : color;
                cb[i++] = (vdata & 0x04) ? 0x0000 : color;
                cb[i++] = (vdata & 0x02) ? 0x0000 : color;
                cb[i++] = (vdata & 0x01) ? 0x0000 : color;
#endif
                data++;
            }
            // wait
            while(!dma_ready);
            // set address
            lcd_set_address_window(x, y, x + img->header.w - 1, y + (BLOB_LINES - 1) < (y + img->header.h - 1) ? y + (BLOB_LINES - 1) : (y + img->header.h - 1));
            // send
            write_cmd(ILI_MEMORY_WRITE);
            DATA;
            /* dma_write(cb, BLOB_SIZE, x + img->header.w - 1, y + (BLOB_LINES - 1) < (y + img->header.h - 1) ? y + (BLOB_LINES - 1) : (y + img->header.h - 1)); */
            y += BLOB_LINES;
            cycles++;
            cb = (cb == blob) ? blob2 : blob;
        }
        while(!dma_ready);
#else
        lcd_set_address_window(x, y, x + img->header.w - 1, y + img->header.h - 1);
        uint32_t length = (img->header.w * img->header.h) >> 3;
        uint8_t *data = (uint8_t *) img->data;
        data += 8;
        write_cmd(ILI_MEMORY_WRITE);
        while(length--) {
            // convert
            register uint32_t vdata = *data;
#if LVGL_VERSION_MAJOR == 8
            write_data((vdata & 0x80) ? 0x0000 : color.full);
            write_data((vdata & 0x40) ? 0x0000 : color.full);
            write_data((vdata & 0x20) ? 0x0000 : color.full);
            write_data((vdata & 0x10) ? 0x0000 : color.full);
            write_data((vdata & 0x08) ? 0x0000 : color.full);
            write_data((vdata & 0x04) ? 0x0000 : color.full);
            write_data((vdata & 0x02) ? 0x0000 : color.full);
            write_data((vdata & 0x01) ? 0x0000 : color.full);
#else 
            write_data((vdata & 0x80) ? 0x0000 : color);
            write_data((vdata & 0x40) ? 0x0000 : color);
            write_data((vdata & 0x20) ? 0x0000 : color);
            write_data((vdata & 0x10) ? 0x0000 : color);
            write_data((vdata & 0x08) ? 0x0000 : color);
            write_data((vdata & 0x04) ? 0x0000 : color);
            write_data((vdata & 0x02) ? 0x0000 : color);
            write_data((vdata & 0x01) ? 0x0000 : color);
#endif
            data++;
        }
#endif
    }
}

#ifdef TOUCH_ENABLED

#define CHSC6540_ADDRESS 0x2E << 1
#define CHSC6540_CHANGE_INT 0x5A

uint8_t touch_config[256];

I2C_HandleTypeDef hi2c2;

void touch_write_bytes(uint8_t reg, uint8_t *data, uint32_t length) {
    unsigned int rv = 0;
    if ((rv = HAL_I2C_Master_Transmit(&hi2c2, CHSC6540_ADDRESS, &reg, 1, 50)) != HAL_I2C_ERROR_NONE) {
        LV_LOG_WARN("TOUCH Transmit error: %08X", rv);
        /* return; */
    }
    if ((rv = HAL_I2C_Master_Transmit(&hi2c2, CHSC6540_ADDRESS, data, length, 50)) != HAL_I2C_ERROR_NONE) {
        LV_LOG_WARN("TOUCH Transmit error: %08X", rv);
        return;
    }
}

int touch_read_bytes(uint8_t reg, uint8_t *buffer, uint32_t length) {
    unsigned int rv = 0;
    if ((rv = HAL_I2C_Master_Transmit(&hi2c2, CHSC6540_ADDRESS, &reg, 1, 1000)) != HAL_I2C_ERROR_NONE) {
        LV_LOG_WARN("TOUCH Transmit error: %08X", rv);
        /* return 1; */
    }
    if ((rv = HAL_I2C_Master_Receive(&hi2c2, CHSC6540_ADDRESS, buffer, length, 1000)) != HAL_I2C_ERROR_NONE) {
        LV_LOG_WARN("TOUCH Receive error: %08X", rv);
        return 1;
    }
    return 0;
}

void EXTI15_10_IRQHandler(void) {
    HAL_NVIC_DisableIRQ(TP_INT_EXTI_IRQn);
    HAL_GPIO_EXTI_IRQHandler(TP_INT_Pin);
    touch_flag = 1;
    // int keeps firing until finger released
}

void touch_init(void) {
    LV_LOG_INFO("Touch init");
    
    touch_flag = 0;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = TP_RESET_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(TP_RESET_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(TP_RESET_GPIO_Port, TP_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(TP_RESET_GPIO_Port, TP_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(200);

    GPIO_InitStruct.Pin = TP_INT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(TP_INT_GPIO_Port, &GPIO_InitStruct);

    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
    PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
    /* PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_HSI; */
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();


    /**I2C2 GPIO Configuration
    PB10     ------> I2C2_SCL
    PB11     ------> I2C2_SDA
    */
    GPIO_InitStruct.Pin = TOUCH_SCL_Pin|TOUCH_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;    
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */

    __HAL_RCC_I2C2_CLK_ENABLE();

    hi2c2.Instance = I2C2;
    hi2c2.Init.Timing = 0x307075B1;     // 100k
    /* hi2c2.Init.Timing = 0x50300AFB;  // 10k */
    /* hi2c2.Init.Timing = 0xF010F2FF;     // 2k */
    hi2c2.Init.OwnAddress1 = 3;
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

    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
    {
        Error_Handler();
    }


    // read data
    int err = 1;
    err = HAL_I2C_Mem_Read(&hi2c2, CHSC6540_ADDRESS, 0x00, I2C_MEMADD_SIZE_8BIT, touch_config, 256, 1000); 
    /* err = touch_read_bytes(0x00, touch_config, 0x80);  */
    /* if (!err) err = touch_read_bytes(0x80, &touch_config[0x80], 0x80);  */
    if (err) {
        // disable int
        HAL_NVIC_DisableIRQ(TP_INT_EXTI_IRQn);
        LV_LOG_WARN("TP Disable, no communication possible. install propper pull up resistors");
        led_red(1);
        for (int i = 0; i < 256; i+=4) {
            LV_LOG_INFO("TP: %3d %02X %02X %02X %02X", i, touch_config[i], touch_config[i+1], touch_config[i+2], touch_config[i+3]);
        }
    } else {
        HAL_NVIC_SetPriority(TP_INT_EXTI_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TP_INT_EXTI_IRQn);
        for (int i = 0; i < 256; i+=4) {
            LV_LOG_INFO("TP: %3d %02X %02X %02X %02X", i, touch_config[i], touch_config[i+1], touch_config[i+2], touch_config[i+3]);
        }
    }

}


int touch_get_pos(int16_t *x, int16_t *y, int32_t *presses) {
    touch_points tpl;
    uint8_t *tpl_d = (uint8_t *) &tpl;
    int rv = 0;
    if (touch_flag) {
        touch_flag = 0;
        /* LV_LOG_INFO("Touch read pos"); */
        HAL_I2C_Mem_Read(&hi2c2, CHSC6540_ADDRESS, 0x00, I2C_MEMADD_SIZE_8BIT, tpl_d, sizeof(touch_points), 50);
        tpl.x1 = __ntohs(tpl.x1_raw);
        tpl.y1 = __ntohs(tpl.y1);
        /* LV_LOG_INFO("TP: %02X %02X %02X%02X, %02X%02X", tpl_d[2], tpl_d[3], tpl_d[4], tpl_d[5], tpl_d[6], tpl_d[7]); */
        /* for (int i = 0; i < sizeof(touch_points); i++) LV_LOG_INFO("TPL: %d %02X", i, tpl.data[i]);  */
        /* LV_LOG_INFO("TPL: %02X %04X %04X", tpl.type, tpl.x1, tpl.y1); */
        if (tpl.num == 0 && tp.num == 1 && tpl.x1 == tp.x1 && tpl.y1 == tp.y1) rv = 1; // release (might be triggered twice)
        memcpy(tp.data, tpl.data, sizeof(touch_points)); // press
        LV_LOG_INFO("Touch: %d %d | %3dx%3d", tpl.num, rv, tpl.x1, tpl.y1);
        *x = tpl.x1;
        *y = tpl.y1;
        *presses = 1;
        HAL_NVIC_EnableIRQ(TP_INT_EXTI_IRQn);
    }
    return rv;
}

#endif
