/*******************************************************************************
 * Size: 20 px
 * Bpp: 4
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef CONTROL_CENTER_APP_FONT_20
#define CONTROL_CENTER_APP_FONT_20 1
#endif

#if CONTROL_CENTER_APP_FONT_20

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+002B "+" */
    0x0, 0x0, 0x86, 0x0, 0x0, 0x0, 0x0, 0xfc,
    0x0, 0x0, 0x0, 0x0, 0xfc, 0x0, 0x0, 0x0,
    0x0, 0xfc, 0x0, 0x0, 0x7f, 0xff, 0xff, 0xff,
    0xf3, 0x5c, 0xcc, 0xff, 0xcc, 0xc2, 0x0, 0x0,
    0xfc, 0x0, 0x0, 0x0, 0x0, 0xfc, 0x0, 0x0,
    0x0, 0x0, 0xfc, 0x0, 0x0,

    /* U+002D "-" */
    0x99, 0x99, 0x8f, 0xff, 0xfe,

    /* U+002E "." */
    0x27, 0x1c, 0xfa, 0x8f, 0x60,

    /* U+0030 "0" */
    0x0, 0x4b, 0xef, 0xd8, 0x10, 0x0, 0x6f, 0xfc,
    0xbe, 0xfd, 0x0, 0x1f, 0xf5, 0x0, 0xc, 0xf9,
    0x7, 0xfc, 0x0, 0x0, 0x3f, 0xf0, 0xaf, 0x80,
    0x0, 0x0, 0xff, 0x2b, 0xf6, 0x0, 0x0, 0xd,
    0xf3, 0xcf, 0x50, 0x0, 0x0, 0xdf, 0x4c, 0xf5,
    0x0, 0x0, 0xd, 0xf4, 0xcf, 0x60, 0x0, 0x0,
    0xdf, 0x4b, 0xf6, 0x0, 0x0, 0xe, 0xf4, 0xaf,
    0x80, 0x0, 0x0, 0xff, 0x26, 0xfc, 0x0, 0x0,
    0x3f, 0xf0, 0x1f, 0xf5, 0x0, 0x1c, 0xf9, 0x0,
    0x6f, 0xfc, 0xbe, 0xfd, 0x10, 0x0, 0x4b, 0xff,
    0xd8, 0x10, 0x0,

    /* U+0031 "1" */
    0x0, 0x5, 0xdf, 0x30, 0x3c, 0xff, 0xf3, 0x2f,
    0xfd, 0xef, 0x32, 0xe6, 0xd, 0xf3, 0x0, 0x0,
    0xdf, 0x30, 0x0, 0xd, 0xf3, 0x0, 0x0, 0xdf,
    0x30, 0x0, 0xd, 0xf3, 0x0, 0x0, 0xdf, 0x30,
    0x0, 0xd, 0xf3, 0x0, 0x0, 0xdf, 0x30, 0x0,
    0xd, 0xf3, 0x0, 0x0, 0xdf, 0x30, 0x0, 0xd,
    0xf3, 0x0, 0x0, 0xdf, 0x30,

    /* U+0032 "2" */
    0x0, 0x18, 0xdf, 0xea, 0x20, 0x0, 0x2e, 0xfd,
    0xbd, 0xff, 0x20, 0xc, 0xf8, 0x0, 0xa, 0xfc,
    0x1, 0xec, 0x0, 0x0, 0x2f, 0xf0, 0x0, 0x10,
    0x0, 0x1, 0xff, 0x10, 0x0, 0x0, 0x0, 0x4f,
    0xf0, 0x0, 0x0, 0x0, 0xc, 0xf9, 0x0, 0x0,
    0x0, 0x8, 0xfe, 0x10, 0x0, 0x0, 0x7, 0xff,
    0x30, 0x0, 0x0, 0x6, 0xff, 0x40, 0x0, 0x0,
    0x5, 0xff, 0x50, 0x0, 0x0, 0x5, 0xff, 0x50,
    0x0, 0x0, 0x4, 0xff, 0x60, 0x0, 0x0, 0x2,
    0xff, 0xe9, 0x99, 0x99, 0x91, 0x4f, 0xff, 0xff,
    0xff, 0xff, 0x30,

    /* U+0033 "3" */
    0xc, 0xff, 0xff, 0xff, 0xf8, 0x0, 0x7a, 0xaa,
    0xac, 0xff, 0x40, 0x0, 0x0, 0x1, 0xef, 0x60,
    0x0, 0x0, 0x1, 0xdf, 0x70, 0x0, 0x0, 0x0,
    0xcf, 0x80, 0x0, 0x0, 0x0, 0xcf, 0xb0, 0x0,
    0x0, 0x0, 0xbf, 0xff, 0xfc, 0x40, 0x0, 0xc,
    0xc7, 0x6a, 0xff, 0x40, 0x0, 0x10, 0x0, 0x7,
    0xfd, 0x0, 0x0, 0x0, 0x0, 0xf, 0xf2, 0x0,
    0x0, 0x0, 0x0, 0xff, 0x31, 0x82, 0x0, 0x0,
    0x2f, 0xf1, 0x6f, 0xc1, 0x0, 0xb, 0xfa, 0x0,
    0xaf, 0xfb, 0xbe, 0xfd, 0x10, 0x0, 0x6c, 0xef,
    0xd8, 0x0, 0x0,

    /* U+0034 "4" */
    0x0, 0x0, 0x0, 0xb, 0xf5, 0x0, 0x0, 0x0,
    0x0, 0x6f, 0xf5, 0x0, 0x0, 0x0, 0x1, 0xff,
    0xf5, 0x0, 0x0, 0x0, 0xb, 0xff, 0xf5, 0x0,
    0x0, 0x0, 0x6f, 0xab, 0xf5, 0x0, 0x0, 0x2,
    0xfe, 0x1b, 0xf5, 0x0, 0x0, 0xc, 0xf4, 0xb,
    0xf5, 0x0, 0x0, 0x7f, 0x90, 0xb, 0xf5, 0x0,
    0x3, 0xfd, 0x0, 0xb, 0xf5, 0x0, 0xd, 0xf3,
    0x0, 0xb, 0xf5, 0x0, 0x6f, 0xff, 0xff, 0xff,
    0xff, 0xf5, 0x4a, 0xaa, 0xaa, 0xae, 0xfc, 0xa3,
    0x0, 0x0, 0x0, 0xb, 0xf5, 0x0, 0x0, 0x0,
    0x0, 0xb, 0xf5, 0x0, 0x0, 0x0, 0x0, 0xb,
    0xf5, 0x0,

    /* U+0035 "5" */
    0x9, 0xff, 0xff, 0xff, 0xf1, 0xb, 0xfb, 0xaa,
    0xaa, 0xa0, 0xd, 0xf1, 0x0, 0x0, 0x0, 0xf,
    0xf0, 0x0, 0x0, 0x0, 0xf, 0xd0, 0x0, 0x0,
    0x0, 0x3f, 0xb4, 0x89, 0x71, 0x0, 0x5f, 0xff,
    0xff, 0xff, 0x50, 0x6f, 0xe4, 0x2, 0x9f, 0xf2,
    0x2, 0x20, 0x0, 0xb, 0xf8, 0x0, 0x0, 0x0,
    0x7, 0xfb, 0x0, 0x0, 0x0, 0x6, 0xfc, 0x29,
    0x0, 0x0, 0x9, 0xf9, 0x9f, 0xa0, 0x0, 0x3f,
    0xf4, 0x1d, 0xfe, 0xbc, 0xff, 0x80, 0x1, 0x8d,
    0xfe, 0xb5, 0x0,

    /* U+0036 "6" */
    0x0, 0x0, 0x1, 0xff, 0x50, 0x0, 0x0, 0x0,
    0xc, 0xfa, 0x0, 0x0, 0x0, 0x0, 0x8f, 0xe1,
    0x0, 0x0, 0x0, 0x3, 0xff, 0x40, 0x0, 0x0,
    0x0, 0xe, 0xf9, 0x0, 0x0, 0x0, 0x0, 0x9f,
    0xd0, 0x0, 0x0, 0x0, 0x2, 0xff, 0xce, 0xfe,
    0x91, 0x0, 0x9, 0xff, 0xfb, 0xbe, 0xfe, 0x10,
    0xe, 0xfc, 0x10, 0x1, 0xcf, 0xa0, 0xf, 0xf4,
    0x0, 0x0, 0x3f, 0xf0, 0xf, 0xf1, 0x0, 0x0,
    0xf, 0xf0, 0xd, 0xf4, 0x0, 0x0, 0x3f, 0xf0,
    0x6, 0xfd, 0x10, 0x1, 0xcf, 0x90, 0x0, 0xbf,
    0xfc, 0xbf, 0xfd, 0x10, 0x0, 0x6, 0xcf, 0xfd,
    0x70, 0x0,

    /* U+0037 "7" */
    0x2f, 0xff, 0xff, 0xff, 0xff, 0x91, 0xbb, 0xbb,
    0xbb, 0xbe, 0xf7, 0x0, 0x0, 0x0, 0x0, 0xef,
    0x10, 0x0, 0x0, 0x0, 0x6f, 0x90, 0x0, 0x0,
    0x0, 0xd, 0xf2, 0x0, 0x0, 0x0, 0x5, 0xfb,
    0x0, 0x0, 0x0, 0x0, 0xcf, 0x40, 0x0, 0x0,
    0x0, 0x4f, 0xd0, 0x0, 0x0, 0x0, 0xb, 0xf6,
    0x0, 0x0, 0x0, 0x2, 0xfe, 0x0, 0x0, 0x0,
    0x0, 0xaf, 0x70, 0x0, 0x0, 0x0, 0x1f, 0xf1,
    0x0, 0x0, 0x0, 0x8, 0xf9, 0x0, 0x0, 0x0,
    0x0, 0xff, 0x20, 0x0, 0x0, 0x0, 0x7f, 0xb0,
    0x0, 0x0, 0x0,

    /* U+0038 "8" */
    0x0, 0x4, 0xbe, 0xfd, 0x70, 0x0, 0x0, 0x7f,
    0xfb, 0xae, 0xfb, 0x0, 0x2, 0xff, 0x30, 0x1,
    0xdf, 0x50, 0x6, 0xfb, 0x0, 0x0, 0x7f, 0xa0,
    0x6, 0xfb, 0x0, 0x0, 0x7f, 0xa0, 0x2, 0xff,
    0x30, 0x1, 0xdf, 0x50, 0x0, 0x6f, 0xfb, 0xae,
    0xfa, 0x0, 0x0, 0x3d, 0xff, 0xff, 0xf6, 0x0,
    0x4, 0xfe, 0x61, 0x4, 0xdf, 0x70, 0xc, 0xf5,
    0x0, 0x0, 0x1f, 0xf1, 0xf, 0xf1, 0x0, 0x0,
    0xd, 0xf4, 0xf, 0xf3, 0x0, 0x0, 0xe, 0xf3,
    0xa, 0xfc, 0x10, 0x0, 0x9f, 0xd0, 0x1, 0xdf,
    0xfb, 0xbe, 0xff, 0x30, 0x0, 0x7, 0xcf, 0xfd,
    0x91, 0x0,

    /* U+0039 "9" */
    0x0, 0x18, 0xdf, 0xeb, 0x40, 0x0, 0x1e, 0xfe,
    0xbc, 0xff, 0x80, 0xb, 0xfa, 0x0, 0x4, 0xff,
    0x31, 0xff, 0x10, 0x0, 0x8, 0xf9, 0x4f, 0xd0,
    0x0, 0x0, 0x5f, 0xb3, 0xfe, 0x0, 0x0, 0x6,
    0xfc, 0x1f, 0xf3, 0x0, 0x0, 0xbf, 0xa0, 0xaf,
    0xd4, 0x2, 0x9f, 0xf6, 0x1, 0xcf, 0xff, 0xff,
    0xfe, 0x0, 0x0, 0x59, 0xa8, 0xdf, 0x50, 0x0,
    0x0, 0x0, 0x6f, 0xb0, 0x0, 0x0, 0x0, 0x3f,
    0xe1, 0x0, 0x0, 0x0, 0x1e, 0xf4, 0x0, 0x0,
    0x0, 0xb, 0xf9, 0x0, 0x0, 0x0, 0x8, 0xfd,
    0x0, 0x0, 0x0,

    /* U+2103 "℃" */
    0x0, 0x48, 0x71, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x8, 0xfe, 0xff, 0x30, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x3f, 0x80, 0x1d, 0xd0,
    0x0, 0x18, 0xdf, 0xfe, 0xa2, 0x0, 0x6f, 0x10,
    0x7, 0xf0, 0x3, 0xef, 0xfb, 0xae, 0xff, 0x40,
    0x5f, 0x40, 0xa, 0xf0, 0x2f, 0xfa, 0x10, 0x0,
    0x7f, 0xe0, 0xd, 0xe8, 0xaf, 0x80, 0xbf, 0xb0,
    0x0, 0x0, 0x9, 0x81, 0x1, 0xae, 0xd7, 0x1,
    0xff, 0x30, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x6, 0xfe, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x8, 0xfb, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x9, 0xfa, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x8,
    0xfc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x5, 0xfe, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1, 0xff, 0x40, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x9f, 0xd1,
    0x0, 0x0, 0xc, 0xd2, 0x0, 0x0, 0x0, 0x0,
    0xd, 0xfd, 0x40, 0x1, 0xaf, 0xd0, 0x0, 0x0,
    0x0, 0x0, 0x1, 0xcf, 0xff, 0xef, 0xfd, 0x20,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x9c, 0xcb,
    0x60, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 188, .box_w = 10, .box_h = 9, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 45, .adv_w = 110, .box_w = 5, .box_h = 2, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 50, .adv_w = 77, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 200, .box_w = 11, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 137, .box_w = 7, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 191, .adv_w = 177, .box_w = 11, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 274, .adv_w = 178, .box_w = 11, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 357, .adv_w = 191, .box_w = 12, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 447, .adv_w = 188, .box_w = 10, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 522, .adv_w = 188, .box_w = 12, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 612, .adv_w = 180, .box_w = 11, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 695, .adv_w = 196, .box_w = 12, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 785, .adv_w = 188, .box_w = 11, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 868, .adv_w = 316, .box_w = 20, .box_h = 17, .ofs_x = 0, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x2, 0x3, 0x5, 0x6, 0x7, 0x8, 0x9,
    0xa, 0xb, 0xc, 0xd, 0xe, 0x20d8
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 43, .range_length = 8409, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 14, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    2, 5,
    2, 6,
    2, 7,
    2, 11,
    2, 13,
    3, 5,
    3, 8,
    3, 10,
    3, 11,
    3, 13,
    4, 11,
    5, 11,
    6, 2,
    7, 5,
    7, 9,
    7, 11,
    7, 13,
    8, 5,
    8, 9,
    8, 11,
    8, 13,
    9, 5,
    9, 10,
    9, 11,
    9, 13,
    10, 5,
    10, 11,
    10, 13,
    11, 2,
    11, 3,
    11, 4,
    11, 8,
    11, 9,
    11, 10,
    11, 11,
    11, 12,
    12, 5,
    12, 11,
    12, 13,
    13, 2,
    13, 3,
    13, 7,
    13, 8,
    13, 9,
    13, 10,
    13, 11,
    13, 12,
    14, 2
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -17, -12, -7, -32, -6, -26, -5, -4,
    -16, -12, -11, -4, -10, -6, -3, -8,
    -5, -6, -5, -15, -6, -6, -2, -10,
    -6, -16, -21, -10, -28, -33, -8, -29,
    -6, -20, 2, -7, -5, -13, -3, -9,
    -19, -3, -12, -3, -9, -9, -2, -13
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 48,
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
const lv_font_t control_center_app_font_20 = {
#else
lv_font_t control_center_app_font_20 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 17,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
    .fallback = NULL,
    .user_data = NULL
};



#endif /*#if CONTROL_CENTER_APP_FONT_20*/

