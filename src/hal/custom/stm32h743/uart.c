#include <string.h>
#include <machine/endian.h>
#include "uart.h"
#include "lvgl.h"
#include "lv_conf.h"
#include "crc.h"
#include "delay.h"
#include "eeprom.h"
#include "stm32h743.h"
#include "stm32h7xx_hal_usart.h"
#include "stm32h7xx_ll_usart.h"
#include "stm32h7xx_ll_rcc.h"
#include "gui.h"
#include "clock.h"
#include "cntl.h"

/*
 * lpusart 1 is used for the buetooth module, usart6 for controller comms if no can is used
 * lpusart1 uses bdma and d3 memory (sram3)
 */
#undef UART_RX_BUFFER_SIZE
#undef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE (16U * 1024U)
#define UART_RX_BUFFER_SIZE (16U * 1024U)

#define CNT_BAUD 115200
#define BT_BAUD 230400

uint8_t RAM_D2 cnt_rx_buffer[UART_RX_BUFFER_SIZE] __attribute__ ((aligned (UART_RX_BUFFER_SIZE)));
uint8_t RAM_D2 cnt_tx_buffer[UART_TX_BUFFER_SIZE] __attribute__ ((aligned (UART_RX_BUFFER_SIZE)));
uint8_t RAM_D2  bt_rx_buffer[UART_RX_BUFFER_SIZE] __attribute__ ((aligned (UART_TX_BUFFER_SIZE)));
uint8_t RAM_D2  bt_tx_buffer[UART_TX_BUFFER_SIZE] __attribute__ ((aligned (UART_TX_BUFFER_SIZE)));
uint8_t RAM_D2 msg[UART_RX_BUFFER_SIZE], scratch[UART_RX_BUFFER_SIZE];
uint8_t imgbuffer[128*128];
int32_t imageindex = 0;
lv_image_dsc_t bt_img = {
  .header = {
    .magic = LV_IMAGE_HEADER_MAGIC,
    .cf = LV_COLOR_FORMAT_A8,
    .flags = 0 | LV_IMAGE_FLAGS_COMPRESSED,
    .w = 128,
    .h = 128,
    .stride = 128,
    .reserved_2 = 0,
  },
  .data_size = 0,
  .data = imgbuffer,
  .reserved = NULL,
};

int32_t scratch_index = 0;

extern settings_t settings;
extern error_state error;

uint8_t *cnt_rx_start = NULL;
uint8_t *cnt_rx_end = NULL;
uint8_t *bt_rx_start = NULL;
uint8_t *bt_rx_end = NULL;

#define BT_MSG_END      0x55
#define BT_MSG_START    0xAA
#define BT_MSG_CONTIUE  0xF0
#define BT_MSG_TAG_TIME_LEFT        0x01
#define BT_MSG_TAG_DISTANCE_LEFT    0x02
#define BT_MSG_TAG_MSG              0x03
#define BT_MSG_TAG_IMG              0x04
#define BT_MSG_TAG_IMG_SIZE         0x05
#define BT_MSG_TAG_SETTIME          0x06
#define BT_MSG_TAG_BATTERY_LEVEL    0x07
#define BT_MSG_TAG_CMD              0x08

#pragma pack(push, 1)
typedef struct bt_msg_t {
    uint8_t preamble;
    uint8_t tag;
    uint8_t length;
    uint8_t crc;
    union {
        uint8_t *data;
        int32_t *int32;
        uint32_t *uint32;
    };
    uint32_t total_length; // not in message!
} bt_msg;
#pragma pack(pop)

volatile uint32_t bt_tcok = 1, bt_idle = 0, bt_conn_state = 0;

UART_HandleTypeDef huart8;
UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_uart8_tx;
DMA_HandleTypeDef hdma_uart8_rx;
DMA_HandleTypeDef hdma_usart6_rx;
DMA_HandleTypeDef hdma_usart6_tx;

#if LV_USE_LOG
#pragma message("Providing log callback, do not attach to a motor")
#if LVGL_VERSION_MAJOR == 9
void lv_log_callback(lv_log_level_t lvl, const char *c);
#else
void lv_log_callback(const char *c);
#endif
#endif
static void uart_init_dma(void);
static void uart_init_ports(uint32_t cnt_baud, uint32_t bt_baud);

void uart_init(uint32_t cnt_baud, uint32_t bt_baud)
{
    LV_LOG_INFO("UART Initialization");
    uart_init_ports(cnt_baud, bt_baud);
    uart_init_dma();

#if (DEBUG && LV_USE_LOG && !defined(SEMIHOSTING) && !defined(SEGGER_RTT) && !defined(SWO))
#pragma message("Using uart to log LV messages")
    lv_log_register_print_cb(lv_log_callback);
#endif
}

