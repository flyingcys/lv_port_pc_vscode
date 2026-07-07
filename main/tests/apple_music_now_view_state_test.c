#define _DEFAULT_SOURCE

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lvgl.h"

#include "../src/v9_apple_music/am_data.h"
#include "../src/v9_apple_music/am_local_scan.h"
#include "../src/v9_apple_music/am_metrics.h"
#include "../src/v9_apple_music/am_player.h"
#include "../src/v9_apple_music/am_sources_csv.h"
#include "../src/v9_apple_music/am_state.h"
#include "../src/v9_apple_music/am_theme.h"
#include "../src/v9_apple_music/apple_music.h"
#include "../src/v9_apple_music/am_icons.h"

#define CHECK(cond) check_true((cond), #cond, __FILE__, __LINE__)

static uint32_t g_draw_buf[800 * 480];
static am_source_kind_t g_source_kind = AM_SOURCE_LOCAL;
static size_t g_current_local_index = 2U;
static size_t g_current_radio_index = 0U;
static bool g_is_playing = false;

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

static void prepare_state_items(am_local_item_t *items)
{
    fill_local_item(&items[0], "/stub/alpha.mp3", "Alpha");
    fill_local_item(&items[1], "/stub/beta.mp3", "Beta");
    fill_local_item(&items[2], "/stub/gamma.mp3", "Gamma");
}

static void load_state_items(am_local_item_t *items, uint64_t *max_recent_seq)
{
    prepare_state_items(items);
    CHECK(am_state_load(AM_STATE_PATH, items, 3U, max_recent_seq) == 0);
}

static uint32_t count_labels_with_text(lv_obj_t *root, const char *text)
{
    uint32_t count = 0U;
    uint32_t child_count;
    uint32_t i;

    if(root == NULL || lv_obj_has_flag(root, LV_OBJ_FLAG_HIDDEN)) return 0U;

    if(lv_obj_check_type(root, &lv_label_class)) {
        const char *value = lv_label_get_text(root);
        if(value != NULL && strcmp(value, text) == 0) count++;
    }

    child_count = lv_obj_get_child_count(root);
    for(i = 0U; i < child_count; i++) {
        count += count_labels_with_text(lv_obj_get_child(root, (int32_t)i), text);
    }
    return count;
}

static uint32_t count_empty_labels(lv_obj_t *root)
{
    uint32_t count = 0U;
    uint32_t child_count;
    uint32_t i;

    if(root == NULL || lv_obj_has_flag(root, LV_OBJ_FLAG_HIDDEN)) return 0U;

    if(lv_obj_check_type(root, &lv_label_class)) {
        const char *value = lv_label_get_text(root);
        if(value != NULL && value[0] == '\0') count++;
    }

    child_count = lv_obj_get_child_count(root);
    for(i = 0U; i < child_count; i++) {
        count += count_empty_labels(lv_obj_get_child(root, (int32_t)i));
    }
    return count;
}

static void collect_label_order(lv_obj_t *root, const char *text, bool skip_hidden,
                                uint32_t *cursor, int32_t *found_order)
{
    uint32_t child_count;
    uint32_t i;

    if(root == NULL || found_order == NULL || *found_order >= 0) return;
    if(skip_hidden && lv_obj_has_flag(root, LV_OBJ_FLAG_HIDDEN)) return;

    if(lv_obj_check_type(root, &lv_label_class)) {
        const char *value = lv_label_get_text(root);
        if(value != NULL) {
            if(strcmp(value, text) == 0 && *found_order < 0) *found_order = (int32_t)(*cursor);
            (*cursor)++;
        }
    }

    child_count = lv_obj_get_child_count(root);
    for(i = 0U; i < child_count; i++) {
        collect_label_order(lv_obj_get_child(root, (int32_t)i), text, skip_hidden, cursor, found_order);
        if(*found_order >= 0) return;
    }
}

