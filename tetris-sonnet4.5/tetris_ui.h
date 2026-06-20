/**
 * @file tetris_ui.h
 * @brief Tetris UI layer header - LVGL v9 interface components
 */

#ifndef TETRIS_UI_H
#define TETRIS_UI_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "tetris_game.h"
#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define CELL_SIZE       20  /* Size of each cell in pixels */

/**********************
 *      TYPEDEFS
 **********************/

/**
 * @brief UI structure containing all LVGL objects
 */
typedef struct {
    lv_obj_t *main_container;       /* Main container */
    lv_obj_t *game_canvas;          /* Game board canvas */
    lv_obj_t *next_canvas;          /* Next piece preview canvas */
    lv_obj_t *score_label;          /* Score display */
    lv_obj_t *lines_label;          /* Lines cleared display */
    lv_obj_t *level_label;          /* Level display */
    lv_obj_t *start_btn;            /* Start button */
    lv_obj_t *pause_btn;            /* Pause button */
    lv_obj_t *reset_btn;            /* Reset button */
    lv_obj_t *game_over_msgbox;     /* Game over dialog */
    lv_draw_buf_t *game_draw_buf;   /* Draw buffer for game canvas */
    lv_draw_buf_t *next_draw_buf;   /* Draw buffer for next piece canvas */
    tetris_game_t *game;            /* Pointer to game instance */
} tetris_ui_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Initialize the Tetris UI
 * @param ui Pointer to UI structure
 * @param game Pointer to game structure
 */
void tetris_ui_init(tetris_ui_t *ui, tetris_game_t *game);

/**
 * @brief Update the UI to reflect current game state
 * @param ui Pointer to UI structure
 */
void tetris_ui_update(tetris_ui_t *ui);

/**
 * @brief Clean up UI resources
 * @param ui Pointer to UI structure
 */
void tetris_ui_cleanup(tetris_ui_t *ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TETRIS_UI_H */
