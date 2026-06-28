#include "apple_music.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "am_data.h"
#include "am_local_scan.h"
#include "am_metrics.h"
#include "am_page_list.h"
#include "am_player.h"
#include "am_shell.h"
#include "am_sources_csv.h"
#include "am_theme.h"
#include "am_widgets.h"

typedef struct {
    lv_obj_t *root;
    lv_obj_t *sidebar;
    lv_obj_t *main;
    lv_obj_t *header;
    lv_obj_t *crumb;
    lv_obj_t *content;
    lv_obj_t *now_view;
    lv_obj_t *cover;
    lv_obj_t *title;
    lv_obj_t *subtitle;
    lv_obj_t *lyrics;
    lv_obj_t *player;
    am_miniplayer_handles_t mini;
    am_view_t current_view;
    am_local_item_t *locals;
    size_t local_count;
    am_radio_item_t *radios;
    size_t radio_count;
} am_app_t;

static am_app_t g_app;
static void am_update_now_view(void);

static void am_free_runtime_data(void)
{
    if(g_app.locals != NULL) {
        am_local_scan_free(g_app.locals);
        g_app.locals = NULL;
    }
    g_app.local_count = 0U;

    if(g_app.radios != NULL) {
        am_sources_csv_free(g_app.radios);
        g_app.radios = NULL;
    }
    g_app.radio_count = 0U;
}

static void am_on_radio_selected(size_t index, void *user)
{
    LV_UNUSED(user);
    am_player_play_radio_index(index);
    if(g_app.current_view == AM_VIEW_NOW) am_update_now_view();
}

static void am_on_local_selected(size_t index, void *user)
{
    LV_UNUSED(user);
    am_player_play_local_index(index);
    if(g_app.current_view == AM_VIEW_NOW) am_update_now_view();
}

static void am_on_playlist_toggle(lv_event_t *e)
{
    LV_UNUSED(e);
    am_player_set_playlist_open(!am_player_playlist_open());
}

static void am_update_now_view(void)
{
    size_t i;
    const char *title = "未播放";
    const char *subtitle = "选择本地音乐或广播电台开始";
    bool radio = (am_player_source_kind() == AM_SOURCE_RADIO);

    if(radio && g_app.radios != NULL && am_player_current_radio_index() < g_app.radio_count) {
        title = g_app.radios[am_player_current_radio_index()].title;
        subtitle = "LIVE 广播电台";
    } else if(am_player_source_kind() == AM_SOURCE_LOCAL &&
              g_app.locals != NULL && am_player_current_local_index() < g_app.local_count) {
        title = g_app.locals[am_player_current_local_index()].title;
        subtitle = "本地资料库";
    }

    if(g_app.title != NULL) lv_label_set_text(g_app.title, title);
    if(g_app.subtitle != NULL) lv_label_set_text(g_app.subtitle, subtitle);
    if(g_app.cover != NULL) {
        lv_obj_set_style_bg_color(g_app.cover, radio ? lv_color_hex(0xfa2d48) : lv_color_hex(0x9a2407), 0);
        lv_obj_set_style_bg_grad_color(g_app.cover, radio ? lv_color_hex(0xfb7a8f) : lv_color_hex(0xffb02f), 0);
        lv_obj_set_style_bg_grad_dir(g_app.cover, LV_GRAD_DIR_VER, 0);
    }
    if(g_app.lyrics != NULL) {
        lv_obj_clean(g_app.lyrics);
        for(i = 0U; i < 4U; i++) {
            lv_obj_t *line = am_text(g_app.lyrics, am_mock_lyrics[i], am_metrics()->f_body,
                                     (i == 1U) ? lv_color_hex(0xfa2d48) : AM_MUTED);
            lv_label_set_long_mode(line, LV_LABEL_LONG_DOT);
            lv_obj_set_width(line, LV_PCT(100));
        }
    }
}

