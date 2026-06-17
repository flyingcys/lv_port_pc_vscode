#include "icon_replace_2_widgets.h"
#include "icon_replace_2_metrics.h"
#include "icon_replace_2_theme.h"
#include "icon_replace_2_glyphs.h"

#define IR2_DOT_INACTIVE_OPA 96   /* 分页点非活跃透明度 */

void ir2_make_decorative(lv_obj_t * obj){
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

lv_obj_t * ir2_widget_app_tile(lv_obj_t * parent, const ir2_app_t * app){
    const ir2_metrics_t * m = ir2_metrics();
    const ir2_theme_t * th = ir2_theme();
    int32_t col_w = ir2_grid_col_w();

    lv_obj_t * tile = lv_obj_create(parent);
    lv_obj_remove_style_all(tile);
    lv_obj_set_size(tile, col_w, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(tile, 6, 0);
    lv_obj_add_flag(tile, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * img = lv_image_create(tile);
    lv_image_set_src(img, app->icon);
    /* 图标 C 数组按 800 档 96px 生成；其它档用缩放贴合 m->icon_size（256=100%） */
    if(m->icon_size != 96) lv_image_set_scale(img, (uint32_t)(256 * m->icon_size / 96));
    ir2_make_decorative(img);

    lv_obj_t * lbl = lv_label_create(tile);
    lv_label_set_text(lbl, app->name);
    lv_obj_set_style_text_font(lbl, m->font_label, 0);
    lv_obj_set_style_text_color(lbl, th->text_primary, 0);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_width(lbl, col_w - 8);
    lv_obj_set_height(lbl, lv_font_get_line_height(m->font_label)); /* 锁单行高，防 CJK 窄列竖排 */
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    ir2_make_decorative(lbl);
    return tile;
}

lv_obj_t * ir2_widget_glass_panel(lv_obj_t * parent, int32_t w, int32_t h){
    const ir2_theme_t * th = ir2_theme();
    lv_obj_t * p = lv_obj_create(parent);
    lv_obj_remove_style_all(p);
    lv_obj_set_size(p, w, h);
    lv_obj_set_style_bg_color(p, th->panel_bg, 0);
    lv_obj_set_style_bg_opa(p, th->panel_opa, 0);          /* 半透明近似毛玻璃 */
    lv_obj_set_style_radius(p, 24, 0);
    lv_obj_set_style_border_color(p, th->glass_border, 0);
    lv_obj_set_style_border_opa(p, th->glass_border_opa, 0);
    lv_obj_set_style_border_width(p, 1, 0);
    lv_obj_set_style_shadow_width(p, 24, 0);
    lv_obj_set_style_shadow_opa(p, 80, 0);
    lv_obj_set_style_shadow_color(p, th->overlay_dark, 0);
    ir2_make_decorative(p);
    return p;
}

lv_obj_t * ir2_widget_toggle(lv_obj_t * parent, const char * glyph, bool on){
    const ir2_metrics_t * m = ir2_metrics(); const ir2_theme_t * th = ir2_theme();
    lv_obj_t * b = lv_obj_create(parent);
    lv_obj_remove_style_all(b); lv_obj_set_size(b, 56, 56);
    lv_obj_set_style_radius(b, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(b, on ? LV_OPA_COVER : th->glass_hi_opa, 0);
    lv_obj_set_style_bg_color(b, on ? th->accent : th->glass_hi, 0);
    lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE); lv_obj_remove_flag(b, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * g = lv_label_create(b);
    lv_obj_set_style_text_font(g, m->font_glyph, 0);
    lv_obj_set_style_text_color(g, th->text_primary, 0);
    lv_label_set_text(g, glyph); lv_obj_center(g); ir2_make_decorative(g);
    return b;
}

lv_obj_t * ir2_widget_slider(lv_obj_t * parent, const char * glyph, int32_t val){
    const ir2_metrics_t * m = ir2_metrics(); const ir2_theme_t * th = ir2_theme();
    lv_obj_t * row = lv_obj_create(parent); lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 10, 0); ir2_make_decorative(row);
    lv_obj_t * g = lv_label_create(row);
    lv_obj_set_style_text_font(g, m->font_glyph, 0);
    lv_obj_set_style_text_color(g, th->text_primary, 0);
    lv_label_set_text(g, glyph); ir2_make_decorative(g);
    lv_obj_t * sl = lv_slider_create(row);
    lv_obj_set_flex_grow(sl, 1);
    lv_obj_set_height(sl, 12);
    lv_slider_set_value(sl, val, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(sl, th->glass_hi, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sl, th->glass_hi_opa, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sl, th->text_primary, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(sl, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_opa(sl, LV_OPA_TRANSP, LV_PART_KNOB);
    return row;
}

lv_obj_t * ir2_widget_dots(lv_obj_t * parent, uint32_t count, uint32_t active){
    const ir2_theme_t * th = ir2_theme();
    lv_obj_t * row = lv_obj_create(parent); lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 8, 0); ir2_make_decorative(row);
    for(uint32_t i=0;i<count;i++){
        lv_obj_t * d = lv_obj_create(row); lv_obj_remove_style_all(d);
        bool a = (i==active);
        lv_obj_set_size(d, a?18:8, 8);
        lv_obj_set_style_radius(d, 4, 0);
        lv_obj_set_style_bg_color(d, th->text_primary, 0);
        lv_obj_set_style_bg_opa(d, a?LV_OPA_COVER:IR2_DOT_INACTIVE_OPA, 0);
        ir2_make_decorative(d);
    }
    return row;
}

void ir2_widget_dots_set_active(lv_obj_t * dots, uint32_t active){
    uint32_t n = lv_obj_get_child_count(dots);
    for(uint32_t i=0;i<n;i++){ lv_obj_t*d=lv_obj_get_child(dots,i); bool a=(i==active);
        lv_obj_set_width(d, a?18:8); lv_obj_set_style_bg_opa(d, a?LV_OPA_COVER:IR2_DOT_INACTIVE_OPA, 0); }
}

lv_obj_t * ir2_widget_panel_handle(lv_obj_t * parent){
    const ir2_metrics_t * m = ir2_metrics();
    const ir2_theme_t * th = ir2_theme();
    int32_t strip_h = (m->screen_h <= 272) ? 20 : 30;
    int32_t pill_w  = (m->screen_h <= 272) ? 40 : 60;
    int32_t pill_h  = (m->screen_h <= 272) ? 4  : 6;

    /* 视觉条：满宽固定高、纯装饰（不可点击，按压穿透到面板由面板统一处理收起） */
    lv_obj_t * strip = lv_obj_create(parent);
    lv_obj_remove_style_all(strip);
    lv_obj_set_size(strip, LV_PCT(100), strip_h);
    lv_obj_set_style_bg_opa(strip, LV_OPA_TRANSP, 0);
    ir2_make_decorative(strip);

    /* 可见小药丸：居中 */
    lv_obj_t * pill = lv_obj_create(strip);
    lv_obj_remove_style_all(pill);
    lv_obj_set_size(pill, pill_w, pill_h);
    lv_obj_set_style_radius(pill, pill_h / 2, 0);
    lv_obj_set_style_bg_color(pill, th->text_primary, 0);
    lv_obj_set_style_bg_opa(pill, 76, 0);   /* ~rgba(255,255,255,0.3) */
    lv_obj_center(pill);
    ir2_make_decorative(pill);
    return strip;
}