static void uart_init_dma(void) {
    memset(bt_rx_buffer, 0x00, sizeof(UART_RX_BUFFER_SIZE));
    memset(bt_tx_buffer, 0x00, sizeof(UART_RX_BUFFER_SIZE));
    memset(cnt_rx_buffer, 0x00, sizeof(UART_RX_BUFFER_SIZE));
    memset(cnt_tx_buffer, 0x00, sizeof(UART_RX_BUFFER_SIZE));

    __HAL_RCC_DMA1_CLK_ENABLE();
    /* __HAL_RCC_BDMA_CLK_ENABLE(); */
    /* uart8 DMA Init */
    /* uart8_TX Init */
    hdma_uart8_tx.Instance = DMA1_Stream0;
    hdma_uart8_tx.Init.Request = DMA_REQUEST_UART8_TX;
    hdma_uart8_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_uart8_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart8_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart8_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart8_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart8_tx.Init.Mode = DMA_CIRCULAR;
    hdma_uart8_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_uart8_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(&huart8,hdmatx,hdma_uart8_tx);

    /* uart8_RX Init */
    hdma_uart8_rx.Instance = DMA1_Stream1; 
    hdma_uart8_rx.Init.Request = DMA_REQUEST_UART8_RX;
    hdma_uart8_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart8_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart8_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart8_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart8_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart8_rx.Init.Mode = DMA_CIRCULAR;
    hdma_uart8_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_uart8_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(&huart8,hdmarx,hdma_uart8_rx);

    /* USART6 DMA Init */
    /* USART6_RX Init */
    hdma_usart6_rx.Instance = DMA1_Stream4;
    hdma_usart6_rx.Init.Request = DMA_REQUEST_USART6_RX;
    hdma_usart6_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart6_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart6_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart6_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart6_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart6_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart6_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart6_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart6_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(&huart6,hdmarx,hdma_usart6_rx);

    /* USART6_TX Init */
    hdma_usart6_tx.Instance = DMA1_Stream5;
    hdma_usart6_tx.Init.Request = DMA_REQUEST_USART6_TX;
    hdma_usart6_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart6_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart6_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart6_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart6_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart6_tx.Init.Mode = DMA_NORMAL;
    hdma_usart6_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart6_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart6_tx) != HAL_OK)
    {
      Error_Handler();
    }

    /* __HAL_LINKDMA(&huart6,hdmatx,hdma_usart6_tx); */

    cnt_rx_start = cnt_rx_buffer;
    cnt_rx_end = &cnt_rx_buffer[UART_RX_BUFFER_SIZE - 1];

    bt_rx_start = bt_rx_buffer;
    bt_rx_end = &bt_rx_buffer[UART_RX_BUFFER_SIZE - 1];


    // TX
    DMA1_Stream0->CR &= ~(DMA_IT_TE | DMA_IT_HT | DMA_IT_DME);
    DMA1_Stream0->CR |= DMA_IT_TC;
    DMA1_Stream0->M0AR = (uint32_t) NULL;
    DMA1_Stream0->PAR = (uint32_t) &UART8->TDR;
    DMA1_Stream0->NDTR = (uint32_t) 0;

    // RX
    DMA1_Stream1->CR &= ~(DMA_IT_TE | DMA_IT_HT | DMA_IT_DME);
    DMA1_Stream1->CR |= DMA_IT_TC;
    DMA1_Stream1->PAR = (uint32_t) &UART8->RDR;
    DMA1_Stream1->M0AR = (uint32_t) cnt_rx_start;
    DMA1_Stream1->NDTR = (uint32_t) UART_RX_BUFFER_SIZE & 0x0000FFFF;

    LV_LOG_INFO("BT DMA Setup");
    LV_LOG_INFO("DMA1_Stream4 ISR: %08lX", DMA1->HISR);
    // TX
    DMA1_Stream5->CR &= ~(DMA_IT_TE | DMA_IT_HT | DMA_IT_DME | DMA_IT_TC | DMA_IT_FE | DMA_SxCR_EN | DMA_SxCR_DBM);
    while(DMA1_Stream5->CR & DMA_SxCR_EN);
    /* DMA1_Stream5->CR &= ~(DMA_IT_TE | DMA_IT_HT | DMA_IT_FE);  */
    DMA1_Stream5->CR = DMA_IT_TC | DMA_SxCR_MINC | DMA_SxCR_TRBUFF;
    DMA1_Stream5->M0AR = (uint32_t) NULL;
    DMA1_Stream5->PAR = (uint32_t) &USART6->TDR;
    DMA1_Stream5->NDTR = (uint32_t) 0;
    DMA1->HIFCR = (DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTCIF5 | DMA_HIFCR_CTEIF5 | DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5);

    // RX
    DMA1_Stream4->CR &= ~(DMA_SxCR_EN);
    while(DMA1_Stream4->CR & DMA_SxCR_EN);
    DMA1_Stream4->CR = DMA_IT_TC | DMA_SxCR_TRBUFF | DMA_SxCR_MINC | DMA_SxCR_CIRC;
    DMA1_Stream4->PAR = (uint32_t) &USART6->RDR;
    DMA1_Stream4->M0AR = (uint32_t) bt_rx_start;
    DMA1_Stream4->NDTR = (uint32_t) UART_RX_BUFFER_SIZE & 0x0000FFFF;
    DMA1->HIFCR = (DMA_HIFCR_CHTIF4 | DMA_HIFCR_CTCIF4 | DMA_HIFCR_CTEIF4 | DMA_HIFCR_CDMEIF4 | DMA_HIFCR_CFEIF4);


    /* DMA1_Stream0->CR |= DMA_SxCR_EN; */
    DMA1_Stream1->CR |= DMA_SxCR_EN;
    /* DMA1_Stream5->CR |= DMA_SxCR_EN; */

    // make sure dma is enabled for rx and tx
    SET_BIT(USART6->CR3, USART_CR3_DMAR);
    SET_BIT(UART8->CR3, USART_CR3_DMAR);
    SET_BIT(USART6->CR3, USART_CR3_DMAT);
    SET_BIT(UART8->CR3, USART_CR3_DMAT);


    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);

    HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

    HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
    
    DMA1_Stream4->CR |= DMA_SxCR_EN;

    LV_LOG_INFO("DMA1_Stream4 ::: %08lX %08lX %08lX %08lX", DMA1_Stream4->CR, DMA1_Stream4->FCR, DMA1_Stream4->PAR, DMA1_Stream4->M0AR);
}

