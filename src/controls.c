#include "controls.h"

#include "at32f415_dma.h"
#include "at32f415_tmr.h"
#include "at32f415_crm.h"
#include "config.h"

adc_data_t adc;
extern volatile uint32_t timer_counter;

uint16_t external_temperature_old_value = 0;

uint8_t power_button_state = BUTTON_RELEASED;
uint8_t up_button_state = BUTTON_RELEASED;
uint8_t down_button_state = BUTTON_RELEASED;
uint8_t nc_button_state = BUTTON_RELEASED;

uint32_t power_button_start = 0;
uint32_t up_button_start = 0;
uint32_t down_button_start = 0;
uint32_t nc_button_start = 0;
uint32_t button_backoff = 0, button_backoff_start = 0;

uint8_t power_button_count = 0;
uint8_t nc_button_count = 0;
uint8_t down_button_count = 0;

int32_t ext_temp_store = 0; // store temperature adc value in case button is pressed

#define DEBUG
#ifdef DEBUG 
uint32_t *debug = (uint32_t *) 0x20007D00;
#endif

/**
 * static functions
 */
static void adc_config(void) {
    adc_base_config_type adc_base_struct;
    crm_periph_clock_enable(CRM_ADC1_PERIPH_CLOCK, TRUE); // use default perph clock
    crm_adc_clock_div_set(CRM_ADC_DIV_6); //

    adc_base_default_para_init(&adc_base_struct);
    adc_base_struct.sequence_mode = TRUE;
    adc_base_struct.repeat_mode = FALSE;
    adc_base_struct.data_align = ADC_RIGHT_ALIGNMENT;
    adc_base_struct.ordinary_channel_length = 5;
    adc_base_config(ADC1, &adc_base_struct);
    adc_ordinary_channel_set(ADC1, ADC_CHANNEL_16, 1, ADC_SAMPLETIME_239_5);
    adc_ordinary_channel_set(ADC1, ADC_CHANNEL_17, 2, ADC_SAMPLETIME_239_5);
    adc_ordinary_channel_set(ADC1, VOLTAGE_DETECT_ADC_CH, 3, ADC_SAMPLETIME_239_5);
    adc_ordinary_channel_set(ADC1, EXT_TEMP_ADC_CH, 4, ADC_SAMPLETIME_239_5);
    adc_ordinary_channel_set(ADC1, NC_BUTTON_ADC_CH, 5, ADC_SAMPLETIME_239_5);

    adc_ordinary_conversion_trigger_set(ADC1, ADC12_ORDINARY_TRIG_SOFTWARE, TRUE);

    adc_dma_mode_enable(ADC1, TRUE);
    adc_tempersensor_vintrv_enable(TRUE); // required only for temp sensor

    adc_enable(ADC1, TRUE);
    adc_calibration_init(ADC1);
    while(adc_calibration_init_status_get(ADC1));
    adc_calibration_start(ADC1);
    while(adc_calibration_status_get(ADC1));

}

static void dma_config(void)
{
    dma_init_type dma_init_struct;
    crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
    dma_reset(DMA1_CHANNEL1);
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = 5;
    dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_base_addr = (uint32_t) &adc;
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD;
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.peripheral_base_addr = (uint32_t)&(ADC1->odt);
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_init_struct.loop_mode_enable = TRUE;
    dma_init(DMA1_CHANNEL1, &dma_init_struct);

    dma_channel_enable(DMA1_CHANNEL1, TRUE);
}

