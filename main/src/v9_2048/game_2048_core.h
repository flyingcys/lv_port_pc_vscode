#ifndef V9_2048_CORE_H
#define V9_2048_CORE_H

#include <stdbool.h>
#include <stdint.h>

#define GAME_2048_MIN_SIZE 4
#define GAME_2048_MAX_SIZE 6
#define GAME_2048_TARGET 2048U

typedef enum {
    GAME_2048_DIR_LEFT = 0,
    GAME_2048_DIR_RIGHT,
    GAME_2048_DIR_UP,
    GAME_2048_DIR_DOWN
} game_2048_dir_t;

typedef enum {
    GAME_2048_STATUS_PLAYING = 0,
    GAME_2048_STATUS_WON,
    GAME_2048_STATUS_OVER
} game_2048_status_t;

typedef struct {
    uint8_t from_row;
    uint8_t from_col;
    uint8_t to_row;
    uint8_t to_col;
    uint32_t value_from;
    uint32_t value_to;
    bool merged;
    bool disappear;
} game_2048_move_step_t;

typedef struct {
    game_2048_move_step_t steps[GAME_2048_MAX_SIZE * GAME_2048_MAX_SIZE];
    uint8_t step_count;
    uint8_t merge_count;
    bool has_new_tile;
    uint8_t new_tile_row;
    uint8_t new_tile_col;
    uint32_t new_tile_value;
} game_2048_move_result_t;

typedef struct {
    uint8_t size;
    uint32_t cells[GAME_2048_MAX_SIZE][GAME_2048_MAX_SIZE];
    uint32_t score;
    uint32_t best_score;
    uint32_t max_tile;
    uint32_t rng_state;
    bool won;
    bool over;
} game_2048_core_t;

bool game_2048_core_init(game_2048_core_t *game, uint8_t size, uint32_t seed);
void game_2048_core_clear(game_2048_core_t *game);
bool game_2048_core_set_size(game_2048_core_t *game, uint8_t size, uint32_t seed);
bool game_2048_core_set_cell(game_2048_core_t *game, uint8_t row, uint8_t col, uint32_t value);
uint32_t game_2048_core_cell(const game_2048_core_t *game, uint8_t row, uint8_t col);
bool game_2048_core_move(game_2048_core_t *game, game_2048_dir_t dir, game_2048_move_result_t *result);
game_2048_status_t game_2048_core_status(const game_2048_core_t *game);

#endif /* V9_2048_CORE_H */
