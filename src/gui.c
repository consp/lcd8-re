#include "gui.h"
#include "img.h"
#include "lvgl.h"
#include "controls.h"
#include "eeprom.h"
#include "clock.h"
#include "uart.h"
#include "comm.h"

extern volatile uint32_t timer_counter;
uint8_t pixelbuffer[PIXEL_BUFFER_LINES * DISPLAY_WIDTH * 2];
uint32_t timer_old = 0;

/*** 
 * data variables
 */

extern settings_t settings; 
int32_t power_value = 0, power_value_old = 0;
int32_t speed = 0, oldspeed = 0;
int32_t battery_value = 0, battery_value_old = 0;
int32_t battery_voltage = 0, battery_voltage_old = 1;
int32_t battery_voltage_controller = 0;
int32_t battery_current = 0;
int32_t int_temperature = 0, int_temperature_old = 0;
int32_t ext_temperature = 0, ext_temperature_old = 0;
int32_t wheel_circumfence = 2230; //(in mm)
int32_t mot_temperature = 0;
int32_t con_temperature = 0;
int32_t avg_speed = 0, avg_power = 0;
uint8_t brake = 0;
uint8_t controller_mode = 0;
uint8_t draw_time_trigger = 0;
uint8_t draw_distances_trigger = 0;
uint8_t draw_power_trigger = 0;
uint8_t draw_temperatures_trigger = 0;
uint8_t draw_speed_trigger = 0;
uint8_t draw_assist_trigger = 0;
uint8_t draw_battery_voltage_trigger = 0;
uint8_t draw_lights_trigger = 0;
uint8_t draw_brake_trigger = 0;
uint8_t draw_controller_mode_trigger = 0;
modes  mode = MODE_NORMAL, mode_back = MODE_NORMAL;

const lv_point_t fakepoints[4] = { {0, 0}, {1, 1}, {2, 2}, {3, 3}}; // work around the horrible lvgl buttons/key system
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
#if LV_USE_METER
lv_meter_indicator_t *indic1, *indic2; 
#endif
#ifdef DEBUG
lv_obj_t *debug_text = NULL;
#endif

lv_obj_t *powerbar_positive = NULL;
lv_obj_t *powerbar_negative = NULL;
lv_obj_t *powerbar_center_line = NULL;
lv_obj_t *powerbar_high_line = NULL;
lv_obj_t *powerbar_shade = NULL;
lv_obj_t *powerbar_text = NULL;

lv_obj_t *temperature_img = NULL;
lv_obj_t *temperature = NULL;
lv_obj_t *voltage = NULL;
lv_obj_t *motor_temperature_img = NULL;
lv_obj_t *motor_temperature = NULL;
lv_obj_t *battery = NULL;
lv_obj_t *battery_bar = NULL;
lv_obj_t *battery_label = NULL;
lv_obj_t *speed_minor = NULL;
lv_obj_t *assist_mode = NULL;
lv_obj_t *meter = NULL;

lv_obj_t *trip_distance_img = NULL;
lv_obj_t *trip_distance_text = NULL;
lv_obj_t *trip_time_img = NULL;
lv_obj_t *trip_time_text = NULL;
lv_obj_t *total_distance_img = NULL;
lv_obj_t *total_distance_text = NULL;
lv_obj_t *avg_speed_text = NULL;
lv_obj_t *max_speed_text = NULL;
lv_obj_t *lights_img = NULL;

lv_obj_t *brake_img = NULL;

lv_obj_t *graph = NULL; 
lv_chart_series_t *graph_series = NULL;
lv_chart_cursor_t *graph_cursor = NULL;

lv_obj_t *cm_label = NULL;

lv_anim_t anim_reset_trip;

lv_style_t text_slim, text_normal;

// images
//
extern const lv_image_dsc_t battery_black;
extern const lv_image_dsc_t icon_clock;
extern const lv_image_dsc_t icon_trip;
extern const lv_image_dsc_t icon_journey;
extern const lv_image_dsc_t icon_temperature;
extern const lv_image_dsc_t icon_engine;
extern const lv_image_dsc_t icon_brake;
extern const lv_image_dsc_t icon_headlight;
extern const lv_image_dsc_t icon_headlight_auto;

// draw and local functions
static void _draw_speed(void);
static void _draw_battery(void);
static void _draw_power(void);
static void _draw_temperature(void);
static void _draw_distances(void);
static void _draw_time(void);
static void _draw_assist(void);
static void _draw_headlights(void);
static void _draw_brake(void);
static void _draw_controller_mode(void);

void _draw_graph(lv_timer_t *timer);
void gui_draw_normal(void);
void gui_draw_settings_main(void);
void gui_draw_settings_display(void);
void gui_draw_settings_controller(void);

void print_time(lv_obj_t *object, uint32_t value);
void print_digit_text(lv_obj_t *object, uint32_t value, uint32_t length, uint32_t decimals, lv_color_t active_color, lv_color_t passive_color);

static void _100ms_timer(lv_timer_t *timer);
static void _500ms_timer(lv_timer_t *timer);
// buttons callback
void _button_presses(void);

// timers
lv_timer_t *draw_graph_timer = NULL;
lv_timer_t *ms100_timer = NULL;
lv_timer_t *ms500_timer = NULL;

// lvgl local
static void graph_event_pre_cb(lv_event_t * e);

void gui_init(void) {
    // setup timer
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
    
#endif
    // input
    mode = MODE_NORMAL;
    gui_draw_normal();
    /* mode = MODE_SETTINGS_MAIN; */
    /* gui_draw_settings_main(); */

    // make light
    lcd_backlight(settings.backlight_level);
}

uint8_t p = 0;
uint32_t inttimer = 0;


uint32_t redraw_speed, 
         redraw_temp_int, 
         redraw_temp_ext = 0;

