#include "gui.h"
#include "img.h"
#include "lvgl.h"
#include "controls.h"

uint8_t pixelbuffer[PIXEL_BUFFER_LINES * DISPLAY_WIDTH * 2];
volatile uint32_t timer = 0;
uint32_t timer_old = 0;

/*** 
 * data variables
 */
int32_t power_value = 0, power_value_old = 0;
int32_t speed = 0, oldspeed = 0;
int32_t battery_value = 0, battery_value_old = 0;
int32_t battery_voltage = 0, battery_voltage_old = 24<<8;
int32_t int_temperature = 0, int_temperature_old = 0;
int32_t ext_temperature = 0, ext_temperature_old = 0;
int32_t mot_temperature = 0, mot_temperature_old = 0;
int32_t avg_speed = 0, avg_power = 0;

/***
 * display variables
 */
#if LVGL_VERSION_MAJOR == 9
lv_display_t *display = NULL;
#else
lv_disp_t *display = NULL;
lv_disp_drv_t disp_drv = {0};
lv_disp_draw_buf_t draw_buf = {0};
#endif

#ifdef DEBUG
lv_obj_t *debug_text = NULL;
#endif

lv_indev_t *buttons = NULL;

lv_obj_t *powerbar_positive = NULL;
lv_obj_t *powerbar_negative = NULL;
lv_obj_t *powerbar_center_line = NULL;
lv_obj_t *powerbar_high_line = NULL;
lv_obj_t *powerbar_shade = NULL;
lv_obj_t *powerbar_text = NULL;

lv_obj_t *temperature = NULL;
lv_obj_t *voltage = NULL;
lv_obj_t *motor_temperature = NULL;
lv_obj_t *battery = NULL;
lv_obj_t *battery_bar = NULL;
lv_obj_t *battery_label = NULL;
lv_obj_t *speed_minor = NULL;

lv_obj_t *trip_distance_text = NULL;
lv_obj_t *trip_time_text = NULL;
lv_obj_t *total_distance_text = NULL;
lv_obj_t *avg_speed_text = NULL;
lv_obj_t *max_speed_text = NULL;

lv_obj_t *graph = NULL; 
lv_chart_series_t *graph_series;
lv_coord_t graph_array[100] = {0};


lv_style_t text_slim, text_normal;

// images
//
#if LVGL_VERSION_MAJOR == 9
extern const lv_image_dsc_t battery_black;
#else
extern const lv_img_dsc_t battery_black;
#endif

// draw and local functions
static void _draw_speed(void);
static void _draw_battery(void);
static void _draw_power(void);
static void _draw_temperature(void);
static void _draw_voltage(void);
void _draw_graph(lv_timer_t *timer);
void gui_draw_init(void);

#ifdef DEBUG
void _sim_update(lv_timer_t *timer);
#endif
static void _avg_timer(lv_timer_t *timer);


// timers
lv_timer_t *draw_graph_timer = NULL;
lv_timer_t *sim_timer = NULL;
lv_timer_t *avg_timer = NULL;

// lvgl local
static void graph_event_cb(lv_event_t * e);

/***
 * LVGL timer functions
 */
uint32_t timer_cb(void) {
    return timer;
}

