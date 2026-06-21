#include "tetris.h"
#include <time.h>
#include <stdlib.h>

/*********************
 *      DEFINES
 *********************/
#define BOARD_COLOR 0x1A1A1A
#define TILE_GAP 1

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    int shape[4][4];
    lv_color_t color;
} tetromino_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static const tetromino_t tetrominos[7] = {
    {{{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}}, {0, 255, 255}}, // I
    {{{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}, {0, 0, 255}},   // J
    {{{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}, {255, 165, 0}}, // L
    {{{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}, {255, 255, 0}}, // O
    {{{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}}, {0, 255, 0}},   // S
    {{{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}, {128, 0, 128}}, // T
    {{{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}, {255, 0, 0}}    // Z
};

static uint8_t board[GRID_HEIGHT][GRID_WIDTH];
static lv_obj_t * tiles[GRID_HEIGHT][GRID_WIDTH];
static int cur_piece_type;
static int cur_piece_rot[4][4];
static int cur_x, cur_y;
static int next_piece_type;
static int score;
static tetris_state_t game_state = GAME_STATE_START;
static lv_timer_t * game_timer;

static lv_obj_t * ui_root;
static lv_obj_t * ui_board;
static lv_obj_t * ui_score_label;
static lv_obj_t * ui_next_preview;
static lv_obj_t * next_tiles[4][4];

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void spawn_piece(void);
static bool check_collision(int tx, int ty, int trot[4][4]);
static void rotate_piece(void);
static void lock_piece(void);
static void clear_lines(void);
static void update_board_ui(void);
static void game_tick_cb(lv_timer_t * t);
static void key_event_cb(lv_event_t * e);
static void start_game(void);

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void spawn_piece(void) {
    cur_piece_type = next_piece_type;
    next_piece_type = rand() % 7;
    
    for(int i=0; i<4; i++) {
        for(int j=0; j<4; j++) {
            cur_piece_rot[i][j] = tetrominos[cur_piece_type].shape[i][j];
        }
    }
    
    cur_x = GRID_WIDTH / 2 - 2;
    cur_y = 0;
    
    if(check_collision(cur_x, cur_y, cur_piece_rot)) {
        game_state = GAME_STATE_OVER;
        lv_timer_pause(game_timer);
        lv_label_set_text(ui_score_label, "GAME OVER\nPress Space");
    }
    
    // Update next piece preview
    for(int i=0; i<4; i++) {
        for(int j=0; j<4; j++) {
            if(tetrominos[next_piece_type].shape[i][j]) {
                lv_obj_set_style_bg_color(next_tiles[i][j], tetrominos[next_piece_type].color, 0);
                lv_obj_set_style_bg_opa(next_tiles[i][j], LV_OPA_COVER, 0);
            } else {
                lv_obj_set_style_bg_opa(next_tiles[i][j], LV_OPA_TRANSP, 0);
            }
        }
    }
}

static bool check_collision(int tx, int ty, int trot[4][4]) {
    for(int i=0; i<4; i++) {
        for(int j=0; j<4; j++) {
            if(trot[i][j]) {
                int bx = tx + j;
                int by = ty + i;
                if(bx < 0 || bx >= GRID_WIDTH || by >= GRID_HEIGHT) return true;
                if(by >= 0 && board[by][bx]) return true;
            }
        }
    }
    return false;
}

static void rotate_piece(void) {
    int next_rot[4][4];
    for(int i=0; i<4; i++) {
        for(int j=0; j<4; j++) {
            next_rot[j][3-i] = cur_piece_rot[i][j];
        }
    }
    
    if(!check_collision(cur_x, cur_y, next_rot)) {
        for(int i=0; i<4; i++) {
            for(int j=0; j<4; j++) {
                cur_piece_rot[i][j] = next_rot[i][j];
            }
        }
    }
}

static void lock_piece(void) {
    for(int i=0; i<4; i++) {
        for(int j=0; j<4; j++) {
            if(cur_piece_rot[i][j]) {
                int bx = cur_x + j;
                int by = cur_y + i;
                if(by >= 0 && by < GRID_HEIGHT && bx >= 0 && bx < GRID_WIDTH) {
                    board[by][bx] = cur_piece_type + 1;
                }
            }
        }
    }
    clear_lines();
    spawn_piece();
}

static void clear_lines(void) {
    int lines_cleared = 0;
    for(int i=GRID_HEIGHT-1; i>=0; i--) {
        bool full = true;
        for(int j=0; j<GRID_WIDTH; j++) {
            if(!board[i][j]) {
                full = false;
                break;
            }
        }
        if(full) {
            lines_cleared++;
            for(int k=i; k>0; k--) {
                for(int j=0; j<GRID_WIDTH; j++) {
                    board[k][j] = board[k-1][j];
                }
            }
            for(int j=0; j<GRID_WIDTH; j++) board[0][j] = 0;
            i++; // Check the same row again
        }
    }
    
    if(lines_cleared == 1) score += 100;
    else if(lines_cleared == 2) score += 300;
    else if(lines_cleared == 3) score += 600;
    else if(lines_cleared == 4) score += 1000;
    
    char buf[32];
    lv_snprintf(buf, sizeof(buf), "SCORE: %d", score);
    lv_label_set_text(ui_score_label, buf);
}

static void update_board_ui(void) {
    // Clear dynamic tiles
    for(int i=0; i<GRID_HEIGHT; i++) {
        for(int j=0; j<GRID_WIDTH; j++) {
            if(board[i][j]) {
                lv_obj_set_style_bg_color(tiles[i][j], tetrominos[board[i][j]-1].color, 0);
                lv_obj_set_style_bg_opa(tiles[i][j], LV_OPA_COVER, 0);
            } else {
                lv_obj_set_style_bg_opa(tiles[i][j], LV_OPA_TRANSP, 0);
            }
        }
    }
    
    // Draw current piece
    if(game_state == GAME_STATE_PLAYING) {
        for(int i=0; i<4; i++) {
            for(int j=0; j<4; j++) {
                if(cur_piece_rot[i][j]) {
                    int bx = cur_x + j;
                    int by = cur_y + i;
                    if(bx >= 0 && bx < GRID_WIDTH && by >= 0 && by < GRID_HEIGHT) {
                        lv_obj_set_style_bg_color(tiles[by][bx], tetrominos[cur_piece_type].color, 0);
                        lv_obj_set_style_bg_opa(tiles[by][bx], LV_OPA_COVER, 0);
                    }
                }
            }
        }
    }
}

static void game_tick_cb(lv_timer_t * t) {
    if(game_state != GAME_STATE_PLAYING) return;
    
    if(!check_collision(cur_x, cur_y + 1, cur_piece_rot)) {
        cur_y++;
    } else {
        lock_piece();
    }
    update_board_ui();
}

static void key_event_cb(lv_event_t * e) {
    uint32_t key = lv_event_get_key(e);
    
    if(game_state == GAME_STATE_OVER || game_state == GAME_STATE_START) {
        if(key == LV_KEY_ENTER || key == ' ') {
            start_game();
        }
        return;
    }
    
    if(key == ' ') {
        if(game_state == GAME_STATE_PLAYING) {
            game_state = GAME_STATE_PAUSED;
            lv_timer_pause(game_timer);
            lv_label_set_text(ui_score_label, "PAUSED\nPress Space");
        } else if(game_state == GAME_STATE_PAUSED) {
            game_state = GAME_STATE_PLAYING;
            lv_timer_resume(game_timer);
            char buf[32];
            lv_snprintf(buf, sizeof(buf), "SCORE: %d", score);
            lv_label_set_text(ui_score_label, buf);
        }
        return;
    }
    
    if(game_state != GAME_STATE_PLAYING) return;
    
    switch(key) {
        case LV_KEY_LEFT:
            if(!check_collision(cur_x - 1, cur_y, cur_piece_rot)) cur_x--;
            break;
        case LV_KEY_RIGHT:
            if(!check_collision(cur_x + 1, cur_y, cur_piece_rot)) cur_x++;
            break;
        case LV_KEY_DOWN:
            if(!check_collision(cur_x, cur_y + 1, cur_piece_rot)) cur_y++;
            break;
        case LV_KEY_UP:
            rotate_piece();
            break;
    }
    update_board_ui();
}

static void start_game(void) {
    for(int i=0; i<GRID_HEIGHT; i++) {
        for(int j=0; j<GRID_WIDTH; j++) board[i][j] = 0;
    }
    score = 0;
    next_piece_type = rand() % 7;
    spawn_piece();
    game_state = GAME_STATE_PLAYING;
    lv_timer_resume(game_timer);
    char buf[32];
    lv_snprintf(buf, sizeof(buf), "SCORE: %d", score);
    lv_label_set_text(ui_score_label, buf);
}

void tetris_init(void) {
    srand(time(NULL));
    
    ui_root = lv_obj_create(lv_screen_active());
    lv_obj_set_size(ui_root, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(ui_root, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(ui_root, 0, 0);
    lv_obj_set_style_border_width(ui_root, 0, 0);
    lv_obj_clear_flag(ui_root, LV_OBJ_FLAG_SCROLLABLE);
    
    // Main Container
    lv_obj_t * cont = lv_obj_create(ui_root);
    lv_obj_set_size(cont, 280, 400);
    lv_obj_center(cont);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x333333), 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Left: Board
    ui_board = lv_obj_create(cont);
    lv_obj_set_size(ui_board, GRID_WIDTH * TILE_SIZE + 4, GRID_HEIGHT * TILE_SIZE + 4);
    lv_obj_set_style_bg_color(ui_board, lv_color_hex(BOARD_COLOR), 0);
    lv_obj_set_style_border_color(ui_board, lv_color_hex(0x888888), 0);
    lv_obj_set_style_border_width(ui_board, 2, 0);
    lv_obj_clear_flag(ui_board, LV_OBJ_FLAG_SCROLLABLE);
    
    for(int i=0; i<GRID_HEIGHT; i++) {
        for(int j=0; j<GRID_WIDTH; j++) {
            tiles[i][j] = lv_obj_create(ui_board);
            lv_obj_set_size(tiles[i][j], TILE_SIZE - TILE_GAP, TILE_SIZE - TILE_GAP);
            lv_obj_set_pos(tiles[i][j], j * TILE_SIZE, i * TILE_SIZE);
            lv_obj_set_style_radius(tiles[i][j], 2, 0);
            lv_obj_set_style_border_width(tiles[i][j], 0, 0);
            lv_obj_set_style_bg_opa(tiles[i][j], LV_OPA_TRANSP, 0);
        }
    }
    
    // Right: Info
    lv_obj_t * info_cont = lv_obj_create(cont);
    lv_obj_set_size(info_cont, 100, 320);
    lv_obj_set_style_bg_color(info_cont, lv_color_hex(0x444444), 0);
    lv_obj_set_flex_flow(info_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(info_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    ui_score_label = lv_label_create(info_cont);
    lv_label_set_text(ui_score_label, "PRESS START");
    lv_obj_set_style_text_color(ui_score_label, lv_color_hex(0xFFFFFF), 0);
    
    lv_label_create(info_cont); // Spacer
    lv_obj_t * next_label = lv_label_create(info_cont);
    lv_label_set_text(next_label, "NEXT:");
    lv_obj_set_style_text_color(next_label, lv_color_hex(0xAAAAAA), 0);
    
    ui_next_preview = lv_obj_create(info_cont);
    lv_obj_set_size(ui_next_preview, 4 * TILE_SIZE, 4 * TILE_SIZE);
    lv_obj_set_style_bg_color(ui_next_preview, lv_color_hex(BOARD_COLOR), 0);
    lv_obj_clear_flag(ui_next_preview, LV_OBJ_FLAG_SCROLLABLE);
    
    for(int i=0; i<4; i++) {
        for(int j=0; j<4; j++) {
            next_tiles[i][j] = lv_obj_create(ui_next_preview);
            lv_obj_set_size(next_tiles[i][j], TILE_SIZE - TILE_GAP, TILE_SIZE - TILE_GAP);
            lv_obj_set_pos(next_tiles[i][j], j * TILE_SIZE, i * TILE_SIZE);
            lv_obj_set_style_radius(next_tiles[i][j], 2, 0);
            lv_obj_set_style_border_width(next_tiles[i][j], 0, 0);
            lv_obj_set_style_bg_opa(next_tiles[i][j], LV_OPA_TRANSP, 0);
        }
    }
    
    // Buttons
    lv_obj_t * btn = lv_btn_create(info_cont);
    lv_obj_t * btn_lbl = lv_label_create(btn);
    lv_label_set_text(btn_lbl, "START");
    lv_obj_add_event_cb(btn, (lv_event_cb_t)start_game, LV_EVENT_CLICKED, NULL);
    
    // Input
    lv_obj_add_event_cb(ui_root, key_event_cb, LV_EVENT_KEY, NULL);
    lv_group_add_obj(lv_group_get_default(), ui_root);
    lv_group_focus_obj(ui_root);
    
    game_timer = lv_timer_create(game_tick_cb, 500, NULL);
    lv_timer_pause(game_timer);
}
