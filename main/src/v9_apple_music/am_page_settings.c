/* main/src/v9_apple_music/am_page_settings.c
 * Apple Music 设置页 —— 对应 HTML renderSettingsPage()
 * 布局:
 *   800/640: flex 列 gap=page_gap → page-header + settings-layout(grid 2cols [settings_nav_w,FR1])
 *     左栏: settings-sidebar (am_panel, 3 tabs)
 *     右栏: settings-main (am_panel, 按 active_tab 渲染内容)
 *   480 (stack_content): flex 列 → page-header + horizontal tab-row + settings-main 全宽
 * page 自身 LV_SIZE_CONTENT 高度,由父容器负责滚动。
 */
#include "am_page_settings.h"
#include "am_widgets.h"
#include "am_theme.h"
#include "am_fonts.h"
#include "am_icons.h"
#include "am_data.h"
#include "am_metrics.h"

/* ── 回调上下文结构 ──────────────────────────────────────────────────── */
typedef struct {
    am_theme_pick_cb_t on_theme;
    am_tab_pick_cb_t   on_tab;
    void              *user;
    int                index;
} am_cb_ctx_t;

static void tab_click_cb(lv_event_t *e)
{
    am_cb_ctx_t *ctx = (am_cb_ctx_t *)lv_event_get_user_data(e);
    if(ctx && ctx->on_tab) ctx->on_tab(ctx->index, ctx->user);
}

static void swatch_click_cb(lv_event_t *e)
{
    am_cb_ctx_t *ctx = (am_cb_ctx_t *)lv_event_get_user_data(e);
    if(ctx && ctx->on_theme) ctx->on_theme(ctx->index, ctx->user);
}

static void free_cb_ctx(lv_event_t *e)
{
    lv_free(lv_event_get_user_data(e));
}

