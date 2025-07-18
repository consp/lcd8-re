#ifndef __STM32GH7XX_IT_H__
#define __STM32GH7XX_IT_H__

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void DMA1_Stream0_IRQHandler(void);
void DMA1_Stream1_IRQHandler(void);
void DMA1_Stream2_IRQHandler(void);
void DMA1_Stream3_IRQHandler(void);
void TIM2_IRQHandler(void);
void USART2_IRQHandler(void);
void OTG_HS_IRQHandler(void);
void MDMA_IRQHandler(void);
void BDMA_Channel0_IRQHandler(void);
void BDMA_Channel1_IRQHandler(void);
void BDMA_Channel2_IRQHandler(void);
void LPUART1_IRQHandler(void);
/* USER CODE BEGIN EFP */
#endif // __STM32GH7XX_IT_H__
