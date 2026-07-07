#define _DEFAULT_SOURCE

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lvgl.h"

#include "../src/v9_apple_music/am_data.h"
#include "../src/v9_apple_music/am_desktop_file_dialog.h"
#include "../src/v9_apple_music/am_icons.h"
#include "../src/v9_apple_music/am_metrics.h"
#include "../src/v9_apple_music/am_player.h"
#include "../src/v9_apple_music/am_sources_csv.h"
#include "../src/v9_apple_music/am_state.h"
#include "../src/v9_apple_music/apple_music.h"
#include "../../main/inc/music_player.h"

#define CHECK(cond) check_true((cond), #cond, __FILE__, __LINE__)

static uint32_t g_draw_buf[800 * 480];
static size_t g_current_local_index = 1U;
static size_t g_current_radio_index = 0U;
static bool g_is_playing = false;
static bool g_is_single_file_mode = false;
static music_play_mode_t g_play_mode = MP_MODE_SEQ;
static char g_single_file_path[1024];
static char g_single_file_title[256];
static am_file_pick_result_t g_pick_result = AM_FILE_PICK_CANCEL;
static char g_pick_path[1024];
static char g_prev_cwd[PATH_MAX];
static char g_test_cwd[PATH_MAX];

static void check_true(bool cond, const char *expr, const char *file, int line)
{
    if(cond) return;

    fprintf(stderr, "CHECK failed: %s (%s:%d)\n", expr, file, line);
    abort();
}

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
}

static void fill_local_item(am_local_item_t *item, const char *path, const char *title)
{
    memset(item, 0, sizeof(*item));
    snprintf(item->path, sizeof(item->path), "%s", path);
    snprintf(item->title, sizeof(item->title), "%s", title);
}

static void setup_test_workspace(void)
{
    char template_path[] = "/tmp/apple_music_now_view_single_file_test_XXXXXX";
    char *dir;

    CHECK(getcwd(g_prev_cwd, sizeof(g_prev_cwd)) != NULL);
    dir = mkdtemp(template_path);
    CHECK(dir != NULL);
    snprintf(g_test_cwd, sizeof(g_test_cwd), "%s", dir);
    CHECK(chdir(g_test_cwd) == 0);
}

static void teardown_test_workspace(void)
{
    if(g_test_cwd[0] != '\0') {
        CHECK(chdir(g_test_cwd) == 0);
        unlink(AM_STATE_PATH);
    }
    if(g_prev_cwd[0] != '\0') CHECK(chdir(g_prev_cwd) == 0);
    if(g_test_cwd[0] != '\0') {
        CHECK(rmdir(g_test_cwd) == 0);
        g_test_cwd[0] = '\0';
    }
}

static void write_state_file(void)
{
    am_local_item_t items[3];

    fill_local_item(&items[0], "/stub/alpha.mp3", "Alpha");
    fill_local_item(&items[1], "/stub/beta.mp3", "Beta");
    fill_local_item(&items[2], "/stub/gamma.mp3", "Gamma");
    items[0].recent_seq = 4U;
    items[1].recent_seq = 9U;
    items[2].favorite = true;

    unlink(AM_STATE_PATH);
    CHECK(am_state_save(AM_STATE_PATH, items, 3U) == 0);
}

static void load_state_items(am_local_item_t *items, uint64_t *max_recent_seq)
{
    fill_local_item(&items[0], "/stub/alpha.mp3", "Alpha");
    fill_local_item(&items[1], "/stub/beta.mp3", "Beta");
    fill_local_item(&items[2], "/stub/gamma.mp3", "Gamma");
    CHECK(am_state_load(AM_STATE_PATH, items, 3U, max_recent_seq) == 0);
}

static lv_obj_t *find_label_object(lv_obj_t *root, const char *text, bool skip_hidden)
{
    uint32_t child_count;
    uint32_t i;

    if(root == NULL) return NULL;
    if(skip_hidden && lv_obj_has_flag(root, LV_OBJ_FLAG_HIDDEN)) return NULL;

    if(lv_obj_check_type(root, &lv_label_class)) {
        const char *value = lv_label_get_text(root);
        if(value != NULL && strcmp(value, text) == 0) return root;
    }

    child_count = lv_obj_get_child_count(root);
    for(i = 0U; i < child_count; i++) {
        lv_obj_t *found = find_label_object(lv_obj_get_child(root, (int32_t)i), text, skip_hidden);
        if(found != NULL) return found;
    }

    return NULL;
}

