#ifndef DESKTOP_PANELS_GEOM_H
#define DESKTOP_PANELS_GEOM_H
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#define DESKTOP_PANEL_CONTROL 1
#define DESKTOP_PANEL_NOTIFY  2

/* 末段移动 >= 此值视为甩动，按方向兜底吸附（露出增量像素） */
#define DESKTOP_PANEL_FLICK_PX 12

/* 由「从全关起算的露出像素 reveal」算出面板 y（已钳制到 [全关, 全开]）。
 * 前置条件：panel_h > 0。
 *   which : DESKTOP_PANEL_CONTROL 或 DESKTOP_PANEL_NOTIFY（其它值按 NOTIFY 处理）
 *   control: 全关 y=-panel_h, 全开 y=0                  （reveal 下滑为正）
 *   notify : 全关 y=screen_h, 全开 y=screen_h-panel_h   （reveal 上滑为正）
 */
int32_t desktop_panel_drag_y(int which, int32_t reveal, int32_t panel_h, int32_t screen_h);

/* 松手吸附：返回 true=吸附到全开。
 *   末段甩动 |last_delta|>=DESKTOP_PANEL_FLICK_PX 按方向兜底（last_delta 朝开为正）；
 *   否则按露出比例：reveal >= 50% panel_h 则开。panel_h<=0 视为关。
 */
bool desktop_panel_snap_open(int32_t reveal, int32_t panel_h, int32_t last_delta);

#ifdef __cplusplus
}
#endif
#endif
