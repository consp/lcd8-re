#include "lcd.h"
#include "at32f415_crm.h"

/* #define MEMORY_DEBUG */
uint16_t dummy = 0;

#ifdef MEMORY_DEBUG
uint8_t *loc =  (uint8_t *) 0x20004060;
uint16_t *loc16 =  (uint16_t *) 0x20004080;
extern uint32_t *debugbuffer32;
#endif

uint32_t delay_slot = 1;

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

void lcd_backlight_init(void) {
    crm_periph_clock_enable(CRM_TMR3_PERIPH_CLOCK, TRUE);

    tmr_base_init(TMR3, 10000-1, TIMER_FREQ(1000000)); // 10khz?
    tmr_cnt_dir_set(TMR3, TMR_COUNT_UP);
    tmr_clock_source_div_set(TMR3, TMR_CLOCK_DIV1);

    tmr_output_config_type tmr_oc_init_structure;
    tmr_output_default_para_init(&tmr_oc_init_structure);
    tmr_oc_init_structure.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_A;
    tmr_oc_init_structure.oc_idle_state = FALSE;
    tmr_oc_init_structure.oc_polarity = TMR_OUTPUT_ACTIVE_HIGH;
    tmr_oc_init_structure.oc_output_state = TRUE;
    tmr_output_channel_config(TMR3, TMR_SELECT_CHANNEL_2, &tmr_oc_init_structure);
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_2, 0);
    tmr_output_channel_buffer_enable(TMR3, TMR_SELECT_CHANNEL_4, TRUE);
    tmr_period_buffer_enable(TMR3, TRUE);

    /* tmr enable counter */
    tmr_counter_enable(TMR3, TRUE);
}

#ifdef USE_TMR_INT
tmr_output_config_type tmr_output_struct;
void *transfer_output = NULL;
uint32_t transfer_length = 0;
uint32_t transfer_halfword = 1;
#endif

void lcd_tmr_init(void) {
#ifdef USE_TMR_INT
    crm_periph_clock_enable(CRM_TMR1_PERIPH_CLOCK, TRUE);
    // need 500ns pulse, 72 cycles at 144hz
    uint32_t timer_period = 71;
    tmr_base_init(TMR1, timer_period, 0);
    tmr_cnt_dir_set(TMR1, TMR_COUNT_UP);
    tmr_output_default_para_init(&tmr_output_struct);
    tmr_output_struct.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_A;
    tmr_output_struct.oc_output_state = TRUE;
    tmr_output_struct.oc_polarity = TMR_OUTPUT_ACTIVE_HIGH;
    tmr_output_struct.oc_idle_state = TRUE;
    tmr_output_struct.occ_output_state = FALSE;
    tmr_output_struct.occ_polarity = TMR_OUTPUT_ACTIVE_HIGH;
    tmr_output_struct.occ_idle_state = FALSE;
    tmr_output_channel_config(TMR1, TMR_SELECT_CHANNEL_4, &tmr_output_struct);
    // pulse after 100ns
    tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_4, 7);
    tmr_interrupt_enable(TMR2, TMR_OVF_INT, TRUE);

    tmr_output_enable(TMR1, FALSE);
    tmr_counter_enable(TMR1, FALSE);
#endif
}

void lcd_init(void) {
    
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE); // we use all channels
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE); // we use all channels
    crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE); // we use all channels
  gpio_init_type gpio_initstructure;
  // remap
  crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);
  gpio_pin_remap_config(SWJTAG_GMUX_010, TRUE);

  for (int i = 0; i < 16; i++) {
    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_pull           = GPIO_PULL_NONE; // external pullup
    gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM;
    gpio_initstructure.gpio_pins           = 1 << i;
    gpio_init(GPIOB, &gpio_initstructure);
  }
    GPIOB->odt = 0xFFFF;

  // Backlight pin, pwm 
    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_pull           = GPIO_PULL_NONE; // external pullup
    gpio_initstructure.gpio_mode           = GPIO_MODE_MUX;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM;
    gpio_initstructure.gpio_pins           = GPIO_PINS_7;
    gpio_init(GPIOA, &gpio_initstructure);

    crm_periph_clock_enable(CRM_TMR3_PERIPH_CLOCK, TRUE);
    lcd_backlight_init();

    // init clocks

    // Read strobe
    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_pull           = GPIO_PULL_UP; // external pullup
    gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM;
    gpio_initstructure.gpio_pins           = PIN_READ;
    gpio_init(GPIOC, &gpio_initstructure);

    // Write strobe
    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_pull           = GPIO_PULL_UP; // external pullup
#ifdef USE_TMR_INT
    gpio_initstructure.gpio_mode           = GPIO_MODE_MUX;
