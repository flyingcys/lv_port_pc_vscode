/* main/src/v9_apple_music/am_page_list.c
 * 共用列表页模板 —— 广播 / 本地 / 歌单三页共享同一布局。
 * 对应 HTML renderListPage() + renderMediaItem()，规格见设计文档 §6.3 / styles.css。
 *
 * 布局（flex 列 gap14）：
 *   1. page-header（eyebrow 胶囊 + 大标题 + 副标题 | search-chip）
 *   2. feature-banner（grid 2 列 [FR120, FR80]，accent 渐变，白字）
 *      左列：banner_title + banner_desc + 3× feature-badge
 *      右列：2× feature-cardlet（白 16% 圆角卡）
 *   3. content-grid（grid 2 列 [FR1, 244px]，gap14）
 *      catalog-panel（col0）：panel-header + 4× media-item
 *      queue-panel  （col1）：headline + queue-badge + 3× queue-item
 */
#include "am_page_list.h"
#include "am_widgets.h"
#include "am_theme.h"
#include "am_fonts.h"
#include "am_icons.h"
#include <stdio.h>   /* snprintf */
#include <string.h>  /* strcmp  */

/* ─────────────────────────── 不透明度常量 ─────────────────────────── */
#define OPA_WHITE_18   46   /* 0.18 × 255 */
#define OPA_WHITE_16   41   /* 0.16 × 255 */
#define OPA_WHITE_84  214   /* 0.84 × 255 */
#define OPA_WHITE_80  204   /* 0.80 × 255 */
#define OPA_WHITE_56  143   /* 0.56 × 255 */
#define OPA_WHITE_62  158   /* 0.62 × 255 */
#define OPA_ACCENT_12  31   /* 0.12 × 255（queue-badge）*/
#define OPA_ACCENT_14  36   /* 0.14 × 255（aux-badge）*/

/* ─────────────────────── 小型静态辅助函数 ─────────────────────────── */

/** feature-badge：accent 背景上的白字小胶囊（白@18% bg + 白字）。
 *  CSS: .feature-badge { padding:6px 10px; r999; bg rgba(255,255,255,0.18); font-size:11px 700 } */
