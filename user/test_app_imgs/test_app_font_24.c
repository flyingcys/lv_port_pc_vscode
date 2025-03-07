/*******************************************************************************
 * Size: 24 px
 * Bpp: 4
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef TEST_APP_FONT_24
#define TEST_APP_FONT_24 1
#endif

#if TEST_APP_FONT_24

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+4E34 "临" */
    0x0, 0x0, 0x0, 0x0, 0x0, 0x28, 0x40, 0x0,
    0x0, 0x0, 0x0, 0x0, 0xf, 0xf3, 0x0, 0x8,
    0xff, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0xff,
    0x30, 0x0, 0xef, 0xa0, 0x0, 0x0, 0x0, 0xc,
    0xd2, 0xf, 0xf3, 0x0, 0x6f, 0xf8, 0x55, 0x55,
    0x55, 0x53, 0xef, 0x30, 0xff, 0x30, 0xd, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xae, 0xf3, 0xf, 0xf3,
    0x5, 0xff, 0xee, 0xee, 0xee, 0xee, 0xe9, 0xef,
    0x30, 0xff, 0x30, 0xef, 0xb0, 0x4d, 0x30, 0x0,
    0x0, 0xe, 0xf3, 0xf, 0xf3, 0x8f, 0xf2, 0xb,
    0xfe, 0x30, 0x0, 0x0, 0xef, 0x30, 0xff, 0x6f,
    0xf8, 0x0, 0xa, 0xff, 0x30, 0x0, 0xe, 0xf3,
    0xf, 0xf8, 0xfd, 0x0, 0x0, 0xa, 0xfa, 0x0,
    0x0, 0xef, 0x30, 0xff, 0x32, 0x74, 0x44, 0x44,
    0x4c, 0x44, 0x41, 0xe, 0xf3, 0xf, 0xf3, 0xe,
    0xff, 0xff, 0xff, 0xff, 0xff, 0x50, 0xef, 0x30,
    0xff, 0x30, 0xef, 0xcc, 0xcf, 0xfc, 0xcf, 0xf5,
    0xe, 0xf3, 0xf, 0xf3, 0xe, 0xf2, 0x1, 0xfe,
    0x0, 0xcf, 0x50, 0xef, 0x30, 0xff, 0x30, 0xef,
    0x20, 0x1f, 0xe0, 0xc, 0xf5, 0xe, 0xf3, 0xf,
    0xf3, 0xe, 0xf2, 0x1, 0xfe, 0x0, 0xcf, 0x50,
    0xef, 0x30, 0xff, 0x30, 0xef, 0x20, 0x1f, 0xe0,
    0xc, 0xf5, 0xe, 0xf3, 0xf, 0xf3, 0xe, 0xf2,
    0x1, 0xfe, 0x0, 0xcf, 0x50, 0xef, 0x30, 0xff,
    0x30, 0xef, 0x20, 0x1f, 0xe0, 0xc, 0xf5, 0xe,
    0xf3, 0xf, 0xf3, 0xe, 0xf3, 0x1, 0xfe, 0x0,
    0xcf, 0x50, 0x67, 0x10, 0xff, 0x30, 0xef, 0xff,
    0xff, 0xff, 0xff, 0xf5, 0x0, 0x0, 0xf, 0xf3,
    0xe, 0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0x0,
    0x0, 0xff, 0x30, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,

    /* U+65F6 "时" */
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x5f, 0xf0, 0x0, 0x45, 0x55, 0x55,
    0x54, 0x0, 0x0, 0x0, 0x5, 0xff, 0x0, 0xe,
    0xff, 0xff, 0xff, 0xe0, 0x0, 0x0, 0x0, 0x5f,
    0xf0, 0x0, 0xef, 0xee, 0xee, 0xfe, 0x0, 0x0,
    0x0, 0x5, 0xff, 0x0, 0xe, 0xf2, 0x0, 0x2f,
    0xe2, 0x55, 0x55, 0x55, 0x8f, 0xf5, 0x54, 0xef,
    0x20, 0x2, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xbe, 0xf2, 0x0, 0x2f, 0xe5, 0xdd, 0xdd,
    0xdd, 0xef, 0xfd, 0xd9, 0xef, 0x20, 0x2, 0xfe,
    0x0, 0x0, 0x0, 0x5, 0xff, 0x0, 0xe, 0xf2,
    0x0, 0x2f, 0xe0, 0x6e, 0x40, 0x0, 0x5f, 0xf0,
    0x0, 0xef, 0xff, 0xff, 0xfe, 0x6, 0xfe, 0x0,
    0x5, 0xff, 0x0, 0xe, 0xff, 0xff, 0xff, 0xe0,
    0xd, 0xf7, 0x0, 0x5f, 0xf0, 0x0, 0xef, 0x42,
    0x24, 0xfe, 0x0, 0x5f, 0xe1, 0x5, 0xff, 0x0,
    0xe, 0xf2, 0x0, 0x2f, 0xe0, 0x0, 0xdf, 0x80,
    0x5f, 0xf0, 0x0, 0xef, 0x20, 0x2, 0xfe, 0x0,
    0x5, 0xff, 0x5, 0xff, 0x0, 0xe, 0xf2, 0x0,
    0x2f, 0xe0, 0x0, 0xd, 0x70, 0x5f, 0xf0, 0x0,
    0xef, 0x20, 0x2, 0xfe, 0x0, 0x0, 0x0, 0x5,
    0xff, 0x0, 0xe, 0xf2, 0x0, 0x2f, 0xe0, 0x0,
    0x0, 0x0, 0x5f, 0xf0, 0x0, 0xef, 0x43, 0x35,
    0xfe, 0x0, 0x0, 0x0, 0x5, 0xff, 0x0, 0xe,
    0xff, 0xff, 0xff, 0xe0, 0x0, 0x0, 0x0, 0x5f,
    0xf0, 0x0, 0xef, 0xff, 0xff, 0xfe, 0x0, 0x3,
    0x66, 0x6b, 0xfe, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x5f, 0xff, 0xff, 0xb0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x2, 0xff, 0xfd, 0xa1,
    0x0, 0x0,

    /* U+6D4B "测" */
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0xac, 0x10, 0x1, 0x11,
    0x11, 0x11, 0x0, 0x0, 0x1, 0xfc, 0x1, 0xef,
    0xe2, 0x1f, 0xff, 0xff, 0xff, 0xa0, 0x0, 0x1,
    0xfc, 0x0, 0x1c, 0xfe, 0x4f, 0xfe, 0xee, 0xef,
    0xa0, 0xaf, 0x11, 0xfc, 0x0, 0x0, 0xcf, 0x5f,
    0xa0, 0x0, 0x1f, 0xa0, 0xaf, 0x11, 0xfc, 0x0,
    0x0, 0x5, 0x1f, 0xa1, 0xd9, 0x1f, 0xa0, 0xaf,
    0x11, 0xfc, 0x3, 0x50, 0x0, 0x1f, 0xa1, 0xfb,
    0x1f, 0xa0, 0xaf, 0x11, 0xfc, 0x1e, 0xf8, 0x0,
    0x1f, 0xa1, 0xfb, 0x1f, 0xa0, 0xaf, 0x11, 0xfc,
    0x8, 0xff, 0x90, 0x1f, 0xa1, 0xfb, 0x1f, 0xa0,
    0xaf, 0x11, 0xfc, 0x0, 0x6f, 0xf9, 0x1f, 0xa1,
    0xfb, 0x1f, 0xa0, 0xaf, 0x11, 0xfc, 0x0, 0x5,
    0xf5, 0x1f, 0xa1, 0xfb, 0x1f, 0xa0, 0xaf, 0x11,
    0xfc, 0x0, 0x0, 0x20, 0x1f, 0xa1, 0xfa, 0x1f,
    0xa0, 0xaf, 0x11, 0xfc, 0x0, 0x0, 0x0, 0x1f,
    0xa1, 0xfa, 0x1f, 0xa0, 0xaf, 0x11, 0xfc, 0x0,
    0x5, 0xb4, 0x1f, 0xa2, 0xf9, 0x1f, 0xa0, 0xaf,
    0x11, 0xfc, 0x0, 0x8, 0xf7, 0x1f, 0xa3, 0xf8,
    0x1f, 0xa0, 0xaf, 0x11, 0xfc, 0x0, 0xc, 0xf4,
    0x1f, 0xa5, 0xf6, 0x1f, 0xa0, 0xaf, 0x11, 0xfc,
    0x0, 0xf, 0xf0, 0xa, 0x69, 0xfe, 0x29, 0x60,
    0xaf, 0x11, 0xfc, 0x0, 0x4f, 0xc0, 0x0, 0x1f,
    0xff, 0xd1, 0x0, 0x1, 0x1, 0xfc, 0x0, 0x8f,
    0x80, 0x0, 0x9f, 0x69, 0xfc, 0x0, 0x0, 0x1,
    0xfc, 0x0, 0xdf, 0x40, 0x6, 0xfc, 0x0, 0xcf,
    0xa0, 0x0, 0x1, 0xfc, 0x1, 0xff, 0x0, 0x7f,
    0xf2, 0x0, 0x1e, 0xf7, 0x0, 0x5, 0xfb, 0x7,
    0xfa, 0x9, 0xff, 0x30, 0x0, 0x4, 0xf4, 0x4f,
    0xff, 0xf9, 0x4, 0xb5, 0x5, 0xe3, 0x0, 0x0,
    0x0, 0x10, 0x1f, 0xed, 0x91, 0x0, 0x0, 0x0,
    0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,

    /* U+8BD5 "试" */
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x18,
    0x70, 0x0, 0x0, 0x0, 0x5, 0xd1, 0x0, 0x0,
    0x0, 0x0, 0x3, 0xfd, 0x1d, 0x70, 0x0, 0x1,
    0xef, 0xc0, 0x0, 0x0, 0x0, 0x0, 0x2f, 0xe2,
    0xef, 0x70, 0x0, 0x3, 0xff, 0x90, 0x0, 0x0,
    0x0, 0x2, 0xfe, 0x2, 0xef, 0x40, 0x0, 0x6,
    0xff, 0x50, 0x0, 0x0, 0x0, 0x1f, 0xf0, 0x3,
    0x90, 0x0, 0x0, 0xa, 0xf5, 0xef, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xf1, 0x0, 0x0, 0x3,
    0xe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x10, 0x0, 0x0, 0x0, 0x22, 0x22, 0x22, 0x22,
    0xff, 0x32, 0x22, 0x20, 0x4c, 0xcc, 0xc3, 0x0,
    0x0, 0x0, 0x0, 0xf, 0xf2, 0x0, 0x0, 0x6,
    0xff, 0xff, 0x50, 0x0, 0x0, 0x0, 0x0, 0xef,
    0x20, 0x0, 0x0, 0x39, 0x9e, 0xf5, 0x4, 0x55,
    0x55, 0x55, 0x2c, 0xf3, 0x0, 0x0, 0x0, 0x0,
    0xcf, 0x50, 0xdf, 0xff, 0xff, 0xf9, 0xbf, 0x50,
    0x0, 0x0, 0x0, 0xc, 0xf5, 0xa, 0xcc, 0xff,
    0xcc, 0x7a, 0xf6, 0x0, 0x0, 0x0, 0x0, 0xcf,
    0x50, 0x0, 0x1f, 0xf0, 0x0, 0x8f, 0x80, 0x0,
    0x0, 0x0, 0xc, 0xf5, 0x0, 0x1, 0xff, 0x0,
    0x6, 0xfa, 0x0, 0x0, 0x0, 0x0, 0xcf, 0x50,
    0x0, 0x1f, 0xf0, 0x0, 0x4f, 0xc0, 0x0, 0x0,
    0x0, 0xc, 0xf5, 0x66, 0x1, 0xff, 0x0, 0x2,
    0xff, 0x0, 0x0, 0x0, 0x0, 0xcf, 0x9f, 0xf0,
    0x1f, 0xf0, 0x2, 0xf, 0xf3, 0x5, 0xd4, 0x0,
    0xc, 0xff, 0xf6, 0x2, 0xff, 0xae, 0xf3, 0xbf,
    0x80, 0x9f, 0x60, 0x0, 0xdf, 0xf6, 0xbe, 0xff,
    0xff, 0xff, 0x46, 0xfe, 0xd, 0xf2, 0x0, 0x3f,
    0xf6, 0xc, 0xff, 0xc9, 0x52, 0x0, 0x1f, 0xfe,
    0xfd, 0x0, 0x0, 0x84, 0x0, 0x22, 0x0, 0x0,
    0x0, 0x0, 0x7f, 0xff, 0x70, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x9f, 0xa0,
    0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 384, .box_w = 21, .box_h = 23, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 242, .adv_w = 384, .box_w = 21, .box_h = 23, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 484, .adv_w = 384, .box_w = 22, .box_h = 24, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 748, .adv_w = 384, .box_w = 23, .box_h = 23, .ofs_x = 0, .ofs_y = -2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x17c2, 0x1f17, 0x3da1
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 20020, .range_length = 15778, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 4, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 4,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t test_app_font_24 = {
#else
lv_font_t test_app_font_24 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 24,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if TEST_APP_FONT_24*/

