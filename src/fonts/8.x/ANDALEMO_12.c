/*******************************************************************************
 * Size: 12 px
 * Bpp: 4
 * Opts: --size 12 -o src/fonts/8.x/ANDALEMO_12.c --bpp 4 --format lvgl --font src/fonts/andalemo.ttf --symbols  0123456789.,-: --lv-font-name lv_font_andalemo_12
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
    0xe, 0x30, 0x40, 0x7, 0x98,

    /* U+002D "-" */
    0x2d, 0xd8, 0xc0,

    /* U+002E "." */
    0x5, 0x0, 0x59, 0x0,

    /* U+0030 "0" */
    0x1, 0xbd, 0xc2, 0x0, 0x46, 0x6c, 0x70, 0x1,
    0x9c, 0x10, 0x48, 0x44, 0x5c, 0x8c, 0xc0, 0x1,
    0xa3, 0x0, 0x4, 0x6a, 0x26, 0x60, 0x2b, 0x1,
    0x89, 0x5, 0xee, 0xbf, 0x80,

    /* U+0031 "1" */
    0x7, 0xe5, 0x3, 0x4, 0x0, 0x1b, 0x80, 0x7f,
    0xf2, 0xd3, 0x12, 0x30,

    /* U+0032 "2" */
    0x7d, 0xd5, 0x3, 0xee, 0x81, 0xc0, 0x2c, 0xe0,
    0xb, 0x74, 0x0, 0x41, 0x40, 0x2f, 0xa0, 0x19,
    0xa0, 0x5, 0x36, 0xe4, 0x80,

    /* U+0033 "3" */
    0x5c, 0xda, 0x15, 0xcd, 0x39, 0x0, 0x8b, 0x41,
    0xbc, 0x94, 0x1b, 0xcd, 0xc0, 0x2e, 0xc0, 0xb,
    0x72, 0x77, 0x42, 0xe0,

    /* U+0034 "4" */
    0x0, 0x97, 0xd0, 0x2, 0x19, 0x40, 0xd, 0x10,
    0x0, 0xc6, 0x64, 0x0, 0xdf, 0x60, 0x19, 0xb,
    0x35, 0x24, 0xd3, 0xb7, 0x49, 0x26, 0x1, 0xf0,

    /* U+0035 "5" */
    0x4e, 0xdd, 0x30, 0x36, 0xe9, 0x80, 0x3c, 0xd9,
    0x20, 0x9d, 0x83, 0x20, 0x15, 0xa8, 0x5, 0x6a,
    0xfb, 0xa1, 0x90,

    /* U+0036 "6" */
    0x6, 0xdd, 0x48, 0x1d, 0x6e, 0xa4, 0x26, 0x0,
    0x33, 0xf6, 0x61, 0x0, 0x19, 0x8e, 0xa1, 0x63,
    0x2, 0x2b, 0xe4, 0x4, 0x8d, 0x2f, 0x72, 0xc0,

    /* U+0037 "7" */
    0xad, 0xdb, 0xc2, 0xb7, 0x52, 0x80, 0x1b, 0xfc,
    0x1, 0x13, 0xa0, 0x5, 0x72, 0x1, 0xa, 0x28,
    0x5, 0x34, 0x1, 0x99, 0x80, 0x10,

    /* U+0038 "8" */
    0x1, 0xbc, 0xc1, 0x0, 0x23, 0x72, 0x38, 0x2,
    0x30, 0x70, 0xa, 0x7a, 0xbf, 0x40, 0x10, 0x75,
    0x34, 0x0, 0x78, 0x5, 0x62, 0x2, 0x50, 0x23,
    0x20, 0xe9, 0xde, 0xa0,

    /* U+0039 "9" */
    0x1b, 0xdb, 0x10, 0xae, 0xdd, 0x40, 0x2a, 0x3,
    0x38, 0x2b, 0x81, 0x1, 0x56, 0x62, 0x0, 0xc6,
    0xb2, 0x80, 0x40, 0x29, 0x90, 0x36, 0xe8, 0x1c,
    0x0,

    /* U+003A ":" */
    0xe, 0x20, 0xd2, 0x3, 0x0, 0xe3, 0x0, 0x69,
    0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 115, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 115, .box_w = 3, .box_h = 3, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 5, .adv_w = 115, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 8, .adv_w = 115, .box_w = 3, .box_h = 2, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 12, .adv_w = 115, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 41, .adv_w = 115, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 53, .adv_w = 115, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 115, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 94, .adv_w = 115, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 118, .adv_w = 115, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 137, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 161, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 183, .adv_w = 115, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 236, .adv_w = 115, .box_w = 3, .box_h = 6, .ofs_x = 2, .ofs_y = 0}
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
    .bpp = 4,
    .kern_classes = 0,
    .bitmap_format = 1,
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

