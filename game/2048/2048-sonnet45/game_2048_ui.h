/**
 * @file game_2048_ui.h
 * @brief 2048 Game UI Header
 */

#ifndef GAME_2048_UI_H
#define GAME_2048_UI_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the 2048 game UI
 * Creates all UI elements (grid, tiles, score display, buttons)
 */
void game_2048_ui_init(void);

/**
 * Update the UI to reflect current game state
 * Called after each move or game state change
 */
void game_2048_ui_update(void);

/**
 * Get the main game container object
 * @return Pointer to the main container
 */
lv_obj_t* game_2048_ui_get_container(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* GAME_2048_UI_H */