static void tmr_config(void) {

    crm_periph_clock_enable(CRM_TMR9_PERIPH_CLOCK, TRUE);
    tmr_output_config_type tmr_oc_init_structure;
    /* tmre base configuration */

    tmr_base_init(TMR9,  BUTTON_MEASURE_PERIOD * 1000, TIMER_FREQ(1000000));
    tmr_cnt_dir_set(TMR9, TMR_COUNT_UP);
    tmr_clock_source_div_set(TMR9, TMR_CLOCK_DIV1);

    /* output compare toggle mode configuration: channel1 */
    tmr_output_default_para_init(&tmr_oc_init_structure);
    tmr_oc_init_structure.oc_mode = TMR_OUTPUT_CONTROL_SWITCH;
    tmr_oc_init_structure.oc_idle_state = FALSE;
    tmr_oc_init_structure.oc_polarity = TMR_OUTPUT_ACTIVE_LOW;
    tmr_oc_init_structure.oc_output_state = FALSE;
    tmr_output_channel_config(TMR9, TMR_SELECT_CHANNEL_1, &tmr_oc_init_structure);
    tmr_channel_value_set(TMR9, TMR_SELECT_CHANNEL_1, 1000);
    tmr_counter_enable(TMR9, TRUE);
    tmr_interrupt_enable(TMR9, TMR_C1_INT, TRUE);
    
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_irq_enable(TMR1_BRK_TMR9_IRQn, 0, 0);
}

/**
 * public functions
 */

void controls_init(void) {

    // configure pins
    //
    gpio_init_type gpio_initstructure;

    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL; // input
    gpio_initstructure.gpio_pull           = POWER_BUTTON_PULL; // external pullup
    gpio_initstructure.gpio_mode           = POWER_BUTTON_MODE;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM; // input
    gpio_initstructure.gpio_pins           = POWER_BUTTON_PIN;
    gpio_init(POWER_BUTTON_GPIO, &gpio_initstructure);

    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL; // input
    gpio_initstructure.gpio_pull           = UP_BUTTON_PULL; // external pullup
    gpio_initstructure.gpio_mode           = UP_BUTTON_MODE;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM; // input
    gpio_initstructure.gpio_pins           = UP_BUTTON_PIN;
    gpio_init(UP_BUTTON_GPIO, &gpio_initstructure);

    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL; // input
    gpio_initstructure.gpio_pull           = DOWN_BUTTON_PULL; // external pullup
    gpio_initstructure.gpio_mode           = DOWN_BUTTON_MODE;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM; // input
    gpio_initstructure.gpio_pins           = DOWN_BUTTON_PIN;
    gpio_init(DOWN_BUTTON_GPIO, &gpio_initstructure);

    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL; // input
    gpio_initstructure.gpio_pull           = POWER_LATCH_PULL; // external pullup
    gpio_initstructure.gpio_mode           = POWER_LATCH_MODE;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM; // input
    gpio_initstructure.gpio_pins           = POWER_LATCH_PIN;
    gpio_init(POWER_LATCH_GPIO, &gpio_initstructure);

    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL; // input
    gpio_initstructure.gpio_pull           = VOLTAGE_DETECT_PULL; // external pullup
    gpio_initstructure.gpio_mode           = VOLTAGE_DETECT_MODE;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM; // input
    gpio_initstructure.gpio_pins           = VOLTAGE_DETECT_PIN;
    gpio_init(VOLTAGE_DETECT_GPIO, &gpio_initstructure);

    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL; // input
    gpio_initstructure.gpio_pull           = NC_BUTTON_PULL; // external pullup
    gpio_initstructure.gpio_mode           = NC_BUTTON_MODE;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM; // input
    gpio_initstructure.gpio_pins           = NC_BUTTON_PIN;
    gpio_init(NC_BUTTON_GPIO, &gpio_initstructure);


    // configure ADC
    //
    dma_config();
    adc_config();
    tmr_config();

}

void power_enable(void) { POWER_LATCH_GPIO->scr = POWER_LATCH_PIN; }
void power_disable(void) { POWER_LATCH_GPIO->clr = POWER_LATCH_PIN; }

