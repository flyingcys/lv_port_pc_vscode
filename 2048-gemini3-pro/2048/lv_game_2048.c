/**
 * @file lv_game_2048.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_game_2048.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/*********************
 *      DEFINES
 *********************/
#define TILE_SIZE 60
#define TILE_GAP 10
#define ANIM_TIME 150

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void game_init(void);
static void create_ui(void);
static void update_ui(void);
static void spawn_new_tile(void);
static void event_handler(lv_event_t * e);
static void reset_btn_event_handler(lv_event_t * e);
static bool move_board(int dx, int dy);
static int get_tile_color(int value);
static void check_game_state(void);
static void show_message(const char * msg);

/**********************
 *  STATIC VARIABLES
 **********************/
static game_2048_t game;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_game_2048_start(void)
{
    srand(time(NULL));
    game_init();
    create_ui();
    update_ui();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void game_init(void)
{
    for(int i = 0; i < MATRIX_SIZE; i++) {
        for(int j = 0; j < MATRIX_SIZE; j++) {
            game.board[i][j] = 0;
        }
    }
    game.score = 0;
    game.state = GAME_STATE_PLAYING;
    spawn_new_tile();
    spawn_new_tile();
}

static void create_ui(void)
{
    /* Create main container */
    game.ui_root = lv_obj_create(lv_screen_active());
    lv_obj_set_size(game.ui_root, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(game.ui_root, lv_color_hex(0xFAF8EF), 0);
    lv_obj_add_flag(game.ui_root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(game.ui_root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(game.ui_root, event_handler, LV_EVENT_ALL, NULL);
    lv_group_add_obj(lv_group_get_default(), game.ui_root);

    /* Score label */
    game.score_label = lv_label_create(game.ui_root);
    lv_label_set_text_fmt(game.score_label, "Score: %d", game.score);
    lv_obj_align(game.score_label, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_text_font(game.score_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(game.score_label, lv_color_hex(0x776E65), 0);

    /* Reset button */
    lv_obj_t * btn = lv_button_create(game.ui_root);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -20, 20);
    lv_obj_add_event_cb(btn, reset_btn_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Reset");

    /* Game board container */
    lv_obj_t * board_cont = lv_obj_create(game.ui_root);
    lv_obj_set_size(board_cont, MATRIX_SIZE * TILE_SIZE + (MATRIX_SIZE + 1) * TILE_GAP, 
                                MATRIX_SIZE * TILE_SIZE + (MATRIX_SIZE + 1) * TILE_GAP);
    lv_obj_align(board_cont, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_color(board_cont, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_radius(board_cont, 6, 0);
    lv_obj_remove_flag(board_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(board_cont, LV_OBJ_FLAG_CLICKABLE); /* Ensure it doesn't consume clicks */

    /* Create grid tiles */
    for(int i = 0; i < MATRIX_SIZE; i++) {
        for(int j = 0; j < MATRIX_SIZE; j++) {
            lv_obj_t * tile = lv_obj_create(board_cont);
            lv_obj_set_size(tile, TILE_SIZE, TILE_SIZE);
            lv_obj_set_pos(tile, TILE_GAP + j * (TILE_SIZE + TILE_GAP), 
                                 TILE_GAP + i * (TILE_SIZE + TILE_GAP));
            lv_obj_set_style_bg_color(tile, lv_color_hex(0xCDC1B4), 0);
            lv_obj_set_style_radius(tile, 3, 0);
            lv_obj_remove_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_remove_flag(tile, LV_OBJ_FLAG_CLICKABLE); /* Ensure it doesn't consume clicks */
            
            lv_obj_t * val_label = lv_label_create(tile);
            lv_obj_center(val_label);
            lv_label_set_text(val_label, "");
            
            game.tile_objs[i][j] = tile;
        }
    }
}

static void update_ui(void)
{
    lv_label_set_text_fmt(game.score_label, "Score: %d", game.score);

    for(int i = 0; i < MATRIX_SIZE; i++) {
        for(int j = 0; j < MATRIX_SIZE; j++) {
            lv_obj_t * tile = game.tile_objs[i][j];
            int val = game.board[i][j];
            
            lv_obj_set_style_bg_color(tile, lv_color_hex(get_tile_color(val)), 0);
            
            lv_obj_t * label = lv_obj_get_child(tile, 0);
            if(val > 0) {
                lv_label_set_text_fmt(label, "%d", val);
                if(val <= 4) lv_obj_set_style_text_color(label, lv_color_hex(0x776E65), 0);
                else lv_obj_set_style_text_color(label, lv_color_hex(0xF9F6F2), 0);
            } else {
                lv_label_set_text(label, "");
            }
        }
    }
}

static void spawn_new_tile(void)
{
    int empty_cells[MATRIX_SIZE * MATRIX_SIZE][2];
    int count = 0;

    for(int i = 0; i < MATRIX_SIZE; i++) {
        for(int j = 0; j < MATRIX_SIZE; j++) {
            if(game.board[i][j] == 0) {
                empty_cells[count][0] = i;
                empty_cells[count][1] = j;
                count++;
            }
        }
    }

    if(count > 0) {
        int idx = rand() % count;
        int r = empty_cells[idx][0];
        int c = empty_cells[idx][1];
        game.board[r][c] = (rand() % 10 < 9) ? 2 : 4;
    }
}

static bool move_board(int dx, int dy)
{
    bool moved = false;
    int merged[MATRIX_SIZE][MATRIX_SIZE] = {0};

    /* Helper to traverse grid in correct order */
    int start_i = (dx > 0) ? MATRIX_SIZE - 1 : 0;
    int end_i = (dx > 0) ? -1 : MATRIX_SIZE;
    int step_i = (dx > 0) ? -1 : 1;

    int start_j = (dy > 0) ? MATRIX_SIZE - 1 : 0;
    int end_j = (dy > 0) ? -1 : MATRIX_SIZE;
    int step_j = (dy > 0) ? -1 : 1;

    /* Logic for moving tiles */
    /* This is a simplified version, handling one direction at a time would be cleaner but this is generic */
    /* Actually, let's split by direction for clarity */
    
    if (dx != 0) { // Vertical move
        for (int j = 0; j < MATRIX_SIZE; j++) {
            for (int i = start_i; i != end_i; i += step_i) {
                if (game.board[i][j] == 0) continue;
                
                int r = i;
                while (true) {
                    int next_r = r + dx;
                    if (next_r < 0 || next_r >= MATRIX_SIZE) break;
                    
                    if (game.board[next_r][j] == 0) {
                        game.board[next_r][j] = game.board[r][j];
                        game.board[r][j] = 0;
                        r = next_r;
                        moved = true;
                    } else if (game.board[next_r][j] == game.board[r][j] && !merged[next_r][j]) {
                        game.board[next_r][j] *= 2;
                        game.score += game.board[next_r][j];
                        game.board[r][j] = 0;
                        merged[next_r][j] = 1;
                        moved = true;
                        break;
                    } else {
                        break;
                    }
                }
            }
        }
    } else { // Horizontal move
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = start_j; j != end_j; j += step_j) {
                if (game.board[i][j] == 0) continue;
                
                int c = j;
                while (true) {
                    int next_c = c + dy;
                    if (next_c < 0 || next_c >= MATRIX_SIZE) break;
                    
                    if (game.board[i][next_c] == 0) {
                        game.board[i][next_c] = game.board[i][c];
                        game.board[i][c] = 0;
                        c = next_c;
                        moved = true;
                    } else if (game.board[i][next_c] == game.board[i][c] && !merged[i][next_c]) {
                        game.board[i][next_c] *= 2;
                        game.score += game.board[i][next_c];
                        game.board[i][c] = 0;
                        merged[i][next_c] = 1;
                        moved = true;
                        break;
                    } else {
                        break;
                    }
                }
            }
        }
    }

    return moved;
}

static void check_game_state(void)
{
    bool won = false;
    bool full = true;
    bool can_merge = false;

    for(int i = 0; i < MATRIX_SIZE; i++) {
        for(int j = 0; j < MATRIX_SIZE; j++) {
            if(game.board[i][j] == 2048) won = true;
            if(game.board[i][j] == 0) full = false;
            
            // Check neighbors for merge
            if(i < MATRIX_SIZE - 1 && game.board[i][j] == game.board[i+1][j]) can_merge = true;
            if(j < MATRIX_SIZE - 1 && game.board[i][j] == game.board[i][j+1]) can_merge = true;
        }
    }

    if(won && game.state != GAME_STATE_WON) {
        game.state = GAME_STATE_WON;
        show_message("You Win!");
    } else if(full && !can_merge && game.state != GAME_STATE_LOST) {
        game.state = GAME_STATE_LOST;
        show_message("Game Over!");
    }
}

static void show_message(const char * msg)
{
    lv_obj_t * mbox = lv_msgbox_create(game.ui_root);
    lv_msgbox_add_title(mbox, "Info");
    lv_msgbox_add_text(mbox, msg);
    lv_msgbox_add_close_button(mbox);
}

static lv_point_t touch_start_point;

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    bool moved = false;

    if(game.state != GAME_STATE_PLAYING) return;

    if(code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        if(key == LV_KEY_UP) moved = move_board(-1, 0);
        else if(key == LV_KEY_DOWN) moved = move_board(1, 0);
        else if(key == LV_KEY_LEFT) moved = move_board(0, -1);
        else if(key == LV_KEY_RIGHT) moved = move_board(0, 1);
    }
    else if(code == LV_EVENT_PRESSED) {
        lv_indev_t * indev = lv_indev_active();
        if(indev) {
            lv_indev_get_point(indev, &touch_start_point);
        }
    }
    else if(code == LV_EVENT_RELEASED) {
        lv_indev_t * indev = lv_indev_active();
        if(indev) {
            lv_point_t touch_end_point;
            lv_indev_get_point(indev, &touch_end_point);

            lv_point_t diff;
            diff.x = touch_end_point.x - touch_start_point.x;
            diff.y = touch_end_point.y - touch_start_point.y;

            int32_t abs_x = LV_ABS(diff.x);
            int32_t abs_y = LV_ABS(diff.y);

            if(LV_MAX(abs_x, abs_y) > 30) { // Threshold
                if(abs_x > abs_y) {
                    if(diff.x > 0) moved = move_board(0, 1);
                    else moved = move_board(0, -1);
                } else {
                    if(diff.y > 0) moved = move_board(1, 0);
                    else moved = move_board(-1, 0);
                }
            }
        }
    }

    if(moved) {
        spawn_new_tile();
        update_ui();
        check_game_state();
    }
}

static void reset_btn_event_handler(lv_event_t * e)
{
    game_init();
    update_ui();
}

static int get_tile_color(int value)
{
    switch(value) {
        case 0: return 0xCDC1B4;
        case 2: return 0xEEE4DA;
        case 4: return 0xEDE0C8;
        case 8: return 0xF2B179;
        case 16: return 0xF59563;
        case 32: return 0xF67C5F;
        case 64: return 0xF65E3B;
        case 128: return 0xEDCF72;
        case 256: return 0xEDCC61;
        case 512: return 0xEDC850;
        case 1024: return 0xEDC53F;
        case 2048: return 0xEDC22E;
        default: return 0x3C3A32;
    }
}
