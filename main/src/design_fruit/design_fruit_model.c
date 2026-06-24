#include "design_fruit_model.h"

#include <stdlib.h>
#include <string.h>

static uint32_t next_rand(design_fruit_model_t *model)
{
    model->rng = model->rng * 1664525u + 1013904223u;
    return model->rng;
}

static uint8_t random_fruit(design_fruit_model_t *model)
{
    return (uint8_t)((next_rand(model) % DESIGN_FRUIT_TYPES) + 1u);
}

static bool in_bounds(uint8_t row, uint8_t col)
{
    return row < DESIGN_FRUIT_ROWS && col < DESIGN_FRUIT_COLS;
}

static bool are_adjacent(uint8_t row_a, uint8_t col_a, uint8_t row_b, uint8_t col_b)
{
    int dr = abs((int)row_a - (int)row_b);
    int dc = abs((int)col_a - (int)col_b);
    return dr + dc == 1;
}

static bool would_match_at(const design_fruit_model_t *model, uint8_t row, uint8_t col, uint8_t value)
{
    if(col >= 2u && model->board[row][col - 1u] == value && model->board[row][col - 2u] == value) {
        return true;
    }
    if(row >= 2u && model->board[row - 1u][col] == value && model->board[row - 2u][col] == value) {
        return true;
    }
    return false;
}

static uint32_t score_for_run(uint8_t count)
{
    if(count == 3u) return 300u;
    if(count == 4u) return 500u;
    return 200u * count;
}

static bool mark_matches(const design_fruit_model_t *model,
                         bool marks[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS],
                         uint32_t *score_delta)
{
    bool found = false;

    memset(marks, 0, sizeof(bool) * DESIGN_FRUIT_ROWS * DESIGN_FRUIT_COLS);
    if(score_delta != NULL) *score_delta = 0;

    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        uint8_t col = 0;
        while(col < DESIGN_FRUIT_COLS) {
            uint8_t value = model->board[row][col];
            uint8_t end = (uint8_t)(col + 1u);
            while(end < DESIGN_FRUIT_COLS && model->board[row][end] == value) end++;
            if(value != 0u && end - col >= 3u) {
                found = true;
                if(score_delta != NULL) *score_delta += score_for_run((uint8_t)(end - col));
                for(uint8_t i = col; i < end; i++) marks[row][i] = true;
            }
            col = end;
        }
    }

    for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
        uint8_t row = 0;
        while(row < DESIGN_FRUIT_ROWS) {
            uint8_t value = model->board[row][col];
            uint8_t end = (uint8_t)(row + 1u);
            while(end < DESIGN_FRUIT_ROWS && model->board[end][col] == value) end++;
            if(value != 0u && end - row >= 3u) {
                found = true;
                if(score_delta != NULL) *score_delta += score_for_run((uint8_t)(end - row));
                for(uint8_t i = row; i < end; i++) marks[i][col] = true;
            }
            row = end;
        }
    }

    return found;
}

static uint8_t count_marks(const bool marks[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS])
{
    uint8_t count = 0;
    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
            if(marks[row][col]) count++;
        }
    }
    return count;
}

static void copy_board(uint8_t dest[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS],
                       const uint8_t src[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS])
{
    memcpy(dest, src, sizeof(uint8_t) * DESIGN_FRUIT_ROWS * DESIGN_FRUIT_COLS);
}

static void collapse_and_refill(design_fruit_model_t *model,
                                const bool marks[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS],
                                design_fruit_round_plan_t *round)
{
    for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
        int write_row = DESIGN_FRUIT_ROWS - 1;
        for(int row = DESIGN_FRUIT_ROWS - 1; row >= 0; row--) {
            if(!marks[row][col]) {
                uint8_t value = model->board[row][col];
                if(round != NULL && row != write_row && round->move_count < DESIGN_FRUIT_MAX_MOVES) {
                    design_fruit_move_t *move = &round->moves[round->move_count++];
                    move->from_row = (uint8_t)row;
                    move->from_col = col;
                    move->to_row = (uint8_t)write_row;
                    move->to_col = col;
                    move->value = value;
                }
                model->board[write_row][col] = model->board[row][col];
                write_row--;
            }
        }
        uint8_t spawn_count = (uint8_t)(write_row + 1);
        while(write_row >= 0) {
            uint8_t value = random_fruit(model);
            model->board[write_row][col] = value;
            if(round != NULL && round->spawn_count < DESIGN_FRUIT_MAX_SPAWNS) {
                design_fruit_spawn_t *spawn = &round->spawns[round->spawn_count++];
                spawn->row = (uint8_t)write_row;
                spawn->col = col;
                spawn->value = value;
                spawn->drop_cells = spawn_count;
            }
            write_row--;
        }
    }
}

static bool resolve_board(design_fruit_model_t *model, uint32_t *score_delta)
{
    bool resolved_any = false;
    bool marks[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];

    if(score_delta != NULL) *score_delta = 0;

    while(true) {
        uint32_t round_score = 0;
        if(!mark_matches(model, marks, &round_score)) break;
        resolved_any = true;
        if(score_delta != NULL) *score_delta += round_score;
        collapse_and_refill(model, marks, NULL);
    }

    return resolved_any;
}

