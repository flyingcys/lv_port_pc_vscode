#include <assert.h>
#include <stdint.h>

#include "lvgl.h"
#include "../src/v9_apple_music/am_metrics.h"
#include "../src/v9_apple_music/am_player.h"
#include "../src/v9_apple_music/apple_music.h"

static uint32_t g_draw_buf[800 * 480];

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
}

void am_player_init(void)
{
}

void am_player_deinit(void)
{
}

void am_player_bind_miniplayer(const am_miniplayer_handles_t *handles)
{
    LV_UNUSED(handles);
}

void am_player_load_local(const char **urls, size_t count, size_t start_index)
{
    LV_UNUSED(urls);
    LV_UNUSED(count);
    LV_UNUSED(start_index);
}

void am_player_play_stream(const char *url, const char *title)
{
    LV_UNUSED(url);
    LV_UNUSED(title);
}

void am_player_on_play_pause(lv_event_t *e)
{
    LV_UNUSED(e);
}

void am_player_on_prev(lv_event_t *e)
{
    LV_UNUSED(e);
}

void am_player_on_next(lv_event_t *e)
{
    LV_UNUSED(e);
}

int main(void)
{
    lv_init();
    lv_display_t *disp = lv_display_create(800, 480);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, g_draw_buf, NULL, sizeof(g_draw_buf), LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_default(disp);

    lv_obj_t *parent = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(parent);
    lv_obj_set_pos(parent, 0, 0);
    lv_obj_set_size(parent, 800, 480);

    am_metrics_init(800, 480);
    apple_music_create_in(parent);
    lv_obj_update_layout(parent);

    assert(lv_obj_get_x(parent) == 0);
    assert(lv_obj_get_y(parent) == 0);
    assert(lv_obj_get_width(parent) == 800);
    assert(lv_obj_get_height(parent) == 480);
    assert(lv_obj_get_child_count(parent) >= 3);

    lv_display_delete(disp);
    return 0;
}