static int32_t find_label_order(lv_obj_t *root, const char *text)
{
    uint32_t cursor = 0U;
    int32_t found_order = -1;

    collect_label_order(root, text, true, &cursor, &found_order);
    return found_order;
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

static lv_obj_t *get_sidebar(lv_obj_t *parent)
{
    return lv_obj_get_child(parent, 0);
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
void am_player_play_local_index(size_t index) { g_current_local_index = index; }
void am_player_play_radio_index(size_t index) { g_current_radio_index = index; }
void am_player_toggle_playback(void)
{
    g_is_playing = !g_is_playing;
}
void am_player_prev(void)
{
    if(g_current_local_index == 0U) g_current_local_index = 2U;
    else g_current_local_index--;
}
void am_player_next(void)
{
    g_current_local_index = (g_current_local_index + 1U) % 3U;
}
void am_player_cycle_mode(void) {}
void am_player_set_volume_percent(uint8_t percent) { LV_UNUSED(percent); }
void am_player_seek_percent(uint8_t percent) { LV_UNUSED(percent); }
void am_player_toggle_mute(void) {}
void am_player_set_playlist_open(bool open) { LV_UNUSED(open); }
bool am_player_playlist_open(void) { return false; }
void am_player_set_playlist_pick_cb(am_player_pick_cb_t cb) { LV_UNUSED(cb); }
am_source_kind_t am_player_source_kind(void) { return g_source_kind; }
uint32_t am_player_track_duration_ms(size_t index)
{
    LV_UNUSED(index);
    return 180000U;
}
size_t am_player_current_local_index(void) { return g_current_local_index; }
size_t am_player_current_radio_index(void) { return g_current_radio_index; }
bool am_player_is_playing(void) { return g_is_playing; }
uint8_t am_player_volume_percent(void) { return 65U; }
void am_player_refresh_ui(void) {}
void am_player_on_play_pause(lv_event_t *e) { LV_UNUSED(e); }
void am_player_on_prev(lv_event_t *e) { LV_UNUSED(e); }
void am_player_on_next(lv_event_t *e) { LV_UNUSED(e); }
void am_player_on_mode(lv_event_t *e) { LV_UNUSED(e); }

static void test_recent_sidebar_and_local_favorite_button(void)
{
    lv_obj_t *parent;
    lv_obj_t *sidebar;
    lv_obj_t *favorite_btn;
    int32_t beta_order;
    int32_t alpha_order;

    g_source_kind = AM_SOURCE_LOCAL;
    g_current_local_index = 2U;
    write_state_file();

    parent = create_parent();
    apple_music_create_in(parent);
    lv_obj_update_layout(parent);
    sidebar = get_sidebar(parent);
    favorite_btn = find_now_view_favorite_button(parent);

    CHECK(sidebar != NULL);
    CHECK(count_labels_with_text(sidebar, "Beta") >= 1U);
    CHECK(count_labels_with_text(sidebar, "Alpha") >= 1U);
    beta_order = find_label_order(sidebar, "Beta");
    alpha_order = find_label_order(sidebar, "Alpha");
    CHECK(beta_order >= 0);
    CHECK(alpha_order >= 0);
    CHECK(beta_order < alpha_order);
    CHECK(favorite_btn != NULL);
    CHECK(!lv_obj_has_flag(favorite_btn, LV_OBJ_FLAG_HIDDEN));

    apple_music_destroy();
    unlink(AM_STATE_PATH);
}

static void test_sidebar_without_recent_has_no_placeholder_labels(void)
{
    lv_obj_t *parent;
    lv_obj_t *sidebar;

    g_source_kind = AM_SOURCE_LOCAL;
    g_current_local_index = 0U;
    g_is_playing = false;
    unlink(AM_STATE_PATH);

    parent = create_parent();
    apple_music_create_in(parent);
    lv_obj_update_layout(parent);
    sidebar = get_sidebar(parent);

    CHECK(sidebar != NULL);
    CHECK(count_labels_with_text(sidebar, "Alpha") == 0U);
    CHECK(count_labels_with_text(sidebar, "Beta") == 0U);
    CHECK(count_labels_with_text(sidebar, "Gamma") == 0U);
    CHECK(count_empty_labels(sidebar) == 0U);

    apple_music_destroy();
    unlink(AM_STATE_PATH);
}

static void test_local_favorite_button_click_persists_state(void)
{
    lv_obj_t *parent;
    lv_obj_t *favorite_btn;
    am_local_item_t loaded[3];
    uint64_t max_recent_seq = 0U;

    g_source_kind = AM_SOURCE_LOCAL;
    g_current_local_index = 2U;
    write_state_file();

    load_state_items(loaded, &max_recent_seq);
    CHECK(loaded[2].favorite == true);
    CHECK(max_recent_seq == 9U);

    parent = create_parent();
    apple_music_create_in(parent);
    lv_obj_update_layout(parent);

    favorite_btn = find_now_view_favorite_button(parent);
    CHECK(!lv_obj_has_flag(favorite_btn, LV_OBJ_FLAG_HIDDEN));
    lv_obj_send_event(favorite_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_update_layout(parent);

    load_state_items(loaded, &max_recent_seq);
    CHECK(loaded[2].favorite == false);
    CHECK(max_recent_seq == 9U);

    apple_music_destroy();
    unlink(AM_STATE_PATH);
}

static void test_next_track_refreshes_recent_sidebar_immediately(void)
{
    lv_obj_t *parent;
    lv_obj_t *sidebar;
    lv_obj_t *next_btn;
    int32_t gamma_order;
    int32_t beta_order;
    int32_t alpha_order;
    am_local_item_t loaded[3];
    uint64_t max_recent_seq = 0U;

    g_source_kind = AM_SOURCE_LOCAL;
    g_current_local_index = 1U;
    write_state_file();

    parent = create_parent();
    apple_music_create_in(parent);
    lv_obj_update_layout(parent);
    sidebar = get_sidebar(parent);
    next_btn = find_miniplayer_button(parent, AM_ICON_NEXT);

    CHECK(sidebar != NULL);
    CHECK(find_label_order(sidebar, "Gamma") < 0);

    lv_obj_send_event(next_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_update_layout(parent);

    gamma_order = find_label_order(sidebar, "Gamma");
    beta_order = find_label_order(sidebar, "Beta");
    alpha_order = find_label_order(sidebar, "Alpha");
    CHECK(gamma_order >= 0);
    CHECK(beta_order >= 0);
    CHECK(alpha_order >= 0);
    CHECK(gamma_order < beta_order);
    CHECK(beta_order < alpha_order);

    load_state_items(loaded, &max_recent_seq);
    CHECK(loaded[2].recent_seq == 10U);
    CHECK(max_recent_seq == 10U);

    apple_music_destroy();
    unlink(AM_STATE_PATH);
}

static void test_pause_does_not_refresh_recent_sidebar(void)
{
    lv_obj_t *parent;
    lv_obj_t *sidebar;
    lv_obj_t *play_btn;
    am_local_item_t loaded[3];
    uint64_t max_recent_seq = 0U;

    g_source_kind = AM_SOURCE_LOCAL;
    g_current_local_index = 2U;
    g_is_playing = true;
    write_state_file();

    parent = create_parent();
    apple_music_create_in(parent);
    lv_obj_update_layout(parent);
    sidebar = get_sidebar(parent);
    play_btn = find_miniplayer_button(parent, AM_ICON_PLAY);

    CHECK(sidebar != NULL);
    CHECK(play_btn != NULL);
    lv_obj_send_event(play_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_update_layout(parent);

    CHECK(count_labels_with_text(sidebar, "Gamma") == 0U);
    CHECK(find_label_order(sidebar, "Beta") >= 0);
    CHECK(find_label_order(sidebar, "Alpha") >= 0);
    CHECK(find_label_order(sidebar, "Beta") < find_label_order(sidebar, "Alpha"));

    load_state_items(loaded, &max_recent_seq);
    CHECK(loaded[2].recent_seq == 0U);
    CHECK(max_recent_seq == 9U);
    CHECK(g_is_playing == false);

    apple_music_destroy();
    unlink(AM_STATE_PATH);
}

static void test_radio_hides_now_view_favorite_button(void)
{
    lv_obj_t *parent;
    lv_obj_t *favorite_btn;

    g_source_kind = AM_SOURCE_RADIO;
    g_current_local_index = 2U;
    g_current_radio_index = 0U;
    write_state_file();

    parent = create_parent();
    apple_music_create_in(parent);
    lv_obj_update_layout(parent);
    favorite_btn = find_now_view_favorite_button(parent);

    CHECK(favorite_btn != NULL);
    CHECK(lv_obj_has_flag(favorite_btn, LV_OBJ_FLAG_HIDDEN));

    apple_music_destroy();
    unlink(AM_STATE_PATH);
}

int main(void)
{
    lv_display_t *disp;

    lv_init();
    disp = lv_display_create(800, 480);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, g_draw_buf, NULL, sizeof(g_draw_buf), LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_default(disp);
    am_metrics_init(800, 480);

    test_recent_sidebar_and_local_favorite_button();
    test_sidebar_without_recent_has_no_placeholder_labels();
    test_local_favorite_button_click_persists_state();
    test_next_track_refreshes_recent_sidebar_immediately();
    test_pause_does_not_refresh_recent_sidebar();
    test_radio_hides_now_view_favorite_button();

    lv_display_delete(disp);
    return 0;
}