tmr_output_config_type tmr_output_struct;
void gui_init(void) {
    // setup timer
    crm_periph_clock_enable(CRM_TMR1_PERIPH_CLOCK, TRUE);
    tmr_base_init(TMR1, 143, 1000-1); // 1khz
    tmr_clock_source_div_set(TMR1, TMR_CLOCK_DIV1);
    tmr_cnt_dir_set(TMR1, TMR_COUNT_UP);
    /* tmr_output_channel_config(TMR1, TMR_SELECT_CHANNEL_4, &tmr_output_struct); */
    tmr_interrupt_enable(TMR1, TMR_OVF_INT, TRUE); // trap on overflow

    tmr_output_enable(TMR1, FALSE);
    tmr_counter_enable(TMR1, TRUE);

    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_irq_enable(TMR1_OVF_TMR10_IRQn, 0, 0);

    lv_init();

    // set display
#if LVGL_VERSION_MAJOR == 8
    lv_disp_drv_init(&disp_drv);
    lv_disp_draw_buf_init(&draw_buf, pixelbuffer, NULL, PIXEL_BUFFER_LINES * DISPLAY_WIDTH);
    disp_drv.draw_buf = &draw_buf;
    disp_drv.flush_cb = lcd_lvgl_flush;    /*Set your driver function*/
    disp_drv.set_px_cb = NULL;
    disp_drv.hor_res = DISPLAY_WIDTH;   /*Set the horizontal resolution of the disp_drv*/
    disp_drv.ver_res =DISPLAY_HEIGHT;   /*Set the vertical resolution of the disp_drv*/
    display = lv_disp_drv_register(&disp_drv);
    /* // ticks */
    /* lv_timer_set_cb(&timer_cb); */
#else
    display = lv_display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_display_set_buffers(display, pixelbuffer, NULL, PIXEL_BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, lcd_lvgl_flush);

    // ticks
    lv_tick_set_cb(&timer_cb);
    
    buttons = lv_indev_create();
    lv_indev_set_type(buttons, LV_INDEV_TYPE_BUTTON);
    lv_indev_set_read_cb(buttons, controls_callback);
#endif
    // input
    gui_draw_init();
}

uint8_t p = 0;
extern uint32_t *debugbuffer32;
uint32_t inttimer = 0;


// use tmr1 for ms interval timer
//
void TMR1_OVF_TMR10_IRQHandler(void)
{
    if(tmr_interrupt_flag_get(TMR1, TMR_OVF_FLAG) != RESET)
    {
        /* lv_tick_inc(1); */
        timer++;
        tmr_flag_clear(TMR1, TMR_OVF_FLAG);
    }
    // overflow is 10ms
}
uint32_t redraw_speed, 
         redraw_temp_int, 
         redraw_temp_ext = 0;

