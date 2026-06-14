#ifndef AM_PAGE_SETTINGS_H
#define AM_PAGE_SETTINGS_H
#include "lvgl/lvgl.h"

typedef void (*am_theme_pick_cb_t)(int theme_index, void *user);
typedef void (*am_tab_pick_cb_t)(int tab_index, void *user);

lv_obj_t *am_page_settings_create(lv_obj_t *content_parent, int active_tab,
                                  am_theme_pick_cb_t on_theme, am_tab_pick_cb_t on_tab, void *user);

#endif /* AM_PAGE_SETTINGS_H */
