#include "gui.h" 
#include "img.h"
#include "lvgl.h"
#include "controls.h"
#include "eeprom.h"
#include "clock.h"
#include "uart.h"
#include "comm.h"
#include "config.h"
#ifdef CAN_ENABLED
#include "can.h"
#endif


#pragma GCC optimize ("Os")

#if defined($PLATFORM_SIM)
#include <cmsis/core/core_cm4.h>
#endif

extern volatile uint32_t HAL_GetTick();

uint32_t timer_old = 0;

/*** 
 * data variables
 */

// avoid long int issues
#ifdef __LP64__ 
#define LID "d"
#define LUI "u"
#else
#define LID "ld"
#define LUI "lu"
#endif

int32_t wh_distance_i32 = 0;
int32_t power_value = 0, power_value_old = 0, avg_power_old = 0;
int32_t speed = 0, oldspeed = 0, erpm = 0;
int32_t battery_value = 0, battery_value_old = 0;
int32_t battery_voltage = 0, battery_voltage_old = 1;
int32_t battery_voltage_controller = 0;
int32_t battery_current = 0;
int32_t motor_current = 0;
uint8_t vesc_status = 0, controller_id = 0;
int16_t duty_cycle = 0;
int32_t tachometer = 0;
int32_t tachometer_abs = 0;
int32_t int_temperature = 0;
int16_t fet_temp_1 = 0, fet_temp_2 = 0, fet_temp_3 = 0;
int32_t ext_temperature = 0;
int32_t wheel_circumfence = 2230; //(in mm)
int32_t mot_temperature = 0;
int32_t con_temperature = 0;
int32_t amphours_total = 0, amphours_regen_total = 0;
float wh_total = 0, wh_regen_total = 0, wh_left = 0, wh_km = 0;
float distance_total = 0;
int32_t avg_speed = 0, avg_power = 0;
uint8_t brake = 0;
uint8_t controller_mode = 0;
int32_t graph_values[GRAPH_POINT_COUNT];
volatile modes mode = MODE_NORMAL, mode_back = MODE_NORMAL;
volatile status_pages status_page = STATUS_SYSTEM;
volatile status_pages status_page_active = STATUS_PAGE_NONE;
uint32_t notification_timeout = 0;;

extern settings_t settings; 
extern error_state error;
extern uint8_t draw_time_trigger;
extern uint8_t draw_distances_trigger;
extern uint8_t draw_power_trigger;
extern uint8_t draw_temperatures_trigger;
extern uint8_t draw_temperatures_trigger_internal;
extern uint8_t draw_speed_trigger;
extern uint8_t draw_assist_trigger;
extern uint8_t draw_battery_voltage_trigger;
extern uint8_t draw_lights_trigger;
extern uint8_t draw_brake_trigger;
extern uint8_t draw_controller_mode_trigger;


const char *speed_text[] = {
    "0", "5", "10", "15", "20", "25", "30", "35", "40", "45", "50", "55", "60", "65", "70", "75", "80", "85", "90", "95", "100"
};
const char *power_text[] = {
    "-1000", "-950", "-900", "-850", "-800", "-750", "-700", "-650", "-600", "-550", "-500", "-450", "-400", "-350", "-300", "-250", "-200", "-150", "-100", "-50", "0", "50", "100", "150", "200", "250", "300", "350", "400", "450", "500", "550", "600", "650", "700", "750", "800", "850", "900", "950", "1000"
};
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

lv_obj_t *bsod = NULL;

lv_obj_t *powerbar_positive = NULL;
lv_obj_t *powerbar_negative = NULL;
lv_obj_t *powerbar_center_line = NULL;
lv_obj_t *powerbar_high_line = NULL;
lv_obj_t *powerbar_shade = NULL;
lv_obj_t *powerbar_text = NULL;

lv_obj_t *power_bar_positive = NULL;
lv_obj_t *power_bar_negative = NULL;
uint32_t power_bar_zero = 0;
lv_obj_t *speed_bar = NULL;

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
lv_obj_t *distance_img = NULL;
lv_obj_t *distance_text = NULL;
lv_obj_t *avg_speed_text = NULL;
lv_obj_t *max_speed_text = NULL;
lv_obj_t *lights_img = NULL;

lv_obj_t *time_text = NULL;
lv_obj_t *brake_img = NULL;

lv_obj_t *graph = NULL; 
lv_obj_t *graph_scale = NULL; 
lv_chart_series_t *graph_series = NULL;
lv_chart_cursor_t *graph_cursor = NULL;
lv_obj_t *light_sensitivity_label = NULL;
lv_obj_t *cm_label = NULL;

lv_obj_t *speed_label = NULL;
lv_obj_t *speed_label_minor = NULL;
lv_obj_t *nav_distance = NULL;
lv_obj_t *nav_time = NULL;
lv_obj_t *nav_img = NULL;
lv_obj_t *notification = NULL;

lv_anim_t anim_reset_trip;

lv_style_t text_slim, text_normal, text_large, text_medium, text_fixed, text_rbt;

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
extern const lv_image_dsc_t icon_headlight_enabled;
extern const lv_image_dsc_t icon_headlight_auto;
extern const lv_image_dsc_t icon_headlight_auto_enabled;
extern const lv_image_dsc_t icon_energy;

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
static void _draw_bsod(void);
static void _draw_graph_switch(void);
#if (DEBUG && (UART_COMM == UART_COMM_EBICS))
static void _draw_controller_mode(void);
#endif
static void _draw_notification(void);

void _draw_graph(lv_timer_t *timer);
void gui_draw_normal(void);
void gui_draw_status(void);
void gui_draw_settings_main(void);
void gui_draw_settings_display(void);
void gui_draw_settings_controller(void);

void print_time(lv_obj_t *object, uint32_t value);
void print_digit_text(lv_obj_t *object, uint32_t value, uint32_t length, uint32_t decimals, lv_color_t active_color, lv_color_t passive_color);

static void _100ms_timer(lv_timer_t *timer);
static void _50ms_timer(lv_timer_t *timer);
static void _500ms_timer(lv_timer_t *timer);
static void _light_level_timer(lv_timer_t *timer);
static void _status_page_timer(lv_timer_t *timer);
static void _heartbeat_timer(lv_timer_t *timer);
// buttons callback
void _button_presses(void);

// timers
lv_timer_t *draw_graph_timer = NULL;
lv_timer_t *ms100_timer = NULL;
lv_timer_t *ms50_timer = NULL;
lv_timer_t *ms500_timer = NULL;
lv_timer_t *status_page_timer = NULL;
lv_timer_t *light_level_timer = NULL;
lv_timer_t *heartbeat_timer = NULL;

// lvgl local
#if LVGL_VERSION_MAJOR == 8
static void graph_event_pre_cb(lv_event_t * e);
#endif

void gui_init(void) {
    LV_LOG_INFO("GUI Initialization");
    // input
    mode = MODE_NORMAL;
    gui_draw_normal();
    /* mode = MODE_SETTINGS_MAIN; */
    /* gui_draw_settings_main(); */
    /* mode = MODE_SETTINGS_DISPLAY; */
    /* gui_draw_settings_display(); */

    // make light
    lcd_backlight(settings.backlight_level);
}

uint8_t p = 0;
uint32_t inttimer = 0;


uint32_t redraw_speed, 
         redraw_temp_int, 
         redraw_temp_ext = 0;

