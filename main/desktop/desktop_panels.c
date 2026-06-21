#include "desktop_panels.h"
#include "desktop_widgets.h"
#include "desktop_metrics.h"
#include "desktop_theme.h"
#include "desktop_glyphs.h"
#include "desktop_data.h"
#include <stdlib.h>
#include <string.h>

struct desktop_panels {
    lv_obj_t * control;
    lv_obj_t * notify;
    int32_t    h_control;     /* 实测内容高 */
    int32_t    h_notify;
    int        active;        /* 0=无 1=控制中心 2=通知中心 */
    int32_t    handle_press_y;/* 把手收起拖拽起点(屏幕Y) */
};

/* ---- 几何辅助 ---- */
static int32_t panel_h_of(desktop_panels_t * p, int which){
    return (which == DESKTOP_PANEL_CONTROL) ? p->h_control : p->h_notify;
}
static lv_obj_t * obj_of(desktop_panels_t * p, int which){
    return (which == DESKTOP_PANEL_CONTROL) ? p->control : p->notify;
}
static int32_t closed_y_of(int which, int32_t H, int32_t screen_h){
    return (which == DESKTOP_PANEL_CONTROL) ? -H : screen_h;
}
static int32_t open_y_of(int which, int32_t H, int32_t screen_h){
    return (which == DESKTOP_PANEL_CONTROL) ? 0 : (screen_h - H);
}