/* ── page-header (eyebrow + 设置 + subtitle + search-chip) ─────────── */
static void build_page_header(lv_obj_t *parent)
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
    lv_obj_clear_flag(hdr, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    /* ─ 左侧列 ─ */
    lv_obj_t *left = lv_obj_create(hdr);
    lv_obj_remove_style_all(left);
    lv_obj_set_flex_grow(left, 1);
    lv_obj_set_height(left, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(left, 4, 0);
    lv_obj_clear_flag(left, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    /* eyebrow 胶囊: r999, 白 56%, pad 6/10, flex row gap8 */
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

    am_text(eyebrow, "Appearance Controls", m->f_label, AM_MUTED_STRONG);

    /* 主标题 */
    lv_obj_t *title = am_text(left, "\xe8\xae\xbe\xe7\xbd\xae", m->f_title, AM_TEXT);
    lv_obj_set_style_pad_top(title, 2, 0);

    /* 副标题(换行) */
    lv_obj_t *sub = am_text(left,
        "\xe6\x9c\xac\xe8\xbd\xae\xe9\x87\x8d\xe7\x82\xb9\xe9\xaa\x8c\xe8\xaf\x81\xe4\xb8\xbb\xe9\xa2\x98\xe8\x89\xb2\xe5\x88\x87\xe6\x8d\xa2\xe3\x80\x81\xe5\x85\xa8\xe5\xb1\x80 shell \xe7\xa8\xb3\xe5\xae\x9a\xe6\x80\xa7\xef\xbc\x8c\xe4\xbb\xa5\xe5\x8f\x8a\xe8\xae\xbe\xe7\xbd\xae\xe9\xa1\xb5\xe4\xbd\x9c\xe4\xb8\xba\xe7\x8b\xac\xe7\xab\x8b\xe6\x95\xb4\xe9\xa1\xb5\xe7\x9a\x84\xe7\xbb\x93\xe6\x9e\x84\xe6\x84\x9f\xe3\x80\x82",
        m->f_body, AM_MUTED);
    lv_label_set_long_mode(sub, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(sub, LV_PCT(100));

    /* ─ 右侧 search-chip ─ */
    lv_obj_t *chip = lv_obj_create(hdr);
    lv_obj_remove_style_all(chip);
    lv_obj_set_size(chip, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(chip, 18, 0);
    lv_obj_set_style_bg_color(chip, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(chip, 159, 0);
    lv_obj_set_style_pad_hor(chip, 14, 0);
    lv_obj_set_style_pad_ver(chip, 10, 0);
    lv_obj_set_flex_flow(chip, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(chip, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(chip, 10, 0);
    lv_obj_clear_flag(chip, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    am_text(chip, AM_ICON_SEARCH2, m->f_label, AM_MUTED);
    am_text(chip,
        "\xe5\x8f\xb3\xe4\xbe\xa7\xe5\x86\x85\xe5\xae\xb9\xe5\x88\x87\xe9\xa1\xb5\xef\xbc\x8c\xe4\xb8\x8d\xe9\x94\x80\xe6\xaf\x81\xe5\xb7\xa6\xe6\xa0\x8f\xe4\xb8\x8e\xe6\x92\xad\xe6\x94\xbe\xe6\x9d\xa1",
        m->f_label, AM_MUTED);
}

/* ── settings-tab (一个左栏 tab 项) ────────────────────────────────── */
static lv_obj_t *build_settings_tab(lv_obj_t *parent, int index, bool active,
                                    am_theme_pick_cb_t on_theme, am_tab_pick_cb_t on_tab,
                                    void *user)
{
    const am_theme_t   *t = am_theme_get(am_theme_current());
    const am_metrics_t *m = am_metrics();

    lv_obj_t *tab = am_card(parent, 16, active ? 0 : 128); /* bg set below */
    lv_obj_set_width(tab, LV_PCT(100));
    lv_obj_set_height(tab, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(tab, 12, 0);
    lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(tab, 3, 0);

    if(active) {
        /* accent@12% 背景 + inset accent border */
        lv_obj_set_style_bg_color(tab, t->accent, 0);
        lv_obj_set_style_bg_opa(tab, AM_OPA_ACCENT_12, 0);
        lv_obj_set_style_border_width(tab, 2, 0);
        lv_obj_set_style_border_color(tab, t->accent, 0);
        lv_obj_set_style_border_opa(tab, LV_OPA_COVER, 0);
        lv_obj_set_style_border_side(tab, LV_BORDER_SIDE_FULL, 0);
    } else {
        lv_obj_set_style_bg_color(tab, AM_WHITE, 0);
        lv_obj_set_style_bg_opa(tab, 128, 0);   /* white@0.50 */
    }

    lv_color_t title_col = active ? t->accent : AM_TEXT;
    lv_color_t desc_col  = active ? t->accent_soft : AM_MUTED;
    /* accent_soft may be too close; use AM_MUTED for inactive and accent for active */
    if(!active) desc_col = AM_MUTED;

    lv_obj_t *title_lbl = am_text(tab, am_settings_tabs[index].title, m->f_body, title_col);
    lv_obj_set_style_text_decor(title_lbl, LV_TEXT_DECOR_NONE, 0); /* bold via font weight; use as-is */
    LV_UNUSED(title_lbl);

    lv_obj_t *desc_lbl = am_text(tab, am_settings_tabs[index].desc, m->f_label, desc_col);
    lv_label_set_long_mode(desc_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(desc_lbl, LV_PCT(100));

    /* 挂点击事件 */
    lv_obj_add_flag(tab, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(tab, LV_OBJ_FLAG_SCROLLABLE);

    am_cb_ctx_t *ctx = lv_malloc(sizeof(am_cb_ctx_t));
    if(ctx) {
        ctx->on_theme = on_theme;
        ctx->on_tab   = on_tab;
        ctx->user     = user;
        ctx->index    = index;
        lv_obj_add_event_cb(tab, tab_click_cb,  LV_EVENT_CLICKED, ctx);
        lv_obj_add_event_cb(tab, free_cb_ctx,   LV_EVENT_DELETE,  ctx);
    }

    return tab;
}

/* ── settings-sidebar (3 tabs) ─────────────────────────────────────── */
static void build_sidebar(lv_obj_t *parent, int active_tab,
                           am_theme_pick_cb_t on_theme, am_tab_pick_cb_t on_tab,
                           void *user)
{
    const am_metrics_t *m = am_metrics();

    lv_obj_t *sidebar = am_panel(parent);
    lv_obj_set_grid_cell(sidebar, LV_GRID_ALIGN_STRETCH, 0, 1,
                         LV_GRID_ALIGN_START, 0, 1);
    lv_obj_set_width(sidebar, LV_PCT(100));
    lv_obj_set_height(sidebar, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(sidebar, m->panel_pad, 0);
    lv_obj_set_flex_flow(sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sidebar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(sidebar, m->content_pad, 0);
    lv_obj_clear_flag(sidebar, LV_OBJ_FLAG_CLICKABLE);

    for(int i = 0; i < 3; i++) {
        build_settings_tab(sidebar, i, (i == active_tab), on_theme, on_tab, user);
    }
}

/* ── theme-swatch ───────────────────────────────────────────────────── */
static lv_obj_t *build_swatch(lv_obj_t *parent, int index, bool active,
                               am_theme_pick_cb_t on_theme, am_tab_pick_cb_t on_tab,
                               void *user)
{
    const am_theme_t   *t_current = am_theme_get(am_theme_current());
    const am_metrics_t *m = am_metrics();
    /* Each swatch uses ITS OWN theme's hero gradient */
    const am_theme_t *t_own = am_theme_get((am_theme_id_t)index);

    lv_obj_t *sw = am_card(parent, 18, 163); /* white@0.64 */
    lv_obj_set_height(sw, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(sw, 10, 0);
    lv_obj_set_flex_flow(sw, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sw, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(sw, 5, 0);

    if(active) {
        /* inset accent border 2px (current theme's accent) */
        lv_obj_set_style_border_width(sw, 2, 0);
        lv_obj_set_style_border_color(sw, t_current->accent, 0);
        lv_obj_set_style_border_opa(sw, LV_OPA_COVER, 0);
        lv_obj_set_style_border_side(sw, LV_BORDER_SIDE_FULL, 0);
    }

    /* swatch-preview: h42, r14, 该主题自己的 hero 三段渐变 */
    lv_obj_t *preview = lv_obj_create(sw);
    lv_obj_remove_style_all(preview);
    lv_obj_set_width(preview, LV_PCT(100));
    lv_obj_set_height(preview, 42);
    lv_obj_set_style_radius(preview, 14, 0);
    lv_obj_set_style_clip_corner(preview, true, 0);
    lv_obj_clear_flag(preview, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    am_fill_grad3(preview, t_own->hero_a, t_own->hero_b, t_own->hero_c);

    /* label */
    lv_obj_t *lbl = am_text(sw, am_theme_presets[index].label, m->f_body, AM_TEXT);
    LV_UNUSED(lbl);

    /* desc */
    lv_obj_t *desc = am_text(sw, am_theme_presets[index].desc, m->f_label, AM_MUTED);
    lv_label_set_long_mode(desc, LV_LABEL_LONG_DOT);
    lv_obj_set_width(desc, LV_PCT(100));

    /* 挂点击事件 */
    lv_obj_add_flag(sw, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(sw, LV_OBJ_FLAG_SCROLLABLE);

    am_cb_ctx_t *ctx = lv_malloc(sizeof(am_cb_ctx_t));
    if(ctx) {
        ctx->on_theme = on_theme;
        ctx->on_tab   = on_tab;
        ctx->user     = user;
        ctx->index    = index;
        lv_obj_add_event_cb(sw, swatch_click_cb, LV_EVENT_CLICKED, ctx);
        lv_obj_add_event_cb(sw, free_cb_ctx,     LV_EVENT_DELETE,  ctx);
    }

    return sw;
}

/* ── settings-card helper (标题 + 段落文字) ─────────────────────────── */
static lv_obj_t *build_settings_card(lv_obj_t *parent, const char *title, const char *body)
{
    const am_metrics_t *m = am_metrics();

    lv_obj_t *card = am_card(parent, 20, 143); /* white@0.56 */
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_height(card, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(card, m->panel_pad, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(card, 6, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    am_text(card, title, m->f_strong, AM_TEXT);

    if(body && body[0]) {
        lv_obj_t *p = am_text(card, body, m->f_label, AM_MUTED);
        lv_label_set_long_mode(p, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(p, LV_PCT(100));
    }

    return card;
}

/* ── about-card (带 span/strong/span 三段) ─────────────────────────── */
static void build_about_card(lv_obj_t *parent, const char *span1,
                              const char *strong, const char *span2)
{
    const am_metrics_t *m = am_metrics();

    lv_obj_t *card = am_card(parent, 18, 138); /* white@0.54 */
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_height(card, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(card, m->content_pad, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(card, 4, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    am_text(card, span1,  m->f_label,  AM_MUTED);
    am_text(card, strong, m->f_metric, AM_TEXT);
    lv_obj_t *sp2 = am_text(card, span2, m->f_label, AM_MUTED);
    lv_label_set_long_mode(sp2, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(sp2, LV_PCT(100));
}

/* ── settings-main: tab 0 外观 ─────────────────────────────────────── */
static void build_main_appearance(lv_obj_t *main_panel,
                                  am_theme_pick_cb_t on_theme, am_tab_pick_cb_t on_tab,
                                  void *user)
{
    const am_theme_t   *t = am_theme_get(am_theme_current());
    const am_metrics_t *m = am_metrics();

    /* ─ 1. 主题模式卡片 ─ */
    lv_obj_t *mode_card = build_settings_card(main_panel,
        "\xe4\xb8\xbb\xe9\xa2\x98\xe6\xa8\xa1\xe5\xbc\x8f",   /* 主题模式 */
        "\xe6\x9c\xac\xe9\x98\xb6\xe6\xae\xb5\xe5\x85\x88\xe5\xae\x8c\xe6\x88\x90\xe6\xb5\x85\xe8\x89\xb2\xe4\xb8\xbb\xe6\x96\xb9\xe6\xa1\x88\xef\xbc\x8c\xe6\xb7\xb1\xe8\x89\xb2\xe4\xb8\x8e\xe7\xb3\xbb\xe7\xbb\x9f\xe8\xb7\x9f\xe9\x9a\x8f\xe4\xbd\x9c\xe4\xb8\xba\xe7\xbb\x93\xe6\x9e\x84\xe5\x8d\xa0\xe4\xbd\x8d\xef\xbc\x8c\xe5\x90\x8e\xe7\xbb\xad\xe5\x9c\xa8 LVGL \xe9\x87\x8c\xe4\xbf\x9d\xe6\x8c\x81\xe5\x90\x8c\xe6\xa0\xb7\xe4\xbf\xa1\xe6\x81\xaf\xe8\xbe\xb9\xe7\x95\x8c\xe3\x80\x82"
        /* 本阶段先完成浅色主方案，深色与系统跟随作为结构占位，后续在 LVGL 里保持同样信息边界。 */
    );

    /* mode-row: flex row gap10, 3 pills */
    lv_obj_t *mode_row = lv_obj_create(mode_card);
    lv_obj_remove_style_all(mode_row);
    lv_obj_set_width(mode_row, LV_SIZE_CONTENT);
    lv_obj_set_height(mode_row, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(mode_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(mode_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(mode_row, 10, 0);
    lv_obj_clear_flag(mode_row, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    am_pill(mode_row, "\xe6\xb5\x85\xe8\x89\xb2", true);   /* 浅色 */
    am_pill(mode_row, "\xe6\xb7\xb1\xe8\x89\xb2", false);  /* 深色 */
    am_pill(mode_row, "\xe8\xb7\x9f\xe9\x9a\x8f\xe7\xb3\xbb\xe7\xbb\x9f", false); /* 跟随系统 */

    /* ─ 2. 主题颜色卡片 ─ */
    lv_obj_t *color_card = build_settings_card(main_panel,
        "\xe4\xb8\xbb\xe9\xa2\x98\xe9\xa2\x9c\xe8\x89\xb2",  /* 主题颜色 */
        "\xe5\x88\x87\xe6\x8d\xa2\xe4\xb8\xbb\xe9\xa2\x98\xe8\x89\xb2\xe6\x97\xb6\xef\xbc\x8c\xe5\x8f\xaa\xe5\x8f\x98\xe6\x9b\xb4\xe8\xa7\x86\xe8\xa7\x89 token\xef\xbc\x8c\xe4\xb8\x8d\xe6\x94\xb9\xe5\x8f\x98\xe5\xaf\xbc\xe8\x88\xaa\xe3\x80\x81\xe5\xb8\x83\xe5\xb1\x80\xe5\x92\x8c\xe4\xbf\xa1\xe6\x81\xaf\xe7\xbb\x93\xe6\x9e\x84\xe3\x80\x82"
        /* 切换主题色时，只变更视觉 token，不改变导航、布局和信息结构。 */
    );

    /* theme-swatches: grid 4 cols (800/640) or 2 cols (480/stack_content) */
    static const int32_t sw_cols4[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                        LV_GRID_FR(1), LV_GRID_FR(1),
                                        LV_GRID_TEMPLATE_LAST};
    static const int32_t sw_cols2[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                        LV_GRID_TEMPLATE_LAST};
    static const int32_t sw_rows1[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    static const int32_t sw_rows2[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *swatches = lv_obj_create(color_card);
    lv_obj_remove_style_all(swatches);
    lv_obj_set_width(swatches, LV_PCT(100));
    lv_obj_set_height(swatches, LV_SIZE_CONTENT);
    if(m->stack_content) {
        lv_obj_set_grid_dsc_array(swatches, sw_cols2, sw_rows2);
    } else {
        lv_obj_set_grid_dsc_array(swatches, sw_cols4, sw_rows1);
    }
    lv_obj_set_style_pad_column(swatches, 10, 0);
    lv_obj_set_style_pad_row(swatches, 10, 0);
    lv_obj_clear_flag(swatches, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(swatches, LV_SCROLLBAR_MODE_OFF);

    int cur = (int)am_theme_current();
    if(m->stack_content) {
        /* 2-col layout: 4 swatches in 2x2 grid */
        for(int i = 0; i < 4; i++) {
            lv_obj_t *sw = build_swatch(swatches, i, (i == cur), on_theme, on_tab, user);
            lv_obj_set_grid_cell(sw, LV_GRID_ALIGN_STRETCH, i % 2, 1,
                                 LV_GRID_ALIGN_START, i / 2, 1);
        }
    } else {
        for(int i = 0; i < 4; i++) {
            lv_obj_t *sw = build_swatch(swatches, i, (i == cur), on_theme, on_tab, user);
            lv_obj_set_grid_cell(sw, LV_GRID_ALIGN_STRETCH, i, 1,
                                 LV_GRID_ALIGN_START, 0, 1);
        }
    }

    /* ─ 3. settings-split: grid 2 cols gap12 ─ */
    static const int32_t sp_cols[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static const int32_t sp_rows[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *split = lv_obj_create(main_panel);
    lv_obj_remove_style_all(split);
    lv_obj_set_width(split, LV_PCT(100));
    lv_obj_set_height(split, LV_SIZE_CONTENT);
    lv_obj_set_grid_dsc_array(split, sp_cols, sp_rows);
    lv_obj_set_style_pad_column(split, 12, 0);
    lv_obj_clear_flag(split, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(split, LV_SCROLLBAR_MODE_OFF);

    /* 圆角强度卡片 */
    lv_obj_t *corner_card = build_settings_card(split,
        "\xe5\x9c\x86\xe8\xa7\x92\xe5\xbc\xba\xe5\xba\xa6",  /* 圆角强度 */
        "\xe5\x81\x8f\xe6\x9f\x94\xe5\x92\x8c\xef\xbc\x8c\xe8\xb4\xb4\xe8\xbf\x91 Apple Music \xe7\x9a\x84\xe5\x8d\xa1\xe7\x89\x87\xe4\xb8\x8e\xe7\x8e\xbb\xe7\x92\x83\xe8\xbe\xb9\xe7\x95\x8c\xe3\x80\x82"
        /* 偏柔和，贴近 Apple Music 的卡片与玻璃边界。 */
    );
    lv_obj_set_grid_cell(corner_card, LV_GRID_ALIGN_STRETCH, 0, 1,
                         LV_GRID_ALIGN_START, 0, 1);

    /* slider-row */
    lv_obj_t *slider_row = lv_obj_create(corner_card);
    lv_obj_remove_style_all(slider_row);
    lv_obj_set_width(slider_row, LV_PCT(100));
    lv_obj_set_height(slider_row, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(slider_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(slider_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(slider_row, 10, 0);
    lv_obj_clear_flag(slider_row, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    /* slider rail: h8 r999, 占剩余宽度 */
    lv_obj_t *rail = lv_obj_create(slider_row);
    lv_obj_remove_style_all(rail);
    lv_obj_set_flex_grow(rail, 1);
    lv_obj_set_height(rail, 8);
    lv_obj_set_style_radius(rail, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(rail, lv_color_hex(0x707588), 0);
    lv_obj_set_style_bg_opa(rail, 31, 0);   /* 0x707588@0.12 */
    lv_obj_set_style_clip_corner(rail, true, 0);
    lv_obj_clear_flag(rail, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(rail, LV_SCROLLBAR_MODE_OFF);

    /* fill child: 62% wide, h=100%, radius999, accent→accent 2段渐变 */
    lv_obj_t *fill = lv_obj_create(rail);
    lv_obj_remove_style_all(fill);
    lv_obj_set_width(fill, LV_PCT(62));
    lv_obj_set_height(fill, LV_PCT(100));
    lv_obj_set_style_radius(fill, LV_RADIUS_CIRCLE, 0);
    lv_obj_clear_flag(fill, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    am_fill_grad2(fill, t->accent, t->accent);

    lv_obj_t *pct_lbl = am_text(slider_row, "62%", m->f_body, AM_TEXT);
    LV_UNUSED(pct_lbl);

    /* 背景氛围卡片 */
    lv_obj_t *amb_card = build_settings_card(split,
        "\xe8\x83\x8c\xe6\x99\xaf\xe6\xb0\x9b\xe5\x9b\xb4",  /* 背景氛围 */
        "\xe7\xbb\xb4\xe6\x8c\x81\xe8\xbd\xbb\xe6\x9f\x94\xe9\x9b\xbe\xe9\x9d\xa2\xef\xbc\x8c\xe4\xb8\x8d\xe8\xae\xa9\xe8\x83\x8c\xe6\x99\xaf\xe7\xab\x9e\xe4\xba\x89\xe4\xbf\xa1\xe6\x81\xaf\xe5\xb1\x82\xe7\xba\xa7\xe3\x80\x82"
        /* 维持轻柔雾面，不让背景竞争信息层级。 */
    );
    lv_obj_set_grid_cell(amb_card, LV_GRID_ALIGN_STRETCH, 1, 1,
                         LV_GRID_ALIGN_START, 0, 1);

    /* toggle-row pills */
    lv_obj_t *toggle_row = lv_obj_create(amb_card);
    lv_obj_remove_style_all(toggle_row);
    lv_obj_set_width(toggle_row, LV_SIZE_CONTENT);
    lv_obj_set_height(toggle_row, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(toggle_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(toggle_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(toggle_row, 10, 0);
    lv_obj_clear_flag(toggle_row, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    am_pill(toggle_row, "\xe6\x9f\x94\xe5\x85\x89", true);  /* 柔光 */
    am_pill(toggle_row, "\xe5\x86\xb7\xe9\x9b\xbe", false); /* 冷雾 */

    /* ─ 4. about-card ─ */
    /* span: 当前主题 / strong: 主题名 / span: 联动区域说明 */
    build_about_card(main_panel,
        "\xe5\xbd\x93\xe5\x89\x8d\xe4\xb8\xbb\xe9\xa2\x98",  /* 当前主题 */
        am_theme_presets[(int)am_theme_current()].label,
        "\xe8\x81\x94\xe5\x8a\xa8\xe5\x8c\xba\xe5\x9f\x9f\xef\xbc\x9a\xe5\xb7\xa6\xe6\xa0\x8f\xe9\x80\x89\xe4\xb8\xad\xe6\x80\x81 / hero \xe6\xb8\x90\xe5\x8f\x98 / \xe8\xbf\xb7\xe4\xbd\xa0\xe6\x92\xad\xe6\x94\xbe\xe6\x9d\xa1\xe8\xbf\x9b\xe5\xba\xa6 / \xe4\xb8\xbb\xe8\xa6\x81\xe6\x8c\x89\xe9\x92\xae\xe3\x80\x82"
        /* 联动区域：左栏选中态 / hero 渐变 / 迷你播放条进度 / 主要按钮。 */
    );
}

/* ── settings-main: tab 1 播放 ─────────────────────────────────────── */
static void build_main_playback(lv_obj_t *main_panel)
{
    /* 播放占位卡片 */
    lv_obj_t *pb_card = build_settings_card(main_panel,
        "\xe6\x92\xad\xe6\x94\xbe\xe5\x8d\xa0\xe4\xbd\x8d",  /* 播放占位 */
        "\xe6\x9c\xac\xe8\xbd\xae\xe4\xb8\x8d\xe6\x8e\xa5\xe7\x9c\x9f\xe5\xae\x9e\xe6\x92\xad\xe6\x94\xbe\xe5\x99\xa8\xef\xbc\x8c\xe4\xbd\x86\xe5\x85\x88\xe7\xa1\xae\xe8\xb4\xa8\xe8\xae\xbe\xe7\xbd\xae\xe9\xa1\xb5\xe9\x87\x8c\xe7\x9a\x84\xe4\xba\x8c\xe7\xba\xa7\xe8\xa1\xa8\xe5\x8d\x95\xe7\xbb\x93\xe6\x9e\x84\xe5\xaf\x86\xe5\xba\xa6\xe3\x80\x82"
        /* 本轮不接真实播放器，但先确认设置页里的二级表单结构密度。 */
    );

    /* toggle-row pills: 自动续播 / Crossfade 8s / 广播优先 */
    lv_obj_t *tog = lv_obj_create(pb_card);
    lv_obj_remove_style_all(tog);
    lv_obj_set_width(tog, LV_SIZE_CONTENT);
    lv_obj_set_height(tog, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(tog, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tog, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(tog, 10, 0);
    lv_obj_clear_flag(tog, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    am_pill(tog, "\xe8\x87\xaa\xe5\x8a\xa8\xe7\xbb\xad\xe6\x92\xad", true);  /* 自动续播 */
    am_pill(tog, "Crossfade 8s", false);
    am_pill(tog, "\xe5\xb9\xbf\xe6\x92\xad\xe4\xbc\x98\xe5\x85\x88", false); /* 广播优先 */

    /* 队列策略卡片 */
    build_settings_card(main_panel,
        "\xe9\x98\x9f\xe5\x88\x97\xe7\xad\x96\xe7\x95\xa5",  /* 队列策略 */
        "\xe8\xbf\x9b\xe5\x85\xa5\xe5\xb9\xbf\xe6\x92\xad\xe9\xa1\xb5\xe6\x97\xb6\xe5\xb1\x95\xe7\xa4\xba\xe6\x9c\x80\xe8\xbf\x91\xe9\xa2\x91\xe9\x81\x93\xef\xbc\x9b\xe8\xbf\x9b\xe5\x85\xa5\xe6\x9c\xac\xe5\x9c\xb0\xe9\xa1\xb5\xe6\x97\xb6\xe4\xbf\x9d\xe6\x8c\x81\xe4\xb8\x8a\xe6\xac\xa1\xe6\x8e\x92\xe5\xba\x8f\xe3\x80\x82"
        /* 进入广播页时展示最近频道；进入本地页时保持上次排序。 */
    );

    /* about-card */
    build_about_card(main_panel,
        "\xe5\x90\x8e\xe7\xbb\xad\xe6\x8e\xa5\xe5\x85\xa5",  /* 后续接入 */
        "\xe5\x8f\xaa\xe6\x9b\xbf\xe6\x8d\xa2\xe5\x8f\xb3\xe4\xbe\xa7\xe5\x86\x85\xe5\xae\xb9\xe5\x8c\xba",  /* 只替换右侧内容区 */
        "\xe8\xbf\x99\xe8\x83\xbd\xe8\xae\xa9\xe6\x92\xad\xe6\x94\xbe\xe6\x9d\xa1\xe4\xb8\x8e\xe5\xb7\xa6\xe6\xa0\x8f\xe4\xbf\x9d\xe6\x8c\x81\xe7\xa8\xb3\xe5\xae\x9a\xef\xbc\x8c\xe9\x99\x8d\xe4\xbd\x8e\xe5\x90\x8e\xe7\xbb\xad\xe6\x8e\xa5\xe9\x80\xbb\xe8\xbe\x91\xe7\x9a\x84\xe5\xa4\x8d\xe6\x9d\x82\xe5\xba\xa6\xe3\x80\x82"
        /* 这能让播放条与左栏保持稳定，降低后续接逻辑的复杂度。 */
    );
}

/* ── settings-main: tab 2 关于 ─────────────────────────────────────── */
static void build_main_about(lv_obj_t *main_panel)
{
    /* 关于样机卡片 */
    build_settings_card(main_panel,
        "\xe5\x85\xb3\xe4\xba\x8e\xe6\xa0\xb7\xe6\x9c\xba",  /* 关于样机 */
        "\xe8\xbf\x99\xe6\x98\xaf\xe5\x85\x88\xe4\xba\x8e LVGL \xe7\x9a\x84 HTML \xe9\xab\x98\xe4\xbf\x9d\xe7\x9c\x9f\xe7\xa1\xae\xe8\xae\xa4\xe7\xa8\xbf\xef\xbc\x8c\xe7\x9b\xae\xe6\xa0\x87\xe6\x98\xaf\xe8\xae\xa9\xe5\xb8\x83\xe5\xb1\x80\xe3\x80\x81\xe6\xb0\x94\xe8\xb4\xa8\xe5\x92\x8c\xe4\xb8\xbb\xe9\xa2\x98\xe8\x81\x94\xe5\x8a\xa8\xe5\x85\x88\xe8\xbe\xbe\xe6\x88\x90\xe5\x85\xb1\xe8\xaf\x86\xe3\x80\x82"
        /* 这是先于 LVGL 的 HTML 高保真确认稿，目标是让布局、气质和主题联动先达成共识。 */
    );

    /* 设备假设卡片 */
    build_settings_card(main_panel,
        "\xe8\xae\xbe\xe5\xa4\x87\xe5\x81\x87\xe8\xae\xbe",  /* 设备假设 */
        "\xe7\x9b\xae\xe6\xa0\x87\xe5\x88\x86\xe8\xbe\xa8\xe7\x8e\x87 800x480\xef\xbc\x8c\xe5\xb7\xa6\xe6\xa0\x8f\xe5\xb8\xb8\xe9\xa9\xbb\xef\xbc\x8c\xe5\x8f\xb3\xe4\xbe\xa7\xe5\x86\x85\xe5\xae\xb9\xe5\x88\x87\xe9\xa1\xb5\xef\xbc\x8c\xe5\xba\x95\xe9\x83\xa8\xe8\xbf\xb7\xe4\xbd\xa0\xe6\x92\xad\xe6\x94\xbe\xe6\x9d\xa1\xe5\x9b\xba\xe5\xae\x9a\xe3\x80\x82"
        /* 目标分辨率 800x480，左栏常驻，右侧内容切页，底部迷你播放条固定。 */
    );

    /* about-card */
    build_about_card(main_panel,
        "\xe4\xb8\x8b\xe4\xb8\x80\xe6\xad\xa5",              /* 下一步 */
        "LVGL v9 flex/grid \xe5\xae\x9e\xe7\x8e\xb0",        /* LVGL v9 flex/grid 实现 */
        "\xe6\xa0\xb7\xe6\x9c\xba\xe7\xa1\xae\xe8\xae\xa4\xe5\x90\x8e\xef\xbc\x8c\xe5\x86\x8d\xe6\x8a\x8a\xe5\x90\x8c\xe4\xb8\x80\xe4\xbf\xa1\xe6\x81\xaf\xe6\x9e\xb6\xe6\x9e\x84\xe8\xbf\x81\xe7\xa7\xbb\xe5\x88\xb0 SDL \xe8\xbf\x90\xe8\xa1\x8c\xe7\x8e\xaf\xe5\xa2\x83\xe3\x80\x82"
        /* 样机确认后，再把同一信息架构迁移到 SDL 运行环境。 */
    );
}

/* ── settings-main panel ───────────────────────────────────────────── */
static void build_main(lv_obj_t *parent, int active_tab,
                        am_theme_pick_cb_t on_theme, am_tab_pick_cb_t on_tab,
                        void *user)
{
    const am_metrics_t *m = am_metrics();

    lv_obj_t *main_panel = am_panel(parent);
    lv_obj_set_grid_cell(main_panel, LV_GRID_ALIGN_STRETCH, 1, 1,
                         LV_GRID_ALIGN_START, 0, 1);
    lv_obj_set_width(main_panel, LV_PCT(100));
    lv_obj_set_height(main_panel, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(main_panel, m->panel_pad, 0);
    lv_obj_set_flex_flow(main_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(main_panel, m->page_gap, 0);
    lv_obj_clear_flag(main_panel, LV_OBJ_FLAG_CLICKABLE);

    switch(active_tab) {
    case 1:  build_main_playback(main_panel); break;
    case 2:  build_main_about(main_panel);    break;
    default: build_main_appearance(main_panel, on_theme, on_tab, user); break;
    }
}

/* ── 480 水平 tab 行 (stack_content 模式) ───────────────────────────── */
static void build_tab_row(lv_obj_t *parent, int active_tab,
                           am_theme_pick_cb_t on_theme, am_tab_pick_cb_t on_tab,
                           void *user)
{
    const am_theme_t   *t = am_theme_get(am_theme_current());
    const am_metrics_t *m = am_metrics();

    /* 外层 panel 全宽 */
    lv_obj_t *row_panel = am_panel(parent);
    lv_obj_set_width(row_panel, LV_PCT(100));
    lv_obj_set_height(row_panel, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(row_panel, m->content_pad, 0);
    lv_obj_set_flex_flow(row_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row_panel, m->content_pad, 0);
    lv_obj_clear_flag(row_panel, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    for(int i = 0; i < 3; i++) {
        bool active = (i == active_tab);

        lv_obj_t *btn = am_card(row_panel, 14, active ? 0 : 102);
        lv_obj_set_flex_grow(btn, 1);
        lv_obj_set_height(btn, LV_SIZE_CONTENT);
        lv_obj_set_style_pad_hor(btn, m->content_pad, 0);
        lv_obj_set_style_pad_ver(btn, 8, 0);
        lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                               LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_row(btn, 2, 0);

        if(active) {
            lv_obj_set_style_bg_color(btn, t->accent, 0);
            lv_obj_set_style_bg_opa(btn, AM_OPA_ACCENT_12, 0);
            lv_obj_set_style_border_width(btn, 2, 0);
            lv_obj_set_style_border_color(btn, t->accent, 0);
            lv_obj_set_style_border_opa(btn, LV_OPA_COVER, 0);
            lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_FULL, 0);
        } else {
            lv_obj_set_style_bg_color(btn, AM_WHITE, 0);
            lv_obj_set_style_bg_opa(btn, 102, 0); /* white@0.40 */
        }

        lv_color_t col = active ? t->accent : AM_TEXT;
        am_text(btn, am_settings_tabs[i].title, m->f_body, col);

        /* 点击回调 */
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

        am_cb_ctx_t *ctx = lv_malloc(sizeof(am_cb_ctx_t));
        if(ctx) {
            ctx->on_theme = on_theme;
            ctx->on_tab   = on_tab;
            ctx->user     = user;
            ctx->index    = i;
            lv_obj_add_event_cb(btn, tab_click_cb, LV_EVENT_CLICKED, ctx);
            lv_obj_add_event_cb(btn, free_cb_ctx,  LV_EVENT_DELETE,  ctx);
        }
    }
}

/* ── 480 stack_content 下的 settings-main panel ─────────────────────── */
static void build_main_stacked(lv_obj_t *parent, int active_tab,
                                am_theme_pick_cb_t on_theme, am_tab_pick_cb_t on_tab,
                                void *user)
{
    const am_metrics_t *m = am_metrics();

    lv_obj_t *main_panel = am_panel(parent);
    lv_obj_set_width(main_panel, LV_PCT(100));
    lv_obj_set_height(main_panel, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(main_panel, m->panel_pad, 0);
    lv_obj_set_flex_flow(main_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(main_panel, m->page_gap, 0);
    lv_obj_clear_flag(main_panel, LV_OBJ_FLAG_CLICKABLE);

    switch(active_tab) {
    case 1:  build_main_playback(main_panel); break;
    case 2:  build_main_about(main_panel);    break;
    default: build_main_appearance(main_panel, on_theme, on_tab, user); break;
    }
}

/* ── 公开入口 ───────────────────────────────────────────────────────── */
lv_obj_t *am_page_settings_create(lv_obj_t *content_parent, int active_tab,
                                  am_theme_pick_cb_t on_theme, am_tab_pick_cb_t on_tab,
                                  void *user)
{
    const am_metrics_t *m = am_metrics();

    /* page 根容器:100%宽,自然高,flex 列 gap=page_gap,透明,禁滚动 */
    lv_obj_t *page = lv_obj_create(content_parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_width(page, LV_PCT(100));
    lv_obj_set_height(page, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(page, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(page, m->page_gap, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);

    /* page-header */
    build_page_header(page);

    if(m->stack_content) {
        /* ── 480 单列模式:水平 tab 行 + settings-main 全宽 ── */
        build_tab_row(page, active_tab, on_theme, on_tab, user);
        build_main_stacked(page, active_tab, on_theme, on_tab, user);
    } else {
        /* ── 800/640 双列模式: grid [settings_nav_w, FR1] ── */
        int32_t nav_w = (int32_t)m->settings_nav_w;
        /* 动态 grid 列描述符(栈上 VLA-like,但 LVGL grid 要求指针持久有效直到对象销毁)
         * 分配在 lv_malloc 保证对象生命周期内有效 */
        int32_t *lay_cols = lv_malloc(3 * sizeof(int32_t));
        int32_t *lay_rows = lv_malloc(2 * sizeof(int32_t));
        if(lay_cols && lay_rows) {
            lay_cols[0] = nav_w;
            lay_cols[1] = LV_GRID_FR(1);
            lay_cols[2] = LV_GRID_TEMPLATE_LAST;
            lay_rows[0] = LV_GRID_CONTENT;
            lay_rows[1] = LV_GRID_TEMPLATE_LAST;
        }

        lv_obj_t *layout = lv_obj_create(page);
        lv_obj_remove_style_all(layout);
        lv_obj_set_width(layout, LV_PCT(100));
        lv_obj_set_height(layout, LV_SIZE_CONTENT);
        if(lay_cols && lay_rows) {
            lv_obj_set_grid_dsc_array(layout, lay_cols, lay_rows);
        }
        lv_obj_set_style_pad_column(layout, m->page_gap, 0);
        lv_obj_clear_flag(layout, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_scrollbar_mode(layout, LV_SCROLLBAR_MODE_OFF);

        /* 释放描述符时机:layout 销毁时,复用 free_cb_ctx (lv_free user_data) */
        if(lay_cols) lv_obj_add_event_cb(layout, free_cb_ctx, LV_EVENT_DELETE, lay_cols);
        if(lay_rows) lv_obj_add_event_cb(layout, free_cb_ctx, LV_EVENT_DELETE, lay_rows);

        /* left: sidebar */
        build_sidebar(layout, active_tab, on_theme, on_tab, user);

        /* right: main content */
        build_main(layout, active_tab, on_theme, on_tab, user);
    }

    return page;
}
