/*******************************************************************************
 * Size: 12 px
 * Bpp: 4
 * Opts: --bpp 4 --size 12 --no-compress --font OPlusSans3-Medium.ttf --symbols 0123456789 --format lvgl -o game_2048_font_12.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef GAME_2048_FONT_12
#define GAME_2048_FONT_12 1
#endif

#if GAME_2048_FONT_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0030 "0" */
    0x2, 0xbf, 0xd6, 0x0, 0xca, 0x14, 0xf4, 0x1f,
    0x20, 0xa, 0xa3, 0xf0, 0x0, 0x8c, 0x3f, 0x0,
    0x8, 0xc3, 0xf0, 0x0, 0x8c, 0x1f, 0x20, 0xa,
    0xa0, 0xca, 0x14, 0xf4, 0x2, 0xbf, 0xe6, 0x0,

    /* U+0031 "1" */
    0x4, 0xcc, 0x6f, 0xdc, 0x31, 0x8c, 0x0, 0x8c,
    0x0, 0x8c, 0x0, 0x8c, 0x0, 0x8c, 0x0, 0x8c,
    0x0, 0x8c,

    /* U+0032 "2" */
    0x5, 0xdf, 0xc2, 0x3, 0xf4, 0x1a, 0xd0, 0x25,
    0x0, 0x4f, 0x0, 0x0, 0x7, 0xe0, 0x0, 0x2,
    0xf5, 0x0, 0x2, 0xe7, 0x0, 0x2, 0xe8, 0x0,
    0x1, 0xe9, 0x0, 0x0, 0x8f, 0xff, 0xff, 0x10,

    /* U+0033 "3" */
    0x4f, 0xff, 0xfa, 0x0, 0x0, 0x7d, 0x10, 0x0,
    0x8b, 0x0, 0x0, 0x9f, 0xfb, 0x20, 0x4, 0x10,
    0xbc, 0x0, 0x0, 0x3, 0xf1, 0x12, 0x0, 0x3f,
    0x18, 0xc2, 0x2c, 0xb0, 0x9, 0xef, 0xa1, 0x0,

    /* U+0034 "4" */
    0x0, 0x0, 0xb9, 0x0, 0x0, 0x6f, 0x90, 0x0,
    0x2e, 0xc9, 0x0, 0xc, 0x79, 0x90, 0x7, 0xc0,
    0x99, 0x2, 0xe2, 0x9, 0x90, 0xaf, 0xee, 0xff,
    0xb1, 0x11, 0x1a, 0xa1, 0x0, 0x0, 0x99, 0x0,

    /* U+0035 "5" */
    0x9, 0xff, 0xff, 0x10, 0xb7, 0x0, 0x0, 0xd,
    0x50, 0x0, 0x0, 0xfb, 0xec, 0x40, 0xd, 0x72,
    0x8f, 0x20, 0x0, 0x0, 0xe6, 0x2, 0x0, 0xd,
    0x61, 0xf6, 0x16, 0xf2, 0x5, 0xdf, 0xd4, 0x0,

    /* U+0036 "6" */
    0x0, 0x9, 0xc0, 0x0, 0x4, 0xe1, 0x0, 0x1,
    0xe3, 0x0, 0x0, 0xae, 0xed, 0x60, 0x2f, 0x92,
    0x6f, 0x46, 0xe0, 0x0, 0xa9, 0x5e, 0x0, 0xa,
    0x91, 0xf8, 0x15, 0xf4, 0x4, 0xcf, 0xd6, 0x0,

    /* U+0037 "7" */
    0x7f, 0xff, 0xff, 0x50, 0x11, 0x13, 0xf1, 0x0,
    0x0, 0x99, 0x0, 0x0, 0x1f, 0x20, 0x0, 0x8,
    0xb0, 0x0, 0x0, 0xf4, 0x0, 0x0, 0x7d, 0x0,
    0x0, 0xe, 0x50, 0x0, 0x6, 0xe0, 0x0, 0x0,

    /* U+0038 "8" */
    0x4, 0xcf, 0xe7, 0x0, 0xf7, 0x13, 0xf4, 0xe,
    0x70, 0x2f, 0x40, 0x4f, 0xff, 0xa0, 0x1e, 0x70,
    0x3e, 0x55, 0xe0, 0x0, 0x8b, 0x6e, 0x0, 0x8,
    0xb1, 0xf8, 0x14, 0xe6, 0x3, 0xcf, 0xe7, 0x0,

    /* U+0039 "9" */
    0x5, 0xdf, 0xd4, 0x2, 0xf6, 0x17, 0xf1, 0x8c,
    0x0, 0xd, 0x68, 0xc0, 0x0, 0xd7, 0x4f, 0x40,
    0x5f, 0x30, 0x8f, 0xff, 0xb0, 0x0, 0x3, 0xd2,
    0x0, 0x0, 0xd5, 0x0, 0x0, 0xba, 0x0, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 120, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 32, .adv_w = 82, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 50, .adv_w = 106, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 82, .adv_w = 107, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 113, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 113, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 210, .adv_w = 108, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 242, .adv_w = 118, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 274, .adv_w = 113, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 48, .range_length = 10, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    1, 8,
    2, 8,
    4, 2,
    4, 6,
    4, 8,
    4, 10,
    5, 2,
    5, 6,
    5, 8,
    5, 10,
    6, 2,
    6, 7,
    6, 8,
    6, 10,
    7, 2,
    7, 8,
    7, 10,
    8, 1,
    8, 5,
    8, 6,
    8, 7,
    8, 8,
    8, 9,
    9, 2,
    9, 8,
    9, 10,
    10, 4,
    10, 5,
    10, 6,
    10, 7,
    10, 8,
    10, 9
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -6, -3, -3, -2, -5, -3, -4, -3,
    -9, -3, -4, -1, -6, -4, -10, -13,
    -6, -5, -17, -3, -12, 1, -4, -3,
    -8, -2, -2, -7, -2, -6, -5, -1
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 32,
    .glyph_ids_size = 0
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
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 1,
    .bpp = 4,
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
const lv_font_t game_2048_font_12 = {
#else
lv_font_t game_2048_font_12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 9,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
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



#endif /*#if GAME_2048_FONT_12*/

