#include "clock_metrics.h"

/* 数字区 LED calc 字体(符号化,仅 0-9) */
LV_FONT_DECLARE(desktop_font_calc_148);
LV_FONT_DECLARE(desktop_font_calc_64);
LV_FONT_DECLARE(desktop_font_calc_34);
LV_FONT_DECLARE(desktop_font_calc_20);

/* 字母标签 Montserrat-Medium 子集 */
LV_FONT_DECLARE(clock_label_font_13);
LV_FONT_DECLARE(clock_label_font_14);
LV_FONT_DECLARE(clock_label_font_28);
LV_FONT_DECLARE(clock_label_font_30);
LV_FONT_DECLARE(clock_label_font_32);
LV_FONT_DECLARE(clock_label_font_33);
LV_FONT_DECLARE(clock_label_font_34);
LV_FONT_DECLARE(clock_label_font_35);

#define GT    { LV_COLOR_MAKE(0x7f,0xde,0xf3), LV_COLOR_MAKE(0x5d,0xca,0xe6), LV_COLOR_MAKE(0x46,0xb6,0xd4) }
#define GB    { LV_COLOR_MAKE(0x6f,0xb4,0xfd), LV_COLOR_MAKE(0x3a,0x9c,0xfc), LV_COLOR_MAKE(0x2a,0x86,0xf2) }
#define GR    { LV_COLOR_MAKE(0xee,0xf6,0xfe), LV_COLOR_MAKE(0xd8,0xec,0xfd), LV_COLOR_MAKE(0xc4,0xde,0xf9) }
#define CD    LV_COLOR_MAKE(0xfb,0x5a,0x53)
#define CI    LV_COLOR_MAKE(0x10,0x40,0x5a)
#define CT    LV_COLOR_MAKE(0xcc,0xc8,0x7d)
#define CW    LV_COLOR_MAKE(0xff,0xff,0xff)   /* TEMP 标签:opa 128 在 view 写死 */

static const clock_metrics_t s_metrics[CLOCK_RES_NUM] = {
    /* -------- 800×480 主档:与 mockup 像素对齐 -------- */
    [CLOCK_RES_800] = {
        .header_h = 72, .footer_h = 72,
        .slot_w = 90, .slot_h = 148,
        .colon_w = 36, .colon_dot = 15, .colon_gap = 24,
        .digit_offset_y = 18, .period_top = 30,
        .alarm_size = 30, .alarm_off_x = 173, .alarm_top = 82,
        .footer_pad = 30, .metric_val_gap = 7,
        .metric_icon_w = 18, .metric_icon_h = 26, .smile_size = 15,
        .fonts = {
            .f_digit      = &desktop_font_calc_148,
            .f_period     = &clock_label_font_28,
            .f_year       = &clock_label_font_30,
            .f_month      = &clock_label_font_35,
            .f_day        = &clock_label_font_32,
            .f_weekday    = &clock_label_font_33,
            /* clock_label_font_34 含 0-9 与 '.'，calc_34 仅数字无法显示 25.6 */
            .f_metric_val = &clock_label_font_34,
            .f_metric_lbl = &clock_label_font_13,
            .f_unit       = &clock_label_font_14,
        },
        .grads = { .grad_top = GT, .grad_bottom = GB, .radial = GR },
        .col_digit = CD, .col_ink = CI, .col_temp = CT, .col_temp_lbl = CW,
    },

    /* -------- 640×480:高同 800,字号复用,仅屏宽不同 -------- */
    [CLOCK_RES_640] = {
        .header_h = 72, .footer_h = 72,
        .slot_w = 90, .slot_h = 148,
        .colon_w = 36, .colon_dot = 15, .colon_gap = 24,
        .digit_offset_y = 18, .period_top = 30,
        .alarm_size = 30, .alarm_off_x = 173, .alarm_top = 82,
        .footer_pad = 30, .metric_val_gap = 7,
        .metric_icon_w = 18, .metric_icon_h = 26, .smile_size = 15,
        .fonts = {
            .f_digit      = &desktop_font_calc_148,
            .f_period     = &clock_label_font_28,
            .f_year       = &clock_label_font_30,
            .f_month      = &clock_label_font_35,
            .f_day        = &clock_label_font_32,
            .f_weekday    = &clock_label_font_33,
            .f_metric_val = &clock_label_font_34,
            .f_metric_lbl = &clock_label_font_13,
            .f_unit       = &clock_label_font_14,
        },
        .grads = { .grad_top = GT, .grad_bottom = GB, .radial = GR },
        .col_digit = CD, .col_ink = CI, .col_temp = CT, .col_temp_lbl = CW,
    },

    /* -------- 480×272 紧凑档:现成较小字号压缩 -------- */
    [CLOCK_RES_480] = {
        .header_h = 36, .footer_h = 36,
        .slot_w = 54, .slot_h = 96,
        .colon_w = 20, .colon_dot = 8, .colon_gap = 12,
        .digit_offset_y = 8, .period_top = 12,
        .alarm_size = 18, .alarm_off_x = 60, .alarm_top = 30,
        .footer_pad = 12, .metric_val_gap = 4,
        .metric_icon_w = 12, .metric_icon_h = 17, .smile_size = 10,
        .fonts = {
            .f_digit      = &desktop_font_calc_64,
            .f_period     = &clock_label_font_14,
            .f_year       = &clock_label_font_14,
            .f_month      = &clock_label_font_14,
            .f_day        = &clock_label_font_14,
            .f_weekday    = &clock_label_font_14,
            .f_metric_val = &desktop_font_calc_20,
            .f_metric_lbl = &clock_label_font_13,
            .f_unit       = &clock_label_font_13,
        },
        .grads = { .grad_top = GT, .grad_bottom = GB, .radial = GR },
        .col_digit = CD, .col_ink = CI, .col_temp = CT, .col_temp_lbl = CW,
    },
};

#undef GT
#undef GB
#undef GR
#undef CD
#undef CI
#undef CT
#undef CW

static clock_res_t s_res = CLOCK_RES_800;

const clock_metrics_t *clock_metrics(void)
{
    return &s_metrics[s_res];
}

void clock_metrics_init(void)
{
    const desktop_metrics_t *dm = desktop_metrics();
    if(dm->screen_w >= 800)      s_res = CLOCK_RES_800;
    else if(dm->screen_w >= 640) s_res = CLOCK_RES_640;
    else                         s_res = CLOCK_RES_480;
}
