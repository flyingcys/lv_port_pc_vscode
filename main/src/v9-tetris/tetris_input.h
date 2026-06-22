/**
 * @file tetris_input.h
 * @brief Tetris input handling header - Keyboard controls
 */

#ifndef TETRIS_INPUT_H
#define TETRIS_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "tetris_game.h"
#include "tetris_ui.h"
#include "lvgl/lvgl.h"

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Initialize input handling
 * @param game Pointer to game structure
 * @param ui Pointer to UI structure
 * @param target Object that receives keyboard focus and events
 */
void tetris_input_init(tetris_game_t *game, tetris_ui_t *ui, lv_obj_t *target);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TETRIS_INPUT_H */
