#ifndef ICON_REPLACE_2_PANELS_H
#define ICON_REPLACE_2_PANELS_H
#include "lvgl.h"
#include "icon_replace_2_panels_geom.h"  /* IR2_PANEL_CONTROL / IR2_PANEL_NOTIFY 供调用方使用 */
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct icon_replace_2_panels icon_replace_2_panels_t;
icon_replace_2_panels_t * ir2_panels_create(lv_obj_t * parent);
void ir2_panels_destroy(icon_replace_2_panels_t * p);

/* 直接吸附到全开/全关（带动画）。截图钩子/程序化调用用。 */
void ir2_panels_show_control(icon_replace_2_panels_t * p, bool show);
void ir2_panels_show_notify(icon_replace_2_panels_t * p, bool show);

/* 截图钩子按 AM_PANEL 设初始全开态（无动画）："control" / "notify" / 其它=none。 */
void ir2_panels_apply_initial(icon_replace_2_panels_t * p, const char * which);

/* 跟手拖拽（由屏幕边缘感应条驱动）。
 *   which: IR2_PANEL_CONTROL / IR2_PANEL_NOTIFY
 *   reveal: 从全关起算的露出像素（control 下滑为正、notify 上滑为正）
 *   begin 从全关起拖；update 实时跟手；end 松手按位置/甩动吸附。
 */
void ir2_panels_drag_begin(icon_replace_2_panels_t * p, int which);
void ir2_panels_drag_update(icon_replace_2_panels_t * p, int which, int32_t reveal);
void ir2_panels_drag_end(icon_replace_2_panels_t * p, int which);

/* 当前展开态：0=无 1=控制中心 2=通知中心。供边缘条门控（展开时忽略边缘按下）。 */
int  ir2_panels_active(icon_replace_2_panels_t * p);

/* 把两个面板置于边缘感应带之上：全屏展开时把手才点得到。
 * 需在边缘感应带创建完成后调用一次。 */
void ir2_panels_bring_to_front(icon_replace_2_panels_t * p);

#ifdef __cplusplus
}
#endif
#endif
