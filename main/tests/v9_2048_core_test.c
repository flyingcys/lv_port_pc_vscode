#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../src/v9_2048/game_2048_core.h"

static void assert_cell(const game_2048_core_t *game, uint8_t row, uint8_t col, uint32_t value)
{
    assert(game_2048_core_cell(game, row, col) == value);
}

int main(void)
{
    game_2048_core_t game;
    game_2048_move_result_t result;

    assert(game_2048_core_init(&game, 4, 7U));
    assert(game.size == 4);

    game_2048_core_clear(&game);
    game_2048_core_set_cell(&game, 0, 0, 2);
    game_2048_core_set_cell(&game, 0, 1, 2);
    game_2048_core_set_cell(&game, 0, 2, 2);
    game_2048_core_set_cell(&game, 0, 3, 2);

    assert(game_2048_core_move(&game, GAME_2048_DIR_LEFT, &result));
    assert_cell(&game, 0, 0, 4);
    assert_cell(&game, 0, 1, 4);
    assert(game.score == 8);
    assert(result.merge_count == 2);
    assert(result.step_count >= 4);

    bool saw_disappearing_merge = false;
    bool saw_surviving_merge = false;
    for(uint8_t i = 0; i < result.step_count; i++) {
        if(result.steps[i].merged && result.steps[i].disappear) saw_disappearing_merge = true;
        if(result.steps[i].merged && !result.steps[i].disappear) saw_surviving_merge = true;
    }
    assert(saw_disappearing_merge);
    assert(saw_surviving_merge);
    assert(result.has_new_tile);

    assert(game_2048_core_init(&game, 5, 11U));
    assert(game.size == 5);
    assert(game_2048_core_init(&game, 6, 13U));
    assert(game.size == 6);
    assert(!game_2048_core_init(&game, 3, 17U));

    return 0;
}
