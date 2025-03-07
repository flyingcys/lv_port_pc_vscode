/*******************************************************************************
 * Size: 38 px
 * Bpp: 4
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef WATCH_05_FONT_38
#define WATCH_05_FONT_38 1
#endif

#if WATCH_05_FONT_38

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0030 "0" */
    0x0, 0x0, 0x1, 0x7b, 0xef, 0xfd, 0xb6, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x8f, 0xff, 0xff, 0xff,
    0xff, 0xe6, 0x0, 0x0, 0x0, 0xb, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x80, 0x0, 0x0, 0x9f,
    0xff, 0xfd, 0x85, 0x68, 0xef, 0xff, 0xf6, 0x0,
    0x3, 0xff, 0xff, 0x70, 0x0, 0x0, 0xa, 0xff,
    0xff, 0x0, 0xb, 0xff, 0xfa, 0x0, 0x0, 0x0,
    0x0, 0xcf, 0xff, 0x70, 0xf, 0xff, 0xf2, 0x0,
    0x0, 0x0, 0x0, 0x5f, 0xff, 0xc0, 0x4f, 0xff,
    0xd0, 0x0, 0x0, 0x0, 0x0, 0xf, 0xff, 0xf1,
    0x6f, 0xff, 0x90, 0x0, 0x0, 0x0, 0x0, 0xd,
    0xff, 0xf3, 0x8f, 0xff, 0x70, 0x0, 0x0, 0x0,
    0x0, 0xb, 0xff, 0xf5, 0x9f, 0xff, 0x70, 0x0,
    0x0, 0x0, 0x0, 0xa, 0xff, 0xf6, 0x9f, 0xff,
    0x60, 0x0, 0x0, 0x0, 0x0, 0x9, 0xff, 0xf6,
    0xaf, 0xff, 0x60, 0x0, 0x0, 0x0, 0x0, 0x9,
    0xff, 0xf6, 0xaf, 0xff, 0x60, 0x0, 0x0, 0x0,
    0x0, 0x9, 0xff, 0xf6, 0xaf, 0xff, 0x60, 0x0,
    0x0, 0x0, 0x0, 0x9, 0xff, 0xf6, 0xaf, 0xff,
    0x60, 0x0, 0x0, 0x0, 0x0, 0x9, 0xff, 0xf6,
    0x9f, 0xff, 0x60, 0x0, 0x0, 0x0, 0x0, 0xa,
    0xff, 0xf6, 0x9f, 0xff, 0x60, 0x0, 0x0, 0x0,
    0x0, 0xa, 0xff, 0xf6, 0x8f, 0xff, 0x70, 0x0,
    0x0, 0x0, 0x0, 0xb, 0xff, 0xf5, 0x6f, 0xff,
    0x90, 0x0, 0x0, 0x0, 0x0, 0xd, 0xff, 0xf3,
    0x4f, 0xff, 0xd0, 0x0, 0x0, 0x0, 0x0, 0xf,
    0xff, 0xf1, 0xf, 0xff, 0xf2, 0x0, 0x0, 0x0,
    0x0, 0x5f, 0xff, 0xc0, 0xa, 0xff, 0xfa, 0x0,
    0x0, 0x0, 0x0, 0xdf, 0xff, 0x70, 0x3, 0xff,
    0xff, 0x80, 0x0, 0x0, 0xa, 0xff, 0xfe, 0x0,
    0x0, 0x9f, 0xff, 0xfd, 0x75, 0x68, 0xef, 0xff,
    0xf5, 0x0, 0x0, 0xb, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x80, 0x0, 0x0, 0x0, 0x8f, 0xff,
    0xff, 0xff, 0xff, 0xe6, 0x0, 0x0, 0x0, 0x0,
    0x1, 0x7c, 0xef, 0xfe, 0xb6, 0x0, 0x0, 0x0,

    /* U+0031 "1" */
    0x0, 0x0, 0x0, 0x3c, 0xff, 0xd0, 0x0, 0x2,
    0xaf, 0xff, 0xfd, 0x0, 0x18, 0xff, 0xff, 0xff,
    0xd0, 0x7e, 0xff, 0xff, 0xff, 0xfd, 0x6f, 0xff,
    0xff, 0xef, 0xff, 0xd6, 0xff, 0xfe, 0x62, 0xff,
    0xfd, 0x6f, 0xf8, 0x0, 0x2f, 0xff, 0xd6, 0x91,
    0x0, 0x2, 0xff, 0xfd, 0x0, 0x0, 0x0, 0x2f,
    0xff, 0xd0, 0x0, 0x0, 0x2, 0xff, 0xfd, 0x0,
    0x0, 0x0, 0x2f, 0xff, 0xd0, 0x0, 0x0, 0x2,
    0xff, 0xfd, 0x0, 0x0, 0x0, 0x2f, 0xff, 0xd0,
    0x0, 0x0, 0x2, 0xff, 0xfd, 0x0, 0x0, 0x0,
    0x2f, 0xff, 0xd0, 0x0, 0x0, 0x2, 0xff, 0xfd,
    0x0, 0x0, 0x0, 0x2f, 0xff, 0xd0, 0x0, 0x0,
    0x2, 0xff, 0xfd, 0x0, 0x0, 0x0, 0x2f, 0xff,
    0xd0, 0x0, 0x0, 0x2, 0xff, 0xfd, 0x0, 0x0,
    0x0, 0x2f, 0xff, 0xd0, 0x0, 0x0, 0x2, 0xff,
    0xfd, 0x0, 0x0, 0x0, 0x2f, 0xff, 0xd0, 0x0,
    0x0, 0x2, 0xff, 0xfd, 0x0, 0x0, 0x0, 0x2f,
    0xff, 0xd0, 0x0, 0x0, 0x2, 0xff, 0xfd, 0x0,
    0x0, 0x0, 0x2f, 0xff, 0xd0, 0x0, 0x0, 0x2,
    0xff, 0xfd,

    /* U+0032 "2" */
    0x0, 0x0, 0x2, 0x8c, 0xef, 0xfd, 0xa4, 0x0,
    0x0, 0x0, 0x0, 0x1a, 0xff, 0xff, 0xff, 0xff,
    0xfc, 0x20, 0x0, 0x0, 0x2e, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0x20, 0x0, 0x1e, 0xff, 0xff,
    0xa6, 0x56, 0xbf, 0xff, 0xfd, 0x10, 0xa, 0xff,
    0xfd, 0x20, 0x0, 0x0, 0x4f, 0xff, 0xf8, 0x2,
    0xff, 0xfe, 0x10, 0x0, 0x0, 0x0, 0x7f, 0xff,
    0xe0, 0x8f, 0xff, 0x60, 0x0, 0x0, 0x0, 0x1,
    0xff, 0xff, 0x20, 0x5b, 0xe0, 0x0, 0x0, 0x0,
    0x0, 0xf, 0xff, 0xf3, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0xff, 0xff, 0x30, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x1f, 0xff, 0xf1, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x6, 0xff, 0xfd,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xdf,
    0xff, 0x70, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x7f, 0xff, 0xe1, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x4f, 0xff, 0xf5, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x3f, 0xff, 0xf9, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x3f, 0xff, 0xfb, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x3e, 0xff, 0xfc, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x2e, 0xff, 0xfd,
    0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2e, 0xff,
    0xfd, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2e,
    0xff, 0xfd, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x2e, 0xff, 0xfd, 0x10, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x2e, 0xff, 0xfd, 0x10, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x2e, 0xff, 0xfd, 0x10, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1e, 0xff, 0xfd, 0x10, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x1d, 0xff, 0xff, 0x53,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x19, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf5, 0xaf,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x5a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xf5,

    /* U+0033 "3" */
    0x0, 0xaf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x0, 0x0, 0xaf, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0x0, 0x0, 0xaf, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xfe, 0x0, 0x0, 0x23,
    0x33, 0x33, 0x33, 0x33, 0x9f, 0xff, 0xe2, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x6, 0xff, 0xfd,
    0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6f,
    0xff, 0xd1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x7, 0xff, 0xfc, 0x10, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x7f, 0xff, 0xb0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x8, 0xff, 0xfa, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x9f, 0xff, 0x90,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x9, 0xff,
    0xff, 0xfe, 0xd9, 0x30, 0x0, 0x0, 0x0, 0x0,
    0xaf, 0xff, 0xff, 0xff, 0xff, 0xfb, 0x10, 0x0,
    0x0, 0x5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xe2, 0x0, 0x0, 0x0, 0xaf, 0xe6, 0x20, 0x3,
    0x9f, 0xff, 0xfc, 0x0, 0x0, 0x0, 0x9, 0x10,
    0x0, 0x0, 0x4, 0xff, 0xff, 0x70, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x7f, 0xff, 0xe0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf,
    0xff, 0xf3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0xb, 0xff, 0xf5, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0xa, 0xff, 0xf6, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0xa, 0xff, 0xf6,
    0x0, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0xd,
    0xff, 0xf4, 0x4, 0xcf, 0x30, 0x0, 0x0, 0x0,
    0x0, 0x2f, 0xff, 0xf1, 0x2f, 0xff, 0xd1, 0x0,
    0x0, 0x0, 0x0, 0xbf, 0xff, 0xb0, 0xa, 0xff,
    0xfd, 0x20, 0x0, 0x0, 0x9, 0xff, 0xff, 0x30,
    0x1, 0xef, 0xff, 0xfb, 0x75, 0x69, 0xef, 0xff,
    0xf8, 0x0, 0x0, 0x3e, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x90, 0x0, 0x0, 0x1, 0xbf, 0xff,
    0xff, 0xff, 0xff, 0xe6, 0x0, 0x0, 0x0, 0x0,
    0x3, 0x8c, 0xef, 0xfd, 0xa6, 0x0, 0x0, 0x0,

    /* U+0034 "4" */
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc, 0xff,
    0xb0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x8, 0xff, 0xfb, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x4, 0xff, 0xff, 0xb0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x1, 0xef, 0xff, 0xfb,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xbf,
    0xff, 0xff, 0xb0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x6f, 0xff, 0xff, 0xfb, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x2f, 0xff, 0xef, 0xff, 0xb0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0xd, 0xff, 0xf4,
    0xff, 0xfb, 0x0, 0x0, 0x0, 0x0, 0x0, 0x9,
    0xff, 0xf6, 0x1f, 0xff, 0xb0, 0x0, 0x0, 0x0,
    0x0, 0x4, 0xff, 0xfa, 0x1, 0xff, 0xfb, 0x0,
    0x0, 0x0, 0x0, 0x1, 0xef, 0xfd, 0x0, 0x1f,
    0xff, 0xb0, 0x0, 0x0, 0x0, 0x0, 0xbf, 0xff,
    0x30, 0x1, 0xff, 0xfb, 0x0, 0x0, 0x0, 0x0,
    0x7f, 0xff, 0x70, 0x0, 0x1f, 0xff, 0xb0, 0x0,
    0x0, 0x0, 0x3f, 0xff, 0xb0, 0x0, 0x1, 0xff,
    0xfb, 0x0, 0x0, 0x0, 0xd, 0xff, 0xe1, 0x0,
    0x0, 0x1f, 0xff, 0xb0, 0x0, 0x0, 0xa, 0xff,
    0xf3, 0x0, 0x0, 0x1, 0xff, 0xfb, 0x0, 0x0,
    0x5, 0xff, 0xf7, 0x0, 0x0, 0x0, 0x1f, 0xff,
    0xb0, 0x0, 0x2, 0xff, 0xfb, 0x0, 0x0, 0x0,
    0x1, 0xff, 0xfb, 0x0, 0x0, 0xcf, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9e,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xf9, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x94, 0x44, 0x44, 0x44,
    0x44, 0x44, 0x45, 0xff, 0xfc, 0x44, 0x42, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x1f, 0xff, 0xb0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
    0xff, 0xfb, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x1f, 0xff, 0xb0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x1, 0xff, 0xfb, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1f,
    0xff, 0xb0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x1, 0xff, 0xfb, 0x0, 0x0,

    /* U+0035 "5" */
    0x0, 0x5f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xf3, 0x0, 0x7, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x30, 0x0, 0x9f, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xf3, 0x0, 0xb, 0xff, 0xf4,
    0x44, 0x44, 0x44, 0x44, 0x44, 0x0, 0x0, 0xdf,
    0xfd, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0xf, 0xff, 0xa0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x1, 0xff, 0xf8, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x3f, 0xff, 0x60, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x6, 0xff, 0xf3, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x8f, 0xff,
    0x15, 0xae, 0xff, 0xd9, 0x30, 0x0, 0x0, 0xa,
    0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xa0, 0x0,
    0x0, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xd1, 0x0, 0xe, 0xff, 0xff, 0xe8, 0x65, 0x7d,
    0xff, 0xff, 0xb0, 0x0, 0xff, 0xff, 0x80, 0x0,
    0x0, 0x8, 0xff, 0xff, 0x60, 0x3, 0x8c, 0x80,
    0x0, 0x0, 0x0, 0xa, 0xff, 0xfd, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x1f, 0xff, 0xf2,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xcf,
    0xff, 0x50, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x9, 0xff, 0xf7, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x9f, 0xff, 0x80, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x9, 0xff, 0xf7, 0x0, 0x12,
    0x0, 0x0, 0x0, 0x0, 0x0, 0xcf, 0xff, 0x51,
    0x8f, 0xd0, 0x0, 0x0, 0x0, 0x0, 0x1f, 0xff,
    0xf1, 0xaf, 0xff, 0x70, 0x0, 0x0, 0x0, 0x9,
    0xff, 0xfc, 0x4, 0xff, 0xff, 0x80, 0x0, 0x0,
    0x7, 0xff, 0xff, 0x50, 0x9, 0xff, 0xff, 0xd8,
    0x55, 0x7d, 0xff, 0xff, 0x90, 0x0, 0xb, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xb0, 0x0, 0x0,
    0x7, 0xff, 0xff, 0xff, 0xff, 0xff, 0x70, 0x0,
    0x0, 0x0, 0x1, 0x7b, 0xef, 0xfe, 0xb7, 0x10,
    0x0, 0x0,

    /* U+0036 "6" */
    0x0, 0x0, 0x0, 0x0, 0x0, 0x6f, 0xff, 0xf2,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2f,
    0xff, 0xf5, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0xd, 0xff, 0xf8, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x9, 0xff, 0xfb, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x5, 0xff, 0xfd,
    0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2,
    0xff, 0xff, 0x20, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xcf, 0xff, 0x50, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x8f, 0xff, 0x80, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4f, 0xff,
    0xb0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x1e, 0xff, 0xd0, 0x14, 0x43, 0x10, 0x0, 0x0,
    0x0, 0x0, 0xa, 0xff, 0xfa, 0xdf, 0xff, 0xff,
    0xc6, 0x0, 0x0, 0x0, 0x3, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfb, 0x10, 0x0, 0x0, 0xcf,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0x0,
    0x0, 0x3f, 0xff, 0xff, 0xb4, 0x10, 0x15, 0xcf,
    0xff, 0xfa, 0x0, 0x8, 0xff, 0xff, 0x70, 0x0,
    0x0, 0x0, 0x8f, 0xff, 0xf3, 0x0, 0xcf, 0xff,
    0xb0, 0x0, 0x0, 0x0, 0x0, 0xbf, 0xff, 0x90,
    0xf, 0xff, 0xf3, 0x0, 0x0, 0x0, 0x0, 0x4,
    0xff, 0xfd, 0x1, 0xff, 0xff, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xf, 0xff, 0xf0, 0x2f, 0xff, 0xe0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0xef, 0xff, 0x11,
    0xff, 0xfe, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf,
    0xff, 0xf0, 0xf, 0xff, 0xf1, 0x0, 0x0, 0x0,
    0x0, 0x1, 0xff, 0xff, 0x0, 0xcf, 0xff, 0x50,
    0x0, 0x0, 0x0, 0x0, 0x6f, 0xff, 0xb0, 0x7,
    0xff, 0xfd, 0x10, 0x0, 0x0, 0x0, 0x1e, 0xff,
    0xf6, 0x0, 0xe, 0xff, 0xfc, 0x20, 0x0, 0x0,
    0x2d, 0xff, 0xfe, 0x0, 0x0, 0x5f, 0xff, 0xff,
    0xa7, 0x67, 0xaf, 0xff, 0xff, 0x30, 0x0, 0x0,
    0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x50,
    0x0, 0x0, 0x0, 0x4d, 0xff, 0xff, 0xff, 0xff,
    0xfd, 0x20, 0x0, 0x0, 0x0, 0x0, 0x5, 0xad,
    0xff, 0xfd, 0x94, 0x0, 0x0, 0x0,

    /* U+0037 "7" */
    0x5f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xf1, 0x5f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xf1, 0x5f, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0x25, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x7f, 0xff, 0xb0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x9f,
    0xff, 0x30, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x1, 0xff, 0xfc, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x8, 0xff, 0xf5, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0xe, 0xff, 0xd0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7f, 0xff,
    0x60, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0xef, 0xfe, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x6, 0xff, 0xf8, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0xd, 0xff, 0xf1, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x5f, 0xff, 0x90,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xcf,
    0xff, 0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x4, 0xff, 0xfb, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xb, 0xff, 0xf3, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x3f, 0xff, 0xc0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xaf, 0xff,
    0x50, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2,
    0xff, 0xfd, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x9, 0xff, 0xf6, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1f, 0xff, 0xe0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x8f, 0xff, 0x80,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xff,
    0xff, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x7, 0xff, 0xf9, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xe, 0xff, 0xf2, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x6f, 0xff, 0xa0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xef, 0xff,
    0x30, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5,
    0xff, 0xfc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,

    /* U+0038 "8" */
    0x0, 0x0, 0x0, 0x38, 0xce, 0xfe, 0xda, 0x50,
    0x0, 0x0, 0x0, 0x0, 0x2, 0xbf, 0xff, 0xff,
    0xff, 0xff, 0xd4, 0x0, 0x0, 0x0, 0x3, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf6, 0x0, 0x0,
    0x1, 0xef, 0xff, 0xf9, 0x53, 0x47, 0xdf, 0xff,
    0xf4, 0x0, 0x0, 0x9f, 0xff, 0xd2, 0x0, 0x0,
    0x0, 0xaf, 0xff, 0xd0, 0x0, 0xe, 0xff, 0xf4,
    0x0, 0x0, 0x0, 0x0, 0xef, 0xff, 0x20, 0x0,
    0xff, 0xff, 0x0, 0x0, 0x0, 0x0, 0xa, 0xff,
    0xf4, 0x0, 0xf, 0xff, 0xf0, 0x0, 0x0, 0x0,
    0x0, 0xaf, 0xff, 0x30, 0x0, 0xdf, 0xff, 0x40,
    0x0, 0x0, 0x0, 0xe, 0xff, 0xf1, 0x0, 0x6,
    0xff, 0xfd, 0x10, 0x0, 0x0, 0x9, 0xff, 0xfa,
    0x0, 0x0, 0xb, 0xff, 0xfe, 0x84, 0x33, 0x6d,
    0xff, 0xfe, 0x20, 0x0, 0x0, 0xa, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfd, 0x20, 0x0, 0x0, 0x0,
    0x1b, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x20, 0x0,
    0x0, 0x0, 0x4e, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x80, 0x0, 0x0, 0x5f, 0xff, 0xfb, 0x51,
    0x0, 0x38, 0xff, 0xff, 0x80, 0x0, 0x1f, 0xff,
    0xf6, 0x0, 0x0, 0x0, 0x2, 0xef, 0xff, 0x50,
    0x9, 0xff, 0xf9, 0x0, 0x0, 0x0, 0x0, 0x3,
    0xff, 0xfe, 0x0, 0xef, 0xff, 0x10, 0x0, 0x0,
    0x0, 0x0, 0xb, 0xff, 0xf3, 0x2f, 0xff, 0xe0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x8f, 0xff, 0x63,
    0xff, 0xfd, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7,
    0xff, 0xf7, 0x2f, 0xff, 0xe0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x9f, 0xff, 0x70, 0xff, 0xff, 0x30,
    0x0, 0x0, 0x0, 0x0, 0xd, 0xff, 0xf4, 0xa,
    0xff, 0xfc, 0x0, 0x0, 0x0, 0x0, 0x6, 0xff,
    0xff, 0x0, 0x3f, 0xff, 0xfb, 0x10, 0x0, 0x0,
    0x6, 0xff, 0xff, 0x70, 0x0, 0x7f, 0xff, 0xff,
    0x96, 0x45, 0x8d, 0xff, 0xff, 0xc0, 0x0, 0x0,
    0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc1,
    0x0, 0x0, 0x0, 0x4d, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x80, 0x0, 0x0, 0x0, 0x0, 0x4, 0x9d,
    0xff, 0xfd, 0xb6, 0x10, 0x0, 0x0,

    /* U+0039 "9" */
    0x0, 0x0, 0x2, 0x8c, 0xef, 0xfe, 0xb7, 0x10,
    0x0, 0x0, 0x0, 0x1, 0xaf, 0xff, 0xff, 0xff,
    0xff, 0xf8, 0x0, 0x0, 0x0, 0x2e, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xc0, 0x0, 0x1, 0xdf,
    0xff, 0xfc, 0x75, 0x58, 0xdf, 0xff, 0xfb, 0x0,
    0xa, 0xff, 0xfe, 0x40, 0x0, 0x0, 0x6, 0xff,
    0xff, 0x50, 0x1f, 0xff, 0xf3, 0x0, 0x0, 0x0,
    0x0, 0x6f, 0xff, 0xd0, 0x6f, 0xff, 0xb0, 0x0,
    0x0, 0x0, 0x0, 0xd, 0xff, 0xf2, 0x9f, 0xff,
    0x60, 0x0, 0x0, 0x0, 0x0, 0x9, 0xff, 0xf6,
    0xaf, 0xff, 0x50, 0x0, 0x0, 0x0, 0x0, 0x7,
    0xff, 0xf8, 0xaf, 0xff, 0x60, 0x0, 0x0, 0x0,
    0x0, 0x8, 0xff, 0xf8, 0x8f, 0xff, 0x90, 0x0,
    0x0, 0x0, 0x0, 0xb, 0xff, 0xf7, 0x4f, 0xff,
    0xe1, 0x0, 0x0, 0x0, 0x0, 0x2f, 0xff, 0xf6,
    0xe, 0xff, 0xfb, 0x0, 0x0, 0x0, 0x1, 0xdf,
    0xff, 0xf2, 0x6, 0xff, 0xff, 0xd6, 0x20, 0x2,
    0x7e, 0xff, 0xff, 0xe0, 0x0, 0xaf, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0x90, 0x0, 0x9,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x20,
    0x0, 0x0, 0x4b, 0xff, 0xff, 0xff, 0xae, 0xff,
    0xf9, 0x0, 0x0, 0x0, 0x0, 0x3, 0x55, 0x30,
    0x6f, 0xff, 0xe1, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x3, 0xff, 0xff, 0x50, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0xd, 0xff, 0xfa, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0xaf, 0xff, 0xd0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5, 0xff,
    0xff, 0x30, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x2f, 0xff, 0xf7, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xcf, 0xff, 0xb0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x8, 0xff, 0xfe, 0x10, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x4f, 0xff, 0xf4,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xef,
    0xff, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0xb, 0xff, 0xfc, 0x0, 0x0, 0x0, 0x0, 0x0,

    /* U+003A ":" */
    0x8, 0xee, 0x70, 0x4f, 0xff, 0xf3, 0x7f, 0xff,
    0xf6, 0x4f, 0xff, 0xf3, 0x7, 0xee, 0x70, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0xee, 0x70,
    0x4f, 0xff, 0xf3, 0x7f, 0xff, 0xf6, 0x4f, 0xff,
    0xf3, 0x7, 0xee, 0x70
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 381, .box_w = 20, .box_h = 28, .ofs_x = 2, .ofs_y = 1},
    {.bitmap_index = 280, .adv_w = 260, .box_w = 11, .box_h = 28, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 434, .adv_w = 336, .box_w = 19, .box_h = 28, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 700, .adv_w = 339, .box_w = 20, .box_h = 28, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 980, .adv_w = 363, .box_w = 21, .box_h = 28, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 1274, .adv_w = 358, .box_w = 19, .box_h = 28, .ofs_x = 2, .ofs_y = 1},
    {.bitmap_index = 1540, .adv_w = 358, .box_w = 21, .box_h = 28, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 1834, .adv_w = 343, .box_w = 20, .box_h = 28, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 2114, .adv_w = 373, .box_w = 21, .box_h = 28, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 2408, .adv_w = 357, .box_w = 20, .box_h = 28, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 2688, .adv_w = 159, .box_w = 6, .box_h = 20, .ofs_x = 2, .ofs_y = 1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 48, .range_length = 11, .glyph_id_start = 1,
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
    -20, -9, -11, -6, -15, -10, -12, -10,
    -28, -11, -12, -3, -18, -12, -30, -40,
    -18, -15, -55, -11, -38, 4, -13, -9,
    -25, -6, -5, -23, -5, -18, -17, -4
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

#if LVGL_VERSION_MAJOR >= 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
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
#if LVGL_VERSION_MAJOR >= 8
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t watch_05_font_38 = {
#else
lv_font_t watch_05_font_38 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 28,          /*The maximum line height required by the font*/
    .base_line = -1,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -3,
    .underline_thickness = 2,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
    .fallback = NULL,
    .user_data = NULL
};



#endif /*#if WATCH_05_FONT_38*/