/* int up_BUTTON_PRESSED(void) { return up_button_cnt > BUTTON_COUNT && up_button_cnt < BUTTON_HOLD ? 1 : 0; } */
/* int down_BUTTON_PRESSED(void) { return down_button_cnt > BUTTON_COUNT && down_button_cnt < BUTTON_HOLD ? 1 : 0; } */
/* int power_BUTTON_PRESSED(void) { return power_button_cnt > BUTTON_COUNT && power_button_cnt < BUTTON_HOLD ? 1 : 0; } */
/* int nc_BUTTON_PRESSED(void) { return nc_button_cnt > BUTTON_COUNT && nc_button_cnt < BUTTON_HOLD ? 1 : 0; } */
/*  */
/* int up_button_hold(void) { return up_button_cnt > BUTTON_HOLD ? up_button_cnt : 0; } */
/* int down_button_hold(void) { return down_button_cnt > BUTTON_HOLD ? down_button_cnt : 0; } */
/* int power_button_hold(void) { return power_button_cnt > BUTTON_HOLD ? power_button_cnt : 0; } */
/* int nc_button_hold(void) { return nc_button_cnt > BUTTON_HOLD ? nc_button_cnt : 0; } */


static inline int power_button_measure(void) {
    if (POWER_BUTTON_MODE == GPIO_MODE_INPUT) return (POWER_BUTTON_GPIO->idt & (POWER_BUTTON_PIN)) > 0;
    return adc.power_button <= 10;
}

static inline int down_button_measure(void) {
    if (DOWN_BUTTON_MODE == GPIO_MODE_INPUT) return (DOWN_BUTTON_GPIO->idt & (DOWN_BUTTON_PIN)) == 0;
    return adc.down_button <= 10;
}

static inline int up_button_measure(void) {
    if (UP_BUTTON_MODE == GPIO_MODE_INPUT) return (UP_BUTTON_GPIO->idt & (UP_BUTTON_PIN)) == 0;
    return adc.up_button <= 10;
}

static inline int nc_button_measure(void) {
    if (NC_BUTTON_MODE == GPIO_MODE_INPUT) return (NC_BUTTON_GPIO->idt & (NC_BUTTON_PIN)) > 0;
    return adc.nc_button <= 10;
}

int32_t int_temp(void) {
    // data from artery example code converted to return 24.8s fixed point format
#define ADC_TEMP_BASE                    ((int32_t)(1638))
#define ADC_TEMP_SLOPE                   ((int32_t)(-4551))
    return (((ADC_TEMP_BASE - adc.temperature_int) << 16) / ADC_TEMP_SLOPE) + (25 << 8);
}

int32_t ext_temp(void) {
    // result is in 24.8s format
    // value is stored for when button is pressed as it pulls to ground
    if (adc.temperature_ext <= 10) {
        return ext_temp_store;
    } else {
        return ext_temp_store = NTC_ADC2Temperature(adc.temperature_ext);
    }
}

