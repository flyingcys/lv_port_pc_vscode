#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "lvgl.h"

#include "../desktop/apple_music_app.h"
#include "../desktop/desktop_app_launcher.h"
#include "../desktop/desktop_metrics.h"

static uint32_t g_draw_buf[800 * 480];
static lv_obj_t *g_last_parent;
static bool g_player_deinit_called;

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
}

void apple_music_create_in(lv_obj_t *parent)
{
    g_last_parent = parent;
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, 32, 32);
    lv_obj_t *sidebar = lv_obj_create(parent);
    lv_obj_set_size(sidebar, 32, 32);
    lv_obj_t *player = lv_obj_create(parent);
    lv_obj_set_size(player, 32, 32);
}

void apple_music_destroy(void)
{
    g_player_deinit_called = true;
}

void am_metrics_init(int hor_res, int ver_res)
{
    assert(hor_res == 800);
    assert(ver_res == 480);
}

void am_player_deinit(void)
{
    g_player_deinit_called = true;
}

int main(void)
{
    lv_init();
    lv_display_t *disp = lv_display_create(800, 480);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, g_draw_buf, NULL, sizeof(g_draw_buf), LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_default(disp);
    desktop_set_resolution(800, 480);

    lv_obj_t *desktop_root = lv_obj_create(lv_screen_active());
    uint32_t before_count = lv_obj_get_child_count(lv_screen_active());

    apple_music_app_launch();

    assert(desktop_app_launcher_is_open());
    assert(g_last_parent != NULL);
    lv_obj_update_layout(lv_screen_active());
    assert(lv_obj_get_child_count(lv_screen_active()) == before_count + 1);
    assert(lv_obj_get_child(lv_screen_active(), 0) == desktop_root);

    lv_obj_t *overlay = lv_obj_get_parent(g_last_parent);
    assert(overlay != NULL);
    assert(lv_obj_get_parent(overlay) == lv_screen_active());
    assert(lv_obj_get_child_count(overlay) >= 2);
    assert(lv_obj_get_width(g_last_parent) == 800);
    assert(lv_obj_get_height(g_last_parent) == 480);
    assert(lv_obj_get_child_count(g_last_parent) >= 3);

    lv_obj_t *back_btn = lv_obj_get_child(overlay, (int32_t)lv_obj_get_child_count(overlay) - 1);
    lv_obj_send_event(back_btn, LV_EVENT_CLICKED, NULL);

    assert(!desktop_app_launcher_is_open());
    assert(g_player_deinit_called);
    assert(lv_obj_get_child_count(lv_screen_active()) == before_count);
    assert(lv_obj_get_child(lv_screen_active(), 0) == desktop_root);

    lv_display_delete(disp);
    return 0;
}
