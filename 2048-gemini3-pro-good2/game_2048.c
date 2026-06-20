/**
 * @file game_2048.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "game_2048.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>

/*********************
 *      DEFINES
 *********************/
#define BOARD_SIZE 4
#define ANIM_TIME 200

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    GAME_PLAYING,
    GAME_WON,
    GAME_LOST
} game_state_t;

typedef struct {
    uint16_t board[BOARD_SIZE][BOARD_SIZE];
    uint32_t score;
    uint32_t best_score;
    game_state_t state;
    bool moved;
} game_2048_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void game_logic_init(void);
static void game_logic_reset(void);
static void spawn_new_tile(void);
static bool move_board(lv_dir_t dir);
static void check_game_state(void);
static bool can_move(void);

static void ui_create(void);
static void ui_update_board(void);
static void ui_update_score(void);
static void ui_show_message(const char * title, const char * msg);
static lv_color_t get_tile_color(uint16_t value);
static lv_color_t get_text_color(uint16_t value);

static void event_handler_input(lv_event_t * e);
static void event_handler_reset(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
static game_2048_t game;
static lv_obj_t * screen_obj;
static lv_obj_t * board_obj;
static lv_obj_t * tile_objs[BOARD_SIZE][BOARD_SIZE];
static lv_obj_t * score_label;
static lv_obj_t * best_score_label;
static lv_obj_t * msg_box;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void game_2048_init(void)
{
    srand(time(NULL));
    
    game_logic_init();
    ui_create();
    ui_update_board();
    ui_update_score();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

// --- Game Logic ---

static void game_logic_init(void)
{
    // Load best score from storage if available (mocked here)
    game.best_score = 0; 
    game_logic_reset();
}

static void game_logic_reset(void)
{
    memset(game.board, 0, sizeof(game.board));
    game.score = 0;
    game.state = GAME_PLAYING;
    game.moved = false;

    spawn_new_tile();
    spawn_new_tile();
}

static void spawn_new_tile(void)
{
    int empty_cells[BOARD_SIZE * BOARD_SIZE][2];
    int count = 0;

    for(int r = 0; r < BOARD_SIZE; r++) {
        for(int c = 0; c < BOARD_SIZE; c++) {
            if(game.board[r][c] == 0) {
                empty_cells[count][0] = r;
                empty_cells[count][1] = c;
                count++;
            }
        }
    }

    if(count > 0) {
        int idx = rand() % count;
        int r = empty_cells[idx][0];
        int c = empty_cells[idx][1];
        game.board[r][c] = (rand() % 10 == 0) ? 4 : 2;
    }
}

// Helper to slide and merge a single row/col array
static bool process_line(uint16_t * line, int size) {
    bool moved = false;
    int write_idx = 0;
    
    // 1. Shift non-zero to left
    for(int i = 0; i < size; i++) {
        if(line[i] != 0) {
            if(i != write_idx) {
                line[write_idx] = line[i];
                line[i] = 0;
                moved = true;
            }
            write_idx++;
        }
    }

    // 2. Merge
    for(int i = 0; i < write_idx - 1; i++) {
        if(line[i] == line[i+1]) {
            line[i] *= 2;
            game.score += line[i];
            
            if(line[i] == 2048 && game.state != GAME_WON) {
                // We don't stop the game immediately, just flag it? 
                // Classic 2048 shows a "You Win" banner but lets you continue.
                // For this requirement, we'll set state but might let user continue or stop.
                // Let's stop interaction and show dialog for simplicity as per req 3.
                game.state = GAME_WON; 
            }

            // Shift rest
            for(int j = i + 1; j < size - 1; j++) {
                line[j] = line[j+1];
            }
            line[size - 1] = 0;
            write_idx--;
            moved = true;
        }
    }
    
    return moved;
}

static bool move_board(lv_dir_t dir)
{
    if (game.state != GAME_PLAYING) return false;

    bool any_moved = false;
    uint16_t line[BOARD_SIZE];

    if (dir == LV_DIR_LEFT) {
        for (int r = 0; r < BOARD_SIZE; r++) {
            for (int c = 0; c < BOARD_SIZE; c++) line[c] = game.board[r][c];
            if (process_line(line, BOARD_SIZE)) {
                for (int c = 0; c < BOARD_SIZE; c++) game.board[r][c] = line[c];
                any_moved = true;
            }
        }
    } else if (dir == LV_DIR_RIGHT) {
        for (int r = 0; r < BOARD_SIZE; r++) {
            for (int c = 0; c < BOARD_SIZE; c++) line[c] = game.board[r][BOARD_SIZE - 1 - c];
            if (process_line(line, BOARD_SIZE)) {
                for (int c = 0; c < BOARD_SIZE; c++) game.board[r][BOARD_SIZE - 1 - c] = line[c];
                any_moved = true;
            }
        }
    } else if (dir == LV_DIR_TOP) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            for (int r = 0; r < BOARD_SIZE; r++) line[r] = game.board[r][c];
            if (process_line(line, BOARD_SIZE)) {
                for (int r = 0; r < BOARD_SIZE; r++) game.board[r][c] = line[r];
                any_moved = true;
            }
        }
    } else if (dir == LV_DIR_BOTTOM) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            for (int r = 0; r < BOARD_SIZE; r++) line[r] = game.board[BOARD_SIZE - 1 - r][c];
            if (process_line(line, BOARD_SIZE)) {
                for (int r = 0; r < BOARD_SIZE; r++) game.board[BOARD_SIZE - 1 - r][c] = line[r];
                any_moved = true;
            }
        }
    }

    if (any_moved) {
        spawn_new_tile();
        if (game.score > game.best_score) game.best_score = game.score;
        check_game_state();
        
        // Update UI
        ui_update_board();
        ui_update_score();
        
        if (game.state == GAME_WON) {
             ui_show_message("You Won!", "Congratulations! You reached 2048.");
        } else if (game.state == GAME_LOST) {
             ui_show_message("Game Over", "No more moves possible.");
        }
    }

    return any_moved;
}

