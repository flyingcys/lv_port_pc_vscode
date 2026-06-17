#include "icon_replace_2_panels.h"
#include "icon_replace_2_widgets.h"
#include "icon_replace_2_metrics.h"
#include "icon_replace_2_theme.h"
#include "icon_replace_2_glyphs.h"
#include "icon_replace_2_data.h"
#include <stdlib.h>
#include <string.h>

struct icon_replace_2_panels {
    lv_obj_t * control;
    lv_obj_t * notify;
    int32_t    h_control;     /* 实测内容高 */
    int32_t    h_notify;
    int        active;        /* 0=无 1=控制中心 2=通知中心 */
    int        dragging;      /* 当前拖拽的 which；0=无 */
    int32_t    cur_reveal;    /* 跟手过程当前 reveal */
    int32_t    last_delta;    /* 末段露出增量（朝开为正），供甩动判定 */
    int32_t    handle_press_y;/* 把手收起拖拽起点(屏幕Y) */
};

/* ---- 几何辅助 ---- */
static int32_t panel_h_of(icon_replace_2_panels_t * p, int which){
    return (which == IR2_PANEL_CONTROL) ? p->h_control : p->h_notify;
}
static lv_obj_t * obj_of(icon_replace_2_panels_t * p, int which){
    return (which == IR2_PANEL_CONTROL) ? p->control : p->notify;
}
static int32_t closed_y_of(int which, int32_t H, int32_t screen_h){
    return (which == IR2_PANEL_CONTROL) ? -H : screen_h;
}
static int32_t open_y_of(int which, int32_t H, int32_t screen_h){
    return (which == IR2_PANEL_CONTROL) ? 0 : (screen_h - H);
}

