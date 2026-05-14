#include "icon_replace_2_desktop.h"

#include "icon_replace_2_layout.h"

struct icon_replace_2_desktop {
    lv_obj_t * host;
    lv_obj_t * tileview;
    lv_obj_t * pages[PAGE_COUNT];
    icon_replace_2_page_changed_cb_t page_changed_cb;
    void * user_data;
    uint32_t current_page;
};

static void desktop_delete_cb(lv_event_t * e);
static void desktop_tileview_value_changed_cb(lv_event_t * e);

icon_replace_2_desktop_t * icon_replace_2_desktop_create(lv_obj_t * parent,
                                                          icon_replace_2_page_changed_cb_t page_changed_cb,
                                                          void * user_data)
{
    uint32_t page_index;
    icon_replace_2_desktop_t * desktop = lv_malloc_zeroed(sizeof(*desktop));

    if(desktop == NULL) {
        return NULL;
    }

    desktop->page_changed_cb = page_changed_cb;
    desktop->user_data = user_data;
    desktop->current_page = 0;

    desktop->host = lv_obj_create(parent);
    if(desktop->host == NULL) {
        lv_free(desktop);
        return NULL;
    }

    lv_obj_add_event_cb(desktop->host, desktop_delete_cb, LV_EVENT_DELETE, desktop);
    lv_obj_set_pos(desktop->host, 0, TOP_BAR_H);
    lv_obj_set_size(desktop->host, DESKTOP_W, DESKTOP_H);
    lv_obj_clear_flag(desktop->host, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(desktop->host, 0, 0);
    lv_obj_set_style_border_width(desktop->host, 0, 0);
    lv_obj_set_style_pad_all(desktop->host, 0, 0);
    lv_obj_set_style_bg_opa(desktop->host, LV_OPA_TRANSP, 0);

    desktop->tileview = lv_tileview_create(desktop->host);
    if(desktop->tileview == NULL) {
        lv_obj_delete(desktop->host);
        return NULL;
    }

    lv_obj_set_size(desktop->tileview, DESKTOP_W, DESKTOP_H);
    lv_obj_set_style_bg_color(desktop->tileview, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_radius(desktop->tileview, 0, 0);
    lv_obj_set_style_border_width(desktop->tileview, 0, 0);
    lv_obj_add_event_cb(desktop->tileview, desktop_tileview_value_changed_cb, LV_EVENT_VALUE_CHANGED, desktop);

    for(page_index = 0; page_index < PAGE_COUNT; page_index++) {
        desktop->pages[page_index] = lv_tileview_add_tile(desktop->tileview, page_index, 0, LV_DIR_ALL);
        if(desktop->pages[page_index] == NULL) {
            lv_obj_delete(desktop->host);
            return NULL;
        }

        lv_obj_set_size(desktop->pages[page_index], DESKTOP_W, DESKTOP_H);
        lv_obj_clear_flag(desktop->pages[page_index], LV_OBJ_FLAG_SCROLLABLE);
    }

    lv_tileview_set_tile(desktop->tileview, desktop->pages[0], LV_ANIM_OFF);

    return desktop;
}

void icon_replace_2_desktop_destroy(icon_replace_2_desktop_t * desktop)
{
    if(desktop == NULL || desktop->host == NULL) {
        return;
    }

    lv_obj_delete(desktop->host);
}

uint32_t icon_replace_2_desktop_get_current_page(const icon_replace_2_desktop_t * desktop)
{
    if(desktop == NULL) {
        return 0;
    }

    return desktop->current_page;
}

lv_obj_t * icon_replace_2_desktop_get_host(const icon_replace_2_desktop_t * desktop)
{
    if(desktop == NULL) {
        return NULL;
    }

    return desktop->host;
}

lv_obj_t * icon_replace_2_desktop_get_tileview(const icon_replace_2_desktop_t * desktop)
{
    if(desktop == NULL) {
        return NULL;
    }

    return desktop->tileview;
}

lv_obj_t * icon_replace_2_desktop_get_page(const icon_replace_2_desktop_t * desktop, uint32_t page_index)
{
    if(desktop == NULL || page_index >= PAGE_COUNT) {
        return NULL;
    }

    return desktop->pages[page_index];
}

bool icon_replace_2_desktop_screen_to_local(const icon_replace_2_desktop_t * desktop,
                                            const lv_point_t * screen_point,
                                            lv_point_t * local_point)
{
    lv_area_t host_coords;

    if(desktop == NULL || desktop->host == NULL || screen_point == NULL || local_point == NULL) {
        return false;
    }

    lv_obj_get_coords(desktop->host, &host_coords);
    local_point->x = screen_point->x - host_coords.x1;
    local_point->y = screen_point->y - host_coords.y1;

    return true;
}

static void desktop_delete_cb(lv_event_t * e)
{
    icon_replace_2_desktop_t * desktop = lv_event_get_user_data(e);
    uint32_t page_index;

    if(desktop == NULL) {
        return;
    }

    desktop->host = NULL;
    desktop->tileview = NULL;
    for(page_index = 0; page_index < PAGE_COUNT; page_index++) {
        desktop->pages[page_index] = NULL;
    }

    lv_free(desktop);
}

static void desktop_tileview_value_changed_cb(lv_event_t * e)
{
    uint32_t page_index;
    icon_replace_2_desktop_t * desktop = lv_event_get_user_data(e);
    lv_obj_t * active_tile;

    if(desktop == NULL) {
        return;
    }

    active_tile = lv_tileview_get_tile_active(desktop->tileview);
    if(active_tile == NULL) {
        return;
    }

    for(page_index = 0; page_index < PAGE_COUNT; page_index++) {
        if(desktop->pages[page_index] == active_tile) {
            desktop->current_page = page_index;

            if(desktop->page_changed_cb != NULL) {
                desktop->page_changed_cb(page_index, desktop->user_data);
            }

            return;
        }
    }
}
