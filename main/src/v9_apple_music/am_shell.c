/* main/src/v9_apple_music/am_shell.c */
#include "am_shell.h"
#include "am_player.h"
#include "am_widgets.h"
#include "am_theme.h"
#include "am_fonts.h"
#include "am_metrics.h"
#include "am_icons.h"
#include "am_data.h"
#include <stdint.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  文件级 nav 回调存储                                               */
/* ------------------------------------------------------------------ */
typedef struct {
    am_nav_cb_t cb;
    void       *user;
} nav_ctx_t;

static void nav_click_cb(lv_event_t *e)
{
    nav_ctx_t *ctx = (nav_ctx_t *)lv_event_get_user_data(e);
    lv_obj_t  *obj = lv_event_get_target(e);
    int        idx = (int)(intptr_t)lv_obj_get_user_data(obj);
    if(ctx && ctx->cb) ctx->cb(idx, ctx->user);
}

static void nav_ctx_free_cb(lv_event_t *e)
{
    lv_free(lv_event_get_user_data(e));
}

/* ------------------------------------------------------------------ */
/*  侧栏构建                                                          */
/* ------------------------------------------------------------------ */
void am_shell_build_sidebar(lv_obj_t *sidebar, int active_nav, am_nav_cb_t cb, void *user)
{
    const am_metrics_t *m = am_metrics();
    const am_theme_t *t = am_theme_get(am_theme_current());

    /* 分配回调上下文(随 sidebar 删除时由 LVGL 的事件机制保持,这里 malloc 并绑定 DELETE 释放) */
    nav_ctx_t *ctx = lv_malloc(sizeof(nav_ctx_t));
    if(!ctx) return;
    ctx->cb   = cb;
    ctx->user = user;
    lv_obj_add_event_cb(sidebar, nav_ctx_free_cb, LV_EVENT_DELETE, ctx);

    /* sidebar 自身:near-white tint 背景 + 右边线 + flex 列 */
    lv_obj_set_style_bg_color(sidebar, lv_color_hex(0xeffaf6), 0);
    lv_obj_set_style_bg_opa(sidebar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(sidebar, LV_BORDER_SIDE_RIGHT, 0);
    lv_obj_set_style_border_width(sidebar, 1, 0);
    lv_obj_set_style_border_color(sidebar, AM_WHITE, 0);
    lv_obj_set_style_border_opa(sidebar, (lv_opa_t)(0.74f * 255), 0);
    lv_obj_set_style_pad_top(sidebar, m->content_pad / 2 + 6, 0);
    lv_obj_set_style_pad_right(sidebar, m->panel_pad - 4, 0);
    lv_obj_set_style_pad_bottom(sidebar, m->content_pad / 2 + 6, 0);
    lv_obj_set_style_pad_left(sidebar, m->panel_pad - 2, 0);
    lv_obj_set_flex_flow(sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(sidebar, m->page_gap - 6, 0);
    lv_obj_set_scrollbar_mode(sidebar, LV_SCROLLBAR_MODE_OFF);

    /* ---- 1. 品牌行 ---- */
    lv_obj_t *brand = lv_obj_create(sidebar);
    lv_obj_remove_style_all(brand);
    lv_obj_set_size(brand, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(brand, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(brand, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(brand, 6, 0);
    lv_obj_set_style_pad_top(brand, 2, 0);
    lv_obj_set_style_pad_left(brand, 4, 0);
    lv_obj_set_style_pad_bottom(brand, 0, 0);
    lv_obj_clear_flag(brand, LV_OBJ_FLAG_SCROLLABLE);

    /* brand-badge: 32×32 r12 三段渐变 */
    lv_obj_t *badge = lv_obj_create(brand);
    lv_obj_remove_style_all(badge);
    lv_obj_set_size(badge, 32, 32);
    lv_obj_set_style_radius(badge, 12, 0);
    lv_obj_set_style_clip_corner(badge, true, 0);
    am_fill_grad3(badge, t->hero_a, t->hero_b, t->hero_c);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *brand_icon = am_text(badge, AM_ICON_BRAND, m->f_metric, AM_WHITE);
    lv_obj_center(brand_icon);

    /* brand-copy: 两行文字列,宽度自适应 sidebar 剩余空间 */
    lv_obj_t *brand_copy = lv_obj_create(brand);
    lv_obj_remove_style_all(brand_copy);
    lv_obj_set_size(brand_copy, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(brand_copy, 1);
    lv_obj_set_flex_flow(brand_copy, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(brand_copy, 2, 0);
    lv_obj_clear_flag(brand_copy, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *copy_strong = am_text(brand_copy, "Broadcast First", m->f_label, AM_TEXT);
    lv_label_set_long_mode(copy_strong, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(copy_strong, LV_PCT(100));
    lv_obj_t *brand_sub = am_text(brand_copy, "Apple Music mockup", m->f_label, AM_MUTED);
    lv_label_set_long_mode(brand_sub, LV_LABEL_LONG_DOT);
    lv_obj_set_width(brand_sub, LV_PCT(100));

    /* ---- 2. "导航" 小标题 ---- */
    am_section_title(sidebar, "导航");

    /* ---- 3. nav-list (0..3) ---- */
    lv_obj_t *navlist = lv_obj_create(sidebar);
    lv_obj_remove_style_all(navlist);
    lv_obj_set_size(navlist, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(navlist, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(navlist, 6, 0);
    lv_obj_clear_flag(navlist, LV_OBJ_FLAG_SCROLLABLE);

    for(int i = 0; i < 4; i++){
        lv_obj_t *item = am_nav_item(navlist, am_nav_items[i].icon, am_nav_items[i].label, i == active_nav);
        lv_obj_set_user_data(item, (void *)(intptr_t)i);
        lv_obj_add_event_cb(item, nav_click_cb, LV_EVENT_CLICKED, ctx);
    }

    /* ---- 4. Spacer (flex-grow 1) ---- */
    lv_obj_t *spacer = lv_obj_create(sidebar);
    lv_obj_remove_style_all(spacer);
    lv_obj_set_size(spacer, 0, 0);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_clear_flag(spacer, LV_OBJ_FLAG_SCROLLABLE);

    /* ---- 5. footer ---- */
    lv_obj_t *footer = lv_obj_create(sidebar);
    lv_obj_remove_style_all(footer);
    lv_obj_set_size(footer, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(footer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(footer, 4, 0);
    lv_obj_set_style_pad_top(footer, 6, 0);
    lv_obj_set_style_pad_hor(footer, 8, 0);
    lv_obj_set_style_border_side(footer, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_width(footer, 1, 0);
    lv_obj_set_style_border_color(footer, AM_WHITE, 0);
    lv_obj_set_style_border_opa(footer, (lv_opa_t)(0.58f * 255), 0);
    lv_obj_clear_flag(footer, LV_OBJ_FLAG_SCROLLABLE);

    /* "偏好" 小标题 */
    am_section_title(footer, "偏好");

    /* 设置 nav-item (index 4) */
    lv_obj_t *settings_item = am_nav_item(footer, am_nav_items[4].icon, am_nav_items[4].label, active_nav == 4);
    lv_obj_set_user_data(settings_item, (void *)(intptr_t)4);
    lv_obj_add_event_cb(settings_item, nav_click_cb, LV_EVENT_CLICKED, ctx);

    /* listener-card */
    lv_obj_t *card = am_card(footer, 18, (lv_opa_t)(0.54f * 255));
    lv_obj_set_size(card, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(card, 8, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(card, 4, 0);
    /* shadow-soft */
    lv_obj_set_style_shadow_width(card, 30, 0);
    lv_obj_set_style_shadow_offset_y(card, 12, 0);
    lv_obj_set_style_shadow_color(card, lv_color_hex(0x363446), 0);
    lv_obj_set_style_shadow_opa(card, 26, 0);

    am_text(card, "正在收听", m->f_label, AM_MUTED);

    lv_obj_t *card_title = am_text(card, "今天先从广播开始，稍后再回到本地资料库。", m->f_body, AM_TEXT);
    lv_label_set_long_mode(card_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(card_title, LV_PCT(100));

    lv_obj_t *card_sub = am_text(card, "当前样机只验证 UI 层次、主题联动和页面边界。", m->f_label, AM_MUTED_STRONG);
    lv_label_set_long_mode(card_sub, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(card_sub, LV_PCT(100));
}

/* ------------------------------------------------------------------ */
/*  迷你播放条内部辅助                                                */
/* ------------------------------------------------------------------ */

/* 控制按钮:圆形,size×size,白底 bg_opa;图标居中 */
static lv_obj_t *ctrl_btn(lv_obj_t *parent, int size, lv_opa_t bg_opa,
                           const lv_font_t *font, const char *glyph, lv_color_t icon_color,
                           bool use_grad3, lv_color_t ga, lv_color_t gb, lv_color_t gc)
{
    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, size, size);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(btn, true, 0);
    if(use_grad3){
        am_fill_grad3(btn, ga, gb, gc);
    } else {
        lv_obj_set_style_bg_color(btn, AM_WHITE, 0);
        lv_obj_set_style_bg_opa(btn, bg_opa, 0);
    }
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *lbl = am_text(btn, glyph, font, icon_color);
    lv_obj_center(lbl);
    return btn;
}


/* 迷你播放条 grid 描述符(lv_obj_set_grid_dsc_array 仅存指针,必须文件级静态)
 * 注意: LV_GRID_FR(100) == LV_GRID_TEMPLATE_LAST(均为 LV_COORD_MAX),会被解析为数组终止符!
 * 因此使用互质的小整数比例: FR(23):FR(20) ≈ 115:100 */
static int32_t s_mcols[] = {LV_GRID_FR(23), LV_GRID_CONTENT, LV_GRID_FR(20), LV_GRID_TEMPLATE_LAST};
static int32_t s_mrows[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

/* ------------------------------------------------------------------ */
/*  迷你播放条构建                                                    */
/* ------------------------------------------------------------------ */
am_miniplayer_handles_t am_shell_build_miniplayer(lv_obj_t *player)
{
    am_miniplayer_handles_t h;
    memset(&h, 0, sizeof(h));
    const am_metrics_t *m = am_metrics();
    const am_theme_t   *t = am_theme_get(am_theme_current());

    /* player_h 驱动的尺寸:art、控制按钮随 tier 缩放 */
    int art_size   = m->player_h - 24;  /* 800→54, 640→54, 480→32 */
    int btn_play   = m->player_h - 36;  /* 800→42, 640→42, 480→20 */
    int btn_skip   = btn_play - 8;      /* 800→34, 640→34, 480→12 */
    if(art_size  < 24) art_size  = 24;
    if(btn_play  < 20) btn_play  = 20;
    if(btn_skip  < 16) btn_skip  = 16;
    int art_radius = art_size / 3;

    /* vert padding:留一点上下呼吸,最少 2px */
    int vpad = (m->player_h - art_size) / 2;
    if(vpad < 2) vpad = 2;
    int hpad = m->panel_pad - 2;
    if(hpad < 4) hpad = 4;

    /* player 自身背景:近白 + 顶边线 */
    lv_obj_set_style_bg_color(player, lv_color_hex(0xf9fafc), 0);
    lv_obj_set_style_bg_opa(player, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(player, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_width(player, 1, 0);
    lv_obj_set_style_border_color(player, AM_WHITE, 0);
    lv_obj_set_style_border_opa(player, (lv_opa_t)(0.74f * 255), 0);
    lv_obj_set_style_pad_ver(player, vpad, 0);
    lv_obj_set_style_pad_hor(player, hpad, 0);
    lv_obj_set_style_pad_column(player, m->page_gap, 0);
    lv_obj_set_scrollbar_mode(player, LV_SCROLLBAR_MODE_OFF);

    /* FR grid: col0=FR(115) now-playing, col1=CONTENT ctrls, col2=FR(100) progress */
    lv_obj_set_grid_dsc_array(player, s_mcols, s_mrows);

    /* ---- col0: now-playing (STRETCH 宽由 FR(115) 决定) ---- */
    lv_obj_t *now = lv_obj_create(player);
    lv_obj_remove_style_all(now);
    lv_obj_set_grid_cell(now, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_flex_flow(now, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(now, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(now, 8, 0);
    lv_obj_clear_flag(now, LV_OBJ_FLAG_SCROLLABLE);

    /* player-art: scaled by player_h */
    lv_obj_t *art = lv_obj_create(now);
    lv_obj_remove_style_all(art);
    lv_obj_set_size(art, art_size, art_size);
    lv_obj_set_style_radius(art, art_radius, 0);
    lv_obj_set_style_clip_corner(art, true, 0);
    am_fill_grad3(art, t->hero_a, t->hero_b, t->hero_c);
    lv_obj_clear_flag(art, LV_OBJ_FLAG_SCROLLABLE);
    /* 装饰白圆 (只在 art 够大时显示) */
    if(art_size >= 36){
        lv_obj_t *art_deco = lv_obj_create(art);
        lv_obj_remove_style_all(art_deco);
        int deco = art_size / 2 - 2;
        lv_obj_set_size(art_deco, deco, deco);
        lv_obj_set_style_radius(art_deco, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(art_deco, AM_WHITE, 0);
        lv_obj_set_style_bg_opa(art_deco, (lv_opa_t)(0.32f * 255), 0);
        lv_obj_clear_flag(art_deco, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(art_deco, LV_ALIGN_BOTTOM_RIGHT, 6, 4);
    }

    /* player-copy: flex-grow 填满 col0 剩余 */
    lv_obj_t *copy = lv_obj_create(now);
    lv_obj_remove_style_all(copy);
    lv_obj_set_size(copy, 0, LV_SIZE_CONTENT);  /* width=0 + flex_grow=1 */
    lv_obj_set_flex_grow(copy, 1);
    lv_obj_set_flex_flow(copy, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(copy, 3, 0);
    lv_obj_clear_flag(copy, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *copy_title = am_text(copy, am_mini.title, m->f_body, AM_TEXT);
    lv_label_set_long_mode(copy_title, LV_LABEL_LONG_DOT);
    lv_obj_set_width(copy_title, LV_PCT(100));
    h.title_label = copy_title;

    lv_obj_t *copy_sub = am_text(copy, am_mini.subtitle, m->f_label, AM_MUTED);
    lv_label_set_long_mode(copy_sub, LV_LABEL_LONG_DOT);
    lv_obj_set_width(copy_sub, LV_PCT(100));
    h.subtitle_label = copy_sub;

    /* ---- col1: player-controls (CONTENT 宽, CENTER 对齐) ---- */
    lv_obj_t *ctrls = lv_obj_create(player);
    lv_obj_remove_style_all(ctrls);
    lv_obj_set_grid_cell(ctrls, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_size(ctrls, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(ctrls, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctrls, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(ctrls, 8, 0);
    lv_obj_clear_flag(ctrls, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *prev_btn = ctrl_btn(ctrls, btn_skip, 184, m->f_icon, AM_ICON_PREV, AM_MUTED_STRONG,
                                   false, t->hero_a, t->hero_b, t->hero_c);
    lv_obj_add_event_cb(prev_btn, am_player_on_prev, LV_EVENT_CLICKED, NULL);

    lv_obj_t *play_btn = ctrl_btn(ctrls, btn_play, LV_OPA_COVER, m->f_icon, AM_ICON_PLAY, AM_WHITE,
                                   true, t->hero_a, t->hero_b, t->hero_c);
    lv_obj_add_event_cb(play_btn, am_player_on_play_pause, LV_EVENT_CLICKED, NULL);
    h.play_icon = lv_obj_get_child(play_btn, 0);

    lv_obj_t *next_btn = ctrl_btn(ctrls, btn_skip, 184, m->f_icon, AM_ICON_NEXT, AM_MUTED_STRONG,
                                   false, t->hero_a, t->hero_b, t->hero_c);
    lv_obj_add_event_cb(next_btn, am_player_on_next, LV_EVENT_CLICKED, NULL);

    /* ---- col2: progress-cluster (STRETCH 宽由 FR(20) 决定) ---- */
    lv_obj_t *cluster = lv_obj_create(player);
    lv_obj_remove_style_all(cluster);
    lv_obj_set_grid_cell(cluster, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    /* 不调用 lv_obj_set_size — STRETCH 已托管宽度 */
    lv_obj_set_flex_flow(cluster, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cluster, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cluster, 6, 0);
    lv_obj_clear_flag(cluster, LV_OBJ_FLAG_SCROLLABLE);

    /* progress-meta: flex row space-between,宽度 100% 跟随 cluster */
    lv_obj_t *meta = lv_obj_create(cluster);
    lv_obj_remove_style_all(meta);
    lv_obj_set_size(meta, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(meta, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(meta, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(meta, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_cur = am_text(meta, am_mini.current, m->f_label, AM_MUTED);
    h.time_cur = lbl_cur;
    am_text(meta, "Now Playing",   m->f_label, AM_MUTED);
    lv_obj_t *lbl_total = am_text(meta, am_mini.total, m->f_label, AM_MUTED);
    h.time_total = lbl_total;

    /* progress-bar rail:6px 高,宽度 100% 跟随 cluster */
    lv_obj_t *rail = lv_obj_create(cluster);
    lv_obj_remove_style_all(rail);
    lv_obj_set_size(rail, LV_PCT(100), 6);
    lv_obj_set_style_radius(rail, 999, 0);
    lv_obj_set_style_clip_corner(rail, true, 0);
    lv_obj_set_style_bg_color(rail, lv_color_hex(0x6d758c), 0);
    lv_obj_set_style_bg_opa(rail, 41, 0);
    lv_obj_clear_flag(rail, LV_OBJ_FLAG_SCROLLABLE);

    /* fill:LV_PCT(progress_pct) — 跟随 rail 宽度 */
    lv_obj_t *fill = lv_obj_create(rail);
    lv_obj_remove_style_all(fill);
    lv_obj_set_size(fill, LV_PCT(am_mini.progress_pct), 6);
    lv_obj_set_style_radius(fill, 999, 0);
    lv_obj_set_style_clip_corner(fill, true, 0);
    am_fill_grad2(fill, t->accent, t->accent_soft);
    lv_obj_clear_flag(fill, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(fill, LV_ALIGN_LEFT_MID, 0, 0);
    h.progress_fill = fill;

    return h;
}
