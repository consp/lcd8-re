#include "gui.h"

static UG_GUI ugui;
static UG_DEVICE device = {
    .x_dim = DISPLAY_WIDTH,
    .y_dim = DISPLAY_HEIGHT,
#ifndef SIM
    .pset = lcd_draw,
    .flush = lcd_update,
#endif
};

#define WINDOW_MAIN_OBJECTS 16

UG_WINDOW window_main;
UG_OBJECT window_main_objects[WINDOW_MAIN_OBJECTS] = {0};
UG_PROGRESS window_main_battery_progress = {0};

UG_TEXTBOX txt4;
UG_TEXTBOX txt1;
UG_TEXTBOX txt2;
UG_TEXTBOX txt3;
#ifdef SIM
x11_data_t *handle;
simcfg_t *simCfg;

void GUI_Setup(UG_DEVICE *device);
void x11_pset(UG_S16 x, UG_S16 y, UG_COLOR c);
void x11_flush(void);
bool x11_setup(int width, int height);
void x11_process();
#define WINDOW_BACK_COLOR C_BLACK 
#define INITIAL_MARGIN  3
#define BTN_WIDTH       100
#define BTN_HEIGHT      30
#define CHB_WIDTH       100
#define CHB_HEIGHT      14

#define OBJ_Y(i)        BTN_HEIGHT*i+(INITIAL_MARGIN*(i+1))

UG_BUTTON btn0;
UG_BUTTON btn1;
UG_BUTTON btn2;
UG_BUTTON btn3;
UG_CHECKBOX chb0;
UG_CHECKBOX chb1;
UG_CHECKBOX chb2;
UG_CHECKBOX chb3;
#endif

// local functions
void _draw_speed(void);