void gui_draw_init(void) {
    lv_theme_default_init(NULL, lv_color_make(0, 0, 0), lv_color_make(255, 255, 255), LV_THEME_DEFAULT_DARK, &lv_font_andalemo_28);

    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_make(0, 0, 0), LV_PART_MAIN);

    // text tyles
    lv_style_set_text_font(&text_normal, &lv_font_andalemo_28);
    lv_style_set_bg_color(&text_normal, lv_color_make(0, 0, 0));
    lv_style_set_text_color(&text_normal, lv_color_make(255, 255, 255));
    lv_style_set_pad_top(&text_normal, 2);
    lv_style_set_text_align(&text_normal, LV_TEXT_ALIGN_LEFT);
    lv_style_set_text_font(&text_slim, &lv_font_fry_32);
    lv_style_set_bg_color(&text_slim, lv_color_make(0, 0, 0));
    lv_style_set_text_color(&text_slim, lv_color_make(255, 255, 255));

    // bars
    powerbar_positive = lv_bar_create(lv_screen_active());

    // positive
    static lv_style_t powerbar_style_normal, powerbar_style_normal_indicator;
    lv_style_set_border_color(&powerbar_style_normal, POWER_COLOR_NORMAL_DARK);
    lv_style_set_border_width(&powerbar_style_normal, 1);
    lv_style_set_radius(&powerbar_style_normal, 0);
    lv_style_init(&powerbar_style_normal_indicator);
    lv_style_set_bg_opa(&powerbar_style_normal_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&powerbar_style_normal_indicator, POWER_COLOR_NORMAL);
    lv_style_set_radius(&powerbar_style_normal_indicator, 0);

    lv_obj_set_pos(powerbar_positive, POWER_REGEN_WIDTH + POWER_BAR_WIDTH, POWER_TOP); 
    lv_obj_set_size(powerbar_positive, POWER_NORMAL_WIDTH, POWER_HEIGHT);
    lv_bar_set_range(powerbar_positive, 0, POWER_MAX);
    lv_obj_add_style(powerbar_positive, &powerbar_style_normal, LV_PART_MAIN);
    lv_obj_add_style(powerbar_positive, &powerbar_style_normal_indicator, LV_PART_INDICATOR);
    lv_bar_set_value(powerbar_positive, 50, LV_ANIM_OFF);

    // negative
    powerbar_negative = lv_bar_create(lv_screen_active());
    static lv_style_t powerbar_style_negative, powerbar_style_negative_indicator;
    lv_style_set_border_color(&powerbar_style_negative, POWER_COLOR_REGEN_DARK);
    lv_style_set_bg_opa(&powerbar_style_negative, LV_OPA_COVER);
    lv_style_set_border_width(&powerbar_style_negative, 1);
    lv_style_set_radius(&powerbar_style_negative, 0);
    lv_style_init(&powerbar_style_negative_indicator);
    lv_style_set_bg_opa(&powerbar_style_negative_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&powerbar_style_negative_indicator, POWER_COLOR_REGEN);
    lv_style_set_radius(&powerbar_style_negative_indicator, 0);

    lv_obj_set_pos(powerbar_negative, 0, POWER_TOP); 
    lv_obj_set_size(powerbar_negative, POWER_REGEN_WIDTH, POWER_HEIGHT);
    lv_obj_set_style_base_dir(powerbar_negative, LV_BASE_DIR_RTL, 0);
    lv_bar_set_range(powerbar_negative, 0, 250);
    lv_obj_add_style(powerbar_negative, &powerbar_style_negative, LV_PART_MAIN);
    lv_obj_add_style(powerbar_negative, &powerbar_style_negative_indicator, LV_PART_INDICATOR);
    lv_bar_set_value(powerbar_negative, 100, LV_ANIM_OFF);


    // lines

    // center bar
    //
    // text
    powerbar_text = lv_label_create(lv_screen_active());
    static lv_style_t powerbar_text_style;
    lv_style_init(&powerbar_text_style);
    lv_style_set_text_font(&powerbar_text_style, &lv_font_andalemo_28);
    lv_style_set_text_color(&powerbar_text_style, lv_color_make(255, 255, 255));
    lv_style_set_bg_color(&powerbar_text_style, POWER_COLOR_BG);
    lv_style_set_bg_opa(&powerbar_text_style, LV_OPA_COVER);
    lv_style_set_pad_top(&powerbar_text_style, 2);
    lv_style_set_text_align(&powerbar_text_style, LV_TEXT_ALIGN_LEFT);
    lv_obj_add_style(powerbar_text, &powerbar_text_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(powerbar_text, POWER_TEXT_X, POWER_TOP);
    lv_obj_set_size(powerbar_text, POWER_TEXT_WIDTH, 32);
    lv_label_set_text_fmt(powerbar_text, "-999", 12);
    lv_label_set_long_mode(powerbar_text, LV_LABEL_LONG_CLIP);
    


    temperature = lv_label_create(lv_screen_active());
    lv_obj_add_style(temperature, &text_slim, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(temperature, TEMP_TEXT_X, TEMP_TEXT_Y);
    lv_obj_set_size(temperature, TEMP_TEXT_WIDTH, TEMP_TEXT_HEIGHT);
    lv_obj_set_style_text_align(temperature, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(temperature, "%02d°c", 12);
    lv_label_set_long_mode(temperature, LV_LABEL_LONG_CLIP);

    /* voltage = lv_label_create(lv_screen_active()); */
    /* lv_obj_add_style(voltage, &text_normal, LV_PART_MAIN | LV_STATE_DEFAULT); */
    /* lv_obj_set_pos(voltage, TEMP_VOLT_TEXT_X, 32); */
    /* lv_obj_set_size(voltage, TEMP_VOLT_TEXT_WIDTH, 32); */
    /* lv_obj_set_style_text_align(voltage, LV_TEXT_ALIGN_LEFT, 0); */
    /* lv_label_set_text_fmt(voltage, "%2d.%1dv", 20, 1); */
    /* lv_label_set_long_mode(voltage, LV_LABEL_LONG_CLIP); */

    motor_temperature = lv_label_create(lv_screen_active());
    lv_obj_add_style(motor_temperature, &text_slim, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(motor_temperature, MOTOR_TEMP_TEXT_X, MOTOR_TEMP_TEXT_Y);
    lv_obj_set_size(motor_temperature, MOTOR_TEMP_TEXT_WIDTH, MOTOR_TEMP_TEXT_HEIGHT);
    lv_obj_set_style_text_align(motor_temperature, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(motor_temperature, "% 3d°c", 123);
    lv_label_set_long_mode(motor_temperature, LV_LABEL_LONG_CLIP);

    // speed, only draw minor since major takes too much memory due to 
    // drawing taking place in buffers
    //
    /* speed_major = lv_label_create(lv_screen_active()); */
    /* lv_obj_add_style(speed_major, &text_large, LV_PART_MAIN | LV_STATE_DEFAULT); */
    /* #<{(| lv_obj_set_style_text_font(speed_major, &lv_font_andalemo_144_numeric, 0); |)}># */
    /* lv_obj_set_pos(speed_major, SPEED_MAJOR_X, SPEED_MAJOR_Y); */
    /* lv_obj_set_size(speed_major, SPEED_MAJOR_WIDTH, SPEED_MAJOR_HEIGHT); */
    /* lv_obj_set_style_text_align(speed_major, LV_TEXT_ALIGN_RIGHT, 0); */
    /* lv_label_set_text_fmt(speed_major, "% 2ld", speed >> 8); */
    /* lv_label_set_long_mode(speed_major, LV_LABEL_LONG_CLIP); */

    speed_minor = lv_label_create(lv_screen_active());
    lv_obj_add_style(speed_minor, &text_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_style_set_pad_top(&powerbar_text_style, 2);
    lv_obj_set_style_text_font(speed_minor, &lv_font_andalemo_72, 0);
    lv_obj_set_pos(speed_minor, SPEED_MINOR_X, SPEED_MINOR_Y);
    lv_obj_set_size(speed_minor, SPEED_MINOR_WIDTH, SPEED_MINOR_HEIGHT);
    lv_obj_set_style_text_align(speed_minor, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(speed_minor, "%1ld", (speed & 0x000000FF) / 26);
    lv_label_set_long_mode(speed_minor, LV_LABEL_LONG_CLIP);


    // battery
    battery = lv_image_create(lv_screen_active());
    lv_image_set_src(battery, &battery_black);
    lv_obj_set_pos(battery, 0, 0);

    battery_bar  = lv_bar_create(lv_screen_active());
    static lv_style_t battery_style_bar, battery_style_bar_indicator;
    lv_style_set_bg_opa(&battery_style_bar, LV_OPA_COVER);
    lv_style_set_border_width(&battery_style_bar, 0);
    lv_style_set_radius(&battery_style_bar, 0);
    lv_style_init(&battery_style_bar_indicator);
    lv_style_set_bg_opa(&battery_style_bar_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&battery_style_bar_indicator, BATTERY_COLOR_90);
    lv_style_set_radius(&battery_style_bar_indicator, 0);

    lv_obj_set_pos(battery_bar, BATTERY_BAR_X, BATTERY_BAR_Y); 
    lv_obj_set_size(battery_bar, BATTERY_BAR_WIDTH, BATTERY_BAR_HEIGHT);
    lv_bar_set_range(battery_bar, 0, 100);
    lv_obj_add_style(battery_bar, &battery_style_bar, LV_PART_MAIN);
    lv_obj_add_style(battery_bar, &battery_style_bar_indicator, LV_PART_INDICATOR);
    lv_bar_set_value(battery_bar, 100, LV_ANIM_OFF);

    battery_label = lv_label_create(lv_screen_active());
    lv_obj_add_style(battery_label, &text_slim, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(battery_label, BATTERY_LABEL_X, BATTERY_LABEL_Y);
    lv_obj_set_size(battery_label, BATTERY_LABEL_WIDTH, BATTERY_LABEL_HEIGHT);
    lv_obj_set_style_text_align(battery_label, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(battery_label, "%02ld.%1ldv", 24, 0);
    lv_label_set_long_mode(battery_label, LV_LABEL_LONG_CLIP);

    // bottom text values
    total_distance_text = lv_label_create(lv_screen_active());
    lv_obj_add_style(total_distance_text, &text_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(total_distance_text, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(total_distance_text, "%04ld", 0);
    lv_label_set_long_mode(total_distance_text, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(total_distance_text, TOTAL_DISTANCE_TEXT_X, TOTAL_DISTANCE_TEXT_Y);
    lv_obj_set_size(total_distance_text, TOTAL_DISTANCE_TEXT_WIDTH, TOTAL_DISTANCE_TEXT_HEIGHT);

    trip_distance_text = lv_label_create(lv_screen_active());
    lv_obj_add_style(trip_distance_text, &text_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(trip_distance_text, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(trip_distance_text, "%04ld", 0);
    lv_label_set_long_mode(trip_distance_text, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(trip_distance_text, TRIP_DISTANCE_TEXT_X, TRIP_DISTANCE_TEXT_Y);
    lv_obj_set_size(trip_distance_text, TRIP_DISTANCE_TEXT_WIDTH, TRIP_DISTANCE_TEXT_HEIGHT);

    trip_time_text = lv_label_create(lv_screen_active());
    lv_obj_add_style(trip_time_text, &text_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(trip_time_text, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(trip_time_text, "%04ld", 0);
    lv_label_set_long_mode(trip_time_text, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(trip_time_text, TRIP_TIME_TEXT_X, TRIP_TIME_TEXT_Y);
    lv_obj_set_size(trip_time_text, TRIP_TIME_TEXT_WIDTH, TRIP_TIME_TEXT_HEIGHT);
    
    // bottom graph
    graph = lv_chart_create(lv_screen_active());
    lv_chart_set_type(graph, LV_CHART_TYPE_BAR);
    lv_obj_set_pos(graph, GRAPH_X, GRAPH_Y);
    lv_obj_set_size(graph, GRAPH_WIDTH, GRAPH_HEIGHT);
    lv_chart_set_range(graph, LV_CHART_AXIS_PRIMARY_X, 0, GRAPH_WIDTH);
    lv_chart_set_range(graph, LV_CHART_AXIS_PRIMARY_Y, 0, 30*256);
    lv_chart_set_axis_tick(graph, LV_CHART_AXIS_PRIMARY_Y, 0, 0, 6, 5, true, 28);

    static lv_style_t graph_style_main, graph_style_indicator, graph_style_item;
    lv_style_set_pad_left(&graph_style_main, 0);
    lv_style_set_pad_right(&graph_style_main, 0);
    lv_style_set_pad_bottom(&graph_style_main, 3);
    lv_style_set_pad_top(&graph_style_main, 3);
    lv_style_set_pad_column(&graph_style_main, 0);
    lv_style_set_bg_color(&graph_style_main, lv_color_make(0, 0, 0));
    lv_style_set_bg_opa(&graph_style_main, LV_OPA_COVER);
    lv_style_set_border_opa(&graph_style_main, LV_OPA_COVER);
    lv_style_set_border_width(&graph_style_main, 1);
    lv_style_set_border_color(&graph_style_main, GRAPH_BORDER_COLOR); // whole border 
    lv_style_set_radius(&graph_style_main, 0); // remove radius

    lv_style_set_line_width(&graph_style_item, 2);
    lv_style_set_bg_color(&graph_style_item, GRAPH_LEGEND_COLOR);
    lv_style_set_pad_column(&graph_style_item, 0);

    lv_obj_add_style(graph, &graph_style_item, LV_PART_ITEMS);
    lv_obj_add_style(graph, &graph_style_main, LV_PART_MAIN);

    lv_obj_set_style_size(graph, 0, LV_PART_INDICATOR); // disable dots
    lv_chart_set_div_line_count(graph, 10, 0);
    lv_chart_set_point_count(graph, GRAPH_WIDTH);
    lv_chart_set_update_mode(graph, LV_CHART_UPDATE_MODE_SHIFT);
    graph_series = lv_chart_add_series(graph, GRAPH_LINE_COLOR, LV_CHART_AXIS_PRIMARY_Y);
    /* lv_chart_set_ext_y_array(graph, graph_series, graph_array); */

    lv_obj_add_event_cb(graph, graph_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL); // add cb to draw lines more normal

#ifdef DEBUG
    debug_text = lv_label_create(lv_screen_active());
    static lv_style_t debug_text_style;
    lv_style_init(&debug_text_style);
    lv_style_set_text_font(&debug_text_style, &lv_font_andalemo_16);
    lv_style_set_text_color(&debug_text_style, lv_color_make(255, 255, 255));
    lv_style_set_bg_color(&debug_text_style, lv_color_make(0, 0, 220));
    lv_style_set_bg_opa(&debug_text_style, LV_OPA_COVER);
    lv_style_set_pad_top(&debug_text_style, 1);
    lv_obj_add_style(debug_text, &debug_text_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(debug_text, DEBUG_X, DEBUG_Y);
    lv_obj_set_size(debug_text, DEBUG_WIDTH, DEBUG_HEIGHT);
    lv_obj_set_style_text_align(debug_text, LV_TEXT_ALIGN_LEFT , 0);
    lv_label_set_text_fmt(debug_text, "DEBUG DEBUG DEBUG %08X", timer);
    lv_label_set_long_mode(debug_text, LV_LABEL_LONG_CLIP);
#endif


    // timers
    draw_graph_timer = lv_timer_create(_draw_graph, 1000, NULL); // update every interval period:w

    avg_timer = lv_timer_create(_avg_timer, 100, NULL);
#ifdef DEBUG
    sim_timer = lv_timer_create(_sim_update, 100, NULL);
#endif
}

void gui_timers_start(void) {

}

void gui_timers_stop(void) {
}

void gui_update(void) {
    _draw_speed();
    _draw_battery();
    _draw_power();
    _draw_temperature();
}

/*
 * lvgl uses too much memory when drawing large areas, so we dump the images streight into
 * the framebuffer
 */
static void _draw_speed(void) {
    if (speed != oldspeed) {
        int32_t tspeed = speed < 0 ? speed * -1 : speed;
        int32_t tospeed = oldspeed < 0 ? oldspeed * -1 : oldspeed;
        int32_t speed_major = (tspeed & 0x0000FF00) >> 8;
        int32_t speed_major_old = (tospeed & 0x0000FF00) >> 8;
        int32_t spd_minor = (tspeed & 0x000000FF);
        int32_t spd_minor_old = (tospeed & 0x000000FF);
        if (speed_major_old % 10 != speed_major % 10) {
            switch (speed_major % 10) {
                default:
                case 0:
                    lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_0);
                    break;
                case 1:
                    lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_1);
                    break;
                case 2:
                    lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_2);
                    break;
                case 3:
                    lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_3);
                    break;
                case 4:
                    lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_4);
                    break;
                case 5:
                    lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_5);
                    break;
                case 6:
                    lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_6);
                    break;
                case 7:
                    lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_7);
                    break;
                case 8:
                    lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_8);
                    break;
                case 9:
                    lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_9);
                    break;
            }
        }
        if (speed_major_old / 10 != speed_major / 10) {
            switch (speed_major / 10) {
                default:
                case 0:
                    lcd_fill(SPEED_MAJOR_X, SPEED_MAJOR_Y, SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y + SPEED_MAJOR_HEIGHT, 0x0000);
                    break;
                case 1:
                    lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_1);
                    break;
                case 2:
                    lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_2);
                    break;
                case 3:
                    lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_3);
                    break;
                case 4:
                    lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_4);
                    break;
                case 5:
                    lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_5);
                    break;
                case 6:
                    lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_6);
                    break;
                case 7:
                    lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_7);
                    break;
                case 8:
                    lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_8);
                    break;
                case 9:
                    lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_9);
                    break;
            }
        }
        /* lv_label_set_text_fmt(speed_major, "% 2ld", tspeed >> 8); */
        if (spd_minor_old != spd_minor) lv_label_set_text_fmt(speed_minor, "%1ld", spd_minor / 26);

    }
    oldspeed = speed;
}