static inline void measure_buttons(void) {
    // trigger adc
    adc_ordinary_software_trigger_enable(ADC1, TRUE);
    // button press addition
    if (timer_counter < button_backoff + button_backoff_start) return;
    if (up_button_measure()) {
        if (up_button_state == BUTTON_RELEASED) {
            up_button_start = timer_counter;
            up_button_state = BUTTON_DOWN;
        } else if (up_button_state == BUTTON_DOWN && timer_counter - up_button_start > BUTTON_LONG_PRESS_TIME) {
            up_button_state = BUTTON_LONG_PRESSED;
        }
    } else {
        if (up_button_state == BUTTON_DOWN && timer_counter - up_button_start > BUTTON_DEBOUNCE_TIME) {
            up_button_state = up_button_state == BUTTON_DOWN ? BUTTON_PRESSED : BUTTON_RELEASED;
            up_button_start = 0;
        } else if (up_button_state != BUTTON_PRESSED) {
            up_button_state = BUTTON_RELEASED;
            up_button_start = 0;
        }
    }

    if (down_button_measure()) {
        if (down_button_state == BUTTON_RELEASED) {
            down_button_start = timer_counter;
            down_button_state = BUTTON_DOWN;
        } else if (down_button_state == BUTTON_DOWN && timer_counter - down_button_start > BUTTON_LONG_PRESS_TIME) {
            down_button_state = BUTTON_LONG_PRESSED;
        }
    } else {
        if (down_button_state == BUTTON_DOWN && timer_counter - down_button_start > BUTTON_DEBOUNCE_TIME) {
            down_button_state = down_button_state == BUTTON_DOWN ? BUTTON_PRESSED : BUTTON_RELEASED;
            down_button_start = 0;
        } else if (down_button_state != BUTTON_PRESSED) {
            down_button_state = BUTTON_RELEASED;
            down_button_start = 0;
        }
    }

    if (power_button_measure()) {
        if (power_button_state == BUTTON_RELEASED) {
            power_button_start = timer_counter;
            power_button_state = BUTTON_DOWN;
        } else if (power_button_state == BUTTON_DOWN && timer_counter - power_button_start > BUTTON_LONG_PRESS_TIME) {
            power_button_state = BUTTON_LONG_PRESSED;
        }
    } else {
        if (power_button_state == BUTTON_DOWN && timer_counter - power_button_start > BUTTON_DEBOUNCE_TIME) {
            power_button_state = power_button_state == BUTTON_DOWN ? BUTTON_PRESSED : BUTTON_RELEASED;
            power_button_start = 0;
        } else if (power_button_state != BUTTON_PRESSED) {
            power_button_state = BUTTON_RELEASED;
            power_button_start = 0;
        }
    }
    if (nc_button_measure()) {
        if (nc_button_state == BUTTON_RELEASED) {
            nc_button_start = timer_counter;
            nc_button_state = BUTTON_DOWN;
        } else if (timer_counter - nc_button_start > BUTTON_LONG_PRESS_TIME) {
            nc_button_state = BUTTON_LONG_PRESSED;
        }
    } else {
        if (nc_button_state == BUTTON_DOWN && timer_counter - nc_button_start > BUTTON_DEBOUNCE_TIME) {
            nc_button_state = nc_button_state == BUTTON_DOWN ? BUTTON_PRESSED : BUTTON_RELEASED;
            nc_button_start = 0;
        } else if (nc_button_state != BUTTON_PRESSED) {
            nc_button_state = BUTTON_RELEASED;
            nc_button_start = 0;
        }
    }
}

void button_release(uint8_t id, uint32_t backoff) {
    button_backoff_start = timer_counter;
    button_backoff = backoff;
    switch(id) {
        case BUTTON_ID_UP:
            up_button_state = BUTTON_RELEASED;
            up_button_start = 0;
            break;
        case BUTTON_ID_DOWN:
            down_button_state = BUTTON_RELEASED;
            down_button_start = 0;
            break;
        case BUTTON_ID_NC:
            nc_button_state = BUTTON_RELEASED;
            nc_button_start = 0;
            break;
        case BUTTON_ID_POWER:
            power_button_state = BUTTON_RELEASED;
            power_button_start = 0;
            break;
    }
}


void TMR1_BRK_TMR9_IRQHandler(void)
{
    if(tmr_interrupt_flag_get(TMR9, TMR_C1_FLAG) != RESET)
    {
        tmr_flag_clear(TMR9, TMR_C1_FLAG );
        measure_buttons();
    }
}

