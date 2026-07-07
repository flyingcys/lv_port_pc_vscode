#include "am_shell.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "am_icons.h"
#include "am_metrics.h"
#include "am_state.h"
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

/* 无边框图标按钮(mockup 传输键是纯字形,无圆圈/描边)。box=触控命中区,glyph 用较大字号。 */
static lv_obj_t *am_icon_button(lv_obj_t *parent, const char *glyph, const lv_font_t *font,
                                int box, lv_color_t color)
{
    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, box, box);
    lv_obj_set_style_radius(btn, 10, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, 10, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label = am_text(btn, glyph, font, color);
    lv_obj_center(label);
    return btn;
}

static lv_obj_t *am_text_button(lv_obj_t *parent, const char *text, const lv_font_t *font,
                                int box, lv_color_t color)
{
    return am_icon_button(parent, text, font, box, color);
}

static lv_obj_t *am_transport_button_base(lv_obj_t *parent, int box)
{
    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, box, box);
    lv_obj_set_style_radius(btn, 10, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, 8, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    return btn;
}

static lv_obj_t *am_transport_symbol_button(lv_obj_t *parent, const char *glyph,
                                            const lv_font_t *font, int box,
                                            lv_color_t color, lv_obj_t **icon_out)
{
    lv_obj_t *btn = am_transport_button_base(parent, box);
    lv_obj_t *label = am_text(btn, glyph, font, color);
    lv_obj_center(label);
    if(icon_out != NULL) *icon_out = label;
    return btn;
}

static lv_obj_t *am_transport_play_button(lv_obj_t *parent, lv_color_t color,
                                          lv_obj_t **play_icon_out, lv_obj_t **pause_icon_out)
{
    lv_obj_t *btn = am_transport_button_base(parent, 38);
    lv_obj_t *play_icon = am_text(btn, LV_SYMBOL_PLAY, &lv_font_montserrat_24, color);
    lv_obj_t *pause_icon = am_text(btn, LV_SYMBOL_PAUSE, &lv_font_montserrat_22, color);

    lv_obj_center(play_icon);
    lv_obj_center(pause_icon);
    lv_obj_add_flag(pause_icon, LV_OBJ_FLAG_HIDDEN);

    if(play_icon_out != NULL) *play_icon_out = play_icon;
    if(pause_icon_out != NULL) *pause_icon_out = pause_icon;
    return btn;
}

