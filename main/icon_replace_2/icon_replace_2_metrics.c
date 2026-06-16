#include "icon_replace_2_metrics.h"

LV_FONT_DECLARE(ir2_simsun_16); LV_FONT_DECLARE(ir2_simsun_12);
LV_FONT_DECLARE(ir2_phosphor_20); LV_FONT_DECLARE(ir2_phosphor_14);

typedef enum { IR2_RES_800, IR2_RES_640, IR2_RES_480, IR2_RES_NUM } ir2_res_t;
static ir2_res_t s_res = IR2_RES_800;

/* 文件级 static —— 表本身长期存活，getter 返回其指针 */
static const ir2_metrics_t s_metrics[IR2_RES_NUM] = {
    /* 800x480 */ { 800,480, 40,24, 96,20, 408,
                    &ir2_simsun_16, &lv_font_montserrat_18,
                    &lv_font_montserrat_48, &ir2_phosphor_20 },
    /* 640x480 */ { 640,480, 40,24, 88,20, 408,
                    &ir2_simsun_16, &lv_font_montserrat_18,
                    &lv_font_montserrat_48, &ir2_phosphor_20 },
    /* 480x272 */ { 480,272, 28,16, 60,14, 231,
                    &ir2_simsun_12, &lv_font_montserrat_14,
                    &lv_font_montserrat_40, &ir2_phosphor_14 },
};

void icon_replace_2_set_resolution(int32_t w, int32_t h)
{
    if(w == 640 && h == 480)      s_res = IR2_RES_640;
    else if(w == 480 && h == 272) s_res = IR2_RES_480;
    else                          s_res = IR2_RES_800;
}

const ir2_metrics_t * ir2_metrics(void) { return &s_metrics[s_res]; }
int32_t ir2_grid_col_w(void) { return ir2_metrics()->screen_w / IR2_GRID_COLS; }
int32_t ir2_grid_row_h(void) {
    const ir2_metrics_t * m = ir2_metrics();
    return (m->screen_h - m->top_bar_h - m->pager_h) / IR2_GRID_ROWS;
}
