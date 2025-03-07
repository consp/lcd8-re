#ifndef UGUI_SIM_H_
#define UGUI_SIM_H_

#include "ugui.h"

typedef struct
{
    int width;
    int height;
    int screenMultiplier;
    int screenMargin;
    uint32_t windowBackColor;
} simcfg_t;

#ifdef X_PROTOCOL
//Data Defines
typedef struct x11_data_s
{
    Display *dis;
    Window win;
    GC gc;
    Visual *visual;
    int screen;
    uint32_t *imgBuffer;
    int simX;
    int simY;
} x11_data_t;
#endif

simcfg_t* GUI_SimCfg(void);

void GUI_Setup(UG_DEVICE *device);

void GUI_Process(void);

#endif // UGUI_SIM_H_
