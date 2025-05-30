#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "stm32h743.h"
#include "config.h"
#include "comm.h"
#include "eeprom.h"
#include "can.h"
#include "lvgl.h"
#include "clock.h"
#include "crc.h"
#include "cntl.h"

FDCAN_HandleTypeDef hfdcan2;
extern error_state error;
extern vesc_mcconf_temp mcconf;
extern volatile int mconf_actual;

#define CAN_LOG     LV_LOG_TRACE

volatile int can_available = 0;

uint32_t drw_tmp_timeout = 0;

typedef struct {
    uint32_t    id;
    ssize_t     offset;
    ssize_t     length;
    uint16_t    crc;
    uint8_t     data[512]; // determined by vesc
    int         empty;
    uint32_t    last_filled;
} vesc_rx_buffer;

vesc_rx_buffer receive_buffers[2];

CRITICAL void FDCAN2_IT0_IRQHandler(void)
{
    led_blue(1);
    CAN_LOG("Can data received");
    can_available = 1;
    FDCAN2->IR = FDCAN_IE_RF0NE | FDCAN_IE_RF1NE | FDCAN_IE_DRXE;
}

CRITICAL void FDCAN2_IT1_IRQHandler(void)
{
    led_blue(1);
    CAN_LOG("Can data received");
    can_available = 1;
    FDCAN2->IR = FDCAN_IE_RF0NE | FDCAN_IE_RF1NE | FDCAN_IE_DRXE;
}
CRITICAL void FDCAN_CAL_IRQHandler(void)
{
    CAN_LOG("Can data received");
    can_available = 1;
    FDCAN2->IR = FDCAN_IE_RF0NE | FDCAN_IE_RF1NE | FDCAN_IE_DRXE;
}


// locals
CRITICAL static void can_vesc_clear_buffers(void) {
    for (int i = 0; i < sizeof(receive_buffers) / sizeof(vesc_rx_buffer); i++) {
        LV_LOG_INFO("Clearing buffer %d", i);
        memset((uint8_t *) &receive_buffers[i], 0x00, sizeof(vesc_rx_buffer));
        receive_buffers[i].empty = 1;
    }
}

CRITICAL static void can_vesc_check_buffers(void) {
    uint32_t ctime = HAL_GetTick();
    for (int i = 0; i < sizeof(receive_buffers) / sizeof(vesc_rx_buffer); i++) {
        if (ctime - receive_buffers[i].last_filled > 1000 && !receive_buffers[i].empty) {
            LV_LOG_INFO("Buffer timeout, clearing %d", i);
            memset((uint8_t *) &receive_buffers[i], 0x00, sizeof(vesc_rx_buffer)); 
            receive_buffers[i].empty = 1;
        }
    }
}
CRITICAL static vesc_rx_buffer *can_vesc_get_buffer(uint32_t id, ssize_t offset) {
    int i = 0;
    vesc_rx_buffer *free = NULL;
    for (; i < sizeof(receive_buffers) / sizeof(vesc_rx_buffer); i++) {
        if (!receive_buffers[i].empty && receive_buffers[i].id == id && receive_buffers[i].length == offset) return &receive_buffers[i];
        if (NULL == free && receive_buffers[i].empty) free = &receive_buffers[i];
    }
    return free;
}

