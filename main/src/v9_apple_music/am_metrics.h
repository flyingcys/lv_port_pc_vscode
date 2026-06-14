#ifndef AM_METRICS_H
#define AM_METRICS_H
#include "lvgl/lvgl.h"

typedef enum { AM_TIER_800, AM_TIER_640, AM_TIER_480, AM_TIER_COUNT } am_tier_t;

typedef struct {
    const char *id;
    int16_t sidebar_w, player_h, nav_h;
    int16_t content_pad, page_gap, panel_pad, card_radius;
    int16_t cover, item_action;
    int16_t queue_w, settings_nav_w;
    bool    stack_content;
    const lv_font_t *f_title, *f_h2, *f_metric, *f_strong, *f_body, *f_label, *f_icon;
} am_metrics_t;

const am_metrics_t *am_metrics(void);
void am_metrics_init(int hor_res, int ver_res);

#endif /* AM_METRICS_H */
