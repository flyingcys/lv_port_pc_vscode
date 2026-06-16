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
#ifdef __cplusplus
}
#endif
#endif
