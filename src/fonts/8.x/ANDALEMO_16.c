/*******************************************************************************
 * Size: 16 px
 * Bpp: 2
 * Opts: --size 16 --no-compress -o src/fonts/8.x/ANDALEMO_16.c --bpp 2 --format lvgl --font src/fonts/andalemo.ttf --symbols  0123456789.,-: --lv-font-name lv_font_andalemo_16
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_FONT_ANDALEMO_16
#define LV_FONT_ANDALEMO_16 1
#endif

#if LV_FONT_ANDALEMO_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */

    /* U+002C "," */
    0x38, 0xb2, 0x40,

    /* U+002D "-" */
    0xbf, 0xc0, 0x0,

    /* U+002E "." */
    0x34, 0xe0,

    /* U+0030 "0" */
    0xb, 0xd0, 0x38, 0x74, 0x70, 0x1c, 0xa0, 0xc,
    0xe0, 0xd, 0xe7, 0xcd, 0xe3, 0x8d, 0xa0, 0xc,
    0x70, 0x1c, 0x38, 0x34, 0xb, 0xe0,

    /* U+0031 "1" */
    0x1f, 0xb, 0xf0, 0xb, 0x0, 0xb0, 0xb, 0x0,
    0xb0, 0xb, 0x0, 0xb0, 0xb, 0x0, 0xb0, 0xff,
    0xf0,

    /* U+0032 "2" */
    0x2f, 0xc0, 0x42, 0xc0, 0x3, 0x40, 0xd, 0x0,
    0x30, 0x2, 0x80, 0x1c, 0x0, 0xd0, 0xd, 0x0,
    0xd0, 0x7, 0xff, 0x80,

    /* U+0033 "3" */
    0x2f, 0x80, 0x42, 0xc0, 0x3, 0x40, 0xd, 0x0,
    0xb0, 0x2f, 0x0, 0xb, 0x0, 0xe, 0x0, 0x38,
    0x42, 0xc3, 0xf8, 0x0,

    /* U+0034 "4" */
    0x0, 0x3c, 0x0, 0x2f, 0x0, 0xd, 0xc0, 0xd,
    0x70, 0x7, 0x1c, 0x3, 0x7, 0x2, 0x81, 0xc0,
    0xff, 0xfd, 0x0, 0x1c, 0x0, 0x7, 0x0, 0x1,
    0xc0,

    /* U+0035 "5" */
    0x2f, 0xf4, 0x90, 0x2, 0x40, 0x9, 0x0, 0x2f,
    0xd0, 0x1, 0xc0, 0x2, 0x80, 0xb, 0x0, 0x38,
    0x42, 0xc3, 0xf8, 0x0,

    /* U+0036 "6" */
    0x7, 0xf4, 0x1d, 0x0, 0x34, 0x0, 0x70, 0x0,
    0x7b, 0xe0, 0xb8, 0x38, 0x70, 0xc, 0x70, 0xd,
    0x34, 0xc, 0x1d, 0x2c, 0x7, 0xe0,

    /* U+0037 "7" */
    0x7f, 0xfc, 0x0, 0x2c, 0x0, 0x34, 0x0, 0x70,
    0x0, 0xe0, 0x0, 0xc0, 0x2, 0x80, 0x3, 0x0,
    0x7, 0x0, 0xd, 0x0, 0x1c, 0x0,

    /* U+0038 "8" */
    0xb, 0xd0, 0x38, 0x78, 0x30, 0x1c, 0x30, 0x28,
    0x1e, 0xe0, 0x1e, 0xf0, 0x70, 0x2c, 0xa0, 0xc,
    0xb0, 0xc, 0x38, 0x3c, 0x1f, 0xe0,

    /* U+0039 "9" */
    0xf, 0xd0, 0x38, 0x70, 0x70, 0x28, 0xb0, 0xc,
    0x74, 0x1c, 0x1f, 0xec, 0x0, 0xc, 0x0, 0x1c,
    0x0, 0x38, 0x10, 0xb0, 0x3f, 0x80,

    /* U+003A ":" */
    0x34, 0x90, 0x0, 0x0, 0x2, 0x4d
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 154, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 154, .box_w = 3, .box_h = 4, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 3, .adv_w = 154, .box_w = 6, .box_h = 2, .ofs_x = 2, .ofs_y = 3},
    {.bitmap_index = 6, .adv_w = 154, .box_w = 3, .box_h = 2, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 8, .adv_w = 154, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 30, .adv_w = 154, .box_w = 6, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 47, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 67, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 87, .adv_w = 154, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 112, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 154, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 154, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 176, .adv_w = 154, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 154, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 220, .adv_w = 154, .box_w = 3, .box_h = 8, .ofs_x = 3, .ofs_y = 0}
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
const lv_font_t lv_font_andalemo_16 = {
#else
lv_font_t lv_font_andalemo_16 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 13,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LV_FONT_ANDALEMO_16*/

