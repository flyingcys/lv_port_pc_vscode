/**
 * @file game_2048.c
 * @brief 2048 Game implementation for LVGL v9
 */

/*********************
 *      INCLUDES
 *********************/
#include "game_2048.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*********************
 *      DEFINES
 *********************/
#define ANIM_DURATION_MS    100     /* Animation duration */
#define SPAWN_ANIM_MS       150     /* Spawn animation duration */

/**********************
 *      TYPEDEFS
 **********************/

/**
 * @brief Color configuration for each tile value
 */
typedef struct {
    uint16_t value;
    lv_color_t bg_color;
    lv_color_t text_color;
    const lv_font_t *font;
} tile_style_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void create_ui(game_2048_t *game, lv_obj_t *parent);
static void create_board(game_2048_t *game);
static void create_header(game_2048_t *game);
static void update_ui(game_2048_t *game);
static void update_cell(game_2048_t *game, int row, int col);
static void spawn_random_tile(game_2048_t *game);
static bool can_move(game_2048_t *game);
static bool move_line(uint16_t *line, int size, uint32_t *score);
static void check_win(game_2048_t *game);
static void show_overlay(game_2048_t *game, const char *message, bool show_continue);
static void hide_overlay(game_2048_t *game);
static lv_color_t get_tile_bg_color(uint16_t value);
static lv_color_t get_tile_text_color(uint16_t value);
static const lv_font_t *get_tile_font(uint16_t value, int32_t cell_size);
static void board_event_cb(lv_event_t *e);
static void reset_btn_event_cb(lv_event_t *e);
static void continue_btn_event_cb(lv_event_t *e);
static void animate_cell_appear(lv_obj_t *cell);
static void anim_scale_cb(void *var, int32_t v);
static void anim_opa_cb(void *var, int32_t v);

/**********************
 *  STATIC VARIABLES
 **********************/
static bool rand_seeded = false;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

game_2048_t *game_2048_create(lv_obj_t *parent)
{
    /* Seed random number generator */
    if (!rand_seeded) {
        srand((unsigned int)time(NULL));
        rand_seeded = true;
    }
    
    /* Allocate game structure */
    game_2048_t *game = lv_malloc(sizeof(game_2048_t));
    if (game == NULL) return NULL;
    
    /* Initialize game data */
    memset(game, 0, sizeof(game_2048_t));
    game->state = GAME_2048_STATE_PLAYING;
    
    /* Calculate layout based on parent size - adaptive design */
    int32_t parent_w = lv_obj_get_width(parent);
    int32_t parent_h = lv_obj_get_height(parent);
    
    /* Use smaller dimension for square board */
    int32_t available_size = LV_MIN(parent_w, parent_h - 140); /* Reserve space for header/footer */
    
    game->cell_gap = 8;
    game->board_padding = 10;
    
    /* Calculate cell size: board_size = padding*2 + cell_size*4 + gap*5 */
    game->cell_size = (available_size - game->board_padding * 2 - game->cell_gap * 5) / GAME_2048_GRID_SIZE;
    
    /* Minimum cell size */
    if (game->cell_size < 50) game->cell_size = 50;
    /* Maximum cell size */
    if (game->cell_size > 100) game->cell_size = 100;
    
    /* Create UI */
    create_ui(game, parent);
    
    /* Initialize game */
    game_2048_reset(game);
    
    return game;
}

void game_2048_reset(game_2048_t *game)
{
    if (game == NULL) return;
    
    /* Clear grid */
    memset(game->grid, 0, sizeof(game->grid));
    
    /* Reset score */
    game->score = 0;
    
    /* Reset state */
    game->state = GAME_2048_STATE_PLAYING;
    
    /* Hide overlay */
    hide_overlay(game);
    
    /* Spawn initial tiles */
    spawn_random_tile(game);
    spawn_random_tile(game);
    
    /* Update display */
    update_ui(game);
}

