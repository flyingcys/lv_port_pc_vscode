#ifndef ICON_REPLACE_2_TOP_BAR_H
#define ICON_REPLACE_2_TOP_BAR_H

#include "lvgl.h"
#include "icon_replace_2_page_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct icon_replace_2_top_bar icon_replace_2_top_bar_t;

typedef enum {
    WIFI_STATE_OFF = 0,
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_WEAK,
    WIFI_STATE_NORMAL,
    WIFI_STATE_STRONG,
} wifi_state_t;

icon_replace_2_top_bar_t * icon_replace_2_top_bar_create(lv_obj_t * parent);
void icon_replace_2_top_bar_destroy(icon_replace_2_top_bar_t * top_bar);
lv_obj_t * icon_replace_2_top_bar_get_root(const icon_replace_2_top_bar_t * top_bar);
void icon_replace_2_top_bar_apply(icon_replace_2_top_bar_t * top_bar, const topbar_page_config_t * config);
void icon_replace_2_top_bar_set_time(icon_replace_2_top_bar_t * top_bar, const char * time_text);
void icon_replace_2_top_bar_set_wifi_state(icon_replace_2_top_bar_t * top_bar, wifi_state_t state);
void icon_replace_2_top_bar_start_minute_timer(icon_replace_2_top_bar_t * top_bar);

#ifdef __cplusplus
}
#endif

#endif
