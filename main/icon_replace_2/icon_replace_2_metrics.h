#ifndef ICON_REPLACE_2_METRICS_H
#define ICON_REPLACE_2_METRICS_H
#include "lvgl.h"
#include <stdint.h>
#include "icon_replace_2_layout.h"   /* 纯尺寸常量：IR2_PAGE_COUNT / 网格维度（lvgl-free） */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t screen_w, screen_h;
    int32_t top_bar_h;
    int32_t pager_h;            /* 分页点区域高 */
    int32_t icon_size;          /* app 图标边长 */
    int32_t icon_radius;
    int32_t panel_h;            /* 面板高（≈85%） */
    const lv_font_t * font_label;     /* SimSun: app 名/面板标题/通知 */
    const lv_font_t * font_clock;     /* Montserrat: 状态栏时钟 */
    const lv_font_t * font_big_clock; /* Montserrat: 锁屏大时钟 */
    const lv_font_t * font_glyph;     /* Phosphor: 状态栏/控制中心图标 */
} ir2_metrics_t;

/* 由 AM_RES（经 icon_replace_2_set_resolution）选择当前档 */
const ir2_metrics_t * ir2_metrics(void);
void icon_replace_2_set_resolution(int32_t w, int32_t h);

/* 网格坐标辅助 */
int32_t ir2_grid_col_w(void);   /* screen_w / IR2_GRID_COLS */
int32_t ir2_grid_row_h(void);   /* (screen_h - top_bar_h - pager_h) / IR2_GRID_ROWS */

#ifdef __cplusplus
}
#endif
#endif