void gui_init(void) {
#ifdef SIM

    /* signal(SIGSEGV, handler);   // install our handler */
    /* signal(SIGBUS, handler);   // install our handler */

    printf("LCD Simulator\n");

    // Get config
    simCfg = (simcfg_t *)malloc(sizeof(simcfg_t));
    simCfg->width = DISPLAY_WIDTH;
    simCfg->height = DISPLAY_HEIGHT;
    simCfg->screenMultiplier = 2;
    simCfg->screenMargin = 10;
    simCfg->windowBackColor = WINDOW_BACK_COLOR;

    //Setup X
    if (true != x11_setup(simCfg->width, simCfg->height))
    {
        printf("Error Initializing X11 driver\n");
    }
    //Setup UGUI
    device.x_dim = simCfg->width;
    device.y_dim = simCfg->height;
    device.pset = &x11_pset;
    device.flush = &x11_flush;

    UG_Init(&ugui, &device);
#else

    UG_DriverRegister(DRIVER_DRAW_LINE, lcd_draw_line);
    UG_DriverRegister(DRIVER_FILL_FRAME, lcd_fill);
    UG_DriverRegister(DRIVER_FILL_AREA, lcd_fill_area);
    UG_DriverRegister(DRIVER_DRAW_BMP, lcd_draw_bmp);


#endif

    UG_FillScreen(C_BLACK);

    UG_FontSetHSpace( 2 );
    UG_FontSetVSpace( 2 );
    UG_SetForecolor( C_WHITE );
    UG_SetBackcolor( C_BLACK );

    //Setup Window
    UG_WindowCreate(&window_main, window_main_objects, WINDOW_MAIN_OBJECTS, NULL);
    UG_WindowSetForeColor(&window_main, C_WHITE);
    UG_WindowSetBackColor(&window_main, C_BLACK);
    UG_WindowSetStyle(&window_main, WND_STYLE_HIDE_TITLE);


    UG_ProgressCreate(&window_main, &window_main_battery_progress, PGB_ID_1, UGUI_POS(0, 0, 120, 10));
    UG_ProgressSetStyle(&window_main, PGB_ID_1, PGB_STYLE_2D | PGB_STYLE_FORE_COLOR_MESH);
    UG_ProgressSetProgress(&window_main, PGB_ID_1, 0);
    UG_ProgressSetForeColor(&window_main, PGB_ID_1, C_GREEN );
    UG_ProgressSetBackColor(&window_main, PGB_ID_1, C_BLACK );

    UG_TextboxCreate(&window_main, &txt1, TXB_ID_0, UGUI_POS(10, 30, 200, 30));
    UG_TextboxSetFont(&window_main, TXB_ID_0, FONT_4X6);
    UG_TextboxSetText(&window_main, TXB_ID_0, "Abc123");
    UG_TextboxSetAlignment(&window_main, TXB_ID_0, ALIGN_BOTTOM_LEFT);
    UG_TextboxCreate(&window_main, &txt2, TXB_ID_1, UGUI_POS(10, 60, 200, 30));
    UG_TextboxSetFont(&window_main, TXB_ID_1, FONT_ANDALEMO_6X10_MONO);
    UG_TextboxSetText(&window_main, TXB_ID_1, "Abc123");
    UG_TextboxSetAlignment(&window_main, TXB_ID_1, ALIGN_BOTTOM_LEFT);
    UG_TextboxCreate(&window_main, &txt3, TXB_ID_2, UGUI_POS(10, 90, 200, 30));
    UG_TextboxSetFont(&window_main, TXB_ID_2, FONT_ANDALEMO_4X7);
    UG_TextboxSetText(&window_main, TXB_ID_2, "Abc123");
    /* UG_TextboxSetAlignment(&window_main, TXB_ID_2, ALIGN_BOTTOM_LEFT); */
    /* UG_TextboxCreate(&window_main, &txt4, TXB_ID_3, UGUI_POS(10, 120, 200, 30)); */
    /* UG_TextboxSetFont(&window_main, TXB_ID_3, FONT_ANDALEMO_5X8); */
    /* UG_TextboxSetText(&window_main, TXB_ID_3, "Abc123"); */
    UG_TextboxSetAlignment(&window_main, TXB_ID_3, ALIGN_BOTTOM_LEFT);
    UG_TextboxSetForeColor(&window_main, TXB_ID_0, C_BLACK);
    UG_TextboxSetForeColor(&window_main, TXB_ID_1, C_BLACK);
    UG_TextboxSetForeColor(&window_main, TXB_ID_2, C_BLACK);
    UG_TextboxSetForeColor(&window_main, TXB_ID_3, C_BLACK);
    UG_TextboxSetBackColor(&window_main, TXB_ID_0, C_AQUA);
    UG_TextboxSetBackColor(&window_main, TXB_ID_1, C_DARK_CYAN);
    UG_TextboxSetBackColor(&window_main, TXB_ID_2, C_DARK_RED);
    UG_TextboxSetBackColor(&window_main, TXB_ID_3, C_DARK_GREEN);


    UG_WindowShow(&window_main);
    
    UG_Update();

}

typedef enum {
    WINDOW_MAIN,
    WINDOW_SETUP
} Windows;

Windows current_window = WINDOW_MAIN;

uint32_t redraw_speed, 
         redraw_temp_int, 
         redraw_temp_ext = 0;

void gui_update(void) {
    switch (current_window) {
        case WINDOW_MAIN:
            if (redraw_speed) {
                _draw_speed();
            }
            _draw_battery();
            break;
        case WINDOW_SETUP:
            break;
    }
    UG_Update();
#ifdef SIM
    x11_process();
#endif
}

void _draw_speed(void) {
    
    /* UG_TextboxCreate(&window_main, &txt3, TXB_ID_3, UGUI_POS(INITIAL_MARGIN*3+BTN_WIDTH+CHB_WIDTH, OBJ_Y(3), 100, 53)); */
    /* UG_TextboxSetFont(&window_main, TXB_ID_3, FONT_32X53); */
    /* UG_TextboxSetText(&window_main, TXB_ID_3, "ABC"); */
    /* #if !defined(UGUI_USE_COLOR_BW) */
    /* UG_TextboxSetBackColor(&window_main, TXB_ID_3, C_PALE_TURQUOISE); */
    /* #endif */
}
uint32_t battery_percentage = 0;
void _draw_battery(void) {
    UG_ProgressSetProgress(&window_main, PGB_ID_1, battery_percentage);
    battery_percentage += 1;
    if (battery_percentage >= 50) battery_percentage = 1;
}

