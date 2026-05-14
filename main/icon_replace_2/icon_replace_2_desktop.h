#ifndef ICON_REPLACE_2_DESKTOP_H
#define ICON_REPLACE_2_DESKTOP_H

#include "lvgl.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct icon_replace_2_desktop icon_replace_2_desktop_t;

typedef void (*icon_replace_2_page_changed_cb_t)(uint32_t page_index, void * user_data);

icon_replace_2_desktop_t * icon_replace_2_desktop_create(lv_obj_t * parent,
                                                          icon_replace_2_page_changed_cb_t page_changed_cb,
                                                          void * user_data);
void icon_replace_2_desktop_destroy(icon_replace_2_desktop_t * desktop);
uint32_t icon_replace_2_desktop_get_current_page(const icon_replace_2_desktop_t * desktop);
lv_obj_t * icon_replace_2_desktop_get_host(const icon_replace_2_desktop_t * desktop);
lv_obj_t * icon_replace_2_desktop_get_tileview(const icon_replace_2_desktop_t * desktop);
lv_obj_t * icon_replace_2_desktop_get_page(const icon_replace_2_desktop_t * desktop, uint32_t page_index);
bool icon_replace_2_desktop_screen_to_local(const icon_replace_2_desktop_t * desktop,
                                            const lv_point_t * screen_point,
                                            lv_point_t * local_point);

#ifdef __cplusplus
}
#endif

#endif
