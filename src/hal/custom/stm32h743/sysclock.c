#include "stm32h743.h"
#include "stm32h7xx_hal_rcc.h"

static void MPU_Config(void);
static void SystemClock_Config(void);
static void PeriphCommonClock_Config(void);

void system_clock_config(void) {
    // enable all ram clocks
    /* __HAL_RCC_AHBSRAM1_CLK_ENABLE() */
    __HAL_RCC_D2SRAM1_CLK_ENABLE();
    __HAL_RCC_D2SRAM2_CLK_ENABLE();
    MPU_Config();
    SCB_EnableICache();
    SCB_EnableDCache();
    SCB_CleanInvalidateDCache();
    HAL_Init();
    SystemClock_Config();
    PeriphCommonClock_Config();
}
/* void SystemClock_Config(void) */
/* { */
/*   RCC_OscInitTypeDef RCC_OscInitStruct = {0}; */
/*   RCC_ClkInitTypeDef RCC_ClkInitStruct = {0}; */
/*  */
/*   #<{(|* Supply configuration update enable */
/*   |)}># */
/*   HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY); */
/*  */
/*   #<{(|* Configure the main internal regulator output voltage */
/*   |)}># */
/*   __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0); */
/*  */
/*   while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {} */
/*  */
/*   #<{(|* Initializes the RCC Oscillators according to the specified parameters */
/*   * in the RCC_OscInitTypeDef structure. */
/*   |)}># */
/*   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI */
/*                               |RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE; */
/*   RCC_OscInitStruct.HSEState = RCC_HSE_ON; */
/*   RCC_OscInitStruct.HSIState = RCC_HSI_DIV4; */
/*   RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT; */
/*   RCC_OscInitStruct.LSIState = RCC_LSI_ON; */
/*   RCC_OscInitStruct.HSI48State = RCC_HSI48_ON; */
/*   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON; */
/*   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI; */
/*   RCC_OscInitStruct.PLL.PLLM = 1; */
/*   RCC_OscInitStruct.PLL.PLLN = 60; */
/*   RCC_OscInitStruct.PLL.PLLP = 2; */
/*   RCC_OscInitStruct.PLL.PLLQ = 2; */
/*   RCC_OscInitStruct.PLL.PLLR = 2; */
/*   RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3; */
/*   RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE; */
/*   RCC_OscInitStruct.PLL.PLLFRACN = 0; */
/*   if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) */
/*   { */
/*     Error_Handler(); */
/*   } */
/*  */
/*   #<{(|* Initializes the CPU, AHB and APB buses clocks */
/*   |)}># */
/*   RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK */
/*                               |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2 */
/*                               |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1; */
/*   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; */
/*   RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1; */
/*   RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2; */
/*   RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2; */
/*   RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; */
/*   RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; */
/*   RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; */
/*  */
/*   if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) */
/*   { */
/*     Error_Handler(); */
/*   } */
/* } */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Supply configuration update enable
    */
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    /** Configure the main internal regulator output voltage
    */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    /* RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI */
    /*                             |RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE; */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_OFF;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 60; // 60
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                                |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; // 2
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

void PeriphCommonClock_Config(void)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Initializes the peripherals clock
    */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CKPER;
    PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
}

void MPU_Config(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct = {0};

    /* Disables the MPU */
    HAL_MPU_Disable();

    /** Initializes and configures the Region and the memory to be protected
    */
    // FCM CMD
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.BaseAddress = 0x60000000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_32B;
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // FCM DATA
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER1;
    MPU_InitStruct.BaseAddress = 0x60020000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_32B;
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // SRAM1
    MPU_InitStruct.Enable = MPU_REGION_ENABLE; 
    MPU_InitStruct.Number = MPU_REGION_NUMBER2;
    MPU_InitStruct.BaseAddress = 0x24000000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // ITCMRAM
    MPU_InitStruct.Enable = MPU_REGION_DISABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER3;
    MPU_InitStruct.BaseAddress = 0x00000000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // DTCMRAM
    MPU_InitStruct.Enable = MPU_REGION_DISABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER4;
    MPU_InitStruct.BaseAddress = 0x20000000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // SRAM2 and SRAM3
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER5;
    MPU_InitStruct.BaseAddress = 0x30000000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Enables the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

