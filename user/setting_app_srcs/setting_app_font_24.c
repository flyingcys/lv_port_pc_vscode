/*******************************************************************************
 * Size: 24 px
 * Bpp: 4
 * Opts: --bpp 4 --size 24 --no-compress --font OPlusSans3-Medium.ttf --symbols 设置关于息屏秒 --format lvgl -o setting_app_font_24.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef SETTING_APP_FONT_24
#define SETTING_APP_FONT_24 1
#endif

#if SETTING_APP_FONT_24

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+4E8E "于" */
    0x2, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xfb, 0x0, 0x2, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xfb, 0x0, 0x0, 0x22,
    0x22, 0x22, 0x23, 0xff, 0x62, 0x22, 0x22, 0x22,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xff, 0x40,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x1, 0xff, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1, 0xff, 0x40, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xff,
    0x40, 0x0, 0x0, 0x0, 0x0, 0x23, 0x33, 0x33,
    0x33, 0x34, 0xff, 0x73, 0x33, 0x33, 0x33, 0x31,
    0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xf7, 0xef, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 0x0, 0x0,
    0x0, 0x0, 0x1, 0xff, 0x40, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xff, 0x40,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x1, 0xff, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1, 0xff, 0x40, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xff,
    0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x1, 0xff, 0x40, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x1, 0xff, 0x40, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
    0xff, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x3, 0x66, 0x69, 0xff, 0x30, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x5, 0xff, 0xff, 0xff, 0x10,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0xfe,
    0xed, 0xa3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,

    /* U+5173 "关" */
    0x0, 0x0, 0x1a, 0x40, 0x0, 0x0, 0x0, 0x8,
    0x50, 0x0, 0x0, 0x0, 0x0, 0xaf, 0xe1, 0x0,
    0x0, 0x0, 0x3f, 0xf7, 0x0, 0x0, 0x0, 0x0,
    0x1e, 0xfc, 0x0, 0x0, 0x0, 0xdf, 0xe0, 0x0,
    0x0, 0x0, 0x0, 0x4, 0xff, 0x70, 0x0, 0xa,
    0xff, 0x30, 0x0, 0x0, 0x0, 0x0, 0x0, 0x9d,
    0x40, 0x0, 0x7f, 0xf7, 0x0, 0x0, 0x0, 0x5,
    0xee, 0xee, 0xff, 0xee, 0xee, 0xff, 0xfe, 0xee,
    0xec, 0x0, 0x5, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfe, 0x0, 0x1, 0x33, 0x33,
    0x33, 0x38, 0xff, 0x33, 0x33, 0x33, 0x33, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x6, 0xff, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6,
    0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x6, 0xff, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x25, 0x55, 0x55, 0x55, 0x59, 0xff, 0x55,
    0x55, 0x55, 0x55, 0x50, 0x8f, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0x7f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xf1, 0x0, 0x0, 0x0, 0x0, 0xf, 0xff,
    0x60, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x8f, 0xff, 0xd0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x2, 0xff, 0xbf, 0xf9, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2e, 0xfe,
    0x17, 0xff, 0x70, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x4, 0xef, 0xf3, 0x0, 0xaf, 0xf9, 0x0, 0x0,
    0x0, 0x0, 0x2, 0xbf, 0xfe, 0x30, 0x0, 0xb,
    0xff, 0xd4, 0x0, 0x0, 0x3, 0xaf, 0xff, 0xc1,
    0x0, 0x0, 0x0, 0x8f, 0xff, 0xd6, 0x10, 0xaf,
    0xff, 0xe6, 0x0, 0x0, 0x0, 0x0, 0x4, 0xdf,
    0xff, 0xf5, 0x5f, 0xe8, 0x10, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x5, 0xcf, 0xe0, 0x5, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x40,

    /* U+5C4F "屏" */
    0x0, 0x1, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x10, 0x0, 0x6, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x0, 0x0,
    0x6f, 0xfc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xdf, 0xe0, 0x0, 0x6, 0xfd, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x5, 0xfe, 0x0, 0x0, 0x6f,
    0xfb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xcf,
    0xe0, 0x0, 0x6, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfe, 0x0, 0x0, 0x6f, 0xe2,
    0x22, 0x3b, 0x92, 0x22, 0x24, 0xd7, 0x22, 0x20,
    0x0, 0x6, 0xfd, 0x0, 0x4, 0xff, 0x20, 0x0,
    0xbf, 0xe1, 0x0, 0x0, 0x0, 0x6f, 0xd0, 0x0,
    0x9, 0xf8, 0x0, 0x5f, 0xf6, 0x0, 0x0, 0x0,
    0x6, 0xfd, 0x7e, 0xee, 0xef, 0xee, 0xef, 0xff,
    0xee, 0xee, 0x40, 0x0, 0x6f, 0xd8, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf5, 0x0, 0x7,
    0xfc, 0x0, 0x4, 0xfe, 0x0, 0x0, 0xef, 0x40,
    0x0, 0x0, 0x0, 0x8f, 0xb0, 0x0, 0x3f, 0xe0,
    0x0, 0xe, 0xf3, 0x0, 0x0, 0x0, 0x9, 0xfa,
    0x0, 0x3, 0xfe, 0x0, 0x0, 0xef, 0x30, 0x0,
    0x0, 0x0, 0xbf, 0xbe, 0xee, 0xff, 0xfe, 0xee,
    0xef, 0xff, 0xee, 0xee, 0x10, 0xe, 0xf9, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1,
    0x1, 0xff, 0x30, 0x0, 0x8f, 0xa0, 0x0, 0xe,
    0xf4, 0x0, 0x0, 0x0, 0x6f, 0xe0, 0x0, 0xc,
    0xf7, 0x0, 0x0, 0xef, 0x30, 0x0, 0x0, 0xb,
    0xfa, 0x0, 0x4, 0xff, 0x10, 0x0, 0xe, 0xf3,
    0x0, 0x0, 0x2, 0xff, 0x50, 0x3, 0xff, 0x90,
    0x0, 0x0, 0xef, 0x30, 0x0, 0x0, 0xaf, 0xe0,
    0x4, 0xff, 0xc0, 0x0, 0x0, 0xe, 0xf3, 0x0,
    0x0, 0x5, 0xd7, 0x0, 0x3f, 0xd1, 0x0, 0x0,
    0x0, 0xef, 0x30, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x30, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0,

    /* U+606F "息" */
    0x0, 0x0, 0x0, 0x0, 0x0, 0x43, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x1f, 0xf5, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x22, 0x22, 0x26, 0xff, 0x22, 0x22, 0x22,
    0x22, 0x0, 0x0, 0x0, 0x3f, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xf0, 0x0, 0x0, 0x3,
    0xff, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xff,
    0x0, 0x0, 0x0, 0x3f, 0xe0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x1f, 0xf0, 0x0, 0x0, 0x3, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0,
    0x0, 0x0, 0x3f, 0xfc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcf, 0xf0, 0x0, 0x0, 0x3, 0xfe, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x1, 0xff, 0x0, 0x0,
    0x0, 0x3f, 0xfa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xbf, 0xf0, 0x0, 0x0, 0x3, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0x0, 0x0, 0x0,
    0x3f, 0xe0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2f,
    0xf0, 0x0, 0x0, 0x3, 0xfe, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x2, 0xff, 0x0, 0x0, 0x0, 0x3f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,
    0x0, 0x0, 0x2, 0xdd, 0xdd, 0xdd, 0xde, 0xdd,
    0xdd, 0xdd, 0xdd, 0x0, 0x0, 0x0, 0x10, 0x0,
    0x0, 0x4, 0xfb, 0x0, 0x0, 0x0, 0x20, 0x0,
    0x0, 0xa, 0xe5, 0x4f, 0xe0, 0x3f, 0xf7, 0x0,
    0x0, 0xbf, 0x20, 0x0, 0x1, 0xff, 0x34, 0xfe,
    0x0, 0x7f, 0xf2, 0x0, 0xb, 0xfc, 0x0, 0x0,
    0x7f, 0xc0, 0x4f, 0xe0, 0x0, 0xcf, 0x73, 0xb5,
    0x1f, 0xf6, 0x0, 0xe, 0xf5, 0x4, 0xfe, 0x0,
    0x2, 0x60, 0x5f, 0xc0, 0x6f, 0xf1, 0x8, 0xfe,
    0x0, 0x3f, 0xf2, 0x0, 0x0, 0x9, 0xfa, 0x0,
    0xdf, 0xa0, 0xcf, 0x60, 0x1, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x50, 0x4, 0xf6, 0x0, 0x50, 0x0,
    0x7, 0xdf, 0xff, 0xff, 0xfd, 0x80, 0x0, 0x1,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0,

    /* U+79D2 "秒" */
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x15,
    0x50, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2,
    0x75, 0x0, 0x0, 0x3f, 0xe0, 0x0, 0x0, 0x0,
    0x0, 0x24, 0x7a, 0xef, 0xfd, 0x0, 0x0, 0x3f,
    0xe0, 0x0, 0x0, 0x0, 0x7, 0xff, 0xff, 0xff,
    0xc8, 0x0, 0x0, 0x3f, 0xe0, 0x14, 0x0, 0x0,
    0x2, 0xfc, 0xcf, 0xd0, 0x0, 0x1, 0xfb, 0x4f,
    0xe3, 0xff, 0x10, 0x0, 0x0, 0x0, 0x4f, 0xd0,
    0x0, 0x5, 0xfd, 0x3f, 0xe0, 0xdf, 0x90, 0x0,
    0x0, 0x0, 0x4f, 0xd0, 0x0, 0x9, 0xf9, 0x3f,
    0xe0, 0x4f, 0xf2, 0x0, 0x8, 0xbb, 0xdf, 0xfb,
    0xbb, 0xe, 0xf4, 0x3f, 0xe0, 0xc, 0xfa, 0x0,
    0xb, 0xff, 0xff, 0xff, 0xff, 0x4f, 0xf0, 0x3f,
    0xe0, 0x4, 0xff, 0x20, 0x4, 0x66, 0xbf, 0xe6,
    0x66, 0xaf, 0xa0, 0x3f, 0xe0, 0x0, 0xdf, 0x90,
    0x0, 0x0, 0xdf, 0xd0, 0x1, 0xff, 0x40, 0x3f,
    0xe0, 0x0, 0x6f, 0xf0, 0x0, 0x3, 0xff, 0xf7,
    0x8, 0xfd, 0x0, 0x3f, 0xe0, 0x2, 0x1f, 0xf4,
    0x0, 0xb, 0xff, 0xff, 0x7a, 0xf6, 0x0, 0x3f,
    0xe0, 0xd, 0xd8, 0x10, 0x0, 0x3f, 0xff, 0xef,
    0xf7, 0x30, 0x0, 0x3f, 0xe0, 0x8f, 0xe1, 0x0,
    0x0, 0xcf, 0xaf, 0xd5, 0xff, 0x30, 0x0, 0x3f,
    0xe4, 0xff, 0x60, 0x0, 0x5, 0xfe, 0x5f, 0xd0,
    0x89, 0x0, 0x0, 0x0, 0x3f, 0xfa, 0x0, 0x0,
    0xe, 0xf6, 0x4f, 0xd0, 0x0, 0x0, 0x0, 0x4,
    0xff, 0xc0, 0x0, 0x0, 0x4f, 0xd0, 0x4f, 0xd0,
    0x0, 0x0, 0x0, 0x7f, 0xfd, 0x10, 0x0, 0x0,
    0x4, 0x40, 0x4f, 0xd0, 0x0, 0x0, 0x2c, 0xff,
    0xc1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4f, 0xd0,
    0x0, 0x29, 0xff, 0xf9, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x4f, 0xd0, 0x3a, 0xff, 0xfd, 0x40,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4f, 0xd4,
    0xff, 0xfe, 0x70, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x4f, 0xd0, 0xcd, 0x70, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,

    /* U+7F6E "置" */
    0x0, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
    0x66, 0x66, 0x20, 0x0, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0x50, 0x0, 0xff,
    0x0, 0x7, 0xf8, 0x0, 0x2f, 0xd0, 0x0, 0xbf,
    0x50, 0x0, 0xff, 0x0, 0x7, 0xf8, 0x0, 0x2f,
    0xd0, 0x0, 0xbf, 0x50, 0x0, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x50, 0x0,
    0x77, 0x77, 0x77, 0x77, 0xab, 0x87, 0x77, 0x77,
    0x77, 0x20, 0x1, 0x22, 0x22, 0x22, 0x22, 0xef,
    0x52, 0x22, 0x22, 0x22, 0x20, 0xb, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1,
    0x3, 0x44, 0x44, 0x44, 0x44, 0xef, 0x74, 0x44,
    0x44, 0x44, 0x40, 0x0, 0x3, 0x77, 0x77, 0x77,
    0xff, 0x97, 0x77, 0x77, 0x60, 0x0, 0x0, 0x7,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0,
    0x0, 0x0, 0x7, 0xfa, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x5f, 0xc0, 0x0, 0x0, 0x7, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x0, 0x0,
    0x7, 0xfc, 0x66, 0x66, 0x66, 0x66, 0x66, 0x9f,
    0xc0, 0x0, 0x0, 0x7, 0xfc, 0x66, 0x66, 0x66,
    0x66, 0x66, 0x9f, 0xc0, 0x0, 0x0, 0x7, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x0,
    0x0, 0x7, 0xfa, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x5f, 0xc0, 0x0, 0x0, 0x7, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xc0, 0x0, 0x0, 0x7,
    0xfc, 0x66, 0x66, 0x66, 0x66, 0x66, 0x9f, 0xc0,
    0x0, 0x0, 0x7, 0xfa, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x5f, 0xc0, 0x0, 0x59, 0x9c, 0xfd, 0x99,
    0x99, 0x99, 0x99, 0x99, 0xbf, 0xe9, 0x98, 0x9f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xfe,

    /* U+8BBE "设" */
    0x0, 0x3b, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1, 0xff, 0xc0, 0x0, 0x1,
    0xff, 0xff, 0xff, 0xff, 0x70, 0x0, 0x0, 0x5f,
    0xfa, 0x0, 0x1, 0xff, 0xff, 0xff, 0xff, 0x70,
    0x0, 0x0, 0x8, 0xff, 0x80, 0x1, 0xff, 0x0,
    0x0, 0xbf, 0x70, 0x0, 0x0, 0x0, 0xbf, 0xd0,
    0x3, 0xff, 0x0, 0x0, 0xbf, 0x70, 0x0, 0x0,
    0x0, 0x1a, 0x0, 0x7, 0xfd, 0x0, 0x0, 0xbf,
    0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2e, 0xf8,
    0x0, 0x0, 0xaf, 0xfe, 0xe9, 0x88, 0x88, 0x82,
    0x3, 0xef, 0xd1, 0x0, 0x0, 0x3e, 0xff, 0xf7,
    0xff, 0xff, 0xf4, 0x8, 0xfe, 0x20, 0x0, 0x0,
    0x0, 0x23, 0x41, 0xbc, 0xcf, 0xf4, 0x0, 0xa5,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x0, 0x0, 0xe,
    0xf4, 0x4, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x0, 0x0, 0xe, 0xf4, 0x3, 0xdf, 0xfd, 0xdd,
    0xdd, 0xde, 0xff, 0x0, 0x0, 0xe, 0xf4, 0x0,
    0x1f, 0xf6, 0x0, 0x0, 0xb, 0xfa, 0x0, 0x0,
    0xe, 0xf4, 0x0, 0x8, 0xfe, 0x0, 0x0, 0x4f,
    0xf2, 0x0, 0x0, 0xe, 0xf4, 0x0, 0x0, 0xef,
    0x90, 0x1, 0xef, 0x90, 0x0, 0x0, 0xe, 0xf4,
    0x6, 0x0, 0x3f, 0xf8, 0x1c, 0xfd, 0x0, 0x0,
    0x0, 0xe, 0xf4, 0x6f, 0x50, 0x6, 0xff, 0xef,
    0xe2, 0x0, 0x0, 0x0, 0xe, 0xfb, 0xff, 0x70,
    0x0, 0xbf, 0xff, 0x40, 0x0, 0x0, 0x0, 0xf,
    0xff, 0xf6, 0x0, 0x3c, 0xff, 0xff, 0xf7, 0x0,
    0x0, 0x0, 0x4f, 0xfe, 0x40, 0x4b, 0xff, 0xf8,
    0x3c, 0xff, 0xe8, 0x20, 0x0, 0x2f, 0xb1, 0x5e,
    0xff, 0xfb, 0x20, 0x0, 0x6e, 0xff, 0xfa, 0x0,
    0x3, 0x0, 0x2f, 0xfb, 0x30, 0x0, 0x0, 0x0,
    0x7e, 0xf6, 0x0, 0x0, 0x0, 0x5, 0x20, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x40
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 384, .box_w = 22, .box_h = 22, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 242, .adv_w = 384, .box_w = 22, .box_h = 24, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 506, .adv_w = 384, .box_w = 23, .box_h = 23, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 771, .adv_w = 384, .box_w = 23, .box_h = 24, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1047, .adv_w = 384, .box_w = 24, .box_h = 24, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1335, .adv_w = 384, .box_w = 22, .box_h = 22, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1577, .adv_w = 384, .box_w = 22, .box_h = 23, .ofs_x = 1, .ofs_y = -3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x2e5, 0xdc1, 0x11e1, 0x2b44, 0x30e0, 0x3d30
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 20110, .range_length = 15665, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 7, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
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
const lv_font_t setting_app_font_24 = {
#else
lv_font_t setting_app_font_24 = {
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
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if SETTING_APP_FONT_24*/

