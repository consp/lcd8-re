#include "lcd.h"

#define MEMORY_DEBUG
uint16_t dummy = 0;

#ifdef MEMORY_DEBUG
uint8_t *loc =  (uint8_t *) 0x20004000;
#endif

typedef struct ili_cmd_t {
    uint8_t cmd;
    uint8_t data[16];
    uint32_t data_length;
} ili_cmd;

const ili_cmd ili_startup_cmds[] = {
		/* {ILI_POSITIVE_GAMMA_CORRECTION,     {0x00, 0x03, 0x09, 0x08, 0x16, 0x0A, 0x3F, 0x78, 0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F}, 15}, */
		/* {ILI_NEGATIVE_GAMMA_CORRECTION,     {0x00, 0x16, 0x19, 0x03, 0x0F, 0x05, 0x32, 0x45, 0x46, 0x04, 0x0E, 0x0D, 0x35, 0x37, 0x0F}, 15}, */
        {ILI_POWER_CONTROL_1,               {0x10, 0x10}, 2},
        /* {ILI_POWER_CONTROL_2,               {0x44}, 1}, */
		/* {ILI_VCOM_CONTROL_1,                {0x00, 0x22, 0x80, 0x44}, 4}, */
		/* {ILI_MEMORY_ACCESS_CONTROL,         {(0x20 | 0x08)}, 1}, */
		/* {ILI_INTERFACE_PIXEL_FORMAT,        {0x55}, 1}, */
		{ILI_INTERFACE_MODE_CONTROL,        {0x00}, 1},
		/* {ILI_FRAME_RATE_CONTROL_NORMAL,     {0xB0, 0x11}, 2}, */
		/* {ILI_DISPLAY_INVERSION_CONTROL,     {0x02}, 1}, */
		/* {ILI_DISPLAY_FUNCTION_CONTROL,      {0x02, 0x02, 0x3B}, 3}, */
		/* {ILI_SET_IMAGE_FUNCTION,            {0x00}, 1}, */
		/* {ILI_WRITE_CTRL_DISPLAY,            {0x28}, 1}, */
		/* {ILI_WRITE_DISPLAY_BRIGHTNESS,      {0x7F}, 1}, */
		/* {ILI_ADJUST_CONTROL_3,              {0xA9, 0x51, 0x2C, 0x82}, 4} */
};

void lcd_init(void) {
    
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE); // we use all channels
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE); // we use all channels
  
  gpio_init_type gpio_initstructure;

  /* for (int i = 0; i < 16; i++) { */
  /*   gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_OPEN_DRAIN; */
  /*   gpio_initstructure.gpio_pull           = GPIO_PULL_NONE; // external pullup */
  /*   gpio_initstructure.gpio_mode           = GPIO_MODE_INPUT; */
  /*   gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM; */
  /*   gpio_initstructure.gpio_pins           = 1 << i; */
  /*   gpio_init(GPIOB, &gpio_initstructure); */
  /* } */

  // Backlight pin, set full on for now, switch to pwm
    gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_pull           = GPIO_PULL_NONE; // external pullup
    gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MAXIMUM;
    gpio_initstructure.gpio_pins           = GPIO_PINS_7;
    gpio_init(GPIOA, &gpio_initstructure);


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
    SET_READ();

    loc[16] = GPIOB->cfglr & 0x000000FF;
    loc[17] = GPIOB->idt & 0x000000FF;
    loc[18] = GPIOB->odt & 0x000000FF;

    GPIOC->odt |= PIN_READ | PIN_WRITE | PIN_CD | PIN_CS; // force all high
    SET_WRITE();
    CLEAR_DATA();
    lcd_backlight(1);

}

void lcd_backlight(uint32_t enable) {
    if (enable) gpio_bits_set(GPIOA, GPIO_PINS_7);
    else gpio_bits_reset(GPIOA, GPIO_PINS_7);
}

