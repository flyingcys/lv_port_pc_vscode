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
 */
void tetris_input_init(tetris_game_t *game, tetris_ui_t *ui);

/**
 * @brief Keyboard event handler
 * @param indev Input device
 * @param data Input data
 */
void tetris_input_read(lv_indev_t *indev, lv_indev_data_t *data);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TETRIS_INPUT_H */
