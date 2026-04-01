#include <cassert>

#include "lvgl/lvgl.h"
#include "smart_home_page.h"

static void dummy_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
}

int main()
{
    lv_area_t coords;

    lv_init();

    static lv_color32_t framebuffer[480 * 480];
    lv_display_t * display = lv_display_create(480, 480);
    lv_display_set_buffers(
        display,
        framebuffer,
        NULL,
        sizeof(framebuffer),
        LV_DISPLAY_RENDER_MODE_DIRECT
    );
    lv_display_set_flush_cb(display, dummy_flush_cb);

    lv_obj_t * page = smart_home_page_create();
    lv_timer_handler();
    assert(page != nullptr);
    assert(lv_obj_get_width(page) == 480);
    assert(lv_obj_get_height(page) == 480);
    const uint32_t child_count = lv_obj_get_child_count(page);
    assert(child_count == 9);

    lv_obj_t * status_bar = lv_obj_get_child(page, 0);
    assert(status_bar != nullptr);
    lv_obj_get_coords(status_bar, &coords);
    assert(coords.x1 == 16);
    assert(coords.y1 == 15);
    assert(lv_obj_get_width(status_bar) == 448);
    assert(lv_obj_get_height(status_bar) == 32);

    lv_obj_t * scenes_label = lv_obj_get_child(page, 1);
    assert(scenes_label != nullptr);
    lv_obj_get_coords(scenes_label, &coords);
    assert(coords.x1 == 16);
    assert(coords.y1 == 70);

    lv_obj_t * top_left_card = lv_obj_get_child(page, 3);
    assert(top_left_card != nullptr);
    lv_obj_get_coords(top_left_card, &coords);
    assert(coords.x1 == 16);
    assert(coords.y1 == 130);
    assert(lv_obj_get_width(top_left_card) == 219);
    assert(lv_obj_get_height(top_left_card) == 145);

    smart_home_page_destroy();
    lv_deinit();
    return 0;
}
