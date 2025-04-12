#ifndef __GUI_H__
#define __GUI_H__

#include "config.h"
#include "ugui/ugui_config.h"
#include "ugui/ugui.h"
#include "ugui/ugui_fonts.h"

#ifndef SIM
#include "lcd.h"
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#include "ugui/ugui_sim.h"
#endif

#define RGB565(x, y, z)     (lv_color_t) { .full = (((x & 0b11111000) << 8) | ((y & 0b11111100) << 3) | (z >> 3)) }
        //lv_color_make(r, g, b) 

#define COLOR_WHITE         RGB565(255, 255, 255)
#define COLOR_BLACK         RGB565(0, 0, 0)
#define COLOR_GREY          RGB565(100, 100, 100)
#define COLOR_DARK_GREY     RGB565(77, 77, 77)
#define COLOR_YELLOW        (lv_color_t) { .full = 0xffe0 }
#define COLOR_BLUE          RGB565(0, 0, 160)
//RGB565(0xFE, 0xFF, 0)

/* locations
 */
#define SPEED_MAJOR_X       0 
#define SPEED_MAJOR_Y       96 // create some space
#define SPEED_MAJOR_WIDTH   92 
#define SPEED_MAJOR_HEIGHT  148
// #define SPEED_MAJOR_BG_COLOR C_BLACK //C_YELLOW// C_BLACK
// #define SPEED_MAJOR_FG_COLOR C_WHITE
#define SPEED_MINOR_WIDTH   26
#define SPEED_MINOR_HEIGHT  72
#define SPEED_MINOR_X       SPEED_MAJOR_X + (SPEED_MAJOR_WIDTH * 2) + 14
#define SPEED_MINOR_Y       (SPEED_MAJOR_HEIGHT + SPEED_MAJOR_Y) - SPEED_MINOR_HEIGHT
#define SPEED_MINOR_BG_COLOR C_BLACK //C_BROWN// C_BLACK
#define SPEED_MINOR_FG_COLOR C_WHITE // C_WHITE
#define SPEED_COLOR_FALSE   0x632c
#define SPEED_COLOR_TRUE    0xFFFF

#define BRAKE_IMG_X         SPEED_MINOR_X - 8
#define BRAKE_IMG_Y         SPEED_MAJOR_Y + 16

#define BATTERY_BAR_X           6  
#define BATTERY_BAR_Y           5
#define BATTERY_BAR_WIDTH       76
#define BATTERY_BAR_HEIGHT      21
#define BATTERY_LABEL_X         96
#define BATTERY_LABEL_Y         0
#define BATTERY_LABEL_WIDTH     96
#define BATTERY_LABEL_HEIGHT    32
#define BATTERY_COLOR_10        RGB565(224, 0, 0)
#define BATTERY_COLOR_20        RGB565(225, 74, 0)
#define BATTERY_COLOR_30        RGB565(255, 128, 0)
#define BATTERY_COLOR_40        RGB565(220, 180, 0)
#define BATTERY_COLOR_50        RGB565(164, 217, 0)
#define BATTERY_COLOR_60        RGB565(0  , 181, 0)
#define BATTERY_COLOR_70        RGB565(0  , 216, 0)
#define BATTERY_COLOR_80        RGB565(0  , 234, 0)
#define BATTERY_COLOR_90        RGB565(0  , 255, 0)
#define BATTERY_COLOR_100       RGB565(108, 255, 108)

#define LIGHTS_IMG_X            ((5 * 32) + 16)
#define LIGHTS_IMG_Y            0

#define TEMP_IMG_X              ((6 * 32) + 16)
#define TEMP_IMG_Y              0
#define TEMP_TEXT_X             TEMP_IMG_X + 32
#define TEMP_TEXT_Y             TEMP_IMG_Y
#define TEMP_TEXT_WIDTH         80
#define TEMP_TEXT_HEIGHT        32

#define MOTOR_TEMP_IMG_X        TEMP_IMG_X
#define MOTOR_TEMP_IMG_Y        TEMP_IMG_Y + 32
#define MOTOR_TEMP_TEXT_X       MOTOR_TEMP_IMG_X + 32
#define MOTOR_TEMP_TEXT_Y       MOTOR_TEMP_IMG_Y
#define MOTOR_TEMP_TEXT_WIDTH   80
#define MOTOR_TEMP_TEXT_HEIGHT  32
//#define RGB565(r, g, b) (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3))

#define POWER_X                 0
#define POWER_Y                 (7 * 32) + 24
#define POWER_HEIGHT            32
#define POWER_BORDER            1
#define POWER_REGEN_WIDTH       (6*8)
#define POWER_BAR_WIDTH         (2)
#define POWER_NORMAL_WIDTH      (24*8)
#define POWER_RED_WIDTH         2
#define POWER_COLOR_BAR         RGB565(255, 151, 0)
#define POWER_COLOR_BORDER      RGB565(77, 77, 77)
#define POWER_COLOR_BORDER_DARK RGB565(40, 40, 40)
#define POWER_COLOR_REGEN       RGB565(0, 255, 0)
#define POWER_COLOR_REGEN_DARK  RGB565(0, 48, 0)
#define POWER_COLOR_NORMAL      RGB565(0, 181, 0)
#define POWER_COLOR_NORMAL_DARK RGB565(0, 32, 0)
#define POWER_COLOR_RED         RGB565(210, 0, 0)
#define POWER_COLOR_BG          RGB565(77, 77, 77)
#define POWER_TEXT_X            (POWER_REGEN_WIDTH + POWER_BAR_WIDTH + POWER_NORMAL_WIDTH)
#define POWER_TEXT_Y            POWER_TOP + 2
#define POWER_TEXT_WIDTH        (DISPLAY_WIDTH - (POWER_REGEN_WIDTH + POWER_BAR_WIDTH + POWER_NORMAL_WIDTH) - 1)
#define POWER_TEXT_HEIGHT       (POWER_HEIGHT - 4)
#define POWER_TEXT_BG_COLOR     POWER_COLOR_BG
#define POWER_TEXT_FG_COLOR     RGB565(255, 255, 255) 
#define POWER_REGEN_INDEX       (POWER_MIN / POWER_REGEN_WIDTH)

