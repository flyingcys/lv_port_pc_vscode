#ifndef DESKTOP_HOME_H
#define DESKTOP_HOME_H

#include "lvgl.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct desktop_home desktop_home_t;

typedef void (*desktop_page_changed_cb_t)(uint32_t page_index, void * user_data);

desktop_home_t * desktop_home_create(lv_obj_t * parent,
                                                          desktop_page_changed_cb_t page_changed_cb,
                                                          void * user_data);
void desktop_home_destroy(desktop_home_t * desktop);
uint32_t desktop_home_get_current_page(const desktop_home_t * desktop);
lv_obj_t * desktop_home_get_host(const desktop_home_t * desktop);
lv_obj_t * desktop_home_get_tileview(const desktop_home_t * desktop);
lv_obj_t * desktop_home_get_page(const desktop_home_t * desktop, uint32_t page_index);
bool desktop_home_screen_to_local(const desktop_home_t * desktop,
                                            const lv_point_t * screen_point,
                                            lv_point_t * local_point);

#ifdef __cplusplus
}
#endif

#endif
