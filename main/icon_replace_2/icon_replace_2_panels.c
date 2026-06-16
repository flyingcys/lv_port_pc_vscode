#include "icon_replace_2_panels.h"
#include "icon_replace_2_widgets.h"
#include "icon_replace_2_metrics.h"
#include "icon_replace_2_theme.h"
#include "icon_replace_2_glyphs.h"
#include <stdlib.h>
#include <string.h>

struct icon_replace_2_panels { lv_obj_t * control; lv_obj_t * notify; };

static void slide_to(lv_obj_t * o, int32_t y, bool anim){
    if(anim){
        lv_anim_t a; lv_anim_init(&a); lv_anim_set_var(&a,o);
        lv_anim_set_exec_cb(&a,(lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_time(&a,250); lv_anim_set_values(&a, lv_obj_get_y(o), y); lv_anim_start(&a);
    } else {
        lv_obj_set_y(o, y);
    }
}

static lv_obj_t * make_title(lv_obj_t * parent, const char * txt){
    const ir2_metrics_t * m = ir2_metrics();
    lv_obj_t * t = lv_label_create(parent);
    lv_obj_set_style_text_font(t, m->font_label, 0);
    lv_obj_set_style_text_color(t, ir2_theme()->text_primary, 0);
    lv_label_set_text(t, txt); ir2_make_decorative(t);
    return t;
}

icon_replace_2_panels_t * ir2_panels_create(lv_obj_t * parent){
    const ir2_metrics_t * m = ir2_metrics();
    icon_replace_2_panels_t * p = lv_malloc_zeroed(sizeof(*p));
    if(!p) return NULL;

    /* 控制中心：顶部下滑，初始位于屏幕上方外 */
    p->control = ir2_widget_glass_panel(parent, m->screen_w, m->panel_h);
    lv_obj_set_pos(p->control, 0, -m->panel_h);
    lv_obj_set_flex_flow(p->control, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(p->control, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(p->control, 16, 0);
    lv_obj_set_style_pad_row(p->control, 12, 0);

    make_title(p->control, "\xe6\x8e\xa7\xe5\x88\xb6\xe4\xb8\xad\xe5\xbf\x83");

    lv_obj_t * row = lv_obj_create(p->control); lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 16, 0); ir2_make_decorative(row);
    ir2_widget_toggle(row, IR2_GLYPH_WIFI, true);
    ir2_widget_toggle(row, IR2_GLYPH_BLUETOOTH, false);
    ir2_widget_toggle(row, IR2_GLYPH_AIRPLANE, true);
    ir2_widget_toggle(row, IR2_GLYPH_MOON, false);

    ir2_widget_slider(p->control, IR2_GLYPH_SUN, 70);
    ir2_widget_slider(p->control, IR2_GLYPH_SPEAKER, 45);

    /* 通知中心容器：Task 20 填充内容；先建空容器置于底部外 */
    p->notify = ir2_widget_glass_panel(parent, m->screen_w, m->panel_h);
    lv_obj_set_pos(p->notify, 0, m->screen_h);

    return p;
}

void ir2_panels_destroy(icon_replace_2_panels_t * p){
    if(!p) return;
    if(p->control) lv_obj_delete(p->control);
    if(p->notify) lv_obj_delete(p->notify);
    lv_free(p);
}

void ir2_panels_show_control(icon_replace_2_panels_t * p, bool show){
    const ir2_metrics_t * m = ir2_metrics();
    if(p && p->control) slide_to(p->control, show ? 0 : -m->panel_h, true);
}
void ir2_panels_show_notify(icon_replace_2_panels_t * p, bool show){
    const ir2_metrics_t * m = ir2_metrics();
    if(p && p->notify) slide_to(p->notify, show ? (m->screen_h - m->panel_h) : m->screen_h, true);
}

void ir2_panels_apply_initial(icon_replace_2_panels_t * p, const char * which){
    const ir2_metrics_t * m = ir2_metrics();
    if(!p || !which) return;
    if(strcmp(which, "control") == 0 && p->control) slide_to(p->control, 0, false);
    else if(strcmp(which, "notify") == 0 && p->notify) slide_to(p->notify, m->screen_h - m->panel_h, false);
}
