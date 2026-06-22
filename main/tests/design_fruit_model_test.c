#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "../src/design_fruit/design_fruit_model.h"

static void test_init_board_has_no_matches(void)
{
    design_fruit_model_t model;
    design_fruit_model_init(&model, 7u);

    assert(design_fruit_model_rows(&model) == 8);
    assert(design_fruit_model_cols(&model) == 8);
    assert(!design_fruit_model_has_matches(&model));
}

static void test_adjacent_swap_resolves_match_and_scores(void)
{
    design_fruit_model_t model;
    static const uint8_t board[8][8] = {
        { 1, 1, 2, 1, 4, 5, 6, 7 },
        { 2, 3, 4, 5, 6, 7, 1, 2 },
        { 3, 4, 5, 6, 7, 1, 2, 3 },
        { 4, 5, 6, 7, 1, 2, 3, 4 },
        { 5, 6, 7, 1, 2, 3, 4, 5 },
        { 6, 7, 1, 2, 3, 4, 5, 6 },
        { 7, 1, 2, 3, 4, 5, 6, 7 },
        { 1, 2, 3, 4, 5, 6, 7, 1 },
    };

    design_fruit_model_init(&model, 11u);
    design_fruit_model_set_board(&model, board);

    assert(!design_fruit_model_has_matches(&model));
    assert(design_fruit_model_swap(&model, 0, 2, 0, 3));
    assert(design_fruit_model_score(&model) >= 300);
    assert(!design_fruit_model_has_matches(&model));
    assert(design_fruit_model_cell(&model, 0, 0) >= 1);
    assert(design_fruit_model_cell(&model, 0, 0) <= 7);
}

static void test_swap_plan_describes_matches_moves_and_spawns(void)
{
    design_fruit_model_t model;
    design_fruit_swap_plan_t plan;
    static const uint8_t board[8][8] = {
        { 1, 1, 2, 1, 4, 5, 6, 7 },
        { 2, 3, 4, 5, 6, 7, 1, 2 },
        { 3, 4, 5, 6, 7, 1, 2, 3 },
        { 1, 1, 2, 1, 5, 2, 3, 4 },
        { 5, 6, 7, 1, 2, 3, 4, 5 },
        { 6, 7, 1, 2, 3, 4, 5, 6 },
        { 7, 1, 2, 3, 4, 5, 6, 7 },
        { 1, 2, 3, 4, 5, 6, 7, 1 },
    };

    design_fruit_model_init(&model, 11u);
    design_fruit_model_set_board(&model, board);

    assert(design_fruit_model_swap_with_plan(&model, 3, 2, 3, 3, &plan));
    assert(plan.accepted);
    assert(plan.round_count >= 1);
    assert(plan.score_delta >= 300);
    assert(plan.rounds[0].match_count >= 3);
    assert(plan.rounds[0].marks[3][0]);
    assert(plan.rounds[0].marks[3][1]);
    assert(plan.rounds[0].marks[3][2]);
    assert(plan.rounds[0].move_count > 0);
    assert(plan.rounds[0].spawn_count > 0);
    assert(plan.rounds[0].spawns[0].drop_cells > 0);
    assert(!design_fruit_model_has_matches(&model));
    assert(design_fruit_model_score(&model) == plan.score_delta);
    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
            assert(design_fruit_model_cell(&model, row, col) == plan.final_board[row][col]);
        }
    }
}

static void test_gap_pattern_without_post_swap_match_is_rejected(void)
{
    design_fruit_model_t model;
    static const uint8_t board[8][8] = {
        { 1, 2, 1, 3, 4, 5, 6, 7 },
        { 2, 3, 4, 5, 6, 7, 1, 2 },
        { 3, 4, 5, 6, 7, 1, 2, 3 },
        { 4, 5, 6, 7, 1, 2, 3, 4 },
        { 5, 6, 7, 1, 2, 3, 4, 5 },
        { 6, 7, 1, 2, 3, 4, 5, 6 },
        { 7, 1, 2, 3, 4, 5, 6, 7 },
        { 1, 2, 3, 4, 5, 6, 7, 1 },
    };

    design_fruit_model_init(&model, 17u);
    design_fruit_model_set_board(&model, board);

    assert(!design_fruit_model_swap(&model, 0, 1, 0, 2));
    assert(design_fruit_model_score(&model) == 0);
    assert(design_fruit_model_cell(&model, 0, 1) == 2);
    assert(design_fruit_model_cell(&model, 0, 2) == 1);
}

static void test_non_adjacent_swap_is_rejected(void)
{
    design_fruit_model_t model;
    design_fruit_model_init(&model, 13u);

    assert(!design_fruit_model_swap(&model, 0, 0, 2, 2));
}

int main(void)
{
    test_init_board_has_no_matches();
    test_adjacent_swap_resolves_match_and_scores();
    test_swap_plan_describes_matches_moves_and_spawns();
    test_gap_pattern_without_post_swap_match_is_rejected();
    test_non_adjacent_swap_is_rejected();
    return 0;
}
