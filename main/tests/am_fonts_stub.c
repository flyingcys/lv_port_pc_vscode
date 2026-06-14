/* Stub font definitions for unit tests that do not exercise font rendering.
   The metrics test only reads integer/bool/string fields; it never dereferences
   font pointers, so zero-initialised lv_font_t objects satisfy the linker. */
#include "lvgl/lvgl.h"
const lv_font_t am_font_11 = {0};
const lv_font_t am_font_12 = {0};
const lv_font_t am_font_13 = {0};
const lv_font_t am_font_14 = {0};
const lv_font_t am_font_16 = {0};
const lv_font_t am_font_18 = {0};
const lv_font_t am_font_24 = {0};
const lv_font_t am_font_34 = {0};
