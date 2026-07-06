#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lvgl.h"
#include "../src/v9_apple_music/am_metrics.h"
#include "../src/v9_apple_music/am_player.h"
#include "../src/v9_apple_music/am_shell.h"
#include "../../main/inc/music_player.h"

static uint32_t g_draw_buf[800 * 480];
static bool g_is_playing = true;
static size_t g_current_index = 0U;

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
}

static int expect_hidden(lv_obj_t *obj, const char *name)
{
    if(!lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) {
        fprintf(stderr, "%s should be hidden\n", name);
        return 1;
    }
    return 0;
}

static int expect_visible(lv_obj_t *obj, const char *name)
{
    if(lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) {
        fprintf(stderr, "%s should be visible\n", name);
        return 1;
    }
    return 0;
}

void music_player_init(const char **urls, size_t count)
{
    LV_UNUSED(urls);
    LV_UNUSED(count);
    g_is_playing = true;
}

void music_player_deinit(void)
{
    g_is_playing = false;
    g_current_index = 0U;
}

void music_player_play(void)
{
    g_is_playing = true;
}

void music_player_pause(void)
{
    g_is_playing = false;
}

void music_player_resume(void)
{
    g_is_playing = true;
}

void music_player_next(void) {}
void music_player_prev(void) {}

void music_player_select(size_t index)
{
    g_current_index = index;
}

size_t music_player_get_count(void)
{
    return 0U;
}

size_t music_player_get_current_index(void)
{
    return g_current_index;
}

const char *music_player_get_title(size_t index)
{
    LV_UNUSED(index);
    return "";
}

bool music_player_is_live(size_t index)
{
    LV_UNUSED(index);
    return false;
}

bool music_player_is_playing(void)
{
    return g_is_playing;
}

void music_player_seek(uint32_t position_ms)
{
    LV_UNUSED(position_ms);
}

uint32_t music_player_get_duration_ms(void)
{
    return 180000U;
}

uint32_t music_player_get_position_ms(void)
{
    return 30000U;
}

uint32_t music_player_get_track_duration_ms(size_t index)
{
    LV_UNUSED(index);
    return 180000U;
}

void music_player_set_play_mode(music_play_mode_t mode)
{
    LV_UNUSED(mode);
}

music_play_mode_t music_player_get_play_mode(void)
{
    return MP_MODE_SEQ;
}

music_play_mode_t music_player_cycle_play_mode(void)
{
    return MP_MODE_SEQ;
}

void music_player_set_volume(uint8_t percent)
{
    LV_UNUSED(percent);
}

uint8_t music_player_get_volume(void)
{
    return 65U;
}

void music_player_mute_toggle(void) {}

bool music_player_is_muted(void)
{
    return false;
}

void local_music_demo_launch(void) {}
void local_music_demo_close(void) {}

static void fill_local_item(am_local_item_t *item, size_t index)
{
    snprintf(item->path, sizeof(item->path), "track-%02u.mp3", (unsigned)index);
    snprintf(item->title, sizeof(item->title), "Track %02u", (unsigned)index);
    item->favorite = false;
}

typedef struct {
    lv_point_t point;
    lv_indev_state_t state;
} drag_state_t;

static void drag_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    drag_state_t *state = (drag_state_t *)lv_indev_get_user_data(indev);

    data->state = state->state;
    data->point = state->point;
    data->timestamp = lv_tick_get();
}

static void simulate_thumb_drag(lv_obj_t *thumb, int32_t dy)
{
    lv_indev_t *indev;
    drag_state_t state;
    lv_area_t coords;

    lv_obj_get_coords(thumb, &coords);
    state.point.x = (coords.x1 + coords.x2) / 2;
    state.point.y = (coords.y1 + coords.y2) / 2;
    state.state = LV_INDEV_STATE_PRESSED;

    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, drag_read_cb);
    lv_indev_set_user_data(indev, &state);

    lv_indev_read(indev);
    lv_obj_send_event(thumb, LV_EVENT_PRESSED, indev);

    state.point.y += dy;
    lv_indev_read(indev);
    lv_obj_send_event(thumb, LV_EVENT_PRESSING, indev);

    state.state = LV_INDEV_STATE_RELEASED;
    lv_indev_read(indev);
    lv_obj_send_event(thumb, LV_EVENT_RELEASED, indev);

    lv_indev_delete(indev);
}

static int32_t playlist_content_range(lv_obj_t *list)
{
    int32_t viewport_h = lv_obj_get_content_height(list);
    int32_t content_h = lv_obj_get_scroll_bottom(list) - lv_obj_get_scroll_top(list) + viewport_h;
    return content_h - viewport_h;
}

