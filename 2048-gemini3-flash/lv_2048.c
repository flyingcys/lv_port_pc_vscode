#include "lv_2048.h"
#include <stdlib.h>
#include <time.h>

/*********************
 *      DEFINES
 *********************/
#define BOARD_SIZE 4
#define TILE_MARGIN 10
#define TILE_SIZE 60
#define ANIM_TIME 200

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_obj_t * obj;
    lv_obj_t * label;
    uint32_t value;
    int x, y; // board coordinates
} tile_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static uint32_t board[BOARD_SIZE][BOARD_SIZE];
static tile_t * tiles[BOARD_SIZE][BOARD_SIZE];
static lv_obj_t * board_obj;
static lv_obj_t * score_label;
static uint32_t score = 0;
static bool game_over = false;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void create_ui(void);
static void add_random_tile(void);
static bool move_tiles(int dx, int dy);
static void update_tile_ui(int x, int y, bool animated);
static void event_cb(lv_event_t * e);
static void reset_game(void);
static bool check_game_over(void);
static void show_message(const char * msg);
static lv_color_t get_tile_color(uint32_t value);
static lv_color_t get_tile_text_color(uint32_t value);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_2048_game_init(void) {
    srand(time(NULL));
    create_ui();
    reset_game();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void create_ui(void) {
    lv_obj_t * screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_palette_main(LV_PALETTE_GREY), 0);

    // Score Label
    score_label = lv_label_create(screen);
    lv_label_set_text(score_label, "Score: 0");
    lv_obj_align(score_label, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_text_font(score_label, &lv_font_montserrat_24, 0);

    // Board Container
    board_obj = lv_obj_create(screen);
    lv_obj_set_size(board_obj, BOARD_SIZE * TILE_SIZE + (BOARD_SIZE + 1) * TILE_MARGIN, 
                               BOARD_SIZE * TILE_SIZE + (BOARD_SIZE + 1) * TILE_MARGIN);
    lv_obj_align(board_obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(board_obj, lv_color_hex(0xbbada0), 0);
    lv_obj_set_style_pad_all(board_obj, TILE_MARGIN, 0);
    lv_obj_set_style_radius(board_obj, 10, 0);
    lv_obj_remove_flag(board_obj, LV_OBJ_FLAG_SCROLLABLE);

    // Background Slots
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            lv_obj_t * slot = lv_obj_create(board_obj);
            lv_obj_set_size(slot, TILE_SIZE, TILE_SIZE);
            lv_obj_set_pos(slot, j * (TILE_SIZE + TILE_MARGIN), i * (TILE_SIZE + TILE_MARGIN));
            lv_obj_set_style_bg_color(slot, lv_color_hex(0xcdc1b4), 0);
            lv_obj_set_style_radius(slot, 5, 0);
            lv_obj_set_style_border_width(slot, 0, 0);
        }
    }

    // Reset Button
    lv_obj_t * btn = lv_button_create(screen);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "New Game");
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);

    // Controls
    lv_obj_add_event_cb(board_obj, event_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(screen, event_cb, LV_EVENT_GESTURE, NULL);
    
    lv_group_t * g = lv_group_get_default();
    if(g) {
        lv_group_add_obj(g, board_obj);
        lv_group_focus_obj(board_obj);
    }
}

static void reset_game(void) {
    score = 0;
    game_over = false;
    lv_label_set_text_fmt(score_label, "Score: %d", score);

    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = 0;
            if(tiles[i][j]) {
                lv_obj_delete(tiles[i][j]->obj);
                free(tiles[i][j]);
                tiles[i][j] = NULL;
            }
        }
    }

    add_random_tile();
    add_random_tile();
}

static void anim_set_scale(void * var, int32_t v) {
    lv_obj_set_style_transform_scale((lv_obj_t *)var, v, 0);
}

static void show_message(const char * msg) {
    lv_obj_t * overlay = lv_obj_create(lv_screen_active());
    lv_obj_set_size(overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_50, 0);

    lv_obj_t * label = lv_label_create(overlay);
    lv_label_set_text(label, msg);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
    lv_obj_center(label);

    lv_obj_t * btn = lv_button_create(overlay);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 60);
    lv_obj_t * btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Restart");
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);
}

