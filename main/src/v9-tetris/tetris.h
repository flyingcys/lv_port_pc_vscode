#ifndef TETRIS_H
#define TETRIS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"

lv_obj_t * tetris_create(lv_obj_t *parent, int32_t screen_w, int32_t screen_h);
void tetris_start(void);
void tetris_stop(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TETRIS_H */
