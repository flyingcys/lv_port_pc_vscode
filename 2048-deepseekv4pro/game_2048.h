// game_2048.h - 2048 game engine (pure ANSI C, no LVGL dependency)
#ifndef GAME_2048_H
#define GAME_2048_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} game_dir_t;

typedef enum {
    STATE_PLAYING,
    STATE_WON,
    STATE_LOST
} game_state_t;

typedef struct {
    uint16_t cells[4][4];   /* 0 = empty, otherwise power-of-two value */
    uint32_t score;
    uint32_t best_score;
    game_state_t state;
    bool moved;             /* true if last move changed the board */
    bool merged[4][4];      /* true for cells that were just merged */
} game_2048_t;

void game_2048_init(game_2048_t *game);
void game_2048_move(game_2048_t *game, game_dir_t dir);
void game_2048_reset(game_2048_t *game);

#ifdef __cplusplus
}
#endif

#endif /* GAME_2048_H */