static bool resolve_board_with_plan(design_fruit_model_t *model, design_fruit_swap_plan_t *plan)
{
    bool resolved_any = false;
    bool marks[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];

    while(true) {
        uint32_t round_score = 0;

        if(!mark_matches(model, marks, &round_score)) break;

        resolved_any = true;
        plan->score_delta += round_score;
        if(plan->round_count < DESIGN_FRUIT_MAX_ROUNDS) {
            design_fruit_round_plan_t *round = &plan->rounds[plan->round_count++];
            memset(round, 0, sizeof(*round));
            copy_board(round->board_before, model->board);
            memcpy(round->marks, marks, sizeof(round->marks));
            round->match_count = count_marks(marks);
            round->score_delta = round_score;
            collapse_and_refill(model, marks, round);
            copy_board(round->board_after, model->board);
        } else {
            collapse_and_refill(model, marks, NULL);
        }
    }

    return resolved_any;
}

void design_fruit_model_init(design_fruit_model_t *model, uint32_t seed)
{
    if(model == NULL) return;
    memset(model, 0, sizeof(*model));
    model->rng = seed != 0u ? seed : 1u;

    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
            uint8_t value;
            do {
                value = random_fruit(model);
            } while(would_match_at(model, row, col, value));
            model->board[row][col] = value;
        }
    }
}

uint8_t design_fruit_model_rows(const design_fruit_model_t *model)
{
    (void)model;
    return DESIGN_FRUIT_ROWS;
}

uint8_t design_fruit_model_cols(const design_fruit_model_t *model)
{
    (void)model;
    return DESIGN_FRUIT_COLS;
}

uint8_t design_fruit_model_cell(const design_fruit_model_t *model, uint8_t row, uint8_t col)
{
    if(model == NULL || !in_bounds(row, col)) return 0u;
    return model->board[row][col];
}

uint32_t design_fruit_model_score(const design_fruit_model_t *model)
{
    return model != NULL ? model->score : 0u;
}

bool design_fruit_model_has_matches(const design_fruit_model_t *model)
{
    bool marks[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];
    if(model == NULL) return false;
    return mark_matches(model, marks, NULL);
}

static bool swap_creates_match(design_fruit_model_t *probe, uint8_t r1, uint8_t c1, uint8_t r2, uint8_t c2)
{
    bool marks[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];
    bool found;
    uint8_t tmp = probe->board[r1][c1];
    probe->board[r1][c1] = probe->board[r2][c2];
    probe->board[r2][c2] = tmp;
    found = mark_matches(probe, marks, NULL);
    tmp = probe->board[r1][c1];
    probe->board[r1][c1] = probe->board[r2][c2];
    probe->board[r2][c2] = tmp;
    return found;
}

bool design_fruit_model_has_available_move(const design_fruit_model_t *model)
{
    design_fruit_model_t probe;

    if(model == NULL) return false;

    /* 在副本上试探每一个相邻交换，命中任意消除即说明仍可走棋。
     * 棋盘在两步之间始终是稳定无消除态，所以"交换后产生消除"等价于该步合法。*/
    probe = *model;
    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
            if(col + 1u < DESIGN_FRUIT_COLS && swap_creates_match(&probe, row, col, row, (uint8_t)(col + 1u))) {
                return true;
            }
            if(row + 1u < DESIGN_FRUIT_ROWS && swap_creates_match(&probe, row, col, (uint8_t)(row + 1u), col)) {
                return true;
            }
        }
    }
    return false;
}

void design_fruit_model_set_board(design_fruit_model_t *model,
                                  const uint8_t board[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS])
{
    if(model == NULL || board == NULL) return;
    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
            uint8_t value = board[row][col];
            model->board[row][col] = (value >= 1u && value <= DESIGN_FRUIT_TYPES) ? value : 1u;
        }
    }
}

bool design_fruit_model_swap(design_fruit_model_t *model,
                             uint8_t row_a,
                             uint8_t col_a,
                             uint8_t row_b,
                             uint8_t col_b)
{
    design_fruit_swap_plan_t plan;

    return design_fruit_model_swap_with_plan(model, row_a, col_a, row_b, col_b, &plan);
}

bool design_fruit_model_swap_with_plan(design_fruit_model_t *model,
                                       uint8_t row_a,
                                       uint8_t col_a,
                                       uint8_t row_b,
                                       uint8_t col_b,
                                       design_fruit_swap_plan_t *plan)
{
    uint8_t before[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];
    uint8_t tmp;

    if(plan != NULL) {
        memset(plan, 0, sizeof(*plan));
        plan->from_row = row_a;
        plan->from_col = col_a;
        plan->to_row = row_b;
        plan->to_col = col_b;
    }

    if(model == NULL) return false;
    if(!in_bounds(row_a, col_a) || !in_bounds(row_b, col_b)) return false;
    if(!are_adjacent(row_a, col_a, row_b, col_b)) return false;

    copy_board(before, model->board);

    tmp = model->board[row_a][col_a];
    model->board[row_a][col_a] = model->board[row_b][col_b];
    model->board[row_b][col_b] = tmp;

    if(plan != NULL) {
        if(!resolve_board_with_plan(model, plan)) {
            copy_board(model->board, before);
            memset(plan->final_board, 0, sizeof(plan->final_board));
            return false;
        }
        plan->accepted = true;
        copy_board(plan->final_board, model->board);
        model->score += plan->score_delta;
        return true;
    }

    uint32_t gained = 0;
    if(!resolve_board(model, &gained)) {
        copy_board(model->board, before);
        return false;
    }

    model->score += gained;
    return true;
}
