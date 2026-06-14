/* main/src/v9_apple_music/am_theme.h */
#ifndef AM_THEME_H
#define AM_THEME_H
#include "lvgl/lvgl.h"

typedef enum { AM_THEME_CYAN, AM_THEME_BLUE, AM_THEME_MINT, AM_THEME_ORANGE, AM_THEME_COUNT } am_theme_id_t;

typedef struct {
    const char *id;
    lv_color_t bg_top, bg_bottom;
    lv_color_t accent, accent_soft;
    lv_color_t hero_a, hero_b, hero_c;
    lv_color_t tile_a, tile_b, tile_c;
} am_theme_t;

/* 共享色(不随主题变) */
#define AM_TEXT          lv_color_hex(0x21242e)
#define AM_MUTED         lv_color_hex(0x7a8194)
#define AM_MUTED_STRONG  lv_color_hex(0x5f6678)
#define AM_WHITE         lv_color_hex(0xffffff)
/* rgba 透明度:bg_color + bg_opa,如 surface=白62% -> AM_OPA_SURFACE */
#define AM_OPA_SURFACE        158  /* 0.62*255 */
#define AM_OPA_SURFACE_STRONG 219  /* 0.86 */
#define AM_OPA_SURFACE_SOFT   112  /* 0.44 */
#define AM_OPA_BORDER         173  /* 0.68 */
#define AM_OPA_ACCENT_12       31  /* 0.12 */

const am_theme_t *am_theme_get(am_theme_id_t id);
am_theme_id_t     am_theme_current(void);
void              am_theme_set(am_theme_id_t id);

#endif /* AM_THEME_H */