void can_deinit(void) {
    __HAL_RCC_FDCAN_CLK_DISABLE();
}
// globals
void can_init(void) {
    LV_LOG_INFO("Initializing CANBUS");
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};


    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_HSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /**FDCAN2 GPIO Configuration
    PA11     ------> FDCAN2_RX
    PA12     ------> FDCAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    hfdcan2.Instance = FDCAN2;
    hfdcan2.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    hfdcan2.Init.Mode = FDCAN_MODE_NORMAL;
    hfdcan2.Init.AutoRetransmission = ENABLE;
    hfdcan2.Init.TransmitPause = DISABLE;
    hfdcan2.Init.ProtocolException = DISABLE;
    hfdcan2.Init.NominalPrescaler = 2;
    hfdcan2.Init.NominalSyncJumpWidth = 1;
    hfdcan2.Init.NominalTimeSeg1 = 11;
    hfdcan2.Init.NominalTimeSeg2 = 4;
    hfdcan2.Init.DataPrescaler = 1;
    hfdcan2.Init.DataSyncJumpWidth = 1;
    hfdcan2.Init.DataTimeSeg1 = 1;
    hfdcan2.Init.DataTimeSeg2 = 1;
    hfdcan2.Init.MessageRAMOffset = 0;
    hfdcan2.Init.StdFiltersNbr = 0;
    hfdcan2.Init.ExtFiltersNbr = 9;
    hfdcan2.Init.RxFifo0ElmtsNbr = 16;
    hfdcan2.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
    hfdcan2.Init.RxFifo1ElmtsNbr = 16;
    hfdcan2.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
    hfdcan2.Init.RxBuffersNbr = 7;
    hfdcan2.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
    hfdcan2.Init.TxEventsNbr = 0;
    hfdcan2.Init.TxBuffersNbr = 0;
    hfdcan2.Init.TxFifoQueueElmtsNbr = 32;
    hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
    hfdcan2.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
    if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK)
    {
        Error_Handler();
    }

    FDCAN_FilterTypeDef filter1;

    /*
     * vesc frame type
     *
     * [28-16] -> 0
     * [15-8] -> cmd
     * [7-0] -> unit id
     */
    filter1.IdType = FDCAN_EXTENDED_ID;
    filter1.FilterIndex = 0;
    filter1.FilterType = FDCAN_FILTER_MASK;
    filter1.FilterConfig = FDCAN_FILTER_TO_RXBUFFER;
    filter1.FilterID1 = (VESC_CAN_CMD_STATUS_1 << 8) | VESC_CAN_ID;
    filter1.FilterID2 = 0xFFFF;
    filter1.RxBufferIndex = 0;
    if (HAL_FDCAN_ConfigFilter(&hfdcan2, &filter1) != HAL_OK)
    {
        Error_Handler();
    }

    filter1.FilterIndex = 1;
    filter1.FilterID1 = (VESC_CAN_CMD_STATUS_2 << 8) | VESC_CAN_ID;
    filter1.RxBufferIndex = 1;
    if (HAL_FDCAN_ConfigFilter(&hfdcan2, &filter1) != HAL_OK)
    {
        Error_Handler();
    }

    filter1.FilterIndex = 2;
    filter1.FilterID1 = (VESC_CAN_CMD_STATUS_4 << 8) | VESC_CAN_ID;
    filter1.RxBufferIndex = 2;
    if (HAL_FDCAN_ConfigFilter(&hfdcan2, &filter1) != HAL_OK)
    {
        Error_Handler();
    }

    filter1.FilterIndex = 3;
    filter1.FilterID1 = (VESC_CAN_CMD_STATUS_5 << 8) | VESC_CAN_ID;
    filter1.RxBufferIndex = 3;
    if (HAL_FDCAN_ConfigFilter(&hfdcan2, &filter1) != HAL_OK)
    {
        Error_Handler();
    }

    filter1.FilterIndex = 4;
    filter1.FilterID1 = (VESC_CAN_CMD_STATUS_3 << 8) | VESC_CAN_ID;
    filter1.RxBufferIndex = 4;
    if (HAL_FDCAN_ConfigFilter(&hfdcan2, &filter1) != HAL_OK)
    {
        Error_Handler();
    }
    filter1.FilterIndex = 5;
    filter1.FilterID1 = (VESC_CAN_CMD_STATUS_6 << 8) | VESC_CAN_ID;
    filter1.RxBufferIndex = 5;
    if (HAL_FDCAN_ConfigFilter(&hfdcan2, &filter1) != HAL_OK)
    {
        Error_Handler();
    }




    filter1.IdType = FDCAN_EXTENDED_ID;
    filter1.FilterIndex = 6;
    filter1.FilterType = FDCAN_FILTER_DUAL;
    filter1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter1.FilterID1 = VESC_CAN_CMD_FILL_RX_BUFFER << 8 | VESC_OWN_ID;
    filter1.FilterID2 = VESC_CAN_CMD_FILL_RX_BUFFER_LONG << 8 | VESC_OWN_ID ;
    filter1.RxBufferIndex = 0;
    if (HAL_FDCAN_ConfigFilter(&hfdcan2, &filter1) != HAL_OK)
    {
        Error_Handler();
    }

    filter1.IdType = FDCAN_EXTENDED_ID;
    filter1.FilterIndex = 7;
    filter1.FilterType = FDCAN_FILTER_DUAL;
    filter1.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;
    filter1.FilterID1 = VESC_CAN_CMD_PROCESS_RX_BUFFER << 8 | VESC_OWN_ID;
    filter1.FilterID2 = VESC_CAN_CMD_PROCESS_RX_BUFFER_SHORT << 8 | VESC_OWN_ID ;
    filter1.RxBufferIndex = 1;
    if (HAL_FDCAN_ConfigFilter(&hfdcan2, &filter1) != HAL_OK)
    {
        Error_Handler();
    }

    filter1.IdType = FDCAN_EXTENDED_ID;
    filter1.FilterIndex = 8;
    filter1.FilterType = FDCAN_FILTER_MASK;
    filter1.FilterConfig = FDCAN_FILTER_TO_RXBUFFER;
    filter1.FilterID1 = VESC_CAN_ID;
    filter1.FilterID2 = 0x000000FF;
    filter1.RxBufferIndex = 6;
    if (HAL_FDCAN_ConfigFilter(&hfdcan2, &filter1) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_FDCAN_ConfigGlobalFilter(&hfdcan2, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO1,FDCAN_ACCEPT_IN_RX_FIFO1 ,FDCAN_ACCEPT_IN_RX_FIFO1 );

    HAL_NVIC_SetPriority(FDCAN2_IT0_IRQn, 0, 1);
    HAL_NVIC_SetPriority(FDCAN2_IT1_IRQn, 0, 1);
    HAL_NVIC_SetPriority(FDCAN_CAL_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
    HAL_NVIC_EnableIRQ(FDCAN2_IT1_IRQn);
    HAL_NVIC_EnableIRQ(FDCAN_CAL_IRQn);

    HAL_FDCAN_Start(&hfdcan2);

    /* __HAL_FDCAN_ENABLE_IT(&hfdcan2, FDCAN_IE_RF0NE | FDCAN_IE_RF1NE | FDCAN_IE_DRXE); */
    FDCAN2->ILE |= 1;
    FDCAN2->IE = FDCAN_IE_RF0NE | FDCAN_IE_RF1NE | FDCAN_IE_DRXE;

    can_vesc_clear_buffers();
}

extern int32_t speed, battery_current, battery_voltage_controller, mot_temperature, con_temperature, wheel_circumfence, amphours_total, amphours_regen_total;
extern float wh_left, wh_total, wh_regen_total, distance_total;
extern uint8_t draw_power_trigger, draw_distances_trigger, draw_speed_trigger, draw_temperatures_trigger, brake, controller_mode, draw_battery_voltage_trigger, draw_brake_trigger, draw_controller_mode_trigger;
extern settings_t settings;


CRITICAL void can_process(void) {
    if (!can_available) return;
    /* LV_LOG_INFO("CAN data available"); */
    FDCAN_RxHeaderTypeDef header;
    uint8_t data[8] = {0};
    vesc_status_1 status1;
    vesc_status_2 status2;
    vesc_status_3 status3;
    vesc_status_4 status4;
    vesc_status_5 status5;
    vesc_status_6 status6;
    vesc_rx_buffer *buf = NULL; 
    can_vesc_check_buffers();

    // status message have their own buffers, blobs go into fifo
    while(HAL_FDCAN_GetRxFifoFillLevel(&hfdcan2, FDCAN_RX_FIFO0) > 0) {
        HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &header, data);

        switch (header.Identifier >> 8) {
            case VESC_CAN_CMD_FILL_RX_BUFFER:
                CAN_LOG("CAN_BUFFER_FILL");
                CAN_LOG("%04X : %02X%02X%02X%02X%02X%02X%02X%02X", header.Identifier, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
                buf = can_vesc_get_buffer(header.Identifier & 0x00FF, data[0]); 
                if (NULL == buf) {
                    LV_LOG_ERROR("No buffer found");
                    break;
                }
                buf->id = header.Identifier & 0x000000FF;
                buf->offset = data[0];
                buf->length = data[0] + header.DataLength - 1;
                memcpy(buf->data + buf->offset, data + 1, header.DataLength - 1);
                buf->last_filled = HAL_GetTick();
                buf->empty = 0;
                break;
            case VESC_CAN_CMD_FILL_RX_BUFFER_LONG:
                CAN_LOG("CAN_BUFFER_FILL_LONG");
                CAN_LOG("%04X : %02X%02X%02X%02X%02X%02X%02X%02X", header.Identifier, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
                ssize_t offset = data[0] << 8 | data[1];
                buf = can_vesc_get_buffer(header.Identifier & 0x00FF, offset); 
                if (NULL == buf) {
                    LV_LOG_ERROR("No buffer found");
                    break;
                }
                buf->id = header.Identifier & 0x000000FF;
                buf->offset = offset;
                buf->length = offset + header.DataLength - 2;
                memcpy(buf->data + buf->offset, data + 2, header.DataLength - 2);
                buf->last_filled = HAL_GetTick();
                buf->empty = 0;
                break;
            default:
                break;
        }
    }

    while(HAL_FDCAN_GetRxFifoFillLevel(&hfdcan2, FDCAN_RX_FIFO1) > 0) {
        HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO1, &header, data);

        switch (header.Identifier >> 8) {
            case VESC_CAN_CMD_PROCESS_RX_BUFFER_SHORT:
                // message is in 
                CAN_LOG("CAN_BUFFER_PROCESS_SHORT");
                CAN_LOG("%04X : %02X%02X%02X%02X%02X%02X%02X%02X", header.Identifier, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
                comm_process_vesc_msg(data + 2, header.DataLength - 2);
                break;
            case VESC_CAN_CMD_PROCESS_RX_BUFFER:
                CAN_LOG("CAN_BUFFER_PROCESS");
                CAN_LOG("%04X : %02X%02X%02X%02X%02X%02X%02X%02X", header.Identifier, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
                int16_t length = data[2] << 8 | data[3];
                uint16_t crc = data[4] << 8 | data[5];
                buf = can_vesc_get_buffer(header.Identifier & 0x00FF,length);
                if (NULL == buf) {
                    LV_LOG_ERROR("No buffer found");
                    break;
                }
                buf->crc = crc16(buf->data, buf->length);
                if (crc != buf->crc) {
                    LV_LOG_ERROR("Processed data crc mismatch: %04X vs %04X", crc, buf->crc);
                }

                comm_process_vesc_msg(buf->data, buf->length);

                memset(buf, 0x00, sizeof(vesc_rx_buffer));
                buf->empty = 1;
                CAN_LOG("Process done");
                break;
            default:
                break;
        }
    }

    if (HAL_FDCAN_IsRxBufferMessageAvailable(&hfdcan2, FDCAN_RX_BUFFER0)) {
        HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_BUFFER0, &header, (uint8_t *) &status1);

        // convert
        int32_t rpm = __ntohl(status1.rpm); 
        if (rpm == 0) {
            clock_set_wheelspeed_timer(0);
            speed = 0;
        } else {
            if (mconf_actual) {
                rpm /= (mcconf.motor_poles >> 1);
            } else {
                rpm /= 10; // default
            }
            
            clock_set_wheelspeed_timer(rpm);
            speed = (6 * settings.wheel_circumfence * rpm) / 10; // cm/h
        }
        //battery_current = (int32_t) (((int32_t) __ntohs(status1.current_motor)) * ((int32_t) 100)); // provided in deciamps 
        // ignore all others
        draw_speed_trigger = 1;
        /* draw_power_trigger = 1;  */
    }

    if (HAL_FDCAN_IsRxBufferMessageAvailable(&hfdcan2, FDCAN_RX_BUFFER1)) {
        HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_BUFFER1, &header, (uint8_t *) &status2);
        amphours_total = (int32_t) __ntohl(status2.amp_hours_consumed); // in mah
        amphours_regen_total = (int32_t) __ntohl(status2.amp_hours_regen); // in mah
    }

    if (HAL_FDCAN_IsRxBufferMessageAvailable(&hfdcan2, FDCAN_RX_BUFFER2)) {
        HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_BUFFER2, &header, (uint8_t *) &status4);
        mot_temperature = (int32_t) ((int16_t) __ntohs(status4.motor_temp));
        con_temperature = (int32_t) ((int16_t) __ntohs(status4.mosfet_temp));
        battery_current = (int32_t) ((int16_t) __ntohs(status4.current_in));
        battery_current *= 100; // as provided in deciamps
        draw_power_trigger = 1; 
        if (HAL_GetTick() - drw_tmp_timeout > 1000) {
            draw_temperatures_trigger = 1; // temperature is not required to print every 25-100ms
            drw_tmp_timeout = HAL_GetTick();
        }
        
    }

    if (HAL_FDCAN_IsRxBufferMessageAvailable(&hfdcan2, FDCAN_RX_BUFFER3)) {
        HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_BUFFER3, &header, (uint8_t *) &status5);
        battery_voltage_controller = (int32_t) __ntohs(status5.voltage) * 100;
        draw_battery_voltage_trigger = 1;
    }

    if (HAL_FDCAN_IsRxBufferMessageAvailable(&hfdcan2, FDCAN_RX_BUFFER4)) {
        HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_BUFFER4, &header, (uint8_t *) &status3);
        wh_total = (float) __ntohl(status3.watt_hours_consumed);
        wh_regen_total = (float) __ntohl(status3.watt_hours_regen);
        CAN_LOG("SM3: %ld %ld", wh_total, wh_regen_total);
    }

    if (HAL_FDCAN_IsRxBufferMessageAvailable(&hfdcan2, FDCAN_RX_BUFFER5)) {
        HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_BUFFER5, &header, (uint8_t *) &status6);
        wh_left = (float) __ntohl(status6.watt_hours_left);
        distance_total = (float) __ntohl(status6.distance_total);
        CAN_LOG("SM6: %ld %ld", wh_left, distance_total);
        draw_distances_trigger = 1; // only trigger on one
    }

    if (HAL_FDCAN_IsRxBufferMessageAvailable(&hfdcan2, FDCAN_RX_BUFFER6)) {
        HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_BUFFER6, &header, data);
        LV_LOG_INFO("Message %02lX for %02lX received, discarding", header.Identifier >> 8, header.Identifier & 0x00FF);
    }

    can_available = 0;
    led_blue(0);
}