/**
* The NTC table has 1025 interpolation points.
* Unit:0.00390625 °C (results in 8.8 format)
* All credits: https://www.sebulli.com/ntc/index.php?lang=en&points=1024&unit=0.00390625&resolution=12+Bit&circuit=pullup&resistor=10000&r25=10000&beta=3950&test_resistance=10000&tmin=-20&tmax=70
*
*/
int NTC_table[1025] = {
  105972, 90128, 74284, 66383, 61277, 57570, 
  54690, 52352, 50395, 48718, 47256, 45963, 
  44806, 43760, 42808, 41936, 41131, 40384, 
  39689, 39038, 38428, 37853, 37310, 36795, 
  36307, 35842, 35399, 34976, 34571, 34182, 
  33810, 33451, 33106, 32774, 32453, 32143, 
  31843, 31553, 31272, 30999, 30734, 30477, 
  30228, 29985, 29748, 29518, 29294, 29075, 
  28861, 28653, 28449, 28250, 28056, 27865, 
  27679, 27497, 27319, 27144, 26972, 26804, 
  26639, 26478, 26319, 26163, 26010, 25860, 
  25712, 25567, 25424, 25283, 25145, 25009, 
  24875, 24744, 24614, 24486, 24360, 24236, 
  24114, 23993, 23875, 23757, 23642, 23528, 
  23415, 23304, 23195, 23087, 22980, 22874, 
  22770, 22667, 22566, 22465, 22366, 22268, 
  22171, 22075, 21980, 21887, 21794, 21702, 
  21611, 21522, 21433, 21345, 21258, 21172, 
  21087, 21002, 20919, 20836, 20754, 20673, 
  20593, 20513, 20434, 20356, 20279, 20202, 
  20126, 20051, 19976, 19902, 19828, 19756, 
  19683, 19612, 19541, 19471, 19401, 19331, 
  19263, 19195, 19127, 19060, 18993, 18927, 
  18862, 18797, 18732, 18668, 18605, 18541, 
  18479, 18416, 18355, 18293, 18232, 18172, 
  18112, 18052, 17993, 17934, 17876, 17818, 
  17760, 17703, 17646, 17589, 17533, 17477, 
  17421, 17366, 17311, 17257, 17203, 17149, 
  17096, 17042, 16990, 16937, 16885, 16833, 
  16781, 16730, 16679, 16628, 16578, 16527, 
  16478, 16428, 16379, 16329, 16281, 16232, 
  16184, 16136, 16088, 16040, 15993, 15946, 
  15899, 15853, 15806, 15760, 15714, 15669, 
  15623, 15578, 15533, 15488, 15444, 15399, 
  15355, 15311, 15268, 15224, 15181, 15138, 
  15095, 15052, 15009, 14967, 14925, 14883, 
  14841, 14800, 14758, 14717, 14676, 14635, 
  14594, 14554, 14513, 14473, 14433, 14393, 
  14354, 14314, 14275, 14235, 14196, 14157, 
  14119, 14080, 14041, 14003, 13965, 13927, 
  13889, 13851, 13814, 13776, 13739, 13702, 
  13665, 13628, 13591, 13554, 13518, 13482, 
  13445, 13409, 13373, 13337, 13302, 13266, 
  13231, 13195, 13160, 13125, 13090, 13055, 
  13020, 12985, 12951, 12916, 12882, 12848, 
  12814, 12780, 12746, 12712, 12678, 12645, 
  12611, 12578, 12545, 12512, 12479, 12446, 
  12413, 12380, 12347, 12315, 12282, 12250, 
  12218, 12185, 12153, 12121, 12089, 12058, 
  12026, 11994, 11963, 11931, 11900, 11868, 
  11837, 11806, 11775, 11744, 11713, 11682, 
  11652, 11621, 11591, 11560, 11530, 11499, 
  11469, 11439, 11409, 11379, 11349, 11319, 
  11289, 11260, 11230, 11200, 11171, 11141, 
  11112, 11083, 11054, 11024, 10995, 10966, 
  10937, 10908, 10880, 10851, 10822, 10794, 
  10765, 10737, 10708, 10680, 10651, 10623, 
  10595, 10567, 10539, 10511, 10483, 10455, 
  10427, 10399, 10372, 10344, 10317, 10289, 
  10261, 10234, 10207, 10179, 10152, 10125, 
  10098, 10071, 10044, 10017, 9990, 9963, 9936, 
  9909, 9882, 9856, 9829, 9802, 9776, 9749, 
  9723, 9697, 9670, 9644, 9618, 9591, 9565, 
  9539, 9513, 9487, 9461, 9435, 9409, 9383, 
  9357, 9332, 9306, 9280, 9255, 9229, 9203, 
  9178, 9152, 9127, 9102, 9076, 9051, 9026, 
  9000, 8975, 8950, 8925, 8900, 8875, 8850, 
  8825, 8800, 8775, 8750, 8725, 8700, 8676, 
  8651, 8626, 8601, 8577, 8552, 8528, 8503, 
  8479, 8454, 8430, 8405, 8381, 8357, 8332, 
  8308, 8284, 8259, 8235, 8211, 8187, 8163, 
  8139, 8115, 8091, 8067, 8043, 8019, 7995, 
  7971, 7947, 7923, 7900, 7876, 7852, 7828, 
  7805, 7781, 7757, 7734, 7710, 7687, 7663, 
  7640, 7616, 7593, 7569, 7546, 7522, 7499, 
  7476, 7452, 7429, 7406, 7382, 7359, 7336, 
  7313, 7290, 7266, 7243, 7220, 7197, 7174, 
  7151, 7128, 7105, 7082, 7059, 7036, 7013, 
  6990, 6967, 6944, 6921, 6899, 6876, 6853, 
  6830, 6807, 6785, 6762, 6739, 6716, 6694, 
  6671, 6648, 6626, 6603, 6580, 6558, 6535, 
  6513, 6490, 6468, 6445, 6423, 6400, 6378, 
  6355, 6333, 6310, 6288, 6265, 6243, 6220, 
  6198, 6176, 6153, 6131, 6108, 6086, 6064, 
  6041, 6019, 5997, 5975, 5952, 5930, 5908, 
  5886, 5863, 5841, 5819, 5797, 5774, 5752, 
  5730, 5708, 5686, 5663, 5641, 5619, 5597, 
  5575, 5553, 5531, 5509, 5486, 5464, 5442, 
  5420, 5398, 5376, 5354, 5332, 5310, 5288, 
  5266, 5244, 5221, 5199, 5177, 5155, 5133, 
  5111, 5089, 5067, 5045, 5023, 5001, 4979, 
  4957, 4935, 4913, 4891, 4869, 4847, 4825, 
  4803, 4781, 4759, 4737, 4715, 4693, 4671, 
  4649, 4627, 4605, 4583, 4561, 4539, 4517, 
  4495, 4473, 4451, 4429, 4407, 4385, 4363, 
  4341, 4319, 4297, 4275, 4253, 4230, 4208, 
  4186, 4164, 4142, 4120, 4098, 4076, 4054, 
  4032, 4010, 3988, 3966, 3943, 3921, 3899, 
  3877, 3855, 3833, 3811, 3788, 3766, 3744, 
  3722, 3700, 3677, 3655, 3633, 3611, 3589, 
  3566, 3544, 3522, 3499, 3477, 3455, 3433, 
  3410, 3388, 3366, 3343, 3321, 3298, 3276, 
  3254, 3231, 3209, 3186, 3164, 3141, 3119, 
  3096, 3074, 3051, 3029, 3006, 2984, 2961, 
  2939, 2916, 2893, 2871, 2848, 2825, 2803, 
  2780, 2757, 2734, 2712, 2689, 2666, 2643, 
  2620, 2598, 2575, 2552, 2529, 2506, 2483, 
  2460, 2437, 2414, 2391, 2368, 2345, 2322, 
  2299, 2275, 2252, 2229, 2206, 2183, 2159, 
  2136, 2113, 2089, 2066, 2043, 2019, 1996, 
  1972, 1949, 1925, 1902, 1878, 1855, 1831, 
  1807, 1784, 1760, 1736, 1712, 1689, 1665, 
  1641, 1617, 1593, 1569, 1545, 1521, 1497, 
  1473, 1449, 1425, 1400, 1376, 1352, 1328, 
  1303, 1279, 1255, 1230, 1206, 1181, 1157, 
  1132, 1107, 1083, 1058, 1033, 1009, 984, 
  959, 934, 909, 884, 859, 834, 809, 784, 759, 
  733, 708, 683, 657, 632, 606, 581, 555, 530, 
  504, 478, 453, 427, 401, 375, 349, 323, 297, 
  271, 245, 218, 192, 166, 139, 113, 87, 60, 
  33, 7, -20, -47, -74, -101, -128, -155, -182, 
  -209, -236, -264, -291, -318, -346, -373, 
  -401, -429, -457, -484, -512, -540, -568, 
  -597, -625, -653, -681, -710, -738, -767, 
  -796, -824, -853, -882, -911, -940, -969, 
  -999, -1028, -1057, -1087, -1117, -1146, 
  -1176, -1206, -1236, -1266, -1296, -1327, 
  -1357, -1387, -1418, -1449, -1479, -1510, 
  -1541, -1572, -1604, -1635, -1666, -1698, 
  -1729, -1761, -1793, -1825, -1857, -1889, 
  -1922, -1954, -1987, -2020, -2052, -2085, 
  -2118, -2152, -2185, -2219, -2252, -2286, 
  -2320, -2354, -2388, -2423, -2457, -2492, 
  -2527, -2562, -2597, -2632, -2667, -2703, 
  -2739, -2775, -2811, -2847, -2884, -2920, 
  -2957, -2994, -3031, -3069, -3106, -3144, 
  -3182, -3220, -3259, -3297, -3336, -3375, 
  -3414, -3454, -3493, -3533, -3573, -3614, 
  -3654, -3695, -3736, -3777, -3819, -3861, 
  -3903, -3945, -3988, -4031, -4074, -4118, 
  -4161, -4205, -4250, -4294, -4339, -4385, 
  -4430, -4476, -4523, -4569, -4616, -4664, 
  -4711, -4759, -4808, -4857, -4906, -4956, 
  -5006, -5056, -5107, -5158, -5210, -5262, 
  -5315, -5368, -5422, -5476, -5531, -5586, 
  -5642, -5698, -5755, -5813, -5871, -5929, 
  -5989, -6049, -6109, -6171, -6233, -6295, 
  -6359, -6423, -6488, -6554, -6620, -6688, 
  -6756, -6825, -6896, -6967, -7039, -7112, 
  -7186, -7262, -7338, -7416, -7495, -7575, 
  -7656, -7739, -7824, -7910, -7997, -8086, 
  -8177, -8269, -8363, -8459, -8558, -8658, 
  -8761, -8866, -8973, -9083, -9196, -9311, 
  -9430, -9552, -9678, -9807, -9940, -10078, 
  -10220, -10367, -10520, -10678, -10842, -11014, 
  -11193, -11380, -11576, -11783, -12001, -12233, 
  -12479, -12742, -13025, -13332, -13667, -14036, 
  -14448, -14917, -15460, -16109, -16924, -18029, 
  -19815, -21601
};
 
 
 
