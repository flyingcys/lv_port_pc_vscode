#ifndef ICON_REPLACE_2_THEME_H
#define ICON_REPLACE_2_THEME_H
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    lv_color_t accent;        /* #007AFF */
    lv_color_t text_primary;  /* #FFFFFF */
    lv_color_t panel_bg;      /* rgba(20,25,40,.75) */
    lv_opa_t   panel_opa;     /* 191 */
    lv_color_t glass_border;  /* rgba(255,255,255,.1) */
    lv_opa_t   glass_border_opa;
    lv_color_t glass_hi;      /* 顶部高光 rgba(255,255,255,.25) */
    lv_opa_t   glass_hi_opa;
    lv_color_t overlay_dark;  /* 暗化遮罩黑 */
    lv_opa_t   overlay_opa;
} ir2_theme_t;
const ir2_theme_t * ir2_theme(void);
#ifdef __cplusplus
}
#endif
#endif