static void write_cmd(uint8_t command) {
    SET_WRITE();
    COMMAND();
    DELAY_NOP_6;
    CS_LOW();
    DELAY_NOP_50;
    WRITE_8BIT(command);
    DELAY_NOP_50;
    CS_HIGH();
    DELAY_NOP_50;
    DELAY_NOP_50;
}

static void write_data(uint16_t data) {
    SET_WRITE();
    DATA();
    CS_LOW();
    DELAY_NOP_50;
    DELAY_NOP_50;
    WRITE_16BIT(data);
    DELAY_NOP_50;
    CS_HIGH();
    DELAY_NOP_50;
    DELAY_NOP_50;
}

static void write_data_8bit(uint8_t data) {
    DATA();
    CS_LOW();
    DELAY_NOP_50;
    WRITE_8BIT(data);
    DELAY_NOP_50;
    CS_HIGH();
    DELAY_NOP_50;
    DELAY_NOP_50;
}

static void read_data(uint16_t *data) {
    DATA();
    CS_LOW();
    SET_READ();
    DELAY_NOP_200;
    DELAY_NOP_200;
    READ_STROBE();
    *data = GPIOB->idt & 0x0000FFFF;
    DELAY_NOP_400;
    SET_WRITE();
    CS_HIGH();
    DELAY_NOP_20;
}

static void read_data_8bit(uint8_t *data) {
    DATA();
    CS_LOW();
    SET_READ();
    DELAY_NOP_200;
    READ_STROBE();
    DELAY_NOP_200;
    *data = GPIOB->idt & 0x000000FF;
    CS_HIGH();
    DELAY_NOP_400;
    SET_WRITE();
    DELAY_NOP_20;
}

void set_address_window(uint32_t x, uint32_t y, uint32_t x2, uint32_t y2) {
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

void lcd_fill(uint16_t color, uint32_t len) {
    write_cmd(ILI_MEMORY_WRITE);

    while (len-- > 0) {
        write_data(color);
    }
}

void lcd_fill_screen(uint16_t color) {
    set_address_window(0, 0, 100, 100);
    lcd_fill(color, 100*100);
}

int detect_lcd(void) {
    // ili9488 has D3 register
#ifdef MEMORY_DEBUG
    loc[0] = 0;
    loc[1]++;
#endif
    uint8_t data[4] = {0};
    write_cmd(0xD3);
    for (int i = 0; i < 4; i++) read_data_8bit(&data[i]);

    if (data[1] == 0x00 && data[2] == 0x94 && data[3] == 0x88) {
        // correct ID4 for 9488
#ifdef MEMORY_DEBUG
        loc[0] = 0xff;
#endif
        return 1;
    }
    return 0;
}

int lcd_start(void) {

    if (!detect_lcd()) {
        return 0;
    }

    write_cmd(ILI_SOFT_RESET);
    delay_ms(200);
    write_cmd(ILI_DISPLAY_ON);
    delay_ms(200);
    /* write_cmd(ILI_INTERFACEV_PIXEL_FORMAT); */
    /* write_data(0x66); */
    /* write_cmd(ILI_SLEEP_OUT); */
    /* delay_ms(200); */
    /* write_cmd(ILI_SLEEP_IN); */
    /* write_cmd(ILI_SLEEP_OUT); */
    /* delay_ms(300); */
    /* write_cmd(0x0F); */
    /* read_data_8bit(&loc[10]); */
    /* read_data_8bit(&loc[11]); */

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

    /* write_cmd(0x04); */
    /* read_data_8bit(&loc[4]); */
    /* read_data_8bit(&loc[5]); */
    /* read_data_8bit(&loc[6]); */
    /* read_data_8bit(&loc[7]); */
    /* write_cmd(0x09); */
    /* read_data_8bit(&loc[8]); */
    /* read_data_8bit(&loc[9]); */
    /* read_data_8bit(&loc[10]); */
    /* read_data_8bit(&loc[11]); */
    /* read_data_8bit(&loc[12]); */
    /* read_data_8bit(&loc[14]); */
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
    /*     delay_ms(100); */
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
