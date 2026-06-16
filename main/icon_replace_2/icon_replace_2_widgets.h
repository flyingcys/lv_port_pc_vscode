#ifndef ICON_REPLACE_2_WIDGETS_H
#define ICON_REPLACE_2_WIDGETS_H
#include "lvgl.h"
#include "icon_replace_2_data.h"
#ifdef __cplusplus
extern "C" {
#endif
/* 在 parent 内创建一个 app 瓷砖（圆角图标 + 中文名），返回根容器（可点击/可拖拽） */
lv_obj_t * ir2_widget_app_tile(lv_obj_t * parent, const ir2_app_t * app);
/* 非交互装饰对象去掉点击与滚动 */
void ir2_make_decorative(lv_obj_t * obj);

lv_obj_t * ir2_widget_glass_panel(lv_obj_t * parent, int32_t w, int32_t h); /* 毛玻璃近似 */
lv_obj_t * ir2_widget_toggle(lv_obj_t * parent, const char * glyph, bool on);/* 控制中心圆钮 */
lv_obj_t * ir2_widget_slider(lv_obj_t * parent, const char * glyph, int32_t val);
lv_obj_t * ir2_widget_dots(lv_obj_t * parent, uint32_t count, uint32_t active);
void       ir2_widget_dots_set_active(lv_obj_t * dots, uint32_t active);
#ifdef __cplusplus
}
#endif
#endif