static void uart_init_ports(uint32_t cnt_baud, uint32_t bt_baud) {

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
#ifdef CONTROLLER_UART_ENABLED

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART8;
    PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_UART8_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**uart8 GPIO Configuration
    PA9     ------> uart8_TX
    PA10     ------> uart8_RX
    */
    GPIO_InitStruct.Pin = CONTROLLER_UART_RX_Pin|CONTROLLER_UART_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART8;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    huart8.Instance = UART8;
    huart8.Init.BaudRate = cnt_baud;
    huart8.Init.WordLength = UART_WORDLENGTH_8B;
    huart8.Init.StopBits = UART_STOPBITS_1;
    huart8.Init.Parity = UART_PARITY_NONE;
    huart8.Init.Mode = UART_MODE_TX_RX;
    huart8.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart8.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart8.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart8.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    huart8.FifoMode = UART_FIFOMODE_DISABLE;
    if (HAL_UART_Init(&huart8) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&huart8, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&huart8, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_UARTEx_DisableFifoMode(&huart8) != HAL_OK)
    {
        Error_Handler();
    }

    UART8->ICR = 0xFFFFFFFF; 

    UART8->CR1 |= 
        USART_CR1_IDLEIE | 
        USART_CR1_TCIE | 
        USART_CR1_PEIE | 
        USART_CR1_CMIE; 
    UART8->CR3 |= 
        USART_CR3_EIE | 
        USART_CR3_CTSIE | 
        USART_CR3_WUFIE; 

    /* uart8 interrupt Init */
    HAL_NVIC_SetPriority(UART8_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(UART8_IRQn);
    __HAL_USART_ENABLE(&huart8);
#endif

#ifdef BT_UART_ENABLED 
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART6;
    PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }


    /* Peripheral clock enable */
    __HAL_RCC_USART6_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX
    */
    GPIO_InitStruct.Pin = BT_UART_TX_Pin|BT_UART_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = BT_FUNC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = BT_RESET_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    huart6.Instance = USART6;
    huart6.Init.BaudRate = bt_baud;
    huart6.Init.WordLength = UART_WORDLENGTH_8B;
    huart6.Init.StopBits = UART_STOPBITS_1;
    huart6.Init.Parity = UART_PARITY_NONE;
    huart6.Init.Mode = UART_MODE_TX_RX;
    huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart6.Init.OverSampling = UART_OVERSAMPLING_16;
    huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart6.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    USART6->CR1 = 0; // disable
    USART6->CR2 = 0; // keep 0
    USART6->CR3 = USART_CR3_OVRDIS;
    USART6->CR1 = ( 
            USART_CR1_RE | // receive enable 
            USART_CR1_TE   // transfer enable
        );
    // use ll functions to set clock
    LV_LOG_INFO("BT Baud set to %ld",bt_baud); 
    /* LL_USART_SetBaudRate(USART6, LL_RCC_GetUSARTClockFreq(LL_RCC_USART16_CLKSOURCE), UART_PRESCALER_DIV1, UART_OVERSAMPLING_16, bt_baud); */
    USART6->BRR = (120000000/bt_baud); // 230326 baud
    USART6->GTPR = 1; // no guard, div1
    /* LL_USART_SetTXRXSwap(USART6, 1); */

    USART6->ICR = 0xFFFFFFFF; 
    // set interrupts
    USART6->CR1 |= 
        USART_CR1_IDLEIE | 
        USART_CR1_RXNEIE | 
        USART_CR1_TCIE; 
    USART6->CR3 |= 
        USART_CR3_EIE; 

    /* USART6 interrupt Init */
    HAL_NVIC_SetPriority(USART6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART6_IRQn);

    LV_LOG_INFO("UART6: %08lX %08lX %08lX %08lX", USART6->CR1, USART6->CR2, USART6->CR3, USART6->BRR);
    USART6->CR1 |= 1; // enable
    /* __HAL_USART_ENABLE(&huart6); */

    bt_reset();
#endif
}

void bt_recv_reset(void) {
    bt_rx_start = bt_rx_end = bt_rx_buffer;
    DMA1_Stream4->M0AR = (uint32_t) bt_rx_buffer;
    DMA1_Stream4->NDTR = (uint32_t) UART_RX_BUFFER_SIZE & 0x0000FFFF;
}

int bt_length(void) {
    if (bt_rx_end == bt_rx_start) return 0;
    int rem = (bt_rx_end - bt_rx_start) % UART_RX_BUFFER_SIZE;
    return rem >= 0 ? rem : rem + UART_RX_BUFFER_SIZE;
}