static lv_obj_t *am_playlist_button(lv_obj_t *parent, int box, lv_color_t color)
{
    lv_obj_t *btn = am_transport_button_base(parent, box);
    lv_obj_t *bar;
    lv_obj_t *dot;
    lv_obj_t *stem;

    bar = lv_obj_create(btn);
    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, 12, 2);
    lv_obj_set_style_radius(bar, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(bar, color, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_pos(bar, 5, 7);

    bar = lv_obj_create(btn);
    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, 10, 2);
    lv_obj_set_style_radius(bar, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(bar, color, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_pos(bar, 5, 13);

    bar = lv_obj_create(btn);
    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, 8, 2);
    lv_obj_set_style_radius(bar, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(bar, color, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_pos(bar, 5, 19);

    stem = lv_obj_create(btn);
    lv_obj_remove_style_all(stem);
    lv_obj_set_size(stem, 2, 8);
    lv_obj_set_style_radius(stem, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(stem, color, 0);
    lv_obj_set_style_bg_opa(stem, LV_OPA_COVER, 0);
    lv_obj_set_pos(stem, 20, 8);

    dot = lv_obj_create(btn);
    lv_obj_remove_style_all(dot);
    lv_obj_set_size(dot, 6, 6);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dot, color, 0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    lv_obj_set_pos(dot, 17, 17);

    return btn;
}

void am_shell_build_sidebar(lv_obj_t *sidebar,
                            am_view_t active_view,
                            const am_local_item_t *locals,
                            size_t local_count,
                            am_nav_cb_t cb,
                            void *user)
{
    const am_metrics_t *m = am_metrics();
    am_nav_ctx_t *ctx;
    size_t recent[4] = {0U};
    size_t recent_count = am_state_collect_recent(locals, local_count, recent, 4U);
    size_t i;

    lv_obj_remove_style_all(sidebar);
    /* remove_style_all 会清掉调用方设的尺寸,这里按 metrics 重设(满高) */
    lv_obj_set_size(sidebar, m->sidebar_w, LV_PCT(100));
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
        /* 注意:sidebar 是常驻对象,每次切页 lv_obj_clean 只清子对象、不删 sidebar 本体。
         * 故 ctx 的释放回调必须挂在"每次重建的子对象"(traffic)上,否则每次切页泄漏一份 ctx。 */
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
        if(ctx != NULL) lv_obj_add_event_cb(traffic, am_nav_ctx_free_cb, LV_EVENT_DELETE, ctx);

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
        lv_obj_t *title = am_text(sidebar, "音乐", m->f_metric, AM_TEXT);
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
    for(i = 0; i < recent_count; i++) {
        lv_obj_t *recent_label = am_text(sidebar, locals[recent[i]].title, m->f_body, AM_MUTED);
        lv_label_set_long_mode(recent_label, LV_LABEL_LONG_DOT);
        lv_obj_set_width(recent_label, LV_PCT(100));
        lv_obj_set_style_pad_left(recent_label, 8, 0);
        lv_obj_set_style_pad_top(recent_label, 5, 0);
    }
}

void am_shell_set_header_title(lv_obj_t *header_label, const char *title)
{
    if(header_label == NULL) return;
    lv_label_set_text(header_label, title != NULL ? title : "");
}

am_miniplayer_handles_t am_shell_build_miniplayer_ex(lv_obj_t *player,
                                                     am_simple_event_cb_t on_mode,
                                                     am_simple_event_cb_t on_prev,
                                                     am_simple_event_cb_t on_play,
                                                     am_simple_event_cb_t on_next,
                                                     am_simple_event_cb_t on_open_file,
                                                     am_simple_event_cb_t on_playlist)
{
    am_miniplayer_handles_t h;
    const am_metrics_t *m = am_metrics();

    memset(&h, 0, sizeof(h));

    lv_obj_remove_style_all(player);
    /* remove_style_all 清掉调用方设的尺寸;重设为满宽固定高(否则塌成默认 130 落到顶部) */
    lv_obj_set_size(player, LV_PCT(100), m->player_h);
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
            h.progress_track = track;

            {
                lv_obj_t *bar = lv_obj_create(track);
                lv_obj_remove_style_all(bar);
                lv_obj_set_size(bar, LV_PCT(100), 4);
                lv_obj_set_style_radius(bar, 2, 0);
                lv_obj_set_style_bg_color(bar, lv_color_hex(0xd6d6db), 0);
                lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
                lv_obj_align(bar, LV_ALIGN_CENTER, 0, 0);
                lv_obj_clear_flag(bar, LV_OBJ_FLAG_CLICKABLE);   /* 让点击穿透到 track */
            }

            h.progress_fill = lv_obj_create(track);
            lv_obj_remove_style_all(h.progress_fill);
            lv_obj_set_size(h.progress_fill, LV_PCT(0), 4);
            lv_obj_set_style_radius(h.progress_fill, 2, 0);
            lv_obj_set_style_bg_color(h.progress_fill, lv_color_hex(0xfa2d48), 0);
            lv_obj_set_style_bg_opa(h.progress_fill, LV_OPA_COVER, 0);
            lv_obj_align(h.progress_fill, LV_ALIGN_LEFT_MID, 0, 0);
            lv_obj_clear_flag(h.progress_fill, LV_OBJ_FLAG_CLICKABLE);

            /* 进度末端白色圆点 knob(mockup .sknob),位置由 refresh_ui 按进度设置 */
            h.knob = lv_obj_create(track);
            lv_obj_remove_style_all(h.knob);
            lv_obj_set_size(h.knob, 12, 12);
            lv_obj_set_style_radius(h.knob, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(h.knob, AM_WHITE, 0);
            lv_obj_set_style_bg_opa(h.knob, LV_OPA_COVER, 0);
            lv_obj_set_style_shadow_width(h.knob, 4, 0);
            lv_obj_set_style_shadow_opa(h.knob, 76, 0);
            lv_obj_set_style_shadow_offset_y(h.knob, 1, 0);
            lv_obj_align(h.knob, LV_ALIGN_LEFT_MID, 0, 0);
            lv_obj_clear_flag(h.knob, LV_OBJ_FLAG_CLICKABLE);
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

        /* mockup 的播放条无标题/副标题(它们在"正在播放"页);传输键整组居中,音量右钉 */
        {
            lv_obj_t *ctrls = lv_obj_create(pbar);
            lv_obj_remove_style_all(ctrls);
            lv_obj_set_height(ctrls, LV_SIZE_CONTENT);
            lv_obj_set_flex_grow(ctrls, 1);   /* .ctrls{flex:1;justify-content:center} */
            lv_obj_set_flex_flow(ctrls, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(ctrls, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_style_pad_column(ctrls, 26, 0);
            lv_obj_clear_flag(ctrls, LV_OBJ_FLAG_SCROLLABLE);

            lv_color_t ctrl_col = lv_color_hex(0x2a2a2e);
            h.btn_mode = am_text_button(ctrls, "SEQ", m->f_label, 30, lv_color_hex(0xfa2d48));
            h.btn_prev = am_transport_symbol_button(ctrls, LV_SYMBOL_PREV, &lv_font_montserrat_20,
                                                    32, ctrl_col, NULL);
            h.btn_play = am_transport_play_button(ctrls, ctrl_col, &h.play_icon, &h.pause_icon);
            h.btn_next = am_transport_symbol_button(ctrls, LV_SYMBOL_NEXT, &lv_font_montserrat_20,
                                                    32, ctrl_col, NULL);
            h.btn_open_file = am_text_button(ctrls, AM_ICON_OPEN_FILE, m->f_label, 30, ctrl_col);
            h.btn_playlist = am_playlist_button(ctrls, 30, ctrl_col);
            lv_obj_set_style_margin_left(h.btn_prev, 2, 0);
            lv_obj_set_style_margin_right(h.btn_next, 2, 0);

            if(on_mode != NULL) lv_obj_add_event_cb(h.btn_mode, on_mode, LV_EVENT_CLICKED, NULL);
            if(on_prev != NULL) lv_obj_add_event_cb(h.btn_prev, on_prev, LV_EVENT_CLICKED, NULL);
            if(on_play != NULL) lv_obj_add_event_cb(h.btn_play, on_play, LV_EVENT_CLICKED, NULL);
            if(on_next != NULL) lv_obj_add_event_cb(h.btn_next, on_next, LV_EVENT_CLICKED, NULL);
            if(on_open_file != NULL) {
                lv_obj_add_event_cb(h.btn_open_file, on_open_file, LV_EVENT_CLICKED, NULL);
            }
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

            h.volume_icon = am_text(vol, AM_ICON_SPEAKER, m->f_label, AM_MUTED);
            {
                lv_obj_t *track = lv_obj_create(vol);
                lv_obj_remove_style_all(track);
                lv_obj_set_size(track, 70, 4);
                lv_obj_set_style_radius(track, 2, 0);
                lv_obj_set_style_bg_color(track, lv_color_hex(0xd6d6db), 0);
                lv_obj_set_style_bg_opa(track, LV_OPA_COVER, 0);
                lv_obj_clear_flag(track, LV_OBJ_FLAG_SCROLLABLE);
                h.volume_track = track;

                h.volume_fill = lv_obj_create(track);
                lv_obj_remove_style_all(h.volume_fill);
                lv_obj_set_size(h.volume_fill, LV_PCT(65), 4);
                lv_obj_set_style_radius(h.volume_fill, 2, 0);
                lv_obj_set_style_bg_color(h.volume_fill, AM_MUTED, 0);
                lv_obj_set_style_bg_opa(h.volume_fill, LV_OPA_COVER, 0);
                lv_obj_align(h.volume_fill, LV_ALIGN_LEFT_MID, 0, 0);
                lv_obj_clear_flag(h.volume_fill, LV_OBJ_FLAG_CLICKABLE);   /* 让点击穿透到 volume_track */
            }
        }
    }

    /* 弹层挂到 main(player 的父,占满右侧区域)而非 78px 的 player——否则会被父裁掉。
     * FLOATING 使其不被 main 的 flex 布局接管、由 align 定位,浮于内容之上。 */
    h.playlist_popup = lv_obj_create(lv_obj_get_parent(player));
    lv_obj_remove_style_all(h.playlist_popup);
    lv_obj_add_flag(h.playlist_popup, LV_OBJ_FLAG_FLOATING);
    lv_obj_clear_flag(h.playlist_popup, LV_OBJ_FLAG_SCROLLABLE);
    /* 尺寸/定位随分辨率自适应:弹层浮在 header 与 player 之间,底部贴 player 上方。
     * 480x272 下固定 300x240+(-96) 会顶部溢出屏幕(顶=272-96-240<0),故按可用高收缩。
     * (header 高 46 见 am_build_root;main 高=屏幕高,main 宽=屏幕宽-sidebar_w。) */
    {
        lv_display_t *disp = lv_display_get_default();
        int scr_w = lv_display_get_horizontal_resolution(disp);
        int scr_h = lv_display_get_vertical_resolution(disp);
        int main_w = scr_w - m->sidebar_w;
        int margin = 14;
        int bottom_off = m->player_h + 12;               /* 浮在 player 上方 */
        int max_h = scr_h - bottom_off - 46 - 8;          /* 顶部不越过 header(46)+留白 */
        int max_w = main_w - 2 * margin;
        int pw = (300 < max_w) ? 300 : max_w;
        int ph = (240 < max_h) ? 240 : max_h;
        lv_obj_set_size(h.playlist_popup, pw, ph);
        lv_obj_align(h.playlist_popup, LV_ALIGN_BOTTOM_RIGHT, -margin, -bottom_off);
    }
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
    lv_obj_add_flag(h.playlist_popup, LV_OBJ_FLAG_HIDDEN);

    h.playlist_scroll_track = lv_obj_create(h.playlist_popup);
    lv_obj_remove_style_all(h.playlist_scroll_track);
    lv_obj_clear_flag(h.playlist_scroll_track, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(h.playlist_scroll_track, 8, LV_PCT(100));
    lv_obj_align(h.playlist_scroll_track, LV_ALIGN_RIGHT_MID, -6, 0);
    lv_obj_set_style_bg_color(h.playlist_scroll_track, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(h.playlist_scroll_track, 20, 0);
    lv_obj_set_style_radius(h.playlist_scroll_track, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_flag(h.playlist_scroll_track, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_flag(h.playlist_scroll_track, LV_OBJ_FLAG_HIDDEN);

    h.playlist_scroll_thumb = lv_obj_create(h.playlist_scroll_track);
    lv_obj_remove_style_all(h.playlist_scroll_thumb);
    lv_obj_set_size(h.playlist_scroll_thumb, LV_PCT(100), 32);
    lv_obj_align(h.playlist_scroll_thumb, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(h.playlist_scroll_thumb, lv_color_hex(0x7d7d7d), 0);
    lv_obj_set_style_bg_opa(h.playlist_scroll_thumb, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(h.playlist_scroll_thumb, LV_RADIUS_CIRCLE, 0);

    {
        lv_obj_t *head = lv_obj_create(h.playlist_popup);
        lv_obj_remove_style_all(head);
        lv_obj_set_size(head, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(head, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(head, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_left(head, 15, 0);
        lv_obj_set_style_pad_right(head, 15, 0);
        lv_obj_set_style_pad_top(head, 12, 0);
        lv_obj_set_style_pad_bottom(head, 12, 0);
        lv_obj_set_style_pad_column(head, 8, 0);
        lv_obj_set_style_border_side(head, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_width(head, 1, 0);
        lv_obj_set_style_border_color(head, lv_color_hex(0x000000), 0);
        lv_obj_set_style_border_opa(head, 24, 0);
        lv_obj_clear_flag(head, LV_OBJ_FLAG_SCROLLABLE);
        am_text(head, "播放列表", m->f_body, AM_TEXT);
        h.playlist_count_label = am_text(head, "", m->f_label, AM_MUTED);
    }

    h.playlist_list = lv_obj_create(h.playlist_popup);
    lv_obj_remove_style_all(h.playlist_list);
    /* 高度用 flex_grow 精确吃掉 head 之后的剩余空间;若用 PCT(100) 则等于弹层满高,
     * 叠上 head 后底部溢出弹层被裁掉(最后一行永远看不全),故用 grow。 */
    lv_obj_set_width(h.playlist_list, LV_PCT(100));
    lv_obj_set_flex_grow(h.playlist_list, 1);
    lv_obj_set_style_pad_all(h.playlist_list, 6, 0);
    lv_obj_set_style_pad_row(h.playlist_list, 4, 0);
    lv_obj_set_style_pad_right(h.playlist_list, 16, 0);
    lv_obj_set_flex_flow(h.playlist_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(h.playlist_list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(h.playlist_list, LV_SCROLLBAR_MODE_OFF);

    lv_obj_update_layout(h.playlist_popup);
    lv_obj_set_height(h.playlist_scroll_track, lv_obj_get_height(h.playlist_list));
    lv_obj_set_width(h.playlist_scroll_track, 8);
    lv_obj_align_to(h.playlist_scroll_track, h.playlist_list, LV_ALIGN_TOP_RIGHT, -6, 0);
    lv_obj_move_foreground(h.playlist_scroll_track);

    return h;
}

am_miniplayer_handles_t am_shell_build_miniplayer(lv_obj_t *player,
                                                  am_simple_event_cb_t on_mode,
                                                  am_simple_event_cb_t on_prev,
                                                  am_simple_event_cb_t on_play,
                                                  am_simple_event_cb_t on_next,
                                                  am_simple_event_cb_t on_playlist)
{
    return am_shell_build_miniplayer_ex(player, on_mode, on_prev, on_play, on_next, NULL,
                                        on_playlist);
}
