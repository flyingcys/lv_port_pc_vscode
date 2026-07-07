#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"

#include "../src/v9_apple_music/am_metrics.h"
#include "../src/v9_apple_music/am_player.h"
#include "../src/v9_apple_music/am_shell.h"
#include "../../main/inc/music_player.h"

static uint32_t g_draw_buf[800 * 480];
static bool g_is_playing = false;
static uint32_t g_position_ms = 0U;
static uint32_t g_duration_ms = 180000U;
static uint8_t g_volume = 65U;

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
}

static void check_true(bool cond, const char *expr, int line)
{
    if(cond) return;
    fprintf(stderr, "CHECK failed: %s (line %d)\n", expr, line);
    abort();
}

#define CHECK(cond) check_true((cond), #cond, __LINE__)

void music_player_init(const char **urls, size_t count)
{
    LV_UNUSED(urls);
    CHECK(count == 1U);
    g_is_playing = true;
    g_position_ms = 0U;
}

void music_player_deinit(void)
{
    g_is_playing = false;
    g_position_ms = 0U;
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
    CHECK(index == 0U);
}

size_t music_player_get_count(void)
{
    return 1U;
}

size_t music_player_get_current_index(void)
{
    return 0U;
}

const char *music_player_get_title(size_t index)
{
    LV_UNUSED(index);
    return "picked";
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
    g_position_ms = position_ms;
}

uint32_t music_player_get_duration_ms(void)
{
    return g_duration_ms;
}

uint32_t music_player_get_position_ms(void)
{
    return g_position_ms;
}

uint32_t music_player_get_track_duration_ms(size_t index)
{
    LV_UNUSED(index);
    return g_duration_ms;
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
    g_volume = percent;
}

uint8_t music_player_get_volume(void)
{
    return g_volume;
}

void music_player_mute_toggle(void) {}

bool music_player_is_muted(void)
{
    return false;
}

void local_music_demo_launch(void) {}
void local_music_demo_close(void) {}

static void setup_display(lv_display_t **disp_out, lv_obj_t **root_out, am_miniplayer_handles_t *handles_out)
{
    lv_display_t *disp;
    lv_obj_t *root;
    lv_obj_t *player;
    lv_obj_t *info;

    lv_init();
    disp = lv_display_create(800, 480);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, g_draw_buf, NULL, sizeof(g_draw_buf), LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_default(disp);
    am_metrics_init(800, 480);

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

    *handles_out = am_shell_build_miniplayer(player, NULL, NULL, NULL, NULL, NULL);
    info = lv_obj_create(root);
    lv_obj_remove_style_all(info);
    lv_obj_set_size(info, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(info, LV_FLEX_FLOW_COLUMN);
    lv_obj_align(info, LV_ALIGN_TOP_LEFT, 0, 0);
    handles_out->title_label = lv_label_create(info);
    lv_label_set_text(handles_out->title_label, "");
    handles_out->subtitle_label = lv_label_create(info);
    lv_label_set_text(handles_out->subtitle_label, "");
    lv_obj_update_layout(root);
    *disp_out = disp;
    *root_out = root;
}

static void teardown_display(lv_display_t *disp)
{
    lv_display_delete(disp);
}

static void test_single_file_playlist_renders_one_row(void)
{
    lv_display_t *disp;
    lv_obj_t *root;
    am_miniplayer_handles_t handles;

    setup_display(&disp, &root, &handles);
    am_player_init();
    am_player_bind_miniplayer(&handles);
    am_player_play_single_file("/tmp/picked.mp3", "picked");
    am_player_set_playlist_open(true);
    am_player_refresh_ui();

    CHECK(root != NULL);
    CHECK(lv_obj_get_child_count(handles.playlist_list) == 1U);
    CHECK(strcmp(lv_label_get_text(handles.title_label), "picked") == 0);
    CHECK(strcmp(lv_label_get_text(handles.subtitle_label), "单个文件") == 0);
    CHECK(!lv_obj_has_flag(handles.playlist_popup, LV_OBJ_FLAG_HIDDEN));
    CHECK(am_player_is_single_file_mode());

    am_player_deinit();
    teardown_display(disp);
}

static void test_single_file_next_prev_do_not_leave_current_file(void)
{
    am_player_init();
    am_player_play_single_file("/tmp/picked.mp3", "picked");
    am_player_next();
    am_player_prev();

    CHECK(am_player_is_single_file_mode());
    CHECK(am_player_source_kind() == AM_SOURCE_LOCAL);
    CHECK(am_player_current_local_index() == 0U);

    am_player_deinit();
}

static void test_clear_single_file_mode_resets_flag(void)
{
    am_player_init();
    am_player_play_single_file("/tmp/picked.mp3", "picked");
    CHECK(am_player_is_single_file_mode());
    am_player_clear_single_file_mode();
    CHECK(!am_player_is_single_file_mode());
    am_player_deinit();
}

int main(void)
{
    test_single_file_playlist_renders_one_row();
    test_single_file_next_prev_do_not_leave_current_file();
    test_clear_single_file_mode_resets_flag();
    return 0;
}
