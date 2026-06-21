#ifndef GAME_2048_H
#define GAME_2048_H

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
 * Initialize the 2048 game
 */
void game_2048_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GAME_2048_H*/