bool game_2048_move(game_2048_t *game, game_2048_dir_t dir)
{
    if (game == NULL) return false;
    if (game->state == GAME_2048_STATE_OVER) return false;
    
    bool moved = false;
    uint16_t temp_line[GAME_2048_GRID_SIZE];
    
    switch (dir) {
        case GAME_2048_DIR_LEFT:
            for (int row = 0; row < GAME_2048_GRID_SIZE; row++) {
                /* Copy row */
                for (int col = 0; col < GAME_2048_GRID_SIZE; col++) {
                    temp_line[col] = game->grid[row][col];
                }
                /* Move left */
                if (move_line(temp_line, GAME_2048_GRID_SIZE, &game->score)) {
                    moved = true;
                    for (int col = 0; col < GAME_2048_GRID_SIZE; col++) {
                        game->grid[row][col] = temp_line[col];
                    }
                }
            }
            break;
            
        case GAME_2048_DIR_RIGHT:
            for (int row = 0; row < GAME_2048_GRID_SIZE; row++) {
                /* Copy row in reverse */
                for (int col = 0; col < GAME_2048_GRID_SIZE; col++) {
                    temp_line[col] = game->grid[row][GAME_2048_GRID_SIZE - 1 - col];
                }
                /* Move (reverse direction) */
                if (move_line(temp_line, GAME_2048_GRID_SIZE, &game->score)) {
                    moved = true;
                    for (int col = 0; col < GAME_2048_GRID_SIZE; col++) {
                        game->grid[row][GAME_2048_GRID_SIZE - 1 - col] = temp_line[col];
                    }
                }
            }
            break;
            
        case GAME_2048_DIR_UP:
            for (int col = 0; col < GAME_2048_GRID_SIZE; col++) {
                /* Copy column */
                for (int row = 0; row < GAME_2048_GRID_SIZE; row++) {
                    temp_line[row] = game->grid[row][col];
                }
                /* Move up */
                if (move_line(temp_line, GAME_2048_GRID_SIZE, &game->score)) {
                    moved = true;
                    for (int row = 0; row < GAME_2048_GRID_SIZE; row++) {
                        game->grid[row][col] = temp_line[row];
                    }
                }
            }
            break;
            
        case GAME_2048_DIR_DOWN:
            for (int col = 0; col < GAME_2048_GRID_SIZE; col++) {
                /* Copy column in reverse */
                for (int row = 0; row < GAME_2048_GRID_SIZE; row++) {
                    temp_line[row] = game->grid[GAME_2048_GRID_SIZE - 1 - row][col];
                }
                /* Move (reverse direction) */
                if (move_line(temp_line, GAME_2048_GRID_SIZE, &game->score)) {
                    moved = true;
                    for (int row = 0; row < GAME_2048_GRID_SIZE; row++) {
                        game->grid[GAME_2048_GRID_SIZE - 1 - row][col] = temp_line[row];
                    }
                }
            }
            break;
    }
    
    if (moved) {
        /* Spawn new tile */
        spawn_random_tile(game);
        
        /* Update best score */
        if (game->score > game->best_score) {
            game->best_score = game->score;
        }
        
        /* Check win condition */
        check_win(game);
        
        /* Check game over */
        if (!can_move(game)) {
            game->state = GAME_2048_STATE_OVER;
            show_overlay(game, "Game Over!", false);
        }
        
        /* Update UI */
        update_ui(game);
    }
    
    return moved;
}

game_2048_state_t game_2048_get_state(game_2048_t *game)
{
    return game ? game->state : GAME_2048_STATE_OVER;
}

uint32_t game_2048_get_score(game_2048_t *game)
{
    return game ? game->score : 0;
}

void game_2048_continue(game_2048_t *game)
{
    if (game == NULL) return;
    if (game->state == GAME_2048_STATE_WON) {
        game->state = GAME_2048_STATE_CONTINUE;
        hide_overlay(game);
    }
}

