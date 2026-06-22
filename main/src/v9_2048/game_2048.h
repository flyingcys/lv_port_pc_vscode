#ifndef V9_2048_H
#define V9_2048_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * game_2048_create(lv_obj_t *parent, int32_t screen_w, int32_t screen_h);
void game_2048_start(void);
void game_2048_stop(void);
void game_2048_set_grid_size(uint8_t size);
uint8_t game_2048_get_grid_size(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* V9_2048_H */