#ifdef SIM

void windowHandler(UG_MESSAGE *msg);
void GUI_Setup(UG_DEVICE *device)
{
    //Setup UGUI
    /* UG_Init(&ugui, device); */
    /*  */
    /* UG_FillScreen(C_BLACK); */

    //Setup Window
    /* UG_WindowCreate(&window_main, window_main_objects, WINDOW_MAIN_OBJECTS, NULL); */
    /* UG_WindowSetTitleTextFont (&window_main, FONT_6X8); */
    /* UG_WindowSetTitleText(&window_main, "App Title"); */

    // Buttons
    /* UG_ButtonCreate(&window_main, &btn0, BTN_ID_0, UGUI_POS(INITIAL_MARGIN, OBJ_Y(0), BTN_WIDTH, BTN_HEIGHT)); */
    /* UG_ButtonSetFont(&window_main, BTN_ID_0, FONT_6X8); */
    /* UG_ButtonSetText(&window_main, BTN_ID_0, "Btn 3D"); */
    /* UG_ButtonSetStyle(&window_main, BTN_ID_0, BTN_STYLE_3D); */
    /*  */
    /* UG_ButtonCreate(&window_main, &btn1, BTN_ID_1, UGUI_POS(INITIAL_MARGIN, OBJ_Y(1), BTN_WIDTH, BTN_HEIGHT)); */
    /* UG_ButtonSetFont(&window_main, BTN_ID_1, FONT_6X8); */
    /* UG_ButtonSetText(&window_main, BTN_ID_1, "Btn 2D T"); */
    /* UG_ButtonSetStyle(&window_main, BTN_ID_1, BTN_STYLE_2D|BTN_STYLE_TOGGLE_COLORS); */
    /*  */
    /* UG_ButtonCreate(&window_main, &btn2, BTN_ID_2, UGUI_POS(INITIAL_MARGIN, OBJ_Y(2), BTN_WIDTH, BTN_HEIGHT)); */
    /* UG_ButtonSetFont(&window_main, BTN_ID_2, FONT_6X8); */
    /* UG_ButtonSetText(&window_main, BTN_ID_2, "Btn 3D Alt"); */
    /* UG_ButtonSetStyle(&window_main, BTN_ID_2, BTN_STYLE_3D|BTN_STYLE_USE_ALTERNATE_COLORS); */
    /* UG_ButtonSetAlternateForeColor(&window_main, BTN_ID_2, C_BLACK); */
    /* UG_ButtonSetAlternateBackColor(&window_main, BTN_ID_2, C_WHITE); */
    /*  */
    /* UG_ButtonCreate(&window_main, &btn3, BTN_ID_3, UGUI_POS(INITIAL_MARGIN, OBJ_Y(3), BTN_WIDTH, BTN_HEIGHT)); */
    /* UG_ButtonSetFont(&window_main, BTN_ID_3, FONT_6X8); */
    /* UG_ButtonSetText(&window_main, BTN_ID_3, "Btn NoB"); */
    /* UG_ButtonSetStyle(&window_main, BTN_ID_3, BTN_STYLE_NO_BORDERS|BTN_STYLE_TOGGLE_COLORS); */
    /*  */
    /* // Checkboxes */
    /* UG_CheckboxCreate(&window_main, &chb0, CHB_ID_0, UGUI_POS(INITIAL_MARGIN*2+BTN_WIDTH, OBJ_Y(0)+7, CHB_WIDTH, CHB_HEIGHT)); */
    /* UG_CheckboxSetFont(&window_main, CHB_ID_0, FONT_6X8); */
    /* UG_CheckboxSetText(&window_main, CHB_ID_0, "CHB"); */
    /* UG_CheckboxSetStyle(&window_main, CHB_ID_0, CHB_STYLE_3D); */
    /* UG_CheckboxSetAlignment(&window_main, CHB_ID_0, ALIGN_TOP_LEFT); */
    /* #if !defined(UGUI_USE_COLOR_BW) */
    /* UG_CheckboxSetBackColor(&window_main, CHB_ID_0, C_PALE_TURQUOISE); */
    /* #endif */
    /*  */
    /* UG_CheckboxCreate(&window_main, &chb1, CHB_ID_1, UGUI_POS(INITIAL_MARGIN*2+BTN_WIDTH, OBJ_Y(1)+7, CHB_WIDTH, CHB_HEIGHT)); */
    /* UG_CheckboxSetFont(&window_main, CHB_ID_1, FONT_6X8); */
    /* UG_CheckboxSetText(&window_main, CHB_ID_1, "CHB"); */
    /* UG_CheckboxSetStyle(&window_main, CHB_ID_1, CHB_STYLE_2D|CHB_STYLE_TOGGLE_COLORS); */
    /* UG_CheckboxSetAlignment(&window_main, CHB_ID_1, ALIGN_CENTER); */
    /* UG_CheckboxShow(&window_main, CHB_ID_1); */
    /*  */
    /* UG_CheckboxCreate(&window_main, &chb2, CHB_ID_2, UGUI_POS(INITIAL_MARGIN*2+BTN_WIDTH, OBJ_Y(2)+7, CHB_WIDTH, CHB_HEIGHT)); */
    /* UG_CheckboxSetFont(&window_main, CHB_ID_2, FONT_6X8); */
    /* UG_CheckboxSetText(&window_main, CHB_ID_2, "CHB"); */
    /* UG_CheckboxSetStyle(&window_main, CHB_ID_2, CHB_STYLE_3D|CHB_STYLE_USE_ALTERNATE_COLORS); */
    /* UG_CheckboxSetAlignment(&window_main, CHB_ID_2, ALIGN_BOTTOM_LEFT); */
    /* UG_CheckboxShow(&window_main, CHB_ID_2); */
    /*  */
    /* UG_CheckboxCreate(&window_main, &chb3, CHB_ID_3, UGUI_POS(INITIAL_MARGIN*2+BTN_WIDTH, OBJ_Y(3)+7, CHB_WIDTH, CHB_HEIGHT)); */
    /* UG_CheckboxSetFont(&window_main, CHB_ID_3, FONT_6X8); */
    /* UG_CheckboxSetText(&window_main, CHB_ID_3, "CHB"); */
    /* UG_CheckboxSetStyle(&window_main, CHB_ID_3, CHB_STYLE_NO_BORDERS|CHB_STYLE_TOGGLE_COLORS); */
    /* UG_CheckboxSetAlignment(&window_main, CHB_ID_3, ALIGN_BOTTOM_RIGHT); */
    /* UG_CheckboxShow(&window_main, CHB_ID_3); */
    /*  */
    /* // Texts */
    /* UG_TextboxCreate(&window_main, &txt0, TXB_ID_0, UGUI_POS(INITIAL_MARGIN*3+BTN_WIDTH+CHB_WIDTH, OBJ_Y(0), 100, 15)); */
    /* UG_TextboxSetFont(&window_main, TXB_ID_0, FONT_4X6); */
    /* UG_TextboxSetText(&window_main, TXB_ID_0, "Small TEXT"); */
    /* #if !defined(UGUI_USE_COLOR_BW) */
    /* UG_TextboxSetBackColor(&window_main, TXB_ID_0, C_PALE_TURQUOISE); */
    /* #endif */
    /*  */
    /* #<{(| UG_TextboxCreate(&window_main, &txt1, TXB_ID_1, UGUI_POS(INITIAL_MARGIN*3+BTN_WIDTH+CHB_WIDTH, OBJ_Y(1)-15, 100, 30)); |)}># */
    /* #<{(| UG_TextboxSetFont(&window_main, TXB_ID_1, FONT_12X20); |)}># */
    /* #<{(| UG_TextboxSetText(&window_main, TXB_ID_1, "Text"); |)}># */
    /* #<{(| #if !defined(UGUI_USE_COLOR_BW) |)}># */
    /* #<{(| UG_TextboxSetBackColor(&window_main, TXB_ID_1, C_PALE_TURQUOISE); |)}># */
    /* #<{(| #endif |)}># */
    /* #<{(| UG_TextboxSetAlignment(&window_main, TXB_ID_1, ALIGN_TOP_RIGHT); |)}># */
    /*  */
    /* #<{(| UG_TextboxCreate(&window_main, &txt2, TXB_ID_2, UGUI_POS(INITIAL_MARGIN*3+BTN_WIDTH+CHB_WIDTH, OBJ_Y(2)-15, 100, 45)); |)}># */
    /* #<{(| UG_TextboxSetFont(&window_main, TXB_ID_2, FONT_24X40); |)}># */
    /* #<{(| UG_TextboxSetText(&window_main, TXB_ID_2, "Text"); |)}># */
    /* #<{(| #if !defined(UGUI_USE_COLOR_BW) |)}># */
    /* #<{(| UG_TextboxSetBackColor(&window_main, TXB_ID_2, C_PALE_TURQUOISE); |)}># */
    /* #<{(| #endif |)}># */
    /*  */
    /* UG_TextboxCreate(&window_main, &txt3, TXB_ID_3, UGUI_POS(INITIAL_MARGIN*3+BTN_WIDTH+CHB_WIDTH, OBJ_Y(3), 100, 53)); */
    /* UG_TextboxSetFont(&window_main, TXB_ID_3, FONT_32X53); */
    /* UG_TextboxSetText(&window_main, TXB_ID_3, "ABC"); */
    /* #if !defined(UGUI_USE_COLOR_BW) */
    /* UG_TextboxSetBackColor(&window_main, TXB_ID_3, C_PALE_TURQUOISE); */
    /* #endif */
    /*  */
    /* // Progress Bar */
    /* UG_ProgressCreate(&window_main, &pgb0, PGB_ID_0, UGUI_POS(INITIAL_MARGIN, OBJ_Y(4)+20, 157, 20)); */
    /* UG_ProgressSetProgress(&window_main, PGB_ID_0, 35); */
    /*  */
    /* UG_ProgressCreate(&window_main, &pgb1, PGB_ID_1, UGUI_POS(159+INITIAL_MARGIN*2, OBJ_Y(4)+25, 156, 10)); */
    /* UG_ProgressSetStyle(&window_main, PGB_ID_1, PGB_STYLE_2D | PGB_STYLE_FORE_COLOR_MESH); */
    /* UG_ProgressSetProgress(&window_main, PGB_ID_1, 75); */

}
bool x11_setup(int width, int height)
{
    //Mem Alloc's
    handle = (x11_data_t *)malloc(sizeof(x11_data_t));
    if (NULL == handle)
        return false;

    handle->imgBuffer = (UG_U32 *)malloc(((width * simCfg->screenMultiplier) * (height * simCfg->screenMultiplier)) * sizeof(UG_U32));
    if (NULL == handle->imgBuffer)
        return false;
    handle->simX = width;
    handle->simY = height;

    //Setup X
    unsigned long black,white;

    handle->dis = XOpenDisplay((char *)0);
    handle->screen = DefaultScreen(handle->dis);
    black = BlackPixel(handle->dis, handle->screen),
    white = WhitePixel(handle->dis, handle->screen);
    handle->win = XCreateSimpleWindow(handle->dis,
                                  DefaultRootWindow(handle->dis),
                                  0,
                                  0,
                                  width * simCfg->screenMultiplier + (simCfg->screenMargin * 2),
                                  height * simCfg->screenMultiplier + (simCfg->screenMargin * 2),
                                  5,
                                  black,
                                  simCfg->windowBackColor);
    XSetStandardProperties(handle->dis, handle->win, "uGUI Window",
                         "uGUI", None, NULL, 0, NULL);
    XSelectInput(handle->dis, handle->win,
               ExposureMask|ButtonPressMask|
               ButtonReleaseMask|KeyPressMask|Button1MotionMask);
    handle->gc = XCreateGC(handle->dis, handle->win, 0,0);
    XSetBackground(handle->dis, handle->gc, white);
    XSetForeground(handle->dis, handle->gc, black);
    XClearWindow(handle->dis, handle->win);
    XMapRaised(handle->dis, handle->win);
    handle->visual = DefaultVisual(handle->dis, 0);

    return true;
}

