/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --bpp 1 --format lvgl --font ../../thirdparty/ttf2ugui/ANDALEMO.TTF --symbols 0123456789 !#$%^&*()-_=+[];:,.<>/?~abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ° --no-compress --lv-include lvgl.h --size 16 --lv-font-name lv_font_andalemo_16 --output ../../src/fonts/8.x/ANDALEMO_16.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
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
    0x0,

    /* U+0021 "!" */
    0xfe, 0x60,

    /* U+0023 "#" */
    0x12, 0x12, 0x22, 0xff, 0x24, 0x24, 0x64, 0xff,
    0x48, 0x48, 0x48,

    /* U+0024 "$" */
    0x10, 0xfd, 0x24, 0x91, 0xc1, 0x85, 0x14, 0x51,
    0xfe, 0x10,

    /* U+0025 "%" */
    0x63, 0x49, 0x25, 0x92, 0x86, 0xc0, 0x40, 0x5c,
    0x31, 0x28, 0x94, 0x51, 0xc0,

    /* U+0026 "&" */
    0x38, 0x22, 0x11, 0x9, 0x83, 0x81, 0x85, 0x65,
    0x1c, 0x86, 0x63, 0x9e, 0x60,

    /* U+0028 "(" */
    0x2, 0x44, 0x88, 0x88, 0x88, 0x42, 0x30,

    /* U+0029 ")" */
    0x4, 0x22, 0x11, 0x11, 0x11, 0x24, 0xc0,

    /* U+002A "*" */
    0x25, 0x5c, 0xea, 0x90,

    /* U+002B "+" */
    0x10, 0x20, 0x47, 0xf1, 0x2, 0x4, 0x0,

    /* U+002C "," */
    0xfc,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x8, 0x21, 0x84, 0x30, 0x82, 0x10, 0x42, 0x8,
    0x0,

    /* U+0030 "0" */
    0x38, 0x8a, 0xc, 0x18, 0x33, 0x66, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+0031 "1" */
    0x63, 0x82, 0x8, 0x20, 0x82, 0x8, 0x20, 0x8f,
    0xc0,

    /* U+0032 "2" */
    0xf8, 0x30, 0x41, 0x4, 0x21, 0x84, 0x21, 0xf,
    0xc0,

    /* U+0033 "3" */
    0x78, 0x30, 0x41, 0x8, 0xc0, 0xc1, 0x4, 0x3f,
    0x0,

    /* U+0034 "4" */
    0xc, 0x18, 0x50, 0xa2, 0x4c, 0x91, 0x62, 0xfe,
    0x8, 0x10,

    /* U+0035 "5" */
    0x7d, 0x4, 0x10, 0x78, 0x30, 0x41, 0x4, 0x2f,
    0x0,

    /* U+0036 "6" */
    0x3e, 0xc1, 0x4, 0x8, 0x1f, 0xa1, 0xc1, 0x82,
    0x8c, 0xf0,

    /* U+0037 "7" */
    0xfe, 0xc, 0x10, 0x60, 0x83, 0x4, 0x8, 0x20,
    0x41, 0x80,

    /* U+0038 "8" */
    0x38, 0x89, 0x12, 0x23, 0x8f, 0x31, 0x41, 0x83,
    0x8d, 0xf0,

    /* U+0039 "9" */
    0x79, 0x8a, 0xc, 0x1c, 0x2f, 0xc0, 0x81, 0x4,
    0x1b, 0xe0,

    /* U+003A ":" */
    0xf0, 0xf,

    /* U+003B ";" */
    0xf0, 0x3, 0x60,

    /* U+003C "<" */
    0x4, 0x66, 0x20, 0x60, 0x60, 0xc0,

    /* U+003D "=" */
    0xff, 0x0, 0x0, 0xff,

    /* U+003E ">" */
    0x81, 0x81, 0x81, 0x19, 0x8c, 0x0,

    /* U+003F "?" */
    0xf0, 0x42, 0x11, 0x11, 0x8, 0x3, 0x18,

    /* U+0041 "A" */
    0xc, 0x6, 0x5, 0x2, 0xc1, 0x21, 0x90, 0xfc,
    0x42, 0x61, 0x20, 0xd0, 0x20,

    /* U+0042 "B" */
    0xf9, 0x1a, 0x14, 0x28, 0xde, 0x23, 0x41, 0x83,
    0xf, 0xe0,

    /* U+0043 "C" */
    0x3c, 0x83, 0x4, 0x8, 0x10, 0x20, 0x40, 0xc0,
    0x84, 0xf0,

    /* U+0044 "D" */
    0xf1, 0x1a, 0x14, 0x18, 0x30, 0x60, 0xc1, 0x85,
    0x1b, 0xc0,

    /* U+0045 "E" */
    0xfc, 0x21, 0x8, 0x7e, 0x10, 0x84, 0x3e,

    /* U+0046 "F" */
    0xfe, 0x8, 0x20, 0x83, 0xe8, 0x20, 0x82, 0x8,
    0x0,

    /* U+0047 "G" */
    0x3c, 0x87, 0x4, 0x8, 0x10, 0x23, 0xc1, 0xc2,
    0x84, 0xf0,

    /* U+0048 "H" */
    0x86, 0x18, 0x61, 0x87, 0xf8, 0x61, 0x86, 0x18,
    0x40,

    /* U+0049 "I" */
    0xf9, 0x8, 0x42, 0x10, 0x84, 0x21, 0x3e,

    /* U+004A "J" */
    0x8, 0x42, 0x10, 0x84, 0x21, 0x8, 0x7c,

    /* U+004B "K" */
    0x86, 0x8c, 0x88, 0x90, 0xa0, 0xf0, 0x90, 0x98,
    0x8c, 0x84, 0x82,

    /* U+004C "L" */
    0x82, 0x8, 0x20, 0x82, 0x8, 0x20, 0x82, 0xf,
    0xc0,

    /* U+004D "M" */
    0xc7, 0x8f, 0x1d, 0x5a, 0xb2, 0x60, 0xc1, 0x83,
    0x6, 0x8,

    /* U+004E "N" */
    0x83, 0x87, 0x8d, 0x1b, 0x32, 0x66, 0xc5, 0x8f,
    0xe, 0x8,

    /* U+004F "O" */
    0x38, 0x8a, 0x1c, 0x18, 0x30, 0x60, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+0050 "P" */
    0xfa, 0x38, 0x61, 0x8f, 0xe8, 0x20, 0x82, 0x8,
    0x0,

    /* U+0051 "Q" */
    0x38, 0x8a, 0x1c, 0x18, 0x30, 0x60, 0xc1, 0x82,
    0x88, 0xe0, 0x60, 0x60,

    /* U+0052 "R" */
    0xf9, 0x1a, 0x14, 0x28, 0xde, 0x24, 0x4c, 0x8d,
    0xa, 0x18,

    /* U+0053 "S" */
    0x7f, 0x8, 0x20, 0x60, 0xe0, 0xc1, 0x4, 0x3f,
    0x0,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,
    0x20, 0x40,

    /* U+0055 "U" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0x61, 0x87, 0x37,
    0x80,

    /* U+0056 "V" */
    0x40, 0xa0, 0xd8, 0x44, 0x22, 0x31, 0x90, 0x48,
    0x2c, 0x14, 0x6, 0x3, 0x0,

    /* U+0057 "W" */
    0xc0, 0xa0, 0x50, 0x28, 0x14, 0xca, 0x6d, 0x54,
    0xa6, 0x53, 0x11, 0x88, 0x40,

    /* U+0058 "X" */
    0x83, 0x42, 0x64, 0x2c, 0x18, 0x18, 0x38, 0x2c,
    0x66, 0x42, 0x83,

    /* U+0059 "Y" */
    0x83, 0x8d, 0x13, 0x62, 0x82, 0x4, 0x8, 0x10,
    0x20, 0x40,

    /* U+005A "Z" */
    0xfe, 0xc, 0x10, 0x41, 0x82, 0xc, 0x10, 0x41,
    0x83, 0xf8,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x4e,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0x9e,

    /* U+005E "^" */
    0x10, 0x18, 0x2c, 0x64, 0x42, 0x82,

    /* U+005F "_" */
    0xff, 0xc0,

    /* U+0061 "a" */
    0x78, 0x30, 0x47, 0x66, 0x18, 0xdd,

    /* U+0062 "b" */
    0x81, 0x2, 0x5, 0xec, 0x50, 0x60, 0xc1, 0x83,
    0x8a, 0xe0,

    /* U+0063 "c" */
    0x3d, 0x8, 0x20, 0x82, 0x4, 0xf,

    /* U+0064 "d" */
    0x2, 0x4, 0x9, 0xd4, 0x70, 0x60, 0xc1, 0x82,
    0x8c, 0xe8,

    /* U+0065 "e" */
    0x38, 0x8a, 0xf, 0xf8, 0x10, 0x10, 0x1f,

    /* U+0066 "f" */
    0x1c, 0x82, 0x3f, 0x20, 0x82, 0x8, 0x20, 0x82,
    0x0,

    /* U+0067 "g" */
    0x7f, 0x8a, 0x16, 0x67, 0x98, 0x20, 0x7f, 0x83,
    0xd, 0xf0,

    /* U+0068 "h" */
    0x82, 0x8, 0x2e, 0xc6, 0x18, 0x61, 0x86, 0x18,
    0x40,

    /* U+0069 "i" */
    0x20, 0x72, 0x49, 0x24, 0x80,

    /* U+006A "j" */
    0x10, 0x7, 0x11, 0x11, 0x11, 0x11, 0x1e,

    /* U+006B "k" */
    0x81, 0x2, 0x4, 0x69, 0x96, 0x38, 0x58, 0x99,
    0x1a, 0x10,

    /* U+006C "l" */
    0xe1, 0x8, 0x42, 0x10, 0x84, 0x21, 0xe,

    /* U+006D "m" */
    0xb7, 0xb6, 0x4c, 0x99, 0x32, 0x64, 0xc9,

    /* U+006E "n" */
    0xbb, 0x18, 0x61, 0x86, 0x18, 0x61,

    /* U+006F "o" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x51, 0x1c,

    /* U+0070 "p" */
    0xb9, 0x8a, 0xc, 0x18, 0x30, 0x71, 0x5c, 0x81,
    0x2, 0x0,

    /* U+0071 "q" */
    0x3a, 0x8e, 0xc, 0x18, 0x30, 0x51, 0x9d, 0x2,
    0x4, 0x8,

    /* U+0072 "r" */
    0xbe, 0x21, 0x8, 0x42, 0x10,

    /* U+0073 "s" */
    0x7e, 0x8, 0x1c, 0x1c, 0x10, 0x7e,

    /* U+0074 "t" */
    0x20, 0x43, 0xf1, 0x2, 0x4, 0x8, 0x10, 0x20,
    0x3c,

    /* U+0075 "u" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0xdd,

    /* U+0076 "v" */
    0xc2, 0x8d, 0x13, 0x22, 0xc5, 0x6, 0x8,

    /* U+0077 "w" */
    0x81, 0x83, 0x9b, 0xda, 0x6a, 0x66, 0x66, 0x42,

    /* U+0078 "x" */
    0xc2, 0xc8, 0xa0, 0xc1, 0x85, 0x19, 0x61,

    /* U+0079 "y" */
    0xc2, 0x8d, 0x13, 0x22, 0xc5, 0x6, 0x8, 0x10,
    0x43, 0x80,

    /* U+007A "z" */
    0xfc, 0x21, 0x84, 0x21, 0x84, 0x3f,

    /* U+007E "~" */
    0x78, 0xc7, 0x80,

    /* U+00B0 "°" */
    0x69, 0x96
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 154, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 154, .box_w = 1, .box_h = 11, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 154, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 14, .adv_w = 154, .box_w = 6, .box_h = 13, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 24, .adv_w = 154, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 37, .adv_w = 154, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 50, .adv_w = 154, .box_w = 4, .box_h = 13, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 57, .adv_w = 154, .box_w = 4, .box_h = 13, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 64, .adv_w = 154, .box_w = 5, .box_h = 6, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 68, .adv_w = 154, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 75, .adv_w = 154, .box_w = 2, .box_h = 3, .ofs_x = 4, .ofs_y = -1},
    {.bitmap_index = 76, .adv_w = 154, .box_w = 5, .box_h = 1, .ofs_x = 2, .ofs_y = 4},
    {.bitmap_index = 77, .adv_w = 154, .box_w = 2, .box_h = 2, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 78, .adv_w = 154, .box_w = 6, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 87, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 154, .box_w = 6, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 106, .adv_w = 154, .box_w = 6, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 115, .adv_w = 154, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 124, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 134, .adv_w = 154, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 143, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 153, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 183, .adv_w = 154, .box_w = 2, .box_h = 8, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 185, .adv_w = 154, .box_w = 2, .box_h = 10, .ofs_x = 4, .ofs_y = -2},
    {.bitmap_index = 188, .adv_w = 154, .box_w = 6, .box_h = 7, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 194, .adv_w = 154, .box_w = 8, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 198, .adv_w = 154, .box_w = 6, .box_h = 7, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 204, .adv_w = 154, .box_w = 5, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 154, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 224, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 244, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 254, .adv_w = 154, .box_w = 5, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 154, .box_w = 6, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 270, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 280, .adv_w = 154, .box_w = 6, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 289, .adv_w = 154, .box_w = 5, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 154, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 303, .adv_w = 154, .box_w = 8, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 314, .adv_w = 154, .box_w = 6, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 323, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 343, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 353, .adv_w = 154, .box_w = 6, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 362, .adv_w = 154, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 374, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 384, .adv_w = 154, .box_w = 6, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 393, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 403, .adv_w = 154, .box_w = 6, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 412, .adv_w = 154, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 425, .adv_w = 154, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 438, .adv_w = 154, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 449, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 459, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 469, .adv_w = 154, .box_w = 3, .box_h = 13, .ofs_x = 4, .ofs_y = -2},
    {.bitmap_index = 474, .adv_w = 154, .box_w = 3, .box_h = 13, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 479, .adv_w = 154, .box_w = 8, .box_h = 6, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 485, .adv_w = 154, .box_w = 10, .box_h = 1, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 487, .adv_w = 154, .box_w = 6, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 493, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 503, .adv_w = 154, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 509, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 519, .adv_w = 154, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 526, .adv_w = 154, .box_w = 6, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 535, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 545, .adv_w = 154, .box_w = 6, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 554, .adv_w = 154, .box_w = 3, .box_h = 11, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 559, .adv_w = 154, .box_w = 4, .box_h = 14, .ofs_x = 2, .ofs_y = -3},
    {.bitmap_index = 566, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 576, .adv_w = 154, .box_w = 5, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 583, .adv_w = 154, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 590, .adv_w = 154, .box_w = 6, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 596, .adv_w = 154, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 603, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 2, .ofs_y = -3},
    {.bitmap_index = 613, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 623, .adv_w = 154, .box_w = 5, .box_h = 8, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 628, .adv_w = 154, .box_w = 6, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 634, .adv_w = 154, .box_w = 7, .box_h = 10, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 643, .adv_w = 154, .box_w = 6, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 649, .adv_w = 154, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 656, .adv_w = 154, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 664, .adv_w = 154, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 671, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 681, .adv_w = 154, .box_w = 6, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 687, .adv_w = 154, .box_w = 9, .box_h = 2, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 690, .adv_w = 154, .box_w = 4, .box_h = 4, .ofs_x = 3, .ofs_y = 7}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint8_t glyph_id_ofs_list_0[] = {
    0, 1, 0, 2, 3, 4, 5
};

static const uint16_t unicode_list_5[] = {
    0x0, 0x32
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 7, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_0, .list_length = 7, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
    },
    {
        .range_start = 40, .range_length = 24, .glyph_id_start = 7,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 65, .range_length = 27, .glyph_id_start = 31,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 93, .range_length = 3, .glyph_id_start = 58,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 97, .range_length = 26, .glyph_id_start = 61,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 126, .range_length = 51, .glyph_id_start = 87,
        .unicode_list = unicode_list_5, .glyph_id_ofs_list = NULL, .list_length = 2, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
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
    .cmap_num = 6,
    .bpp = 1,
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
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
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

