/**
 * @file lv_game_2048.h
 *
 */

#ifndef LV_GAME_2048_H
#define LV_GAME_2048_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../../lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define MATRIX_SIZE 4

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_WON,
    GAME_STATE_LOST
} game_state_t;

typedef struct {
    int board[MATRIX_SIZE][MATRIX_SIZE];
    int score;
    game_state_t state;
    lv_obj_t * ui_root;
    lv_obj_t * score_label;
    lv_obj_t * tile_objs[MATRIX_SIZE][MATRIX_SIZE];
} game_2048_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Start the 2048 game
 */
void lv_game_2048_start(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_GAME_2048_H*/
