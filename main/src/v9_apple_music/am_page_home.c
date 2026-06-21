/* main/src/v9_apple_music/am_page_home.c
 * Apple Music 主页内容区 —— 对应 HTML renderHomePage()
 * 布局:flex 列 gap16 → page-header(space-between) + home-grid(2 列)
 *   home-grid 左:hero-card(渐变 + 绝对装饰,顶部对齐,自然高度)
 *   home-grid 右:side-stack(flex 列 gap14:recommend-panel + recent-panel,自然高度)
 * page 自身为 LV_SIZE_CONTENT 高度,由父容器负责滚动。
 */
#include "am_page_home.h"
#include "am_widgets.h"
#include "am_theme.h"
#include "am_fonts.h"
#include "am_icons.h"
#include "am_data.h"
#include "am_metrics.h"
#include <stdio.h>

/* ── 静态 helper:媒体行(cover + 标题/副标 + action 图标) ────────────── */
static lv_obj_t *build_media_row(lv_obj_t *parent, const char *title,
                                  const char *subtitle, const char *action_glyph)
{
    const am_theme_t   *t = am_theme_get(am_theme_current());
    const am_metrics_t *m = am_metrics();

    /* 行容器:白 46%,r18,pad 10/12,flex row gap12 align center */
    lv_obj_t *row = am_card(parent, m->card_radius, 117);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_hor(row, 12, 0);
    lv_obj_set_style_pad_ver(row, 10, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 12, 0);

    /* 封面 cover×cover (metrics-exact) */
    am_cover(row, m->cover, t->tile_a, t->tile_c);

    /* 标题/副标列 flex-grow
     * LVGL LV_LABEL_LONG_DOT triggers when text height > label height.
     * We must clamp each label to exactly ONE line height so DOT activates.
     * col itself can be SIZE_CONTENT height (sum of two clamped labels). */
    lv_obj_t *col = lv_obj_create(row);
    lv_obj_remove_style_all(col);
    lv_obj_set_size(col, 0, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(col, 1);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(col, 4, 0);
    lv_obj_set_style_pad_all(col, 0, 0);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);

    /* title: ONE line — fixed height = body line height; DOT truncates overflow */
    int32_t title_h = (int32_t)lv_font_get_line_height(m->f_body);
    lv_obj_t *t_lbl = am_text(col, title, m->f_body, AM_TEXT);
    lv_label_set_long_mode(t_lbl, LV_LABEL_LONG_DOT);
    lv_obj_set_size(t_lbl, LV_PCT(100), title_h);

    /* subtitle: ONE line — fixed height = label line height; DOT truncates */
    int32_t sub_h = (int32_t)lv_font_get_line_height(m->f_label);
    lv_obj_t *s_lbl = am_text(col, subtitle, m->f_label, AM_MUTED);
    lv_label_set_long_mode(s_lbl, LV_LABEL_LONG_DOT);
    lv_obj_set_size(s_lbl, LV_PCT(100), sub_h);

    /* 动作按钮 */
    am_item_action(row, action_glyph);

    return row;
}