int bt_get(uint8_t *data, uint32_t length, int move) {
    if (bt_length() > 0 && bt_length() <= length) {
        int rv = bt_length();
        if (bt_rx_start + rv > bt_rx_buffer + UART_RX_BUFFER_SIZE) { // wraparound
            int rv2 = rv - (UART_RX_BUFFER_SIZE - ((uint32_t ) bt_rx_start % UART_RX_BUFFER_SIZE));
            if (data != NULL) {
                memcpy(data, bt_rx_start, UART_RX_BUFFER_SIZE - ((uint32_t) bt_rx_start % UART_RX_BUFFER_SIZE));
                data += UART_RX_BUFFER_SIZE - ((uint32_t) bt_rx_start % UART_RX_BUFFER_SIZE);
                memcpy(data, bt_rx_buffer, rv2);
            }
            if (move) bt_rx_start = bt_rx_buffer + rv2;
        } else {
            if (data != NULL) memcpy(data, bt_rx_start, rv);
            if (move) {
                bt_rx_start += rv;
                if (bt_rx_start > bt_rx_buffer + UART_RX_BUFFER_SIZE) bt_rx_start = bt_rx_buffer + ((int32_t)bt_rx_start % UART_RX_BUFFER_SIZE);
            }
        }
        return rv;
    }
    return 0;
}

#define ENDSIGNAL ""
const char *AT = (const char *) "AT" ENDSIGNAL;
const char *AT_ADVI = (const char *) "AT+ADVI5" ENDSIGNAL;
const char *AT_BAUD = (const char *) "AT+BAUD8" ENDSIGNAL; // set to 230400 baud;
const char *AT_RESET = (const char *) "AT+RESET" ENDSIGNAL;
const char *AT_MODE0 = (const char *) "AT+MODE0" ENDSIGNAL;
const char *AT_NAME = (const char *) "AT+NAMEBike" ENDSIGNAL;
const char *AT_PASS = (const char *) "AT+PASS123456" ENDSIGNAL;
const char *AT_ROLE = (const char *) "AT+ROLE0" ENDSIGNAL;
const char *AT_ADTY = (const char *) "AT+ADTY0" ENDSIGNAL;
const char *AT_PIO = (const char *) "AT+PIO11" ENDSIGNAL;
const char *OK = (const char *) "OK" ENDSIGNAL;

int bt_recv_ok(uint32_t delay, uint8_t *compare, uint32_t compare_length) {
    LV_LOG_INFO("Waiting for response");
    uint8_t bf[32] = {0};
    uint32_t start = HAL_GetTick();
    while((!bt_idle) && HAL_GetTick() - start < delay);
    LV_LOG_INFO("Checking response");
    if (compare == NULL) {
        bt_get(bf, strlen(OK), 1);
        if (memcmp(bf, OK, strlen(OK)) == 0) {
            LV_LOG_INFO("Response received");
            return 1;
        }
    } else {
        bt_get(bf, compare_length, 1);
        if (memcmp(bf, compare, compare_length) == 0) {
            LV_LOG_INFO("Response received");
            return 1;
        }
    }
    LV_LOG_INFO("NO RESPONSE WITHIN TIME");
    return 0;
}

#define BT_SEND(x) bt_send((const uint8_t *) x, strlen(x))

void bt_send_init(uint32_t program) {
    LV_LOG_INFO("BT Send init");
    HAL_Delay(10);
    USART6->CR1 &= ~(1L);
    USART6->BRR = (120000000L/9600L);
    USART6->CR1 |= 1;
    bt_reset();
    HAL_Delay(10);
    BT_SEND(AT);
    while(!bt_tcok);
    /* if (!bt_recv_ok(10, NULL, 0)) { */
    LV_LOG_INFO("Setting to 230k");
    LV_LOG_INFO("%d", bt_length());
    LV_LOG_INFO("%s", bt_rx_buffer);
    BT_SEND(AT_BAUD);
    while(!bt_tcok);
    bt_recv_ok(10, (uint8_t *) "OK+Set:8\n", 9);
    BT_SEND(AT_RESET);
    while(!bt_tcok);
    /* } */
    LV_LOG_INFO("Continueing boot");
    
    USART6->CR1 &= ~(1L);
    USART6->BRR = (120000000L/230400L);
    USART6->CR1 |= 1;
    bt_reset();
    // might have missed signal: process
    LV_LOG_INFO("Sending PIO");
    BT_SEND(AT_PIO);
    int rv = bt_recv_ok(20, (uint8_t *)"OK+Set:1", 8);
    if (rv == 0) return;
    LV_LOG_INFO("Sending Mode 0");
    BT_SEND(AT_MODE0);
    rv = bt_recv_ok(20, (uint8_t *)"OK+Set:0", 8);
    if (rv == 0) return;
    LV_LOG_INFO("Sending Advertisement interval");
    BT_SEND(AT_ADVI);
    rv = bt_recv_ok(20, (uint8_t *)"OK+Set:5", 8);
    if (rv == 0) return;
    LV_LOG_INFO("Sending BT Name");
    BT_SEND(AT_NAME);
    rv = bt_recv_ok(20, (uint8_t *)"OK+Set:Bike", 11); 
    if (rv == 0) return;
    LV_LOG_INFO("Sending passphrase");
    BT_SEND(AT_PASS);
    rv = bt_recv_ok(20, (uint8_t *)"OK+Set:123456", 13);
    if (rv == 0) return;
    LV_LOG_INFO("Sending role");
    BT_SEND(AT_ROLE);
    rv = bt_recv_ok(20, (uint8_t *)"OK+Set:0", 8);
    if (rv == 0) return;
    LV_LOG_INFO("Sending advertise");
    BT_SEND(AT_ADTY);
    bt_recv_ok(20, (uint8_t *)"OK+Set:0", 8);
    LV_LOG_INFO("Bluetooth initialized!"); 
    // now available as "serial" device on char. 0xffe1
}

