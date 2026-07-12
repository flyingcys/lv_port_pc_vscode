#ifndef CLOCK_METRICS_H
#define CLOCK_METRICS_H

/* ========================================================================
 * clock_metrics — 时钟首页三档中心化 metrics(800×480 / 640×480 / 480×272)
 * 单一事实源:所有结构尺寸/字号/位置一律读 C(),禁止写死数字。
 * font_role 角色,每档一套子集位图字号。
 * ======================================================================== */

#include "lvgl.h"
#include "desktop_metrics.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CLOCK_RES_800 = 0,   /* 800×480  主档,与 mockup 像素对齐 */
    CLOCK_RES_640,       /* 640×480  等比缩放 */
    CLOCK_RES_480,       /* 480×272  最小档 */
    CLOCK_RES_NUM
} clock_res_t;

/* 字号角色(语义名),代码里只写角色不写字号 */
typedef struct {
    const lv_font_t *f_digit;      /* LED calc: HH:MM:SS(主档 148) */
    const lv_font_t *f_period;     /* 时段 MORNING/EVENING 等(主档 28) */
    const lv_font_t *f_year;       /* 年 2024(主档 30) */
    const lv_font_t *f_month;      /* 月份 JANUARY(主档 35) */
    const lv_font_t *f_day;        /* 日期 30(主档 32) */
    const lv_font_t *f_weekday;    /* 星期 MONDAY(主档 33) */
    const lv_font_t *f_metric_val; /* 温度湿度数值(主档 34) */
    const lv_font_t *f_metric_lbl; /* TEMP/HUMIDITY 标签(主档 13) */
    const lv_font_t *f_unit;       /* °C / %(主档 14) */
} clock_fonts_t;

/* 渐变角色(各档色彩相同,scaling 的是尺寸) */
typedef struct {
    lv_color_t grad_top[3];     /* 顶栏青渐变 #7fdef3→#5dcae6→#46b6d4 */
    lv_color_t grad_bottom[3];  /* 底栏蓝渐变 #6fb4fd→#3a9cfc→#2a86f2 */
    lv_color_t radial[3];       /* 中部放射浅蓝 #eef6fe→#d8ecfd→#c4def9 */
} clock_grads_t;

typedef struct {
    int32_t    header_h;        /* 顶栏高(主档 72) */
    int32_t    footer_h;        /* 底栏高(主档 72) */
    int32_t    slot_w;          /* 数字位宽(主档 90) */
    int32_t    slot_h;          /* 数字位高(主档 148) */
    int32_t    colon_w;         /* 冒号列宽(主档 36) */
    int32_t    colon_dot;       /* 冒号圆点尺寸(主档 15) */
    int32_t    colon_gap;       /* 冒号两点间距(主档 24) */
    int32_t    digit_offset_y;  /* 时钟整体下移给时段留白(主档 18) */
    int32_t    period_top;      /* 时段距主区顶(主档 30) */
    int32_t    alarm_size;      /* 装饰闹钟(主档 30) */
    int32_t    alarm_off_x;     /* 闹钟时钟中心偏移(主档 +173) */
    int32_t    alarm_top;       /* 闹钟距主区顶(主档 82) */
    int32_t    footer_pad;      /* 底栏左右 padding(主档 30) */
    int32_t    metric_val_gap;  /* 图标-数值 gap(主档 7) */
    int32_t    metric_icon_w;   /* 温度/水滴图标宽(主档 18) */
    int32_t    metric_icon_h;   /* 温度/水滴图标高(主档 26) */
    int32_t    smile_size;      /* 笑脸(主档 15) */
    clock_fonts_t fonts;
    clock_grads_t grads;
    lv_color_t  col_digit;      /* 时钟/时段 珊瑚红 #fb5a53 */
    lv_color_t  col_ink;        /* 日期/星期/湿度 深蓝墨 #10405a */
    lv_color_t  col_temp;       /* 温度数值 暗金 #ccc87d */
    lv_color_t  col_temp_lbl;   /* TEMP 标签 rgba(白,.5) */
} clock_metrics_t;

/* 取当前档 metrics —— 按 desktop_metrics() 屏宽就近向下取档 */
const clock_metrics_t *clock_metrics(void);
#define C()    (clock_metrics())

void clock_metrics_init(void);   /* 在 desktop_set_resolution 后由 desktop 调用一次 */

#ifdef __cplusplus
}
}

#endif
#endif
