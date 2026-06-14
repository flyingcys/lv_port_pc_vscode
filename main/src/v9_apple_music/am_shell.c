/* main/src/v9_apple_music/am_shell.c */
#include "am_shell.h"
#include "am_widgets.h"
#include "am_theme.h"
#include "am_fonts.h"
#include "am_icons.h"
#include "am_data.h"
#include <stdint.h>

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
    lv_obj_set_style_pad_top(sidebar, 12, 0);
    lv_obj_set_style_pad_right(sidebar, 14, 0);
    lv_obj_set_style_pad_bottom(sidebar, 12, 0);
    lv_obj_set_style_pad_left(sidebar, 16, 0);
    lv_obj_set_flex_flow(sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(sidebar, 10, 0);
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
    lv_obj_t *brand_icon = am_text(badge, AM_ICON_BRAND, &am_font_18, AM_WHITE);
    lv_obj_center(brand_icon);

    /* brand-copy: 两行文字列 */
    lv_obj_t *brand_copy = lv_obj_create(brand);
    lv_obj_remove_style_all(brand_copy);
    lv_obj_set_size(brand_copy, 92, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(brand_copy, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(brand_copy, 2, 0);
    lv_obj_clear_flag(brand_copy, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *copy_strong = am_text(brand_copy, "Broadcast First", &am_font_11, AM_TEXT);
    lv_label_set_long_mode(copy_strong, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(copy_strong, 92);
    /* 副标题可换行;宽度 = sidebar(164) - pad_l(16) - badge(32) - gap(6) - pad_r(14) - brand_pad_left(4) = 92px */
    lv_obj_t *brand_sub = am_text(brand_copy, "Apple Music mockup", &am_font_11, AM_MUTED);
    lv_label_set_long_mode(brand_sub, LV_LABEL_LONG_DOT);
    lv_obj_set_width(brand_sub, 92);

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

    am_text(card, "正在收听", &am_font_11, AM_MUTED);

    lv_obj_t *card_title = am_text(card, "今天先从广播开始，稍后再回到本地资料库。", &am_font_12, AM_TEXT);
    lv_label_set_long_mode(card_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(card_title, LV_PCT(100));

    lv_obj_t *card_sub = am_text(card, "当前样机只验证 UI 层次、主题联动和页面边界。", &am_font_11, AM_MUTED_STRONG);
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


/* ------------------------------------------------------------------ */
/*  迷你播放条构建                                                    */
/* ------------------------------------------------------------------ */
void am_shell_build_miniplayer(lv_obj_t *player)
{
    const am_theme_t *t = am_theme_get(am_theme_current());

    /* player 自身背景:近白 + 顶边线 */
    lv_obj_set_style_bg_color(player, lv_color_hex(0xf9fafc), 0);
    lv_obj_set_style_bg_opa(player, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(player, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_width(player, 1, 0);
    lv_obj_set_style_border_color(player, AM_WHITE, 0);
    lv_obj_set_style_border_opa(player, (lv_opa_t)(0.74f * 255), 0);
    lv_obj_set_style_pad_top(player, 12, 0);
    lv_obj_set_style_pad_right(player, 20, 0);
    lv_obj_set_style_pad_bottom(player, 14, 0);
    lv_obj_set_style_pad_left(player, 14, 0);
    lv_obj_set_scrollbar_mode(player, LV_SCROLLBAR_MODE_OFF);

    /* 用 flex row 实现 3 列布局:
     * player 内宽 = 636-14-20=602, ctrls=130, gap=16×2=32, rest=440
     * now = ceil(440×115/215)=235, cluster = 440-235=205 */
    lv_obj_set_flex_flow(player, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(player, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(player, 16, 0);

    /* ---- cell 0: now-playing (固定宽度 235px) ---- */
    lv_obj_t *now = lv_obj_create(player);
    lv_obj_remove_style_all(now);
    lv_obj_set_size(now, 235, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(now, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(now, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(now, 12, 0);
    lv_obj_clear_flag(now, LV_OBJ_FLAG_SCROLLABLE);

    /* player-art: 52×52 r18 三段渐变 + 装饰白圆 */
    lv_obj_t *art = lv_obj_create(now);
    lv_obj_remove_style_all(art);
    lv_obj_set_size(art, 52, 52);
    lv_obj_set_style_radius(art, 18, 0);
    lv_obj_set_style_clip_corner(art, true, 0);
    am_fill_grad3(art, t->hero_a, t->hero_b, t->hero_c);
    lv_obj_clear_flag(art, LV_OBJ_FLAG_SCROLLABLE);
    /* ::after 装饰白圆 */
    lv_obj_t *art_deco = lv_obj_create(art);
    lv_obj_remove_style_all(art_deco);
    lv_obj_set_size(art_deco, 28, 28);
    lv_obj_set_style_radius(art_deco, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(art_deco, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(art_deco, (lv_opa_t)(0.32f * 255), 0);
    lv_obj_clear_flag(art_deco, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(art_deco, LV_ALIGN_BOTTOM_RIGHT, 6, 4);

    /* player-copy: flex col,固定宽度 = now(235) - art(52) - gap(12) = 171px */
    lv_obj_t *copy = lv_obj_create(now);
    lv_obj_remove_style_all(copy);
    lv_obj_set_size(copy, 171, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(copy, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(copy, 4, 0);
    lv_obj_clear_flag(copy, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *copy_title = am_text(copy, am_mini.title, &am_font_13, AM_TEXT);
    lv_label_set_long_mode(copy_title, LV_LABEL_LONG_DOT);
    lv_obj_set_width(copy_title, 171);

    lv_obj_t *copy_sub = am_text(copy, am_mini.subtitle, &am_font_11, AM_MUTED);
    lv_label_set_long_mode(copy_sub, LV_LABEL_LONG_DOT);
    lv_obj_set_width(copy_sub, 171);

    /* ---- cell 1: player-controls ---- */
    lv_obj_t *ctrls = lv_obj_create(player);
    lv_obj_remove_style_all(ctrls);
    lv_obj_set_size(ctrls, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(ctrls, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctrls, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(ctrls, 10, 0);
    lv_obj_clear_flag(ctrls, LV_OBJ_FLAG_SCROLLABLE);

    /* ⏮ prev: 34×34, white@184 */
    ctrl_btn(ctrls, 34, 184, &am_font_14, AM_ICON_PREV, AM_MUTED_STRONG,
             false, t->hero_a, t->hero_b, t->hero_c);
    /* ▶ play: 42×42, hero gradient */
    ctrl_btn(ctrls, 42, LV_OPA_COVER, &am_font_16, AM_ICON_PLAY, AM_WHITE,
             true, t->hero_a, t->hero_b, t->hero_c);
    /* ⏭ next: 34×34, white@184 */
    ctrl_btn(ctrls, 34, 184, &am_font_14, AM_ICON_NEXT, AM_MUTED_STRONG,
             false, t->hero_a, t->hero_b, t->hero_c);

    /* ---- cell 2: progress-cluster (固定宽度 205px) ---- */
    lv_obj_t *cluster = lv_obj_create(player);
    lv_obj_remove_style_all(cluster);
    lv_obj_set_size(cluster, 205, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cluster, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cluster, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cluster, 8, 0);
    lv_obj_clear_flag(cluster, LV_OBJ_FLAG_SCROLLABLE);

    /* progress-meta: flex row space-between;固定宽 205px */
    lv_obj_t *meta = lv_obj_create(cluster);
    lv_obj_remove_style_all(meta);
    lv_obj_set_size(meta, 205, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(meta, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(meta, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(meta, LV_OBJ_FLAG_SCROLLABLE);

    am_text(meta, am_mini.current, &am_font_11, AM_MUTED);
    am_text(meta, "Now Playing",   &am_font_11, AM_MUTED);
    am_text(meta, am_mini.total,   &am_font_11, AM_MUTED);

    /* progress-bar rail:6px 高,固定宽 205px,accent@16% 底色 */
    lv_obj_t *rail = lv_obj_create(cluster);
    lv_obj_remove_style_all(rail);
    lv_obj_set_size(rail, 205, 6);
    lv_obj_set_style_radius(rail, 999, 0);
    lv_obj_set_style_clip_corner(rail, true, 0);
    /* rgba(109,117,140,0.16) ≈ bg_opa=41 */
    lv_obj_set_style_bg_color(rail, lv_color_hex(0x6d758c), 0);
    lv_obj_set_style_bg_opa(rail, 41, 0);
    lv_obj_clear_flag(rail, LV_OBJ_FLAG_SCROLLABLE);

    /* fill:accent 渐变,44% × 205 = 90px */
    lv_obj_t *fill = lv_obj_create(rail);
    lv_obj_remove_style_all(fill);
    lv_obj_set_size(fill, (205 * am_mini.progress_pct + 50) / 100, 6);
    lv_obj_set_style_radius(fill, 999, 0);
    lv_obj_set_style_clip_corner(fill, true, 0);
    am_fill_grad2(fill, t->accent, t->accent_soft);
    lv_obj_clear_flag(fill, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(fill, LV_ALIGN_LEFT_MID, 0, 0);
}
