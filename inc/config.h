#ifndef __CONFIG_H__
#define __CONFIG_H__


#define DISPLAY_WIDTH   320
#define DISPLAY_HEIGHT  480

#define FILTER_SHIFT 3

#define PIXEL_BUFFER_LINES 32 // preferably 32 or higher, this save tearing to the graph and nothing else
#define PIXEL_BUFFER_SIZE (PIXEL_BUFFER_LINES * 2 * DISPLAY_WIDTH)

#define REFRESH_INTERVAL 33000 // refresh interval in us

#define EEPROM_SIZE 4096

#define POWER_MAX 250
#define POWER_MIN -250

#define DEBUG

#endif // __CONFIG_H__