void _draw_graph(lv_timer_t *timer) {
    lv_chart_set_next_value(graph, graph_series, avg_speed);
}

static void _draw_battery(void) {
    battery_voltage += (voltage_ebat() - battery_voltage) >> FILTER_SHIFT;
    if (battery_voltage != battery_voltage_old) {
        lv_label_set_text_fmt(battery_label, "% 2ld.%1ldv", battery_voltage >> 8, (battery_voltage & 0x000000FF) / 26);
        battery_voltage_old = battery_voltage;
    }
    if (battery_value != battery_value_old) {
        lv_bar_set_value(battery_bar, battery_value, LV_ANIM_OFF); 
        if (battery_value < 10) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_10, LV_PART_INDICATOR);
        else if (battery_value < 20) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_20, LV_PART_INDICATOR);
        else if (battery_value < 30) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_30, LV_PART_INDICATOR);
        else if (battery_value < 40) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_40, LV_PART_INDICATOR);
        else if (battery_value < 50) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_50, LV_PART_INDICATOR);
        else if (battery_value < 60) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_60, LV_PART_INDICATOR);
        else if (battery_value < 70) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_70, LV_PART_INDICATOR);
        else if (battery_value < 80) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_80, LV_PART_INDICATOR);
        else if (battery_value < 90) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_90, LV_PART_INDICATOR);
        else lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_100 , LV_PART_INDICATOR);
        battery_value_old = battery_value;
    }
}