static void add_random_tile(void) {
    int empty_slots[BOARD_SIZE * BOARD_SIZE];
    int count = 0;

    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            if(board[i][j] == 0) {
                empty_slots[count++] = i * BOARD_SIZE + j;
            }
        }
    }

    if(count > 0) {
        int index = empty_slots[rand() % count];
        int r = index / BOARD_SIZE;
        int c = index % BOARD_SIZE;
        board[r][c] = (rand() % 10 == 0) ? 4 : 2;

        tile_t * tile = malloc(sizeof(tile_t));
        tile->value = board[r][c];
        tile->x = c;
        tile->y = r;
        tile->obj = lv_obj_create(board_obj);
        lv_obj_set_size(tile->obj, TILE_SIZE, TILE_SIZE);
        lv_obj_set_style_radius(tile->obj, 5, 0);
        lv_obj_set_style_border_width(tile->obj, 0, 0);
        lv_obj_remove_flag(tile->obj, LV_OBJ_FLAG_SCROLLABLE);

        tile->label = lv_label_create(tile->obj);
        lv_obj_center(tile->label);
        
        tiles[r][c] = tile;
        update_tile_ui(c, r, true);

        // Appear animation
        lv_obj_set_style_transform_scale(tile->obj, 0, 0);
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, tile->obj);
        lv_anim_set_values(&a, 0, 256); // 256 = 1.0
        lv_anim_set_duration(&a, ANIM_TIME);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)anim_set_scale);
        lv_anim_start(&a);
    }
}

static void update_tile_ui(int x, int y, bool animated) {
    tile_t * tile = tiles[y][x];
    if(!tile) return;

    lv_label_set_text_fmt(tile->label, "%"PRIu32, tile->value);
    lv_obj_set_style_bg_color(tile->obj, get_tile_color(tile->value), 0);
    lv_obj_set_style_text_color(tile->label, get_tile_text_color(tile->value), 0);

    if(tile->value >= 1000) lv_obj_set_style_text_font(tile->label, &lv_font_montserrat_14, 0);
    else if(tile->value >= 100) lv_obj_set_style_text_font(tile->label, &lv_font_montserrat_18, 0);
    else lv_obj_set_style_text_font(tile->label, &lv_font_montserrat_24, 0);

    int pos_x = x * (TILE_SIZE + TILE_MARGIN);
    int pos_y = y * (TILE_SIZE + TILE_MARGIN);

    if(animated) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, tile->obj);
        lv_anim_set_values(&a, lv_obj_get_x(tile->obj), pos_x);
        lv_anim_set_duration(&a, ANIM_TIME);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_start(&a);

        lv_anim_set_values(&a, lv_obj_get_y(tile->obj), pos_y);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_start(&a);
    } else {
        lv_obj_set_pos(tile->obj, pos_x, pos_y);
    }
}

