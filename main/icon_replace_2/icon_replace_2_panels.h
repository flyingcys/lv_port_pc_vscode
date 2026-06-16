#ifndef ICON_REPLACE_2_PANELS_H
#define ICON_REPLACE_2_PANELS_H
#include "lvgl.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct icon_replace_2_panels icon_replace_2_panels_t;
icon_replace_2_panels_t * ir2_panels_create(lv_obj_t * parent);
void ir2_panels_destroy(icon_replace_2_panels_t * p);
void ir2_panels_show_control(icon_replace_2_panels_t * p, bool show);
void ir2_panels_show_notify(icon_replace_2_panels_t * p, bool show);
/* 供截图钩子按 AM_PANEL 设初始态："control" / "notify" / 其它=none。直接定位(无动画)便于快照。 */
void ir2_panels_apply_initial(icon_replace_2_panels_t * p, const char * which);
#ifdef __cplusplus
}
#endif
#endif
