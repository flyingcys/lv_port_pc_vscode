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

typedef struct {
    uint8_t board[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];
    uint32_t score;
    uint32_t rng;
} design_fruit_model_t;

void design_fruit_model_init(design_fruit_model_t *model, uint32_t seed);
uint8_t design_fruit_model_rows(const design_fruit_model_t *model);
uint8_t design_fruit_model_cols(const design_fruit_model_t *model);
uint8_t design_fruit_model_cell(const design_fruit_model_t *model, uint8_t row, uint8_t col);
uint32_t design_fruit_model_score(const design_fruit_model_t *model);
bool design_fruit_model_has_matches(const design_fruit_model_t *model);
void design_fruit_model_set_board(design_fruit_model_t *model,
                                  const uint8_t board[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS]);
bool design_fruit_model_swap(design_fruit_model_t *model,
                             uint8_t row_a,
                             uint8_t col_a,
                             uint8_t row_b,
                             uint8_t col_b);

#ifdef __cplusplus
}
#endif

#endif /* DESIGN_FRUIT_MODEL_H */
