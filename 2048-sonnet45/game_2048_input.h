/**
 * @file game_2048_input.h
 * @brief 2048 Game Input Handler Header
 */

#ifndef GAME_2048_INPUT_H
#define GAME_2048_INPUT_H

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
 * Initialize input handlers for the game
 * Sets up keyboard and touch/mouse gesture detection
 * @param container The main container object to attach input handlers to
 */
void game_2048_input_init(lv_obj_t * container);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* GAME_2048_INPUT_H */