static void _draw_power(void) {
    if (power_value == power_value_old) {
        // do nothing
    } else if (power_value >= 0 && power_value_old > 0) {
        lv_bar_set_value(powerbar_positive, power_value, LV_ANIM_OFF);
        lv_bar_set_value(powerbar_negative, 0, LV_ANIM_OFF);
        lv_label_set_text_fmt(powerbar_text, "%ld", power_value);
    } else if (power_value >= 0 && power_value_old < 0) {
        lv_bar_set_value(powerbar_positive, power_value, LV_ANIM_OFF);
        lv_bar_set_value(powerbar_negative, 0, LV_ANIM_OFF);
        lv_label_set_text_fmt(powerbar_text, "%ld", power_value);
    } else if (power_value < 0 && power_value_old >= 0) {
        lv_bar_set_value(powerbar_positive, 0, LV_ANIM_OFF);
        lv_bar_set_value(powerbar_negative, power_value * -1, LV_ANIM_OFF);
        lv_label_set_text_fmt(powerbar_text, "%ld", power_value);
    } else if (power_value < 0 && power_value_old < 0) {
        lv_bar_set_value(powerbar_positive, 0, LV_ANIM_OFF);
        lv_bar_set_value(powerbar_negative, power_value * -1, LV_ANIM_OFF);
        lv_label_set_text_fmt(powerbar_text, "%ld", power_value);
    }

    power_value_old = power_value;
}

