#include "lcd.h"
#include "hal/lcd8/gd32f303/lcd.h"
#include "uart.h"
#include "gd32f30x.h"
#include <cmsis/core/cmsis_gcc.h>

/* #define MEMORY_DEBUG */
uint16_t dummy = 0;


#ifdef DMA_WRITE

#if LVGL_VERSION_MAJOR == 9
static lv_display_t *dp = NULL;
#else
static lv_disp_drv_t *dp = NULL;
#endif
volatile int dma_ready = 1;
void dma_write(uint16_t *data, uint32_t length, uint32_t final_x, uint32_t final_y);
#endif

typedef struct ili_cmd_t {
    uint8_t cmd;
    uint8_t data[16];
    uint32_t data_length;
} ili_cmd;
#define STARTUP_COMMAND_LENGTH 14
const ili_cmd ili_startup_cmds[STARTUP_COMMAND_LENGTH] = {
		{ILI_POSITIVE_GAMMA_CORRECTION,     {0x00, 0x09, 0x16, 0x09, 0x17, 0x0A, 0x3F, 0x78, 0x4B, 0x07, 0x0E, 0x0F, 0x18, 0x1A, 0x0F}, 15}, // panel specific
		{ILI_NEGATIVE_GAMMA_CORRECTION,     {0x00, 0x1D, 0x1F, 0x05, 0x0F, 0x05, 0x33, 0x34, 0x43, 0x02, 0x0A, 0x08, 0x2E, 0x33, 0x0F}, 15}, // panel specific
        {ILI_FRAME_RATE_CONTROL_NORMAL,     {0xA0}, 1}, // Fosc,78.13hz
        {ILI_DISPLAY_INVERSION_CONTROL,     {0x02}, 1}, // 2 dot inversion 
        {ILI_POWER_CONTROL_1,               {0x12, 0x12}, 2}, // +/- 4.6895
        {ILI_POWER_CONTROL_2,               {0x41}, 1}, // VCI*6/VCI*4
        {ILI_VCOM_CONTROL_1,                {0x00, 0x25, 0x80}, 3}, // [ignore NV byte] [-1.35938] [read from vcomreg] last byte is NV memory programmed and ignored
		{ILI_DISPLAY_FUNCTION_CONTROL,      {0x02}, 1}, // if correct only sets first byte to non display area AGND 
		{ILI_DISPLAY_FUNCTION_CONTROL,      {0x02, 0x02, 0x3B}, 3},
		{ILI_MEMORY_ACCESS_CONTROL,         {0x48}, 1}, // Column inverted, BGR filters
		{ILI_INTERFACE_PIXEL_FORMAT,        {0x55}, 1}, // RGB565
        {ILI_SET_IMAGE_FUNCTION,            {0x00}, 1}, // Disable 24bit mode
        {ILI_ADJUST_CONTROL_3,              {0xA9, 0x51, 0x2C, 0x82}, 4},// default (only one bit can be changed)
		{ILI_WRITE_DISPLAY_BRIGHTNESS,      {0x7F}, 1}
};

static void write_data_8bit(uint8_t data);
static void write_cmd(uint8_t data);
static void write_data(uint16_t data);
static void lcd_set_address_window(uint32_t x, uint32_t y, uint32_t x2, uint32_t y2);

void lcd_backlight_init(void) {

    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER13);
    timer_deinit(TIMER13);

    timer_initpara.prescaler         = 120-1; // 1MHz
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 99; // 10khz
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER13 ,&timer_initpara);

    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER13,TIMER_CH_0,&timer_ocintpara);
    
    timer_channel_output_pulse_value_config(TIMER13,TIMER_CH_0, 99);
    timer_channel_output_mode_config(TIMER13,TIMER_CH_0,TIMER_OC_MODE_PWM0);
    /* timer_channel_output_shadow_config(TIMER13,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE); */

    /* tmr enable counter */
    /* timer_auto_reload_shadow_enable(TIMER13); */
    timer_enable(TIMER13);
}

void lcd_tmr_init(void) {
}

void lcd_init(void) {
    
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_AF);
    
    /* SWD remap */
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
    /* GPIOB output */
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_MAX, GPIO_PIN_ALL); // GPIOB for data bus
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_7);  // GPIOA - 7 for pwm backlight

    lcd_backlight_init();

    // init clocks

    // Read strobe
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, PIN_READ);
    // Write strobe
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, PIN_WRITE);
    // Command/Data
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, PIN_CD);
    // Chip select
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, PIN_CS);

    //CLEAR_DATA();
    SET_WRITE();
    CS_ACTIVE;
    READ_IDLE;
    WRITE_IDLE;
    COMMAND;


