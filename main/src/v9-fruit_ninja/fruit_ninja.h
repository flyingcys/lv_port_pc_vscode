#ifndef FRUIT_NINJA_H
#define FRUIT_NINJA_H

#include "lvgl/lvgl.h"

lv_obj_t * fruit_ninja_create(lv_obj_t * parent, int32_t screen_w, int32_t screen_h);
void fruit_ninja_start(void);
void fruit_ninja_stop(void);

#endif