/* ── page-header ─────────────────────────────────────────────────────── */
static lv_obj_t *build_page_header(lv_obj_t *parent)
{
    const am_theme_t   *t = am_theme_get(am_theme_current());
    const am_metrics_t *m = am_metrics();

    lv_obj_t *hdr = lv_obj_create(parent);
    lv_obj_remove_style_all(hdr);
    lv_obj_set_width(hdr, LV_PCT(100));
    lv_obj_set_height(hdr, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_SPACE_BETWEEN,
                           LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(hdr, m->panel_pad, 0);
    lv_obj_clear_flag(hdr, LV_OBJ_FLAG_SCROLLABLE);

    /* ─ 左侧列 ─ */
    lv_obj_t *left = lv_obj_create(hdr);
    lv_obj_remove_style_all(left);
    lv_obj_set_flex_grow(left, 1);
    lv_obj_set_height(left, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(left, 4, 0);
    lv_obj_clear_flag(left, LV_OBJ_FLAG_SCROLLABLE);

    /* eyebrow 胶囊:r999, 白 56%, pad 6/10, flex row gap8 */
    lv_obj_t *eyebrow = lv_obj_create(left);
    lv_obj_remove_style_all(eyebrow);
    lv_obj_set_size(eyebrow, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(eyebrow, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(eyebrow, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(eyebrow, 143, 0);   /* 0.56*255 */
    lv_obj_set_style_pad_hor(eyebrow, 10, 0);
    lv_obj_set_style_pad_ver(eyebrow, 6, 0);
    lv_obj_set_flex_flow(eyebrow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(eyebrow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(eyebrow, 8, 0);
    lv_obj_clear_flag(eyebrow, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    /* 7px accent 点 */
    lv_obj_t *dot = lv_obj_create(eyebrow);
    lv_obj_remove_style_all(dot);
    lv_obj_set_size(dot, 7, 7);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dot, t->accent, 0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    am_text(eyebrow, "广播优先首页", m->f_label, AM_MUTED_STRONG);

    /* 主页大标题 */
    lv_obj_t *title = am_text(left, "主页", m->f_title, AM_TEXT);
    lv_obj_set_style_pad_top(title, 2, 0);

    /* 副标题(换行) — max-width leaves room for search chip */
    lv_obj_t *sub = am_text(left,
        "早上好，现在适合先听一会广播，再从最近播放回到你的收藏。",
        m->f_body, AM_MUTED);
    lv_label_set_long_mode(sub, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_max_width(sub, LV_PCT(100), 0);

    /* ─ 右侧 search-chip ─ */
    lv_obj_t *chip = lv_obj_create(hdr);
    lv_obj_remove_style_all(chip);
    lv_obj_set_size(chip, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(chip, 18, 0);
    lv_obj_set_style_bg_color(chip, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(chip, 159, 0);   /* 0.62*255 */
    lv_obj_set_style_pad_hor(chip, 14, 0);
    lv_obj_set_style_pad_ver(chip, 10, 0);
    lv_obj_set_flex_flow(chip, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(chip, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(chip, 10, 0);
    lv_obj_clear_flag(chip, LV_OBJ_FLAG_SCROLLABLE);

    am_text(chip, AM_ICON_SEARCH, m->f_label, AM_MUTED);
    am_text(chip, "搜索占位，本轮不接真实功能", m->f_label, AM_MUTED);

    return hdr;
}

/* ── hero-card ───────────────────────────────────────────────────────── */
static lv_obj_t *build_hero_card(lv_obj_t *parent)
{
    const am_theme_t   *t = am_theme_get(am_theme_current());
    const am_metrics_t *m = am_metrics();

    /* hero min_height scales with tier:
     * 800 (sidebar_w=164)→290, 640 (sidebar_w=140)→220, 480 (sidebar_w=108)→160 */
    int16_t hero_min_h;
    if (m->sidebar_w >= 150)       hero_min_h = 218;   /* AM_TIER_800 — matches mockup min-height:218px */
    else if (m->sidebar_w >= 120)  hero_min_h = 210;   /* AM_TIER_640 */
    else                           hero_min_h = 150;   /* AM_TIER_480 */

    /* 外框:r28, 三段渐变, 阴影;自然高度 */
    lv_obj_t *hero = lv_obj_create(parent);
    lv_obj_remove_style_all(hero);
    lv_obj_set_width(hero, LV_PCT(100));
    lv_obj_set_height(hero, hero_min_h);   /* 固定高度:与 mockup 紧凑 hero 一致,不被内容/grid 撑高 */
    lv_obj_set_style_radius(hero, 28, 0);
    am_fill_grad3(hero, t->hero_a, t->hero_b, t->hero_c);
    lv_obj_set_style_shadow_width(hero, 44, 0);
    lv_obj_set_style_shadow_offset_y(hero, 24, 0);
    lv_obj_set_style_shadow_color(hero, t->accent, 0);
    lv_obj_set_style_shadow_opa(hero, 61, 0);   /* 0.24*255 */
    lv_obj_set_style_pad_left(hero, 24, 0);
    lv_obj_set_style_pad_right(hero, 24, 0);
    lv_obj_set_style_pad_top(hero, 18, 0);
    lv_obj_set_style_pad_bottom(hero, 18, 0);
    lv_obj_set_scrollbar_mode(hero, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(hero, LV_OBJ_FLAG_SCROLLABLE);

    /* hero-copy:flex 列,顶部对齐 */
    lv_obj_t *copy = lv_obj_create(hero);
    lv_obj_remove_style_all(copy);
    lv_obj_set_size(copy, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    /* Limit copy max-width so pill at top-right doesn't collide with title.
     * At 640/480 the hero column is narrower, so we allow LV_PCT(60) max. */
    lv_obj_set_style_max_width(copy, LV_PCT(62), 0);
    lv_obj_set_flex_flow(copy, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(copy, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(copy, 0, 0);
    lv_obj_clear_flag(copy, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(copy, LV_ALIGN_TOP_LEFT, 0, 0);

    /* topline */
    lv_obj_t *topline = am_text(copy, am_home_hero.topline, m->f_label, AM_WHITE);
    lv_obj_set_style_opa(topline, 214, 0);   /* 0.84*255 */

    /* hero title — uses h2 role */
    lv_obj_t *htitle = am_text(copy, am_home_hero.title, m->f_h2, AM_WHITE);
    lv_label_set_long_mode(htitle, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(htitle, LV_PCT(100));
    lv_obj_set_style_pad_top(htitle, 10, 0);
    lv_obj_set_style_pad_bottom(htitle, 8, 0);

    /* hero desc — uses body role */
    lv_obj_t *hdesc = am_text(copy, am_home_hero.desc, m->f_label, AM_WHITE);
    lv_obj_set_style_opa(hdesc, 214, 0);   /* 0.84*255 */
    lv_label_set_long_mode(hdesc, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(hdesc, LV_PCT(100));

    /* ── hero-actions 绝对定位,左下 ── */
    lv_obj_t *actions = lv_obj_create(hero);
    lv_obj_remove_style_all(actions);
    lv_obj_set_size(actions, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(actions, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(actions, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(actions, 10, 0);
    lv_obj_clear_flag(actions, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(actions, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    /* 继续收听 button-primary:r999,白 92%,AM_TEXT */
    lv_obj_t *btn_p = lv_obj_create(actions);
    lv_obj_remove_style_all(btn_p);
    lv_obj_set_size(btn_p, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(btn_p, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_p, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(btn_p, 235, 0);   /* 0.92*255 */
    lv_obj_set_style_pad_hor(btn_p, 16, 0);
    lv_obj_set_style_pad_ver(btn_p, 10, 0);
    lv_obj_clear_flag(btn_p, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *lbl_p = am_text(btn_p, "继续收听", m->f_label, AM_TEXT);
    lv_obj_center(lbl_p);

    /* 探索更多 button-secondary:r999,白 16%,AM_WHITE */
    lv_obj_t *btn_s = lv_obj_create(actions);
    lv_obj_remove_style_all(btn_s);
    lv_obj_set_size(btn_s, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(btn_s, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_s, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(btn_s, 41, 0);    /* 0.16*255 */
    lv_obj_set_style_pad_hor(btn_s, 16, 0);
    lv_obj_set_style_pad_ver(btn_s, 10, 0);
    lv_obj_clear_flag(btn_s, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *lbl_s = am_text(btn_s, "探索更多", m->f_label, AM_WHITE);
    lv_obj_center(lbl_s);

    /* ── floating-pill 绝对定位,右上,top inset=14 to clear title area ── */
    lv_obj_t *pill = lv_obj_create(hero);
    lv_obj_remove_style_all(pill);
    lv_obj_set_size(pill, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(pill, 16, 0);
    lv_obj_set_style_bg_color(pill, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(pill, 41, 0);    /* ~0.16 */
    lv_obj_set_style_pad_hor(pill, 11, 0);
    lv_obj_set_style_pad_ver(pill, 7, 0);
    lv_obj_clear_flag(pill, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *pill_lbl = am_text(pill, am_home_hero.badge, m->f_label,
                                  lv_color_hex(0xf0f0f0));
    lv_obj_center(pill_lbl);
    /* Anchor top-right with top inset=14 — pill width is ~66px,
     * copy is constrained to 62% so they don't overlap horizontally */
    lv_obj_align(pill, LV_ALIGN_TOP_RIGHT, 0, 14);

    /* ── hero-visual:vinyl 装饰,绝对,右下 ── */
    /* 外容器 144×144 */
    lv_obj_t *visual = lv_obj_create(hero);
    lv_obj_remove_style_all(visual);
    lv_obj_set_size(visual, 144, 144);
    lv_obj_set_style_opa(visual, 204, 0);   /* 0.80*255 */
    lv_obj_clear_flag(visual, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(visual, LV_ALIGN_BOTTOM_RIGHT, 6, 18);

    /* vinyl-ring:inset 18 → size=144-36=108,r_circle,border 16px 白 16% */
    lv_obj_t *ring = lv_obj_create(visual);
    lv_obj_remove_style_all(ring);
    lv_obj_set_size(ring, 108, 108);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ring, 16, 0);
    lv_obj_set_style_border_color(ring, AM_WHITE, 0);
    lv_obj_set_style_border_opa(ring, 41, 0);   /* 0.16*255 */
    lv_obj_clear_flag(ring, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(ring, LV_ALIGN_CENTER, 0, 0);

    /* vinyl-core:inset 50 → size=144-100=44,r_circle,白 86% */
    lv_obj_t *core = lv_obj_create(visual);
    lv_obj_remove_style_all(core);
    lv_obj_set_size(core, 44, 44);
    lv_obj_set_style_radius(core, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(core, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(core, 219, 0);   /* 0.86*255 */
    lv_obj_clear_flag(core, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(core, LV_ALIGN_CENTER, 0, 0);

    return hero;
}

/* ── recommend-panel ─────────────────────────────────────────────────── */
static lv_obj_t *build_recommend_panel(lv_obj_t *parent)
{
    const am_theme_t   *t = am_theme_get(am_theme_current());
    const am_metrics_t *m = am_metrics();

    lv_obj_t *panel = am_panel(parent);
    lv_obj_set_width(panel, LV_PCT(100));
    lv_obj_set_height(panel, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(panel, m->panel_pad, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(panel, 0, 0);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    /* panel-header:row,space-between */
    lv_obj_t *header = lv_obj_create(panel);
    lv_obj_remove_style_all(header);
    lv_obj_set_width(header, LV_PCT(100));
    lv_obj_set_height(header, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN,
                           LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_bottom(header, 10, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    am_text(header, "推荐广播", m->f_strong, AM_TEXT);
    am_text(header, "查看全部", m->f_label, t->accent);

    /* 推荐列表:flex 列 gap6 */
    lv_obj_t *list = lv_obj_create(panel);
    lv_obj_remove_style_all(list);
    lv_obj_set_width(list, LV_PCT(100));
    lv_obj_set_height(list, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(list, 6, 0);
    lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLLABLE);

    for(int i = 0; i < 3; i++){
        build_media_row(list,
                        am_home_recommends[i].title,
                        am_home_recommends[i].subtitle,
                        AM_ICON_PLAY);
    }

    return panel;
}

/* ── recent-panel ────────────────────────────────────────────────────── */
static lv_obj_t *build_recent_panel(lv_obj_t *parent)
{
    const am_theme_t   *t = am_theme_get(am_theme_current());
    const am_metrics_t *m = am_metrics();

    lv_obj_t *panel = am_panel(parent);
    lv_obj_set_width(panel, LV_PCT(100));
    lv_obj_set_height(panel, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(panel, m->panel_pad, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(panel, 0, 0);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    /* panel-header */
    lv_obj_t *header = lv_obj_create(panel);
    lv_obj_remove_style_all(header);
    lv_obj_set_width(header, LV_PCT(100));
    lv_obj_set_height(header, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN,
                           LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_bottom(header, 10, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    am_text(header, "最近播放", m->f_strong, AM_TEXT);
    am_text(header, "继续", m->f_label, t->accent);

    /* 最近列表:flex 列 gap6 */
    lv_obj_t *list = lv_obj_create(panel);
    lv_obj_remove_style_all(list);
    lv_obj_set_width(list, LV_PCT(100));
    lv_obj_set_height(list, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(list, 6, 0);
    lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLLABLE);

    for(int i = 0; i < 3; i++){
        /* 副标题格式:"<kind> · <subtitle>" */
        char sub_buf[80];
        snprintf(sub_buf, sizeof(sub_buf), "%s · %s",
                 am_home_recent[i].kind, am_home_recent[i].subtitle);
        build_media_row(list,
                        am_home_recent[i].title,
                        sub_buf,
                        AM_ICON_REPLAY);
    }

    /* recent-metrics:3 列 grid,gap 10,mt 12 */
    /* 使用 LVGL grid 3 等宽列 */
    static const int32_t mcols[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
                                    LV_GRID_TEMPLATE_LAST};
    static const int32_t mrows[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *metrics_grid = lv_obj_create(panel);
    lv_obj_remove_style_all(metrics_grid);
    lv_obj_set_width(metrics_grid, LV_PCT(100));
    lv_obj_set_height(metrics_grid, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_top(metrics_grid, 8, 0);
    lv_obj_set_grid_dsc_array(metrics_grid, mcols, mrows);
    lv_obj_set_style_pad_column(metrics_grid, 8, 0);
    lv_obj_set_style_pad_row(metrics_grid, 0, 0);
    lv_obj_clear_flag(metrics_grid, LV_OBJ_FLAG_SCROLLABLE);

    for(int i = 0; i < 3; i++){
        /* metric 卡:r16,白 54%,pad 8 */
        lv_obj_t *card = am_card(metrics_grid, 16, 138);
        lv_obj_set_style_pad_all(card, 8, 0);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                               LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(card, 4, 0);
        lv_obj_set_grid_cell(card, LV_GRID_ALIGN_STRETCH, i, 1,
                             LV_GRID_ALIGN_START, 0, 1);

        lv_obj_t *lbl  = am_text(card, am_home_metrics[i].label, m->f_label, AM_MUTED);
        lv_obj_set_style_text_letter_space(lbl, 1, 0);  /* letter-spacing 近似 */

        am_text(card, am_home_metrics[i].value, m->f_metric, AM_TEXT);
    }

    return panel;
}

/* ── 公开入口 ────────────────────────────────────────────────────────── */
lv_obj_t *am_page_home_create(lv_obj_t *content_parent)
{
    const am_metrics_t *m = am_metrics();

    /* page 根:flex 列 gap,宽 100%,高 LV_SIZE_CONTENT(自然高度,父容器负责滚动)
     * 无背景,无滚动条 */
    lv_obj_t *page = lv_obj_create(content_parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_width(page, LV_PCT(100));
    lv_obj_set_height(page, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(page, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(page, m->page_gap, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    /* 1. page-header */
    build_page_header(page);

    if (m->stack_content) {
        /* 480 单列模式:hero + 推荐面板 + 最近面板 垂直堆叠,全宽,可滚动 */
        lv_obj_t *col = lv_obj_create(page);
        lv_obj_remove_style_all(col);
        lv_obj_set_width(col, LV_PCT(100));
        lv_obj_set_height(col, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                               LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(col, m->page_gap, 0);
        lv_obj_set_style_pad_all(col, 0, 0);
        lv_obj_set_scrollbar_mode(col, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);

        build_hero_card(col);          /* hero — 全宽 */
        build_recommend_panel(col);    /* 推荐广播(3 items) — 全宽 */
        build_recent_panel(col);       /* 最近播放(3 items + 3 metrics) — 全宽 */
    } else {
        /* 800/640 双列模式:home-grid [FR125, FR92] */
        static const int32_t gcols[] = {LV_GRID_FR(125), LV_GRID_FR(92),
                                        LV_GRID_TEMPLATE_LAST};
        static const int32_t grows[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

        lv_obj_t *grid = lv_obj_create(page);
        lv_obj_remove_style_all(grid);
        lv_obj_set_width(grid, LV_PCT(100));
        lv_obj_set_height(grid, LV_SIZE_CONTENT);
        lv_obj_set_style_pad_column(grid, m->page_gap, 0);
        lv_obj_set_style_pad_row(grid, 0, 0);
        lv_obj_set_style_pad_all(grid, 0, 0);
        lv_obj_set_grid_dsc_array(grid, gcols, grows);
        lv_obj_set_scrollbar_mode(grid, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);

        /* col0:hero-card — START 对齐,不随侧栏拉伸 */
        lv_obj_t *hero = build_hero_card(grid);
        lv_obj_set_grid_cell(hero, LV_GRID_ALIGN_STRETCH, 0, 1,
                              LV_GRID_ALIGN_START, 0, 1);

        /* col1:side-stack — flex 列 gap=panel_pad,自然高度(比 hero 高,撑起 grid 行高) */
        lv_obj_t *side = lv_obj_create(grid);
        lv_obj_remove_style_all(side);
        lv_obj_set_width(side, LV_PCT(100));
        lv_obj_set_height(side, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(side, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(side, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                               LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(side, m->panel_pad, 0);
        lv_obj_set_style_pad_all(side, 0, 0);
        lv_obj_set_scrollbar_mode(side, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(side, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_grid_cell(side, LV_GRID_ALIGN_STRETCH, 1, 1,
                              LV_GRID_ALIGN_START, 0, 1);

        build_recommend_panel(side);   /* 推荐广播(3 items) */
        build_recent_panel(side);      /* 最近播放(3 items + 3 metrics) */
    }

    return page;
}