#ifdef DMA_WRITE

    crm_periph_clock_enable(CRM_DMA2_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_TMR1_PERIPH_CLOCK, TRUE);

    gpio_pin_remap_config(TMR1_GMUX_0010, TRUE);
    static dma_init_type dma_init_struct = {0};

    tmr_reset(TMR1);
    /* tmr enable counter */
    tmr_output_config_type tmr_output_struct;
    tmr_base_init(TMR1, 15, 0);
    /* tmr_base_init(TMR1, 11, 0); */
    tmr_cnt_dir_set(TMR1, TMR_COUNT_UP);
    tmr_clock_source_div_set(TMR1, TMR_CLOCK_DIV1);
    tmr_output_default_para_init(&tmr_output_struct);
    tmr_output_struct.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_B;
    tmr_output_struct.oc_output_state = TRUE;
    tmr_output_struct.oc_polarity = TMR_OUTPUT_ACTIVE_LOW;
    tmr_output_struct.oc_idle_state = FALSE;
    tmr_output_channel_config(TMR1, TMR_SELECT_CHANNEL_4, &tmr_output_struct);
    tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_4, 12);
    /* tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_4,  8); */

    /* tmr_interrupt_enable(TMR1, TMR_OVF_INT, TRUE); */

    /* tmr_dma_request_enable(TMR1, TMR_C4_DMA_REQUEST, TRUE); */
    tmr_dma_request_enable(TMR1, TMR_OVERFLOW_DMA_REQUEST, TRUE);

    dma_reset(DMA2_CHANNEL1);
    dma_init_struct.buffer_size = 0;
    dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_base_addr = 0;
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD;
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.peripheral_base_addr = (uint32_t)&GPIOB->odt;
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_init_struct.loop_mode_enable = FALSE;
    dma_init(DMA2_CHANNEL1, &dma_init_struct);

    dma_interrupt_enable(DMA2_CHANNEL1, DMA_FDT_INT, TRUE);
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_irq_enable(DMA2_Channel1_IRQn, 0, 0);
    nvic_irq_enable(TMR1_OVF_TMR10_IRQn, 1, 0);

    
    /* dma_flexible_config(DMA2, FLEX_CHANNEL1, DMA_FLEXIBLE_TMR1_CH4); */
    dma_flexible_config(DMA2, FLEX_CHANNEL1, DMA_FLEXIBLE_TMR1_OVERFLOW);
    
    TMR1->brk_bit.oen = 1;

    /* while(1) { */
    /*     dma_write(dma_test_data, 16); */
    /*     delay_us(10); */
    /* } */
#endif

    CS_ACTIVE;
}
#ifdef DMA_WRITE

uint32_t dma_final_x = 0;
uint32_t dma_final_y = 0;
uint16_t *dma_final_bytes = NULL;
void dma_write(uint16_t *data, uint32_t length, uint32_t final_x, uint32_t final_y) {
    dma_ready = 0;
#if X_BACKOFF
    dma_final_x = final_x - (X_BACKOFF - 1);
    dma_final_y = final_y;
    dma_final_bytes = data + length - (1 + X_BACKOFF);
#endif
    /* tmr_flag_clear(TMR1, TMR_OVF_FLAG); */
    tmr_flag_clear(TMR1, TMR_C4_FLAG);
    GPIOC->clr = 1 << 9;
    TMR1->cval = 0;
    // force first blob
    GPIOB->odt = *data; // initial blob
    data++;
    DMA2_CHANNEL1->ctrl_bit.chen = 0;
#if X_BACKOFF
    DMA2_CHANNEL1->dtcnt = length - (1 + X_BACKOFF); // skip first and last
#else
    DMA2_CHANNEL1->dtcnt = length - 1; // skip first and last
#endif
    DMA2_CHANNEL1->maddr = (uint32_t) (data);
    GPIOC->cfghr &= (uint32_t)~(0x000000F0); // set timer output
    GPIOC->cfghr |= (uint32_t) (0x000000B0);
    DMA2_CHANNEL1->ctrl_bit.chen = 1;
    TMR1->ctrl1_bit.tmren = 1;
}

