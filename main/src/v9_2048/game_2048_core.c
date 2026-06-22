#include "game_2048_core.h"

#include <string.h>

typedef struct {
    uint8_t row;
    uint8_t col;
    uint32_t value;
} tile_info_t;

static uint32_t next_random(game_2048_core_t *game)
{
    game->rng_state = game->rng_state * 1664525U + 1013904223U;
    return game->rng_state;
}

static void get_cell_coords(uint8_t size, game_2048_dir_t dir, uint8_t line, uint8_t index,
                            uint8_t *row, uint8_t *col)
{
    switch(dir) {
        case GAME_2048_DIR_LEFT:
            *row = line;
            *col = index;
            break;
        case GAME_2048_DIR_RIGHT:
            *row = line;
            *col = (uint8_t)(size - 1U - index);
            break;
        case GAME_2048_DIR_UP:
            *row = index;
            *col = line;
            break;
        case GAME_2048_DIR_DOWN:
        default:
            *row = (uint8_t)(size - 1U - index);
            *col = line;
            break;
    }
}

static bool has_available_moves(const game_2048_core_t *game)
{
    for(uint8_t row = 0; row < game->size; row++) {
        for(uint8_t col = 0; col < game->size; col++) {
            uint32_t value = game->cells[row][col];
            if(value == 0U) return true;
            if(col + 1U < game->size && game->cells[row][col + 1U] == value) return true;
            if(row + 1U < game->size && game->cells[row + 1U][col] == value) return true;
        }
    }
    return false;
}

static void spawn_random_tile(game_2048_core_t *game, game_2048_move_result_t *result)
{
    uint8_t empty_rows[GAME_2048_MAX_SIZE * GAME_2048_MAX_SIZE];
    uint8_t empty_cols[GAME_2048_MAX_SIZE * GAME_2048_MAX_SIZE];
    uint8_t empty_count = 0U;

    for(uint8_t row = 0U; row < game->size; row++) {
        for(uint8_t col = 0U; col < game->size; col++) {
            if(game->cells[row][col] == 0U) {
                empty_rows[empty_count] = row;
                empty_cols[empty_count] = col;
                empty_count++;
            }
        }
    }

    if(empty_count == 0U) {
        if(result != NULL) result->has_new_tile = false;
        return;
    }

    uint8_t index = (uint8_t)(next_random(game) % empty_count);
    uint8_t row = empty_rows[index];
    uint8_t col = empty_cols[index];
    uint32_t value = (next_random(game) % 10U) == 0U ? 4U : 2U;

    game->cells[row][col] = value;
    if(value > game->max_tile) game->max_tile = value;

    if(result != NULL) {
        result->has_new_tile = true;
        result->new_tile_row = row;
        result->new_tile_col = col;
        result->new_tile_value = value;
    }
}

bool game_2048_core_init(game_2048_core_t *game, uint8_t size, uint32_t seed)
{
    if(game == NULL || size < GAME_2048_MIN_SIZE || size > GAME_2048_MAX_SIZE) return false;

    memset(game, 0, sizeof(*game));
    game->size = size;
    game->rng_state = seed != 0U ? seed : 1U;
    spawn_random_tile(game, NULL);
    spawn_random_tile(game, NULL);
    return true;
}

void game_2048_core_clear(game_2048_core_t *game)
{
    if(game == NULL) return;

    uint8_t size = game->size;
    uint32_t best = game->best_score;
    uint32_t seed = game->rng_state != 0U ? game->rng_state : 1U;
    memset(game, 0, sizeof(*game));
    game->size = size;
    game->best_score = best;
    game->rng_state = seed;
}

bool game_2048_core_set_size(game_2048_core_t *game, uint8_t size, uint32_t seed)
{
    return game_2048_core_init(game, size, seed);
}

bool game_2048_core_set_cell(game_2048_core_t *game, uint8_t row, uint8_t col, uint32_t value)
{
    if(game == NULL || row >= game->size || col >= game->size) return false;
    game->cells[row][col] = value;
    if(value > game->max_tile) game->max_tile = value;
    if(game->max_tile >= GAME_2048_TARGET) game->won = true;
    game->over = !has_available_moves(game);
    return true;
}

