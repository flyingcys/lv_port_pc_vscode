#include "clock_view.h"
#include "clock_model.h"
#include "clock_metrics.h"

#include "lvgl.h"

#include <stdio.h>
#include <string.h>

LV_IMAGE_DECLARE(img_clock_alarm);
LV_IMAGE_DECLARE(img_clock_thermometer);
LV_IMAGE_DECLARE(img_clock_drop);
LV_IMAGE_DECLARE(img_clock_smile);

/* 数字滚动时长(对齐 mockup --roll-dur 520ms) */
#define ROLL_MS 520

typedef struct {
    lv_obj_t *cont;      /* slot 容器(clip_corner) */
    lv_obj_t *cur;       /* 当前数字 label */
    char       cur_digit;
} digit_slot_t;

struct clock_view;
static void grad3_free_cb(lv_event_t *e);
static void colon_breathe_cb(void *var, int32_t v);

struct clock_view {
    lv_obj_t *root;
    lv_obj_t *header;
    lv_obj_t *main;
    lv_obj_t *footer;

    lv_obj_t *lbl_year;
    lv_obj_t *lbl_month;
    lv_obj_t *lbl_day;

    lv_obj_t *lbl_period;
    lv_obj_t *img_alarm;
    lv_obj_t *clock_row;

    digit_slot_t slots[6];
    lv_obj_t    *colons[2];

    lv_obj_t *lbl_weekday;
    lv_obj_t *lbl_temp_val;
    lv_obj_t *lbl_humid_val;

    lv_timer_t *tick;
    const clock_metrics_t *m;
};

/* -------- 渐变助手(3 段竖向,malloc + 删除时释放) -------- */
static void grad3(lv_obj_t *o, lv_color_t c1, lv_color_t c2, lv_color_t c3)
{
    lv_grad_dsc_t *d = lv_malloc(sizeof(*d));
    if(!d) return;
    lv_memzero(d, sizeof(*d));
    d->dir = LV_GRAD_DIR_VER;
    d->stops_count = 3;
    d->stops[0].color = c1; d->stops[0].frac = 0;   d->stops[0].opa = LV_OPA_COVER;
    d->stops[1].color = c2; d->stops[1].frac = 128; d->stops[1].opa = LV_OPA_COVER;
    d->stops[2].color = c3; d->stops[2].frac = 255; d->stops[2].opa = LV_OPA_COVER;
    lv_obj_set_style_bg_grad(o, d, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(o, grad3_free_cb, LV_EVENT_DELETE, d);
}

static void grad3_free_cb(lv_event_t *e)
{
    lv_free(lv_event_get_user_data(e));
}

/* -------- 数字位 -------- */
static lv_obj_t *digit_slot_create(clock_view_t *clk, lv_obj_t *parent, digit_slot_t *out)
{
    const clock_metrics_t *m = clk->m;
    lv_obj_t *cont = lv_obj_create(parent);
    /* 必须先 remove_style_all 再 set_size:否则会清掉宽高,6 位数字塌成 1-2 个残影 */
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, m->slot_w, m->slot_h);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_style_radius(cont, 0, 0);
    /* 默认不设 OVERFLOW_VISIBLE → 子对象裁剪到 slot 矩形,滚动动画才干净 */
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    lv_obj_t *lbl = lv_label_create(cont);
    lv_obj_set_style_text_font(lbl, m->fonts.f_digit, 0);
    lv_obj_set_style_text_color(lbl, m->col_digit, 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lbl, m->slot_w);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);

    out->cont = cont;
    out->cur = lbl;
    out->cur_digit = 0;
    return cont;
}

static void set_y_cb(void *var, int32_t v) { lv_obj_set_y((lv_obj_t *)var, v); }

static void colon_breathe_cb(void *var, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)var, (lv_opa_t)v, 0);
}

static void digit_anim_ready_cb(lv_anim_t *a)
{
    digit_slot_t *out = (digit_slot_t *)a->user_data;
    lv_obj_t *incoming = (lv_obj_t *)a->var;
    lv_obj_delete(out->cur);
    out->cur = incoming;
    lv_obj_set_y(incoming, 0);
}