static bool move_tiles(int dx, int dy) {
    bool moved = false;
    bool merged[BOARD_SIZE][BOARD_SIZE] = { {false} };

    int start_x = (dx > 0) ? BOARD_SIZE - 1 : 0;
    int start_y = (dy > 0) ? BOARD_SIZE - 1 : 0;
    int end_x = (dx > 0) ? -1 : BOARD_SIZE;
    int end_y = (dy > 0) ? -1 : BOARD_SIZE;
    int step_x = (dx > 0) ? -1 : 1;
    int step_y = (dy > 0) ? -1 : 1;

    for(int i = start_y; i != end_y; i += step_y) {
        for(int j = start_x; j != end_x; j += step_x) {
            if(board[i][j] == 0) continue;

            int next_i = i + dy;
            int next_j = j + dx;
            int current_i = i;
            int current_j = j;

            while(next_i >= 0 && next_i < BOARD_SIZE && next_j >= 0 && next_j < BOARD_SIZE) {
                if(board[next_i][next_j] == 0) {
                    // Move
                    board[next_i][next_j] = board[current_i][current_j];
                    board[current_i][current_j] = 0;
                    tiles[next_i][next_j] = tiles[current_i][current_j];
                    tiles[current_i][current_j] = NULL;
                    
                    tiles[next_i][next_j]->x = next_j;
                    tiles[next_i][next_j]->y = next_i;
                    
                    current_i = next_i;
                    current_j = next_j;
                    next_i += dy;
                    next_j += dx;
                    moved = true;
                } else if(board[next_i][next_j] == board[current_i][current_j] && !merged[next_i][next_j]) {
                    // Merge
                    board[next_i][next_j] *= 2;
                    board[current_i][current_j] = 0;
                    score += board[next_i][next_j];
                    merged[next_i][next_j] = true;

                    lv_obj_delete(tiles[next_i][next_j]->obj);
                    free(tiles[next_i][next_j]);
                    
                    tiles[next_i][next_j] = tiles[current_i][current_j];
                    tiles[current_i][current_j] = NULL;
                    tiles[next_i][next_j]->x = next_j;
                    tiles[next_i][next_j]->y = next_i;
                    tiles[next_i][next_j]->value = board[next_i][next_j];

                    moved = true;
                    break;
                } else {
                    break;
                }
            }
        }
    }

    if(moved) {
        for(int i = 0; i < BOARD_SIZE; i++) {
            for(int j = 0; j < BOARD_SIZE; j++) {
                if(tiles[i][j]) update_tile_ui(j, i, true);
            }
        }
        lv_label_set_text_fmt(score_label, "Score: %d", score);
        add_random_tile();
        if(check_game_over()) {
            game_over = true;
            show_message("Game Over!");
        }
    }

    return moved;
}

static bool check_game_over(void) {
    bool win = false;
    bool can_move = false;
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            if(board[i][j] == 2048) win = true;
            if(board[i][j] == 0) can_move = true;
            if(i < BOARD_SIZE - 1 && board[i][j] == board[i+1][j]) can_move = true;
            if(j < BOARD_SIZE - 1 && board[i][j] == board[i][j+1]) can_move = true;
        }
    }

    if(win) {
        show_message("You Win!");
        game_over = true;
        return true;
    }
    return false;
}

static void event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        reset_game();
    } else if(code == LV_EVENT_KEY) {
        if(game_over) return;
        uint32_t key = lv_event_get_key(e);
        LV_LOG_USER("Key pressed: %d", (int)key);
        switch(key) {
            case LV_KEY_UP:    move_tiles(0, -1); break;
            case LV_KEY_DOWN:  move_tiles(0, 1);  break;
            case LV_KEY_LEFT:  move_tiles(-1, 0); break;
            case LV_KEY_RIGHT: move_tiles(1, 0);  break;
        }
    } else if(code == LV_EVENT_GESTURE) {
        if(game_over) return;
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        LV_LOG_USER("Gesture detected: %d", (int)dir);
        switch(dir) {
            case LV_DIR_TOP:    move_tiles(0, -1); break;
            case LV_DIR_BOTTOM: move_tiles(0, 1);  break;
            case LV_DIR_LEFT:   move_tiles(-1, 0); break;
            case LV_DIR_RIGHT:  move_tiles(1, 0);  break;
        }
    }
}

static lv_color_t get_tile_color(uint32_t value) {
    switch(value) {
        case 2:    return lv_color_hex(0xeee4da);
        case 4:    return lv_color_hex(0xede0c8);
        case 8:    return lv_color_hex(0xf2b179);
        case 16:   return lv_color_hex(0xf59563);
        case 32:   return lv_color_hex(0xf67c5f);
        case 64:   return lv_color_hex(0xf65e3b);
        case 128:  return lv_color_hex(0xedcf72);
        case 256:  return lv_color_hex(0xedcc61);
        case 512:  return lv_color_hex(0xedc850);
        case 1024: return lv_color_hex(0xedc53f);
        case 2048: return lv_color_hex(0xedc22e);
        default:   return lv_color_hex(0x3c3a32);
    }
}

static lv_color_t get_tile_text_color(uint32_t value) {
    if(value <= 4) return lv_color_hex(0x776e65);
    return lv_color_hex(0xf9f6f2);
}