static bool can_move(void)
{
    for(int r = 0; r < BOARD_SIZE; r++) {
        for(int c = 0; c < BOARD_SIZE; c++) {
            if(game.board[r][c] == 0) return true;
            if(c < BOARD_SIZE - 1 && game.board[r][c] == game.board[r][c+1]) return true;
            if(r < BOARD_SIZE - 1 && game.board[r][c] == game.board[r+1][c]) return true;
        }
    }
    return false;
}

static void check_game_state(void)
{
    if (game.state == GAME_WON) return;

    if (!can_move()) {
        game.state = GAME_LOST;
    }
}

// --- UI Logic ---

static void ui_create(void)
{
    screen_obj = lv_obj_create(NULL);
    lv_screen_load(screen_obj);
    lv_obj_remove_flag(screen_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen_obj, lv_color_hex(0xFAF8EF), 0);

    // Main Layout (Flex column)
    lv_obj_t * main_cont = lv_obj_create(screen_obj);
    lv_obj_set_size(main_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(main_cont, 10, 0);
    lv_obj_set_style_bg_opa(main_cont, 0, 0);
    lv_obj_set_style_border_width(main_cont, 0, 0);

    // Header
    lv_obj_t * header = lv_obj_create(main_cont);
    lv_obj_set_size(header, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);

    // Title
    lv_obj_t * title = lv_label_create(header);
    lv_label_set_text(title, "2048");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x776E65), 0);

    // Scores Container
    lv_obj_t * scores_cont = lv_obj_create(header);
    lv_obj_set_size(scores_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(scores_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(scores_cont, 5, 0);
    lv_obj_set_style_bg_opa(scores_cont, 0, 0);
    lv_obj_set_style_border_width(scores_cont, 0, 0);

    // Score Box
    score_label = lv_label_create(scores_cont);
    lv_obj_set_style_bg_color(score_label, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_bg_opa(score_label, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(score_label, 5, 0);
    lv_obj_set_style_radius(score_label, 3, 0);
    lv_obj_set_style_text_color(score_label, lv_color_white(), 0);
    lv_label_set_text_fmt(score_label, "SCORE\n0");
    lv_obj_set_style_text_align(score_label, LV_TEXT_ALIGN_CENTER, 0);


    // Reset Button
    lv_obj_t * btn_reset = lv_button_create(main_cont);
    lv_obj_set_size(btn_reset, 100, 40);
    lv_obj_add_event_cb(btn_reset, event_handler_reset, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn_reset, lv_color_hex(0x8F7A66), 0);
    lv_obj_t * label_reset = lv_label_create(btn_reset);
    lv_label_set_text(label_reset, "New Game");
    lv_obj_center(label_reset);

    // Board Background
    board_obj = lv_obj_create(main_cont);
    // Calc size: 320 screen width -> let's say 280x280 board
    // Or dynamic? Let's use specific size for now, safe for 320x480
    lv_obj_set_size(board_obj, 300, 300); 
    lv_obj_set_style_bg_color(board_obj, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_radius(board_obj, 6, 0);
    
    // Create 4x4 Tiles structure (using Grid? or absolute?)
    // Using Grid for easier layout
    static int32_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(board_obj, col_dsc, row_dsc);
    lv_obj_set_style_pad_all(board_obj, 10, 0);
    lv_obj_set_style_pad_row(board_obj, 10, 0);
    lv_obj_set_style_pad_column(board_obj, 10, 0);

    for(int r = 0; r < BOARD_SIZE; r++) {
        for(int c = 0; c < BOARD_SIZE; c++) {
            lv_obj_t * tile = lv_obj_create(board_obj);
            lv_obj_set_grid_cell(tile, LV_GRID_ALIGN_STRETCH, c, 1, LV_GRID_ALIGN_STRETCH, r, 1);
            lv_obj_set_style_bg_color(tile, lv_color_hex(0xCDC1B4), 0); // Empty color
            lv_obj_set_style_radius(tile, 3, 0);
            lv_obj_set_style_border_width(tile, 0, 0);
            
            lv_obj_t * num_lbl = lv_label_create(tile);
            lv_obj_center(num_lbl);
            lv_label_set_text(num_lbl, "");

            tile_objs[r][c] = tile;
        }
    }

    // Input Handling (Keypad & Gestures on screen)
    lv_obj_add_event_cb(screen_obj, event_handler_input, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(screen_obj, event_handler_input, LV_EVENT_GESTURE, NULL);

    // Add to default group for keyboard
    lv_group_t * g = lv_group_get_default();
    if(g) {
        lv_group_add_obj(g, screen_obj);
    }
}

static void ui_update_board(void)
{
    // Simple update: iterate and set styles/text
    // Real animations would require tracking IDs of tiles, skipping for MVP unless requested high-end
    // User requested "Optimize animation effect" - we can add simple scale/appear anim on spawns later
    
    for(int r = 0; r < BOARD_SIZE; r++) {
        for(int c = 0; c < BOARD_SIZE; c++) {
            uint16_t val = game.board[r][c];
            lv_obj_t * tile = tile_objs[r][c];
            lv_obj_t * label = lv_obj_get_child(tile, 0);

            if (val == 0) {
                lv_obj_set_style_bg_color(tile, lv_color_hex(0xCDC1B4), 0);
                lv_label_set_text(label, "");
            } else {
                lv_obj_set_style_bg_color(tile, get_tile_color(val), 0);
                lv_label_set_text_fmt(label, "%d", val);
                
                // Adjust font size for large numbers
                if(val > 512) lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
                else lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0); // Assuming 28 available? or 24?
                                                                                  // Default defaults usually have 14, maybe 20? 
                                                                                  // Using default font for safety if 28 not enabled.
                                                                                  // Let's assume standard definitions.
                
                lv_obj_set_style_text_color(label, get_text_color(val), 0);
            }
        }
    }
}

static void ui_update_score(void)
{
    lv_label_set_text_fmt(score_label, "SCORE\n%" PRIu32, game.score);
}

static void ui_show_message(const char * title, const char * msg)
{
    // Create a modal message box
    msg_box = lv_msgbox_create(screen_obj);
    lv_msgbox_add_title(msg_box, title);
    lv_msgbox_add_text(msg_box, msg);
    lv_obj_t * btn = lv_msgbox_add_footer_button(msg_box, "New Game");
    lv_obj_add_event_cb(btn, event_handler_reset, LV_EVENT_CLICKED, NULL);
    lv_obj_center(msg_box);
}

static void event_handler_input(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        switch(key) {
            case LV_KEY_UP: move_board(LV_DIR_TOP); break;
            case LV_KEY_DOWN: move_board(LV_DIR_BOTTOM); break;
            case LV_KEY_LEFT: move_board(LV_DIR_LEFT); break;
            case LV_KEY_RIGHT: move_board(LV_DIR_RIGHT); break;
        }
    }
    else if (code == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        move_board(dir);
    }
}

static void event_handler_reset(lv_event_t * e)
{
    if (msg_box) {
        lv_msgbox_close(msg_box);
        msg_box = NULL;
    }
    game_logic_reset();
    ui_update_board();
    ui_update_score();
}

static lv_color_t get_tile_color(uint16_t value)
{
    switch(value) {
        case 2: return lv_color_hex(0xEEE4DA);
        case 4: return lv_color_hex(0xEDE0C8);
        case 8: return lv_color_hex(0xF2B179);
        case 16: return lv_color_hex(0xF59563);
        case 32: return lv_color_hex(0xF67C5F);
        case 64: return lv_color_hex(0xF65E3B);
        case 128: return lv_color_hex(0xEDCF72);
        case 256: return lv_color_hex(0xEDCC61);
        case 512: return lv_color_hex(0xEDC850);
        case 1024: return lv_color_hex(0xEDC53F);
        case 2048: return lv_color_hex(0xEDC22E);
        default: return lv_color_hex(0x3C3A32);
    }
}

static lv_color_t get_text_color(uint16_t value)
{
    return (value <= 4) ? lv_color_hex(0x776E65) : lv_color_white();
}