void can_send(uint32_t id, uint8_t *data, ssize_t length) {
    /* LV_LOG_INFO("%04X : %02X%02X%02X%02X%02X%02X%02X%02X", id, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]); */
    /* volatile uint32_t brp = FDCAN2->TXBRP; */
    /* volatile uint32_t bc = FDCAN2->TXBC; */
    volatile uint32_t pindex = (FDCAN2->TXFQS & FDCAN_TXFQS_TFQPI) >> FDCAN_TXFQS_TFQPI_Pos; 
    /* volatile uint32_t gindex = (FDCAN2->TXFQS & FDCAN_TXFQS_TFQGI) >> FDCAN_TXFQS_TFQGI_Pos;  */

    // copy
    uint32_t w1, w2;
    w1 = FDCAN_ESI_ACTIVE | FDCAN_EXTENDED_ID | FDCAN_DATA_FRAME | id;
    w2 = FDCAN_NO_TX_EVENTS | FDCAN_CLASSIC_CAN | FDCAN_BRS_OFF | (length & 0x000000FF) << 16U;
    uint32_t *txaddr = (uint32_t *) (hfdcan2.msgRam.TxBufferSA + (pindex * hfdcan2.Init.TxElmtSize * 4U)); 
    *txaddr++ = w1;
    *txaddr++ = w2;
    int j = 0;
    for (; j < length / 4; j++, length-=4) {
        *txaddr++ = data[(j*4)+3] << 24U | data[(j*4)+2] << 16U | data[(j*4)+1] << 8U | data[(j*4)];
    }
    *txaddr = 0;
    for (int i = 0; i < length; i++) {
        *txaddr |= data[(j*4)+i] << (8U * i);
    }


    // transmit
    FDCAN2->TXBAR = (1 << pindex); 

    /* Vif (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &header, data) != HAL_OK) { */
    /*     LV_LOG_ERROR("Could not send can data"); */
    /* } */
    /* volatile uint32_t txfqs = FDCAN2->TXFQS; */
    /* LV_LOG_INFO("Send can to %08lX length %d %08X %08X %08X %08X", id, length, brp, pindex, txfqs, bc); */
}

