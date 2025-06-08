#ifndef GAME_2048_IMPROVED_H
#define GAME_2048_IMPROVED_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 创建改进版2048游戏
 * @param parent 父容器对象
 */
void game_2048_improved(lv_obj_t *parent);

/**
 * 清理游戏资源
 */
void game_2048_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_2048_IMPROVED_H 