/* void TMR1_OVF_TMR10_IRQHandler(void) */
/* { */
/*     // ssytem is high */
/*     TMR1->ists = ~TMR_OVF_FLAG; */
/*     if (++tmr_cnt == tmr_end) TMR1->ctrl1_bit.tmren = 0; */
/* } */

// there is no way to capture the last pulses (as in: block them)
// So we stop writing 1 blob earlier and finish it manually
// This costs one setup cycle and one byte burst
void DMA2_Channel1_IRQHandler(void)
{
    __disable_irq();
    CS_IDLE;
    WRITE_IDLE;

    GPIOC->cfghr &= (uint32_t)~(0x00000080);
    /* GPIOC->cfghr |= (uint32_t) (0x00000030); */
    TMR1->ctrl1_bit.tmren = 0;
    DMA2->clr = DMA2_FDT1_FLAG;
    CS_ACTIVE;
#if X_BACKOFF
    // this is a medium-long one, reset column, row is correct
    write_cmd(ILI_COL_ADDR_SET);
    write_data_8bit(dma_final_x   >> 8);
    write_data_8bit(dma_final_x  );
    write_data_8bit((dma_final_x + (X_BACKOFF - 1)) >> 8);
    write_data_8bit(dma_final_x + (X_BACKOFF - 1));
    write_cmd(ILI_PA_ADDR_SET);
    write_data_8bit(dma_final_y >> 8);
    write_data_8bit(dma_final_y);
    write_data_8bit((dma_final_y) >> 8);
    write_data_8bit(dma_final_y);
    write_cmd(ILI_MEMORY_WRITE);
    // unroll to avoid unneeded branches
#pragma GCC unroll 32
    for (int i = 0; i < X_BACKOFF; i++) write_data(*dma_final_bytes++);
#endif
    dma_ready = 1;
#if LVGL_VERSION_MAJOR == 9
    lv_display_flush_ready(dp);
#else
    lv_disp_flush_ready(dp);
#endif

    __enable_irq();
}
#endif

int memcpy_dma(const uint8_t *target, const uint8_t *source, const uint32_t length) {
    return 0;
}

void lcd_backlight(uint32_t value) { // 1-100
    if (value < 1) value = 2;
    else if (value > 100) value = 100;
    timer_channel_output_pulse_value_config(TIMER13,TIMER_CH_0,value-1);
}

static inline void write_cmd(uint8_t command) {
    WRITE_8BIT(command);
    COMMAND;
    /* CS_ACTIVE; */
    READ_IDLE;
    WRITE_ACTIVE;
    /* DELAY_NOP_6; */
    WRITE_IDLE;
    /* DELAY_NOP_6; */
    /* CS_IDLE; */
    /* CLEAR_DATA(); */
}

static inline void write_data(uint16_t data) {
    WRITE_16BIT(data);
    DATA;
    //CS_ACTIVE;
    READ_IDLE;
    WRITE_ACTIVE;
    /* DELAY_NOP_6; */
    WRITE_IDLE;
    /* DELAY_NOP_6; */
    //CS_IDLE;
    /* CLEAR_DATA(); */
}

static void write_data_8bit(uint8_t data) {
    DATA;
    //CS_ACTIVE;
    READ_IDLE;
    WRITE_8BIT(data);
    WRITE_ACTIVE;
    /* DELAY_NOP_6; */
    WRITE_IDLE;
    /* DELAY_NOP_6; */
    //CS_IDLE;
    /* CLEAR_DATA(); */
}

void read_data_16bit(uint16_t *data) {
    DATA;
    //CS_ACTIVE;
    WRITE_IDLE;
    READ_ACTIVE;
    DELAY_NOP_400;
    *data = READ();
    DELAY_NOP_50;
    READ_IDLE;
    DELAY_NOP_50;
    //CS_IDLE;
    DELAY_NOP_50;
}

void read_data_8bit(uint8_t *data) {
    DATA;
    //CS_ACTIVE;
    WRITE_IDLE;
    READ_ACTIVE;
    DELAY_NOP_400;
    READ_IDLE;
    *data = READ() & 0x00FF;
    DELAY_NOP_50;
    //CS_IDLE;
    DELAY_NOP_50;
}