int bt_isconnected(void) {
    return HAL_GPIO_ReadPin(BT_FUNC_GPIO_Port, BT_FUNC_Pin);
}
#define BLOCK_LENGTH 20
int bt_msg_check(bt_msg *b) {
    if (b->length > BLOCK_LENGTH) return 0;
    uint8_t block[BLOCK_LENGTH + 1];
    memcpy(block, b, 3);
    memcpy(&block[3], b->data, b->length);
    // simple sum for now
    uint8_t crc = 0;
    for (int i = 0; i < b->length + 3; i++) crc += block[i];
    if (crc == b->crc) return 1;
    return 0;
}

void bt_check(void) {
    if (!bt_isconnected()) 
    {
        led_blue(0);
        return;
    }
    if (led_blue_state() == 0) led_blue(1);

    if (bt_length() > 0) {
        int size = bt_get(msg, sizeof(msg), 0); // get all in sequence
        // message is now in separate buffer
        int i = 0;
        while(i < size) {
            switch (msg[i]) {
                case BT_MSG_START:
                case BT_MSG_END:
                case BT_MSG_CONTIUE:
                {
                    // do stuff
                    bt_msg b;
                    memcpy(&b, &msg[i], 4);
                    b.data = &msg[i+4];
                    if (b.length > BLOCK_LENGTH) {
                        LV_LOG_INFO("Invalid message length");
                        i++;
                    } else if (!bt_msg_check(&b)) {
                        LV_LOG_INFO("Invalid checksum");
                        i++;
                    } else {
                        // valid 
                        LV_LOG_INFO("MSG VALID %02X", b.preamble); 
                        i += (4 + b.length);
                        if (b.preamble == BT_MSG_START) scratch_index = 0;
                        memcpy(&scratch[scratch_index], b.data, b.length);
                        scratch_index += b.length;
                        if (b.preamble == BT_MSG_END) {
                            switch (b.tag) {
                                case BT_MSG_TAG_MSG:
                                    LV_LOG_INFO("MSG: %s", (char *) scratch);
                                    gui_write_bt_msg((char *) scratch, scratch_index);
                                    break;
                                case BT_MSG_TAG_DISTANCE_LEFT:
                                    {
                                        int32_t *v = (int32_t *) scratch;
                                        LV_LOG_INFO("Distance left: %ld", __ntohl(*v));
                                        gui_write_bt_distance(__ntohl(*v));
                                        break;
                                    }
                                case BT_MSG_TAG_TIME_LEFT:
                                    {
                                        int32_t *v = (int32_t *) scratch;
                                        LV_LOG_INFO("Time left: %ld", __ntohl(*v));
                                        gui_write_bt_time(__ntohl(*v));
                                        break;
                                    }
                                case BT_MSG_TAG_IMG:
                                    {
                                        LV_LOG_INFO("IMG");
                                        memcpy(&imgbuffer[imageindex], scratch, scratch_index);
                                        imageindex += scratch_index;
                                        if (imageindex >= sizeof(imgbuffer)) imageindex = 0;
                                    }
                                    break;

                                case BT_MSG_TAG_IMG_SIZE:
                                    {
                                        int32_t *v = (int32_t *) scratch;
                                        LV_LOG_INFO("Img size: %ld", __ntohl(*v));
                                        bt_img.data_size = __ntohl(*v);
                                        if (imageindex != __ntohl(*v)) {
                                            LV_LOG_INFO("IMAGE SIZE MISMATCH: %ld %ld", imageindex, __ntohl(*v));
                                        }
                                        imageindex = 0;
                                        gui_write_bt_img(&bt_img);
                                    }
                                    break;
                                case BT_MSG_TAG_SETTIME:
                                    {
                                        // year = 2, rest 1 byte
                                        LV_LOG_INFO("Set Time");
                                        int16_t *v = (int16_t *) scratch;
                                        clock_set_date(__ntohs(*v), (int32_t) scratch[2], (int32_t) scratch[3], (int32_t) scratch[7]);
                                        clock_set_time((int32_t) scratch[4], (int32_t) scratch[5], (int32_t) scratch[6]);
                                    }
                            }
                            // phone is still connected, disable shutdown
                            update_shutdown();
                            scratch_index = 0;
                        }
                    }
                    break;
                }
                default:
                    i++;
                    break;
            }
        }
        bt_rx_start += (i);
        if (bt_rx_start > bt_rx_buffer + UART_RX_BUFFER_SIZE) bt_rx_start = bt_rx_buffer + ((int32_t)bt_rx_start % UART_RX_BUFFER_SIZE);
        /* while(j < size) { */
        /*     for (; j < size && msg[j] != BT_MSG_END; j++); */
        /*     for (; i < size && msg[i] != BT_MSG_START && msg[i] != BT_MSG_END; i++); // find PREAMBLE */
        /*     LV_LOG_INFO("BT MSG: %ld %ld %ld", i, j, size); */
        /*     if (j == size) { */
        /*         char mb[33]; */
        /*         int x = 0; */
        /*         for (; x < j && x < 16; x++) { */
        /*             sprintf(&mb[x*2], "%02X", msg[x]); */
        /*         } */
        /*         LV_LOG_INFO("MSG: %s", mb); */
        /*         return; // not end tag found */
        /*     } */
        /*     if (i <= j) { */
        /*         // start tag and end tag found do loop */
        /*         bt_msg a, b; */
        /*         memcpy(&a, &msg[i], 4); */
        /*         a.data = &msg[i+4]; */
        /*         a.length = __ntohs(a.length); */
        /*         memcpy(&b, &msg[i], 4); */
        /*         b.length = __ntohs(b.length); */
        /*         j += 4 + b.length; */
        /*         LV_LOG_INFO("MSG START: %02X %02X %hd", a.preamble, a.tag, a.length); */
        /*  */
        /*         for (int x = i; x < size && msg[x] != BT_MSG_END; x++) { */
        /*             memcpy(&b, &msg[x], 4); */
        /*             b.data = &msg[x+4]; */
        /*             b.length = __ntohs(b.length); */
        /*             if (b.tag == a.tag && b.preamble == BT_MSG_CONTIUE || b.preamble == BT_MSG_END) { */
        /*                 uint16_t z = a.length;  */
        /*                 a.length += b.length; */
        /*                 LV_LOG_INFO("MSG CNTIN: %02X %02X %hd %hd", b.preamble, b.tag, b.length, z); */
        /*                 memcpy(&a.data[a.length], b.data, b.length); // in place! */
        /*             } */
        /*         } */
        /*         switch (a.tag) { */
        /*             case BT_MSG_TAG_MSG: */
        /*                 LV_LOG_INFO("MSG: %s", (char *) a.data); */
        /*                 gui_write_bt_msg((char *) a.data, a.length); */
        /*                 break; */
        /*             case BT_MSG_TAG_DISTANCE_LEFT: */
        /*                 LV_LOG_INFO("Distance/time left: %ld", __ntohl(*a.int32)); */
        /*                 gui_write_bt_distance(__ntohl(*a.int32)); */
        /*                 break; */
        /*             case BT_MSG_TAG_TIME_LEFT: */
        /*                 LV_LOG_INFO("Distance/time left: %ld", __ntohl(*a.int32)); */
        /*                 gui_write_bt_time(__ntohl(*a.int32)); */
        /*                 break; */
        /*             case BT_MSG_TAG_IMG: */
        /*                 LV_LOG_INFO("Img"); */
        /*                 break; */
        /*         } */
        /*         y = j; */
        /*     } */
        /*     bt_rx_start += y; */
        /*     if (bt_rx_start > bt_rx_buffer + UART_RX_BUFFER_SIZE) bt_rx_start = bt_rx_buffer + ((int32_t)bt_rx_start % UART_RX_BUFFER_SIZE); */
        /* } */
    }
}