#define TOTAL_DISTANCE_IMG_X            0
#define TOTAL_DISTANCE_IMG_Y            (13*32) 
#define TOTAL_DISTANCE_TEXT_X           TOTAL_DISTANCE_IMG_X + 32
#define TOTAL_DISTANCE_TEXT_Y           TOTAL_DISTANCE_IMG_Y
#define TOTAL_DISTANCE_TEXT_WIDTH       96
#define TOTAL_DISTANCE_TEXT_HEIGHT      32

#define TRIP_DISTANCE_IMG_X             0
#define TRIP_DISTANCE_IMG_Y             (14 * 32)
#define TRIP_DISTANCE_TEXT_X            TRIP_DISTANCE_IMG_X + 32
#define TRIP_DISTANCE_TEXT_Y            TRIP_DISTANCE_IMG_Y
#define TRIP_DISTANCE_TEXT_WIDTH        96    
#define TRIP_DISTANCE_TEXT_HEIGHT       32

#define TRIP_TIME_IMG_X                 (5 * 32)
#define TRIP_TIME_IMG_Y                 (14 * 32)
#define TRIP_TIME_TEXT_X                TRIP_TIME_IMG_X + 32
#define TRIP_TIME_TEXT_Y                TRIP_TIME_IMG_Y
#define TRIP_TIME_TEXT_WIDTH            96    
#define TRIP_TIME_TEXT_HEIGHT           32

// #define TIME_IMG_X                      (5 * 32)
// #define TIME_IMG_Y                      (14 * 32)
#define TIME_TEXT_X                     (1 * 32)//TIME_IMG_X + 32
#define TIME_TEXT_Y                     (1 * 32) //TIME_IMG_Y
#define TIME_TEXT_WIDTH                 96    
#define TIME_TEXT_HEIGHT                32

#define GRAPH_X                         0 
#define GRAPH_Y                         POWER_Y + 32
#define GRAPH_WIDTH                     (DISPLAY_WIDTH - (GRAPH_X))
#define GRAPH_HEIGHT                    (4 * 32)
#define GRAPH_BORDER_COLOR              RGB565(100, 100, 100)
#define GRAPH_DIV_COLOR                 RGB565(60, 60, 60)
#define GRAPH_LINE_COLOR                RGB565(144, 144, 144)
#define GRAPH_LEGEND_COLOR              RGB565(122, 122, 122)
#define GRAPH_CURSOR_COLOR              RGB565(0, 0, 144)
#define GRAPH_POINT_COUNT               256

#define ASSIST_X                        256
#define ASSIST_Y                        96
#define ASSIST_WIDTH                    64
#define ASSIST_HEIGHT                   96 + 16

#define CM_LABEL_X                      320-32
#define CM_LABEL_Y                      GRAPH_Y + GRAPH_HEIGHT
#define CM_LABEL_WIDTH                  32
#define CM_LABEL_HEIGHT                 32


#define DEBUG_X                 0
#define DEBUG_Y                 DISPLAY_HEIGHT - 16
#define DEBUG_WIDTH             320
#define DEBUG_HEIGHT            16

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

void gui_init();
void gui_update(void);
void button_presses(void);


// images
extern const lv_img_dsc_t large_0;
extern const lv_img_dsc_t large_1;
extern const lv_img_dsc_t large_2;
extern const lv_img_dsc_t large_3;
extern const lv_img_dsc_t large_4;
extern const lv_img_dsc_t large_5;
extern const lv_img_dsc_t large_6;
extern const lv_img_dsc_t large_7;
extern const lv_img_dsc_t large_8;
extern const lv_img_dsc_t large_9;

// 9 to 8 translations
#if LVGL_VERSION_MAJOR == 8
#define lv_screen_active            lv_scr_act
#define lv_image_create             lv_img_create
#define lv_image_set_src            lv_img_set_src
#define lv_image_dsc_t              lv_img_dsc_t
#define lv_point_precise_t          lv_point_t 
#endif
#if LVGL_VERSION_MAJOR == 9
#define LV_IMG_CF_INDEXED_4BIT      LV_COLOR_FORMAT_I4
#endif

uint32_t timer_cb(void);

typedef enum {
    MODE_NORMAL = 0,
    MODE_TRIP_RESET,
    MODE_SETTINGS_MAIN,
    MODE_SETTINGS_CONTROLLER,
    MODE_SETTINGS_DISPLAY,
    MODE_SETTINGS_CLOCK,
    MODE_SETTINGS_EVENT_CALLBACK
} modes;

typedef enum {
    LIGHTS_MODE_MANUAL = 0,
    LIGHTS_MODE_AUTOMATIC
} lights_mode;

void gui_increment_trip(void);

#endif // __GUI_H__