void can_vesc_send(uint32_t command, uint8_t *data, ssize_t data_length) {
    if (data_length > 8) {
        LV_LOG_INFO("Invalid length: %d", data_length);
    } else {
        can_send(command << 8 | VESC_CAN_ID, data, data_length);
    }
}

void can_vesc_shutdown(void) {
    can_vesc_send(VESC_CAN_CMD_SHUTDOWN, NULL, 0);
}

void can_vesc_set_battery_current_limit(int32_t min, int32_t max, int store) {
    // in mah
    /* int32_t buffer[1]; */
    /* buffer[0] = __REV(max); */
    /* buffer[1] = __REV(min); */
    /* can_vesc_send(store? VESC_CAN_CMD_SET_CURRENT_LIMIT_IN : VESC_CAN_CMD_SET_CURRENT_LIMIT_IN_STORE, (uint8_t *) buffer, sizeof(int32_t) * 2); */
}
void can_vesc_set_motor_current_limit(int32_t min, int32_t max, int store) {
    // in mah
    int32_t buffer[2];
    buffer[0] = __REV(max);
    buffer[1] = __REV(min);
    can_vesc_send(store? VESC_CAN_CMD_SET_CURRENT_LIMIT : VESC_CAN_CMD_SET_CURRENT_LIMIT_STORE, (uint8_t *) buffer, sizeof(int32_t) * 2);
}

void can_vesc_set_current_scale_max(void) {
    uint8_t buffer[4];
    float assist_level = ((float) settings.assist_last) * (1.0f/((float) settings.assist_levels));
    buffer_append_float32_auto(buffer, assist_level);
    can_vesc_send(VESC_CAN_CMD_SET_CURRENT_SCALE_MAX, buffer, 4);
}

