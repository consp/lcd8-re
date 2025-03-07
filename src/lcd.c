#include "lcd.h"
#include "at32f415_crm.h"

#define MEMORY_DEBUG
uint16_t dummy = 0;

#ifdef MEMORY_DEBUG
uint8_t *loc =  (uint8_t *) 0x20004060;
uint16_t *loc16 =  (uint16_t *) 0x20004080;
#endif

uint32_t delay_slot = 1;

typedef struct ili_cmd_t {
    uint8_t cmd;
    uint8_t data[16];
    uint32_t data_length;
} ili_cmd;

const ili_cmd ili_startup_cmds[] = {
		{ILI_POSITIVE_GAMMA_CORRECTION,     {0x00, 0x03, 0x09, 0x08, 0x16, 0x0A, 0x3F, 0x78, 0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F}, 15},
		{ILI_NEGATIVE_GAMMA_CORRECTION,     {0x00, 0x16, 0x19, 0x03, 0x0F, 0x05, 0x32, 0x45, 0x46, 0x04, 0x0E, 0x0D, 0x35, 0x37, 0x0F}, 15},
        {ILI_POWER_CONTROL_1,               {0x10, 0x10}, 2},
        {ILI_POWER_CONTROL_2,               {0x44}, 1},
		{ILI_VCOM_CONTROL_1,                {0x00, 0x22, 0x80, 0x44}, 4},
		{ILI_MEMORY_ACCESS_CONTROL,         {(0x20 | 0x08)}, 1},
		{ILI_INTERFACE_PIXEL_FORMAT,        {0x55}, 1},
		{ILI_INTERFACE_MODE_CONTROL,        {0x00}, 1},
		{ILI_FRAME_RATE_CONTROL_NORMAL,     {0xB0, 0x11}, 2},
		{ILI_DISPLAY_INVERSION_CONTROL,     {0x02}, 1},
		{ILI_DISPLAY_FUNCTION_CONTROL,      {0x02, 0x02, 0x3B}, 3},
		{ILI_SET_IMAGE_FUNCTION,            {0x00}, 1},
		{ILI_WRITE_CTRL_DISPLAY,            {0x28}, 1},
		{ILI_WRITE_DISPLAY_BRIGHTNESS,      {0x7F}, 1},
		{ILI_ADJUST_CONTROL_3,              {0xA9, 0x51, 0x2C, 0x82}, 4}
};

void lcd_backlight_init(void) {
  /* tmr3 time base configuration */

    tmr_base_init(TMR3, 1000, 14); // 10khz?
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
    tmr_output_channel_buffer_enable(TMR3, TMR_SELECT_CHANNEL_2, TRUE);
    tmr_period_buffer_enable(TMR3, TRUE);

    /* tmr enable counter */
    tmr_counter_enable(TMR3, TRUE);
}

void lcd_init(void) {
    
  gpio_init_type gpio_initstructure;

  for (int i = 0; i < 16; i++) {
    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_pull           = GPIO_PULL_NONE; // external pullup
    gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM;
    gpio_initstructure.gpio_pins           = 1 << i;
    gpio_init(GPIOB, &gpio_initstructure);
  }

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
    gpio_initstructure.gpio_pins           = GPIO_PINS_8;
    gpio_init(GPIOC, &gpio_initstructure);

    // Write strobe
    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_pull           = GPIO_PULL_UP; // external pullup
    gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM;
    gpio_initstructure.gpio_pins           = GPIO_PINS_9;
    gpio_init(GPIOC, &gpio_initstructure);

    // Command/Data
    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_pull           = GPIO_PULL_UP; // external pullup
    gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM;
    gpio_initstructure.gpio_pins           = GPIO_PINS_10;
    gpio_init(GPIOC, &gpio_initstructure);

    // clock? cs?
    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_pull           = GPIO_PULL_UP; // external pullup
    gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM;
    gpio_initstructure.gpio_pins           = GPIO_PINS_11;
    gpio_init(GPIOC, &gpio_initstructure);

    CLEAR_DATA();

    loc[4] = GPIOB->cfglr & 0x000000FF;
    loc[5] = GPIOB->idt & 0x000000FF;
    loc[6] = GPIOB->odt & 0x000000FF;

    GPIOC->odt |= PIN_READ | PIN_WRITE | PIN_CD | PIN_CS; // force all high
    SET_WRITE();

}