#else
    gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
#endif
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM;
    gpio_initstructure.gpio_pins           = PIN_WRITE;
    gpio_init(GPIOC, &gpio_initstructure);

    // Command/Data
    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_pull           = GPIO_PULL_UP; // external pullup
    gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM;
    gpio_initstructure.gpio_pins           = PIN_CD;
    gpio_init(GPIOC, &gpio_initstructure);

    // cs
    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_pull           = GPIO_PULL_UP; // external pullup
    gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM;
    gpio_initstructure.gpio_pins           = PIN_CS;
    gpio_init(GPIOC, &gpio_initstructure);

    //CLEAR_DATA();
#ifdef MEMORY_DEBUG
    loc[4] = GPIOB->cfglr & 0x000000FF;
    loc[5] = GPIOB->idt & 0x000000FF;
    loc[6] = GPIOB->odt & 0x000000FF;
#endif

    GPIOC->odt |= PIN_READ | PIN_WRITE | PIN_CD | PIN_CS; // force all high
    SET_WRITE();
    CS_ACTIVE;
}

void lcd_backlight(uint32_t value) { // 0-100
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_2, value * 100);
}

static inline void write_cmd(uint8_t command) {
    WRITE_8BIT(command);
    COMMAND;
    //CS_ACTIVE;
    READ_IDLE;
    WRITE_ACTIVE;
    /* DELAY_NOP_6; */
    WRITE_IDLE;
    /* DELAY_NOP_6; */
    //CS_IDLE;
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
    *data = GPIOB->idt & 0xFFFF;
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
    *data = GPIOB->idt & 0x00FF;
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


void lcd_set_address_window(uint32_t x, uint32_t y, uint32_t x2, uint32_t y2) {
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

void write_data_tmr_int(void *data, uint32_t length, uint32_t halfword) {
#ifdef USE_TMR_INT
    // set pin to mux
    WRITE_STROBE_TMR();
    DATA;
    //CS_ACTIVE;
    READ_IDLE;
    WRITE_IDLE;
    // initialize transfer
    transfer_output = (void *) data;
    transfer_length = length;
    transfer_halfword = halfword;
    // setup initial data
    if (transfer_halfword) {
        GPIOB->odt = *((uint16_t *) transfer_output++);
    } else {
        GPIOB->odt = *((uint8_t *) transfer_output++);
    }
    // transfer
    TMR1->brk_bit.oen = 1;
    TMR1->ctrl1_bit.tmren = 1; 
#endif
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
#ifdef USE_TMR_INT 
#else
    while(length--) write_data(color);
#endif
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

#ifdef USE_TMR_INT
void TMR1_GLOBAL_IRQHandler(void)
{
    if((TMR1->ists & TMR_OVF_FLAG) && (TMR1->iden & TMR_OVF_FLAG)) {
        TMR1->ists = ~TMR_OVF_FLAG;
        if (transfer_halfword) {
            GPIOB->odt = *((uint16_t *) transfer_output++);
        } else {
            GPIOB->odt = *((uint8_t *) transfer_output++);
        }
        transfer_length--;
        if (!transfer_length) {
            //CS_IDLE;
            WRITE_IDLE;
            TMR1->brk_bit.oen = 0;
            TMR1->ctrl1_bit.tmren = 0;
            WRITE_STROBE_GPIO();
        }
    }
}
#endif

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

#if LVGL_VERSION_MAJOR == 9
void lcd_lvgl_flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixmap) {
#else
void lcd_lvgl_flush(lv_disp_drv_t *display, const lv_area_t *area, lv_color_t *pixmap) { 
#endif
    uint16_t *pixels = (uint16_t *) pixmap;
    uint32_t length = 0;
    if (area->y2 == area->y1) length = 1;
    else length = (area->y2 - area->y1) + 1;
    if (area->x1 != area->x2) length *= ((area->x2 - area->x1) + 1);
    lcd_set_address_window(area->x1, area->y1, area->x2, area->y2);
    write_cmd(ILI_MEMORY_WRITE);
    /* if (length % 32) { */
    /*     while(length--) write_data(*pixels++); */
    /* } else { */
#pragma GCC unroll 8
        while(length--) write_data(*pixels++);
    /* } */
#if LVGL_VERSION_MAJOR == 9
    lv_display_flush_ready(display);
#else
    lv_disp_flush_ready(display);
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
        lcd_set_address_window(x, y, x + img->header.w - 1, y + img->header.h - 1);
        uint32_t length = (img->header.w * img->header.h) >> 3;
        uint8_t *data = (uint8_t *) img->data;
        data += 8;
        write_cmd(ILI_MEMORY_WRITE);
#pragma GCC unroll 8
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
    }
}