// http://www.mi.uni-koeln.de/c/mirror/www.cs.curtin.edu.au/units/cg252-502/notes/lect5h1.html

//Process Function
void x11_process(void)
{
    XEvent event;
#define BUFFER_SIZE         ((handle->simX * simCfg->screenMultiplier) * (handle->simY * simCfg->screenMultiplier)) * sizeof(UG_U32)
    uint32_t *ximage = (uint32_t *)malloc(BUFFER_SIZE);

    //Check for events
    while (XCheckMaskEvent(handle->dis,
        ButtonPressMask | ButtonReleaseMask | Button1MotionMask | PointerMotionMask, &event) == true)
    {
        #if defined(UGUI_USE_TOUCH)
        static int mouse_down;
        switch (event.type)
        {
            case ButtonPress:
                // we are interested only in LEFT button (button 1)
                if(event.xbutton.button != 1)
                    break;
                mouse_down = 1;
                UG_TouchUpdate((event.xbutton.x - simCfg->screenMargin) / simCfg->screenMultiplier, (event.xbutton.y - simCfg->screenMargin) / simCfg->screenMultiplier, TOUCH_STATE_PRESSED);
            break;

            case ButtonRelease:
                // we are interested only in LEFT button (button 1)
                if(event.xbutton.button != 1)
                    break;
                mouse_down = 0;
                UG_TouchUpdate(-1, -1, TOUCH_STATE_RELEASED);
            break;

            case MotionNotify:
                if(mouse_down && ( event.xmotion.state & Button1Mask ))
                {
                    UG_TouchUpdate((event.xmotion.x - simCfg->screenMargin) / simCfg->screenMultiplier, (event.xmotion.y - simCfg->screenMargin) / simCfg->screenMultiplier, TOUCH_STATE_PRESSED);
                }
            break;
        }
        #endif
    }
    if (ximage != NULL)
    {
        memcpy(ximage, handle->imgBuffer, BUFFER_SIZE);
        XImage *img = XCreateImage(handle->dis,
                                   handle->visual,
                                   24,
                                   ZPixmap,
                                   0,
                                   (char *)ximage,
                                   handle->simX * simCfg->screenMultiplier,
                                   handle->simY * simCfg->screenMultiplier,
                                   32,
                                   0);
        XPutImage(handle->dis,
              handle->win,
              handle->gc,
              img,
              0,
              0,
              simCfg->screenMargin,
              simCfg->screenMargin,
              handle->simX * simCfg->screenMultiplier,
              handle->simY * simCfg->screenMultiplier);
        XDestroyImage(img);
        XFlush(handle->dis);
    }
}