static void slide_to(lv_obj_t * o, int32_t y, bool anim, bool overshoot){
    if(anim){
        lv_anim_t a; lv_anim_init(&a); lv_anim_set_var(&a,o);
        lv_anim_set_exec_cb(&a,(lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_time(&a,400); lv_anim_set_values(&a, lv_obj_get_y(o), y);
        lv_anim_set_path_cb(&a, overshoot ? lv_anim_path_overshoot : lv_anim_path_ease_in);
        lv_anim_start(&a);
    } else {
        lv_anim_delete(o, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_obj_set_y(o, y);
    }
}

/* ---- 收起拖拽回调（绑在整个面板上：空白处任意反向滑动都能收起） ---- */
/* 沿祖先链判定按压所属面板（装饰子对象的按压会穿透到面板，target 即面板本身） */
static int which_of_panel(desktop_panels_t * p, lv_obj_t * obj){
    while(obj){
        if(obj == p->control) return DESKTOP_PANEL_CONTROL;
        if(obj == p->notify)  return DESKTOP_PANEL_NOTIFY;
        obj = lv_obj_get_parent(obj);
    }
    return 0;
}
static void panel_pressed_cb(lv_event_t * e){
    desktop_panels_t * p = lv_event_get_user_data(e);
    int which = which_of_panel(p, lv_event_get_target(e));
    if(which == 0 || p->active != which) return;
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    p->handle_press_y = pt.y;
}
static void panel_pressing_cb(lv_event_t * e){
    desktop_panels_t * p = lv_event_get_user_data(e);
    int which = which_of_panel(p, lv_event_get_target(e));
    if(which == 0 || p->active != which) return;
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    int32_t dy = pt.y - p->handle_press_y;
    int32_t close_amount = (which == DESKTOP_PANEL_CONTROL) ? -dy : dy;
    if(close_amount > 50) {
        const desktop_metrics_t * m = desktop_metrics();
        int32_t H = panel_h_of(p, which);
        slide_to(obj_of(p, which), closed_y_of(which, H, m->screen_h), true, false);
        p->active = 0;
    }
}
static lv_obj_t * make_title(lv_obj_t * parent, const char * txt){
    const desktop_metrics_t * m = desktop_metrics();
    lv_obj_t * t = lv_label_create(parent);
    lv_obj_set_style_text_font(t, m->font_label, 0);
    lv_obj_set_style_text_color(t, desktop_theme()->text_primary, 0);
    lv_label_set_text(t, txt); desktop_make_decorative(t);
    return t;
}

/* 把收起拖拽回调挂到面板本身（整块面板都是收起手势面，按压不再穿透到感应带/桌面） */
static void attach_drag(desktop_panels_t * p, lv_obj_t * panel){
    lv_obj_add_flag(panel, LV_OBJ_FLAG_CLICKABLE);   /* glass_panel 默认装饰性，这里恢复可点击 */
    lv_obj_add_event_cb(panel, panel_pressed_cb,  LV_EVENT_PRESSED,  p);
    lv_obj_add_event_cb(panel, panel_pressing_cb, LV_EVENT_PRESSING, p);
}

desktop_panels_t * desktop_panels_create(lv_obj_t * parent){
    const desktop_metrics_t * m = desktop_metrics();
    desktop_panels_t * p = lv_malloc_zeroed(sizeof(*p));
    if(!p) return NULL;

    /* ---------- 控制中心（顶部下滑，全屏铺满） ---------- */
    p->control = desktop_widget_glass_panel(parent, m->screen_w, m->screen_h);
    lv_obj_set_style_radius(p->control, 0, 0);   /* 全屏铺满，去圆角防露壁纸 */
    lv_obj_set_flex_flow(p->control, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(p->control, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(p->control, 16, 0);
    lv_obj_set_style_pad_row(p->control, 12, 0);

    make_title(p->control, "\xe6\x8e\xa7\xe5\x88\xb6\xe4\xb8\xad\xe5\xbf\x83");  /* 控制中心 */

    lv_obj_t * row = lv_obj_create(p->control); lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 16, 0); desktop_make_decorative(row);
    desktop_widget_toggle(row, DESKTOP_GLYPH_WIFI, true);
    desktop_widget_toggle(row, DESKTOP_GLYPH_BLUETOOTH, false);
    desktop_widget_toggle(row, DESKTOP_GLYPH_AIRPLANE, true);
    desktop_widget_toggle(row, DESKTOP_GLYPH_MOON, false);

    desktop_widget_slider(p->control, DESKTOP_GLYPH_SUN, 70);
    desktop_widget_slider(p->control, DESKTOP_GLYPH_SPEAKER, 45);

    /* 弹性占位：把把手顶到全屏面板底部 */
    lv_obj_t * ctrl_spacer = lv_obj_create(p->control);
    lv_obj_remove_style_all(ctrl_spacer);
    lv_obj_set_width(ctrl_spacer, LV_PCT(100));
    lv_obj_set_height(ctrl_spacer, 0);
    lv_obj_set_flex_grow(ctrl_spacer, 1);
    desktop_make_decorative(ctrl_spacer);

    /* 控制中心把手在底部（纯视觉，收起手势绑在整块面板） */
    desktop_widget_panel_handle(p->control);
    attach_drag(p, p->control);

    /* ---------- 通知中心（底部上滑，全屏铺满） ---------- */
    p->notify = desktop_widget_glass_panel(parent, m->screen_w, m->screen_h);
    lv_obj_set_style_radius(p->notify, 0, 0);   /* 全屏铺满，去圆角防露壁纸 */
    lv_obj_set_flex_flow(p->notify, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(p->notify, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(p->notify, 16, 0);
    lv_obj_set_style_pad_row(p->notify, 10, 0);

    /* 通知中心把手在顶部（纯视觉，收起手势绑在整块面板） */
    desktop_widget_panel_handle(p->notify);
    attach_drag(p, p->notify);

    make_title(p->notify, "\xe9\x80\x9a\xe7\x9f\xa5\xe4\xb8\xad\xe5\xbf\x83");  /* 通知中心 */

    const desktop_metrics_t * mm = desktop_metrics();
    const desktop_theme_t * th = desktop_theme();
    for(uint32_t i = 0; i < desktop_notify_count; i++) {
        const desktop_notify_t * n = &desktop_notifies[i];
        lv_obj_t * card = lv_obj_create(p->notify);
        lv_obj_remove_style_all(card);
        lv_obj_set_size(card, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_color(card, th->glass_hi, 0);
        lv_obj_set_style_bg_opa(card, th->glass_hi_opa, 0);
        lv_obj_set_style_radius(card, 14, 0);
        lv_obj_set_style_pad_all(card, 10, 0);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(card, 4, 0);
        desktop_make_decorative(card);

        lv_obj_t * title = lv_label_create(card);
        lv_obj_set_style_text_font(title, mm->font_label, 0);
        lv_obj_set_style_text_color(title, th->text_primary, 0);
        lv_label_set_text(title, n->title);
        desktop_make_decorative(title);

        lv_obj_t * body = lv_label_create(card);
        lv_obj_set_style_text_font(body, mm->font_label, 0);
        lv_obj_set_style_text_color(body, th->text_primary, 0);
        lv_obj_set_style_text_opa(body, 204, 0);
        lv_obj_set_width(body, LV_PCT(100));
        lv_label_set_long_mode(body, LV_LABEL_LONG_MODE_WRAP);
        lv_label_set_text(body, n->body);
        desktop_make_decorative(body);

        lv_obj_t * tm = lv_label_create(card);
        lv_obj_set_style_text_font(tm, mm->font_label, 0);
        lv_obj_set_style_text_color(tm, th->text_primary, 0);
        lv_obj_set_style_text_opa(tm, 128, 0);
        lv_label_set_text(tm, n->time);
        desktop_make_decorative(tm);
    }

    /* ---------- 全屏高度，置初始全关位 ---------- */
    p->h_control = m->screen_h;
    p->h_notify  = m->screen_h;
    lv_obj_set_pos(p->control, 0, closed_y_of(DESKTOP_PANEL_CONTROL, p->h_control, m->screen_h));
    lv_obj_set_pos(p->notify,  0, closed_y_of(DESKTOP_PANEL_NOTIFY,  p->h_notify,  m->screen_h));

    p->active = 0;
    return p;
}

void desktop_panels_destroy(desktop_panels_t * p){
    if(!p) return;
    if(p->control) lv_obj_delete(p->control);
    if(p->notify) lv_obj_delete(p->notify);
    lv_free(p);
}

void desktop_panels_show_control(desktop_panels_t * p, bool show){
    const desktop_metrics_t * m = desktop_metrics();
    if(!p || !p->control) return;
    slide_to(p->control,
             show ? open_y_of(DESKTOP_PANEL_CONTROL, p->h_control, m->screen_h)
                  : closed_y_of(DESKTOP_PANEL_CONTROL, p->h_control, m->screen_h), true, show);
    p->active = show ? DESKTOP_PANEL_CONTROL : 0;
}
void desktop_panels_show_notify(desktop_panels_t * p, bool show){
    const desktop_metrics_t * m = desktop_metrics();
    if(!p || !p->notify) return;
    slide_to(p->notify,
             show ? open_y_of(DESKTOP_PANEL_NOTIFY, p->h_notify, m->screen_h)
                  : closed_y_of(DESKTOP_PANEL_NOTIFY, p->h_notify, m->screen_h), true, show);
    p->active = show ? DESKTOP_PANEL_NOTIFY : 0;
}

void desktop_panels_apply_initial(desktop_panels_t * p, const char * which){
    const desktop_metrics_t * m = desktop_metrics();
    if(!p || !which) return;
    if(strcmp(which, "control") == 0 && p->control) {
        slide_to(p->control, open_y_of(DESKTOP_PANEL_CONTROL, p->h_control, m->screen_h), false, false);
        p->active = DESKTOP_PANEL_CONTROL;
    } else if(strcmp(which, "notify") == 0 && p->notify) {
        slide_to(p->notify, open_y_of(DESKTOP_PANEL_NOTIFY, p->h_notify, m->screen_h), false, false);
        p->active = DESKTOP_PANEL_NOTIFY;
    }
}

int desktop_panels_active(desktop_panels_t * p){
    return p ? p->active : 0;
}

void desktop_panels_bring_to_front(desktop_panels_t * p){
    if(!p) return;
    /* 置于边缘感应带之上：全屏展开时把手才点得到（关闭时面板在屏外，不挡感应带） */
    if(p->control) lv_obj_move_foreground(p->control);
    if(p->notify)  lv_obj_move_foreground(p->notify);
}
