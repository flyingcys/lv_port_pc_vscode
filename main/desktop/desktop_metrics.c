#include "desktop_metrics.h"

LV_FONT_DECLARE(desktop_font_simsun_16); LV_FONT_DECLARE(desktop_font_simsun_12);
LV_FONT_DECLARE(desktop_font_phosphor_20); LV_FONT_DECLARE(desktop_font_phosphor_14);

typedef enum { DESKTOP_RES_800, DESKTOP_RES_640, DESKTOP_RES_480, DESKTOP_RES_NUM } desktop_res_t;
static desktop_res_t s_res = DESKTOP_RES_800;

/* 文件级 static —— 表本身长期存活，getter 返回其指针 */
static const desktop_metrics_t s_metrics[DESKTOP_RES_NUM] = {
    /* 800x480 */ { 800,480, 40,24, 96,20, 408,
                    &desktop_font_simsun_16, &lv_font_montserrat_18,
                    &lv_font_montserrat_48, &desktop_font_phosphor_20 },
    /* 640x480 */ { 640,480, 40,24, 88,20, 408,
                    &desktop_font_simsun_16, &lv_font_montserrat_18,
                    &lv_font_montserrat_48, &desktop_font_phosphor_20 },
    /* 480x272 */ { 480,272, 28,16, 60,14, 231,
                    &desktop_font_simsun_12, &lv_font_montserrat_14,
                    &lv_font_montserrat_40, &desktop_font_phosphor_14 },
};

void desktop_set_resolution(int32_t w, int32_t h)
{
    if(w == 640 && h == 480)      s_res = DESKTOP_RES_640;
    else if(w == 480 && h == 272) s_res = DESKTOP_RES_480;
    else                          s_res = DESKTOP_RES_800;
}

const desktop_metrics_t * desktop_metrics(void) { return &s_metrics[s_res]; }
int32_t desktop_grid_col_w(void) { return desktop_metrics()->screen_w / DESKTOP_GRID_COLS; }
int32_t desktop_grid_row_h(void) {
    const desktop_metrics_t * m = desktop_metrics();
    return (m->screen_h - m->top_bar_h - m->pager_h) / DESKTOP_GRID_ROWS;
}