void game_2048_delete(game_2048_t *game)
{
    if (game == NULL) return;
    
    if (game->main_container) {
        lv_obj_delete(game->main_container);
    }
    
    lv_free(game);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * @brief Create main UI structure
 */
static void create_ui(game_2048_t *game, lv_obj_t *parent)
{
    /* Main container */
    game->main_container = lv_obj_create(parent);
    lv_obj_set_size(game->main_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(game->main_container, lv_color_hex(0xFAF8EF), 0);
    lv_obj_set_style_bg_opa(game->main_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(game->main_container, 0, 0);
    lv_obj_set_style_pad_all(game->main_container, 10, 0);
    lv_obj_set_flex_flow(game->main_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(game->main_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(game->main_container, 10, 0);
    lv_obj_remove_flag(game->main_container, LV_OBJ_FLAG_SCROLLABLE);
    
    /* Store game pointer for event callbacks */
    lv_obj_set_user_data(game->main_container, game);
    
    /* Create header with title and scores */
    create_header(game);
    
    /* Create game board */
    create_board(game);
}

/**
 * @brief Create header with title and score display
 */
static void create_header(game_2048_t *game)
{
    /* Header container */
    lv_obj_t *header = lv_obj_create(game->main_container);
    lv_obj_set_size(header, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_remove_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    
    /* Title */
    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "2048");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x776E65), 0);
    
    /* Score container */
    lv_obj_t *score_cont = lv_obj_create(header);
    lv_obj_set_size(score_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(score_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(score_cont, 0, 0);
    lv_obj_set_style_pad_all(score_cont, 0, 0);
    lv_obj_set_flex_flow(score_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(score_cont, 8, 0);
    lv_obj_remove_flag(score_cont, LV_OBJ_FLAG_SCROLLABLE);
    
    /* Score box */
    lv_obj_t *score_box = lv_obj_create(score_cont);
    lv_obj_set_size(score_box, 70, 50);
    lv_obj_set_style_bg_color(score_box, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_bg_opa(score_box, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(score_box, 6, 0);
    lv_obj_set_style_border_width(score_box, 0, 0);
    lv_obj_set_style_pad_all(score_box, 5, 0);
    lv_obj_set_flex_flow(score_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(score_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_remove_flag(score_box, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t *score_title = lv_label_create(score_box);
    lv_label_set_text(score_title, "SCORE");
    lv_obj_set_style_text_font(score_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(score_title, lv_color_hex(0xEEE4DA), 0);
    
    game->score_value_label = lv_label_create(score_box);
    lv_label_set_text(game->score_value_label, "0");
    lv_obj_set_style_text_font(game->score_value_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(game->score_value_label, lv_color_hex(0xFFFFFF), 0);
    
    /* Best score box */
    lv_obj_t *best_box = lv_obj_create(score_cont);
    lv_obj_set_size(best_box, 70, 50);
    lv_obj_set_style_bg_color(best_box, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_bg_opa(best_box, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(best_box, 6, 0);
    lv_obj_set_style_border_width(best_box, 0, 0);
    lv_obj_set_style_pad_all(best_box, 5, 0);
    lv_obj_set_flex_flow(best_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(best_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_remove_flag(best_box, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t *best_title = lv_label_create(best_box);
    lv_label_set_text(best_title, "BEST");
    lv_obj_set_style_text_font(best_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(best_title, lv_color_hex(0xEEE4DA), 0);
    
    game->best_value_label = lv_label_create(best_box);
    lv_label_set_text(game->best_value_label, "0");
    lv_obj_set_style_text_font(game->best_value_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(game->best_value_label, lv_color_hex(0xFFFFFF), 0);
    
    /* Instruction and reset button row */
    lv_obj_t *btn_row = lv_obj_create(game->main_container);
    lv_obj_set_size(btn_row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_row, 0, 0);
    lv_obj_set_style_pad_all(btn_row, 0, 0);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_remove_flag(btn_row, LV_OBJ_FLAG_SCROLLABLE);
    
    /* Instruction label */
    lv_obj_t *instr = lv_label_create(btn_row);
    lv_label_set_text(instr, "Swipe or use arrow keys");
    lv_obj_set_style_text_font(instr, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(instr, lv_color_hex(0x776E65), 0);
    
    /* Reset button */
    lv_obj_t *reset_btn = lv_button_create(btn_row);
    lv_obj_set_size(reset_btn, 80, 35);
    lv_obj_set_style_bg_color(reset_btn, lv_color_hex(0x8F7A66), 0);
    lv_obj_set_style_radius(reset_btn, 6, 0);
    lv_obj_set_user_data(reset_btn, game);
    lv_obj_add_event_cb(reset_btn, reset_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *btn_label = lv_label_create(reset_btn);
    lv_label_set_text(btn_label, "New Game");
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(btn_label);
}

/**
 * @brief Create game board
 */
static void create_board(game_2048_t *game)
{
    int32_t board_size = game->board_padding * 2 + 
                        game->cell_size * GAME_2048_GRID_SIZE + 
                        game->cell_gap * (GAME_2048_GRID_SIZE + 1);
    
    /* Board container */
    game->board = lv_obj_create(game->main_container);
    lv_obj_set_size(game->board, board_size, board_size);
    lv_obj_set_style_bg_color(game->board, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_bg_opa(game->board, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(game->board, 8, 0);
    lv_obj_set_style_border_width(game->board, 0, 0);
    lv_obj_set_style_pad_all(game->board, game->board_padding, 0);
    lv_obj_remove_flag(game->board, LV_OBJ_FLAG_SCROLLABLE);
    
    /* Store game pointer for event callbacks */
    lv_obj_set_user_data(game->board, game);
    
    /* Add event callbacks for input */
    lv_obj_add_event_cb(game->board, board_event_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(game->board, board_event_cb, LV_EVENT_GESTURE, NULL);
    
    /* Make board focusable for keyboard input */
    lv_obj_add_flag(game->board, LV_OBJ_FLAG_CLICKABLE);
    lv_group_t *g = lv_group_get_default();
    if (g) {
        lv_group_add_obj(g, game->board);
        lv_group_focus_obj(game->board);
    }
    
    /* Create cells */
    for (int row = 0; row < GAME_2048_GRID_SIZE; row++) {
        for (int col = 0; col < GAME_2048_GRID_SIZE; col++) {
            int32_t x = game->cell_gap + col * (game->cell_size + game->cell_gap);
            int32_t y = game->cell_gap + row * (game->cell_size + game->cell_gap);
            
            /* Cell background (empty slot) */
            lv_obj_t *slot = lv_obj_create(game->board);
            lv_obj_set_pos(slot, x, y);
            lv_obj_set_size(slot, game->cell_size, game->cell_size);
            lv_obj_set_style_bg_color(slot, lv_color_hex(0xCDC1B4), 0);
            lv_obj_set_style_bg_opa(slot, LV_OPA_COVER, 0);
            lv_obj_set_style_radius(slot, 6, 0);
            lv_obj_set_style_border_width(slot, 0, 0);
            lv_obj_remove_flag(slot, LV_OBJ_FLAG_SCROLLABLE);
            
            /* Cell with value (tile) */
            lv_obj_t *cell = lv_obj_create(game->board);
            lv_obj_set_pos(cell, x, y);
            lv_obj_set_size(cell, game->cell_size, game->cell_size);
            lv_obj_set_style_radius(cell, 6, 0);
            lv_obj_set_style_border_width(cell, 0, 0);
            lv_obj_remove_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_add_flag(cell, LV_OBJ_FLAG_HIDDEN); /* Hidden initially */
            
            /* Cell label */
            lv_obj_t *label = lv_label_create(cell);
            lv_obj_center(label);
            
            game->cells[row][col] = cell;
        }
    }
    
    /* Create overlay for game over/win */
    game->overlay = lv_obj_create(game->board);
    lv_obj_set_size(game->overlay, board_size - game->board_padding * 2, 
                    board_size - game->board_padding * 2);
    lv_obj_set_pos(game->overlay, 0, 0);
    lv_obj_set_style_bg_color(game->overlay, lv_color_hex(0xFAF8EF), 0);
    lv_obj_set_style_bg_opa(game->overlay, LV_OPA_80, 0);
    lv_obj_set_style_border_width(game->overlay, 0, 0);
    lv_obj_set_style_radius(game->overlay, 6, 0);
    lv_obj_set_flex_flow(game->overlay, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(game->overlay, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(game->overlay, 15, 0);
    lv_obj_add_flag(game->overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(game->overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_user_data(game->overlay, game);
    
    game->overlay_label = lv_label_create(game->overlay);
    lv_obj_set_style_text_font(game->overlay_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(game->overlay_label, lv_color_hex(0x776E65), 0);
}

/**
 * @brief Update all UI elements
 */
static void update_ui(game_2048_t *game)
{
    /* Update score labels */
    lv_label_set_text_fmt(game->score_value_label, "%"LV_PRIu32, game->score);
    lv_label_set_text_fmt(game->best_value_label, "%"LV_PRIu32, game->best_score);
    
    /* Update all cells */
    for (int row = 0; row < GAME_2048_GRID_SIZE; row++) {
        for (int col = 0; col < GAME_2048_GRID_SIZE; col++) {
            update_cell(game, row, col);
        }
    }
}

/**
 * @brief Update single cell display
 */
static void update_cell(game_2048_t *game, int row, int col)
{
    lv_obj_t *cell = game->cells[row][col];
    uint16_t value = game->grid[row][col];
    
    if (value == 0) {
        lv_obj_add_flag(cell, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(cell, LV_OBJ_FLAG_HIDDEN);
        
        /* Set background color */
        lv_obj_set_style_bg_color(cell, get_tile_bg_color(value), 0);
        lv_obj_set_style_bg_opa(cell, LV_OPA_COVER, 0);
        
        /* Update label */
        lv_obj_t *label = lv_obj_get_child(cell, 0);
        if (label) {
            lv_label_set_text_fmt(label, "%d", value);
            lv_obj_set_style_text_color(label, get_tile_text_color(value), 0);
            lv_obj_set_style_text_font(label, get_tile_font(value, game->cell_size), 0);
            lv_obj_center(label);
        }
    }
}

/**
 * @brief Spawn a random tile (2 or 4) in empty cell
 */
static void spawn_random_tile(game_2048_t *game)
{
    /* Count empty cells */
    int empty_count = 0;
    int empty_cells[GAME_2048_GRID_SIZE * GAME_2048_GRID_SIZE][2];
    
    for (int row = 0; row < GAME_2048_GRID_SIZE; row++) {
        for (int col = 0; col < GAME_2048_GRID_SIZE; col++) {
            if (game->grid[row][col] == 0) {
                empty_cells[empty_count][0] = row;
                empty_cells[empty_count][1] = col;
                empty_count++;
            }
        }
    }
    
    if (empty_count == 0) return;
    
    /* Pick random empty cell */
    int idx = rand() % empty_count;
    int row = empty_cells[idx][0];
    int col = empty_cells[idx][1];
    
    /* 90% chance for 2, 10% chance for 4 */
    game->grid[row][col] = (rand() % 10 < 9) ? 2 : 4;
    
    /* Animate appearance */
    update_cell(game, row, col);
    animate_cell_appear(game->cells[row][col]);
}

/**
 * @brief Check if any move is possible
 */
static bool can_move(game_2048_t *game)
{
    /* Check for empty cells */
    for (int row = 0; row < GAME_2048_GRID_SIZE; row++) {
        for (int col = 0; col < GAME_2048_GRID_SIZE; col++) {
            if (game->grid[row][col] == 0) return true;
        }
    }
    
    /* Check for possible merges */
    for (int row = 0; row < GAME_2048_GRID_SIZE; row++) {
        for (int col = 0; col < GAME_2048_GRID_SIZE; col++) {
            uint16_t val = game->grid[row][col];
            /* Check right neighbor */
            if (col < GAME_2048_GRID_SIZE - 1 && game->grid[row][col + 1] == val) return true;
            /* Check bottom neighbor */
            if (row < GAME_2048_GRID_SIZE - 1 && game->grid[row + 1][col] == val) return true;
        }
    }
    
    return false;
}

/**
 * @brief Move and merge a line (row or column)
 * @return true if any tiles moved
 */
static bool move_line(uint16_t *line, int size, uint32_t *score)
{
    bool moved = false;
    uint16_t original[GAME_2048_GRID_SIZE];
    
    /* Save original state */
    for (int i = 0; i < size; i++) {
        original[i] = line[i];
    }
    
    /* Step 1: Compact non-zero values to the left */
    int write_pos = 0;
    for (int i = 0; i < size; i++) {
        if (line[i] != 0) {
            if (write_pos != i) {
                line[write_pos] = line[i];
                line[i] = 0;
            }
            write_pos++;
        }
    }
    
    /* Step 2: Merge adjacent equal values */
    for (int i = 0; i < size - 1; i++) {
        if (line[i] != 0 && line[i] == line[i + 1]) {
            line[i] *= 2;
            *score += line[i];
            line[i + 1] = 0;
        }
    }
    
    /* Step 3: Compact again after merging */
    write_pos = 0;
    for (int i = 0; i < size; i++) {
        if (line[i] != 0) {
            if (write_pos != i) {
                line[write_pos] = line[i];
                line[i] = 0;
            }
            write_pos++;
        }
    }
    
    /* Check if anything changed */
    for (int i = 0; i < size; i++) {
        if (line[i] != original[i]) {
            moved = true;
            break;
        }
    }
    
    return moved;
}

/**
 * @brief Check for win condition
 */
static void check_win(game_2048_t *game)
{
    if (game->state != GAME_2048_STATE_PLAYING) return;
    
    for (int row = 0; row < GAME_2048_GRID_SIZE; row++) {
        for (int col = 0; col < GAME_2048_GRID_SIZE; col++) {
            if (game->grid[row][col] >= GAME_2048_WIN_VALUE) {
                game->state = GAME_2048_STATE_WON;
                show_overlay(game, "You Win!", true);
                return;
            }
        }
    }
}

/**
 * @brief Show overlay with message
 */
static void show_overlay(game_2048_t *game, const char *message, bool show_continue)
{
    lv_label_set_text(game->overlay_label, message);
    
    /* Remove old buttons if any */
    uint32_t child_count = lv_obj_get_child_count(game->overlay);
    while (child_count > 1) {
        lv_obj_t *child = lv_obj_get_child(game->overlay, child_count - 1);
        lv_obj_delete(child);
        child_count--;
    }
    
    /* Add Try Again button */
    lv_obj_t *try_btn = lv_button_create(game->overlay);
    lv_obj_set_size(try_btn, 100, 40);
    lv_obj_set_style_bg_color(try_btn, lv_color_hex(0x8F7A66), 0);
    lv_obj_set_style_radius(try_btn, 6, 0);
    lv_obj_set_user_data(try_btn, game);
    lv_obj_add_event_cb(try_btn, reset_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *try_label = lv_label_create(try_btn);
    lv_label_set_text(try_label, "Try Again");
    lv_obj_set_style_text_font(try_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(try_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(try_label);
    
    /* Add Continue button for win state */
    if (show_continue) {
        lv_obj_t *cont_btn = lv_button_create(game->overlay);
        lv_obj_set_size(cont_btn, 100, 40);
        lv_obj_set_style_bg_color(cont_btn, lv_color_hex(0xEDC22E), 0);
        lv_obj_set_style_radius(cont_btn, 6, 0);
        lv_obj_set_user_data(cont_btn, game);
        lv_obj_add_event_cb(cont_btn, continue_btn_event_cb, LV_EVENT_CLICKED, NULL);
        
        lv_obj_t *cont_label = lv_label_create(cont_btn);
        lv_label_set_text(cont_label, "Continue");
        lv_obj_set_style_text_font(cont_label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(cont_label, lv_color_hex(0x776E65), 0);
        lv_obj_center(cont_label);
    }
    
    lv_obj_remove_flag(game->overlay, LV_OBJ_FLAG_HIDDEN);
    
    /* Animate overlay appearance */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, game->overlay);
    lv_anim_set_values(&a, LV_OPA_0, LV_OPA_80);
    lv_anim_set_duration(&a, 300);
    lv_anim_set_exec_cb(&a, anim_opa_cb);
    lv_anim_start(&a);
}

/**
 * @brief Hide overlay
 */
static void hide_overlay(game_2048_t *game)
{
    lv_obj_add_flag(game->overlay, LV_OBJ_FLAG_HIDDEN);
}

/**
 * @brief Get tile background color based on value
 */
static lv_color_t get_tile_bg_color(uint16_t value)
{
    switch (value) {
        case 2:    return lv_color_hex(0xEEE4DA);
        case 4:    return lv_color_hex(0xEDE0C8);
        case 8:    return lv_color_hex(0xF2B179);
        case 16:   return lv_color_hex(0xF59563);
        case 32:   return lv_color_hex(0xF67C5F);
        case 64:   return lv_color_hex(0xF65E3B);
        case 128:  return lv_color_hex(0xEDCF72);
        case 256:  return lv_color_hex(0xEDCC61);
        case 512:  return lv_color_hex(0xEDC850);
        case 1024: return lv_color_hex(0xEDC53F);
        case 2048: return lv_color_hex(0xEDC22E);
        default:   return lv_color_hex(0x3C3A32); /* Super tiles */
    }
}

/**
 * @brief Get tile text color based on value
 */
static lv_color_t get_tile_text_color(uint16_t value)
{
    if (value <= 4) {
        return lv_color_hex(0x776E65);
    } else {
        return lv_color_hex(0xF9F6F2);
    }
}

/**
 * @brief Get appropriate font based on value and cell size
 */
static const lv_font_t *get_tile_font(uint16_t value, int32_t cell_size)
{
    /* Adjust font size based on cell size and number of digits */
    if (cell_size >= 80) {
        if (value < 100) return &lv_font_montserrat_32;
        if (value < 1000) return &lv_font_montserrat_28;
        return &lv_font_montserrat_22;
    } else if (cell_size >= 60) {
        if (value < 100) return &lv_font_montserrat_26;
        if (value < 1000) return &lv_font_montserrat_22;
        return &lv_font_montserrat_18;
    } else {
        if (value < 100) return &lv_font_montserrat_20;
        if (value < 1000) return &lv_font_montserrat_16;
        return &lv_font_montserrat_14;
    }
}

/**
 * @brief Board event callback for keyboard and gesture input
 */
static void board_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    game_2048_t *game = (game_2048_t *)lv_obj_get_user_data(obj);
    
    if (game == NULL) return;
    if (game->state == GAME_2048_STATE_OVER || 
        game->state == GAME_2048_STATE_WON) return;
    
    if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        switch (key) {
            case LV_KEY_UP:
                game_2048_move(game, GAME_2048_DIR_UP);
                break;
            case LV_KEY_DOWN:
                game_2048_move(game, GAME_2048_DIR_DOWN);
                break;
            case LV_KEY_LEFT:
                game_2048_move(game, GAME_2048_DIR_LEFT);
                break;
            case LV_KEY_RIGHT:
                game_2048_move(game, GAME_2048_DIR_RIGHT);
                break;
        }
    } else if (code == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        switch (dir) {
            case LV_DIR_TOP:
                game_2048_move(game, GAME_2048_DIR_UP);
                break;
            case LV_DIR_BOTTOM:
                game_2048_move(game, GAME_2048_DIR_DOWN);
                break;
            case LV_DIR_LEFT:
                game_2048_move(game, GAME_2048_DIR_LEFT);
                break;
            case LV_DIR_RIGHT:
                game_2048_move(game, GAME_2048_DIR_RIGHT);
                break;
            default:
                break;
        }
    }
}

/**
 * @brief Reset button event callback
 */
static void reset_btn_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    game_2048_t *game = (game_2048_t *)lv_obj_get_user_data(btn);
    
    if (game) {
        game_2048_reset(game);
        /* Refocus board for keyboard input */
        lv_group_focus_obj(game->board);
    }
}

/**
 * @brief Continue button event callback
 */
static void continue_btn_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    game_2048_t *game = (game_2048_t *)lv_obj_get_user_data(btn);
    
    if (game) {
        game_2048_continue(game);
        /* Refocus board for keyboard input */
        lv_group_focus_obj(game->board);
    }
}

/**
 * @brief Animation callback for scale transformation
 */
static void anim_scale_cb(void *var, int32_t v)
{
    lv_obj_set_style_transform_scale((lv_obj_t *)var, v, 0);
}

/**
 * @brief Animation callback for opacity
 */
static void anim_opa_cb(void *var, int32_t v)
{
    lv_obj_set_style_bg_opa((lv_obj_t *)var, (lv_opa_t)v, 0);
}

/**
 * @brief Animate cell appearance (scale up effect)
 */
static void animate_cell_appear(lv_obj_t *cell)
{
    /* Scale animation using transform */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, cell);
    lv_anim_set_values(&a, 128, 256);  /* 0.5x to 1x scale */
    lv_anim_set_duration(&a, SPAWN_ANIM_MS);
    lv_anim_set_exec_cb(&a, anim_scale_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    lv_anim_start(&a);
}
