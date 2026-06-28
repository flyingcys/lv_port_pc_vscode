#include "am_shell.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "am_icons.h"
#include "am_metrics.h"
#include "am_theme.h"
#include "am_widgets.h"

typedef struct {
    am_nav_cb_t cb;
    void *user;
} am_nav_ctx_t;

static void am_nav_click_cb(lv_event_t *e)
{
    am_nav_ctx_t *ctx = (am_nav_ctx_t *)lv_event_get_user_data(e);
    intptr_t index = (intptr_t)lv_obj_get_user_data(lv_event_get_target(e));
    if(ctx != NULL && ctx->cb != NULL) ctx->cb((am_view_t)index, ctx->user);
}

static void am_nav_ctx_free_cb(lv_event_t *e)
{
    lv_free(lv_event_get_user_data(e));
}

static lv_obj_t *am_simple_button(lv_obj_t *parent, const char *glyph, const lv_font_t *font,
                                  int size, bool filled)
{
    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, size, size);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn, filled ? AM_TEXT : AM_WHITE, 0);
    lv_obj_set_style_bg_opa(btn, filled ? LV_OPA_COVER : 0, 0);
    if(!filled) {
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xd6d6db), 0);
        lv_obj_set_style_border_opa(btn, LV_OPA_COVER, 0);
    }
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label = am_text(btn, glyph, font, filled ? AM_WHITE : AM_TEXT);
    lv_obj_center(label);
    return btn;
}

