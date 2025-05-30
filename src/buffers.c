#include "config.h"
#include "eeprom.h"
#include "comm.h"
#include "gui.h"

error_state error = 0;
settings_t settings = {0};

uint8_t draw_time_trigger = 0;
uint8_t draw_distances_trigger = 0;
uint8_t draw_power_trigger = 0;
uint8_t draw_temperatures_trigger = 0;
uint8_t draw_temperatures_trigger_internal = 0;
uint8_t draw_speed_trigger = 0;
uint8_t draw_assist_trigger = 0;
uint8_t draw_battery_voltage_trigger = 0;
uint8_t draw_lights_trigger = 0;
uint8_t draw_brake_trigger = 0;
uint8_t draw_controller_mode_trigger = 0;

vesc_mcconf_temp mcconf;
volatile int mconf_actual = 0;
volatile int mconf_updated = 0; 

#if defined(STM32H743)
#if (!defined(DIRECT))
uint8_t RAM_D2  pixelbuffer[PIXEL_BUFFER_SIZE] __attribute__ ((aligned (4)));
uint8_t RAM_D2  pixelbuffer2[PIXEL_BUFFER_SIZE] __attribute__ ((aligned (4)));
#endif
#if defined(DIRECT) && defined(PARTIAL)
uint8_t RAMHIGH pixelbuffer[DISPLAY_WIDTH * 512 * COLOR_SIZE / 2] __attribute__ ((aligned (8)));
uint8_t RAMHIGH pixelbuffer2[DISPLAY_WIDTH * 512 * COLOR_SIZE / 2] __attribute__ ((aligned (8)));
#else
uint8_t RAMHIGH framebuffer[(COLOR_SIZE * DISPLAY_HEIGHT * DISPLAY_WIDTH)] __attribute__ ((aligned (4)));
#endif
/* uint8_t *pixelbuffer2 = NULL; */
#else
uint8_t RAMHIGH pixelbuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(lv_color_t)] __attribute__ ((aligned(8)));
#endif

#ifdef MEMPOOL
uint8_t MEMPOOL_LOC mempool[MEMPOOL];
#endif
