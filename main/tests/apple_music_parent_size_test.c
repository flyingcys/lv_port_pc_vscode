#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "lvgl.h"
#include "../src/v9_apple_music/am_metrics.h"
#include "../src/v9_apple_music/am_data.h"
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

void am_player_set_sources(const am_local_item_t *locals, size_t local_count,
                           const am_radio_item_t *radios, size_t radio_count)
{
    LV_UNUSED(locals);
    LV_UNUSED(local_count);
    LV_UNUSED(radios);
    LV_UNUSED(radio_count);
}

void am_player_play_local_index(size_t index)
{
    LV_UNUSED(index);
}

void am_player_play_radio_index(size_t index)
{
    LV_UNUSED(index);
}

void am_player_toggle_playback(void)
{
}

void am_player_prev(void)
{
}

void am_player_next(void)
{
}

void am_player_cycle_mode(void)
{
}

void am_player_set_volume_percent(uint8_t percent)
{
    LV_UNUSED(percent);
}

void am_player_set_playlist_open(bool open)
{
    LV_UNUSED(open);
}

bool am_player_playlist_open(void)
{
    return false;
}

am_source_kind_t am_player_source_kind(void)
{
    return AM_SOURCE_NONE;
}

uint32_t am_player_track_duration_ms(size_t index)
{
    LV_UNUSED(index);
    return 0U;
}

size_t am_player_current_local_index(void)
{
    return 0U;
}

size_t am_player_current_radio_index(void)
{
    return 0U;
}

bool am_player_is_playing(void)
{
    return false;
}

uint8_t am_player_volume_percent(void)
{
    return 65U;
}

void am_player_refresh_ui(void)
{
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

void am_player_on_mode(lv_event_t *e)
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
    assert(lv_obj_get_child_count(parent) >= 2);
    assert(lv_obj_get_child(parent, 0) != NULL);
    assert(lv_obj_get_child(parent, 1) != NULL);
    assert(lv_obj_get_child_count(lv_obj_get_child(parent, 1)) >= 3);

    lv_display_delete(disp);
    return 0;
}
