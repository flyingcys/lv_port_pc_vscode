#ifndef DESIGN_FRUIT_MODEL_H
#define DESIGN_FRUIT_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DESIGN_FRUIT_ROWS 8
#define DESIGN_FRUIT_COLS 8
#define DESIGN_FRUIT_TYPES 7
#define DESIGN_FRUIT_MAX_ROUNDS 8
#define DESIGN_FRUIT_MAX_MOVES (DESIGN_FRUIT_ROWS * DESIGN_FRUIT_COLS)
#define DESIGN_FRUIT_MAX_SPAWNS (DESIGN_FRUIT_ROWS * DESIGN_FRUIT_COLS)

typedef struct {
    uint8_t board[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];
    uint32_t score;
    uint32_t rng;
} design_fruit_model_t;

typedef struct {
    uint8_t from_row;
    uint8_t from_col;
    uint8_t to_row;
    uint8_t to_col;
    uint8_t value;
} design_fruit_move_t;

typedef struct {
    uint8_t row;
    uint8_t col;
    uint8_t value;
    uint8_t drop_cells;
} design_fruit_spawn_t;

typedef struct {
    uint8_t board_before[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];
    uint8_t board_after[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];
    bool marks[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];
    uint8_t match_count;
    uint16_t move_count;
    uint16_t spawn_count;
    uint32_t score_delta;
    design_fruit_move_t moves[DESIGN_FRUIT_MAX_MOVES];
    design_fruit_spawn_t spawns[DESIGN_FRUIT_MAX_SPAWNS];
} design_fruit_round_plan_t;

typedef struct {
    bool accepted;
    uint8_t from_row;
    uint8_t from_col;
    uint8_t to_row;
    uint8_t to_col;
    uint8_t round_count;
    uint32_t score_delta;
    uint8_t final_board[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];
    design_fruit_round_plan_t rounds[DESIGN_FRUIT_MAX_ROUNDS];
} design_fruit_swap_plan_t;

void design_fruit_model_init(design_fruit_model_t *model, uint32_t seed);
uint8_t design_fruit_model_rows(const design_fruit_model_t *model);
uint8_t design_fruit_model_cols(const design_fruit_model_t *model);
uint8_t design_fruit_model_cell(const design_fruit_model_t *model, uint8_t row, uint8_t col);
uint32_t design_fruit_model_score(const design_fruit_model_t *model);
bool design_fruit_model_has_matches(const design_fruit_model_t *model);
/* 是否还存在任意一步合法交换能够形成消除（用于游戏结束判定）。*/
bool design_fruit_model_has_available_move(const design_fruit_model_t *model);
void design_fruit_model_set_board(design_fruit_model_t *model,
                                  const uint8_t board[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS]);
bool design_fruit_model_swap(design_fruit_model_t *model,
                             uint8_t row_a,
                             uint8_t col_a,
                             uint8_t row_b,
                             uint8_t col_b);
bool design_fruit_model_swap_with_plan(design_fruit_model_t *model,
                                       uint8_t row_a,
                                       uint8_t col_a,
                                       uint8_t row_b,
                                       uint8_t col_b,
                                       design_fruit_swap_plan_t *plan);

#ifdef __cplusplus
}
#endif

#endif /* DESIGN_FRUIT_MODEL_H */
