/*******************************************************************************
 * Size: 8 px
 * Bpp: 4
 * Opts: --bpp 4 --size 8 --no-compress --font OPlusSans3-Medium.ttf --symbols 0123456789 --format lvgl -o game_2048_font_8.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef GAME_2048_FONT_8
#define GAME_2048_FONT_8 1
#endif

#if GAME_2048_FONT_8

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0030 "0" */
    0xa, 0xca, 0x5, 0x90, 0x85, 0x75, 0x5, 0x77,
    0x50, 0x58, 0x75, 0x5, 0x74, 0x90, 0x95, 0xa,
    0xca, 0x0,

    /* U+0031 "1" */
    0x2b, 0x88, 0x98, 0x5, 0x80, 0x58, 0x5, 0x80,
    0x58, 0x5, 0x80,

    /* U+0032 "2" */
    0x2c, 0xc6, 0x8, 0x30, 0xe0, 0x0, 0xe, 0x0,
    0x6, 0x90, 0x3, 0xc0, 0x1, 0xd1, 0x0, 0xad,
    0xbb, 0x0,

    /* U+0033 "3" */
    0x6c, 0xdb, 0x0, 0xc, 0x30, 0x7, 0x80, 0x1,
    0xeb, 0x50, 0x0, 0xd, 0x5, 0x0, 0xd0, 0x5c,
    0xc5, 0x0,

    /* U+0034 "4" */
    0x0, 0x3b, 0x0, 0xb, 0xb0, 0x4, 0xab, 0x0,
    0xc2, 0xb0, 0x57, 0x1b, 0xa, 0xcc, 0xe6, 0x0,
    0x1b, 0x0,

    /* U+0035 "5" */
    0xe, 0xcc, 0x2, 0x90, 0x0, 0x38, 0x0, 0x5,
    0xdc, 0x70, 0x13, 0xb, 0x22, 0x20, 0xa3, 0x2c,
    0xc9, 0x0,

    /* U+0036 "6" */
    0x0, 0xc3, 0x0, 0x6a, 0x0, 0x1e, 0x20, 0x6,
    0xeb, 0x90, 0x96, 0x9, 0x47, 0x50, 0x85, 0x1b,
    0xba, 0x0,

    /* U+0037 "7" */
    0x8c, 0xce, 0x30, 0x0, 0xc0, 0x0, 0x58, 0x0,
    0xb, 0x20, 0x1, 0xb0, 0x0, 0x75, 0x0, 0xd,
    0x0, 0x0,

    /* U+0038 "8" */
    0xa, 0xc9, 0x4, 0x80, 0xa2, 0x49, 0xb, 0x20,
    0xdd, 0xc0, 0x76, 0x7, 0x68, 0x50, 0x67, 0x1b,
    0xba, 0x0,

    /* U+0039 "9" */
    0x1c, 0xc8, 0x8, 0x50, 0xb2, 0xa2, 0x8, 0x48,
    0x60, 0xc1, 0xb, 0xca, 0x0, 0x9, 0x20, 0x6,
    0x80, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 80, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 18, .adv_w = 55, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 29, .adv_w = 71, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 47, .adv_w = 71, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 65, .adv_w = 76, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 75, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 101, .adv_w = 75, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 119, .adv_w = 72, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 137, .adv_w = 78, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 155, .adv_w = 75, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0}
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
    -4, -2, -2, -1, -3, -2, -2, -2,
    -6, -2, -2, -1, -4, -2, -6, -8,
    -4, -3, -12, -2, -8, 1, -3, -2,
    -5, -1, -1, -5, -1, -4, -4, -1
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
const lv_font_t game_2048_font_8 = {
#else
lv_font_t game_2048_font_8 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 7,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if GAME_2048_FONT_8*/

