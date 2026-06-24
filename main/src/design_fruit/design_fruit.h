#ifndef DESIGN_FRUIT_H
#define DESIGN_FRUIT_H

#include <stdbool.h>
#include <stdint.h>

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t *design_fruit_create(lv_obj_t *parent, int32_t screen_w, int32_t screen_h);
void design_fruit_start(void);
void design_fruit_stop(void);

/* 测试 / 无头快照演示用钩子。board8x8 指向 64 个 uint8_t（行优先，值 1..7）。*/
void design_fruit_test_set_board(const uint8_t *board8x8);
void design_fruit_test_swap(uint8_t row_a, uint8_t col_a, uint8_t row_b, uint8_t col_b);
bool design_fruit_test_is_animating(void);
void design_fruit_test_show_gameover(void);

#ifdef __cplusplus
}
#endif

#endif /* DESIGN_FRUIT_H */