void am_shell_build_sidebar(lv_obj_t *sidebar, am_view_t active_view, am_nav_cb_t cb, void *user)
{
    const am_metrics_t *m = am_metrics();
    am_nav_ctx_t *ctx;
    size_t i;

    lv_obj_remove_style_all(sidebar);
    lv_obj_set_style_bg_color(sidebar, lv_color_hex(0xf0f0f3), 0);
    lv_obj_set_style_bg_opa(sidebar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(sidebar, LV_BORDER_SIDE_RIGHT, 0);
    lv_obj_set_style_border_width(sidebar, 1, 0);
    lv_obj_set_style_border_color(sidebar, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_opa(sidebar, 24, 0);
    lv_obj_set_style_pad_top(sidebar, 0, 0);
    lv_obj_set_style_pad_left(sidebar, 12, 0);
    lv_obj_set_style_pad_right(sidebar, 12, 0);
    lv_obj_set_style_pad_bottom(sidebar, 12, 0);
    lv_obj_set_style_pad_row(sidebar, 0, 0);
    lv_obj_set_flex_flow(sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(sidebar, LV_SCROLLBAR_MODE_OFF);

    ctx = lv_malloc(sizeof(*ctx));
    if(ctx != NULL) {
        ctx->cb = cb;
        ctx->user = user;
        lv_obj_add_event_cb(sidebar, am_nav_ctx_free_cb, LV_EVENT_DELETE, ctx);
    }

    {
        lv_obj_t *traffic = lv_obj_create(sidebar);
        lv_obj_remove_style_all(traffic);
        lv_obj_set_size(traffic, LV_PCT(100), 44);
        lv_obj_set_flex_flow(traffic, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(traffic, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_left(traffic, 4, 0);
        lv_obj_set_style_pad_column(traffic, 8, 0);
        lv_obj_clear_flag(traffic, LV_OBJ_FLAG_SCROLLABLE);

        lv_color_t colors[3] = {
            lv_color_hex(0xff5f57),
            lv_color_hex(0xfebc2e),
            lv_color_hex(0x28c840),
        };
        for(i = 0; i < 3U; i++) {
            lv_obj_t *dot = lv_obj_create(traffic);
            lv_obj_remove_style_all(dot);
            lv_obj_set_size(dot, 12, 12);
            lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(dot, colors[i], 0);
            lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        }
    }

    {
        lv_obj_t *title = am_text(sidebar, "音乐", m->f_h2, AM_TEXT);
        lv_obj_set_style_pad_left(title, 6, 0);
        lv_obj_set_style_pad_bottom(title, 12, 0);
    }

    {
        lv_obj_t *nav = lv_obj_create(sidebar);
        lv_obj_remove_style_all(nav);
        lv_obj_set_size(nav, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(nav, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(nav, 2, 0);
        lv_obj_clear_flag(nav, LV_OBJ_FLAG_SCROLLABLE);

        for(i = 0; i < AM_VIEW_COUNT; i++) {
            lv_obj_t *item = lv_obj_create(nav);
            bool active = ((am_view_t)i == active_view);
            lv_obj_remove_style_all(item);
            lv_obj_set_size(item, LV_PCT(100), LV_SIZE_CONTENT);
            lv_obj_set_style_radius(item, 7, 0);
            lv_obj_set_style_pad_hor(item, 8, 0);
            lv_obj_set_style_pad_ver(item, 7, 0);
            lv_obj_set_style_pad_column(item, 9, 0);
            lv_obj_set_style_bg_color(item, lv_color_hex(0x000000), 0);
            lv_obj_set_style_bg_opa(item, active ? 20 : LV_OPA_TRANSP, 0);
            lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_user_data(item, (void *)(intptr_t)i);
            if(ctx != NULL) lv_obj_add_event_cb(item, am_nav_click_cb, LV_EVENT_CLICKED, ctx);

            {
                lv_obj_t *icon = am_text(item, am_nav_items[i].icon, m->f_icon, lv_color_hex(0xfa2d48));
                LV_UNUSED(icon);
            }
            {
                lv_obj_t *label = am_text(item, am_nav_items[i].label, m->f_body, AM_TEXT);
                if(active) lv_obj_set_style_text_font(label, m->f_strong, 0);
            }
        }
    }

    am_section_title(sidebar, "最近播放");
    for(i = 0; i < 4U; i++) {
        lv_obj_t *recent = am_text(sidebar, am_recent_titles[i], m->f_body, AM_MUTED);
        lv_label_set_long_mode(recent, LV_LABEL_LONG_DOT);
        lv_obj_set_width(recent, LV_PCT(100));
        lv_obj_set_style_pad_left(recent, 8, 0);
        lv_obj_set_style_pad_top(recent, 5, 0);
    }
}

void am_shell_set_header_title(lv_obj_t *header_label, const char *title)
{
    if(header_label == NULL) return;
    lv_label_set_text(header_label, title != NULL ? title : "");
}

am_miniplayer_handles_t am_shell_build_miniplayer(lv_obj_t *player,
                                                  am_simple_event_cb_t on_mode,
                                                  am_simple_event_cb_t on_prev,
                                                  am_simple_event_cb_t on_play,
                                                  am_simple_event_cb_t on_next,
                                                  am_simple_event_cb_t on_playlist)
{
    am_miniplayer_handles_t h;
    const am_metrics_t *m = am_metrics();

    memset(&h, 0, sizeof(h));

    lv_obj_remove_style_all(player);
    lv_obj_set_style_bg_color(player, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(player, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(player, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_width(player, 1, 0);
    lv_obj_set_style_border_color(player, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_opa(player, 24, 0);
    lv_obj_set_style_pad_left(player, 24, 0);
    lv_obj_set_style_pad_right(player, 24, 0);
    lv_obj_set_style_pad_top(player, 10, 0);
    lv_obj_set_style_pad_bottom(player, 14, 0);
    lv_obj_set_scrollbar_mode(player, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(player, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(player, 10, 0);

    {
        lv_obj_t *scrub = lv_obj_create(player);
        lv_obj_remove_style_all(scrub);
        lv_obj_set_size(scrub, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(scrub, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(scrub, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(scrub, 10, 0);
        lv_obj_clear_flag(scrub, LV_OBJ_FLAG_SCROLLABLE);

        h.time_cur = am_text(scrub, "--:--", m->f_label, AM_MUTED);

        {
            lv_obj_t *track = lv_obj_create(scrub);
            lv_obj_remove_style_all(track);
            lv_obj_set_height(track, 12);
            lv_obj_set_flex_grow(track, 1);
            lv_obj_clear_flag(track, LV_OBJ_FLAG_SCROLLABLE);

            {
                lv_obj_t *bar = lv_obj_create(track);
                lv_obj_remove_style_all(bar);
                lv_obj_set_size(bar, LV_PCT(100), 4);
                lv_obj_set_style_radius(bar, 2, 0);
                lv_obj_set_style_bg_color(bar, lv_color_hex(0xd6d6db), 0);
                lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
                lv_obj_align(bar, LV_ALIGN_CENTER, 0, 0);
            }

            h.progress_fill = lv_obj_create(track);
            lv_obj_remove_style_all(h.progress_fill);
            lv_obj_set_size(h.progress_fill, LV_PCT(0), 4);
            lv_obj_set_style_radius(h.progress_fill, 2, 0);
            lv_obj_set_style_bg_color(h.progress_fill, lv_color_hex(0xfa2d48), 0);
            lv_obj_set_style_bg_opa(h.progress_fill, LV_OPA_COVER, 0);
            lv_obj_align(h.progress_fill, LV_ALIGN_LEFT_MID, 0, 0);
        }

        h.time_total = am_text(scrub, "--:--", m->f_label, AM_MUTED);
    }

    {
        lv_obj_t *pbar = lv_obj_create(player);
        lv_obj_remove_style_all(pbar);
        lv_obj_set_size(pbar, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(pbar, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(pbar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(pbar, LV_OBJ_FLAG_SCROLLABLE);

        {
            lv_obj_t *meta = lv_obj_create(pbar);
            lv_obj_remove_style_all(meta);
            lv_obj_set_width(meta, 220);
            lv_obj_set_height(meta, LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(meta, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_style_pad_row(meta, 4, 0);
            lv_obj_clear_flag(meta, LV_OBJ_FLAG_SCROLLABLE);

            h.title_label = am_text(meta, "未播放", m->f_body, AM_TEXT);
            h.subtitle_label = am_text(meta, "选择本地音乐或广播电台开始", m->f_label, AM_MUTED);
        }

        {
            lv_obj_t *ctrls = lv_obj_create(pbar);
            lv_obj_remove_style_all(ctrls);
            lv_obj_set_size(ctrls, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(ctrls, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(ctrls, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_style_pad_column(ctrls, 26, 0);
            lv_obj_clear_flag(ctrls, LV_OBJ_FLAG_SCROLLABLE);

            h.btn_mode = am_simple_button(ctrls, AM_ICON_REPLAY, m->f_icon, 24, false);
            h.btn_prev = am_simple_button(ctrls, AM_ICON_PREV, m->f_icon, 24, false);
            h.btn_play = am_simple_button(ctrls, AM_ICON_PLAY, m->f_icon, 30, true);
            h.play_icon = lv_obj_get_child(h.btn_play, 0);
            h.btn_next = am_simple_button(ctrls, AM_ICON_NEXT, m->f_icon, 24, false);
            h.btn_playlist = am_simple_button(ctrls, AM_ICON_LIST, m->f_icon, 24, false);

            if(on_mode != NULL) lv_obj_add_event_cb(h.btn_mode, on_mode, LV_EVENT_CLICKED, NULL);
            if(on_prev != NULL) lv_obj_add_event_cb(h.btn_prev, on_prev, LV_EVENT_CLICKED, NULL);
            if(on_play != NULL) lv_obj_add_event_cb(h.btn_play, on_play, LV_EVENT_CLICKED, NULL);
            if(on_next != NULL) lv_obj_add_event_cb(h.btn_next, on_next, LV_EVENT_CLICKED, NULL);
            if(on_playlist != NULL) lv_obj_add_event_cb(h.btn_playlist, on_playlist, LV_EVENT_CLICKED, NULL);
        }

        {
            lv_obj_t *vol = lv_obj_create(pbar);
            lv_obj_remove_style_all(vol);
            lv_obj_set_size(vol, 110, LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(vol, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(vol, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_style_pad_column(vol, 7, 0);
            lv_obj_clear_flag(vol, LV_OBJ_FLAG_SCROLLABLE);

            am_text(vol, AM_ICON_SPEAKER, m->f_label, AM_MUTED);
            {
                lv_obj_t *track = lv_obj_create(vol);
                lv_obj_remove_style_all(track);
                lv_obj_set_size(track, 70, 4);
                lv_obj_set_style_radius(track, 2, 0);
                lv_obj_set_style_bg_color(track, lv_color_hex(0xd6d6db), 0);
                lv_obj_set_style_bg_opa(track, LV_OPA_COVER, 0);
                lv_obj_clear_flag(track, LV_OBJ_FLAG_SCROLLABLE);

                h.volume_fill = lv_obj_create(track);
                lv_obj_remove_style_all(h.volume_fill);
                lv_obj_set_size(h.volume_fill, LV_PCT(65), 4);
                lv_obj_set_style_radius(h.volume_fill, 2, 0);
                lv_obj_set_style_bg_color(h.volume_fill, AM_MUTED, 0);
                lv_obj_set_style_bg_opa(h.volume_fill, LV_OPA_COVER, 0);
                lv_obj_align(h.volume_fill, LV_ALIGN_LEFT_MID, 0, 0);
            }
        }
    }

    h.playlist_popup = lv_obj_create(player);
    lv_obj_remove_style_all(h.playlist_popup);
    lv_obj_set_size(h.playlist_popup, 300, 240);
    lv_obj_set_style_bg_color(h.playlist_popup, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(h.playlist_popup, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(h.playlist_popup, 1, 0);
    lv_obj_set_style_border_color(h.playlist_popup, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_opa(h.playlist_popup, 24, 0);
    lv_obj_set_style_radius(h.playlist_popup, 12, 0);
    lv_obj_set_style_shadow_width(h.playlist_popup, 24, 0);
    lv_obj_set_style_shadow_opa(h.playlist_popup, 56, 0);
    lv_obj_set_style_shadow_offset_y(h.playlist_popup, 8, 0);
    lv_obj_set_style_pad_all(h.playlist_popup, 0, 0);
    lv_obj_set_style_pad_row(h.playlist_popup, 0, 0);
    lv_obj_set_flex_flow(h.playlist_popup, LV_FLEX_FLOW_COLUMN);
    lv_obj_align(h.playlist_popup, LV_ALIGN_BOTTOM_RIGHT, -18, -96);
    lv_obj_add_flag(h.playlist_popup, LV_OBJ_FLAG_HIDDEN);

    {
        lv_obj_t *head = lv_obj_create(h.playlist_popup);
        lv_obj_remove_style_all(head);
        lv_obj_set_size(head, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_left(head, 15, 0);
        lv_obj_set_style_pad_right(head, 15, 0);
        lv_obj_set_style_pad_top(head, 12, 0);
        lv_obj_set_style_pad_bottom(head, 12, 0);
        lv_obj_set_style_border_side(head, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_width(head, 1, 0);
        lv_obj_set_style_border_color(head, lv_color_hex(0x000000), 0);
        lv_obj_set_style_border_opa(head, 24, 0);
        lv_obj_clear_flag(head, LV_OBJ_FLAG_SCROLLABLE);
        am_text(head, "播放列表", m->f_body, AM_TEXT);
    }

    h.playlist_list = lv_obj_create(h.playlist_popup);
    lv_obj_remove_style_all(h.playlist_list);
    lv_obj_set_size(h.playlist_list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(h.playlist_list, 6, 0);
    lv_obj_set_style_pad_row(h.playlist_list, 4, 0);
    lv_obj_set_flex_flow(h.playlist_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(h.playlist_list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(h.playlist_list, LV_SCROLLBAR_MODE_AUTO);

    return h;
}