static void am_build_now_view(lv_obj_t *parent)
{
    const am_metrics_t *m = am_metrics();
    lv_obj_t *view = lv_obj_create(parent);
    lv_obj_t *info;

    lv_obj_remove_style_all(view);
    lv_obj_set_size(view, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_left(view, 26, 0);
    lv_obj_set_style_pad_right(view, 26, 0);
    lv_obj_set_style_pad_top(view, 18, 0);
    lv_obj_set_style_pad_bottom(view, 18, 0);
    lv_obj_set_style_pad_column(view, 26, 0);
    lv_obj_set_flex_flow(view, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(view, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(view, LV_OBJ_FLAG_SCROLLABLE);

    g_app.cover = lv_obj_create(view);
    lv_obj_remove_style_all(g_app.cover);
    lv_obj_set_size(g_app.cover, 188, 188);
    lv_obj_set_style_radius(g_app.cover, 14, 0);
    lv_obj_set_style_shadow_width(g_app.cover, 30, 0);
    lv_obj_set_style_shadow_opa(g_app.cover, 56, 0);
    lv_obj_set_style_shadow_offset_y(g_app.cover, 12, 0);

    info = lv_obj_create(view);
    lv_obj_remove_style_all(info);
    lv_obj_set_flex_grow(info, 1);
    lv_obj_set_height(info, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(info, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(info, 6, 0);
    lv_obj_clear_flag(info, LV_OBJ_FLAG_SCROLLABLE);

    g_app.title = am_text(info, "未播放", m->f_title, AM_TEXT);
    g_app.subtitle = am_text(info, "选择本地音乐或广播电台开始", m->f_body, AM_MUTED);

    g_app.lyrics = lv_obj_create(info);
    lv_obj_remove_style_all(g_app.lyrics);
    lv_obj_set_size(g_app.lyrics, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(g_app.lyrics, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(g_app.lyrics, 6, 0);
    lv_obj_clear_flag(g_app.lyrics, LV_OBJ_FLAG_SCROLLABLE);

    g_app.now_view = view;
    am_update_now_view();
}

static void am_show_view(am_view_t view)
{
    lv_obj_clean(g_app.sidebar);
    am_shell_build_sidebar(g_app.sidebar, view, (am_nav_cb_t)am_show_view, NULL);
    am_shell_set_header_title(g_app.crumb, am_view_label(view));
    lv_obj_clean(g_app.content);

    switch(view) {
        case AM_VIEW_RADIO:
            am_page_list_build_radio(g_app.content, g_app.radios, g_app.radio_count,
                                     am_player_current_radio_index(),
                                     am_on_radio_selected, NULL);
            break;
        case AM_VIEW_FAVORITES:
            am_page_list_build_favorites(g_app.content, g_app.locals, g_app.local_count,
                                         am_player_current_local_index(),
                                         am_on_local_selected, NULL);
            break;
        case AM_VIEW_NOW:
        default:
            am_build_now_view(g_app.content);
            break;
    }

    g_app.current_view = view;
    am_update_now_view();
    am_player_refresh_ui();
}

static void am_on_nav(am_view_t view, void *user)
{
    LV_UNUSED(user);
    am_show_view(view);
}

static void am_load_runtime_data(void)
{
    if(g_app.radios == NULL) {
        am_sources_csv_load("third-party/hls_player_demo/qa/production_test/config/sources.csv",
                            &g_app.radios, &g_app.radio_count);
    }
    if(g_app.locals == NULL) {
        am_local_scan_dir("third-party/hls_player_demo/test_file", &g_app.locals, &g_app.local_count);
    }
    am_player_set_sources(g_app.locals, g_app.local_count, g_app.radios, g_app.radio_count);
}

static void am_build_root(lv_obj_t *parent)
{
    const am_metrics_t *m = am_metrics();
    lv_coord_t width = lv_obj_get_width(parent);
    lv_coord_t height = lv_obj_get_height(parent);
    lv_coord_t main_width;
    lv_coord_t content_height;
    lv_display_t *disp;

    if(width <= 0 || height <= 0) {
        disp = lv_obj_get_display(parent);
        if(disp != NULL) {
            if(width <= 0) width = lv_display_get_horizontal_resolution(disp);
            if(height <= 0) height = lv_display_get_vertical_resolution(disp);
        }
    }

    am_free_runtime_data();
    memset(&g_app, 0, sizeof(g_app));
    g_app.root = parent;
    g_app.current_view = AM_VIEW_NOW;

    lv_obj_clean(parent);
    lv_obj_remove_style_all(parent);
    lv_obj_set_style_bg_color(parent, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);
    lv_obj_set_pos(parent, 0, 0);
    lv_obj_set_size(parent, width, height);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    main_width = width - m->sidebar_w;
    if(main_width < 0) main_width = width;
    content_height = height - 46 - m->player_h;
    if(content_height < 0) content_height = 0;

    g_app.sidebar = lv_obj_create(parent);
    lv_obj_remove_style_all(g_app.sidebar);
    lv_obj_set_pos(g_app.sidebar, 0, 0);
    lv_obj_set_size(g_app.sidebar, m->sidebar_w, height);

    g_app.main = lv_obj_create(parent);
    lv_obj_remove_style_all(g_app.main);
    lv_obj_set_pos(g_app.main, m->sidebar_w, 0);
    lv_obj_set_size(g_app.main, main_width, height);
    lv_obj_set_scrollbar_mode(g_app.main, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(g_app.main, LV_OBJ_FLAG_SCROLLABLE);

    g_app.header = lv_obj_create(g_app.main);
    lv_obj_remove_style_all(g_app.header);
    lv_obj_set_pos(g_app.header, 0, 0);
    lv_obj_set_size(g_app.header, LV_PCT(100), 46);
    lv_obj_set_style_pad_left(g_app.header, 22, 0);
    lv_obj_set_style_pad_right(g_app.header, 22, 0);
    lv_obj_set_style_border_side(g_app.header, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(g_app.header, 1, 0);
    lv_obj_set_style_border_color(g_app.header, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_opa(g_app.header, 24, 0);
    lv_obj_set_flex_flow(g_app.header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(g_app.header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    g_app.crumb = am_text(g_app.header, "正在播放", m->f_body, AM_TEXT);

    g_app.content = lv_obj_create(g_app.main);
    lv_obj_remove_style_all(g_app.content);
    lv_obj_set_pos(g_app.content, 0, 46);
    lv_obj_set_size(g_app.content, LV_PCT(100), content_height);
    lv_obj_set_scrollbar_mode(g_app.content, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(g_app.content, LV_OBJ_FLAG_SCROLLABLE);

    g_app.player = lv_obj_create(g_app.main);
    lv_obj_remove_style_all(g_app.player);
    lv_obj_set_pos(g_app.player, 0, height - m->player_h);
    lv_obj_set_size(g_app.player, LV_PCT(100), m->player_h);

    am_player_init();
    g_app.mini = am_shell_build_miniplayer(g_app.player,
                                           am_player_on_mode,
                                           am_player_on_prev,
                                           am_player_on_play_pause,
                                           am_player_on_next,
                                           am_on_playlist_toggle);
    am_player_bind_miniplayer(&g_app.mini);
    am_load_runtime_data();
    am_shell_build_sidebar(g_app.sidebar, AM_VIEW_NOW, am_on_nav, NULL);
    am_show_view(AM_VIEW_NOW);
}

void apple_music_create(void)
{
    am_build_root(lv_screen_active());
}

void apple_music_create_in(lv_obj_t *parent)
{
    am_build_root(parent);
}

void apple_music_destroy(void)
{
    am_player_deinit();
    am_free_runtime_data();
    memset(&g_app, 0, sizeof(g_app));
}
