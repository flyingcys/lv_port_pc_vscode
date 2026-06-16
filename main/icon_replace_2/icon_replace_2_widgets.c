#include "icon_replace_2_widgets.h"
#include "icon_replace_2_metrics.h"
#include "icon_replace_2_theme.h"

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
