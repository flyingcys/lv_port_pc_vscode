#include "desktop_theme.h"
static const desktop_theme_t s_theme = {
    .accent           = LV_COLOR_MAKE(0x00,0x7A,0xFF),
    .text_primary     = LV_COLOR_MAKE(0xFF,0xFF,0xFF),
    .panel_bg         = LV_COLOR_MAKE(0x14,0x19,0x28),
    .panel_opa        = 191,     /* 0.75*255 */
    .glass_border     = LV_COLOR_MAKE(0xFF,0xFF,0xFF),
    .glass_border_opa = 26,      /* 0.1*255 */
    .glass_hi         = LV_COLOR_MAKE(0xFF,0xFF,0xFF),
    .glass_hi_opa     = 64,      /* 0.25*255 */
    .overlay_dark     = LV_COLOR_MAKE(0x00,0x00,0x00),
    .overlay_opa      = 102,     /* 0.4*255 */
};
const desktop_theme_t * desktop_theme(void) { return &s_theme; }