static lv_obj_t *get_main_panel(lv_obj_t *parent)
{
    return lv_obj_get_child(parent, 1);
}

static lv_obj_t *find_now_view_favorite_button(lv_obj_t *parent)
{
    lv_obj_t *main = get_main_panel(parent);
    lv_obj_t *icon = find_label_object(main, AM_ICON_HEART, false);

    CHECK(main != NULL);
    CHECK(icon != NULL);
    return lv_obj_get_parent(icon);
}

static lv_obj_t *find_miniplayer_button(lv_obj_t *parent, const char *icon_text)
{
    lv_obj_t *main = get_main_panel(parent);
    lv_obj_t *icon = find_label_object(main, icon_text, true);

    CHECK(main != NULL);
    CHECK(icon != NULL);
    return lv_obj_get_parent(icon);
}

static lv_obj_t *create_parent(void)
{
    lv_obj_t *parent = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(parent);
    lv_obj_set_pos(parent, 0, 0);
    lv_obj_set_size(parent, 800, 480);
    return parent;
}

int am_local_scan_dir(const char *dir, am_local_item_t **items, size_t *count)
{
    am_local_item_t *locals;

    CHECK(strcmp(dir, "third-party/hls_player_demo/test_file") == 0);
    CHECK(items != NULL);
    CHECK(count != NULL);

    locals = (am_local_item_t *)calloc(3U, sizeof(*locals));
    CHECK(locals != NULL);
    fill_local_item(&locals[0], "/stub/alpha.mp3", "Alpha");
    fill_local_item(&locals[1], "/stub/beta.mp3", "Beta");
    fill_local_item(&locals[2], "/stub/gamma.mp3", "Gamma");
    *items = locals;
    *count = 3U;
    return 0;
}

void am_local_scan_free(am_local_item_t *items)
{
    free(items);
}

int am_sources_csv_load(const char *path, am_radio_item_t **items, size_t *count)
{
    am_radio_item_t *radios;

    CHECK(strcmp(path, "third-party/hls_player_demo/qa/production_test/config/sources.tsv") == 0);
    CHECK(items != NULL);
    CHECK(count != NULL);

    radios = (am_radio_item_t *)calloc(1U, sizeof(*radios));
    CHECK(radios != NULL);
    snprintf(radios[0].title, sizeof(radios[0].title), "%s", "Radio One");
    snprintf(radios[0].url, sizeof(radios[0].url), "%s", "http://example.test/live.m3u8");
    radios[0].duration_ms = 1000U;
    radios[0].network_cache_ms = 1000U;
    *items = radios;
    *count = 1U;
    return 0;
}

void am_sources_csv_free(am_radio_item_t *items)
{
    free(items);
}

