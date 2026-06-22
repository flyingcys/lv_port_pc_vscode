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

static void collapse_and_refill(design_fruit_model_t *model,
                                const bool marks[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS])
{
    for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
        int write_row = DESIGN_FRUIT_ROWS - 1;
        for(int row = DESIGN_FRUIT_ROWS - 1; row >= 0; row--) {
            if(!marks[row][col]) {
                model->board[write_row][col] = model->board[row][col];
                write_row--;
            }
        }
        while(write_row >= 0) {
            model->board[write_row][col] = random_fruit(model);
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
        collapse_and_refill(model, marks);
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
    uint8_t before[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];
    uint8_t tmp;
    uint32_t gained = 0;

    if(model == NULL) return false;
    if(!in_bounds(row_a, col_a) || !in_bounds(row_b, col_b)) return false;
    if(!are_adjacent(row_a, col_a, row_b, col_b)) return false;

    memcpy(before, model->board, sizeof(before));

    tmp = model->board[row_a][col_a];
    model->board[row_a][col_a] = model->board[row_b][col_b];
    model->board[row_b][col_b] = tmp;

    if(!resolve_board(model, &gained)) {
        memcpy(model->board, before, sizeof(before));
        return false;
    }

    model->score += gained;
    return true;
}