void gui_draw_normal(void) {
    lv_theme_default_init(NULL, COLOR_BLACK, COLOR_WHITE, LV_THEME_DEFAULT_DARK, &lv_font_andalemo_32);

    lv_obj_set_style_bg_color(lv_screen_active(), COLOR_BLACK, LV_PART_MAIN);
    lv_obj_set_layout(lv_screen_active(), 0);

    // text tyles
    lv_style_set_text_font(&text_normal, &lv_font_andalemo_32);
    lv_style_set_bg_color(&text_normal, COLOR_BLACK);
    lv_style_set_text_color(&text_normal, COLOR_WHITE);
    lv_style_set_pad_top(&text_normal, 3);
    lv_style_set_text_align(&text_normal, LV_TEXT_ALIGN_LEFT);
    lv_style_set_text_font(&text_slim, &lv_font_fry_32);
    lv_style_set_bg_color(&text_slim, COLOR_BLACK);
    lv_style_set_text_color(&text_slim, COLOR_WHITE);

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

    lv_obj_set_pos(powerbar_positive, POWER_REGEN_WIDTH + POWER_BAR_WIDTH, POWER_Y + POWER_BORDER); 
    lv_obj_set_size(powerbar_positive, POWER_NORMAL_WIDTH, POWER_HEIGHT - (POWER_BORDER * 2));
    lv_bar_set_range(powerbar_positive, 0, settings.power_max);
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

    lv_obj_set_pos(powerbar_negative, POWER_BORDER, POWER_Y + POWER_BORDER); 
    lv_obj_set_size(powerbar_negative, POWER_REGEN_WIDTH - POWER_BORDER, POWER_HEIGHT - (POWER_BORDER * 2));
    lv_obj_set_style_base_dir(powerbar_negative, LV_BASE_DIR_RTL, 0);
    lv_bar_set_range(powerbar_negative, 0, settings.power_min * -1);
    lv_obj_add_style(powerbar_negative, &powerbar_style_negative, LV_PART_MAIN);
    lv_obj_add_style(powerbar_negative, &powerbar_style_negative_indicator, LV_PART_INDICATOR);
    lv_bar_set_value(powerbar_negative, 100, LV_ANIM_OFF);


    // lines
    lv_obj_t *powerbar_border = lv_line_create(lv_screen_active());
    static lv_point_precise_t powerbar_border_points[] = { {0, 0}, {DISPLAY_WIDTH-1, 0}, {DISPLAY_WIDTH-1, POWER_HEIGHT - 1}, {0, POWER_HEIGHT - 1}, {0, 0}};
    static lv_style_t powerbar_border_style;
    lv_style_init(&powerbar_border_style);
    lv_style_set_line_width(&powerbar_border_style, POWER_BORDER);
    lv_style_set_line_color(&powerbar_border_style, POWER_COLOR_BG);
    lv_style_set_line_rounded(&powerbar_border_style, false);
    lv_line_set_points(powerbar_border, powerbar_border_points, 5);
    lv_obj_add_style(powerbar_border, &powerbar_border_style, LV_PART_MAIN);
    lv_obj_set_pos(powerbar_border, POWER_X, POWER_Y);

    // center bar
    lv_obj_t *powerbar_centerline = lv_line_create(lv_screen_active());
    static lv_point_precise_t powerbar_centerline_points[] = { {0, POWER_BORDER}, {0, POWER_HEIGHT - (2 * POWER_BORDER)}, {POWER_BAR_WIDTH - 1, POWER_HEIGHT - (2 * POWER_BORDER)}, {POWER_BAR_WIDTH - 1, 0}};
    static lv_style_t powerbar_centerline_style;
    lv_style_init(&powerbar_centerline_style);
    lv_style_set_line_width(&powerbar_centerline_style, 1);
    lv_style_set_line_color(&powerbar_centerline_style, POWER_COLOR_BAR);
    lv_style_set_line_rounded(&powerbar_centerline_style, false);
    lv_line_set_points(powerbar_centerline, powerbar_centerline_points , 4);
    lv_obj_add_style(powerbar_centerline, &powerbar_centerline_style, LV_PART_MAIN);
    lv_obj_set_pos(powerbar_centerline, POWER_X + POWER_REGEN_WIDTH, POWER_Y + POWER_BORDER);

    // text
    powerbar_text = lv_label_create(lv_screen_active());
    static lv_style_t powerbar_text_style;
    lv_style_init(&powerbar_text_style);
    lv_style_set_text_font(&powerbar_text_style, &lv_font_andalemo_32);
    lv_style_set_text_color(&powerbar_text_style, COLOR_WHITE);
    lv_style_set_bg_color(&powerbar_text_style, POWER_COLOR_BG);
    lv_style_set_bg_opa(&powerbar_text_style, LV_OPA_COVER);
    lv_style_set_pad_top(&powerbar_text_style, 2);
    lv_style_set_text_align(&powerbar_text_style, LV_TEXT_ALIGN_LEFT);
    lv_obj_add_style(powerbar_text, &powerbar_text_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(powerbar_text, POWER_TEXT_X, POWER_Y + 1);
    lv_obj_set_size(powerbar_text, POWER_TEXT_WIDTH, 32);
    lv_label_set_text_fmt(powerbar_text, "-999");
    lv_label_set_long_mode(powerbar_text, LV_LABEL_LONG_CLIP);

    temperature_img = lv_image_create(lv_screen_active());
    lv_image_set_src(temperature_img, &icon_temperature);
    lv_obj_set_pos(temperature_img, TEMP_IMG_X, TEMP_IMG_Y);
    temperature = lv_label_create(lv_screen_active());
    lv_obj_add_style(temperature, &text_slim, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(temperature, TEMP_TEXT_X, TEMP_TEXT_Y);
    lv_obj_set_size(temperature, TEMP_TEXT_WIDTH, TEMP_TEXT_HEIGHT);
    lv_obj_set_style_text_align(temperature, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(temperature, "%02d°C", 12);
    lv_label_set_long_mode(temperature, LV_LABEL_LONG_CLIP);

    /* voltage = lv_label_create(lv_screen_active()); */
    /* lv_obj_add_style(voltage, &text_normal, LV_PART_MAIN | LV_STATE_DEFAULT); */
    /* lv_obj_set_pos(voltage, TEMP_VOLT_TEXT_X, 32); */
    /* lv_obj_set_size(voltage, TEMP_VOLT_TEXT_WIDTH, 32); */
    /* lv_obj_set_style_text_align(voltage, LV_TEXT_ALIGN_LEFT, 0); */
    /* lv_label_set_text_fmt(voltage, "%2d.%1dv", 20, 1); */
    /* lv_label_set_long_mode(voltage, LV_LABEL_LONG_CLIP); */

    motor_temperature_img = lv_image_create(lv_screen_active());
    lv_image_set_src(motor_temperature_img, &icon_engine);
    lv_obj_set_pos(motor_temperature_img, MOTOR_TEMP_IMG_X, MOTOR_TEMP_IMG_Y);
    motor_temperature = lv_label_create(lv_screen_active());
    lv_obj_add_style(motor_temperature, &text_slim, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(motor_temperature, MOTOR_TEMP_TEXT_X, MOTOR_TEMP_TEXT_Y);
    lv_obj_set_size(motor_temperature, MOTOR_TEMP_TEXT_WIDTH, MOTOR_TEMP_TEXT_HEIGHT);
    lv_obj_set_style_text_align(motor_temperature, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(motor_temperature, "% 3d°C", 123);
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
    lv_obj_set_style_text_font(speed_minor, &lv_font_andalemo_72, 0);
    lv_obj_set_pos(speed_minor, SPEED_MINOR_X, SPEED_MINOR_Y);
    lv_obj_set_size(speed_minor, SPEED_MINOR_WIDTH, SPEED_MINOR_HEIGHT);
    lv_obj_set_style_text_align(speed_minor, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_pad_left(speed_minor, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(speed_minor, 0, LV_PART_MAIN);
    lv_label_set_text_fmt(speed_minor, "%1ld", (speed & 0x000000FF) / 26);
    lv_label_set_long_mode(speed_minor, LV_LABEL_LONG_CLIP);

    // brake icon
    brake_img = lv_image_create(lv_screen_active());
    lv_image_set_src(brake_img, &icon_brake);
    lv_obj_set_pos(brake_img, BRAKE_IMG_X, BRAKE_IMG_Y);
    lv_obj_add_flag(brake_img, LV_OBJ_FLAG_HIDDEN);


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
    lv_style_set_bg_color(&battery_style_bar_indicator, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_bg_grad_color(&battery_style_bar_indicator, lv_palette_main(LV_PALETTE_GREEN));
    lv_style_set_bg_grad_dir(&battery_style_bar_indicator, LV_GRAD_DIR_HOR);
    lv_style_set_radius(&battery_style_bar_indicator, 0);

    lv_obj_set_pos(battery_bar, BATTERY_BAR_X, BATTERY_BAR_Y); 
    lv_obj_set_size(battery_bar, BATTERY_BAR_WIDTH, BATTERY_BAR_HEIGHT);
    lv_bar_set_range(battery_bar, 0, 100);
    lv_obj_add_style(battery_bar, &battery_style_bar, LV_PART_MAIN);
    lv_obj_add_style(battery_bar, &battery_style_bar_indicator, LV_PART_INDICATOR);
    lv_bar_set_value(battery_bar, 0, LV_ANIM_OFF);

    battery_label = lv_label_create(lv_screen_active());
    lv_obj_add_style(battery_label, &text_slim, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(battery_label, BATTERY_LABEL_X, BATTERY_LABEL_Y);
    lv_obj_set_size(battery_label, BATTERY_LABEL_WIDTH, BATTERY_LABEL_HEIGHT);
    lv_obj_set_style_text_align(battery_label, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(battery_label, "%02d.%1dV", 24, 0);
    lv_label_set_long_mode(battery_label, LV_LABEL_LONG_CLIP);

    // bottom text values
    total_distance_img = lv_image_create(lv_screen_active());
    lv_image_set_src(total_distance_img, &icon_journey);
    lv_obj_set_pos(total_distance_img, TOTAL_DISTANCE_IMG_X, TOTAL_DISTANCE_IMG_Y);
    total_distance_text = lv_label_create(lv_screen_active());
    lv_obj_add_style(total_distance_text, &text_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(total_distance_text, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(total_distance_text, "%04d", 0);
    lv_label_set_long_mode(total_distance_text, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(total_distance_text, TOTAL_DISTANCE_TEXT_X, TOTAL_DISTANCE_TEXT_Y);
    lv_obj_set_size(total_distance_text, TOTAL_DISTANCE_TEXT_WIDTH, TOTAL_DISTANCE_TEXT_HEIGHT);
    lv_label_set_recolor(total_distance_text, true);

    trip_distance_img = lv_image_create(lv_screen_active());
    lv_image_set_src(trip_distance_img, &icon_trip);
    lv_obj_set_pos(trip_distance_img, TRIP_DISTANCE_IMG_X, TRIP_DISTANCE_IMG_Y);
    trip_distance_text = lv_label_create(lv_screen_active());
    lv_obj_add_style(trip_distance_text, &text_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(trip_distance_text, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(trip_distance_text, "%04d", 0);
    lv_label_set_long_mode(trip_distance_text, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(trip_distance_text, TRIP_DISTANCE_TEXT_X, TRIP_DISTANCE_TEXT_Y);
    lv_obj_set_size(trip_distance_text, TRIP_DISTANCE_TEXT_WIDTH, TRIP_DISTANCE_TEXT_HEIGHT);
    lv_label_set_recolor(trip_distance_text, true);

    trip_time_img = lv_image_create(lv_screen_active());
    lv_image_set_src(trip_time_img, &icon_clock);
    lv_obj_set_pos(trip_time_img, TRIP_TIME_IMG_X, TRIP_TIME_IMG_Y);
    trip_time_text = lv_label_create(lv_screen_active());
    lv_obj_add_style(trip_time_text, &text_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(trip_time_text, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(trip_time_text, "%04d", 0);
    lv_label_set_long_mode(trip_time_text, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(trip_time_text, TRIP_TIME_TEXT_X, TRIP_TIME_TEXT_Y);
    lv_obj_set_size(trip_time_text, TRIP_TIME_TEXT_WIDTH, TRIP_TIME_TEXT_HEIGHT);
    
    // bottom graph
    graph = lv_chart_create(lv_screen_active());
    lv_chart_set_type(graph, LV_CHART_TYPE_LINE);
    lv_obj_set_pos(graph, GRAPH_X + 24, GRAPH_Y);
    lv_obj_set_size(graph, GRAPH_WIDTH - 24, GRAPH_HEIGHT);
    lv_chart_set_range(graph, LV_CHART_AXIS_PRIMARY_X, 0, GRAPH_POINT_COUNT);
    lv_chart_set_range(graph, LV_CHART_AXIS_PRIMARY_Y, 0, settings.graph_max * 1000);
#if LVGL_VERSION_MAJOR == 8
    lv_chart_set_axis_tick(graph, LV_CHART_AXIS_PRIMARY_Y, 0, 0, settings.graph_max / 5, 1, true, 28); // minor must be >=1 due to multiplication bug (*0)
    /* lv_chart_set_axis_tick(graph, LV_CHART_AXIS_PRIMARY_X, 0, 0, 0, 0, false, 0); */
#else
        /*Create a scale also with 100% width*/
    lv_obj_t * graph_scale = lv_scale_create(lv_screen_active());
    lv_scale_set_mode(graph_scale, LV_SCALE_MODE_VERTICAL_LEFT);
    lv_obj_set_pos(graph_scale, GRAPH_X, GRAPH_Y);
    lv_obj_set_size(graph_scale, GRAPH_HEIGHT, 24);
    lv_scale_set_total_tick_count(graph_scale, 6);
    lv_scale_set_major_tick_every(graph_scale, 1);
    /* lv_obj_set_style_pad_hor(graph_scale, lv_chart_get_first_point_center_offset(chart), 0); */
    static const char * yaxis[] = {"0", "5", "10", "15", "20", "25", "30", NULL};
    lv_scale_set_text_src(graph_scale, yaxis);
#endif

    static lv_style_t graph_style_main, graph_style_ticks, graph_style_item;
    lv_style_set_pad_left(&graph_style_main, 0);
    lv_style_set_pad_right(&graph_style_main, 0);
    lv_style_set_pad_bottom(&graph_style_main, 3);
    lv_style_set_pad_top(&graph_style_main, 3);
    lv_style_set_pad_column(&graph_style_main, 0);
    lv_style_set_bg_color(&graph_style_main, COLOR_BLACK);
    lv_style_set_bg_opa(&graph_style_main, LV_OPA_COVER);
    lv_style_set_border_opa(&graph_style_main, LV_OPA_COVER);
    lv_style_set_border_width(&graph_style_main, 1);
    lv_style_set_border_color(&graph_style_main, GRAPH_BORDER_COLOR); // whole border 
    lv_style_set_radius(&graph_style_main, 0); // remove radius

    lv_style_set_line_width(&graph_style_item, 3);
    lv_style_set_bg_color(&graph_style_item, GRAPH_LEGEND_COLOR);
    lv_style_set_pad_column(&graph_style_item, 0);

    lv_style_set_line_width(&graph_style_ticks, 1);
    lv_style_set_line_color(&graph_style_ticks, GRAPH_LEGEND_COLOR);
    lv_style_set_text_color(&graph_style_ticks, GRAPH_LEGEND_COLOR);
    lv_style_set_text_font(&graph_style_ticks, &lv_font_andalemo_12);

    lv_obj_add_style(graph, &graph_style_item, LV_PART_ITEMS);
    lv_obj_add_style(graph, &graph_style_main, LV_PART_MAIN);
    lv_obj_add_style(graph, &graph_style_ticks, LV_PART_TICKS);

#if LVGL_VERSION_MAJOR == 8
    lv_obj_set_style_size(graph, 0, LV_PART_INDICATOR); // disable dots
#else
    lv_obj_set_style_size(graph, 0, 0, LV_PART_INDICATOR); // disable dots
#endif
    if (!settings.graph_shift) graph_cursor = lv_chart_add_cursor(graph, GRAPH_CURSOR_COLOR, LV_DIR_TOP | LV_DIR_BOTTOM);

    lv_chart_set_div_line_count(graph, settings.speed_max / 5, 0);
    lv_chart_set_point_count(graph, GRAPH_POINT_COUNT);
    /* lv_chart_set_update_mode(graph, LV_CHART_UPDATE_MODE_SHIFT); */
    lv_chart_set_update_mode(graph, settings.graph_shift ? LV_CHART_UPDATE_MODE_SHIFT : LV_CHART_UPDATE_MODE_CIRCULAR);
    graph_series = lv_chart_add_series(graph, GRAPH_LINE_COLOR, LV_CHART_AXIS_PRIMARY_Y);
    /* lv_chart_set_ext_y_array(graph, graph_series, graph_array); */
    lv_chart_set_all_value(graph, graph_series, 0);
    lv_obj_add_event_cb(graph, graph_event_pre_cb, LV_EVENT_DRAW_PART_BEGIN, NULL); // add cb to draw lines more normal
                                                                                    //
    cm_label = lv_label_create(lv_screen_active());
    lv_obj_add_style(cm_label, &text_slim, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(cm_label, CM_LABEL_X, CM_LABEL_Y);
    lv_obj_set_size(cm_label, CM_LABEL_WIDTH, CM_LABEL_HEIGHT);
    lv_obj_set_style_text_align(cm_label, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text(cm_label, "X");

#ifdef DEBUG
    /* debug_text = lv_label_create(lv_screen_active()); */
    /* static lv_style_t debug_text_style; */
    /* lv_style_init(&debug_text_style); */
    /* lv_style_set_text_font(&debug_text_style, &lv_font_andalemo_16); */
    /* lv_style_set_text_color(&debug_text_style, COLOR_WHITE); */
    /* lv_style_set_bg_color(&debug_text_style, lv_color_make(0, 0, 220)); */
    /* lv_style_set_bg_opa(&debug_text_style, LV_OPA_COVER); */
    /* lv_style_set_pad_top(&debug_text_style, 1); */
    /* lv_obj_add_style(debug_text, &debug_text_style, LV_PART_MAIN | LV_STATE_DEFAULT); */
    /* lv_obj_set_pos(debug_text, DEBUG_X, DEBUG_Y); */
    /* lv_obj_set_size(debug_text, DEBUG_WIDTH, DEBUG_HEIGHT); */
    /* lv_obj_set_style_text_align(debug_text, LV_TEXT_ALIGN_LEFT , 0); */
    /* lv_label_set_text_fmt(debug_text, "DEBUG DEBUG DEBUG %08X", timer_counter); */
    /* lv_label_set_long_mode(debug_text, LV_LABEL_LONG_CLIP); */
    /* lv_obj_add_flag(debug_text, LV_OBJ_FLAG_HIDDEN); */
#endif

    /* meter = lv_meter_create(lv_scr_act()); */
    /* lv_obj_set_pos(meter, 0, 64); */
    /* lv_obj_set_size(meter, 320, 320); */
    /*  */
    /* lv_style_t meter_main_style; */
    /* lv_style_init(&meter_main_style); */
    /* lv_style_set_bg_color(&meter_main_style, COLOR_BLACK); */
    /* lv_style_set_bg_opa(&meter_main_style, LV_OPA_TRANSP); */
    /* lv_style_set_border_width(&meter_main_style, 0); */
    /* lv_obj_add_style(meter, &meter_main_style, LV_PART_MAIN | LV_PART_TICKS); */
    /* lv_obj_set_style_bg_color(meter, COLOR_BLACK, LV_PART_MAIN); */
    /* lv_obj_set_style_bg_opa(meter, LV_OPA_TRANSP, LV_PART_MAIN); */
    /*  */
    /* #<{(|Remove the circle from the middle|)}># */
    /* lv_obj_remove_style(meter, NULL, LV_PART_INDICATOR); */
    /*  */
    /* #<{(|Add a scale first|)}># */
    /* lv_meter_scale_t * scale_power = lv_meter_add_scale(meter); */
    /* lv_meter_scale_t * scale_speed = lv_meter_add_scale(meter); */
    /* lv_meter_set_scale_ticks(meter, scale_power, 12, 2, 16, COLOR_GREY); */
    /* lv_meter_set_scale_major_ticks(meter, scale_power, 1, 3, 16, COLOR_GREY, 24); */
    /* lv_meter_set_scale_range(meter, scale_power, -25, settings.power_max, 180, 180); */
    /*  */
    /* lv_meter_set_scale_ticks(meter, scale_speed, 1 + (max_speed / 5), 2, 16, COLOR_WHITE); */
    /* lv_meter_set_scale_major_ticks(meter, scale_speed, 1, 2, 30, lv_color_hex3(0xA00), -15); */
    /* lv_meter_set_scale_range(meter, scale_speed, 0, max_speed, 180, 180); */
    /*  */
    /* #<{(|Add a three arc indicator|)}># */
    /* indic1 = lv_meter_add_arc(meter, scale_speed, 16, lv_palette_main(LV_PALETTE_RED), 0); */
    /* indic2 = lv_meter_add_arc(meter, scale_power, 16, lv_palette_main(LV_PALETTE_GREEN), -17); */
    /* lv_meter_set_indicator_end_value(meter, indic1, 25); */
    /* lv_meter_set_indicator_end_value(meter, indic2, 250); */

    // assist roller
    static lv_style_t assist_mode_selected_style, assist_mode_normal_style;
    lv_style_init(&assist_mode_selected_style);
    lv_style_set_text_font(&assist_mode_selected_style, &lv_font_andalemo_72);
    lv_style_set_bg_opa(&assist_mode_selected_style, LV_OPA_TRANSP);
    lv_style_set_bg_color(&assist_mode_selected_style, COLOR_BLACK);
    lv_style_set_text_align(&assist_mode_selected_style, LV_TEXT_ALIGN_CENTER);

    lv_style_init(&assist_mode_selected_style);
    lv_style_set_text_font(&assist_mode_normal_style, &lv_font_andalemo_72);
    lv_style_set_bg_opa(&assist_mode_normal_style, LV_OPA_TRANSP);
    lv_style_set_bg_color(&assist_mode_normal_style, COLOR_BLACK);
    lv_style_set_text_align(&assist_mode_normal_style, LV_TEXT_ALIGN_CENTER);


    const char * opts;
    switch (settings.assist_levels) {
        default:
        case 5:
            opts = "0\n1\n2\n3\n4\n5\n";
        case 9:
            opts = "0\n1\n2\n3\n4\n5\n6\n7\n8\n9";
            break;
        case 10:
            break;
    }

    /*A roller on the left with left aligned text, and custom width*/
    assist_mode = lv_roller_create(lv_screen_active());
    lv_roller_set_options(assist_mode, opts, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(assist_mode, 1);
    lv_obj_set_pos(assist_mode, ASSIST_X, ASSIST_Y);
    lv_obj_set_size(assist_mode, ASSIST_WIDTH, ASSIST_HEIGHT);
    lv_obj_add_style(assist_mode, &assist_mode_normal_style, LV_PART_MAIN);
    lv_obj_add_style(assist_mode, &assist_mode_selected_style, LV_PART_SELECTED);
    /* lv_obj_set_style_text_font(assist_mode, &lv_font_andalemo_72, LV_PART_MAIN | LV_PART_SELECTED); */
    /* lv_obj_set_style_text_align(assist_mode, LV_TEXT_ALIGN_CENTER, 0); */
    /* lv_obj_align(assist_mode, LV_ALIGN_LEFT_MID, 10, 0); */
    /* lv_obj_add_event_cb(assist_mode, event_handler, LV_EVENT_ALL, NULL); */
    lv_roller_set_selected(assist_mode, settings.assist_last, LV_ANIM_OFF);

    lights_img = lv_image_create(lv_screen_active());
    lv_image_set_src(lights_img, &icon_headlight);
    lv_obj_set_pos(lights_img, LIGHTS_IMG_X, LIGHTS_IMG_Y);

    // timers
    //
    if (draw_graph_timer) {
        //
        lv_timer_del(draw_graph_timer);
    }
    draw_graph_timer = lv_timer_create(_draw_graph, (settings.graph_duration * 60000) / GRAPH_POINT_COUNT, NULL); // update every interval period

    if (ms100_timer == NULL) {
        ext_temperature = ext_temp();
        int_temperature = int_temp();

        avg_speed = speed;
        avg_power = power_value;
        battery_voltage = voltage_ebat();
    }

    if (ms100_timer == NULL) ms100_timer = lv_timer_create(_100ms_timer, 100, NULL);
    if (ms500_timer == NULL) ms500_timer = lv_timer_create(_500ms_timer, 500, NULL);

    draw_lights_trigger = 1;
    draw_speed_trigger = 1;
    draw_distances_trigger = 1;

}

int32_t list_item = 0, list_item_max = 0;
lv_obj_t *list = NULL;
lv_obj_t *list_items[20] = {0};

struct item_event_data {
    int32_t min;
    int32_t max;
    int32_t step;
    void    *value;
    int32_t value_size;
    void    *function;
};

// item generator functions
#define LIST_ITEM_LABEL(index, text)\
    it1 = list_items[index] = lv_label_create(list); \
    lv_obj_add_style(it1, &text_normal, LV_PART_MAIN); \
    lv_obj_add_style(it1, &item_style, LV_PART_MAIN); \
    lv_obj_set_width(it1, DISPLAY_WIDTH); \
    lv_label_set_text_static(it1, text);

#define LIST_ITEM_CHECKBOX(index, text, valvalue, valsize)\
    it1 = list_items[index] = lv_checkbox_create(list); \
    lv_obj_add_style(it1, &text_normal, LV_PART_MAIN); \
    lv_obj_add_style(it1, &item_style, LV_PART_MAIN); \
    lv_obj_set_width(it1, DISPLAY_WIDTH); \
    if (valvalue) lv_obj_add_state(it1, LV_STATE_CHECKED);\
    lv_obj_add_style(it1, &item_style_checkbox, LV_PART_INDICATOR);\
    lv_obj_add_style(it1, &item_style_checkbox_checked, LV_PART_INDICATOR | LV_STATE_CHECKED);\
    static struct item_event_data eventdata_ ## index = { .min = 0, .max = 1, .step = 1, .value = &valvalue, .value_size = valsize, .function = NULL };\
    lv_obj_add_event_cb(it1, item_event_cb, LV_EVENT_GESTURE, &eventdata_ ## index);\
    lv_checkbox_set_text_static(it1, text);

#define LIST_ITEM_COUNTER(index, text, valmin, valmax, valstep, valvalue, valsize, valfunction)\
    it1 = list_items[index] = lv_obj_create(list); \
    lv_obj_set_width(it1, DISPLAY_WIDTH); \
    lv_obj_add_style(it1, &list_style, LV_PART_MAIN); \
    it2 = lv_label_create(it1); \
    lv_obj_add_style(it2, &text_normal, LV_PART_MAIN); \
    lv_obj_add_style(it2, &item_style, LV_PART_MAIN); \
    lv_obj_set_width(it2, DISPLAY_WIDTH - 96); \
    lv_label_set_text_static(it2, text); \
    it3 = lv_label_create(it1); \
    lv_obj_add_style(it3, &text_normal, LV_PART_MAIN); \
    lv_obj_add_style(it3, &item_style, LV_PART_MAIN); \
    lv_obj_set_align(it3, LV_ALIGN_TOP_RIGHT); \
    lv_obj_set_style_text_align(it3, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN); \
    lv_obj_set_width(it3, 96); \
    lv_label_set_text_fmt(it3, "%d", valvalue); \
    lv_obj_set_height(it1, 36);\
    lv_obj_set_scrollbar_mode(it1, LV_SCROLLBAR_MODE_OFF);\
    static struct item_event_data eventdata_ ## index = { .min = valmin, .max = valmax, .step = valstep, .value = &valvalue, .value_size = valsize, .function = valfunction };\
    lv_obj_add_event_cb(it1, item_event_cb, LV_EVENT_GESTURE, &eventdata_ ## index);

#define LIST_ITEM_HIGHLIGHT(index) \
    lv_obj_add_style(list_items[index], &item_style_checked, LV_PART_MAIN);\
    lv_obj_add_state(list_items[index], LV_STATE_FOCUSED);\
    for(int i = 0; i < lv_obj_get_child_cnt(list_items[index]); i++) { lv_obj_t * child = lv_obj_get_child(list_items[index], i); lv_obj_add_style(child, &item_style_checked, LV_PART_MAIN); };\
    lv_obj_scroll_to_view(list_items[index], LV_ANIM_ON);

#define LIST_ITEM_NORMAL(index)    \
    lv_obj_add_style(list_items[index], &item_style, LV_PART_MAIN);\
    for(int i = 0; i < lv_obj_get_child_cnt(list_items[index]); i++) { lv_obj_t * child = lv_obj_get_child(list_items[index], i); lv_obj_add_style(child, &item_style, LV_PART_MAIN); };\
    lv_obj_clear_state(list_items[index], LV_STATE_FOCUSED);

#define LIST_ITEM_SELECT(index)    \
    lv_obj_add_style(list_items[index], &item_style_selected, LV_PART_MAIN);\
    for(int i = 0; i < lv_obj_get_child_cnt(list_items[index]); i++) { lv_obj_t * child = lv_obj_get_child(list_items[index], i); lv_obj_add_style(child, &item_style_selected, LV_PART_MAIN); };\
    lv_obj_add_state(list_items[index], LV_STATE_FOCUSED);

static void item_event_cb(lv_event_t *e) {
    // 
    struct item_event_data *data = lv_event_get_user_data(e);
    int value = *((int*) lv_event_get_param(e));

    int8_t *n = (int8_t*)data->value;
    int16_t *m = (int16_t*)data->value;
    int32_t *o = (int32_t*)data->value;

    int t = 0;
    switch(data->value_size) {
        case 1:
            t = (int) *n;
            break;
        case 2:
            t = (int) *m;
            break;
        case 4:
            t = (int) *o;
            break;
    }

    lv_obj_t *cb = lv_event_get_current_target(e);
    LV_LOG_INFO("A");
    if (lv_obj_check_type(cb, &lv_checkbox_class)) {
        LV_LOG_INFO("B");
        if (lv_obj_has_state(cb, LV_STATE_CHECKED)) {
            lv_obj_clear_state(cb, LV_STATE_CHECKED);
            t = 0;
        } else {
            lv_obj_add_state(cb, LV_STATE_CHECKED);
            t = 1;
        }
    } else {
        t = t + (value * data->step);
    
        if (t > data->max) t = data->max;
        if (t < data->min) t = data->min;

        lv_obj_t *i = lv_obj_get_child(lv_event_get_target(e), 1);
        lv_label_set_text_fmt(i, "%d", t);
        if (NULL != data->function) {
            void (*ptr)(int) = data->function;
            ptr(t);
        }
    }

    switch(data->value_size) {
        case 1:
            *n = (int8_t) t;
            break;
        case 2:
            *m = (int16_t) t;
            break;
        case 4:
            *o = (uint32_t) t;
            break;
    }
}

static lv_style_t item_style, item_style_checked, item_style_selected, list_style, list_scrollbar, item_style_checkbox, item_style_checkbox_checked;
void gui_draw_settings_main(void) {
    lv_theme_default_init(NULL, COLOR_BLACK, COLOR_WHITE, LV_THEME_DEFAULT_DARK, &lv_font_fry_32);
    lv_obj_set_style_bg_color(lv_screen_active(), COLOR_BLACK, LV_PART_MAIN);

    // text tyles
    lv_style_set_text_font(&text_normal, &lv_font_fry_32);
    lv_style_set_bg_color(&text_normal, COLOR_BLACK);
    lv_style_set_text_color(&text_normal, COLOR_WHITE);
    lv_style_set_pad_top(&text_normal, 2);
    lv_style_set_pad_left(&text_normal, 3);
    lv_style_set_text_align(&text_normal, LV_TEXT_ALIGN_LEFT);

    lv_style_set_border_color(&list_style, COLOR_BLACK);
    lv_style_set_bg_color(&list_style, COLOR_BLACK);
    lv_style_set_border_width(&list_style, 0);
    lv_style_set_pad_all(&list_style, 0);
    lv_style_set_bg_color(&list_style, COLOR_BLACK);
    lv_style_set_radius(&list_style, 0);
    lv_style_set_border_width(&list_style, 0);

    lv_style_set_bg_opa(&list_scrollbar, LV_OPA_TRANSP);

    lv_style_set_border_color(&item_style, COLOR_BLACK);
    lv_style_set_bg_color(&item_style, COLOR_BLACK);
    lv_style_set_text_color(&item_style, COLOR_WHITE);
    lv_style_set_pad_all(&item_style, 0);
    lv_style_set_border_width(&item_style, 0);

    lv_style_set_border_color(&item_style_checked, COLOR_BLACK);
    lv_style_set_bg_color(&item_style_checked, COLOR_YELLOW);
    lv_style_set_bg_opa(&item_style_checked, LV_OPA_COVER);
    lv_style_set_text_color(&item_style_checked, COLOR_BLACK);
    lv_style_set_border_width(&item_style_checked, 0);

    lv_style_set_border_color(&item_style_selected, COLOR_BLACK);
    lv_style_set_bg_color(&item_style_selected, COLOR_BLUE);
    lv_style_set_bg_opa(&item_style_selected, LV_OPA_COVER);
    lv_style_set_text_color(&item_style_selected, COLOR_WHITE);
    lv_style_set_border_width(&item_style_selected, 0);

    lv_style_init(&item_style_checkbox);
    lv_style_init(&item_style_checkbox_checked);
    lv_style_set_radius(&item_style_checkbox, LV_RADIUS_CIRCLE);
    lv_style_set_radius(&item_style_checkbox_checked, LV_RADIUS_CIRCLE);
    lv_style_set_bg_color(&item_style_checkbox_checked, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_img_src(&item_style_checkbox_checked, NULL);

    // list etc
    list = lv_obj_create(lv_screen_active());
    lv_obj_set_layout(list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(list, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_obj_set_pos(list, 0, 0);
    lv_obj_set_style_pad_column(list, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(list, 0, LV_PART_MAIN);
    lv_obj_add_style(list, &list_style, LV_PART_MAIN);
    lv_obj_add_style(list, &list_scrollbar, LV_PART_SCROLLBAR);
    lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);


    lv_obj_t *it1, *it2, *it3 = NULL;

    LIST_ITEM_LABEL(0, "DISPLAY");
    LIST_ITEM_LABEL(1, "CONTROLLER");
    LIST_ITEM_COUNTER(2, "BACKLIGHT", 0, 100, 1, settings.backlight_level, 1, lcd_backlight);
    LIST_ITEM_LABEL(3, "BACK");

    LIST_ITEM_HIGHLIGHT(0);    

    list_item = 0;
    list_item_max = 4;
}

void gui_draw_controller_settings(void) {
    list = lv_obj_create(lv_screen_active());
    lv_obj_set_layout(list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(list, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_obj_set_pos(list, 0, 0);
    lv_obj_set_style_pad_column(list, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(list, 0, LV_PART_MAIN);
    lv_obj_add_style(list, &list_style, LV_PART_MAIN);
    lv_obj_add_style(list, &list_scrollbar, LV_PART_SCROLLBAR);
    lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);


    lv_obj_t *it1, *it2, *it3 = NULL;

    LIST_ITEM_COUNTER(0, "SPEED ASSIST MAX.", 0, 100, 1, settings.speed_assist_max, 1, NULL);
    LIST_ITEM_COUNTER(1, "WHEEL CIRC.", 0, 300, 1, settings.wheel_circumfence, 2, NULL);
    LIST_ITEM_COUNTER(2, "CURRENT MAX.", 0, 100000, 100, settings.current_max, 2, NULL);
    LIST_ITEM_LABEL(3, "BACK");
    /* LIST_ITEM_LABEL(1, "Controller"); */
    /* LIST_ITEM_COUNTER(2, "Backlight", 0, 100, 1, settings.backlight_level, 1, lcd_backlight); */
    /* LIST_ITEM_LABEL(3, "Back"); */
    /*  */
    LIST_ITEM_HIGHLIGHT(0);    

    list_item = 0;
    list_item_max = 4;
}

void gui_draw_display_settings(void) {
    list = lv_obj_create(lv_screen_active());
    lv_obj_set_layout(list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(list, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_obj_set_pos(list, 0, 0);
    lv_obj_set_style_pad_column(list, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(list, 0, LV_PART_MAIN);
    lv_obj_add_style(list, &list_style, LV_PART_MAIN);
    lv_obj_add_style(list, &list_scrollbar, LV_PART_SCROLLBAR);
    lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);


    lv_obj_t *it1, *it2, *it3 = NULL;

    LIST_ITEM_COUNTER(0, "BATTERY VMIN", 0, 60000, 100, settings.battery_voltage_min, 2, NULL);
    LIST_ITEM_COUNTER(1, "BATTERY VMAX", 0, 60000, 100, settings.battery_voltage_max, 2, NULL);
    LIST_ITEM_COUNTER(2, "GRAPH DURATION", 0, 30, 1, settings.graph_duration, 1, NULL);
    LIST_ITEM_CHECKBOX(3, "GRAPH MODE SHIFT", settings.graph_shift, 1);
    LIST_ITEM_COUNTER(4, "GRAPH MAX", 0, 100, 1, settings.graph_max, 1, NULL);
    LIST_ITEM_COUNTER(5, "ASSIST LEVELS", 0, 9, 1, settings.assist_levels, 1, NULL);
    LIST_ITEM_COUNTER(6, "SPEED MAX", 0, 100, 1, settings.speed_max, 1, NULL);
    LIST_ITEM_COUNTER(7, "SPEED REDLINE", 0, 100, 1, settings.speed_redline, 1, NULL);
    LIST_ITEM_COUNTER(8, "POWER MIN", -1000, 0, 1, settings.power_min, 2, NULL);
    LIST_ITEM_COUNTER(9, "POWER MAX", 0, 1000, 1, settings.power_max, 2, NULL);
    LIST_ITEM_CHECKBOX(10, "VOLTAGE FROM CONT.", settings.battery_voltage_from_controller, 1);
    LIST_ITEM_LABEL(11, "BACK");
    
    /* LIST_ITEM_LABEL(1, "Controller"); */
    /* LIST_ITEM_COUNTER(2, "Backlight", 0, 100, 1, settings.backlight_level, 1, lcd_backlight); */
    /* LIST_ITEM_LABEL(3, "Back"); */
    /*  */
    LIST_ITEM_HIGHLIGHT(0);    

    list_item = 0;
    list_item_max = 12;
}


void gui_timers_start(void) {
}

void gui_timers_stop(void) {
}

void gui_update(void) {
    switch(mode) {
        case MODE_NORMAL:
        case MODE_TRIP_RESET:
            _draw_battery();
            _draw_power();
            _draw_temperature();
            _draw_speed();
            _draw_time();
            _draw_distances();
            _draw_assist();
            _draw_headlights();
            _draw_brake();
            _draw_controller_mode();
            break;
        default:
            break;
    }
}

/*
 * lvgl uses too much memory when drawing large areas, so we dump the images streight into
 * the framebuffer
 */
static void _draw_speed(void) {
    if (draw_speed_trigger) {
        int32_t tspeed = speed < 0 ? speed * -1 : speed;
        switch ((tspeed % 10000) / 1000) {
            default:
            case 0:
                lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_0, SPEED_COLOR_TRUE);
                break;
            case 1:
                lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_1, SPEED_COLOR_TRUE);
                break;
            case 2:
                lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_2, SPEED_COLOR_TRUE);
                break;
            case 3:
                lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_3, SPEED_COLOR_TRUE);
                break;
            case 4:
                lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_4, SPEED_COLOR_TRUE);
                break;
            case 5:
                lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_5, SPEED_COLOR_TRUE);
                break;
            case 6:
                lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_6, SPEED_COLOR_TRUE);
                break;
            case 7:
                lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_7, SPEED_COLOR_TRUE);
                break;
            case 8:
                lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_8, SPEED_COLOR_TRUE);
                break;
            case 9:
                lcd_draw_large_text(SPEED_MAJOR_X + SPEED_MAJOR_WIDTH, SPEED_MAJOR_Y, &large_9, SPEED_COLOR_TRUE);
                break;
        }
        switch (tspeed / 10000) {
            default:
            case 0:
                lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_0, SPEED_COLOR_FALSE);
                break;
            case 1:
                lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_1, SPEED_COLOR_TRUE);
                break;
            case 2:
                lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_2, SPEED_COLOR_TRUE);
                break;
            case 3:
                lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_3, SPEED_COLOR_TRUE);
                break;
            case 4:
                lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_4, SPEED_COLOR_TRUE);
                break;
            case 5:
                lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_5, SPEED_COLOR_TRUE);
                break;
            case 6:
                lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_6, SPEED_COLOR_TRUE);
                break;
            case 7:
                lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_7, SPEED_COLOR_TRUE);
                break;
            case 8:
                lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_8, SPEED_COLOR_TRUE);
                break;
            case 9:
                lcd_draw_large_text( SPEED_MAJOR_X, SPEED_MAJOR_Y, &large_9, SPEED_COLOR_TRUE);
                break;
        }
        lv_label_set_text_fmt(speed_minor, "%1ld", (tspeed % 1000) / 100);
        /* lv_meter_set_indicator_end_value(meter, indic1, speed_major); */
        draw_speed_trigger = 0;
    }
    oldspeed = speed;
}

void _draw_graph(lv_timer_t *timer) {
    if (mode == MODE_NORMAL) {
        if (graph_cursor) lv_chart_set_cursor_point(graph, graph_cursor, graph_series, graph_series->start_point);
        lv_chart_set_next_value(graph, graph_series, avg_speed);
    }
}

static void _draw_battery(void) {
    if (draw_battery_voltage_trigger) {
        lv_label_set_text_fmt(battery_label, "% 2ld.%1ldV", battery_voltage / 1000, (battery_voltage % 1000) / 100);
        battery_voltage_old = battery_voltage;
        battery_value = (((battery_voltage - settings.battery_voltage_min) * 100) / (settings.battery_voltage_max - settings.battery_voltage_min));
        if (battery_value != battery_value_old) {
            lv_bar_set_value(battery_bar, battery_value, LV_ANIM_OFF); 
            /* if (battery_value < 10) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_10, LV_PART_INDICATOR); */
            /* else if (battery_value < 20) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_20, LV_PART_INDICATOR); */
            /* else if (battery_value < 30) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_30, LV_PART_INDICATOR); */
            /* else if (battery_value < 40) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_40, LV_PART_INDICATOR); */
            /* else if (battery_value < 50) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_50, LV_PART_INDICATOR); */
            /* else if (battery_value < 60) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_60, LV_PART_INDICATOR); */
            /* else if (battery_value < 70) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_70, LV_PART_INDICATOR); */
            /* else if (battery_value < 80) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_80, LV_PART_INDICATOR); */
            /* else if (battery_value < 90) lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_90, LV_PART_INDICATOR); */
            /* else lv_obj_set_style_bg_color(battery_bar, BATTERY_COLOR_100 , LV_PART_INDICATOR); */
            battery_value_old = battery_value;
        }
    }
}

static void _draw_controller_mode(void) {
    if (draw_controller_mode_trigger) {
        char *md = "N";
        if (controller_mode == 1) md = "S";
        else if (controller_mode == 5) md = "I";
        else if (controller_mode == 6) md = "P";
        lv_label_set_text(cm_label, md);
    }
}

static void _draw_time(void) {
    if (draw_time_trigger) {
        print_time(trip_time_text, settings.trip_time);
        draw_time_trigger = 0;
    }
}

static void _draw_power(void) {
    if (draw_power_trigger) {
        power_value = battery_voltage * battery_current;
        power_value /= 1000000; // convert to W
        if (power_value == power_value_old) {
            // do nothing
        } else if (power_value >= 0 && power_value_old > 0) {
            lv_bar_set_value(powerbar_positive, power_value, LV_ANIM_ON);
            lv_bar_set_value(powerbar_negative, 0, LV_ANIM_OFF);
            lv_label_set_text_fmt(powerbar_text, "%ld", power_value);
        } else if (power_value >= 0 && power_value_old < 0) {
            lv_bar_set_value(powerbar_positive, power_value, LV_ANIM_ON);
            lv_bar_set_value(powerbar_negative, 0, LV_ANIM_OFF);
            lv_label_set_text_fmt(powerbar_text, "%ld", power_value);
        } else if (power_value < 0 && power_value_old >= 0) {
            lv_bar_set_value(powerbar_positive, 0, LV_ANIM_OFF);
            lv_bar_set_value(powerbar_negative, power_value * -1, LV_ANIM_ON);
            lv_label_set_text_fmt(powerbar_text, "%ld", power_value);
        } else if (power_value < 0 && power_value_old < 0) {
            lv_bar_set_value(powerbar_positive, 0, LV_ANIM_OFF);
            lv_bar_set_value(powerbar_negative, power_value * -1, LV_ANIM_ON);
            lv_label_set_text_fmt(powerbar_text, "%ld", power_value);
        }

        power_value_old = power_value;
        draw_power_trigger = 1;
    }    
}

static void _draw_temperature(void) {
    if (ext_temperature_old != ext_temperature) { // internal
        lv_label_set_text_fmt(temperature, "% 2ld.%1ld°C", ext_temperature >> 8, (ext_temperature & 0x000000FF) / 26);
    }
    if (draw_temperatures_trigger) {
        lv_label_set_text_fmt(motor_temperature, "% 3ld°C", mot_temperature / 10);
    }

    int_temperature_old = int_temperature;
    ext_temperature_old = ext_temperature;
}

static void _draw_assist(void) {
    if (draw_assist_trigger) {
        lv_roller_set_selected(assist_mode, settings.assist_last, LV_ANIM_ON);
        comm_send_display_status(); // inform controller
        draw_assist_trigger = 0;
    }
}

static void _draw_headlights(void) {
    if (draw_lights_trigger) {
        if (settings.lights_mode == LIGHTS_MODE_AUTOMATIC) {
            lv_image_set_src(lights_img, &icon_headlight_auto);
            lv_obj_clear_flag(lights_img, LV_OBJ_FLAG_HIDDEN);
        } else {
            if (settings.lights_enabled) {
                lv_image_set_src(lights_img, &icon_headlight);
                lv_obj_clear_flag(lights_img, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(lights_img, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

static void _draw_distances(void) {
    if (draw_distances_trigger) {
        if (settings.total_distance < 100000000) {
            print_digit_text(total_distance_text, (settings.total_distance), 3, 1, COLOR_WHITE, COLOR_GREY);
        } else {
            print_digit_text(total_distance_text, settings.total_distance, 4, 0, COLOR_WHITE, COLOR_GREY);
        }
        if (settings.trip_distance < 100000000) {
            print_digit_text(trip_distance_text, (settings.trip_distance), 3, 1, COLOR_WHITE, COLOR_GREY);
        } else {
            print_digit_text(trip_distance_text, settings.trip_distance, 4, 0, COLOR_WHITE, COLOR_GREY);
        }
    }
}

void print_digit_text(lv_obj_t *object, uint32_t value, uint32_t length, uint32_t decimals, lv_color_t active_color, lv_color_t passive_color) { // string must be large enough to contain all data, check beforehand

    const uint32_t powers[6] = { 1, 10, 100, 1000, 10000, 100000 };
    int preval = 0;
    int cnt = 0;
    int decimal_value = 0;
    char string[64] = {0};
    // decimals is in x.x, normal in 32bit
    lv_obj_set_style_text_color(object, active_color, LV_PART_MAIN);
    if (decimals) {
        decimal_value = (value % 100000) / 10000;
        value /= 100000;
    }
    if (value / powers[length-1] == 0) {
        cnt += sprintf(string + cnt, "#%02X%02X%02X ", passive_color.ch.red << 3, passive_color.ch.green << 2, passive_color.ch.blue << 3);
        preval = 1;
    }
    for (int i = length; i > 0; i--) {
        uint32_t tval = value / powers[i-1];
        value %= powers[i-1];
        if (tval != 0 && preval) {
            cnt += sprintf(&string[cnt], "#");
            preval = 0;
        }
        cnt += sprintf(&string[cnt], "%lu", tval);
    }
    if (decimals) cnt += sprintf(&string[cnt], ".");
    for (int i = decimals; i > 0; i--) {
        uint32_t tval = decimal_value / powers[i-1];
        decimal_value %= powers[i-1];
        if (preval) {
            cnt += sprintf(&string[cnt], "#");
            preval = 0;
        }
        cnt += sprintf(&string[cnt], "%lu", tval);
    }
    lv_label_set_text_fmt(object, string); // fmt will copy
}

void print_time(lv_obj_t *object, uint32_t value) {
    char string[64];
    uint32_t hours = (value / 3600000);
    uint32_t minutes = (value - (hours * 3600000)) /  60000;
    uint32_t seconds = (value % 60000) / 1000;

    if (hours != 0) {
        sprintf(string, "%02ld:%02ld", hours, minutes);
    } else {
        sprintf(string, "%02ld.%02ld", minutes, seconds);
    }
    lv_label_set_text_fmt(object, string); 
}

static void _draw_brake(void) {
    if (draw_brake_trigger) {
        if (brake) lv_obj_clear_flag(brake_img, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(brake_img, LV_OBJ_FLAG_HIDDEN);
    }
}

/**********************************************
 * Timers
 **********************************************/
static void _100ms_timer(lv_timer_t *timer) {
    ext_temperature += (ext_temp() - ext_temperature) >> TEMPERATURE_FILTER_SHIFT;
    int_temperature += (int_temp() - int_temperature) >> TEMPERATURE_FILTER_SHIFT;

    avg_speed += (speed - avg_speed) >> SPEED_FILTER_SHIFT;
    avg_power += (power_value - avg_power) >> POWER_FILTER_SHIFT;

    if (settings.battery_voltage_from_controller) {
        battery_voltage += (battery_voltage_controller - battery_voltage) >> TEMPERATURE_FILTER_SHIFT;
    } else {
        battery_voltage += (voltage_ebat() - battery_voltage) >> TEMPERATURE_FILTER_SHIFT;
    }

    settings.trip_time += timer_counter - timer_old;

    timer_old = timer_counter;
}

static void _500ms_timer(lv_timer_t *timer) {
    // update bottom data
    draw_time_trigger = 1;
    draw_distances_trigger = 1;
    draw_battery_voltage_trigger = 1;
}


/***************************************************
 * Callbacks
 ***************************************************/
static void graph_event_pre_cb(lv_event_t * e)
{
    lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);

    if(dsc->part == LV_PART_MAIN) {
        if(dsc->line_dsc == NULL || dsc->p1 == NULL || dsc->p2 == NULL) return;

        if(dsc->p1->y == dsc->p2->y) {
            dsc->line_dsc->color  = GRAPH_DIV_COLOR;
        }
    } if (dsc->type == LV_CHART_DRAW_PART_TICK_LABEL) {
        sprintf(dsc->text, "%2ld", dsc->value / 1000); // always smaller than actual text
    }
}

static void trip_reset_anim(void *none, long int value) {
    if (value >= 100) {
        lv_obj_set_style_text_color(trip_distance_text, COLOR_GREY, 0);
        lv_obj_set_style_text_color(trip_time_text, COLOR_GREY, 0);
    } else {
        lv_obj_set_style_text_color(trip_distance_text, COLOR_WHITE, 0);
        lv_obj_set_style_text_color(trip_time_text, COLOR_WHITE, 0);
    }
}

static int event_item = 0;
/*
 *
 *  Up/Down -> changes assistance
 *  Power -> cycles something
 *
 *  Long press up -> Lights
 *  Long press power -> shutdown
 *  Long press down -> Reset
 *
 *  Long press up or down + other button detected -> Settings
 *
 *  Repeated does nothing
 */
void button_presses(void) {
    uint8_t code = buttons_pressed(); 
    switch (mode) {
        case MODE_NORMAL:
            // check first if two buttons pressed
            switch (code & (BUTTON_MASK(BUTTON_ID_UP) | BUTTON_MASK(BUTTON_ID_DOWN))) {
                case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_UP, BUTTON_DOWN):
                case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_UP, BUTTON_PRESSED):
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_DOWN):
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_PRESSED):
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED):
                    // one long press and other down
                    button_release(BUTTON_ID_UP, 500);
                    button_release(BUTTON_ID_DOWN, 500);
                    mode = MODE_SETTINGS_MAIN;
                    // clean all
                    lv_obj_clean(lv_screen_active());
                    gui_draw_settings_main();
                    break;
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_PRESSED):
                    if (settings.assist_last < settings.assist_levels) settings.assist_last++;
                    button_release(BUTTON_ID_UP, 0);
                    comm_send_display_status();
                    draw_assist_trigger = 1;
                    break;
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED):
                    settings.lights_mode = LIGHTS_MODE_AUTOMATIC;
                    comm_send_display_status();
                    button_release(BUTTON_ID_UP, 150);
                    draw_lights_trigger = 1;
                    break;
                case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_PRESSED):
                    if (settings.assist_last > 0) settings.assist_last--;
                    comm_send_display_status();
                    draw_assist_trigger = 1;
                    button_release(BUTTON_ID_DOWN, 0);
                    break;
                case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED):
                    settings.lights_enabled = ~settings.lights_enabled;
                    settings.lights_mode = LIGHTS_MODE_MANUAL;
                    comm_send_display_status();
                    button_release(BUTTON_ID_DOWN, 150);
                    draw_lights_trigger = 1;
                    break;
                /* default: */
                /*     button_release(BUTTON_ID_UP, 0); */
                /*     button_release(BUTTON_ID_DOWN, 0); */
                /*     button_release(BUTTON_ID_POWER, 0); */
                /*     break; */
            }

            switch (code & BUTTON_MASK(BUTTON_ID_POWER)) {
                case BUTTON_STATE(BUTTON_ID_POWER, BUTTON_PRESSED):
                    button_release(BUTTON_ID_POWER, 250);
                    mode = MODE_TRIP_RESET;
                    lv_anim_init(&anim_reset_trip);
                    lv_anim_set_values(&anim_reset_trip, 0, 200);
                    lv_anim_set_repeat_count(&anim_reset_trip, LV_ANIM_REPEAT_INFINITE);
                    lv_anim_set_exec_cb(&anim_reset_trip, trip_reset_anim);
                    lv_anim_set_time(&anim_reset_trip, 150);
                    lv_anim_set_playback_time(&anim_reset_trip, 150);
                    lv_anim_start(&anim_reset_trip);
                    break;
                case BUTTON_STATE(BUTTON_ID_POWER, BUTTON_LONG_PRESSED):
                    // shutdown!
                    if (timer_counter > 2150) {
                        // avoid shutdown/start loops
                        eeprom_write_settings();
                        lcd_backlight(0);
                        delay_ms(250);
                        power_disable();
                        delay_ms(1500);
                    }
                    break;
            }
            break;
        case MODE_TRIP_RESET:
            switch (code & BUTTON_MASK(BUTTON_ID_POWER)) {
                case BUTTON_STATE(BUTTON_ID_POWER, BUTTON_PRESSED):
                    button_release(BUTTON_ID_POWER, 200);
                    settings.trip_distance = 0;
                    settings.trip_time = 0;
                    eeprom_write_settings();
                    mode = MODE_NORMAL;
                    break;
            }
            switch (code & (BUTTON_MASK(BUTTON_ID_UP) | BUTTON_MASK(BUTTON_ID_DOWN))) {
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_PRESSED):
                case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_PRESSED):
                case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_PRESSED) | BUTTON_STATE(BUTTON_ID_UP, BUTTON_PRESSED):
                    mode = MODE_NORMAL;
                    button_release(BUTTON_ID_DOWN, 150);
                    button_release(BUTTON_ID_UP, 150);
                    break;
            }
            if (mode == MODE_NORMAL) {
                lv_anim_del(NULL, trip_reset_anim);
                lv_obj_set_style_text_color(trip_distance_text, COLOR_WHITE, 0);
                lv_obj_set_style_text_color(trip_time_text, COLOR_WHITE, 0);
                draw_distances_trigger = 1;
            }
            break;
        case MODE_SETTINGS_EVENT_CALLBACK:
            switch (code & (BUTTON_MASK(BUTTON_ID_UP) | BUTTON_MASK(BUTTON_ID_DOWN) | BUTTON_MASK(BUTTON_ID_POWER))) {
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_PRESSED):
                    button_release(BUTTON_ID_UP, 50);
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED):
                    event_item = 1;
                    lv_event_send(list_items[list_item], LV_EVENT_GESTURE, &event_item);
                    delay_ms(5);
                    break;
                case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_PRESSED):
                    button_release(BUTTON_ID_DOWN, 50);
                case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED):
                    event_item = -1;
                    lv_event_send(list_items[list_item], LV_EVENT_GESTURE, &event_item);
                    delay_ms(5);
                    break;
                case BUTTON_STATE(BUTTON_ID_POWER, BUTTON_PRESSED):
                    button_release(BUTTON_ID_POWER, 150);
                    mode = mode_back;
                    LIST_ITEM_HIGHLIGHT(list_item);
                    break;
            }
            break;
        case MODE_SETTINGS_MAIN:
            switch (code & (BUTTON_MASK(BUTTON_ID_UP) | BUTTON_MASK(BUTTON_ID_DOWN) | BUTTON_MASK(BUTTON_ID_POWER))) {
                case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_UP, BUTTON_DOWN):
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_DOWN):
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED):
                    break; // ignore
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_PRESSED):
                    LIST_ITEM_NORMAL(list_item);
                    list_item--;
                    if (list_item < 0) list_item = list_item_max - 1;
                    LIST_ITEM_HIGHLIGHT(list_item);
                    button_release(BUTTON_ID_UP, 0);
                    break;
                case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_PRESSED):
                    LIST_ITEM_NORMAL(list_item);
                    list_item++;
                    list_item = ((uint32_t) list_item) % list_item_max;
                    LIST_ITEM_HIGHLIGHT(list_item);
                    button_release(BUTTON_ID_DOWN, 0);
                    break;
                case BUTTON_STATE(BUTTON_ID_POWER, BUTTON_PRESSED):
                    button_release(BUTTON_ID_POWER, 150);
                    switch(list_item) {
                        case 3: // back
                            mode = MODE_NORMAL;
                            // clean all
                            lv_obj_clean(lv_screen_active());
                            gui_draw_normal();
                            // save settings
                            eeprom_write_settings();
                            // send settings to controller
                            comm_send_display_settings();
                            comm_send_display_status();
                            comm_send_controller_settings();
                            break;
                        case 0: // goto display settings
                            mode = MODE_SETTINGS_DISPLAY;
                            lv_obj_clean(lv_screen_active());
                            gui_draw_display_settings();
                            break;
                        case 1: // goto controller settings
                            mode = MODE_SETTINGS_CONTROLLER;
                            lv_obj_clean(lv_screen_active());
                            gui_draw_controller_settings();
                            break;
                        case 2: // items
                            mode_back = MODE_SETTINGS_MAIN;
                            mode = MODE_SETTINGS_EVENT_CALLBACK;
                            LIST_ITEM_SELECT(list_item); 
                            break;
                    }
                    break;
            }
            break;
        case MODE_SETTINGS_DISPLAY:
            switch (code & (BUTTON_MASK(BUTTON_ID_UP) | BUTTON_MASK(BUTTON_ID_DOWN) | BUTTON_MASK(BUTTON_ID_POWER))) {
                case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_UP, BUTTON_DOWN):
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_DOWN):
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED):
                    break; // ignore
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_PRESSED):
                    button_release(BUTTON_ID_UP, 50);
                    LIST_ITEM_NORMAL(list_item);
                    list_item--;
                    if (list_item < 0) list_item = list_item_max - 1;
                    LIST_ITEM_HIGHLIGHT(list_item);
                    break;
                case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_PRESSED):
                    button_release(BUTTON_ID_DOWN, 50);
                    LIST_ITEM_NORMAL(list_item);
                    list_item++;
                    list_item = ((uint32_t) list_item) % list_item_max;
                    LIST_ITEM_HIGHLIGHT(list_item);
                    break;
                case BUTTON_STATE(BUTTON_ID_POWER, BUTTON_PRESSED):
                    button_release(BUTTON_ID_POWER, 150);
                    switch(list_item) {
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                        case 9:
                        case 10:
                            mode_back = MODE_SETTINGS_DISPLAY;
                            mode = MODE_SETTINGS_EVENT_CALLBACK;
                            LIST_ITEM_SELECT(list_item); 
                            break;
                        case 11: // back
                            mode = MODE_SETTINGS_MAIN;
                            // clean all
                            lv_obj_clean(lv_screen_active());
                            gui_draw_settings_main();
                            break;
                    }
                    break;
            }
            break;
        case MODE_SETTINGS_CONTROLLER:
            switch (code & (BUTTON_MASK(BUTTON_ID_UP) | BUTTON_MASK(BUTTON_ID_DOWN) | BUTTON_MASK(BUTTON_ID_POWER))) {
                case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_UP, BUTTON_DOWN):
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_DOWN):
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED):
                    break; // ignore
                case BUTTON_STATE(BUTTON_ID_UP, BUTTON_PRESSED):
                    LIST_ITEM_NORMAL(list_item);
                    list_item--;
                    if (list_item < 0) list_item = list_item_max - 1;
                    LIST_ITEM_HIGHLIGHT(list_item);
                    button_release(BUTTON_ID_UP, 0);
                    break;
                case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_PRESSED):
                    LIST_ITEM_NORMAL(list_item);
                    list_item++;
                    list_item = ((uint32_t) list_item) % list_item_max;
                    LIST_ITEM_HIGHLIGHT(list_item);
                    button_release(BUTTON_ID_DOWN, 0);
                    break;
                case BUTTON_STATE(BUTTON_ID_POWER, BUTTON_PRESSED):
                    button_release(BUTTON_ID_POWER, 150);
                    switch(list_item) {
                        case 3: // back
                            mode = MODE_SETTINGS_MAIN;
                            // clean all
                            lv_obj_clean(lv_screen_active());
                            gui_draw_settings_main();
                            break;
                        case 0:
                        case 1:
                        case 2: // items
                            mode_back = MODE_SETTINGS_CONTROLLER;
                            mode = MODE_SETTINGS_EVENT_CALLBACK;
                            LIST_ITEM_SELECT(list_item); 
                            break;
                    }
                    break;
            }
            break;
    }
}

void gui_increment_trip(void) {
        settings.trip_distance += settings.wheel_circumfence;
        settings.total_distance += settings.wheel_circumfence;
}