uint32_t game_2048_core_cell(const game_2048_core_t *game, uint8_t row, uint8_t col)
{
    if(game == NULL || row >= game->size || col >= game->size) return 0U;
    return game->cells[row][col];
}

bool game_2048_core_move(game_2048_core_t *game, game_2048_dir_t dir, game_2048_move_result_t *result)
{
    if(game == NULL || dir > GAME_2048_DIR_DOWN) return false;
    if(result != NULL) memset(result, 0, sizeof(*result));
    if(game->over) return false;

    uint32_t new_cells[GAME_2048_MAX_SIZE][GAME_2048_MAX_SIZE];
    memset(new_cells, 0, sizeof(new_cells));
    bool moved = false;

    for(uint8_t line = 0U; line < game->size; line++) {
        tile_info_t collected[GAME_2048_MAX_SIZE];
        uint8_t collected_count = 0U;

        for(uint8_t idx = 0U; idx < game->size; idx++) {
            uint8_t row;
            uint8_t col;
            get_cell_coords(game->size, dir, line, idx, &row, &col);
            uint32_t value = game->cells[row][col];
            if(value != 0U) {
                collected[collected_count].row = row;
                collected[collected_count].col = col;
                collected[collected_count].value = value;
                collected_count++;
            }
        }

        uint8_t write_index = 0U;
        uint8_t read_index = 0U;
        while(read_index < collected_count) {
            tile_info_t current = collected[read_index];
            uint8_t dest_row;
            uint8_t dest_col;
            get_cell_coords(game->size, dir, line, write_index, &dest_row, &dest_col);

            if((read_index + 1U) < collected_count && collected[read_index + 1U].value == current.value) {
                uint32_t merged_value = current.value << 1U;
                new_cells[dest_row][dest_col] = merged_value;

                if(result != NULL && result->step_count + 2U <= GAME_2048_MAX_SIZE * GAME_2048_MAX_SIZE) {
                    game_2048_move_step_t *step = &result->steps[result->step_count++];
                    step->from_row = current.row;
                    step->from_col = current.col;
                    step->to_row = dest_row;
                    step->to_col = dest_col;
                    step->value_from = current.value;
                    step->value_to = merged_value;
                    step->merged = true;
                    step->disappear = false;

                    step = &result->steps[result->step_count++];
                    step->from_row = collected[read_index + 1U].row;
                    step->from_col = collected[read_index + 1U].col;
                    step->to_row = dest_row;
                    step->to_col = dest_col;
                    step->value_from = collected[read_index + 1U].value;
                    step->value_to = merged_value;
                    step->merged = true;
                    step->disappear = true;
                    result->merge_count++;
                }

                game->score += merged_value;
                if(merged_value > game->max_tile) game->max_tile = merged_value;
                moved = true;
                read_index += 2U;
            } else {
                new_cells[dest_row][dest_col] = current.value;

                if(result != NULL && result->step_count < GAME_2048_MAX_SIZE * GAME_2048_MAX_SIZE) {
                    game_2048_move_step_t *step = &result->steps[result->step_count++];
                    step->from_row = current.row;
                    step->from_col = current.col;
                    step->to_row = dest_row;
                    step->to_col = dest_col;
                    step->value_from = current.value;
                    step->value_to = current.value;
                    step->merged = false;
                    step->disappear = false;
                }

                if(dest_row != current.row || dest_col != current.col) moved = true;
                read_index++;
            }
            write_index++;
        }
    }

    if(!moved) {
        if(result != NULL) result->step_count = 0U;
        return false;
    }

    memcpy(game->cells, new_cells, sizeof(game->cells));
    if(game->max_tile >= GAME_2048_TARGET) game->won = true;
    spawn_random_tile(game, result);
    if(game->score > game->best_score) game->best_score = game->score;
    game->over = !has_available_moves(game);
    return true;
}

game_2048_status_t game_2048_core_status(const game_2048_core_t *game)
{
    if(game == NULL) return GAME_2048_STATUS_PLAYING;
    if(game->over) return GAME_2048_STATUS_OVER;
    if(game->won) return GAME_2048_STATUS_WON;
    return GAME_2048_STATUS_PLAYING;
}