void lcd_backlight(uint32_t value) { // 0-100
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_2, value * 10);
}

static void write_cmd(uint8_t command) {
    COMMAND;
    CS_ACTIVE;
    READ_IDLE;
    WRITE_IDLE;
    WRITE_8BIT(command);
    WRITE_ACTIVE;
    DELAY_NOP_200;
    WRITE_IDLE;
    CS_IDLE;
    CLEAR_DATA();
}

static void write_data(uint16_t data) {
    DATA;
    CS_ACTIVE;
    READ_IDLE;
    WRITE_IDLE;
    WRITE_16BIT(data);
    DELAY_NOP_6;
    WRITE_ACTIVE;
    DELAY_NOP_50;
    WRITE_IDLE;
    CS_IDLE;
    CLEAR_DATA();
}

static void write_data_8bit(uint8_t data) {
    DATA;
    CS_ACTIVE;
    READ_IDLE;
    WRITE_IDLE;
    WRITE_8BIT(data);
    DELAY_NOP_6;
    WRITE_ACTIVE;
    DELAY_NOP_50;
    WRITE_IDLE;
    CS_IDLE;
    CLEAR_DATA();
}

static void read_data_16bit(uint16_t *data) {
    DATA;
    CS_ACTIVE;
    WRITE_IDLE;
    READ_ACTIVE;
    DELAY_NOP_400;
    *data = GPIOB->idt & 0xFFFF;
    READ_IDLE;
    CS_IDLE;
}

static void read_data_8bit(uint8_t *data) {
    DATA;
    CS_ACTIVE;
    WRITE_IDLE;
    READ_ACTIVE;
    DELAY_NOP_400;
    *data = GPIOB->idt & 0x00FF;
    READ_IDLE;
    CS_IDLE;
}

