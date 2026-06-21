#ifndef CALC_H
#define CALC_H
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
lv_obj_t * calc_create(lv_obj_t * parent, int32_t screen_w, int32_t screen_h);
typedef void (*calc_close_cb)(void);
void calc_set_close_cb(lv_obj_t * card, calc_close_cb cb);
#ifdef __cplusplus
}
#endif
#endif
