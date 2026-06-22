#ifndef DESIGN_FRUIT_H
#define DESIGN_FRUIT_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t *design_fruit_create(lv_obj_t *parent, int32_t screen_w, int32_t screen_h);
void design_fruit_start(void);
void design_fruit_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* DESIGN_FRUIT_H */
