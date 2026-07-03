/* main/src/v9_apple_music/am_fonts.h */
#ifndef AM_FONTS_H
#define AM_FONTS_H
#include "lvgl/lvgl.h"
LV_FONT_DECLARE(am_font_11) LV_FONT_DECLARE(am_font_12) LV_FONT_DECLARE(am_font_13)
LV_FONT_DECLARE(am_font_14) LV_FONT_DECLARE(am_font_16) LV_FONT_DECLARE(am_font_18)
LV_FONT_DECLARE(am_font_24) LV_FONT_DECLARE(am_font_34)
LV_FONT_DECLARE(am_font_480_10) LV_FONT_DECLARE(am_font_480_12) LV_FONT_DECLARE(am_font_480_15)
LV_FONT_DECLARE(am_font_480_18) LV_FONT_DECLARE(am_font_480_22)

/* 烘焙火焰封面(188x188 ARGB8888),来源:mockup .cover 的径向火焰 CSS。
 * 运行时用 radius+clip_corner 裁圆角,电台态改用纯色/线性渐变块。 */
LV_IMAGE_DECLARE(am_cover_fire);
#endif
