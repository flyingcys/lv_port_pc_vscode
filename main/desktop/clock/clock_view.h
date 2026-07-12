#ifndef CLOCK_VIEW_H
#define CLOCK_VIEW_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct clock_view clock_view_t;

/* 在 desktop page_0 容器上创建时钟首页,撑满父容器 */
clock_view_t *clock_view_create(lv_obj_t *parent);
void           clock_view_destroy(clock_view_t *clk);

/* 立即用当前时间刷新(无滚动,用于首帧/可见性恢复/冻结) */
void clock_view_update_static(clock_view_t *clk);

#ifdef __cplusplus
}
#endif

#endif