static void slide_to(lv_obj_t * o, int32_t y, bool anim){
    if(anim){
        lv_anim_t a; lv_anim_init(&a); lv_anim_set_var(&a,o);
        lv_anim_set_exec_cb(&a,(lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_time(&a,400); lv_anim_set_values(&a, lv_obj_get_y(o), y);
        lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
        lv_anim_start(&a);
    } else {
        lv_anim_delete(o, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_obj_set_y(o, y);
    }
}

/* 跟手：按 reveal 直接定位（无动画） */
static void apply_reveal(icon_replace_2_panels_t * p, int which, int32_t reveal){
    const ir2_metrics_t * m = ir2_metrics();
    int32_t H = panel_h_of(p, which);
    slide_to(obj_of(p, which), ir2_panel_drag_y(which, reveal, H, m->screen_h), false);
}

/* 松手吸附 */
static void snap_release(icon_replace_2_panels_t * p, int which){
    const ir2_metrics_t * m = ir2_metrics();
    int32_t H = panel_h_of(p, which);
    bool open = ir2_panel_snap_open(p->cur_reveal, H, p->last_delta);
    slide_to(obj_of(p, which),
             open ? open_y_of(which, H, m->screen_h) : closed_y_of(which, H, m->screen_h),
             true);
    p->active = open ? which : 0;
    p->dragging = 0;
}

/* ---- 内部拖拽状态机（展开/收起共用） ---- */
static void drag_begin_internal(icon_replace_2_panels_t * p, int which, int32_t reveal0){
    p->dragging   = which;
    p->cur_reveal = reveal0;
    p->last_delta = 0;
}
static void drag_update_internal(icon_replace_2_panels_t * p, int which, int32_t reveal){
    p->last_delta = reveal - p->cur_reveal;   /* 朝开为正 */
    p->cur_reveal = reveal;
    apply_reveal(p, which, reveal);
}

/* ---- 收起拖拽回调（绑在整个面板上：空白处任意反向滑动都能收起） ---- */
/* 沿祖先链判定按压所属面板（装饰子对象的按压会穿透到面板，target 即面板本身） */
static int which_of_panel(icon_replace_2_panels_t * p, lv_obj_t * obj){
    while(obj){
        if(obj == p->control) return IR2_PANEL_CONTROL;
        if(obj == p->notify)  return IR2_PANEL_NOTIFY;
        obj = lv_obj_get_parent(obj);
    }
    return 0;
}
static void panel_pressed_cb(lv_event_t * e){
    icon_replace_2_panels_t * p = lv_event_get_user_data(e);
    int which = which_of_panel(p, lv_event_get_target(e));
    if(which == 0) return;
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    p->handle_press_y = pt.y;
    drag_begin_internal(p, which, panel_h_of(p, which));   /* 起点=全开 */
}
static void panel_pressing_cb(lv_event_t * e){
    icon_replace_2_panels_t * p = lv_event_get_user_data(e);
    int which = which_of_panel(p, lv_event_get_target(e));
    if(which == 0 || p->dragging != which) return;
    int32_t H = panel_h_of(p, which);
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    int32_t dy = pt.y - p->handle_press_y;
    /* 收起方向：control 向上(dy<0)收起；notify 向下(dy>0)收起。反向拖拽不展开（钳到全开） */
    int32_t close_amount = (which == IR2_PANEL_CONTROL) ? -dy : dy;
    if(close_amount < 0) close_amount = 0;
    int32_t reveal = H - close_amount;
    if(reveal < 0) reveal = 0;
    drag_update_internal(p, which, reveal);
}
static void panel_released_cb(lv_event_t * e){
    icon_replace_2_panels_t * p = lv_event_get_user_data(e);
    int which = which_of_panel(p, lv_event_get_target(e));
    if(which == 0 || p->dragging != which) return;
    snap_release(p, which);
}

static lv_obj_t * make_title(lv_obj_t * parent, const char * txt){
    const ir2_metrics_t * m = ir2_metrics();
    lv_obj_t * t = lv_label_create(parent);
    lv_obj_set_style_text_font(t, m->font_label, 0);
    lv_obj_set_style_text_color(t, ir2_theme()->text_primary, 0);
    lv_label_set_text(t, txt); ir2_make_decorative(t);
    return t;
}

/* 把收起拖拽回调挂到面板本身（整块面板都是收起手势面，按压不再穿透到感应带/桌面） */
static void attach_drag(icon_replace_2_panels_t * p, lv_obj_t * panel){
    lv_obj_add_flag(panel, LV_OBJ_FLAG_CLICKABLE);   /* glass_panel 默认装饰性，这里恢复可点击 */
    lv_obj_add_event_cb(panel, panel_pressed_cb,  LV_EVENT_PRESSED,  p);
    lv_obj_add_event_cb(panel, panel_pressing_cb, LV_EVENT_PRESSING, p);
    lv_obj_add_event_cb(panel, panel_released_cb, LV_EVENT_RELEASED, p);
}

icon_replace_2_panels_t * ir2_panels_create(lv_obj_t * parent){
    const ir2_metrics_t * m = ir2_metrics();
    icon_replace_2_panels_t * p = lv_malloc_zeroed(sizeof(*p));
    if(!p) return NULL;

    /* ---------- 控制中心（顶部下滑，全屏铺满） ---------- */
    p->control = ir2_widget_glass_panel(parent, m->screen_w, m->screen_h);
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
    lv_obj_set_style_pad_column(row, 16, 0); ir2_make_decorative(row);
    ir2_widget_toggle(row, IR2_GLYPH_WIFI, true);
    ir2_widget_toggle(row, IR2_GLYPH_BLUETOOTH, false);
    ir2_widget_toggle(row, IR2_GLYPH_AIRPLANE, true);
    ir2_widget_toggle(row, IR2_GLYPH_MOON, false);

    ir2_widget_slider(p->control, IR2_GLYPH_SUN, 70);
    ir2_widget_slider(p->control, IR2_GLYPH_SPEAKER, 45);

    /* 弹性占位：把把手顶到全屏面板底部 */
    lv_obj_t * ctrl_spacer = lv_obj_create(p->control);
    lv_obj_remove_style_all(ctrl_spacer);
    lv_obj_set_width(ctrl_spacer, LV_PCT(100));
    lv_obj_set_height(ctrl_spacer, 0);
    lv_obj_set_flex_grow(ctrl_spacer, 1);
    ir2_make_decorative(ctrl_spacer);

    /* 控制中心把手在底部（纯视觉，收起手势绑在整块面板） */
    ir2_widget_panel_handle(p->control);
    attach_drag(p, p->control);

    /* ---------- 通知中心（底部上滑，全屏铺满） ---------- */
    p->notify = ir2_widget_glass_panel(parent, m->screen_w, m->screen_h);
    lv_obj_set_style_radius(p->notify, 0, 0);   /* 全屏铺满，去圆角防露壁纸 */
    lv_obj_set_flex_flow(p->notify, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(p->notify, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(p->notify, 16, 0);
    lv_obj_set_style_pad_row(p->notify, 10, 0);

    /* 通知中心把手在顶部（纯视觉，收起手势绑在整块面板） */
    ir2_widget_panel_handle(p->notify);
    attach_drag(p, p->notify);

    make_title(p->notify, "\xe9\x80\x9a\xe7\x9f\xa5\xe4\xb8\xad\xe5\xbf\x83");  /* 通知中心 */

    const ir2_metrics_t * mm = ir2_metrics();
    const ir2_theme_t * th = ir2_theme();
    for(uint32_t i = 0; i < ir2_notify_count; i++) {
        const ir2_notify_t * n = &ir2_notifies[i];
        lv_obj_t * card = lv_obj_create(p->notify);
        lv_obj_remove_style_all(card);
        lv_obj_set_size(card, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_color(card, th->glass_hi, 0);
        lv_obj_set_style_bg_opa(card, th->glass_hi_opa, 0);
        lv_obj_set_style_radius(card, 14, 0);
        lv_obj_set_style_pad_all(card, 10, 0);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(card, 4, 0);
        ir2_make_decorative(card);

        lv_obj_t * title = lv_label_create(card);
        lv_obj_set_style_text_font(title, mm->font_label, 0);
        lv_obj_set_style_text_color(title, th->text_primary, 0);
        lv_label_set_text(title, n->title);
        ir2_make_decorative(title);

        lv_obj_t * body = lv_label_create(card);
        lv_obj_set_style_text_font(body, mm->font_label, 0);
        lv_obj_set_style_text_color(body, th->text_primary, 0);
        lv_obj_set_style_text_opa(body, 204, 0);
        lv_obj_set_width(body, LV_PCT(100));
        lv_label_set_long_mode(body, LV_LABEL_LONG_MODE_WRAP);
        lv_label_set_text(body, n->body);
        ir2_make_decorative(body);

        lv_obj_t * tm = lv_label_create(card);
        lv_obj_set_style_text_font(tm, mm->font_label, 0);
        lv_obj_set_style_text_color(tm, th->text_primary, 0);
        lv_obj_set_style_text_opa(tm, 128, 0);
        lv_label_set_text(tm, n->time);
        ir2_make_decorative(tm);
    }

    /* ---------- 全屏高度，置初始全关位 ---------- */
    p->h_control = m->screen_h;
    p->h_notify  = m->screen_h;
    lv_obj_set_pos(p->control, 0, closed_y_of(IR2_PANEL_CONTROL, p->h_control, m->screen_h));
    lv_obj_set_pos(p->notify,  0, closed_y_of(IR2_PANEL_NOTIFY,  p->h_notify,  m->screen_h));

    p->active = 0;
    p->dragging = 0;
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
    if(!p || !p->control) return;
    slide_to(p->control,
             show ? open_y_of(IR2_PANEL_CONTROL, p->h_control, m->screen_h)
                  : closed_y_of(IR2_PANEL_CONTROL, p->h_control, m->screen_h), true);
    p->active = show ? IR2_PANEL_CONTROL : 0;
    p->dragging = 0;
}
void ir2_panels_show_notify(icon_replace_2_panels_t * p, bool show){
    const ir2_metrics_t * m = ir2_metrics();
    if(!p || !p->notify) return;
    slide_to(p->notify,
             show ? open_y_of(IR2_PANEL_NOTIFY, p->h_notify, m->screen_h)
                  : closed_y_of(IR2_PANEL_NOTIFY, p->h_notify, m->screen_h), true);
    p->active = show ? IR2_PANEL_NOTIFY : 0;
    p->dragging = 0;
}

void ir2_panels_apply_initial(icon_replace_2_panels_t * p, const char * which){
    const ir2_metrics_t * m = ir2_metrics();
    if(!p || !which) return;
    if(strcmp(which, "control") == 0 && p->control) {
        slide_to(p->control, open_y_of(IR2_PANEL_CONTROL, p->h_control, m->screen_h), false);
        p->active = IR2_PANEL_CONTROL;
    } else if(strcmp(which, "notify") == 0 && p->notify) {
        slide_to(p->notify, open_y_of(IR2_PANEL_NOTIFY, p->h_notify, m->screen_h), false);
        p->active = IR2_PANEL_NOTIFY;
    }
}

void ir2_panels_drag_begin(icon_replace_2_panels_t * p, int which){
    if(!p) return;
    if(which != IR2_PANEL_CONTROL && which != IR2_PANEL_NOTIFY) return;
    drag_begin_internal(p, which, 0);   /* 从全关起拖 */
}
void ir2_panels_drag_update(icon_replace_2_panels_t * p, int which, int32_t reveal){
    if(!p || p->dragging != which) return;
    drag_update_internal(p, which, reveal);
}
void ir2_panels_drag_end(icon_replace_2_panels_t * p, int which){
    if(!p || p->dragging != which) return;
    snap_release(p, which);
}

int ir2_panels_active(icon_replace_2_panels_t * p){
    return p ? p->active : 0;
}

void ir2_panels_bring_to_front(icon_replace_2_panels_t * p){
    if(!p) return;
    /* 置于边缘感应带之上：全屏展开时把手才点得到（关闭时面板在屏外，不挡感应带） */
    if(p->control) lv_obj_move_foreground(p->control);
    if(p->notify)  lv_obj_move_foreground(p->notify);
}
