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

void gui_init();
void gui_update(void);

#endif // __GUI_H__
