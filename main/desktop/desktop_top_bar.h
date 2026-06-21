#ifndef DESKTOP_TOP_BAR_H
#define DESKTOP_TOP_BAR_H

#include "lvgl.h"
#include "desktop_page_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct desktop_top_bar desktop_top_bar_t;

typedef enum {
    WIFI_STATE_OFF = 0,
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_WEAK,
    WIFI_STATE_NORMAL,
    WIFI_STATE_STRONG,
} wifi_state_t;

desktop_top_bar_t * desktop_top_bar_create(lv_obj_t * parent);
void desktop_top_bar_destroy(desktop_top_bar_t * top_bar);
lv_obj_t * desktop_top_bar_get_root(const desktop_top_bar_t * top_bar);
void desktop_top_bar_apply(desktop_top_bar_t * top_bar, const topbar_page_config_t * config);
void desktop_top_bar_set_time(desktop_top_bar_t * top_bar, const char * time_text);
void desktop_top_bar_set_wifi_state(desktop_top_bar_t * top_bar, wifi_state_t state);
void desktop_top_bar_start_minute_timer(desktop_top_bar_t * top_bar);

#ifdef __cplusplus
}
#endif

#endif