int main(void)
{
    enum { SHORT_COUNT = 2, LONG_COUNT = 28 };
    lv_display_t *disp;
    lv_obj_t *root;
    lv_obj_t *player;
    am_miniplayer_handles_t handles;
    am_local_item_t short_items[SHORT_COUNT];
    am_local_item_t long_items[LONG_COUNT];
    lv_area_t thumb_coords_before;
    lv_area_t thumb_coords_after;
    size_t i;

    for(i = 0; i < SHORT_COUNT; i++) fill_local_item(&short_items[i], i);
    for(i = 0; i < LONG_COUNT; i++) fill_local_item(&long_items[i], i);

    lv_init();
    disp = lv_display_create(800, 480);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, g_draw_buf, NULL, sizeof(g_draw_buf), LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_default(disp);

    am_metrics_init(800, 480);
    am_player_init();

    root = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(root);
    lv_obj_set_pos(root, 0, 0);
    lv_obj_set_size(root, 604, 480);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    player = lv_obj_create(root);
    lv_obj_remove_style_all(player);
    lv_obj_set_size(player, 604, am_metrics()->player_h);
    lv_obj_align(player, LV_ALIGN_BOTTOM_MID, 0, 0);

    handles = am_shell_build_miniplayer(player, NULL, NULL, NULL, NULL, NULL);
    am_player_bind_miniplayer(&handles);
    am_player_set_playlist_open(true);

    am_player_set_sources(short_items, SHORT_COUNT, NULL, 0U);
    am_player_play_local_index(0U);
    am_player_refresh_ui();
    lv_obj_update_layout(root);

    if(expect_hidden(handles.playlist_scroll_track, "track")) return 1;

    am_player_set_sources(long_items, LONG_COUNT, NULL, 0U);
    am_player_play_local_index(0U);
    am_player_refresh_ui();
    lv_obj_update_layout(root);

    if(expect_visible(handles.playlist_scroll_track, "track")) return 1;
    if(lv_obj_get_height(handles.playlist_scroll_thumb) < 10) {
        fprintf(stderr, "thumb height too small: %d\n",
                (int)lv_obj_get_height(handles.playlist_scroll_thumb));
        return 1;
    }

    lv_obj_get_coords(handles.playlist_scroll_thumb, &thumb_coords_before);
    {
        int32_t before = lv_obj_get_scroll_y(handles.playlist_list);
        simulate_thumb_drag(handles.playlist_scroll_thumb, 24);
        int32_t after = lv_obj_get_scroll_y(handles.playlist_list);
        if(after == before) {
            fprintf(stderr, "scroll y should change after dragging thumb\n");
            return 1;
        }
        if(after < before) {
            fprintf(stderr, "scroll y should increase after dragging thumb downward: before=%d after=%d\n",
                    (int)before, (int)after);
            return 1;
        }
    }
    {
        int32_t content_range;
        lv_area_t track_coords;
        lv_area_t thumb_coords;
        int32_t track_content_bottom;
        int32_t bottom_drag_dy;

        lv_obj_set_style_pad_top(handles.playlist_scroll_track, 10, 0);
        lv_obj_set_style_pad_bottom(handles.playlist_scroll_track, 10, 0);
        lv_obj_scroll_to_y(handles.playlist_list, 0, LV_ANIM_OFF);
        am_player_refresh_ui();
        lv_obj_update_layout(root);

        content_range = playlist_content_range(handles.playlist_list);
        lv_obj_get_coords(handles.playlist_scroll_track, &track_coords);
        lv_obj_get_coords(handles.playlist_scroll_thumb, &thumb_coords);
        bottom_drag_dy = track_coords.y2 - ((thumb_coords.y1 + thumb_coords.y2) / 2) + 1;
        simulate_thumb_drag(handles.playlist_scroll_thumb, bottom_drag_dy);
        if(lv_obj_get_scroll_y(handles.playlist_list) < content_range - 2) {
            fprintf(stderr, "scroll y should clamp near bottom after large thumb drag: got=%d expected>=%d\n",
                    (int)lv_obj_get_scroll_y(handles.playlist_list),
                    (int)(content_range - 2));
            return 1;
        }

        lv_obj_get_coords(handles.playlist_scroll_track, &track_coords);
        lv_obj_get_coords(handles.playlist_scroll_thumb, &thumb_coords);
        track_content_bottom = track_coords.y1
                             + lv_obj_get_style_pad_top(handles.playlist_scroll_track, 0)
                             + lv_obj_get_content_height(handles.playlist_scroll_track);
        if(thumb_coords.y2 > track_content_bottom) {
            fprintf(stderr, "thumb should stay within track content area after bottom drag: thumb_bottom=%d content_bottom=%d\n",
                    (int)thumb_coords.y2,
                    (int)track_content_bottom);
            return 1;
        }
    }
    lv_obj_scroll_to_y(handles.playlist_list, 120, LV_ANIM_OFF);
    lv_timer_handler();
    lv_obj_update_layout(root);
    lv_obj_get_coords(handles.playlist_scroll_thumb, &thumb_coords_after);
    if(thumb_coords_after.y1 <= thumb_coords_before.y1) {
        fprintf(stderr, "thumb should move after scroll: before=%d after=%d\n",
                (int)thumb_coords_before.y1,
                (int)thumb_coords_after.y1);
        return 1;
    }

    am_player_deinit();
    lv_display_delete(disp);
    return 0;
}