void gui_draw_normal(void) {
    reset_all_buttons();
    if (NULL != light_level_timer) {
        lv_timer_delete(light_level_timer);
        light_level_timer = NULL;
    }
    /* lv_theme_default_init(NULL, COLOR_BLACK, COLOR_WHITE, LV_THEME_DEFAULT_DARK, &lv_font_plex_32); */

    lv_obj_set_style_bg_color(lv_screen_active(), COLOR_BLACK, LV_PART_MAIN);
    lv_obj_set_layout(lv_screen_active(), 0);

    lv_obj_set_scrollbar_mode(lv_screen_active(), LV_SCROLLBAR_MODE_OFF);
    // text tyles
    lv_style_set_text_font(&text_normal, &lv_font_plex_32);
    lv_style_set_bg_color(&text_normal, COLOR_BLACK);
    lv_style_set_text_color(&text_normal, COLOR_WHITE);
    lv_style_set_pad_top(&text_normal, 3);
    lv_style_set_text_align(&text_normal, LV_TEXT_ALIGN_LEFT);

    lv_style_set_text_font(&text_rbt, &lv_font_roboto_32);
    lv_style_set_bg_color(&text_rbt, COLOR_BLACK);
    lv_style_set_text_color(&text_rbt, COLOR_WHITE);
    lv_style_set_pad_top(&text_rbt, 2);
    lv_style_set_text_align(&text_rbt, LV_TEXT_ALIGN_LEFT);

    lv_style_set_text_font(&text_slim, &lv_font_fry_32);
    lv_style_set_bg_color(&text_slim, COLOR_BLACK);
    lv_style_set_text_color(&text_slim, COLOR_WHITE);

    lv_style_set_text_font(&text_medium, &lv_font_fry_64);
    lv_style_set_bg_color(&text_medium, COLOR_BLACK);
    lv_style_set_text_color(&text_medium, COLOR_WHITE);
    lv_style_set_pad_top(&text_medium, 3);
    lv_style_set_text_align(&text_medium, LV_TEXT_ALIGN_LEFT);

    lv_style_set_text_font(&text_large, &lv_font_fry_128);
    lv_style_set_bg_color(&text_large, COLOR_BLACK);
    lv_style_set_text_color(&text_large, COLOR_WHITE);
    lv_style_set_pad_top(&text_large, 3);
    lv_style_set_text_align(&text_large, LV_TEXT_ALIGN_LEFT);

    // bars
#if LVGL_VERSION_MAJOR == 8
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
    lv_bar_set_range(powerbar_positive, 1, settings.power_max); // using 0 as starting point causes devision by zero exceptions on sim
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
    lv_bar_set_range(powerbar_negative, 1, settings.power_min * -1); // using 0 as starting point causes devision by zero exceptions on sim
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
    lv_style_set_text_font(&powerbar_text_style, &lv_font_plex_32);
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
#else
    // use scales
    lv_obj_t *speed_scale = lv_scale_create(lv_screen_active());
    lv_obj_set_pos(speed_scale, SPEED_SCALE_X, SPEED_SCALE_Y);
    lv_obj_set_size(speed_scale, SPEED_SCALE_WIDTH, SPEED_SCALE_HEIGHT);
    lv_scale_set_label_show(speed_scale, TRUE);
    lv_scale_set_mode(speed_scale, LV_SCALE_MODE_ROUND_OUTER);
    lv_scale_set_total_tick_count(speed_scale, (settings.speed_max / 5) + 1);
    lv_scale_set_major_tick_every(speed_scale, 1);
    lv_obj_set_style_length(speed_scale, 0, LV_PART_ITEMS);
    lv_obj_set_style_length(speed_scale, 0, LV_PART_INDICATOR);
    lv_scale_set_range(speed_scale, 0, settings.speed_max);
    lv_scale_set_rotation(speed_scale, 160);
    lv_scale_set_angle_range(speed_scale, 220);
    /* lv_obj_center(speed_scale); */

    static char *custom_labels_speed[20] = {0};

    int i = 0;
    for (; i < ((settings.speed_max) / 5) + 1; i++) {
        custom_labels_speed[i] = (char *) speed_text[i];
    }
    lv_scale_set_text_src(speed_scale, (const char **) custom_labels_speed);

    lv_obj_set_style_radius(speed_scale, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(speed_scale, SPEED_COLOR_REDLINE, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(speed_scale, LV_OPA_TRANSP, LV_PART_MAIN);

    static lv_style_t speed_scale_main_style;
    lv_style_init(&speed_scale_main_style);
    lv_style_set_arc_width(&speed_scale_main_style, 0);
    lv_obj_add_style(speed_scale, &speed_scale_main_style, LV_PART_MAIN);

    static lv_style_t speed_indicator_style;
    lv_style_init(&speed_indicator_style);

    lv_style_set_text_font(&speed_indicator_style, &lv_font_montserrat_12);
    lv_style_set_text_color(&speed_indicator_style, SPEED_COLOR_TEXT);
    lv_style_set_border_width(&speed_indicator_style, 1U);
    lv_style_set_border_color(&speed_indicator_style, RGB565(255,255,255));
    /* lv_style_set_line_width(&speed_indicator_style, 2u); */
    /* lv_style_set_width(&speed_indicator_style, 2U); */
    /* lv_style_set_line_color(&speed_indicator_style, RGB565(0, 128, 0)); */
    lv_obj_add_style(speed_scale, &speed_indicator_style, LV_PART_INDICATOR);

    /* lv_obj_set_style_length(speed_scale, 1, LV_PART_INDICATOR); */
    /* lv_obj_set_style_radial_offset(speed_scale, -10, LV_PART_INDICATOR); */
    /* lv_obj_set_style_line_color(speed_scale, RGB565(0, 196, 0), LV_PART_INDICATOR); */

    static lv_style_t style_speed_first_section;
    lv_style_init(&style_speed_first_section);
    lv_style_set_arc_color(&style_speed_first_section, SPEED_COLOR_BACKGROUND);
    lv_style_set_arc_width(&style_speed_first_section, 20);
    /* lv_style_set_line_width(&style_speed_first_section, 20U); */
    /* lv_style_set_width(&style_speed_first_section, 20U); */

    lv_scale_section_t * section_speed_first = lv_scale_add_section(speed_scale);
    lv_scale_section_set_range(section_speed_first, 0, settings.speed_redline);
    lv_scale_section_set_style(section_speed_first, LV_PART_MAIN, &style_speed_first_section);
    lv_scale_section_set_style(section_speed_first, LV_PART_INDICATOR, &style_speed_first_section);
    lv_scale_section_set_style(section_speed_first, LV_PART_ITEMS, &style_speed_first_section);

    static lv_style_t style_speed_second_section;
    lv_style_init(&style_speed_second_section);
    lv_style_set_arc_color(&style_speed_second_section, SPEED_COLOR_REDLINE);
    lv_style_set_arc_width(&style_speed_second_section, 20);

    lv_scale_section_t * section_speed_second = lv_scale_add_section(speed_scale);
    lv_scale_section_set_range(section_speed_second, settings.speed_redline, settings.speed_max);
    lv_scale_section_set_style(section_speed_second, LV_PART_MAIN, &style_speed_second_section);
    lv_scale_section_set_style(section_speed_second, LV_PART_INDICATOR, &style_speed_second_section);
    lv_scale_section_set_style(section_speed_second, LV_PART_ITEMS, &style_speed_second_section);



    lv_obj_t *power_scale = lv_scale_create(speed_scale);
    lv_obj_set_size(power_scale, SPEED_SCALE_WIDTH - 48, SPEED_SCALE_HEIGHT - 16);
    lv_obj_set_pos(power_scale, 24, 24);
    lv_scale_set_label_show(power_scale, TRUE);
    lv_scale_set_mode(power_scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_total_tick_count(power_scale, (((settings.power_min * -1) + settings.power_max) / 25) + 1);
    lv_scale_set_major_tick_every(power_scale, 4);
    /* lv_obj_align(power_scale, LV_ALIGN_TOP_MID, 0, 16); */
    /* lv_obj_center(power_scale); */

    lv_obj_set_style_length(power_scale, 5, LV_PART_ITEMS);
    lv_obj_set_style_length(power_scale, 18, LV_PART_INDICATOR);
    lv_scale_set_range(power_scale, settings.power_min, settings.power_max);
    lv_scale_set_rotation(power_scale, 160);
    lv_scale_set_angle_range(power_scale, 220);

    static char *custom_labels_power[20] = {0};
    i = 0;
    int start = 20 + (settings.power_min / 50);
    for (; i < (((settings.power_min * -1) + settings.power_max) / 100) + 1; i++) {
        custom_labels_power[i] = (char *) power_text[start + (i*2)];
    }
    lv_scale_set_text_src(power_scale, (const char **) custom_labels_power);

    lv_obj_set_style_radius(power_scale, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(power_scale, POWER_COLOR_REDLINE, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(power_scale, LV_OPA_TRANSP, LV_PART_MAIN);

    static lv_style_t power_scale_main_style;
    lv_style_init(&power_scale_main_style);
    lv_style_set_arc_width(&power_scale_main_style, 0);
    lv_style_set_line_width(&power_scale_main_style, 2U);
    lv_obj_add_style(power_scale, &power_scale_main_style, LV_PART_MAIN);

    static lv_style_t indicator_style;
    lv_style_init(&indicator_style);

    lv_style_set_text_font(&indicator_style, &lv_font_montserrat_12);
    lv_style_set_text_color(&indicator_style, POWER_COLOR_TEXT);
    lv_style_set_border_width(&indicator_style, 1U);
    lv_style_set_border_color(&indicator_style, POWER_COLOR_BORDER);
    lv_obj_add_style(power_scale, &indicator_style, LV_PART_INDICATOR);

    static lv_style_t style_first_section;
    lv_style_init(&style_first_section);
    lv_style_set_arc_color(&style_first_section, POWER_COLOR_BACKGROUND);
    lv_style_set_arc_width(&style_first_section, 20);

    lv_scale_section_t * section_first = lv_scale_add_section(power_scale);
    lv_scale_section_set_range(section_first, settings.power_min, settings.power_redline);
    lv_scale_section_set_style(section_first, LV_PART_MAIN, &style_first_section);
    lv_scale_section_set_style(section_first, LV_PART_INDICATOR, &style_first_section);
    lv_scale_section_set_style(section_first, LV_PART_ITEMS, &style_first_section);

    static lv_style_t style_second_section;
    lv_style_init(&style_second_section);
    lv_style_set_arc_color(&style_second_section, POWER_COLOR_REDLINE);
    lv_style_set_bg_opa(&style_second_section, LV_OPA_COVER);
    lv_style_set_arc_width(&style_second_section, 20);

    lv_scale_section_t * section_second = lv_scale_add_section(power_scale);
    lv_scale_section_set_range(section_second, settings.power_redline, settings.power_max);
    lv_scale_section_set_style(section_second, LV_PART_MAIN, &style_second_section);
    lv_scale_section_set_style(section_second, LV_PART_INDICATOR, &style_second_section);
    lv_scale_section_set_style(section_second, LV_PART_ITEMS, &style_second_section);


    // power bar

    power_bar_zero = (settings.power_min * -220) / ((settings.power_min * -1) + settings.power_max);
    power_bar_positive = lv_arc_create(power_scale);
    lv_obj_set_size(power_bar_positive, SPEED_SCALE_WIDTH - 52, SPEED_SCALE_HEIGHT - 20);
    lv_obj_set_pos(power_bar_positive, 2, 2);
    lv_arc_set_rotation(power_bar_positive, 160);
    lv_arc_set_bg_angles(power_bar_positive, power_bar_zero, 220);
    lv_obj_remove_style(power_bar_positive, NULL, LV_PART_KNOB);
    lv_arc_set_mode(power_bar_positive, LV_ARC_MODE_NORMAL);
    lv_arc_set_range(power_bar_positive, 0, settings.power_max);
    lv_obj_set_style_bg_opa(power_bar_positive, LV_OPA_TRANSP, LV_PART_MAIN);
    /* lv_obj_set_style_bg_color(power_bar_positive, RGB565(0, 0, 255), LV_PART_MAIN); */
    lv_obj_set_style_arc_opa(power_bar_positive, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(power_bar_positive, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(power_bar_positive, 0, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(power_bar_positive, POWER_COLOR_INDICATOR, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(power_bar_positive, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(power_bar_positive, 16, LV_PART_INDICATOR);
    lv_arc_set_value(power_bar_positive, 0);

    power_bar_negative = lv_arc_create(power_scale);
    lv_obj_set_size(power_bar_negative, SPEED_SCALE_WIDTH - 52, SPEED_SCALE_HEIGHT - 20);
    lv_obj_set_pos(power_bar_negative, 2, 2);
    lv_arc_set_rotation(power_bar_negative, 160);
    lv_arc_set_bg_angles(power_bar_negative, 0, power_bar_zero);
    lv_obj_remove_style(power_bar_negative, NULL, LV_PART_KNOB);
    lv_arc_set_range(power_bar_negative, 0, settings.power_min * -1);
    lv_arc_set_mode(power_bar_negative, LV_ARC_MODE_REVERSE);
    lv_obj_set_style_bg_opa(power_bar_negative, LV_OPA_TRANSP, LV_PART_MAIN);
    /* lv_obj_set_style_bg_color(power_bar_negative, RGB565(0, 0, 255), LV_PART_MAIN); */
    lv_obj_set_style_arc_opa(power_bar_negative, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(power_bar_negative, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(power_bar_negative, 0, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(power_bar_negative, POWER_COLOR_INDICATOR, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(power_bar_negative, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(power_bar_negative, 16, LV_PART_INDICATOR);
    lv_arc_set_value(power_bar_negative, 0);
    /* lv_arc_set_angles(power_bar_positive, 40, 60); */

    speed_bar = lv_arc_create(speed_scale);
    lv_obj_set_size(speed_bar, SPEED_SCALE_WIDTH - 4, SPEED_SCALE_HEIGHT - 4);
    lv_obj_set_pos(speed_bar, 2, 2);
    lv_arc_set_rotation(speed_bar, 160);
    lv_arc_set_bg_angles(speed_bar, 0, 220);
    lv_obj_remove_style(speed_bar, NULL, LV_PART_KNOB);
    lv_arc_set_range(speed_bar, 0, settings.speed_max * 1000);
    lv_obj_set_style_arc_opa(speed_bar, LV_OPA_TRANSP, LV_PART_MAIN);
    /* lv_obj_set_style_bg_color(speed_bar, RGB565(0, 255, 0), LV_PART_MAIN); */
    lv_obj_set_style_arc_rounded(speed_bar, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(speed_bar, 0, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(speed_bar, SPEED_COLOR_INDICATOR, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(speed_bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(speed_bar, 16, LV_PART_INDICATOR);
    lv_obj_set_style_radial_offset(speed_bar, 2, LV_PART_INDICATOR);
    lv_arc_set_value(speed_bar, 0);
    /* lv_arc_set_angles(speed_bar, 0, 100); */
    // speed arc

#endif

    temperature_img = lv_image_create(lv_screen_active());
    lv_image_set_src(temperature_img, &icon_temperature);
    lv_obj_set_pos(temperature_img, TEMP_IMG_X, TEMP_IMG_Y);
    temperature = lv_label_create(lv_screen_active());
    lv_obj_add_style(temperature, &text_slim, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(temperature, TEMP_TEXT_X, TEMP_TEXT_Y);
    lv_obj_set_size(temperature, TEMP_TEXT_WIDTH, TEMP_TEXT_HEIGHT);
    lv_obj_set_style_text_align(temperature, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(temperature, "%02d°", 12);
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
    lv_label_set_text_fmt(motor_temperature, "% 3d°", 123);
    lv_label_set_long_mode(motor_temperature, LV_LABEL_LONG_CLIP);

    // brake icon
    brake_img = lv_image_create(lv_screen_active());
    lv_image_set_src(brake_img, &icon_brake);
    lv_obj_set_pos(brake_img, BRAKE_IMG_X, BRAKE_IMG_Y);
    lv_obj_add_flag(brake_img, LV_OBJ_FLAG_HIDDEN);


    // battery
    battery = lv_image_create(lv_screen_active());
    lv_image_set_src(battery, &battery_black);
    lv_obj_set_pos(battery, 0, 0);
    lv_obj_set_style_bg_color(battery, lv_color_black(), LV_PART_MAIN);

    battery_bar  = lv_bar_create(lv_screen_active());
    static lv_style_t battery_style_bar, battery_style_bar_indicator;
    /* lv_style_set_bg_opa(&battery_style_bar, LV_OPA_COVER); */
    lv_style_set_border_width(&battery_style_bar, 0);
    lv_style_set_radius(&battery_style_bar, 0);
    lv_style_init(&battery_style_bar_indicator);
    lv_style_set_bg_opa(&battery_style_bar_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&battery_style_bar_indicator, lv_palette_main(LV_PALETTE_RED));
    /* lv_style_set_bg_grad_color(&battery_style_bar_indicator, lv_palette_main(LV_PALETTE_GREEN)); */
    /* lv_style_set_bg_grad_dir(&battery_style_bar_indicator, LV_GRAD_DIR_HOR); */
    lv_style_set_radius(&battery_style_bar_indicator, 0);

    lv_obj_set_style_bg_color(battery_bar, lv_color_black(), LV_PART_MAIN);
    lv_obj_add_style(battery_bar, &battery_style_bar, LV_PART_MAIN);
    lv_obj_add_style(battery_bar, &battery_style_bar_indicator, LV_PART_INDICATOR);
    lv_obj_set_pos(battery_bar, BATTERY_BAR_X, BATTERY_BAR_Y); 
    lv_obj_set_size(battery_bar, BATTERY_BAR_WIDTH, BATTERY_BAR_HEIGHT);
    lv_bar_set_range(battery_bar, 0, 100);
    lv_bar_set_value(battery_bar, 50, LV_ANIM_OFF);

    battery_label = lv_label_create(lv_screen_active());
    lv_obj_add_style(battery_label, &text_slim, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(battery_label, BATTERY_LABEL_X, BATTERY_LABEL_Y);
    lv_obj_set_size(battery_label, BATTERY_LABEL_WIDTH, BATTERY_LABEL_HEIGHT);
    lv_obj_set_style_text_align(battery_label, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(battery_label, "%02" LID ".%1" LID "V", (int32_t) 24, (int32_t) 0);
    lv_label_set_long_mode(battery_label, LV_LABEL_LONG_CLIP);

    // bottom text values
    total_distance_img = lv_image_create(lv_screen_active());
    lv_image_set_src(total_distance_img, &icon_journey);
    lv_obj_set_pos(total_distance_img, TOTAL_DISTANCE_IMG_X, TOTAL_DISTANCE_IMG_Y);
    total_distance_text = lv_label_create(lv_screen_active());
    lv_obj_add_style(total_distance_text, &text_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(total_distance_text, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(total_distance_text, "000.0");
    lv_label_set_long_mode(total_distance_text, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(total_distance_text, TOTAL_DISTANCE_TEXT_X, TOTAL_DISTANCE_TEXT_Y);
    lv_obj_set_size(total_distance_text, TOTAL_DISTANCE_TEXT_WIDTH, TOTAL_DISTANCE_TEXT_HEIGHT);
    lv_label_set_recolor(total_distance_text, true);

#if UART_COMM == UART_COMM_VESC
    distance_img = lv_image_create(lv_screen_active());
    lv_image_set_src(distance_img, &icon_energy);
    lv_obj_set_pos(distance_img, DISTANCE_LEFT_IMG_X, DISTANCE_LEFT_IMG_Y);
    distance_text = lv_label_create(lv_screen_active());
    lv_obj_add_style(distance_text, &text_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(distance_text, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(distance_text, "00.0");
    lv_label_set_long_mode(distance_text, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(distance_text, DISTANCE_LEFT_TEXT_X, DISTANCE_LEFT_TEXT_Y);
    lv_obj_set_size(distance_text, DISTANCE_LEFT_TEXT_WIDTH, DISTANCE_LEFT_TEXT_HEIGHT);
    lv_label_set_recolor(distance_text, true);
#endif

    trip_distance_img = lv_image_create(lv_screen_active());
    lv_image_set_src(trip_distance_img, &icon_trip);
    lv_obj_set_pos(trip_distance_img, TRIP_DISTANCE_IMG_X, TRIP_DISTANCE_IMG_Y);
    trip_distance_text = lv_label_create(lv_screen_active());
    lv_obj_add_style(trip_distance_text, &text_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(trip_distance_text, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(trip_distance_text, "000.0");
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
    lv_label_set_text_fmt(trip_time_text, "000.0");
    lv_label_set_long_mode(trip_time_text, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(trip_time_text, TRIP_TIME_TEXT_X, TRIP_TIME_TEXT_Y);
    lv_obj_set_size(trip_time_text, TRIP_TIME_TEXT_WIDTH, TRIP_TIME_TEXT_HEIGHT);

#ifdef LEXT_INSTALLED 
    time_text = lv_label_create(lv_screen_active());
    lv_obj_add_style(time_text, &text_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(time_text, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text_fmt(time_text, "00:00");
    lv_label_set_long_mode(time_text, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(time_text, TIME_TEXT_X, TIME_TEXT_Y);
    lv_obj_set_size(time_text, TIME_TEXT_WIDTH, TIME_TEXT_HEIGHT);
#endif
    {
        graph = lv_chart_create(lv_screen_active());
        lv_chart_set_type(graph, LV_CHART_TYPE_LINE);
        lv_obj_set_pos(graph, GRAPH_X + 24, GRAPH_Y);
        lv_obj_set_size(graph, GRAPH_WIDTH - 24, GRAPH_HEIGHT);
        lv_chart_set_range(graph, LV_CHART_AXIS_PRIMARY_X, 0, GRAPH_POINT_COUNT);
        lv_chart_set_range(graph, LV_CHART_AXIS_PRIMARY_Y, 0, settings.graph_max * 1000);

        graph_scale = lv_scale_create(lv_screen_active());
        lv_scale_set_mode(graph_scale, LV_SCALE_MODE_VERTICAL_LEFT);
        lv_obj_set_pos(graph_scale, GRAPH_X, GRAPH_Y);
        lv_obj_set_size(graph_scale, 24, GRAPH_HEIGHT);
        lv_scale_set_label_show(graph_scale, true);
        lv_obj_set_style_length(graph_scale, 3, LV_PART_INDICATOR);
        lv_obj_set_style_length(graph_scale, 1, LV_PART_ITEMS);
        lv_scale_set_total_tick_count(graph_scale, 7);
        lv_scale_set_major_tick_every(graph_scale, 1);
        lv_scale_set_range(graph_scale, 0, settings.graph_max);
        /* lv_obj_set_style_pad_hor(graph_scale, lv_chart_get_first_point_center_offset(chart), 0); */
        /* static const char * yaxis[] = {"0", "5", "10", "15", "20", "25", "30", NULL}; */
        /* lv_scale_set_text_src(graph_scale, yaxis); */

        static lv_style_t graph_scale_indicator_style;
        lv_style_init(&graph_scale_indicator_style);
        lv_style_set_text_font(&graph_scale_indicator_style, LV_FONT_DEFAULT);
        lv_style_set_text_color(&graph_scale_indicator_style, lv_color_darken(lv_color_white(), 2));
        lv_style_set_width(&graph_scale_indicator_style, 3U);      /*Tick length*/
        lv_style_set_line_width(&graph_scale_indicator_style, 2U);  /*Tick width*/
        lv_obj_add_style(graph_scale, &graph_scale_indicator_style, LV_PART_INDICATOR);

        static lv_style_t graph_scale_minor_ticks_style;
        lv_style_init(&graph_scale_minor_ticks_style);
        lv_style_set_line_color(&graph_scale_minor_ticks_style, lv_color_lighten(lv_color_white(), 3));
        lv_style_set_width(&graph_scale_minor_ticks_style, 2U);         /*Tick length*/
        lv_style_set_line_width(&graph_scale_minor_ticks_style, 1U);    /*Tick width*/
        lv_obj_add_style(graph_scale, &graph_scale_minor_ticks_style, LV_PART_ITEMS);

        static lv_style_t graph_scale_main_line_style;
        lv_style_init(&graph_scale_main_line_style);
        /* Main line properties */
        lv_style_set_line_color(&graph_scale_main_line_style, lv_color_darken(lv_color_white(), 1));
        lv_style_set_line_width(&graph_scale_main_line_style, 2U); // Tick width
        lv_obj_add_style(graph_scale, &graph_scale_main_line_style, LV_PART_MAIN);

        static lv_style_t graph_style_main, graph_style_item;
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
        lv_style_set_line_color(&graph_style_main, GRAPH_DIV_COLOR);
        lv_style_set_line_width(&graph_style_item, 1);
        lv_style_set_radius(&graph_style_main, 0); // remove radius

        lv_style_set_line_width(&graph_style_item, 2);
        lv_style_set_bg_color(&graph_style_item, GRAPH_CHART_COLOR);
        lv_style_set_pad_column(&graph_style_item, 0);

        lv_obj_add_style(graph, &graph_style_item, LV_PART_ITEMS);
        lv_obj_add_style(graph, &graph_style_main, LV_PART_MAIN);

        lv_obj_set_style_size(graph, 0, 0, LV_PART_INDICATOR); // disable dots
        if (settings.graph_field != GRAPH_FIELD_SHIFT) graph_cursor = lv_chart_add_cursor(graph, GRAPH_CURSOR_COLOR, LV_DIR_TOP | LV_DIR_BOTTOM);

        lv_chart_set_div_line_count(graph, settings.speed_max / 5, 0);
        lv_chart_set_point_count(graph, GRAPH_POINT_COUNT);
        /* lv_chart_set_update_mode(graph, LV_CHART_UPDATE_MODE_SHIFT); */
        lv_chart_set_update_mode(graph, settings.graph_field == GRAPH_FIELD_SHIFT ? LV_CHART_UPDATE_MODE_SHIFT : LV_CHART_UPDATE_MODE_CIRCULAR);
        graph_series = lv_chart_add_series(graph, GRAPH_LINE_COLOR, LV_CHART_AXIS_PRIMARY_Y);
        lv_chart_set_series_ext_y_array(graph, graph_series, graph_values); 
        /* lv_chart_set_ext_y_array(graph, graph_series, graph_array); */
        lv_chart_set_all_value(graph, graph_series, 0);

        speed_label = lv_label_create(lv_screen_active());
        lv_obj_add_style(speed_label, &text_large, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(speed_label, LV_TEXT_ALIGN_RIGHT, 0);
        lv_label_set_text_fmt(speed_label, "00");
        lv_label_set_long_mode(speed_label, LV_LABEL_LONG_CLIP);
        lv_obj_set_pos(speed_label, SPEED_LABEL_X, SPEED_LABEL_Y);
        lv_obj_set_size(speed_label, SPEED_LABEL_WIDTH, SPEED_LABEL_HEIGHT);

        speed_label_minor = lv_label_create(lv_screen_active());
        lv_obj_add_style(speed_label_minor, &text_slim, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(speed_label_minor, LV_TEXT_ALIGN_LEFT, 0);
        lv_label_set_text_fmt(speed_label_minor, "0");
        lv_label_set_long_mode(speed_label_minor, LV_LABEL_LONG_CLIP);
        lv_obj_set_pos(speed_label_minor, SPEED_LABEL_MINOR_X, SPEED_LABEL_MINOR_Y);
        lv_obj_set_size(speed_label_minor, SPEED_LABEL_MINOR_WIDTH, SPEED_LABEL_MINOR_HEIGHT);


        notification = lv_label_create(lv_screen_active());
        lv_obj_add_style(notification, &text_rbt, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(notification, LV_TEXT_ALIGN_LEFT, 0);
        /* lv_label_set_text_fmt(notification, "This is a notification of at least two lines"); */
        lv_label_set_long_mode(notification, LV_LABEL_LONG_WRAP);
        lv_obj_set_pos(notification, NOTIFICATION_X, NOTIFICATION_Y);
        lv_obj_set_size(notification, NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT);
        lv_obj_set_style_bg_color(notification, NOTIFICATION_BG, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(notification, LV_OPA_90, LV_PART_MAIN);
        lv_obj_set_style_text_align(notification, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_move_foreground(notification);
        lv_obj_add_flag(notification, LV_OBJ_FLAG_HIDDEN);
        /* gui_write_bt_msg("Starting up ...", 17); */

        nav_distance = lv_label_create(lv_screen_active());
        lv_obj_add_style(nav_distance, &text_slim, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(nav_distance, LV_TEXT_ALIGN_LEFT, 0);
        lv_label_set_text_fmt(nav_distance, " ");
        lv_label_set_long_mode(nav_distance, LV_LABEL_LONG_CLIP);
        lv_obj_set_pos(nav_distance, NAV_DISTANCE_X, NAV_DISTANCE_Y);
        lv_obj_set_size(nav_distance, NAV_DISTANCE_WIDTH, NAV_DISTANCE_HEIGHT);

        nav_time = lv_label_create(lv_screen_active());
        lv_obj_add_style(nav_time, &text_slim, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(nav_time, LV_TEXT_ALIGN_LEFT, 0);
        lv_label_set_text_fmt(nav_time, " ");
        lv_label_set_long_mode(nav_time, LV_LABEL_LONG_CLIP);
        lv_obj_set_pos(nav_time, NAV_TIME_X, NAV_TIME_Y);
        lv_obj_set_size(nav_time, NAV_TIME_WIDTH, NAV_TIME_HEIGHT);

        nav_img = lv_image_create(lv_screen_active());
        lv_obj_set_pos(nav_img, NAV_IMG_X, NAV_IMG_Y);
        lv_obj_set_size(nav_img, NAV_IMG_WIDTH, NAV_IMG_HEIGHT);
        /* lv_obj_set_style_bg_color(nav_img, lv_color_white(), LV_PART_MAIN); */
        /* lv_obj_set_style_bg_opa(nav_img, LV_OPA_COVER, LV_PART_MAIN); */
    }

    _draw_graph_switch();
#if  (DEBUG && (UART_COMM == UART_COMM_EBICS))
    cm_label = lv_label_create(lv_screen_active());
    lv_obj_add_style(cm_label, &text_slim, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(cm_label, CM_LABEL_X, CM_LABEL_Y);
    lv_obj_set_size(cm_label, CM_LABEL_WIDTH, CM_LABEL_HEIGHT);
    lv_obj_set_style_text_align(cm_label, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text(cm_label, "X");
#endif

#ifdef DEBUG
    /* debug_text = lv_label_create(lv_screen_active()); */
    /* static lv_style_t debug_text_style; */
    /* lv_style_init(&debug_text_style); */
    /* lv_style_set_text_font(&debug_text_style, &lv_font_plex_16); */
    /* lv_style_set_text_color(&debug_text_style, COLOR_WHITE); */
    /* lv_style_set_bg_color(&debug_text_style, lv_color_make(0, 0, 220)); */
    /* lv_style_set_bg_opa(&debug_text_style, LV_OPA_COVER); */
    /* lv_style_set_pad_top(&debug_text_style, 1); */
    /* lv_obj_add_style(debug_text, &debug_text_style, LV_PART_MAIN | LV_STATE_DEFAULT); */
    /* lv_obj_set_pos(debug_text, DEBUG_X, DEBUG_Y); */
    /* lv_obj_set_size(debug_text, DEBUG_WIDTH, DEBUG_HEIGHT); */
    /* lv_obj_set_style_text_align(debug_text, LV_TEXT_ALIGN_LEFT , 0); */
    /* lv_label_set_text_fmt(debug_text, "DEBUG DEBUG DEBUG %08X", HAL_GetTick()); */
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
    lv_style_set_text_font(&assist_mode_selected_style, &lv_font_plex_72);
    lv_style_set_bg_opa(&assist_mode_selected_style, LV_OPA_TRANSP);
    lv_style_set_bg_color(&assist_mode_selected_style, COLOR_BLACK);
    lv_style_set_text_align(&assist_mode_selected_style, LV_TEXT_ALIGN_CENTER);

    lv_style_init(&assist_mode_selected_style);
    lv_style_set_text_font(&assist_mode_normal_style, &lv_font_plex_72);
    lv_style_set_bg_opa(&assist_mode_normal_style, LV_OPA_TRANSP);
    lv_style_set_bg_color(&assist_mode_normal_style, COLOR_BLACK);
    lv_style_set_text_align(&assist_mode_normal_style, LV_TEXT_ALIGN_CENTER);
#if LVGL_VERSION_MAJOR == 9
    lv_style_set_border_width(&assist_mode_normal_style, 0);
#endif


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
    lv_obj_set_style_bg_color(assist_mode, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(assist_mode, lv_color_black(), LV_PART_SELECTED);
    lv_obj_set_style_text_line_space(assist_mode, 5, LV_PART_MAIN);
    lv_obj_set_style_text_color(assist_mode, lv_color_white(), LV_PART_SELECTED);

    lv_obj_set_style_bg_opa(assist_mode, LV_OPA_50, LV_PART_SELECTED);

    /* lv_obj_set_style_text_font(assist_mode, &lv_font_plex_72, LV_PART_MAIN | LV_PART_SELECTED); */
    /* lv_obj_set_style_text_align(assist_mode, LV_TEXT_ALIGN_CENTER, 0); */
    /* lv_obj_align(assist_mode, LV_ALIGN_LEFT_MID, 10, 0); */
    /* lv_obj_add_event_cb(assist_mode, event_handler, LV_EVENT_ALL, NULL); */
    lv_roller_set_selected(assist_mode, settings.assist_last, LV_ANIM_ON);
    lv_obj_set_style_anim_duration(assist_mode, 250, LV_PART_MAIN);

    lights_img = lv_image_create(lv_screen_active());
    lv_image_set_src(lights_img, &icon_headlight);
    lv_obj_set_pos(lights_img, LIGHTS_IMG_X, LIGHTS_IMG_Y);
    lv_obj_move_background(lights_img);


    // bsod
    bsod = lv_label_create(lv_screen_active());
    lv_obj_add_style(bsod, &text_slim, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(bsod, BSOD_X, BSOD_Y);
    lv_obj_set_size(bsod, BSOD_WIDTH, BSOD_HEIGHT);
    lv_obj_set_style_text_align(bsod, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(bsod, "");
    lv_label_set_long_mode(bsod, LV_LABEL_LONG_MODE_WRAP);
    lv_obj_set_style_bg_color(bsod, BSOD_BG, LV_PART_MAIN);
    lv_obj_set_style_text_color(bsod, BSOD_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bsod, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_font(bsod, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_add_flag(bsod, LV_OBJ_FLAG_HIDDEN);


    // timers
    //
    /* if (draw_graph_timer != NULL) { */
    /*     // */
    /*     lv_timer_del(draw_graph_timer); */
    /* } */

    if (draw_graph_timer == NULL) draw_graph_timer = lv_timer_create(_draw_graph, (settings.graph_duration * 60000) / GRAPH_POINT_COUNT, NULL); // update every interval period

    if (ms100_timer == NULL) {
        ext_temperature = ext_temp();
        int_temperature = int_temp();
        power_value = (battery_current * battery_voltage) / 1000000;
        avg_speed = speed;
        avg_power = power_value;
        battery_voltage = voltage_ebat();
    }

    if (ms100_timer == NULL) ms100_timer = lv_timer_create(_100ms_timer, 100, NULL);
    if (ms50_timer == NULL) ms50_timer = lv_timer_create(_50ms_timer, 50, NULL);
    if (ms500_timer == NULL) ms500_timer = lv_timer_create(_500ms_timer, 500, NULL);
    if (heartbeat_timer == NULL) heartbeat_timer = lv_timer_create(_heartbeat_timer, 1000, NULL);

    draw_lights_trigger = 1;
    draw_speed_trigger = 1;
    draw_distances_trigger = 1;
    draw_temperatures_trigger_internal = 1;
    draw_battery_voltage_trigger = 1;

}

typedef struct {
    int size; // 32b = 4, 16b = 2 etc.
    int sig; // 0 unsigned, 1 signed, -1 float
    void *ref; // reference
    int32_t (*refps)(void); // reference
    uint32_t (*refpu)(void); // reference
    float (*refpc)(void *); // reference
} status_item;

char *status_page_1_text[] = {
    "System",
    "ms since start",
    "Light level",
    "External temp",
    "Internal temp",
    "CPU temp",
    "RTC Bat. V.",
    "Battery voltage"
};

const status_item status_page_1_items[] = {
    { 0, 0, NULL, NULL, NULL, NULL},
    { 4, 1, NULL, NULL, HAL_GetTick, NULL },
    { 4, 1, NULL, light_level, NULL, NULL},
    { 4, 1, NULL, ext_temp, NULL, NULL},
    { 4, 1, NULL, int_temp, NULL, NULL},
    { 4, 1, NULL, internal_temperature, NULL, NULL},
    { 4, 1, NULL, voltage_bat, NULL, NULL},
    { 4, 1, NULL, voltage_ebat, NULL, NULL}
};

int status_page_1_size = 8;

char *status_page_2_text[] = {
    "Controller",
    "Controller temp",
    "Motor temp",
    "Avg. motor cur.",
    "Avg. current",
    "Duty cycle",
    "e-Rpm",
    "Input voltage",
    "Amp hours",
    "Amp hours charged",
    "Tacho",
    "Tacho abs",
    "FET1 temp",
    "FET2 temp",
    "FET3 temp"
};

const status_item status_page_2_items[] = {
    { 0, 0, NULL, NULL, NULL, NULL},
    { 2, 1, &con_temperature, NULL, NULL, NULL},
    { 2, 1, &mot_temperature, NULL, NULL, NULL},
    { 4, 0, &motor_current, NULL, NULL, NULL},
    { 4, 0, &battery_current, NULL, NULL, NULL},
    { 2, 1, &duty_cycle, NULL, NULL, NULL},
    { 4, 1, &erpm, NULL, NULL, NULL},
    { 4, 1, &battery_voltage_controller, NULL, NULL, NULL},
    { 4, 1, &amphours_total, NULL, NULL, NULL},
    { 4, 1, &amphours_regen_total, NULL, NULL, NULL},
    { 4, 1, &tachometer, NULL, NULL, NULL},
    { 4, 1, &tachometer_abs, NULL, NULL, NULL},
    { 2, 1, &fet_temp_1, NULL, NULL, NULL},
    { 2, 1, &fet_temp_2, NULL, NULL, NULL},
    { 2, 1, &fet_temp_3, NULL, NULL, NULL},
};

int status_page_2_size = 15;

char *status_page_3_text[] = {
    "McConf",
    "Cur. Scale min",
    "Cur. Scale max",
    "erpm min",
    "erpm max",
    "duty min",
    "duty max",
    "power min",
    "power max",
    "current min",
    "current max",
    "motor poles",
    "gear ratio",
    "wheel diameter"
};

extern vesc_mcconf_temp mcconf;
const status_item status_page_3_items[] = {
    { 0, 0, NULL, NULL, NULL, NULL},
    { 4, -1, &mcconf.current_scale_min, NULL, NULL, uint32_to_float32_auto},
    { 4, -1, &mcconf.current_scale_max, NULL, NULL, uint32_to_float32_auto},
    { 4, -1, &mcconf.erpm_min, NULL, NULL, uint32_to_float32_auto},
    { 4, -1, &mcconf.erpm_max, NULL, NULL, uint32_to_float32_auto},
    { 4, -1, &mcconf.min_duty, NULL, NULL, uint32_to_float32_auto},
    { 4, -1, &mcconf.max_duty, NULL, NULL, uint32_to_float32_auto},
    { 4, -1, &mcconf.watt_min, NULL, NULL, uint32_to_float32_auto},
    { 4, -1, &mcconf.watt_max, NULL, NULL, uint32_to_float32_auto},
    { 4, -1, &mcconf.in_current_min, NULL, NULL, uint32_to_float32_auto},
    { 4, -1, &mcconf.in_current_max, NULL, NULL, uint32_to_float32_auto},
    { 1, 0, &mcconf.motor_poles, NULL, NULL, NULL},
    { 4, -1, &mcconf.gear_ratio, NULL, NULL, uint32_to_float32_auto},
    { 4, -1, &mcconf.wheel_diameter, NULL, NULL, uint32_to_float32_auto},
};

int status_page_3_size = 13;

lv_obj_t *page_items[15] = {0};
void draw_status_page(char *status_text[], const status_item *status_items, ssize_t size) {
    if (status_page_active != status_page) {
        LV_LOG_INFO("Creating new status page %d, current %d", status_page, status_page_active);
        status_page_active = status_page;

        lv_obj_invalidate(lv_screen_active());
        lv_refr_now(display);

        /* lv_theme_default_init(NULL, COLOR_BLACK, COLOR_WHITE, LV_THEME_DEFAULT_DARK, &lv_font_fry_32); */
        lv_obj_set_style_bg_color(lv_screen_active(), COLOR_BLACK, LV_PART_MAIN);
        lv_obj_set_scrollbar_mode(lv_screen_active(), LV_SCROLLBAR_MODE_OFF);
        static int32_t col_dsc[] = {DISPLAY_WIDTH >> 1, DISPLAY_WIDTH >> 1, LV_GRID_TEMPLATE_LAST};   /* 2 columns with 100- and 400-px width */
        static int32_t row_dsc[] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, LV_GRID_TEMPLATE_LAST}; /* 3 100-px tall rows */
        lv_obj_set_layout(lv_screen_active(), 0);
        lv_obj_t * cont = lv_obj_create(lv_screen_active());
        lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
        lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
        lv_obj_set_size(cont, DISPLAY_WIDTH, DISPLAY_HEIGHT);
        lv_obj_set_style_bg_color(cont, COLOR_BLACK, LV_PART_MAIN);
        lv_obj_center(cont);
        lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_layout(cont, LV_LAYOUT_GRID);

        lv_style_set_text_font(&text_normal, &lv_font_fry_32);
        lv_style_set_bg_color(&text_normal, COLOR_BLACK);
        lv_style_set_text_color(&text_normal, COLOR_WHITE);
        lv_style_set_pad_top(&text_normal, 2);
        lv_style_set_pad_left(&text_normal, 3);
        lv_style_set_text_align(&text_normal, LV_TEXT_ALIGN_LEFT);

        lv_style_set_text_font(&text_fixed, &lv_font_plex_32);
        lv_style_set_bg_color(&text_fixed, COLOR_BLACK);
        lv_style_set_text_color(&text_fixed, COLOR_WHITE);
        lv_style_set_pad_top(&text_fixed, 3);
        lv_style_set_text_align(&text_fixed, LV_TEXT_ALIGN_LEFT);

        for (int i = 0; i < size; i++) {
            lv_obj_t *leftlbl = lv_label_create(cont);
            lv_obj_t *rightlbl = lv_label_create(cont);
            page_items[i] = rightlbl;

            lv_obj_add_style(leftlbl, &text_normal, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(leftlbl, LV_TEXT_ALIGN_LEFT, 0);
            lv_label_set_long_mode(leftlbl, LV_LABEL_LONG_CLIP);

            lv_obj_add_style(rightlbl, &text_fixed, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(rightlbl, LV_TEXT_ALIGN_RIGHT, 0);
            lv_label_set_long_mode(rightlbl, LV_LABEL_LONG_CLIP);
            lv_obj_set_size(rightlbl, DISPLAY_WIDTH / 2, 32);
        
            lv_label_set_text(leftlbl, status_text[i]);
            lv_label_set_text(rightlbl, "");

            lv_obj_set_grid_cell(leftlbl,   LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, i, 1); 
            lv_obj_set_grid_cell(rightlbl,  LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, i, 1); 
        }
    }
    for (int i = 0; i < size; i++) {
        char value[32] = {0}; 
        if (status_items[i].ref != NULL) {
            switch (status_items[i].sig) {
                case 0:
                    switch (status_items[i].size) {
                        case 1:
                            lv_snprintf(value, 32, "%hhu", *((uint8_t *) status_items[i].ref));
                            break;
                        case 2:
                            lv_snprintf(value, 32, "%hu", *((uint16_t *) status_items[i].ref));
                            break;
                        default:
                        case 4:
                            lv_snprintf(value, 32, "%lu", *((uint32_t *) status_items[i].ref));
                            break;
                    }
                    break;
                case 1:
                    switch (status_items[i].size) {
                        case 1:
                            lv_snprintf(value, 32, "%hhd", *((int8_t *) status_items[i].ref));
                            break;
                        case 2:
                            lv_snprintf(value, 32, "%hd", *((int16_t *) status_items[i].ref));
                            break;
                        default:
                        case 4:
                            lv_snprintf(value, 32, "%ld", *((int32_t *) status_items[i].ref));
                            break;
                    }
                    break;
                case -1:
                    if (status_items[i].refpc != NULL) {
                        lv_snprintf(value, 32, "%.2f", status_items[i].refpc(status_items[i].ref));
                    } else {
                        lv_snprintf(value, 32, "%.2f", *((float *) status_items[i].ref));
                    }
                    break;
            }
        } else if (status_items[i].refpu != NULL) {
            lv_snprintf(value, 32, "%lu", status_items[i].refpu());
        } else if (status_items[i].refps != NULL) {
            lv_snprintf(value, 32, "%ld", status_items[i].refps());
        }

        lv_label_set_text_fmt(page_items[i], "%s", value);
    }

}

static CRITICAL void _status_page_timer(lv_timer_t *timer) {
    gui_draw_status();
}

void gui_draw_status(void) {
    LV_LOG_TRACE("Drawing status screen page %d", status_page);

    if (status_page_timer == NULL) status_page_timer = lv_timer_create(_status_page_timer, 100, NULL);
    switch (status_page) {
        default:
        case STATUS_PAGE_NONE:
            LV_LOG_ERROR("INVALID STATUS PAGE");
            break;
        case STATUS_SYSTEM:
            LV_LOG_TRACE("Status page system");
            draw_status_page(status_page_1_text, (const status_item *) status_page_1_items, status_page_1_size);
            break;
        case STATUS_CONTROLLER:
            LV_LOG_TRACE("Status page controller");
            draw_status_page(status_page_2_text, (const status_item *) status_page_2_items, status_page_2_size);
            comm_vesc_packet_send_get_data(0xFFFFFFFF); // get all
            break;
        case STATUS_MCCONF:
            LV_LOG_TRACE("Status page mcconf");
            draw_status_page(status_page_3_text, (const status_item *) status_page_3_items, status_page_3_size);
            comm_vesc_packet_send_mconf_get();
            break;
    }
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
    char    **list;
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
    lv_label_set_text_fmt(it3, "%" LID, (int32_t) valvalue); \
    lv_obj_set_height(it1, 36);\
    lv_obj_set_scrollbar_mode(it1, LV_SCROLLBAR_MODE_OFF);\
    static struct item_event_data eventdata_ ## index = { .min = valmin, .max = valmax, .step = valstep, .value = &valvalue, .value_size = valsize, .function = valfunction, .list = NULL };\
    lv_obj_add_event_cb(it1, item_event_cb, LV_EVENT_GESTURE, &eventdata_ ## index);


#define LIST_ITEM_LIST(index, text, valmin, valmax, valvalue, valsize, lst)\
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
    lv_label_set_text_fmt(it3, "%" LID, (int32_t) valvalue); \
    lv_obj_set_height(it1, 36);\
    lv_obj_set_scrollbar_mode(it1, LV_SCROLLBAR_MODE_OFF);\
    static struct item_event_data eventdata_ ## index = { .min = valmin, .max = valmax, .step = 1, .value = &valvalue, .value_size = valsize, .function = NULL, .list = (char **) lst };\
    lv_obj_add_event_cb(it1, item_event_cb, LV_EVENT_GESTURE, &eventdata_ ## index);

#define LIST_ITEM_HIGHLIGHT(index) \
    lv_obj_remove_style(list_items[index], &item_style, LV_PART_MAIN); \
    lv_obj_remove_style(list_items[index], &item_style_checked, LV_PART_MAIN); \
    lv_obj_add_style(list_items[index], &item_style_checked, LV_PART_MAIN);\
    lv_obj_add_state(list_items[index], LV_STATE_FOCUSED);\
    for(int i = 0; i < lv_obj_get_child_cnt(list_items[index]); i++) { lv_obj_t * child = lv_obj_get_child(list_items[index], i); lv_obj_add_style(child, &item_style_checked, LV_PART_MAIN); };\
    for(int i = 0; i < lv_obj_get_child_cnt(list_items[index]); i++) { lv_obj_t * child = lv_obj_get_child(list_items[index], i); lv_obj_remove_style(child, &item_style, LV_PART_MAIN); };\
    for(int i = 0; i < lv_obj_get_child_cnt(list_items[index]); i++) { lv_obj_t * child = lv_obj_get_child(list_items[index], i); lv_obj_remove_style(child, &item_style_selected, LV_PART_MAIN); };\
    lv_obj_scroll_to_view(list_items[index], LV_ANIM_ON);

#define LIST_ITEM_NORMAL(index)    \
    lv_obj_remove_style(list_items[index], &item_style_selected, LV_PART_MAIN); \
    lv_obj_remove_style(list_items[index], &item_style, LV_PART_MAIN); \
    lv_obj_add_style(list_items[index], &item_style, LV_PART_MAIN);\
    for(int i = 0; i < lv_obj_get_child_cnt(list_items[index]); i++) { lv_obj_t * child = lv_obj_get_child(list_items[index], i); lv_obj_add_style(child, &item_style, LV_PART_MAIN); };\
    for(int i = 0; i < lv_obj_get_child_cnt(list_items[index]); i++) { lv_obj_t * child = lv_obj_get_child(list_items[index], i); lv_obj_remove_style(child, &item_style_checked, LV_PART_MAIN); };\
    for(int i = 0; i < lv_obj_get_child_cnt(list_items[index]); i++) { lv_obj_t * child = lv_obj_get_child(list_items[index], i); lv_obj_remove_style(child, &item_style_selected, LV_PART_MAIN); };\
    lv_obj_clear_state(list_items[index], LV_STATE_FOCUSED);

#define LIST_ITEM_SELECT(index)    \
    lv_obj_remove_style(list_items[index], &item_style_checked, LV_PART_MAIN); \
    lv_obj_remove_style(list_items[index], &item_style, LV_PART_MAIN); \
    lv_obj_add_style(list_items[index], &item_style_selected, LV_PART_MAIN);\
    for(int i = 0; i < lv_obj_get_child_cnt(list_items[index]); i++) { lv_obj_t * child = lv_obj_get_child(list_items[index], i); lv_obj_add_style(child, &item_style_selected, LV_PART_MAIN); };\
    for(int i = 0; i < lv_obj_get_child_cnt(list_items[index]); i++) { lv_obj_t * child = lv_obj_get_child(list_items[index], i); lv_obj_remove_style(child, &item_style_checked, LV_PART_MAIN); };\
    for(int i = 0; i < lv_obj_get_child_cnt(list_items[index]); i++) { lv_obj_t * child = lv_obj_get_child(list_items[index], i); lv_obj_remove_style(child, &item_style, LV_PART_MAIN); };\
    lv_obj_add_state(list_items[index], LV_STATE_FOCUSED);

static void item_event_cb(lv_event_t *e) {
    // 
    struct item_event_data *data = lv_event_get_user_data(e);
    int value = *((int*) lv_event_get_param(e));

    int8_t *n = (int8_t*)data->value;
    int16_t *m = (int16_t*)data->value;
    int32_t *o = (int32_t*)data->value;
    uint8_t *p = (uint8_t*)data->value;
    uint16_t *q = (uint16_t*)data->value;
    uint32_t *r = (uint32_t*)data->value;

    int32_t t = 0;
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
        case 11:
            t = (int) *p;
            break;
        case 12:
            t = (int) *q;
            break;
        case 14:
            t = (int) *r;
            break;
    }

    lv_obj_t *cb = lv_event_get_current_target(e);
    if (lv_obj_check_type(cb, &lv_checkbox_class)) {
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
        if (data->list != NULL) {
            lv_label_set_text_fmt(i, "%s", data->list[t]);
        } else {
            lv_label_set_text_fmt(i, "%" LID, t);
        }
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
            *o = (int32_t) t;
            break;
        case 11:
            *p = (int8_t) t;
            break;
        case 12:
            *q = (int16_t) t;
            break;
        case 14:
            *r = (int32_t) t;
            break;
    }
}

static lv_style_t item_style, item_style_checked, item_style_selected, list_style, list_scrollbar, item_style_checkbox, item_style_checkbox_checked;
void gui_draw_settings_main(void) {
    reset_all_buttons();
    lv_obj_invalidate(lv_screen_active());
    lv_refr_now(display);
    /* lv_theme_default_init(NULL, COLOR_BLACK, COLOR_WHITE, LV_THEME_DEFAULT_DARK, &lv_font_fry_32); */
    lv_obj_set_style_bg_color(lv_screen_active(), COLOR_BLACK, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(lv_screen_active(), LV_SCROLLBAR_MODE_OFF);

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
    lv_style_set_pad_all(&item_style_checked, 0);
    lv_style_set_border_width(&item_style_checked, 0);

    lv_style_set_border_color(&item_style_selected, COLOR_BLACK);
    lv_style_set_bg_color(&item_style_selected, COLOR_BLUE);
    lv_style_set_bg_opa(&item_style_selected, LV_OPA_COVER);
    lv_style_set_text_color(&item_style_selected, COLOR_WHITE);
    lv_style_set_pad_all(&item_style_selected, 0);
    lv_style_set_border_width(&item_style_selected, 0);

    lv_style_init(&item_style_checkbox);
    lv_style_init(&item_style_checkbox_checked);
    lv_style_set_radius(&item_style_checkbox, LV_RADIUS_CIRCLE);
    lv_style_set_radius(&item_style_checkbox_checked, LV_RADIUS_CIRCLE);
    lv_style_set_bg_color(&item_style_checkbox_checked, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_img_src(&item_style_checkbox_checked, NULL);
    lv_style_set_pad_all(&item_style_selected, 0);

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
    LIST_ITEM_LABEL(2, "FACTORY RESET");
#if LEXT_INSTALLED
    LIST_ITEM_LABEL(3, "TIME"); 
    LIST_ITEM_COUNTER(4, "BACKLIGHT", 1, 100, 1, settings.backlight_level, 1, lcd_backlight);
    LIST_ITEM_COUNTER(5, "BACKLIGHT SENS.", -1, 10, 1, settings.backlight_sensitivity, 1, NULL);
    LIST_ITEM_COUNTER(6, "AUTO LIGHT", 10, 65000, 10, settings.light_sensitivity, 12, NULL);
    LIST_ITEM_LABEL(7, "BACK");

    light_sensitivity_label = lv_label_create(lv_screen_active());
    lv_obj_add_style(light_sensitivity_label, &text_normal, LV_PART_MAIN);
    lv_obj_add_style(light_sensitivity_label, &item_style, LV_PART_MAIN);
    lv_obj_set_pos(light_sensitivity_label, 140, (4 * 36) + 2);
    lv_obj_set_size(light_sensitivity_label, 96, 36);
    lv_obj_set_style_text_align(light_sensitivity_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_label_set_text_fmt(light_sensitivity_label, "%" LID, (int32_t) light_level());
    lv_obj_set_style_bg_opa(light_sensitivity_label, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_text_color(light_sensitivity_label, COLOR_GREY, LV_PART_MAIN); 
    lv_obj_set_style_text_font(light_sensitivity_label, &lv_font_plex_32, LV_PART_MAIN);

    list_item_max = 8;
#else
    LIST_ITEM_COUNTER(3, "BACKLIGHT", 1, 100, 1, settings.backlight_level, 1, lcd_backlight);
    LIST_ITEM_COUNTER(4, "BACKLIGHT SENS.", -1, 10, 1, settings.backlight_sensitivity, 1, NULL);
    LIST_ITEM_COUNTER(5, "AUTO LIGHT", 10, 65000, 10, settings.light_sensitivity, 12, NULL);
    LIST_ITEM_LABEL(6, "BACK");
    
    light_sensitivity_label = lv_label_create(lv_screen_active());
    lv_obj_add_style(light_sensitivity_label, &text_normal, LV_PART_MAIN);
    lv_obj_add_style(light_sensitivity_label, &item_style, LV_PART_MAIN);
    lv_obj_set_pos(light_sensitivity_label, 140, (4 * 36) + 2);
    lv_obj_set_size(light_sensitivity_label, 96, 36);
    lv_obj_set_style_text_align(light_sensitivity_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_label_set_text_fmt(light_sensitivity_label, "%" LID, (int32_t) light_level());
    lv_obj_set_style_bg_opa(light_sensitivity_label, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_text_color(light_sensitivity_label, COLOR_GREY, LV_PART_MAIN); 
    lv_obj_set_style_text_font(light_sensitivity_label, &lv_font_plex_32, LV_PART_MAIN);

    list_item_max = 7;
#endif

    LIST_ITEM_HIGHLIGHT(0);    

    list_item = 0;

    // add light level event timer

    if (light_level_timer == NULL) light_level_timer = lv_timer_create(_light_level_timer, 250, NULL);
}

void gui_draw_settings_controller(void) {
    reset_all_buttons();
    if (NULL != light_level_timer) {
        lv_timer_delete(light_level_timer);
        light_level_timer = NULL;
    }
    lv_refr_now(display);
    list = lv_obj_create(lv_screen_active());
    lv_obj_set_scrollbar_mode(lv_screen_active(), LV_SCROLLBAR_MODE_OFF);
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

    LIST_ITEM_COUNTER(0, "SPEED ASSIST MAX.", 0, 100, 1, settings.speed_assist_max, 11, NULL);
    LIST_ITEM_COUNTER(1, "WHEEL CIRC.", 0, 300, 1, settings.wheel_circumfence, 12, NULL);
    LIST_ITEM_COUNTER(2, "CURRENT MAX.", 0, 65000, 100, settings.current_max, 12, NULL);
    LIST_ITEM_COUNTER(3, "REGEN CUR. MAX.", 0, 65000, 100, settings.regen_current, 12, NULL);
    LIST_ITEM_COUNTER(4, "PAS TIMEOUT", 0, 32000, 100, settings.pas_timeout, 12, NULL);
    LIST_ITEM_COUNTER(5, "PAS RAMP", 0, 10000, 100, settings.pas_ramp, 12, NULL);
    LIST_ITEM_COUNTER(6, "MIN POWER", -1000, 1000, 10, settings.controller_power_min, 2, NULL); 
    LIST_ITEM_COUNTER(7, "MAX POWER", -1000, 1000, 10, settings.controller_power_max, 2, NULL); 
    LIST_ITEM_LABEL(8, "BACK");
    /* LIST_ITEM_LABEL(1, "Controller"); */
    /* LIST_ITEM_COUNTER(2, "Backlight", 0, 100, 1, settings.backlight_level, 1, lcd_backlight); */
    /* LIST_ITEM_LABEL(3, "Back"); */
    /*  */
    LIST_ITEM_HIGHLIGHT(0);    

    list_item = 0;
    list_item_max = 9;
}

#if LEXT_INSTALLED
static uint8_t hour = 0, min = 0, sec = 0, month = 0, day = 0;
static uint16_t year = 0;
#endif
void gui_draw_time(void) {
#if LEXT_INSTALLED
    if (NULL != light_level_timer) {
        lv_timer_delete(light_level_timer);
        light_level_timer = NULL;
    }
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

    clock_get_all(&hour, &min, &sec, &year, &month, &day);

    lv_obj_t *it1, *it2, *it3 = NULL;

    LIST_ITEM_COUNTER(0, "HOUR",    0, 23, 1, hour, 11, NULL);
    LIST_ITEM_COUNTER(1, "MINUTES", 0, 59, 1, min, 11, NULL);
    LIST_ITEM_COUNTER(2, "SECONDS", 0, 59, 1, sec, 11, NULL);
    LIST_ITEM_COUNTER(3, "YEAR", 2025, 2099, 1, year + 2000, 12, NULL);
    LIST_ITEM_COUNTER(4, "MONTH",   1, 12, 1, month, 11, NULL);
    LIST_ITEM_COUNTER(5, "DAY",     1, 31, 1, day, 11, NULL);
    LIST_ITEM_LABEL(6, "BACK");

    LIST_ITEM_HIGHLIGHT(0);    

    list_item = 0;
    list_item_max = 7;
#endif
}

void gui_draw_settings_display(void) {
    reset_all_buttons();
    if (NULL != light_level_timer) {
        lv_timer_delete(light_level_timer);
        light_level_timer = NULL;
    }
    lv_refr_now(display);
    list = lv_obj_create(lv_screen_active());
    lv_obj_set_scrollbar_mode(lv_screen_active(), LV_SCROLLBAR_MODE_OFF);
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
    static const char *graphlist[] =  { "SHIFT", "END", "PWR/SPD", "UNUSED" };

    LIST_ITEM_COUNTER(0, "BATTERY VMIN", 0, 60000, 100, settings.battery_voltage_min, 12, NULL);
    LIST_ITEM_COUNTER(1, "BATTERY VMAX", 0, 60000, 100, settings.battery_voltage_max, 12, NULL);
    LIST_ITEM_COUNTER(2, "GRAPH DURATION", 0, 30, 1, settings.graph_duration, 11, NULL);
    LIST_ITEM_LIST(3, "GRAPH MODE", 0, GRAPH_FIELD_SPEED_AVG, settings.graph_field, 1, graphlist);
    LIST_ITEM_COUNTER(4, "GRAPH MAX", 0, 100, 1, settings.graph_max, 11, NULL);
    LIST_ITEM_COUNTER(5, "ASSIST LEVELS", 0, 9, 1, settings.assist_levels, 1, NULL);
    LIST_ITEM_COUNTER(6, "SPEED REDLINE", 0, 100, 1, settings.speed_redline, 11, NULL);
    LIST_ITEM_COUNTER(7, "SPEED MAX", 0, 100, 1, settings.speed_max, 11, NULL);
    LIST_ITEM_COUNTER(8, "POWER MIN", -1000, 0, 1, settings.power_min, 2, NULL);
    LIST_ITEM_COUNTER( 9, "POWER REDLINE", 0, 1000, 1, settings.power_redline, 2, NULL);
    LIST_ITEM_COUNTER(10, "POWER MAX", 0, 1000, 1, settings.power_max, 2, NULL);
    LIST_ITEM_CHECKBOX(11, "VOLTAGE FROM CONT.", settings.battery_voltage_from_controller, 1);
    LIST_ITEM_COUNTER(12, "SHUTDOWN TIME", 0, 255, 1, settings.shutdown_timer, 11, NULL);
    LIST_ITEM_LABEL(13, "BACK");
    
    /* LIST_ITEM_LABEL(1, "Controller"); */
    /* LIST_ITEM_COUNTER(2, "Backlight", 0, 100, 1, settings.backlight_level, 1, lcd_backlight); */
    /* LIST_ITEM_LABEL(3, "Back"); */
    /*  */
    LIST_ITEM_HIGHLIGHT(0);    

    list_item = 0;
    list_item_max = 14;
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
#if (DEBUG && (UART_COMM == UART_COMM_EBICS))
            _draw_controller_mode();
#endif
            _draw_bsod();
            _draw_notification();
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
        // avoid memory or bad data corruption making a mess of things
        if (speed <= 99000 && speed >= -99000) {
            lv_arc_set_value(speed_bar, avg_speed);

            if (settings.graph_field == GRAPH_FIELD_SPEED_POWER_AVG) {

                int32_t tspeed = avg_speed < 0 ? avg_speed * -1 : avg_speed;
                lv_label_set_text_fmt(speed_label, "%2" LID, (tspeed / 1000));
                lv_label_set_text_fmt(speed_label_minor, "%1" LID, (tspeed % 1000) / 100);
            }
        }
        /* lv_meter_set_indicator_end_value(meter, indic1, speed_major); */
        draw_speed_trigger = 0;
    }
    oldspeed = speed;
}

void _draw_graph(lv_timer_t *timer) {
    if (mode == MODE_NORMAL) {
        /* if (graph_cursor) lv_chart_set_cursor_point(graph, graph_cursor, graph_series, graph_series->start_point); */
        if (graph && avg_speed <= 99000 && avg_speed >= -99000) lv_chart_set_next_value(graph, graph_series, avg_speed);
    }
}
static void _draw_battery(void) {
    if (draw_battery_voltage_trigger) {
        draw_battery_voltage_trigger = 0;
        lv_label_set_text_fmt(battery_label, "% 2" LID ".%" LID "V", battery_voltage / 1000, (battery_voltage % 1000) / 100);
        battery_voltage_old = battery_voltage;
        battery_value = (((battery_voltage - settings.battery_voltage_min) * 100) / (settings.battery_voltage_max - settings.battery_voltage_min));
        if (battery_value != battery_value_old) {
            lv_bar_set_value(battery_bar, (uint32_t) battery_value, LV_ANIM_ON); 
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
#ifdef BT_UART_ENABLED
            bt_send_battery((uint8_t) (battery_value < 0 ? 0 : battery_value));
#endif
        }
    }
}

#if UART_COMM == UART_COMM_EBICS
static void _draw_controller_mode(void) {
    if (draw_controller_mode_trigger) {
        draw_controller_mode_trigger = 0;
        char *md = "N";
        if (controller_mode == 1) md = "S";
        else if (controller_mode == 5) md = "I";
        else if (controller_mode == 6) md = "P";
        lv_label_set_text(cm_label, md);
    }
}
#endif

uint32_t bsod_timeout = 0;

static void _draw_bsod(void) {
    uint8_t show = 0;
    char msg[512] = {0};

    /* if (!comm_ready) { */
    /*     show = 1; */
    /*     bsod_timeout = HAL_GetTick(); */
    /*     msg = "Waiting for controller to boot"; */
    /* } */
    if (error & ERROR_EEPROM_READ) { show = 1; sprintf(msg, "%s%s", msg, "Cannot read settings from EEPROM\n"); };
    if (error & ERROR_EEPROM_WRITE) { show = 1; sprintf(msg, "%s%s", msg, "Cannot write data to EEPROM\n"); };
    if (error & ERROR_UART_RECEIVE) { show = 1; sprintf(msg, "%s%s", msg, "Failed to receive any data from uart\n"); };
    if (error & ERROR_UART_TRANSMIT) { show = 1; sprintf(msg, "%s%s", msg, "Failed to transmit data over uart\n"); };

    if (show) {
        bsod_timeout = HAL_GetTick();
    }
    
    if ((lv_obj_has_flag(bsod, LV_OBJ_FLAG_HIDDEN)) && (bsod_timeout != 0 && (HAL_GetTick() - bsod_timeout < 1000))) {
        lv_label_set_text(bsod, msg);
        lv_obj_clear_flag(bsod, LV_OBJ_FLAG_HIDDEN);
    } else if ((!lv_obj_has_flag(bsod, LV_OBJ_FLAG_HIDDEN)) && ((HAL_GetTick() - bsod_timeout > 1000))){
        lv_obj_add_flag(bsod, LV_OBJ_FLAG_HIDDEN);
    }
}

static void _draw_graph_switch(void) {
    if (settings.graph_field == GRAPH_FIELD_SHIFT || settings.graph_field == GRAPH_FIELD_RUN) {
        lv_obj_add_flag(speed_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(speed_label_minor, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(nav_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(nav_distance, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(nav_time, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(graph, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(graph_scale, LV_OBJ_FLAG_HIDDEN);
    } else if (settings.graph_field == GRAPH_FIELD_SPEED_POWER_AVG) {
        lv_obj_clear_flag(speed_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(speed_label_minor, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(nav_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(nav_distance, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(nav_time, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(graph, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(graph_scale, LV_OBJ_FLAG_HIDDEN);
    }
}

static void _draw_time(void) {
    if (draw_time_trigger) {
        print_time(trip_time_text, settings.trip_time);
#ifdef LEXT_INSTALLED
        uint8_t h = 0, m = 0;
        clock_get_time(&h, &m);
        lv_label_set_text_fmt(time_text, "%02" LID ":%02" LID, (int32_t) h, (int32_t) m);
#endif
        draw_time_trigger = 0;
    }
}

static void _draw_power(void) {
    if (draw_power_trigger) {
        /* power_value = battery_voltage * battery_current; */
        /* power_value /= 1000000; // convert to W */

        if (avg_power == avg_power_old) {
            // do nothing
        } else if (avg_power >= 0 && avg_power_old > 0) {
            lv_arc_set_value(power_bar_positive, avg_power);
            lv_arc_set_value(power_bar_negative, 0);
        } else if (avg_power >= 0 && avg_power_old < 0) {
            lv_arc_set_value(power_bar_positive, avg_power);
            lv_arc_set_value(power_bar_negative, 0);
        } else if (avg_power < 0 && avg_power_old >= 0) {
            lv_arc_set_value(power_bar_positive, 0);
            lv_arc_set_value(power_bar_negative, avg_power * -1);
        } else if (avg_power < 0 && avg_power_old < 0) {
            lv_arc_set_value(power_bar_positive, 0);
            lv_arc_set_value(power_bar_negative, avg_power * -1);
        }

        avg_power_old = avg_power;
        draw_power_trigger = 0;
    }    
}

static void _draw_temperature(void) {
#ifdef DEBUG
    if (draw_temperatures_trigger_internal) { // internal
        lv_label_set_text_fmt(temperature, "% 2" LID ".%1" LID "°", ext_temperature / 1000, (ext_temperature % 1000) / 100);
        lv_label_set_text_fmt(motor_temperature, "% 2" LID ".%1" LID "°", mot_temperature >> 8, (mot_temperature & 0x000000FF) / 26);
        draw_temperatures_trigger_internal = 0;
    }
#endif
    if (draw_temperatures_trigger) {
        lv_label_set_text_fmt(temperature, "% 2" LID ".%1" LID "°", ext_temperature / 1000, (ext_temperature % 1000) / 100);
        lv_label_set_text_fmt(motor_temperature, "% 3" LID "°C", mot_temperature / 10);
        draw_temperatures_trigger = 0;
    }
}

static void _draw_assist(void) {
    if (draw_assist_trigger) {
        lv_roller_set_selected(assist_mode, settings.assist_last, LV_ANIM_ON);
        draw_assist_trigger = 0;
    }
}

static void _draw_headlights(void) {
    if (draw_lights_trigger) {
        if (settings.lights_mode == LIGHTS_MODE_AUTOMATIC) {
            lv_image_set_src(lights_img, 
                    settings.lights_enabled ? 
                    &icon_headlight_auto_enabled : 
                    &icon_headlight_auto);

            lv_obj_clear_flag(lights_img, LV_OBJ_FLAG_HIDDEN);
        } else {
            if (settings.lights_enabled) {
                lv_image_set_src(lights_img, &icon_headlight_enabled);
                lv_obj_clear_flag(lights_img, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(lights_img, LV_OBJ_FLAG_HIDDEN);
            }
        }
        draw_lights_trigger = 0;
    }
}

static void _draw_distances(void) {
    if (draw_distances_trigger) {
        if (settings.total_distance < 100000000) {
            /* LV_LOG_INFO("DST %d", settings.total_distance); */
            print_digit_text(total_distance_text, (settings.total_distance), 3, 1, COLOR_WHITE, COLOR_GREY);
        } else {
            print_digit_text(total_distance_text, settings.total_distance, 4, 0, COLOR_WHITE, COLOR_GREY);
        }
        if (settings.trip_distance < 100000000) {
            print_digit_text(trip_distance_text, (settings.trip_distance), 3, 1, COLOR_WHITE, COLOR_GREY);
        } else {
            print_digit_text(trip_distance_text, settings.trip_distance, 4, 0, COLOR_WHITE, COLOR_GREY);
        }
#if UART_COMM == UART_COMM_VESC
        if (wh_left > 0 && distance_total > 0) {
            float usage = (wh_total / 10000.0f) / (distance_total / 1000.0f); // wh/m
            float wh_distance = (wh_left / 1000.0f) / usage; // distance in m
            int32_t wh_distance_i32_tmp = (int32_t) (wh_distance * 100);
            if (wh_distance_i32 == 0) wh_distance_i32 = wh_distance_i32_tmp;
            else wh_distance_i32 += (wh_distance_i32_tmp - wh_distance_i32) >> 5;
            if (distance_total < 100) {
                lv_label_set_text(distance_text, "--.-");
            } else {
                print_digit_text(distance_text, (int32_t) (wh_distance_i32), 3, 1, COLOR_WHITE, COLOR_GREY); 
            }
        } else {
            lv_label_set_text(distance_text, "--.-");
        }
#endif
        draw_distances_trigger = 0;
    }
}

void print_digit_text(lv_obj_t *object, uint32_t value, uint32_t length, uint32_t decimals, lv_color_t active_color, lv_color_t passive_color) { // string must be large enough to contain all data, check beforehand

    const uint32_t powers[6] = { 1, 10, 100, 1000, 10000, 100000 };
    int preval = 0;
    int cnt = 0;
    int decimal_value = 0;
    char string[64] = {0};
    lv_obj_set_style_text_color(object, active_color, LV_PART_MAIN);
    if (decimals) {
        decimal_value = (value % 100000) / 10000;
        value /= 100000;
    }
    if (value / powers[length-1] == 0) {
#if LVGL_VERSION_MAJOR == 8
        cnt += sprintf(string + cnt, "#%02X%02X%02X ", passive_color.ch.red << 3, passive_color.ch.green << 2, passive_color.ch.blue << 3);
#else
        cnt += sprintf(string + cnt, "#%02X%02X%02X ", passive_color.red, passive_color.green, passive_color.blue);
#endif
        preval = 1;
    }
    for (int i = length; i > 0; i--) {
        uint32_t tval = value / powers[i-1];
        value %= powers[i-1];
        if (tval != 0 && preval) {
            cnt += sprintf(&string[cnt], "#");
            preval = 0;
        }
        cnt += sprintf(&string[cnt], "%" LUI, tval);
    }
    if (decimals) cnt += sprintf(&string[cnt], ".");
    for (int i = decimals; i > 0; i--) {
        uint32_t tval = decimal_value / powers[i-1];
        decimal_value %= powers[i-1];
        if (preval) {
            cnt += sprintf(&string[cnt], "#");
            preval = 0;
        }
        cnt += sprintf(&string[cnt], "%" LID, tval);
    }
    lv_label_set_text_fmt(object, string); // fmt will copy
}

void print_time(lv_obj_t *object, uint32_t value) {
    char string[64];
    uint32_t hours = (value / 3600000);
    uint32_t minutes = (value - (hours * 3600000)) /  60000;
    uint32_t seconds = (value % 60000) / 1000;

    if (hours != 0) {
        sprintf(string, "%02" LID ":%02" LID, hours, minutes);
    } else {
        sprintf(string, "%02" LID ".%02" LID , minutes, seconds);
    }
    lv_label_set_text_fmt(object, string); 
}

static void _draw_brake(void) {
    if (draw_brake_trigger) {
        if (brake) lv_obj_clear_flag(brake_img, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(brake_img, LV_OBJ_FLAG_HIDDEN);
        draw_brake_trigger = 0;
    }
}

/**********************************************
 * Timers
 **********************************************/
static CRITICAL void _100ms_timer(lv_timer_t *timer) {
    ext_temperature += (ext_temp() - ext_temperature) >> TEMPERATURE_FILTER_SHIFT;
    int_temperature += (int_temp() - int_temperature) >> TEMPERATURE_FILTER_SHIFT;

    /* power_value = (battery_current * battery_voltage) / 1000000; */
    /* avg_speed += (speed - avg_speed) >> SPEED_FILTER_SHIFT; */
    /* avg_power += (power_value - avg_power) >> POWER_FILTER_SHIFT; */

    if (settings.battery_voltage_from_controller) {
        battery_voltage += (battery_voltage_controller - battery_voltage) >> VOLTAGE_FILTER_SHIFT;
    } else {
        battery_voltage += (voltage_ebat() - battery_voltage) >> VOLTAGE_FILTER_SHIFT;
    }

    settings.trip_time += HAL_GetTick() - timer_old;

    timer_old = HAL_GetTick();
}

static CRITICAL void _50ms_timer(lv_timer_t *timer) {
    power_value = (battery_current * battery_voltage) / 1000000;
    avg_speed += (speed - avg_speed) >> SPEED_FILTER_SHIFT;
    avg_power += (power_value - avg_power) >> POWER_FILTER_SHIFT;
}

static CRITICAL void _heartbeat_timer(lv_timer_t *timer) {
    led_green(!led_green_state());
}

static CRITICAL void _light_level_timer(lv_timer_t *timer) {
    if (NULL != light_sensitivity_label) {
        lv_label_set_text_fmt(light_sensitivity_label, "%" LID, light_level());
    }
}

static CRITICAL void _500ms_timer(lv_timer_t *timer) {
    // update bottom data
    draw_time_trigger = 1;
#ifdef DEBUG
    draw_distances_trigger = 1;
#endif
}


/***************************************************
 * Callbacks
 ***************************************************/
/* static void graph_event_pre_cb(lv_event_t * e) */
/* { */
/*     lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e); */
/*  */
/*     if(dsc->part == LV_PART_MAIN) { */
/*         if(dsc->line_dsc == NULL || dsc->p1 == NULL || dsc->p2 == NULL) return; */
/*  */
/*         if(dsc->p1->y == dsc->p2->y) { */
/*             dsc->line_dsc->color  = GRAPH_DIV_COLOR; */
/*         } */
/*     } if (dsc->type == LV_CHART_DRAW_PART_TICK_LABEL) { */
/*         sprintf(dsc->text, "%2" LID, dsc->value / 1000); // always smaller than actual text */
/*     } */
/* } */
#ifdef __LP64__
static void trip_reset_anim(void *none, int value) {
#else
static void trip_reset_anim(void *none, long int value) {
#endif
    if (value >= 100) {
        lv_obj_set_style_text_color(trip_distance_text, COLOR_GREY, 0);
        lv_obj_set_style_text_color(trip_time_text, COLOR_GREY, 0);
    } else {
        lv_obj_set_style_text_color(trip_distance_text, COLOR_WHITE, 0);
        lv_obj_set_style_text_color(trip_time_text, COLOR_WHITE, 0);
    }
}

static int event_item = 0;
static inline void mode_normal_buttons(uint8_t code) {
    switch (code & (BUTTON_MASK(BUTTON_ID_UP) | BUTTON_MASK(BUTTON_ID_DOWN))) {
        case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_UP, BUTTON_DOWN):
        case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_UP, BUTTON_PRESSED):
        case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_DOWN):
        case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_PRESSED):
        case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED):
            // one long press and other down
            LV_LOG_INFO("Moving to settings");
            button_release(BUTTON_ID_UP, 500);
            button_release(BUTTON_ID_DOWN, 500);
            mode = MODE_SETTINGS_MAIN;
            // clean all
            lv_obj_clean(lv_screen_active());
            
            gui_draw_settings_main();
            
            break;
        case BUTTON_STATE(BUTTON_ID_UP, BUTTON_PRESSED):
            LV_LOG_INFO("Up");
            if (settings.assist_last < settings.assist_levels) settings.assist_last++;
            button_release(BUTTON_ID_UP, 0);
#if defined(CAN_ENABLED)
            can_vesc_set_current_scale_max();
#else
            comm_send_display_settings();
#endif
            draw_assist_trigger = 1;
            break;
        case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED):
            LV_LOG_INFO("Up long");
            settings.lights_mode = LIGHTS_MODE_AUTOMATIC;
            button_release(BUTTON_ID_UP, 500);
            comm_send_lights();
            draw_lights_trigger = 1;
            break;
        case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_PRESSED):
            LV_LOG_INFO("Down");
            if (settings.assist_last > 0) settings.assist_last--;
            draw_assist_trigger = 1;
#if defined(CAN_ENABLED)
            can_vesc_set_current_scale_max();
#else
            comm_send_display_settings();
#endif
            button_release(BUTTON_ID_DOWN, 0);
            break;
        case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED):
            LV_LOG_INFO("Down long");
            settings.lights_mode = LIGHTS_MODE_MANUAL;
            if (!settings.lights_enabled) settings.lights_enabled = 1;
            else settings.lights_enabled = 0;
            button_release(BUTTON_ID_DOWN, 500);
            comm_send_lights();
            draw_lights_trigger = 1;
            break;
        /* default: */
        /*     button_release(BUTTON_ID_UP, 0); */
        /*     button_release(BUTTON_ID_DOWN, 0); */
        /*     break; */
    }

    switch (code & (BUTTON_MASK(BUTTON_ID_POWER))) {
        case BUTTON_STATE(BUTTON_ID_POWER, BUTTON_PRESSED):
            LV_LOG_INFO("Power");
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
            LV_LOG_INFO("Power long");
            // shutdown!
            if (HAL_GetTick() > 2150) {
                // avoid shutdown/start loops
                eeprom_write_settings();
                lcd_backlight(0);
                power_disable();
                //
            }
            break;
    }

    switch (code & (BUTTON_MASK(BUTTON_ID_NC))) {
        case BUTTON_STATE(BUTTON_ID_NC, BUTTON_LONG_PRESSED):
            LV_LOG_INFO("M long");
            button_release(BUTTON_ID_NC, 500);
            lv_obj_clean(lv_screen_active());
            mode = MODE_STATUS;
            gui_draw_status();
            break;
        case BUTTON_STATE(BUTTON_ID_NC, BUTTON_PRESSED):
            LV_LOG_INFO("M short");
            if (settings.graph_field == GRAPH_FIELD_SPEED_POWER_AVG) settings.graph_field = GRAPH_FIELD_SHIFT;
            else settings.graph_field = GRAPH_FIELD_SPEED_POWER_AVG;
            _draw_graph_switch();
            button_release(BUTTON_ID_NC, 500);
            break;
    }
}

static inline void mode_status_buttons(uint8_t code) {
    switch (code & (BUTTON_MASK(BUTTON_ID_POWER) | BUTTON_MASK(BUTTON_ID_NC))) {
        case BUTTON_STATE(BUTTON_ID_NC, BUTTON_PRESSED):
        case BUTTON_STATE(BUTTON_ID_NC, BUTTON_LONG_PRESSED):
        case BUTTON_STATE(BUTTON_ID_POWER, BUTTON_PRESSED):
        case BUTTON_STATE(BUTTON_ID_POWER, BUTTON_LONG_PRESSED):
            LV_LOG_INFO("Button M");
            button_release(BUTTON_ID_NC, 250);
            button_release(BUTTON_ID_POWER, 250);
            mode = MODE_NORMAL;
            status_page_active = STATUS_PAGE_NONE;

            if (NULL != status_page_timer) {
                lv_timer_delete(status_page_timer);
                status_page_timer = NULL;
            }
            lv_obj_clean(lv_screen_active()); 
            gui_draw_normal();

            draw_lights_trigger = 1;
            draw_battery_voltage_trigger = 1;
            draw_speed_trigger = 1;
            draw_power_trigger = 1;
            draw_distances_trigger = 1;
            break;
        default:
            /* LV_LOG_INFO("Invalid button combination: %02X %02X %02X", code, BUTTON_MASK(BUTTON_ID_POWER) | BUTTON_MASK(BUTTON_ID_NC), BUTTON_STATE(BUTTON_ID_NC, BUTTON_PRESSED)); */
            break;
    }
    switch (code & (BUTTON_MASK(BUTTON_ID_UP) | BUTTON_MASK(BUTTON_ID_DOWN))) {
        case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_PRESSED):
            LV_LOG_INFO("Button DOWN");
            button_release(BUTTON_ID_DOWN, 500);
            if (status_page < STATUS_PAGE_MAX) {
                status_page++;
                lv_obj_clean(lv_screen_active());
                gui_draw_status();
            }
            break;

        case BUTTON_STATE(BUTTON_ID_UP, BUTTON_PRESSED):
            LV_LOG_INFO("Button UP");
            button_release(BUTTON_ID_UP, 500);
            if (status_page > 0) {
                status_page--;
                lv_obj_clean(lv_screen_active());
                gui_draw_status();
            }
            break;
        case BUTTON_STATE(BUTTON_ID_UP, BUTTON_DOWN):
        case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_DOWN):
        case BUTTON_STATE(BUTTON_ID_NC, BUTTON_DOWN):
        case BUTTON_STATE(BUTTON_ID_POWER, BUTTON_DOWN):
        case 0:
            break;
        default:
            LV_LOG_INFO("Invalid button combination: %08X", code);
            break;
    }
}

static inline void mode_settings_main_buttons(uint8_t code) {

    switch (code & (BUTTON_MASK(BUTTON_ID_UP) | BUTTON_MASK(BUTTON_ID_DOWN) | BUTTON_MASK(BUTTON_ID_POWER) | BUTTON_MASK(BUTTON_ID_NC))) {
        case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_UP, BUTTON_DOWN):
        case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_DOWN):
        case BUTTON_STATE(BUTTON_ID_UP, BUTTON_LONG_PRESSED) | BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_LONG_PRESSED):
            break; // ignore
        case BUTTON_STATE(BUTTON_ID_UP, BUTTON_PRESSED):
            LIST_ITEM_NORMAL(list_item);
            list_item--;
            if (list_item < 0) list_item = list_item_max - 1;
            LIST_ITEM_HIGHLIGHT(list_item);
            button_release(BUTTON_ID_UP, 250);
            break;
        case BUTTON_STATE(BUTTON_ID_DOWN, BUTTON_PRESSED):
            LIST_ITEM_NORMAL(list_item);
            list_item++;
            list_item = ((uint32_t) list_item) % list_item_max;
            LIST_ITEM_HIGHLIGHT(list_item);
            button_release(BUTTON_ID_DOWN, 250);
            break;
        case BUTTON_STATE(BUTTON_ID_POWER, BUTTON_PRESSED):
        case BUTTON_STATE(BUTTON_ID_NC, BUTTON_PRESSED):
            button_release(BUTTON_ID_POWER, 250);
            button_release(BUTTON_ID_NC, 250);
            switch(list_item) {
                case 0: // goto display settings
                    mode = MODE_SETTINGS_DISPLAY;
                    lv_obj_clean(lv_screen_active());
                    
                    gui_draw_settings_display();
                    
                    break;
                case 1: // goto controller settings
                    mode = MODE_SETTINGS_CONTROLLER;
                    lv_obj_clean(lv_screen_active());
                    gui_draw_settings_controller();
                    break;
                case 2:
                    eeprom_factory_reset();
                    lv_obj_clean(lv_screen_active());
                    gui_draw_settings_main();
                    break;
                case 3: // goto clock settings
#if LEXT_INSTALLED
                    mode = MODE_SETTINGS_CLOCK;
                    lv_obj_clean(lv_screen_active());
                    gui_draw_time();
                    break;
                case 6:
#endif
                case 5:
                case 4:
                    mode_back = MODE_SETTINGS_MAIN;
                    mode = MODE_SETTINGS_EVENT_CALLBACK;
                    LIST_ITEM_SELECT(list_item); 
                    break;
#if LEXT_INSTALLED 
                case 7:
#else
                case 6: // back
#endif
                    mode = MODE_NORMAL;
                    // clean all
                    lv_obj_clean(lv_screen_active());
                    gui_draw_normal();
                    settings.factory_reset = 0x55; 
                    // save settings
                    eeprom_write_settings();
                    // send settings to controller
                    comm_send_display_status();
                    comm_send_controller_settings();

                    draw_lights_trigger = 1;
                    draw_battery_voltage_trigger = 1;
                    draw_speed_trigger = 1;
                    draw_power_trigger = 1;
                    draw_distances_trigger = 1;
                    break;
            }
            break;
    }
}

static inline void mode_settings_display_buttons(uint8_t code) {
    switch (code & (BUTTON_MASK(BUTTON_ID_UP) | BUTTON_MASK(BUTTON_ID_DOWN) | BUTTON_MASK(BUTTON_ID_POWER) | BUTTON_MASK(BUTTON_ID_NC))) {
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
        case BUTTON_STATE(BUTTON_ID_NC, BUTTON_PRESSED):
            button_release(BUTTON_ID_POWER, 150);
            button_release(BUTTON_ID_NC, 150);
            switch(list_item) {
                case 11:
                    event_item = 1;
                    lv_event_send(list_items[list_item], LV_EVENT_GESTURE, &event_item);
                    break;
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
                case 12:
                    mode_back = MODE_SETTINGS_DISPLAY;
                    mode = MODE_SETTINGS_EVENT_CALLBACK;
                    LIST_ITEM_SELECT(list_item); 
                    break;
                case 13: // back
                    mode = MODE_SETTINGS_MAIN;
                    // clean all
                    lv_obj_clean(lv_screen_active());
                    
                    gui_draw_settings_main();
                    
                    break;
            }
            break;
    }
}

static inline void mode_settings_controller_buttons(uint8_t code) {
    switch (code & (BUTTON_MASK(BUTTON_ID_UP) | BUTTON_MASK(BUTTON_ID_DOWN) | BUTTON_MASK(BUTTON_ID_POWER) | BUTTON_MASK(BUTTON_ID_NC))) {
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
        case BUTTON_STATE(BUTTON_ID_NC, BUTTON_PRESSED):
            button_release(BUTTON_ID_POWER, 150);
            button_release(BUTTON_ID_NC, 150);
            switch(list_item) {
                case 8: // back
                    mode = MODE_SETTINGS_MAIN;
                    // clean all
                    lv_obj_clean(lv_screen_active());
                    
                    gui_draw_settings_main();
                    
                    break;
                default:
                    mode_back = MODE_SETTINGS_CONTROLLER;
                    mode = MODE_SETTINGS_EVENT_CALLBACK;
                    LIST_ITEM_SELECT(list_item); 
                    break;
            }
            break;
    }
}

static inline void mode_settings_time_buttons(uint8_t code) {
    switch (code & (BUTTON_MASK(BUTTON_ID_UP) | BUTTON_MASK(BUTTON_ID_DOWN) | BUTTON_MASK(BUTTON_ID_POWER) | BUTTON_MASK(BUTTON_ID_NC))) {
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
        case BUTTON_STATE(BUTTON_ID_NC, BUTTON_PRESSED):
            button_release(BUTTON_ID_POWER, 150);
            button_release(BUTTON_ID_NC, 150);
            switch(list_item) {
                default:
                    mode_back = MODE_SETTINGS_CLOCK;
                    mode = MODE_SETTINGS_EVENT_CALLBACK;
                    LIST_ITEM_SELECT(list_item); 
                    break;
                case 6: // back
                    mode = MODE_SETTINGS_MAIN;
                    // clean all
                    lv_obj_clean(lv_screen_active());
                    // set time
                    clock_set_date((uint32_t) year - 2000, (uint32_t) month, (uint32_t) day, 0);
                    clock_set_time((uint32_t) hour, (uint32_t) min, (uint32_t) sec);
                    
                    gui_draw_settings_main();
                    
                    break;
            }
            break;
    }
}

static inline void mode_event_callback_buttons(uint8_t code) {
    switch (code & (BUTTON_MASK(BUTTON_ID_UP) | BUTTON_MASK(BUTTON_ID_DOWN) | BUTTON_MASK(BUTTON_ID_POWER) | BUTTON_MASK(BUTTON_ID_NC))) {
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
        case BUTTON_STATE(BUTTON_ID_NC, BUTTON_PRESSED):
            button_release(BUTTON_ID_NC, 1500);
            button_release(BUTTON_ID_POWER, 150);
            mode = mode_back;
            LIST_ITEM_HIGHLIGHT(list_item);
            break;
    }
}

static inline void mode_trip_reset_buttons(uint8_t code) {
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
}

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
CRITICAL void button_presses(void) {
    uint8_t code = buttons_pressed(); 
    /* LV_LOG_INFO("MODE: %d, code: %d", mode, code); */
    switch (mode) {
        case MODE_NORMAL:
            // check first if two buttons pressed
            mode_normal_buttons(code);
            break;
        case MODE_STATUS:
            mode_status_buttons(code);
            break;
        case MODE_TRIP_RESET:
            mode_trip_reset_buttons(code);
            break;
        case MODE_SETTINGS_EVENT_CALLBACK:
            mode_event_callback_buttons(code);
            break;
        case MODE_SETTINGS_MAIN:
            mode_settings_main_buttons(code);
            break;
        case MODE_SETTINGS_DISPLAY:
            mode_settings_display_buttons(code);
            break;
        case MODE_SETTINGS_CONTROLLER:
            mode_settings_controller_buttons(code);
            break;
        case MODE_SETTINGS_CLOCK:
#if LEXT_INSTALLED
            mode_settings_time_buttons(code);
#endif
            break;
    }
}

CRITICAL void gui_increment_trip(void) {
        settings.trip_distance += settings.wheel_circumfence;
        settings.total_distance += settings.wheel_circumfence;
}

CRITICAL void _draw_notification(void) {
    if (HAL_GetTick() - notification_timeout > settings.notification_timeout) {
        lv_obj_add_flag(notification, LV_OBJ_FLAG_HIDDEN);
    }
}

CRITICAL void gui_write_bt_msg(char *msg, int length) {
    if (strlen(msg) == 0 || length == 0) return;
    LV_LOG_INFO("New bt msg: %s", msg);
    lv_obj_clear_flag(notification, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_height(notification, LV_SIZE_CONTENT);
    lv_label_set_text_fmt(notification, "%s", msg);
    lv_obj_update_layout(notification);
    int height = lv_obj_get_height(notification);
    /* lv_obj_set_height(notification, height < NOTIFICATION_HEIGHT ? height : NOTIFICATION_HEIGHT);  */
    lv_obj_set_y(notification, height <= NOTIFICATION_HEIGHT ? NOTIFICATION_Y + ((NOTIFICATION_HEIGHT - height) / 2) : NOTIFICATION_Y);
    notification_timeout = HAL_GetTick();
}

CRITICAL void gui_write_bt_distance(int32_t distance) {
    if (distance < 1000) {
        lv_label_set_text_fmt(nav_distance, "%ldm", distance);
    } else {
        lv_label_set_text_fmt(nav_distance, "%ld.%1ldkm", distance / 1000, distance % 1000);
    }
}
CRITICAL void gui_write_bt_time(int32_t time) {
    if (time < 3600) {
        lv_label_set_text_fmt(nav_time, "%ldm", time / 60);
    } else {
        lv_label_set_text_fmt(nav_time, "%ld:%02ld", time / 3600, (time % 3600) / 60);
    }
}

CRITICAL void gui_write_bt_img(lv_image_dsc_t *img) {
    lv_image_cache_drop(NULL);
    lv_image_set_src(nav_img, img);
    lv_obj_set_style_img_recolor(nav_img, lv_color_white(), LV_PART_MAIN);
    LV_LOG_INFO("Update img");
}

