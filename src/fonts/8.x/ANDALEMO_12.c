/*******************************************************************************
 * Size: 12 px
 * Bpp: 2
 * Opts: --size 12 --no-compress -o src/fonts/8.x/ANDALEMO_12.c --bpp 2 --format lvgl --font src/fonts/andalemo.ttf --symbols  0123456789.,-: --lv-font-name lv_font_andalemo_12
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_FONT_ANDALEMO_12
#define LV_FONT_ANDALEMO_12 1
#endif

#if LV_FONT_ANDALEMO_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */

    /* U+002C "," */
    0x30, 0x81, 0x0,

    /* U+002D "-" */
    0x3f, 0x0,

    /* U+002E "." */
    0x10, 0xc0,

    /* U+0030 "0" */
    0xb, 0xc0, 0x91, 0xc3, 0x3, 0xc, 0xd9, 0x32,
    0x24, 0xc0, 0xc2, 0x43, 0x2, 0xf0,

    /* U+0031 "1" */
    0x1d, 0x6, 0x40, 0x90, 0x24, 0x9, 0x2, 0x40,
    0x91, 0xff,

    /* U+0032 "2" */
    0x7e, 0x0, 0x90, 0x18, 0x9, 0x6, 0x2, 0x2,
    0x2, 0xfe,

    /* U+0033 "3" */
    0x7e, 0x0, 0xa0, 0x24, 0x78, 0x2, 0x40, 0x60,
    0x26, 0xf8,

    /* U+0034 "4" */
    0x1, 0xd0, 0xe, 0x40, 0x99, 0x2, 0x24, 0x30,
    0x91, 0xff, 0xc0, 0x9, 0x0, 0x24,

    /* U+0035 "5" */
    0x7f, 0x58, 0x6, 0x1, 0xf8, 0x2, 0x80, 0x30,
    0x29, 0xf8,

    /* U+0036 "6" */
    0x1f, 0x83, 0x0, 0x90, 0xe, 0xf4, 0xd0, 0xc9,
    0xc, 0x70, 0xc1, 0xf4,

    /* U+0037 "7" */
    0xbf, 0xc0, 0x18, 0x2, 0x40, 0x30, 0x9, 0x0,
    0xc0, 0x24, 0x3, 0x0,

    /* U+0038 "8" */
    0xb, 0xc0, 0x91, 0xc2, 0x43, 0x2, 0xb0, 0x28,
    0x60, 0xc0, 0xc3, 0x43, 0x3, 0xf4,

    /* U+0039 "9" */
    0x2e, 0x9, 0x18, 0xc0, 0xc9, 0xc, 0x2e, 0xc0,
    0xc, 0x2, 0x47, 0xe0,

    /* U+003A ":" */
    0x30, 0x0, 0x0, 0x0, 0xc0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 115, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 115, .box_w = 3, .box_h = 3, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 3, .adv_w = 115, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 5, .adv_w = 115, .box_w = 3, .box_h = 2, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 7, .adv_w = 115, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 21, .adv_w = 115, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 31, .adv_w = 115, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 41, .adv_w = 115, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 51, .adv_w = 115, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 65, .adv_w = 115, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 75, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 87, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 115, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 113, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 115, .box_w = 3, .box_h = 6, .ofs_x = 2, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0xc, 0xd, 0xe
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 15, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 4, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    },
    {
        .range_start = 48, .range_length = 11, .glyph_id_start = 5,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 2,
    .bpp = 2,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t lv_font_andalemo_12 = {
#else
lv_font_t lv_font_andalemo_12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 10,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LV_FONT_ANDALEMO_12*/

