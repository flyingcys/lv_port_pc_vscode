#ifndef TETRIS_H
#define TETRIS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"

#define GRID_WIDTH 10
#define GRID_HEIGHT 20
#define TILE_SIZE 15

typedef enum {
    GAME_STATE_START,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_OVER
} tetris_state_t;

void tetris_init(void);

#ifdef __cplusplus
}
#endif

#endif /*TETRIS_H*/