/**
* \brief    Converts the ADC result into a temperature value.
*
*           P1 and p2 are the interpolating point just before and after the
*           ADC value. The function interpolates between these two points
*           The resulting code is very small and fast.
*           Only one integer multiplication is used.
*           The compiler can replace the division by a shift operation.
*
*           In the temperature range from -20°C to 70°C the error
*           caused by the usage of a table is 0.005°C
*
* \param    adc_value  The converted ADC result
* \return              The temperature in 0.00390625 °C
*
*/
int32_t NTC_ADC2Temperature(int16_t adc_value){
 
  int p1,p2;
  /* Estimate the interpolating point before and after the ADC value. */
  p1 = NTC_table[ (adc_value >> 2)  ];
  p2 = NTC_table[ (adc_value >> 2)+1];
 
  /* Interpolate between both points. */
  return p1 - ( (p1-p2) * (adc_value & 0x0003) ) / 4;
};

int32_t voltage_mcu(void) {
    return 0;
}

int32_t voltage_ebat(void) { // in mv
    return (adc.voltage_battery << 16) / 2598;//10150; // 24.8
}

uint8_t buttons_pressed(void) {
    uint8_t bt = (down_button_state << BUTTON_ID_DOWN) | (up_button_state << BUTTON_ID_UP) | (nc_button_state << BUTTON_ID_NC) | (power_button_state << BUTTON_ID_POWER);
    return bt;
}
