#include "game_2048.h"

#include <stdlib.h>
#include <string.h>

#include "lvgl/lvgl.h"

typedef struct {
    uint8_t row;
    uint8_t col;
    uint16_t value;
} tile_info_t;

static void get_cell_coords(game_2048_dir_t dir, uint8_t line, uint8_t index, uint8_t *row, uint8_t *col);
static void spawn_random_tile(game_2048_t *game, game_2048_move_result_t *result);
static bool has_available_moves(const game_2048_t *game);

void game_2048_init(game_2048_t *game)
{
    if(game == NULL) {
        return;
    }

    memset(game, 0, sizeof(*game));
    srand((unsigned int)lv_tick_get());

    spawn_random_tile(game, NULL);
    spawn_random_tile(game, NULL);
}

void game_2048_reset(game_2048_t *game)
{
    if(game == NULL) {
        return;
    }

    uint32_t seed = lv_tick_get();
    memset(game, 0, sizeof(*game));
    srand(seed);

    spawn_random_tile(game, NULL);
    spawn_random_tile(game, NULL);
}

bool game_2048_move(game_2048_t *game, game_2048_dir_t dir, game_2048_move_result_t *result)
{
    if((game == NULL) || (dir > GAME_2048_DIR_DOWN)) {
        return false;
    }

    if(result != NULL) {
        memset(result, 0, sizeof(*result));
    }

    if(game->over) {
        return false;
    }

    uint16_t new_cells[GAME_2048_SIZE][GAME_2048_SIZE];
    memset(new_cells, 0, sizeof(new_cells));

    bool moved = false;

    for(uint8_t line = 0U; line < GAME_2048_SIZE; line++) {
        tile_info_t collected[GAME_2048_SIZE];
        uint8_t collected_count = 0U;

        for(uint8_t idx = 0U; idx < GAME_2048_SIZE; idx++) {
            uint8_t row;
            uint8_t col;
            get_cell_coords(dir, line, idx, &row, &col);
            uint16_t value = game->cells[row][col];
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
            get_cell_coords(dir, line, write_index, &dest_row, &dest_col);

            if((read_index + 1U) < collected_count && collected[read_index + 1U].value == current.value) {
                uint16_t merged_value = (uint16_t)(current.value << 1U);
                new_cells[dest_row][dest_col] = merged_value;

                if(result != NULL) {
                    game_2048_move_step_t *step = &result->steps[result->step_count++];
                    step->from_row = current.row;
                    step->from_col = current.col;
                    step->to_row = dest_row;
                    step->to_col = dest_col;
                    step->value_from = current.value;
                    step->value_to = merged_value;
                    step->merged = true;
                    step->disappear = false;

                    game_2048_move_step_t *merge_step = &result->steps[result->step_count++];
                    merge_step->from_row = collected[read_index + 1U].row;
                    merge_step->from_col = collected[read_index + 1U].col;
                    merge_step->to_row = dest_row;
                    merge_step->to_col = dest_col;
                    merge_step->value_from = collected[read_index + 1U].value;
                    merge_step->value_to = merged_value;
                    merge_step->merged = true;
                    merge_step->disappear = true;
                }

                if((dest_row != current.row) || (dest_col != current.col)) {
                    moved = true;
                }
                moved = true;

                game->score += merged_value;
                if(merged_value > game->max_tile) {
                    game->max_tile = merged_value;
                }

                read_index += 2U;
            } else {
                new_cells[dest_row][dest_col] = current.value;

                if(result != NULL) {
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

                if((dest_row != current.row) || (dest_col != current.col)) {
                    moved = true;
                }

                read_index++;
            }

            write_index++;
        }
    }

    if(!moved) {
        if(result != NULL) {
            result->step_count = 0U;
        }
        return false;
    }

    memcpy(game->cells, new_cells, sizeof(game->cells));

    if(game->max_tile >= GAME_2048_TARGET) {
        game->won = true;
    }

    spawn_random_tile(game, result);

    if(!has_available_moves(game)) {
        game->over = true;
    }

    return true;
}

uint16_t game_2048_get_cell(const game_2048_t *game, uint8_t row, uint8_t col)
{
    if((game == NULL) || (row >= GAME_2048_SIZE) || (col >= GAME_2048_SIZE)) {
        return 0U;
    }

    return game->cells[row][col];
}

uint32_t game_2048_get_score(const game_2048_t *game)
{
    if(game == NULL) {
        return 0U;
    }

    return game->score;
}

game_2048_status_t game_2048_get_status(const game_2048_t *game)
{
    if(game == NULL) {
        return GAME_2048_STATUS_PLAYING;
    }

    if(game->over) {
        return GAME_2048_STATUS_OVER;
    }

    if(game->won) {
        return GAME_2048_STATUS_WON;
    }

    return GAME_2048_STATUS_PLAYING;
}

static void get_cell_coords(game_2048_dir_t dir, uint8_t line, uint8_t index, uint8_t *row, uint8_t *col)
{
    switch(dir) {
        case GAME_2048_DIR_LEFT:
            *row = line;
            *col = index;
            break;
        case GAME_2048_DIR_RIGHT:
            *row = line;
            *col = (uint8_t)(GAME_2048_SIZE - 1U - index);
            break;
        case GAME_2048_DIR_UP:
            *row = index;
            *col = line;
            break;
        case GAME_2048_DIR_DOWN:
        default:
            *row = (uint8_t)(GAME_2048_SIZE - 1U - index);
            *col = line;
            break;
    }
}

static void spawn_random_tile(game_2048_t *game, game_2048_move_result_t *result)
{
    uint8_t empty_rows[GAME_2048_SIZE * GAME_2048_SIZE];
    uint8_t empty_cols[GAME_2048_SIZE * GAME_2048_SIZE];
    uint8_t empty_count = 0U;

    for(uint8_t row = 0U; row < GAME_2048_SIZE; row++) {
        for(uint8_t col = 0U; col < GAME_2048_SIZE; col++) {
            if(game->cells[row][col] == 0U) {
                empty_rows[empty_count] = row;
                empty_cols[empty_count] = col;
                empty_count++;
            }
        }
    }

    if(empty_count == 0U) {
        if(result != NULL) {
            result->has_new_tile = false;
        }
        return;
    }

    uint8_t index = (uint8_t)(rand() % empty_count);
    uint8_t row = empty_rows[index];
    uint8_t col = empty_cols[index];
    uint16_t value = (uint16_t)((rand() % 10U) == 0U ? 4U : 2U);

    game->cells[row][col] = value;
    if(value > game->max_tile) {
        game->max_tile = value;
    }

    if(result != NULL) {
        result->has_new_tile = true;
        result->new_tile_row = row;
        result->new_tile_col = col;
        result->new_tile_value = value;
    }
}

static bool has_available_moves(const game_2048_t *game)
{
    for(uint8_t row = 0U; row < GAME_2048_SIZE; row++) {
        for(uint8_t col = 0U; col < GAME_2048_SIZE; col++) {
            uint16_t value = game->cells[row][col];
            if(value == 0U) {
                return true;
            }

            if(col + 1U < GAME_2048_SIZE && game->cells[row][col + 1U] == value) {
                return true;
            }

            if(row + 1U < GAME_2048_SIZE && game->cells[row + 1U][col] == value) {
                return true;
            }
        }
    }

    return false;
}