//Internal
void x11_pset(UG_S16 x, UG_S16 y, UG_COLOR c)
{
    UG_U32 tmp = c;

#if defined(UGUI_USE_COLOR_BW)
    /* Convert B/W to RGB888 */
    tmp = c == C_WHITE ? 0xFFFFFF : 0x000000;
#elif defined(UGUI_USE_COLOR_RGB565)
    /* Convert RGB565 to RGB888 */
    tmp = _UG_ConvertRGB565ToRGB888(c);
#endif

    for(UG_U8 i = 0; i < simCfg->screenMultiplier ; i++) {
        for(UG_U8 j = 0; j < simCfg->screenMultiplier ; j++) {
            handle->imgBuffer[(simCfg->width * simCfg->screenMultiplier * ((y * simCfg->screenMultiplier) + j)) + (x * simCfg->screenMultiplier) + i] = tmp;
        }
    }
}

void x11_flush(void)
{
    // nop
}

static const char* message_type[] = {
    "NONE",
    "WINDOW",
    "OBJECT"
};
static const char* event_type[] = {
    "NONE",
    "PRERENDER",
    "POSTRENDER",
    "PRESSED",
    "RELEASED"
    };

void decode_msg(UG_MESSAGE* msg)
{
    printf("%s %s for ID %d (SubId %d)\n", 
        message_type[msg->type],
        event_type[msg->event],
        msg->id, msg->sub_id);
}

void windowHandler(UG_MESSAGE *msg)
{
    static UG_U16 x0, y0;

    decode_msg(msg);
}
#endif