static lv_obj_t *build_feature_badge(lv_obj_t *parent, const char *text)
{
    lv_obj_t *badge = lv_obj_create(parent);
    lv_obj_remove_style_all(badge);
    lv_obj_set_size(badge, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(badge, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(badge, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(badge, OPA_WHITE_18, 0);
    lv_obj_set_style_pad_hor(badge, 10, 0);
    lv_obj_set_style_pad_ver(badge, 6, 0);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = am_text(badge, text, &am_font_11, AM_WHITE);
    lv_obj_center(lbl);

    return badge;
}

/** aux-badge：media-item 尾部的小标签（accent@14% bg + accent 字）。
 *  CSS: .item-action { bg rgba(accent,0.14); color:accent; r50% } — 此处文字版非图标版。*/
static lv_obj_t *build_aux_badge(lv_obj_t *parent, const char *text)
{
    const am_theme_t *t = am_theme_get(am_theme_current());

    lv_obj_t *badge = lv_obj_create(parent);
    lv_obj_remove_style_all(badge);
    lv_obj_set_size(badge, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(badge, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(badge, t->accent, 0);
    lv_obj_set_style_bg_opa(badge, OPA_ACCENT_14, 0);
    lv_obj_set_style_pad_hor(badge, 8, 0);
    lv_obj_set_style_pad_ver(badge, 4, 0);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    am_text(badge, text, &am_font_11, t->accent);

    return badge;
}

/* ─────────────────────── 1. page-header ───────────────────────────── */
static lv_obj_t *build_page_header(lv_obj_t *parent, const am_list_page_t *data)
{
    const am_theme_t *t = am_theme_get(am_theme_current());

    /* 外行：space-between，flex row */
    lv_obj_t *hdr = lv_obj_create(parent);
    lv_obj_remove_style_all(hdr);
    lv_obj_set_width(hdr, LV_PCT(100));
    lv_obj_set_height(hdr, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_SPACE_BETWEEN,
                           LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(hdr, 18, 0);
    lv_obj_clear_flag(hdr, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    /* ── 左列 ── */
    lv_obj_t *left = lv_obj_create(hdr);
    lv_obj_remove_style_all(left);
    lv_obj_set_flex_grow(left, 1);
    lv_obj_set_height(left, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(left, 4, 0);
    lv_obj_clear_flag(left, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    /* eyebrow 胶囊（白 56%，r999，dot + text） */
    lv_obj_t *eyebrow = lv_obj_create(left);
    lv_obj_remove_style_all(eyebrow);
    lv_obj_set_size(eyebrow, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(eyebrow, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(eyebrow, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(eyebrow, OPA_WHITE_56, 0);
    lv_obj_set_style_pad_hor(eyebrow, 10, 0);
    lv_obj_set_style_pad_ver(eyebrow, 6, 0);
    lv_obj_set_flex_flow(eyebrow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(eyebrow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(eyebrow, 8, 0);
    lv_obj_clear_flag(eyebrow, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    /* 7px accent 点 */
    lv_obj_t *dot = lv_obj_create(eyebrow);
    lv_obj_remove_style_all(dot);
    lv_obj_set_size(dot, 7, 7);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dot, t->accent, 0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    am_text(eyebrow, data->eyebrow, &am_font_11, AM_MUTED_STRONG);

    /* 大标题（34px） */
    lv_obj_t *title_lbl = am_text(left, data->title, &am_font_34, AM_TEXT);
    lv_obj_set_style_pad_top(title_lbl, 2, 0);

    /* 副标题（换行，最宽 380px） */
    lv_obj_t *sub = am_text(left, data->subtitle, &am_font_13, AM_MUTED);
    lv_label_set_long_mode(sub, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(sub, 380);

    /* ── 右侧 search-chip ── */
    lv_obj_t *chip = lv_obj_create(hdr);
    lv_obj_remove_style_all(chip);
    lv_obj_set_size(chip, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(chip, 18, 0);
    lv_obj_set_style_bg_color(chip, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(chip, OPA_WHITE_62, 0);
    lv_obj_set_style_pad_hor(chip, 14, 0);
    lv_obj_set_style_pad_ver(chip, 10, 0);
    lv_obj_set_flex_flow(chip, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(chip, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(chip, 10, 0);
    lv_obj_clear_flag(chip, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    am_text(chip, AM_ICON_SEARCH, &am_font_12, AM_MUTED);
    am_text(chip, "静态目录视图", &am_font_12, AM_MUTED);

    return hdr;
}

/* ─────────────────────── 2. feature-banner ────────────────────────── */
static lv_obj_t *build_feature_banner(lv_obj_t *parent, const am_list_page_t *data)
{
    const am_theme_t *t = am_theme_get(am_theme_current());

    /* CSS: grid [1.2fr 0.8fr] gap12, p18, r26, accent 渐变, 阴影 */
    static const int32_t bcols[] = {LV_GRID_FR(120), LV_GRID_FR(80),
                                    LV_GRID_TEMPLATE_LAST};
    static const int32_t brows[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *banner = lv_obj_create(parent);
    lv_obj_remove_style_all(banner);
    lv_obj_set_width(banner, LV_PCT(100));
    lv_obj_set_height(banner, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(banner, 26, 0);
    lv_obj_set_style_pad_all(banner, 18, 0);
    lv_obj_set_style_pad_column(banner, 12, 0);
    lv_obj_set_style_pad_row(banner, 0, 0);
    lv_obj_set_grid_dsc_array(banner, bcols, brows);
    /* accent → accent_soft 竖向渐变，近似 CSS 135° 线性渐变 */
    am_fill_grad2(banner, t->accent, t->accent_soft);
    /* 阴影（accent 20%） */
    lv_obj_set_style_shadow_width(banner, 40, 0);
    lv_obj_set_style_shadow_offset_y(banner, 18, 0);
    lv_obj_set_style_shadow_color(banner, t->accent, 0);
    lv_obj_set_style_shadow_opa(banner, 51, 0);   /* 0.20 × 255 */
    lv_obj_set_scrollbar_mode(banner, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(banner, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    /* ── 左列（col0）：标题 + 描述 + badges ── */
    lv_obj_t *left_cell = lv_obj_create(banner);
    lv_obj_remove_style_all(left_cell);
    lv_obj_set_width(left_cell, LV_PCT(100));
    lv_obj_set_height(left_cell, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(left_cell, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left_cell, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(left_cell, 0, 0);
    lv_obj_set_style_pad_all(left_cell, 0, 0);
    lv_obj_clear_flag(left_cell, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_grid_cell(left_cell, LV_GRID_ALIGN_STRETCH, 0, 1,
                          LV_GRID_ALIGN_START, 0, 1);

    /* banner_title（24px 白，换行） */
    lv_obj_t *bn_title = am_text(left_cell, data->banner_title, &am_font_24, AM_WHITE);
    lv_label_set_long_mode(bn_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(bn_title, LV_PCT(100));

    /* banner_desc（13px 白 84%，换行，mt8） */
    lv_obj_t *bn_desc = am_text(left_cell, data->banner_desc, &am_font_13, AM_WHITE);
    lv_label_set_long_mode(bn_desc, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(bn_desc, LV_PCT(100));
    lv_obj_set_style_opa(bn_desc, OPA_WHITE_84, 0);
    lv_obj_set_style_pad_top(bn_desc, 8, 0);

    /* feature-badges（flex row wrap gap8，mt16） */
    lv_obj_t *badges_row = lv_obj_create(left_cell);
    lv_obj_remove_style_all(badges_row);
    lv_obj_set_width(badges_row, LV_PCT(100));
    lv_obj_set_height(badges_row, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(badges_row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(badges_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(badges_row, 8, 0);
    lv_obj_set_style_pad_row(badges_row, 6, 0);
    lv_obj_set_style_pad_top(badges_row, 16, 0);
    lv_obj_set_style_pad_all(badges_row, 0, 0);
    lv_obj_set_style_pad_top(badges_row, 16, 0);
    lv_obj_clear_flag(badges_row, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    for(int i = 0; i < 3; i++){
        build_feature_badge(badges_row, data->badges[i]);
    }

    /* ── 右列（col1）：feature-visual，2× feature-cardlet ── */
    /* CSS: .feature-visual { align-content:center; justify-items:end; gap:10px } */
    lv_obj_t *right_cell = lv_obj_create(banner);
    lv_obj_remove_style_all(right_cell);
    lv_obj_set_width(right_cell, LV_PCT(100));
    lv_obj_set_height(right_cell, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(right_cell, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right_cell, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END,
                           LV_FLEX_ALIGN_END);
    lv_obj_set_style_pad_row(right_cell, 10, 0);
    lv_obj_set_style_pad_all(right_cell, 0, 0);
    lv_obj_clear_flag(right_cell, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_grid_cell(right_cell, LV_GRID_ALIGN_STRETCH, 1, 1,
                          LV_GRID_ALIGN_START, 0, 1);

    /* feature-cardlet 辅助 lambda（C 无 lambda，内联两次） */
    /* cardlet 1：queue_label + 说明文字 */
    {
        lv_obj_t *cardlet = lv_obj_create(right_cell);
        lv_obj_remove_style_all(cardlet);
        lv_obj_set_size(cardlet, 160, LV_SIZE_CONTENT);
        lv_obj_set_style_radius(cardlet, 18, 0);
        lv_obj_set_style_bg_color(cardlet, AM_WHITE, 0);
        lv_obj_set_style_bg_opa(cardlet, OPA_WHITE_16, 0);
        lv_obj_set_style_pad_all(cardlet, 12, 0);
        lv_obj_set_flex_flow(cardlet, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(cardlet, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                               LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(cardlet, 0, 0);
        lv_obj_clear_flag(cardlet, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *strong1 = am_text(cardlet, data->queue_label, &am_font_13, AM_WHITE);
        lv_obj_set_style_text_decor(strong1, LV_TEXT_DECOR_NONE, 0); /* bold via font weight */

        lv_obj_t *span1 = am_text(cardlet,
            "先用静态密度确认布局,再迁入 LVGL flex/grid。",
            &am_font_11, AM_WHITE);
        lv_obj_set_style_opa(span1, OPA_WHITE_80, 0);
        lv_obj_set_style_pad_top(span1, 6, 0);
        lv_label_set_long_mode(span1, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(span1, LV_PCT(100));
    }

    /* cardlet 2：视觉重点 + 说明文字 */
    {
        lv_obj_t *cardlet = lv_obj_create(right_cell);
        lv_obj_remove_style_all(cardlet);
        lv_obj_set_size(cardlet, 160, LV_SIZE_CONTENT);
        lv_obj_set_style_radius(cardlet, 18, 0);
        lv_obj_set_style_bg_color(cardlet, AM_WHITE, 0);
        lv_obj_set_style_bg_opa(cardlet, OPA_WHITE_16, 0);
        lv_obj_set_style_pad_all(cardlet, 12, 0);
        lv_obj_set_flex_flow(cardlet, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(cardlet, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                               LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(cardlet, 0, 0);
        lv_obj_clear_flag(cardlet, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

        am_text(cardlet, "视觉重点", &am_font_13, AM_WHITE);

        lv_obj_t *span2 = am_text(cardlet,
            "玻璃卡片、主题色高亮、二级信息尽量轻。",
            &am_font_11, AM_WHITE);
        lv_obj_set_style_opa(span2, OPA_WHITE_80, 0);
        lv_obj_set_style_pad_top(span2, 6, 0);
        lv_label_set_long_mode(span2, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(span2, LV_PCT(100));
    }

    return banner;
}

/* ─────────────────── 3a. media-item（catalog 行）─────────────────── */
/* CSS: .media-item { grid [44, 1fr, auto] gap12 p10/12 r18 bg rgba(255,255,255,0.46) } */
static lv_obj_t *build_media_item(lv_obj_t *parent, const am_media_item_t *item)
{
    const am_theme_t *t = am_theme_get(am_theme_current());

    /* kind → 封面渐变色（CSS .media-item[data-kind="..."] .media-cover） */
    lv_color_t cov_a, cov_b;
    if(strcmp(item->kind, "album") == 0){
        cov_a = lv_color_hex(0xdce8ff);
        cov_b = lv_color_hex(0xb3c6ff);
    } else if(strcmp(item->kind, "radio") == 0){
        cov_a = lv_color_hex(0xf8d9ec);
        cov_b = lv_color_hex(0xffd2cf);
    } else if(strcmp(item->kind, "playlist") == 0){
        cov_a = lv_color_hex(0xd3f8f0);
        cov_b = lv_color_hex(0xd7ecff);
    } else {
        /* song / default：主题 tile 色 */
        cov_a = t->tile_a;
        cov_b = t->tile_b;
    }

    /* 行容器（白 46%，r18，flex row gap12，pad 10/12） */
    lv_obj_t *row = am_card(parent, 18, 117);   /* 0.46 × 255 ≈ 117 */
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_hor(row, 12, 0);
    lv_obj_set_style_pad_ver(row, 10, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 12, 0);
    /* am_card 已 clear scrollable；CLICKABLE 也不需要 */
    lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE);

    /* 封面 44×44 */
    am_cover(row, 44, cov_a, cov_b);

    /* meta 列（flex-grow 1）：标题 + 副标题 */
    lv_obj_t *meta = lv_obj_create(row);
    lv_obj_remove_style_all(meta);
    lv_obj_set_flex_grow(meta, 1);
    lv_obj_set_height(meta, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(meta, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(meta, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(meta, 4, 0);
    lv_obj_clear_flag(meta, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *t_lbl = am_text(meta, item->title, &am_font_13, AM_TEXT);
    lv_label_set_long_mode(t_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(t_lbl, LV_PCT(100));

    lv_obj_t *s_lbl = am_text(meta, item->subtitle, &am_font_11, AM_MUTED);
    lv_label_set_long_mode(s_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_lbl, LV_PCT(100));

    /* extra（flex row gap10，align center）：meta span + aux badge */
    lv_obj_t *extra = lv_obj_create(row);
    lv_obj_remove_style_all(extra);
    lv_obj_set_size(extra, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(extra, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(extra, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(extra, 10, 0);
    lv_obj_clear_flag(extra, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    am_text(extra, item->meta, &am_font_11, AM_MUTED);
    build_aux_badge(extra, item->aux);

    return row;
}

/* ──────────────────── 3b. catalog-panel（col0）──────────────────── */
static lv_obj_t *build_catalog_panel(lv_obj_t *parent, const am_list_page_t *data)
{
    const am_theme_t *t = am_theme_get(am_theme_current());

    lv_obj_t *panel = am_panel(parent);
    lv_obj_set_width(panel, LV_PCT(100));
    lv_obj_set_height(panel, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(panel, 16, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(panel, 12, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    /* panel-header：space-between，"广播 列表" + "4 项假数据" */
    lv_obj_t *ph = lv_obj_create(panel);
    lv_obj_remove_style_all(ph);
    lv_obj_set_width(ph, LV_PCT(100));
    lv_obj_set_height(ph, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(ph, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ph, LV_FLEX_ALIGN_SPACE_BETWEEN,
                           LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(ph, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    char panel_title[64];
    snprintf(panel_title, sizeof(panel_title), "%s 列表", data->title);
    am_text(ph, panel_title, &am_font_14, AM_TEXT);
    am_text(ph, "4 项假数据", &am_font_11, t->accent);

    /* catalog-list（flex col gap8） */
    lv_obj_t *list = lv_obj_create(panel);
    lv_obj_remove_style_all(list);
    lv_obj_set_width(list, LV_PCT(100));
    lv_obj_set_height(list, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(list, 8, 0);
    lv_obj_clear_flag(list, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    for(int i = 0; i < 4; i++){
        build_media_item(list, &data->list[i]);
    }

    return panel;
}

/* ──────────────────── 3c. queue-panel（col1, 244px）────────────────── */
static lv_obj_t *build_queue_panel(lv_obj_t *parent, const am_list_page_t *data)
{
    lv_obj_t *panel = am_panel(parent);
    lv_obj_set_width(panel, 244);
    lv_obj_set_height(panel, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(panel, 16, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(panel, 14, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    /* queue-headline（13px/800 AM_TEXT）*/
    am_text(panel, "下一步结构观察", &am_font_13, AM_TEXT);

    /* queue-badge（accent@12% pill，queue_label） */
    am_pill(panel, data->queue_label, true);

    /* queue-list（flex col gap8） */
    lv_obj_t *qlist = lv_obj_create(panel);
    lv_obj_remove_style_all(qlist);
    lv_obj_set_width(qlist, LV_PCT(100));
    lv_obj_set_height(qlist, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(qlist, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(qlist, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(qlist, 8, 0);
    lv_obj_clear_flag(qlist, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    for(int i = 0; i < 3; i++){
        /* queue-item：白 48%，r16，pad 10/12，flex col */
        /* CSS: .queue-item { p10/12; r16; bg rgba(255,255,255,0.48) } */
        lv_obj_t *qi = am_card(qlist, 16, 122);   /* 0.48 × 255 ≈ 122 */
        lv_obj_set_width(qi, LV_PCT(100));
        lv_obj_set_height(qi, LV_SIZE_CONTENT);
        lv_obj_set_style_pad_hor(qi, 12, 0);
        lv_obj_set_style_pad_ver(qi, 10, 0);
        lv_obj_set_flex_flow(qi, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(qi, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                               LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(qi, 4, 0);
        lv_obj_clear_flag(qi, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_t *qt = am_text(qi, data->queue[i].title, &am_font_12, AM_TEXT);
        LV_UNUSED(qt);

        lv_obj_t *qs = am_text(qi, data->queue[i].subtitle, &am_font_11, AM_MUTED);
        lv_label_set_long_mode(qs, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(qs, LV_PCT(100));
    }

    return panel;
}

/* ──────────────────────── 公开入口 ────────────────────────────────── */
lv_obj_t *am_page_list_create(lv_obj_t *content_parent, const am_list_page_t *data)
{
    /* page 根：flex 列 gap14，宽 100%，高 LV_SIZE_CONTENT。
     * 父容器负责滚动，page 自身禁用滚动。 */
    lv_obj_t *page = lv_obj_create(content_parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_width(page, LV_PCT(100));
    lv_obj_set_height(page, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(page, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(page, 14, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    /* 1. page-header */
    build_page_header(page, data);

    /* 2. feature-banner */
    build_feature_banner(page, data);

    /* 3. content-grid（grid [FR1, 244px] gap14） */
    static const int32_t cgcols[] = {LV_GRID_FR(1), 244, LV_GRID_TEMPLATE_LAST};
    static const int32_t cgrows[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *cgrid = lv_obj_create(page);
    lv_obj_remove_style_all(cgrid);
    lv_obj_set_width(cgrid, LV_PCT(100));
    lv_obj_set_height(cgrid, LV_SIZE_CONTENT);
    lv_obj_set_grid_dsc_array(cgrid, cgcols, cgrows);
    lv_obj_set_style_pad_column(cgrid, 14, 0);
    lv_obj_set_style_pad_row(cgrid, 0, 0);
    lv_obj_set_style_pad_all(cgrid, 0, 0);
    lv_obj_set_scrollbar_mode(cgrid, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(cgrid, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    /* catalog-panel → col0 */
    lv_obj_t *cat = build_catalog_panel(cgrid, data);
    lv_obj_set_grid_cell(cat, LV_GRID_ALIGN_STRETCH, 0, 1,
                          LV_GRID_ALIGN_START, 0, 1);

    /* queue-panel → col1 */
    lv_obj_t *que = build_queue_panel(cgrid, data);
    lv_obj_set_grid_cell(que, LV_GRID_ALIGN_STRETCH, 1, 1,
                          LV_GRID_ALIGN_START, 0, 1);

    return page;
}
