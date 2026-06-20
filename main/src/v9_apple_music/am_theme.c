#include "am_theme.h"

/* lv_color_t is {blue, green, red} — use a compile-time constant initializer
 * instead of lv_color_hex() (which is an inline function, not a constant expr) */
#define C(hex) { (uint8_t)((hex) & 0xFF), (uint8_t)(((hex) >> 8) & 0xFF), (uint8_t)(((hex) >> 16) & 0xFF) }

static const am_theme_t g_themes[AM_THEME_COUNT] = {
 [AM_THEME_CYAN]={"cyan",
    C(0xeef8f5), C(0xe3edf1),
    C(0x4bbcae), C(0xd7f5ef),
    C(0xa8ebe0), C(0x59c7ba), C(0x2b7373),
    C(0xdaf6f0), C(0xf1fffc), C(0xdff4fb)},
 [AM_THEME_BLUE]={"blue",
    C(0xeef4ff), C(0xe5ebfb),
    C(0x4c78ff), C(0xdbe5ff),
    C(0x91b3ff), C(0x567cff), C(0x2b3b81),
    C(0xdce8ff), C(0xeff3ff), C(0xdff6ff)},
 [AM_THEME_MINT]={"mint",
    C(0xedf8f4), C(0xe5eff2),
    C(0x23b497), C(0xd4f7ee),
    C(0x7ae1cc), C(0x28b89c), C(0x1a6662),
    C(0xd9f8ef), C(0xf0fffb), C(0xddf4ff)},
 [AM_THEME_ORANGE]={"orange",
    C(0xfff3e7), C(0xf2ebea),
    C(0xf08d3c), C(0xffe6cf),
    C(0xffc66f), C(0xf18b47), C(0x8f4a32),
    C(0xffe5cc), C(0xfff6ef), C(0xffeedc)},
};

#undef C

static am_theme_id_t g_current = AM_THEME_MINT;  /* 默认 mint */

const am_theme_t *am_theme_get(am_theme_id_t id){ return &g_themes[id]; }
am_theme_id_t am_theme_current(void){ return g_current; }
void am_theme_set(am_theme_id_t id){ if(id < AM_THEME_COUNT) g_current = id; }