static void read_register(uint8_t command, void *location, uint32_t bits, uint32_t num) {
    SET_WRITE();
    write_cmd(command);
    SET_READ();
    /* CS_ACTIVE; */
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
    /* CS_IDLE; */
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

void lcd_draw_data(uint16_t *data, ssize_t length) {
    // 32bit is faster
    
    write_cmd(ILI_MEMORY_WRITE);
    if (length % 2) { length--; write_data(*data++); };
    uint32_t *data32 = (uint32_t*) data;
    while (length-=2) {
        write_data(*data32 >> 16);
        write_data(*data32++);
    }
}

int detect_lcd(void) {
    // ili9488 has D3 register
    uint8_t data[4] = {0};
    read_register(0xD3, &data, 0, 4);
    loc[1] = data[1];
    loc[2] = data[2];
    loc[3] = data[3];
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
void lcd_draw(UG_S16 x, UG_S16 y, UG_COLOR color) {
    lcd_set_address_window(x, y, x+1, y);
    write_cmd(ILI_MEMORY_WRITE);
    write_data(color);
}

UG_RESULT lcd_draw_line(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR color) {
    return lcd_fill(x1, y1, x2, y2, color);
}

void lcd_fill_pixels(uint32_t length, UG_COLOR color) {
   while(length--) write_data(color);
}

UG_RESULT lcd_fill(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR color) {
   lcd_set_address_window(x1, y1, x2, y2);
   write_cmd(ILI_MEMORY_WRITE);
   lcd_fill_pixels(x2-x1 * y2-y1, color);
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
    if (y2 > DISPLAY_HEIGHT) x = DISPLAY_HEIGHT - 1;
    lcd_set_address_window(x, y, x2, y2);
    lcd_draw_data((uint16_t *)bmp->p, bmp->width * bmp->height);
    return UG_RESULT_OK;
}

// does nothing, we do not have a frame buffer
void lcd_update(void) {
}


int lcd_start(void) {

    if (!detect_lcd()) {
        //return 0;
    }

    write_cmd(ILI_SLEEP_OUT);
    delay_ms(200);

    // clear
    read_register(0x00, &loc[16], 0, 2);
    
    // read display info
    read_register(0x04, &loc[18], 0, 4);


    /* write_cmd(ILI_SOFT_RESET); */
    /* delay_ms(400); */
    /* write_cmd(ILI_SLEEP_OUT); */
    /* delay_ms(400); */
    /* write_cmd(ILI_DISPLAY_ON); */
    /* delay_ms(200); */
    /* write_cmd(ILI_INTERFACEV_PIXEL_FORMAT); */
    /* write_data(0x66); */
    /* write_cmd(ILI_SLEEP_OUT); */
    /* delay_ms(200); */
    /* write_cmd(ILI_SLEEP_IN); */
    /* write_cmd(ILI_SLEEP_OUT); */
    /* delay_ms(300); */
    /* write_cmd(0x0F); */
    /* read_data_8bit(&loc[4]); */
    /* read_data_8bit(&loc[5]); */

    /* write_cmd(0x04); */
    /* read_data_8bit(&loc[4]); */
    /* read_data_8bit(&loc[5]); */
    /* read_data_8bit(&loc[6]); */
    /* read_data_8bit(&loc[7]); */
    /* write_cmd(ILI_DISPLAY_ON); */
    /* delay_ms(300); */

    /* delay_ms(400); */
    /* write_cmd(0x00); */
    /* write_cmd(0x00); */
    /* write_cmd(0x00); */
    /* write_cmd(0x3A); */
    /* write_data(0x55); */
    /* write_cmd(ILI_DISPLAY_ON); */
    /* delay_ms(100); */
    /* write_cmd(0x09); */
    /* read_data_8bit(&loc[12]); */
    /* read_data_8bit(&loc[12]); */
    /* read_data_8bit(&loc[13]); */
    /* read_data_8bit(&loc[14]); */
    /* read_data_8bit(&loc[15]); */
    /* delay_us(100); */
    /*  */
    /* write_cmd(0x0A); */
    /* read_data_8bit(&loc[15]); */
    /* read_data_8bit(&loc[16]); */
    /* write_cmd(0x0B); */
    /* read_data_8bit(&loc[17]); */
    /* read_data_8bit(&loc[18]); */
    /* write_cmd(0x0C); */
    /* read_data_8bit(&loc[19]); */
    /* read_data_8bit(&loc[19]); */
    /* write_cmd(0x0D); */
    /* read_data_8bit(&loc[20]); */
    /* read_data_8bit(&loc[20]); */

    /* read_data_8bit(&loc[4]); */
    /* read_data_8bit(&loc[4]); */
    /*  */
    /* write_cmd(0x0B); */
    /* read_data_8bit(&loc[5]); */
    /* read_data_8bit(&loc[5]); */
    /*  */
    /* write_cmd(0x0C); */
    /* read_data_8bit(&loc[6]); */
    /* read_data_8bit(&loc[6]); */
    /* #<{(| read_data_8bit(&loc[7]); |)}># */
    /* for (int i = 0; i < 100; i++) { */
    /*     read_data_8bit(&loc[21+i]); */
    /* } */
    /* uint32_t i = 0; */
    /* write_cmd(ili_startup_cmds[i].cmd); */
    /* for (int x = 0; x < ili_startup_cmds[i].data_length; x++) { */
    /*     DELAY_NOP_200; */
    /*     write_data_8bit(ili_startup_cmds[i].data[x]); */
    /* } */
    /* i = 1; */
    /* write_cmd(ili_startup_cmds[i].cmd); */
    /* for (int x = 0; x < ili_startup_cmds[i].data_length; x++) { */
    /*     DELAY_NOP_200; */
    /*     write_data_8bit(ili_startup_cmds[i].data[x]); */
    /* } */
    /* i = 2; */
    /* write_cmd(ili_startup_cmds[i].cmd); */
    /* for (int x = 0; x < ili_startup_cmds[i].data_length; x++) { */
    /*     DELAY_NOP_200; */
    /*     write_data_8bit(ili_startup_cmds[i].data[x]); */
    /* } */
    /* i = 3; */
    /* write_cmd(ili_startup_cmds[i].cmd); */
    /* for (int x = 0; x < ili_startup_cmds[i].data_length; x++) { */
    /*     DELAY_NOP_200; */
    /*     write_data_8bit(ili_startup_cmds[i].data[x]); */
    /* } */
    /* i = 4; */
    /* write_cmd(ili_startup_cmds[i].cmd); */
    /* for (int x = 0; x < ili_startup_cmds[i].data_length; x++) { */
    /*     DELAY_NOP_200; */
    /*     write_data_8bit(ili_startup_cmds[i].data[x]); */
    /* } */
    /* i = 5; */
    /* write_cmd(ili_startup_cmds[i].cmd); */
    /* for (int x = 0; x < ili_startup_cmds[i].data_length; x++) { */
    /*     DELAY_NOP_200; */
    /*     write_data_8bit(ili_startup_cmds[i].data[x]); */
    /* } */
    /* for (int i = 0; i < 15; i++) { */
    /*     write_cmd(ili_startup_cmds[i].cmd); */
    /*     for (int x = 0; x < ili_startup_cmds[i].data_length; x++) { */
    /*         write_data_8bit(ili_startup_cmds[i].data[x]); */
    /*     } */
    /* } */
    /* write_cmd(0x09); */
    /* read_data_8bit(&loc[8]); */
    /* read_data_8bit(&loc[8]); */
    /* read_data_8bit(&loc[9]); */
    /* read_data_8bit(&loc[10]); */
    /* read_data_8bit(&loc[11]); */
    /*  */
    /* write_cmd(0x00); */
    /* write_cmd(0x00); */
    /* write_cmd(0x00); */
    /* write_cmd(0x09); */
    /* loc+=16; */
    /* read_data_8bit(&loc[4]); */
    /* read_data_8bit(&loc[4]); */
    /* read_data_8bit(&loc[5]); */
    /* read_data_8bit(&loc[6]); */
    /* read_data_8bit(&loc[7]); */
    /*  */
    /* write_cmd(0x0A); */
    /* read_data_8bit(&loc[8]); */
    /* read_data_8bit(&loc[8]); */
    /* write_cmd(0x0B); */
    /* read_data_8bit(&loc[9]); */
    /* read_data_8bit(&loc[9]); */
    /* write_cmd(0x0C); */
    /* read_data_8bit(&loc[10]); */
    /* read_data_8bit(&loc[10]); */
    /* write_cmd(0x0D); */
    /* read_data_8bit(&loc[11]); */
    /* read_data_8bit(&loc[11]); */
    /*  */
    /*  */

    write_cmd(ILI_DISPLAY_ON);
    delay_ms(100);

    read_register(0x04, &loc[22], 0, 4);

    // display status
    read_register(0x0A, &loc[32], 0, 2);
    read_register(0x0B, &loc[34], 0, 2);
    read_register(0x0C, &loc[36], 0, 2);
    read_register(0x0D, &loc[38], 0, 2);
    read_register(0x04, &loc[40], 0, 4);
    /* lcd_fill_screen(RGB(0, 0, 255)); */
    /* lcd_fill_screen(0xAAAA); */
    /* set_address_window(1, 10, 1, 10); */
    /* write_cmd(ILI_MEMORY_WRITE); */
    /* for (int i = 0; i < 10 * 10; i++) write_data(0x55AA); */
    /*  */
    /* delay_ms(50); */
    /* set_address_window(1, 10, 1, 10); */
    /* write_cmd(0x2D); // read */
    /* uint16_t *b = (uint16_t*) loc; */
    /* for (int i = 0; i < 100; i++) { */
    /*     read_data(&b[16+i]); */
    /* } */
    /* write_cmd(ILI_SLEEP_OUT); */
    /* delay_ms(150); */
    /* write_cmd(ILI_DISPLAY_ON); */
    /* delay_ms(500); */
    /*  */
    /* write_cmd(0x09); */
    /* read_data_8bit(&loc[4]); */
    /* read_data_8bit(&loc[4]); */
    /* read_data_8bit(&loc[5]); */
    /* read_data_8bit(&loc[6]); */
    /* read_data_8bit(&loc[7]); */

    /* write_cmd(0x09); */
    /* read_data_8bit(&loc[8]); */
    /* read_data_8bit(&loc[8]); */
    /* read_data_8bit(&loc[9]); */
    /* read_data_8bit(&loc[10]); */
    /* read_data_8bit(&loc[11]); */
    return 1;
}

