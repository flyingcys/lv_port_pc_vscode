/**
 * @file game_2048.h
 * @brief 2048 Game implementation using LVGL v9
 */

#ifndef GAME_2048_H
#define GAME_2048_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"

/**
 * @brief Create the 2048 game UI
 * @param parent Parent LVGL object to place the game on (typically lv_screen_active())
 */
void game_2048_create(lv_obj_t *parent);

#ifdef __cplusplus
}
#endif

#endif /* GAME_2048_H */