void am_player_init(void) {}
void am_player_deinit(void) {}
void am_player_bind_miniplayer(const am_miniplayer_handles_t *handles) { LV_UNUSED(handles); }
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
    g_current_local_index = index;
    g_is_single_file_mode = false;
}
void am_player_play_radio_index(size_t index)
{
    g_current_radio_index = index;
    g_is_single_file_mode = false;
}
void am_player_play_single_file(const char *path, const char *title)
{
    CHECK(path != NULL);
    CHECK(title != NULL);
    snprintf(g_single_file_path, sizeof(g_single_file_path), "%s", path);
    snprintf(g_single_file_title, sizeof(g_single_file_title), "%s", title);
    g_is_single_file_mode = true;
    g_is_playing = true;
}
void am_player_toggle_playback(void)
{
    g_is_playing = !g_is_playing;
}
void am_player_prev(void)
{
    if(g_is_single_file_mode) return;
    g_current_local_index = (g_current_local_index == 0U) ? 2U : (g_current_local_index - 1U);
}
void am_player_next(void)
{
    if(g_is_single_file_mode) return;
    g_current_local_index = (g_current_local_index + 1U) % 3U;
}
void am_player_cycle_mode(void)
{
    switch(g_play_mode) {
        case MP_MODE_SEQ:
            g_play_mode = MP_MODE_REPEAT_ONE;
            break;
        case MP_MODE_REPEAT_ONE:
            g_play_mode = MP_MODE_REPEAT_ALL;
            break;
        case MP_MODE_REPEAT_ALL:
            g_play_mode = MP_MODE_SHUFFLE;
            break;
        case MP_MODE_SHUFFLE:
        default:
            g_play_mode = MP_MODE_SEQ;
            break;
    }
}
void am_player_set_volume_percent(uint8_t percent) { LV_UNUSED(percent); }
void am_player_seek_percent(uint8_t percent) { LV_UNUSED(percent); }
void am_player_toggle_mute(void) {}
void am_player_set_playlist_open(bool open) { LV_UNUSED(open); }
bool am_player_playlist_open(void) { return false; }
void am_player_set_playlist_pick_cb(am_player_pick_cb_t cb) { LV_UNUSED(cb); }
am_source_kind_t am_player_source_kind(void) { return AM_SOURCE_LOCAL; }
uint32_t am_player_track_duration_ms(size_t index)
{
    LV_UNUSED(index);
    return 180000U;
}
size_t am_player_current_local_index(void) { return g_current_local_index; }
size_t am_player_current_radio_index(void) { return g_current_radio_index; }
bool am_player_is_playing(void) { return g_is_playing; }
bool am_player_is_single_file_mode(void) { return g_is_single_file_mode; }
uint8_t am_player_volume_percent(void) { return 65U; }
void am_player_clear_single_file_mode(void)
{
    g_is_single_file_mode = false;
    g_single_file_path[0] = '\0';
    g_single_file_title[0] = '\0';
}
void am_player_refresh_ui(void) {}
void am_player_on_play_pause(lv_event_t *e) { LV_UNUSED(e); }
void am_player_on_prev(lv_event_t *e)
{
    LV_UNUSED(e);
    am_player_prev();
}
void am_player_on_next(lv_event_t *e)
{
    LV_UNUSED(e);
    am_player_next();
}
void am_player_on_mode(lv_event_t *e)
{
    LV_UNUSED(e);
    am_player_cycle_mode();
}

am_file_pick_result_t am_desktop_file_dialog_pick_audio(char *path_buf, size_t path_buf_size)
{
    if(g_pick_result != AM_FILE_PICK_OK) return g_pick_result;

    CHECK(path_buf != NULL);
    CHECK(path_buf_size > strlen(g_pick_path));
    snprintf(path_buf, path_buf_size, "%s", g_pick_path);
    return AM_FILE_PICK_OK;
}

static void test_open_file_click_updates_now_view_without_polluting_state(void)
{
    lv_obj_t *parent;
    lv_obj_t *open_file_btn;
    lv_obj_t *favorite_btn;
    am_local_item_t loaded[3];
    uint64_t max_recent_seq = 0U;

    g_current_local_index = 1U;
    g_current_radio_index = 0U;
    g_is_playing = false;
    g_is_single_file_mode = false;
    g_play_mode = MP_MODE_SEQ;
    snprintf(g_pick_path, sizeof(g_pick_path), "%s", "/tmp/My Song.mp3");
    g_pick_result = AM_FILE_PICK_OK;
    g_single_file_path[0] = '\0';
    g_single_file_title[0] = '\0';

    write_state_file();
    load_state_items(loaded, &max_recent_seq);
    CHECK(loaded[2].favorite == true);
    CHECK(max_recent_seq == 9U);

    parent = create_parent();
    apple_music_create_in(parent);
    lv_obj_update_layout(parent);

    open_file_btn = find_miniplayer_button(parent, AM_ICON_OPEN_FILE);
    lv_obj_send_event(open_file_btn, LV_EVENT_CLICKED, NULL);
    lv_timer_handler();
    lv_obj_update_layout(parent);

    CHECK(g_is_single_file_mode);
    CHECK(strcmp(g_single_file_path, "/tmp/My Song.mp3") == 0);
    CHECK(strcmp(g_single_file_title, "My Song") == 0);
    CHECK(find_label_object(parent, "My Song", true) != NULL);
    CHECK(find_label_object(parent, "本地文件", true) != NULL);

    favorite_btn = find_now_view_favorite_button(parent);
    CHECK(lv_obj_has_flag(favorite_btn, LV_OBJ_FLAG_HIDDEN));

    load_state_items(loaded, &max_recent_seq);
    CHECK(loaded[0].recent_seq == 4U);
    CHECK(loaded[1].recent_seq == 9U);
    CHECK(loaded[2].recent_seq == 0U);
    CHECK(loaded[2].favorite == true);
    CHECK(max_recent_seq == 9U);

    apple_music_destroy();
    unlink(AM_STATE_PATH);
}