static void digit_slot_set(clock_view_t *clk, digit_slot_t *out, char next, bool anim)
{
    if(out->cur_digit == next) return;

    if(!anim) {
        char buf[2] = {next, 0};
        lv_label_set_text(out->cur, buf);
        out->cur_digit = next;
        return;
    }

    const clock_metrics_t *m = clk->m;
    lv_obj_t *incoming = lv_label_create(out->cont);
    lv_obj_set_style_text_font(incoming, m->fonts.f_digit, 0);
    lv_obj_set_style_text_color(incoming, m->col_digit, 0);
    lv_obj_set_style_text_align(incoming, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(incoming, m->slot_w);
    lv_obj_align(incoming, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_y(incoming, m->slot_h);
    char buf[2] = {next, 0};
    lv_label_set_text(incoming, buf);

    lv_anim_t a_out;
    lv_anim_init(&a_out);
    lv_anim_set_var(&a_out, out->cur);
    lv_anim_set_exec_cb(&a_out, set_y_cb);
    lv_anim_set_values(&a_out, 0, -(int32_t)m->slot_h);
    lv_anim_set_time(&a_out, ROLL_MS);
    lv_anim_set_path_cb(&a_out, lv_anim_path_ease_in_out);
    lv_anim_start(&a_out);

    lv_anim_t a_in;
    lv_anim_init(&a_in);
    lv_anim_set_var(&a_in, incoming);
    lv_anim_set_exec_cb(&a_in, set_y_cb);
    lv_anim_set_values(&a_in, (int32_t)m->slot_h, 0);
    lv_anim_set_time(&a_in, ROLL_MS);
    lv_anim_set_path_cb(&a_in, lv_anim_path_ease_in_out);
    lv_anim_set_user_data(&a_in, out);
    lv_anim_set_ready_cb(&a_in, digit_anim_ready_cb);
    lv_anim_start(&a_in);

    out->cur_digit = next;
}

/* -------- 冒号 -------- */
static lv_obj_t *colon_create(clock_view_t *clk, lv_obj_t *parent, lv_obj_t **out_colon)
{
    const clock_metrics_t *m = clk->m;
    lv_obj_t *col = lv_obj_create(parent);
    lv_obj_remove_style_all(col);
    lv_obj_set_size(col, m->colon_w, m->slot_h);
    lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    /* 两圆点绝对定位:水平居中、垂直以 colon_gap 对称 */
    int32_t cx = (m->colon_w - m->colon_dot) / 2;
    int32_t cy = m->slot_h / 2;
    int32_t half_gap = m->colon_gap / 2;
    for(int i = 0; i < 2; i++) {
        lv_obj_t *dot = lv_obj_create(col);
        lv_obj_remove_style_all(dot);
        lv_obj_set_size(dot, m->colon_dot, m->colon_dot);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(dot, m->col_digit, 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(dot, 0, 0);
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_pos(dot, cx,
                       (i == 0) ? (cy - half_gap - m->colon_dot) : (cy + half_gap));
    }

    /* 呼吸:opacity 255↔158(0.62),1.8s 周期 — 下到第一个 dot,其余跟 opa(单点足够) */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, col);
    lv_anim_set_exec_cb(&a, colon_breathe_cb);
    lv_anim_set_values(&a, 255, 158);
    lv_anim_set_time(&a, 900);
    lv_anim_set_playback_time(&a, 900);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    *out_colon = col;
    return col;
}

/* -------- 应用模型到视图 -------- */
static void apply_model(clock_view_t *clk, const clock_view_model_t *vm, bool anim)
{
    char buf[16];

    lv_label_set_text(clk->lbl_year, vm->year);
    lv_label_set_text(clk->lbl_month, vm->month);
    snprintf(buf, sizeof(buf), "%u", vm->day_of_month);
    lv_label_set_text(clk->lbl_day, buf);

    lv_label_set_text(clk->lbl_period,
                      vm->period == PERIOD_MORNING ? "MORNING" :
                      vm->period == PERIOD_AFTERNOON ? "AFTERNOON" :
                      vm->period == PERIOD_EVENING ? "EVENING" : "NIGHT");

    for(int i = 0; i < 6; i++) digit_slot_set(clk, &clk->slots[i], vm->digits[i], anim);

    lv_label_set_text(clk->lbl_weekday, vm->weekday);

    snprintf(buf, sizeof(buf), "%.1f", vm->temp);
    lv_label_set_text(clk->lbl_temp_val, buf);
    snprintf(buf, sizeof(buf), "%d", (int)vm->humidity);
    lv_label_set_text(clk->lbl_humid_val, buf);
    /* 右对齐湿度数值到 % 左侧(文字宽度变化后重排) */
    lv_obj_align(clk->lbl_humid_val, LV_ALIGN_RIGHT_MID,
                 -(clk->m->footer_pad + 22), 12);
}

static void tick_cb(lv_timer_t *timer)
{
    clock_view_t *clk = (clock_view_t *)lv_timer_get_user_data(timer);
    clock_view_model_t vm;
    clock_model_build(&vm);
    apply_model(clk, &vm, true);
    lv_timer_set_period(timer, clock_model_delay_ms_to_next_second());
}

/* -------- 公开接口 -------- */
clock_view_t *clock_view_create(lv_obj_t *parent)
{
    clock_view_t *clk = lv_malloc_zeroed(sizeof(*clk));
    if(!clk) return NULL;
    clk->m = C();

    const clock_metrics_t *m = clk->m;

    lv_obj_t *root = lv_obj_create(parent);
    /* host/page 已全屏;时钟直接撑满父容器(= 整屏) */
    lv_obj_set_size(root, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_pad_row(root, 0, 0);   /* 去掉 header/main/footer 之间黑缝 */
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_radius(root, 0, 0);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    clk->root = root;

    /* ---- header ---- */
    lv_obj_t *header = lv_obj_create(root);
    lv_obj_set_size(header, lv_pct(100), m->header_h);
    lv_obj_set_flex_grow(header, 0);
    grad3(header, m->grads.grad_top[0], m->grads.grad_top[1], m->grads.grad_top[2]);
    lv_obj_set_style_pad_all(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    clk->header = header;

    clk->lbl_year = lv_label_create(header);
    lv_obj_set_style_text_font(clk->lbl_year, m->fonts.f_year, 0);
    lv_obj_set_style_text_color(clk->lbl_year, m->col_ink, 0);
    lv_obj_align(clk->lbl_year, LV_ALIGN_LEFT_MID, 34, 0);

    clk->lbl_month = lv_label_create(header);
    lv_obj_set_style_text_font(clk->lbl_month, m->fonts.f_month, 0);
    lv_obj_set_style_text_color(clk->lbl_month, m->col_ink, 0);
    lv_obj_center(clk->lbl_month);

    clk->lbl_day = lv_label_create(header);
    lv_obj_set_style_text_font(clk->lbl_day, m->fonts.f_day, 0);
    lv_obj_set_style_text_color(clk->lbl_day, m->col_ink, 0);
    lv_obj_align(clk->lbl_day, LV_ALIGN_RIGHT_MID, -34, 0);

    /* ---- main ---- */
    lv_obj_t *main = lv_obj_create(root);
    lv_obj_set_width(main, lv_pct(100));
    lv_obj_set_flex_grow(main, 1);
    grad3(main, m->grads.radial[0], m->grads.radial[1], m->grads.radial[2]);
    lv_obj_set_style_pad_all(main, 0, 0);
    lv_obj_set_style_border_width(main, 0, 0);
    lv_obj_set_style_radius(main, 0, 0);
    lv_obj_clear_flag(main, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    clk->main = main;

    clk->lbl_period = lv_label_create(main);
    lv_obj_set_style_text_font(clk->lbl_period, m->fonts.f_period, 0);
    lv_obj_set_style_text_color(clk->lbl_period, m->col_digit, 0);
    lv_obj_set_style_text_letter_space(clk->lbl_period, 9, 0); /* ~0.34em @28px */
    lv_obj_align(clk->lbl_period, LV_ALIGN_TOP_MID, 0, m->period_top);

    /* 时钟行 */
    lv_obj_t *row = lv_obj_create(main);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_SIZE_CONTENT, m->slot_h);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_column(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_align(row, LV_ALIGN_CENTER, 0, m->digit_offset_y);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    clk->clock_row = row;

    const int order[8] = {0, 1, -1, 2, 3, -2, 4, 5}; /* -1/-2 = 冒号 */
    for(int i = 0; i < 8; i++) {
        if(order[i] < 0) colon_create(clk, row, &clk->colons[(-order[i]) - 1]);
        else digit_slot_create(clk, row, &clk->slots[order[i]]);
    }

    /* 装饰闹钟:时钟中心右偏 alarm_off_x,主区顶 alarm_top */
    lv_obj_t *alarm = lv_image_create(main);
    lv_image_set_src(alarm, &img_clock_alarm);
    lv_obj_set_style_image_recolor(alarm, m->col_digit, 0);
    lv_obj_set_style_image_recolor_opa(alarm, LV_OPA_COVER, 0);
    lv_obj_set_size(alarm, m->alarm_size, m->alarm_size);
    lv_obj_align(alarm, LV_ALIGN_TOP_MID, m->alarm_off_x, m->alarm_top);
    lv_obj_clear_flag(alarm, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    clk->img_alarm = alarm;

    /* ---- footer:绝对定位三列,避免嵌套 flex 塌缩/裁切 ---- */
    lv_obj_t *footer = lv_obj_create(root);
    lv_obj_set_size(footer, lv_pct(100), m->footer_h);
    lv_obj_set_flex_grow(footer, 0);
    grad3(footer, m->grads.grad_bottom[0], m->grads.grad_bottom[1], m->grads.grad_bottom[2]);
    lv_obj_set_style_pad_all(footer, 0, 0);
    lv_obj_set_style_border_width(footer, 0, 0);
    lv_obj_set_style_radius(footer, 0, 0);
    lv_obj_clear_flag(footer, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    clk->footer = footer;

    /* 温度(左): TEMP  °C / 🌡 25.6 */
    lv_obj_t *tlbl = lv_label_create(footer);
    lv_obj_set_style_text_font(tlbl, m->fonts.f_metric_lbl, 0);
    lv_obj_set_style_text_color(tlbl, m->col_temp_lbl, 0);
    lv_obj_set_style_text_opa(tlbl, 128, 0);
    lv_label_set_text(tlbl, "TEMP");
    lv_obj_align(tlbl, LV_ALIGN_LEFT_MID, m->footer_pad, -14);

    lv_obj_t *tunit = lv_label_create(footer);
    lv_obj_set_style_text_font(tunit, m->fonts.f_unit, 0);
    lv_obj_set_style_text_color(tunit, m->col_ink, 0);
    lv_label_set_text(tunit, "°C");
    lv_obj_align(tunit, LV_ALIGN_LEFT_MID, m->footer_pad + 70, -14);

    lv_obj_t *ticon = lv_image_create(footer);
    lv_image_set_src(ticon, &img_clock_thermometer);
    lv_obj_set_style_image_recolor(ticon, m->col_temp_lbl, 0);
    lv_obj_set_style_image_recolor_opa(ticon, 158, 0);
    lv_obj_set_size(ticon, m->metric_icon_w, m->metric_icon_h);
    lv_obj_clear_flag(ticon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(ticon, LV_ALIGN_LEFT_MID, m->footer_pad, 12);

    clk->lbl_temp_val = lv_label_create(footer);
    lv_obj_set_style_text_font(clk->lbl_temp_val, m->fonts.f_metric_val, 0);
    lv_obj_set_style_text_color(clk->lbl_temp_val, m->col_temp, 0);
    lv_obj_align(clk->lbl_temp_val, LV_ALIGN_LEFT_MID,
                 m->footer_pad + m->metric_icon_w + m->metric_val_gap, 12);

    /* 星期(中) */
    clk->lbl_weekday = lv_label_create(footer);
    lv_obj_set_style_text_font(clk->lbl_weekday, m->fonts.f_weekday, 0);
    lv_obj_set_style_text_color(clk->lbl_weekday, m->col_ink, 0);
    lv_obj_align(clk->lbl_weekday, LV_ALIGN_CENTER, 0, 0);

    /* 湿度(右): HUMIDITY ☺ / 💧 65 % */
    lv_obj_t *hlbl = lv_label_create(footer);
    lv_obj_set_style_text_font(hlbl, m->fonts.f_metric_lbl, 0);
    lv_obj_set_style_text_color(hlbl, m->col_ink, 0);
    lv_label_set_text(hlbl, "HUMIDITY");
    lv_obj_align(hlbl, LV_ALIGN_RIGHT_MID, -(m->footer_pad + m->smile_size + 20), -14);

    lv_obj_t *smile = lv_image_create(footer);
    lv_image_set_src(smile, &img_clock_smile);
    lv_obj_set_style_image_recolor(smile, m->col_ink, 0);
    lv_obj_set_style_image_recolor_opa(smile, LV_OPA_COVER, 0);
    lv_obj_set_size(smile, m->smile_size, m->smile_size);
    lv_obj_clear_flag(smile, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(smile, LV_ALIGN_RIGHT_MID, -m->footer_pad, -14);

    lv_obj_t *hunit = lv_label_create(footer);
    lv_obj_set_style_text_font(hunit, m->fonts.f_unit, 0);
    lv_obj_set_style_text_color(hunit, m->col_ink, 0);
    lv_label_set_text(hunit, "%");
    lv_obj_align(hunit, LV_ALIGN_RIGHT_MID, -m->footer_pad, 12);

    clk->lbl_humid_val = lv_label_create(footer);
    lv_obj_set_style_text_font(clk->lbl_humid_val, m->fonts.f_metric_val, 0);
    lv_obj_set_style_text_color(clk->lbl_humid_val, m->col_ink, 0);
    /* 数值右对齐到 % 左侧,apply_model 后微调不够,先用固定偏移 */
    lv_obj_align(clk->lbl_humid_val, LV_ALIGN_RIGHT_MID, -(m->footer_pad + 22), 12);

    lv_obj_t *dicon = lv_image_create(footer);
    lv_image_set_src(dicon, &img_clock_drop);
    lv_obj_set_style_image_recolor(dicon, m->col_ink, 0);
    lv_obj_set_style_image_recolor_opa(dicon, LV_OPA_COVER, 0);
    lv_obj_set_size(dicon, m->metric_icon_w, m->metric_icon_h);
    lv_obj_clear_flag(dicon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(dicon, LV_ALIGN_RIGHT_MID,
                 -(m->footer_pad + 22 + 50 + m->metric_val_gap), 12);

    /* 首帧静态 + 启动走时 */
    clock_view_model_t vm;
    clock_model_build(&vm);
    apply_model(clk, &vm, false);

    clk->tick = lv_timer_create(tick_cb, clock_model_delay_ms_to_next_second(), clk);

    return clk;
}

void clock_view_destroy(clock_view_t *clk)
{
    if(!clk) return;
    if(clk->tick) lv_timer_delete(clk->tick);
    if(clk->root) lv_obj_delete(clk->root);
    lv_free(clk);
}

void clock_view_update_static(clock_view_t *clk)
{
    if(!clk) return;
    clock_view_model_t vm;
    clock_model_build(&vm);
    apply_model(clk, &vm, false);
    if(clk->tick) lv_timer_set_period(clk->tick, clock_model_delay_ms_to_next_second());
}