static void _draw_temperature(void) {
   if (ext_temperature_old != ext_temperature) {
       lv_label_set_text_fmt(temperature, "% 2d.%1d°c", ext_temperature >> 8, (ext_temperature & 0x000000FF) / 26);
   }

   int_temperature_old = int_temperature;
   ext_temperature_old = ext_temperature;
}

static void _avg_timer(lv_timer_t *timer) {
    if (ext_temperature == 0) ext_temperature = ext_temp();
    if (int_temperature == 0) int_temperature = int_temp();
    if (avg_speed == 0) avg_speed = speed;
    if (power_value == 0) avg_power = power_value;
    ext_temperature += (ext_temp() - ext_temperature) >> FILTER_SHIFT;
    int_temperature += (int_temp() - int_temperature) >> FILTER_SHIFT;

    avg_speed += (speed - avg_speed) >> FILTER_SHIFT;
    avg_power += (power_value - avg_power) >> FILTER_SHIFT;
}

#ifdef DEBUG
void _sim_update(lv_timer_t *timer) {
    power_value+=5;
    battery_value+=1;
    speed += 96;
    if (power_value > POWER_MAX) power_value = POWER_MIN;
    if (speed > (30 << 8)) speed = 0;
    if (battery_value > 100) battery_value =0;
}
#endif

// callbacks lvgl
static void graph_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);

    lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
    if(dsc->part == LV_PART_MAIN) {
        if(dsc->line_dsc == NULL || dsc->p1 == NULL || dsc->p2 == NULL) return;

        if(dsc->p1->y == dsc->p2->y) {
            dsc->line_dsc->color  = GRAPH_DIV_COLOR;
        }
    }
}