void read_register(uint8_t command, void *location, uint32_t bits, uint32_t num) {
    SET_WRITE();
    write_cmd(command);
    SET_READ();
    /* //CS_ACTIVE; */
    if (bits) {
        uint16_t *location_16 = (uint16_t *) location;
        while (num--) {
            read_data_16bit(location_16++);
        }
    } else {
        uint8_t *location_8 = (uint8_t *) location;
        while (num--) {
            read_data_8bit(location_8++);
        }
    }
    /* //CS_IDLE; */
    SET_WRITE();
}


static void lcd_set_address_window(uint32_t x, uint32_t y, uint32_t x2, uint32_t y2) {
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

void lcd_command(uint8_t cmd) {
    write_cmd(cmd);
}

void lcd_draw_data(uint16_t *data, ssize_t length) {
#ifdef USE_TMR_INT
    // check if available
    while (TMR1->ctrl1_bit.tmren == 1 && TMR1->brk_bit.oen == 1);
#endif
    write_cmd(ILI_MEMORY_WRITE);
#ifdef USE_TMR_INT
    write_data_tmr_int(data, length, 1);
#else
    while (length--) write_data(*data++);
#endif
}

int detect_lcd(void) {
    // ili9488 has D3 register
    uint8_t data[4] = {0};
    read_register(0xD3, &data, 0, 4);
#ifdef MEMORY_DEBUG
    loc[1] = data[1];
    loc[2] = data[2];
    loc[3] = data[3];
#endif
    if (data[1] == 0x00 && data[2] == 0x94 && data[3] == 0x88) {
        // correct ID4 for 9488
#ifdef MEMORY_DEBUG
        loc[0] = 0xff;
#endif
        return 1;
    }
    return 0;
}

/*
 * ugui accelerator functions
 */
#if 0
void lcd_draw(UG_S16 x, UG_S16 y, UG_COLOR color) {
    lcd_set_address_window(x, y, x+1, y);
    write_cmd(ILI_MEMORY_WRITE);
    write_data(color);
}

UG_RESULT lcd_draw_line(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR color) {
    return lcd_fill(x1, y1, x2, y2, color);
}


UG_RESULT lcd_fill(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR color) {
   lcd_set_address_window(x1, y1, x2, y2);
   write_cmd(ILI_MEMORY_WRITE);
   lcd_fill_pixels(((x2-x1) + 1) * ((y2-y1) + 1), color);
   return UG_RESULT_OK;
}


void (*lcd_fill_area(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2))(uint32_t, UG_COLOR) {
    lcd_set_address_window(x1, y1, x2, y2);
    write_cmd(ILI_MEMORY_WRITE);
    return lcd_fill_pixels;
}

UG_RESULT lcd_draw_bmp(UG_S16 x, UG_S16 y, UG_BMP *bmp) {
    uint16_t x2 = x + bmp->width - 1;
    uint16_t y2 = y + bmp->height - 1;

    if (x2 > DISPLAY_WIDTH) x = DISPLAY_WIDTH - 1;
    if (y2 > DISPLAY_HEIGHT) y = DISPLAY_HEIGHT - 1;
    lcd_set_address_window(x, y, x2, y2);
    lcd_draw_data((uint16_t *)bmp->p, bmp->width * bmp->height);
    return UG_RESULT_OK;
}
#endif

void lcd_fill_pixels(uint32_t length, uint16_t color) {
    while(length--) write_data(color);
}

void lcd_fill(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint16_t color) {
   lcd_set_address_window(x1, y1, x2, y2);
   write_cmd(ILI_MEMORY_WRITE);
   lcd_fill_pixels(((x2-x1) + 1) * ((y2-y1) + 1), color);
}

// does nothing, we do not have a frame buffer
void lcd_update(void) {
}


int lcd_start(void) {

    delay_ms(50);
    write_cmd(0x00);
    write_data(0x0001);
    write_cmd(ILI_SLEEP_OUT);
    delay_ms(170);
    if (!detect_lcd()) {
        //return 0;
    }

    for (int i = 0; i < STARTUP_COMMAND_LENGTH; i++) {
        write_cmd(ili_startup_cmds[i].cmd);
        for (int x = 0; x < ili_startup_cmds[i].data_length; x++) {
            write_data_8bit(ili_startup_cmds[i].data[x]);
        }
    }
    write_cmd(ILI_SLEEP_OUT);
    delay_ms(120);
    write_cmd(ILI_DISPLAY_ON);
    delay_ms(200); // 186?
   
    lcd_set_address_window(0, 0, 320, 480);
    write_cmd(ILI_MEMORY_WRITE);
    lcd_fill(0, 0, 320, 480, RGB(255, 0, 0));
    return 1;
}

void lcd_test(void) {
    write_cmd(0x55);
    write_data_8bit(0xAA);
    for (int i = 0; i <= 255; i++) {
        WRITE_16BIT(i);
        delay_10ns(1);
    }
    for (int i = 0; i <= 65536; i++) {
        WRITE_8BIT(0b00011100);
        DELAY_NOP_6;
        WRITE_8BIT(0);
        DELAY_NOP_6;
    }
    WRITE_16BIT(0xFFFF);
}

lv_area_t nar;
uint32_t nl;
#if LVGL_VERSION_MAJOR == 9
void lcd_lvgl_flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixmap) {
#else
void lcd_lvgl_flush(lv_disp_drv_t *display, const lv_area_t *area, lv_color_t *pixmap) { 
#endif
    uint16_t *pixels = (uint16_t *) pixmap;
    uint32_t length = 0;
    length = (area->y2 - area->y1) + 1;
    length *= ((area->x2 - area->x1) + 1);
    memcpy(&nar, area, sizeof(lv_area_t));
    nl = length;
    
    lcd_set_address_window(area->x1, area->y1, area->x2, area->y2);
    write_cmd(ILI_MEMORY_WRITE);
#ifdef DMA_WRITE 
    dp = display;
    DATA;
    dma_write(pixels, length, area->x2, area->y2);
#else
#pragma GCC unroll 8
    while(length--) write_data(*pixels++);

#if LVGL_VERSION_MAJOR == 9
    lv_display_flush_ready(display);
#else
    lv_disp_flush_ready(display);
#endif
#endif
}

#if LVGL_VERSION_MAJOR == 9
void lcd_draw_large_text(uint32_t x, uint32_t y, const lv_image_dsc_t *img, uint16_t color) {
    if (img->header.cf == LV_COLOR_FORMAT_I1) {
#else
void lcd_draw_large_text(uint32_t x, uint32_t y, const lv_img_dsc_t *img, uint16_t color) {
    if (img->header.cf == LV_IMG_CF_INDEXED_1BIT) {
#endif
        // images stored as 1bpp, need to iterate over and draw raw
#ifdef DMA_WRITE
/*
 * Calculation speed exceeds dma draw time at 8 lines or more
 * 1.865ms non-dma -Os
 * 0.72ms non-dma -O3/2/1/fast
 * 1.1ms per char with 37 lines
 * 1.07ms per char with 8 lines
 * 1.12ms per char with 4 lines
 */
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
                cb[i++] = (vdata & 0x80) ? 0x0000 : color;
                cb[i++] = (vdata & 0x40) ? 0x0000 : color;
                cb[i++] = (vdata & 0x20) ? 0x0000 : color;
                cb[i++] = (vdata & 0x10) ? 0x0000 : color;
                cb[i++] = (vdata & 0x08) ? 0x0000 : color;
                cb[i++] = (vdata & 0x04) ? 0x0000 : color;
                cb[i++] = (vdata & 0x02) ? 0x0000 : color;
                cb[i++] = (vdata & 0x01) ? 0x0000 : color;
                data++;
            }
            // wait
            while(!dma_ready);
            // set address
            lcd_set_address_window(x, y, x + img->header.w - 1, y + (BLOB_LINES - 1) < (y + img->header.h - 1) ? y + (BLOB_LINES - 1) : (y + img->header.h - 1));
            // send
            write_cmd(ILI_MEMORY_WRITE);
            DATA;
            dma_write(cb, BLOB_SIZE, x + img->header.w - 1, y + (BLOB_LINES - 1) < (y + img->header.h - 1) ? y + (BLOB_LINES - 1) : (y + img->header.h - 1));
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
            write_data((vdata & 0x80) ? 0x0000 : color);
            write_data((vdata & 0x40) ? 0x0000 : color);
            write_data((vdata & 0x20) ? 0x0000 : color);
            write_data((vdata & 0x10) ? 0x0000 : color);
            write_data((vdata & 0x08) ? 0x0000 : color);
            write_data((vdata & 0x04) ? 0x0000 : color);
            write_data((vdata & 0x02) ? 0x0000 : color);
            write_data((vdata & 0x01) ? 0x0000 : color);
            data++;
        }
#endif
    }
}