static void check_single_file_now_view_state(lv_obj_t *parent)
{
    lv_obj_t *favorite_btn;

    CHECK(g_is_single_file_mode);
    CHECK(strcmp(g_single_file_path, "/tmp/My Song.mp3") == 0);
    CHECK(strcmp(g_single_file_title, "My Song") == 0);
    CHECK(find_label_object(parent, "My Song", true) != NULL);
    CHECK(find_label_object(parent, "本地文件", true) != NULL);

    favorite_btn = find_now_view_favorite_button(parent);
    CHECK(lv_obj_has_flag(favorite_btn, LV_OBJ_FLAG_HIDDEN));
}

static void check_state_file_unchanged(void)
{
    am_local_item_t loaded[3];
    uint64_t max_recent_seq = 0U;

    load_state_items(loaded, &max_recent_seq);
    CHECK(loaded[0].recent_seq == 4U);
    CHECK(loaded[1].recent_seq == 9U);
    CHECK(loaded[2].recent_seq == 0U);
    CHECK(loaded[2].favorite == true);
    CHECK(max_recent_seq == 9U);
}

static void open_single_file_session(lv_obj_t *parent)
{
    lv_obj_t *open_file_btn;

    open_file_btn = find_miniplayer_button(parent, AM_ICON_OPEN_FILE);
    lv_obj_send_event(open_file_btn, LV_EVENT_CLICKED, NULL);
    lv_timer_handler();
    lv_obj_update_layout(parent);
    check_single_file_now_view_state(parent);
}

static void click_mode_until(lv_obj_t *mode_btn, music_play_mode_t target_mode)
{
    int guard = 0;

    while(g_play_mode != target_mode && guard < 8) {
        lv_obj_send_event(mode_btn, LV_EVENT_CLICKED, NULL);
        lv_timer_handler();
        guard++;
    }

    CHECK(g_play_mode == target_mode);
}

static void test_single_file_mode_survives_transport_across_modes(void)
{
    lv_obj_t *parent;
    lv_obj_t *mode_btn;
    lv_obj_t *prev_btn;
    lv_obj_t *next_btn;
    const music_play_mode_t modes[] = {MP_MODE_SEQ, MP_MODE_REPEAT_ALL, MP_MODE_SHUFFLE};
    size_t i;

    g_current_local_index = 1U;
    g_current_radio_index = 0U;
    g_is_playing = false;
    g_is_single_file_mode = false;
    g_play_mode = MP_MODE_SEQ;
    snprintf(g_pick_path, sizeof(g_pick_path), "%s", "/tmp/My Song.mp3");
    g_pick_result = AM_FILE_PICK_OK;
    g_single_file_path[0] = '\0';
    g_single_file_title[0] = '\0';

    write_state_file();
    parent = create_parent();
    apple_music_create_in(parent);
    lv_obj_update_layout(parent);
    open_single_file_session(parent);

    mode_btn = find_miniplayer_button(parent, "SEQ");
    prev_btn = find_miniplayer_button(parent, LV_SYMBOL_PREV);
    next_btn = find_miniplayer_button(parent, LV_SYMBOL_NEXT);

    for(i = 0U; i < sizeof(modes) / sizeof(modes[0]); i++) {
        click_mode_until(mode_btn, modes[i]);
        lv_obj_send_event(prev_btn, LV_EVENT_CLICKED, NULL);
        lv_timer_handler();
        lv_obj_send_event(next_btn, LV_EVENT_CLICKED, NULL);
        lv_timer_handler();
        lv_obj_update_layout(parent);

        CHECK(g_play_mode == modes[i]);
        CHECK(g_current_local_index == 1U);
        check_single_file_now_view_state(parent);
        check_state_file_unchanged();
    }

    apple_music_destroy();
    unlink(AM_STATE_PATH);
}

int main(void)
{
    lv_display_t *disp;

    g_prev_cwd[0] = '\0';
    g_test_cwd[0] = '\0';
    setup_test_workspace();

    lv_init();
    disp = lv_display_create(800, 480);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, g_draw_buf, NULL, sizeof(g_draw_buf), LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_default(disp);
    am_metrics_init(800, 480);

    test_open_file_click_updates_now_view_without_polluting_state();
    test_single_file_mode_survives_transport_across_modes();

    lv_display_delete(disp);
    teardown_test_workspace();
    return 0;
}