void bt_send_battery(uint8_t level) {
    uint8_t cmd[5] = { BT_MSG_END, BT_MSG_TAG_BATTERY_LEVEL, 0x01, 0x00, level };
    cmd[3] = cmd[0] + cmd[1] + cmd[2] + cmd[4];
    if (bt_isconnected()) bt_send(cmd, 5);
    LV_LOG_INFO("Send BT Battery level");
}

void bt_reset(void){
    HAL_GPIO_WritePin(BT_RESET_GPIO_Port, BT_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(BT_RESET_GPIO_Port, BT_RESET_Pin, GPIO_PIN_SET);
    bt_recv_reset();

}

void bt_send_cmd(uint8_t id) {
    uint8_t cmd[5] = { BT_MSG_END, BT_MSG_TAG_CMD, 0x01, 0x00, id };
    cmd[3] = cmd[0] + cmd[1] + cmd[2] + cmd[4];
    if (bt_isconnected()) bt_send(cmd, 5);
    LV_LOG_INFO("Send BT cmd %02X", id);
}

CRITICAL void cnt_send(const uint8_t *buffer, ssize_t length) {

#ifndef CAN_ENABLED
    LV_LOG_INFO("Sending to CNT: %d", length);
    memcpy(cnt_tx_buffer, buffer, length);
    DMA1_Stream0->CR = 0; // reconfigure 
    while(DMA1_Stream0->CR & DMA_SxCR_EN); // wait for clearance
    // only enable TC
    DMA1_Stream0->PAR = (uint32_t) &UART8->TDR;
    DMA1_Stream0->M0AR = (uint32_t) cnt_tx_buffer;
    DMA1_Stream0->NDTR = (uint32_t) length;

    DMA1_Stream0->CR = DMA_SxCR_PFCTRL | DMA_SxCR_MINC | DMA_SxCR_DIR_0;
    DMA1_Stream0->CR |= (DMA_IT_TE | DMA_IT_HT | DMA_IT_FE | DMA_IT_DME);
    DMA1_Stream0->CR |= DMA_IT_TC;

    UART8->CR1 |= 
        USART_CR1_IDLEIE | 
        USART_CR1_TCIE; 
    UART8->CR3 |= 
        USART_CR3_EIE | 
        USART_CR3_CTSIE | 
        USART_CR3_WUFIE; 
    UART8->ICR = UART_CLEAR_TCF;
    DMA1->LIFCR |= DMA_LIFCR_CTCIF0;


    UART8->CR1 |= USART_CR1_UE;
    DMA1_Stream0->CR |= DMA_SxCR_EN;
    ATOMIC_SET_BIT(UART8->CR3, USART_CR3_DMAT);
#endif
}

CRITICAL void bt_send(const uint8_t *buffer, uint32_t length) {
    bt_idle = 0;
    while(!bt_tcok);
    LV_LOG_INFO("Sending to BT: %lu", length);
    memcpy(bt_tx_buffer, buffer, length);

    DMA1_Stream5->CR &=  ~DMA_SxCR_EN;
    while(DMA1_Stream5->CR & DMA_SxCR_EN); // wait for clearance
    LV_LOG_INFO("Sending to BT setup dma");
    DMA1->HIFCR = (DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTCIF5 | DMA_HIFCR_CTEIF5 | DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5);
    // only enable TC
//DMA_SxCR_TRBUFF |DMA_SxCR_CIRC |  | DMA_SxCR_PFCTRL
    DMA1_Stream5->CR = (DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_TCIE | DMA_SxCR_TEIE);
    DMA1_Stream5->FCR = 0; //(uint32_t) (~DMA_IT_FE);

    DMA1_Stream5->NDTR = (uint32_t) length & 0x0000FFFF;
    DMA1_Stream5->PAR = (uint32_t) &USART6->TDR;
    DMA1_Stream5->M0AR = (uint32_t) bt_tx_buffer;

    LV_LOG_INFO("Enable Ints");
    USART6->CR1 |= 
        USART_CR1_IDLEIE | 
        USART_CR1_TCIE; 
    /* USART6->CR3 |=  */
    /*     USART_CR3_EIE |  */
    /*     USART_CR3_CTSIE |  */
    /*     USART_CR3_WUFIE;  */
    /* DMA1->HIFCR |= DMA_HIFCR_CTCIF5; */

    /* USART6->CR1 |= USART_CR1_UE; */
    /* DMA1_Stream5->CR = rv | DMA_SxCR_EN; */
    bt_tcok = 0;
    /* __HAL_UART_CLEAR_FLAG(&huart6, UART_CLEAR_TCF); */
    /* ATOMIC_SET_BIT(USART6->CR3, USART_CR3_DMAT); */
    DMA1_Stream5->CR |= DMA_SxCR_EN;
}



int32_t tval = 0;

CRITICAL int uart_get_data(uint8_t *data, uint32_t *length) {
    /* if (uart_rx_ready) { */
    /*     uart_rx_ready = 0; */
    /*     *length = read_buffer_length; */
    /*     if (*length > UART_RX_BUFFER_SIZE) *length = UART_RX_BUFFER_SIZE; */
    /*     if (uart_rx_start == uart_rx_end) { */
    /*         return 0; */
    /*     } else if (uart_rx_end < uart_rx_start) { */
    /*         memcpy(data, uart_rx_start, UART_RX_BUFFER_SIZE - (uart_rx_start - uart_rx_buffer)); */
    /*         memcpy(&data[UART_RX_BUFFER_SIZE - (uart_rx_start - uart_rx_buffer)], uart_rx_buffer, uart_rx_end - uart_rx_buffer); */
    /*     } else { */
    /*         memcpy(data, uart_rx_start, uart_rx_end - uart_rx_start); */
    /*     } */
    /*     uart_rx_start = uart_rx_end; */
    /*     read_buffer_length = 0; */
    /*     #<{(| char bla[65] = {0}; |)}># */
    /*     #<{(| for (int i = 0; i < *length && i < 32; i++) { |)}># */
    /*     #<{(|     sprintf(&bla[i*2], "%02X", data[i]); |)}># */
    /*     #<{(| } |)}># */
    /*     #<{(| printf("%lu %s\n", *length, bla); |)}># */
    /*  */
    /*     dma_channel_enable(DMA1_CHANNEL5, TRUE); */
    /*     return 1; */
    /* } else { */
    /*     // make sure */
    /*     dma_channel_enable(DMA1_CHANNEL5, TRUE); */
    /* } */
    return 0;
}


#ifdef DEBUG
uint32_t divval(void) {
    /* return USART1->baudr_bit.div; */
}
#endif

#if (LV_USE_LOG && !defined(SEMIHOSTING) && !defined(SEGGER_RTT)) || defined(SWO)
#if LVGL_VERSION_MAJOR == 9
void lv_log_callback(lv_log_level_t lvl, const char *c) {
#else
void lv_log_callback(const char *c) {
#endif
#if defined(SWO)
    int l = strlen(c);
    while(l--) ITM_SendChar(*c++);
#else
    uart_send(c, strlen(c), 0);
#endif
}
#endif


static inline void uart_int_error_handler(uint32_t in) {
    if (in & USART_ISR_TCBGT) LV_LOG_INFO("Transmission complete before guard");
    if (in & USART_ISR_BUSY) LV_LOG_INFO("USART6 Busy");
    if (in & USART_ISR_ABRE) LV_LOG_INFO("Auto baud rate error");
    if (in & USART_ISR_UDR) LV_LOG_INFO("SPI underrun error");
    if (in & USART_ISR_RTOF) { LV_LOG_INFO("Receiver Timeout"); error |= ERROR_UART_RECEIVE; }
    if (in & USART_ISR_TXE_TXFNF) LV_LOG_INFO("Transmission registry empty");
    if (in & USART_ISR_RXNE_RXFNE) LV_LOG_INFO("Read registry not empty");
    if (in & USART_ISR_ORE) { LV_LOG_INFO("Overrun error"); error |= ERROR_UART_TRANSMIT; }
    if (in & USART_ISR_NE) { LV_LOG_INFO("Noise detection"); error |= ERROR_UART_RECEIVE; }
    if (in & USART_ISR_FE) { LV_LOG_INFO("Frameing error"); error |= ERROR_UART_TRANSMIT; }
    if (in & USART_ISR_PE) { LV_LOG_INFO("Parity error"); error |= ERROR_UART_RECEIVE; }
#ifdef CAN_ENABLED
    error &= ~(ERROR_UART_RECEIVE | ERROR_UART_TRANSMIT);
#endif
}

/**
  * @brief This function handles DMA1 stream1 global interrupt.
  */
CRITICAL void DMA1_Stream0_IRQHandler(void)
{
    LV_LOG_TRACE("CNT UART TX DMA Int: %08lX %08lX", DMA1->LISR, DMA1->HISR);
    if (DMA1->LISR & DMA_LISR_TCIF1) LV_LOG_TRACE("Transfer complete"); 
    if (DMA1->LISR & DMA_LISR_HTIF1) LV_LOG_TRACE("Half Transfer complete"); 
    if (DMA1->LISR & DMA_LISR_TEIF1) LV_LOG_TRACE("Transfer Error"); 
    if (DMA1->LISR & DMA_LISR_DMEIF1) LV_LOG_TRACE("Direct mode error "); 
    if (DMA1->LISR & DMA_LISR_FEIF1) LV_LOG_TRACE("Fifo error "); 

    DMA1->LIFCR = (DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTCIF0 | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0); 
}



CRITICAL void USART6_IRQHandler(void) {
    uint32_t ints = USART6->ISR;
    USART6->ICR = 0xFFFFFFFF; 
    LV_LOG_TRACE("BT USART Int: %08lX", ints);
#ifdef DEBUG
    uart_int_error_handler(ints);
#endif

    if (ints & USART_ISR_TC) {
        LV_LOG_TRACE("Transmission complete");
        USART6->CR1 &= ~(USART_CR1_TCIE);
        bt_tcok = 1;
    }
    if (ints & USART_ISR_IDLE) {
        // update receiver
        bt_rx_end = (uint8_t *) (DMA1_Stream4->M0AR + UART_RX_BUFFER_SIZE - DMA1_Stream4->NDTR);
        /* LV_LOG_INFO("Receiver idle: %08lX %08lX %d %08lX %08lX", DMA1_Stream4->NDTR, DMA1_Stream4->M0AR, bt_length(), bt_rx_end, bt_rx_start); */
        bt_idle = 1;
    }
    // clear all
}

CRITICAL void DMA1_Stream1_IRQHandler(void)
{
  LV_LOG_TRACE("CNT UART TX DMA In");

  DMA1->LIFCR = (DMA_LIFCR_CHTIF1 | DMA_LIFCR_CTCIF1 | DMA_LIFCR_CTEIF1 | DMA_LIFCR_CDMEIF1 | DMA_LIFCR_CFEIF1);
}



/**
  * @brief This function handles BDMA channel1 global interrupt.
  */
CRITICAL void DMA1_Stream4_IRQHandler(void)
{
    uint32_t st = DMA1->HISR;
    DMA1->HIFCR = (DMA_HIFCR_CHTIF4 | DMA_HIFCR_CTCIF4 | DMA_HIFCR_CTEIF4 | DMA_HIFCR_CDMEIF4 | DMA_HIFCR_CFEIF4);

    if (st & DMA_HISR_TCIF4) LV_LOG_INFO("Transfer complete"); 
    if (st & DMA_HISR_HTIF4) LV_LOG_INFO("Half Transfer complete"); 
    if (st & DMA_HISR_TEIF4) LV_LOG_INFO("Transfer Error"); 
    if (st & DMA_HISR_DMEIF4) LV_LOG_INFO("Direct mode error "); 
    if (st & DMA_HISR_FEIF4) LV_LOG_INFO("Fifo error "); 
    /* LV_LOG_INFO("BT  RX DMA Int %d", bt_length()); */
    LV_LOG_INFO("DMA1_Stream4 ::: %08lX", st);
    LV_LOG_INFO("DMA1_Stream4 ::: %08lX %08lX %08lX %08lX", DMA1_Stream4->CR, DMA1_Stream4->FCR, DMA1_Stream4->PAR, DMA1_Stream4->M0AR);
    // reenable
    DMA1_Stream4->CR |= DMA_SxCR_EN;
    /* LV_LOG_INFO("%d", bt_length()); */
}

CRITICAL void DMA1_Stream5_IRQHandler(void)
{
    uint32_t st = DMA1->HISR;
    DMA1->HIFCR = (DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTCIF5 | DMA_HIFCR_CTEIF5 | DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5);
#ifdef DEBUG
    LV_LOG_INFO("BT TX DMA Int: %08lX", st);
    if (st & DMA_HISR_TCIF5) LV_LOG_INFO("Transfer complete"); 
    if (st & DMA_HISR_HTIF5) LV_LOG_INFO("Half Transfer complete"); 
    if (st & DMA_HISR_TEIF5) LV_LOG_INFO("Transfer Error"); 
    if (st & DMA_HISR_DMEIF5) LV_LOG_INFO("Direct mode error "); 
    if (st & DMA_HISR_FEIF5) LV_LOG_INFO("Fifo error "); 
#endif

    if (st & DMA_HISR_TCIF5) {
        DMA1_Stream5->CR &= ~(DMA_SxCR_EN);
    }
}

/**
  * @brief This function handles uart8 global interrupt.
  */
CRITICAL void UART8_IRQHandler(void)
{
    LV_LOG_INFO("CNT UART Int: %08lX", UART8->ISR);

    uart_int_error_handler(UART8->ISR);
    UART8->ICR |= 0xFFFFFFFF; 
}

/* [Info]	(0.120, +0)	 DMA1_Stream4_IRQHandler: DMA1_Stream4 ::: 00100510 00000020 40011424 38000C00 uart.c:676 */
/* [Info]	(0.036, +5)	 uart_init_dma: DMA1_Stream4 :::           00100511 00000020 40011424 38000C00 uart.c:207 */
