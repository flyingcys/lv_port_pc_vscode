#ifndef AM_PAGE_LIST_H
#define AM_PAGE_LIST_H

#include "lvgl/lvgl.h"

#include "am_data.h"

typedef void (*am_radio_select_cb_t)(size_t index, void *user);
typedef void (*am_local_select_cb_t)(size_t index, void *user);

lv_obj_t *am_page_list_build_radio(lv_obj_t *parent,
                                   const am_radio_item_t *items,
                                   size_t count,
                                   size_t current_index,
                                   am_radio_select_cb_t on_select,
                                   void *user);

lv_obj_t *am_page_list_build_favorites(lv_obj_t *parent,
                                       const am_local_item_t *items,
                                       size_t count,
                                       size_t current_index,
                                       am_local_select_cb_t on_select,
                                       void *user);

#endif /* AM_PAGE_LIST_H */
