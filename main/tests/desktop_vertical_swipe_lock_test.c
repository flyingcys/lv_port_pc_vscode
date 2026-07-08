#include <assert.h>
#include <stdint.h>

#include "lvgl.h"

#include "../desktop/desktop.h"
#include "../desktop/desktop_metrics.h"

static uint32_t g_draw_buf[800 * 480];

static void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
}

static lv_obj_t * find_tileview(lv_obj_t * root)
{
    if(root == NULL) {
        return NULL;
    }

    if(lv_obj_check_type(root, &lv_tileview_class)) {
        return root;
    }

    uint32_t child_count = lv_obj_get_child_count(root);
    for(uint32_t i = 0; i < child_count; i++) {
        lv_obj_t * found = find_tileview(lv_obj_get_child(root, i));
        if(found != NULL) {
            return found;
        }
    }

    return NULL;
}

int main(void)
{
    lv_init();

    lv_display_t * disp = lv_display_create(800, 480);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, g_draw_buf, NULL, sizeof(g_draw_buf), LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_default(disp);

    desktop_set_resolution(800, 480);
    desktop_run();
    lv_obj_update_layout(lv_screen_active());

    lv_obj_t * screen = lv_screen_active();
    lv_obj_t * tileview = find_tileview(screen);
    lv_obj_t * host = tileview ? lv_obj_get_parent(tileview) : NULL;

    assert(tileview != NULL);
    assert(host != NULL);
    assert(!lv_obj_has_flag(screen, LV_OBJ_FLAG_SCROLLABLE));
    assert(lv_obj_get_scroll_dir(tileview) == LV_DIR_HOR);
    assert(!lv_obj_has_flag(tileview, LV_OBJ_FLAG_SCROLL_CHAIN_VER));
    assert(!lv_obj_has_flag(host, LV_OBJ_FLAG_SCROLL_CHAIN_VER));

    lv_display_delete(disp);
    return 0;
}
