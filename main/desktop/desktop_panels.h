#ifndef DESKTOP_PANELS_H
#define DESKTOP_PANELS_H
#include "lvgl.h"
#include "desktop_panels_geom.h"  /* DESKTOP_PANEL_CONTROL / DESKTOP_PANEL_NOTIFY 供调用方使用 */
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct desktop_panels desktop_panels_t;
desktop_panels_t * desktop_panels_create(lv_obj_t * parent);
void desktop_panels_destroy(desktop_panels_t * p);

/* 直接吸附到全开/全关（带动画）。截图钩子/程序化调用用。 */
void desktop_panels_show_control(desktop_panels_t * p, bool show);
void desktop_panels_show_notify(desktop_panels_t * p, bool show);

/* 截图钩子按 AM_PANEL 设初始全开态（无动画）："control" / "notify" / 其它=none。 */
void desktop_panels_apply_initial(desktop_panels_t * p, const char * which);

/* 当前展开态：0=无 1=控制中心 2=通知中心。供边缘条门控（展开时忽略边缘按下）。 */
int  desktop_panels_active(desktop_panels_t * p);

/* 把两个面板置于边缘感应带之上：全屏展开时把手才点得到。
 * 需在边缘感应带创建完成后调用一次。 */
void desktop_panels_bring_to_front(desktop_panels_t * p);

#ifdef __cplusplus
}
#endif
#endif
