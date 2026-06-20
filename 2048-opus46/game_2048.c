/**
 * @file game_2048.c
 * @brief Complete 2048 game implementation using LVGL v9 native widgets
 *
 * Features:
 * - Mouse gesture and keyboard arrow key controls
 * - Score tracking with real-time display
 * - Win (reach 2048) / Lose (no moves) detection
 * - New Game reset functionality
 * - Smooth tile movement and spawn animations
 * - Optimized for 320x480 embedded screens
 */

/*********************
 *      INCLUDES
 *********************/
#include "game_2048.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/*********************
 *      DEFINES
 *********************/
#define GRID_SIZE       4
#define TILE_SIZE       62
#define TILE_GAP        6
#define BOARD_PAD       8
#define BOARD_SIZE      (TILE_SIZE * GRID_SIZE + TILE_GAP * (GRID_SIZE + 1))
#define ANIM_MOVE_TIME  120   /* ms for tile slide animation */
#define ANIM_POP_TIME   100   /* ms for new tile pop animation */

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    uint16_t board[GRID_SIZE][GRID_SIZE];
    uint32_t score;
    bool     game_over;
    bool     game_won;
    bool     win_shown;           /* only show win dialog once */

    /* UI elements */
    lv_obj_t *parent;
    lv_obj_t *score_label;
    lv_obj_t *board_cont;
    lv_obj_t *tiles[GRID_SIZE][GRID_SIZE];      /* tile background objects */
    lv_obj_t *tile_labels[GRID_SIZE][GRID_SIZE]; /* number labels */
    lv_obj_t *overlay;            /* win/lose overlay */
} game_state_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static game_state_t g_game;
static bool g_rand_seeded = false;

/**********************
 * COLOR PALETTE
 **********************/
static lv_color_t tile_color(uint16_t val)
{
    switch(val) {
        case 0:    return lv_color_hex(0xCDC1B4);
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
        default:   return lv_color_hex(0x3C3A32);
    }
}

static lv_color_t tile_text_color(uint16_t val)
{
    if(val <= 4) return lv_color_hex(0x776E65);
    return lv_color_hex(0xF9F6F2);
}

static const lv_font_t * tile_font(uint16_t val)
{
    if(val < 100)   return &lv_font_montserrat_24;
    if(val < 1000)  return &lv_font_montserrat_20;
    return &lv_font_montserrat_16;
}

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void     game_init(void);
static void     reset_board(void);
static void     spawn_tile(void);
static bool     move_left(void);
static bool     move_right(void);
static bool     move_up(void);
static bool     move_down(void);
static bool     can_move(void);
static bool     check_win(void);
static void     update_ui(void);
static void     update_tile(int r, int c);
static void     show_overlay(const char *msg);
static void     hide_overlay(void);
static void     do_move(lv_dir_t dir);

/* Animation helpers */
static void     anim_tile_pop(int r, int c);

/* Event callbacks */
static void     gesture_cb(lv_event_t *e);
static void     key_cb(lv_event_t *e);
static void     new_game_cb(lv_event_t *e);
static void     overlay_btn_cb(lv_event_t *e);

/**********************
 *   IMPLEMENTATION
 **********************/

/**
 * @brief Seed the random number generator (once)
 */
static void ensure_rand_seeded(void)
{
    if(!g_rand_seeded) {
        srand((unsigned int)time(NULL));
        g_rand_seeded = true;
    }
}

/**
 * @brief Reset the board data (no UI update)
 */
static void reset_board(void)
{
    memset(g_game.board, 0, sizeof(g_game.board));
    g_game.score     = 0;
    g_game.game_over = false;
    g_game.game_won  = false;
    g_game.win_shown = false;
}

/**
 * @brief Full game init: reset + spawn two tiles + update UI
 */
static void game_init(void)
{
    reset_board();
    spawn_tile();
    spawn_tile();
    update_ui();
}

/**
 * @brief Spawn a new tile (2 with 90% probability, 4 with 10%) in a random empty cell
 */
static void spawn_tile(void)
{
    ensure_rand_seeded();

    /* Count empty cells */
    int empty[GRID_SIZE * GRID_SIZE][2];
    int count = 0;
    for(int r = 0; r < GRID_SIZE; r++) {
        for(int c = 0; c < GRID_SIZE; c++) {
            if(g_game.board[r][c] == 0) {
                empty[count][0] = r;
                empty[count][1] = c;
                count++;
            }
        }
    }
    if(count == 0) return;

    int idx = rand() % count;
    int r = empty[idx][0];
    int c = empty[idx][1];
    g_game.board[r][c] = (rand() % 10 < 9) ? 2 : 4;

    /* Animate the new tile after UI update */
    anim_tile_pop(r, c);
}

/*----------- MERGE LOGIC -----------*/

/**
 * @brief Slide and merge a single row to the left (in-place)
 * @return true if anything changed
 */
static bool merge_row_left(uint16_t row[GRID_SIZE], uint32_t *score_add)
{
    uint16_t orig[GRID_SIZE];
    memcpy(orig, row, sizeof(orig));

    /* 1) Compact: remove zeros */
    uint16_t tmp[GRID_SIZE] = {0};
    int pos = 0;
    for(int i = 0; i < GRID_SIZE; i++) {
        if(row[i] != 0) tmp[pos++] = row[i];
    }

    /* 2) Merge adjacent equal tiles */
    for(int i = 0; i < GRID_SIZE - 1; i++) {
        if(tmp[i] != 0 && tmp[i] == tmp[i + 1]) {
            tmp[i] *= 2;
            *score_add += tmp[i];
            tmp[i + 1] = 0;
        }
    }

    /* 3) Compact again */
    pos = 0;
    for(int i = 0; i < GRID_SIZE; i++) {
        if(tmp[i] != 0) row[pos++] = tmp[i];
    }
    for(; pos < GRID_SIZE; pos++) row[pos] = 0;

    return memcmp(orig, row, sizeof(orig)) != 0;
}

static bool move_left(void)
{
    bool changed = false;
    for(int r = 0; r < GRID_SIZE; r++) {
        uint32_t add = 0;
        if(merge_row_left(g_game.board[r], &add)) changed = true;
        g_game.score += add;
    }
    return changed;
}

static bool move_right(void)
{
    bool changed = false;
    for(int r = 0; r < GRID_SIZE; r++) {
        /* Reverse, merge left, reverse back */
        uint16_t rev[GRID_SIZE];
        for(int c = 0; c < GRID_SIZE; c++) rev[c] = g_game.board[r][GRID_SIZE - 1 - c];
        uint32_t add = 0;
        if(merge_row_left(rev, &add)) {
            changed = true;
            for(int c = 0; c < GRID_SIZE; c++) g_game.board[r][GRID_SIZE - 1 - c] = rev[c];
        }
        g_game.score += add;
    }
    return changed;
}

static bool move_up(void)
{
    bool changed = false;
    for(int c = 0; c < GRID_SIZE; c++) {
        uint16_t col[GRID_SIZE];
        for(int r = 0; r < GRID_SIZE; r++) col[r] = g_game.board[r][c];
        uint32_t add = 0;
        if(merge_row_left(col, &add)) {
            changed = true;
            for(int r = 0; r < GRID_SIZE; r++) g_game.board[r][c] = col[r];
        }
        g_game.score += add;
    }
    return changed;
}

static bool move_down(void)
{
    bool changed = false;
    for(int c = 0; c < GRID_SIZE; c++) {
        uint16_t col[GRID_SIZE];
        for(int r = 0; r < GRID_SIZE; r++) col[r] = g_game.board[GRID_SIZE - 1 - r][c];
        uint32_t add = 0;
        if(merge_row_left(col, &add)) {
            changed = true;
            for(int r = 0; r < GRID_SIZE; r++) g_game.board[GRID_SIZE - 1 - r][c] = col[r];
        }
        g_game.score += add;
    }
    return changed;
}

/**
 * @brief Check if any move is possible
 */
static bool can_move(void)
{
    for(int r = 0; r < GRID_SIZE; r++) {
        for(int c = 0; c < GRID_SIZE; c++) {
            if(g_game.board[r][c] == 0) return true;
            if(c < GRID_SIZE - 1 && g_game.board[r][c] == g_game.board[r][c + 1]) return true;
            if(r < GRID_SIZE - 1 && g_game.board[r][c] == g_game.board[r + 1][c]) return true;
        }
    }
    return false;
}

/**
 * @brief Check if any tile has reached 2048
 */
static bool check_win(void)
{
    for(int r = 0; r < GRID_SIZE; r++) {
        for(int c = 0; c < GRID_SIZE; c++) {
            if(g_game.board[r][c] >= 2048) return true;
        }
    }
    return false;
}

/*----------- UI UPDATE -----------*/

static void update_tile(int r, int c)
{
    uint16_t val = g_game.board[r][c];
    lv_obj_t *tile  = g_game.tiles[r][c];
    lv_obj_t *label = g_game.tile_labels[r][c];

    /* Background color */
    lv_obj_set_style_bg_color(tile, tile_color(val), 0);

    if(val == 0) {
        lv_label_set_text(label, "");
    } else {
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%d", val);
        lv_label_set_text(label, buf);
        lv_obj_set_style_text_color(label, tile_text_color(val), 0);
        lv_obj_set_style_text_font(label, tile_font(val), 0);
    }
}

static void update_ui(void)
{
    /* Update score */
    char buf[32];
    lv_snprintf(buf, sizeof(buf), "Score: %u", (unsigned)g_game.score);
    lv_label_set_text(g_game.score_label, buf);

    /* Update all tiles */
    for(int r = 0; r < GRID_SIZE; r++) {
        for(int c = 0; c < GRID_SIZE; c++) {
            update_tile(r, c);
        }
    }
}

/*----------- ANIMATION -----------*/

/**
 * @brief Scale-pop animation callback (transform scale)
 */
static void anim_scale_cb(void *obj, int32_t v)
{
    lv_obj_set_style_transform_scale(obj, v, 0);
}

static void anim_tile_pop(int r, int c)
{
    lv_obj_t *tile = g_game.tiles[r][c];
    if(tile == NULL) return;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, tile);
    lv_anim_set_values(&a, 50, 256);   /* scale from ~20% to 100% (256 = 1.0x) */
    lv_anim_set_time(&a, ANIM_POP_TIME);
    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    lv_anim_set_exec_cb(&a, anim_scale_cb);
    lv_anim_start(&a);
}

/**
 * @brief Fade-in animation for overlays
 */
static void anim_opa_cb(void *obj, int32_t v)
{
    lv_obj_set_style_opa(obj, (lv_opa_t)v, 0);
}

/*----------- GAME MOVE -----------*/

static void do_move(lv_dir_t dir)
{
    if(g_game.game_over) return;

    bool changed = false;
    switch(dir) {
        case LV_DIR_LEFT:  changed = move_left();  break;
        case LV_DIR_RIGHT: changed = move_right(); break;
        case LV_DIR_TOP:   changed = move_up();    break;
        case LV_DIR_BOTTOM:changed = move_down();  break;
        default: return;
    }

    if(!changed) return;

    update_ui();
    spawn_tile();
    update_ui();

    /* Check win */
    if(check_win() && !g_game.win_shown) {
        g_game.game_won  = true;
        g_game.win_shown = true;
        show_overlay("You Win!");
        return;
    }

    /* Check lose */
    if(!can_move()) {
        g_game.game_over = true;
        show_overlay("Game Over!");
    }
}

/*----------- EVENT HANDLERS -----------*/

static void gesture_cb(lv_event_t *e)
{
    (void)e;
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    do_move(dir);
}

static void key_cb(lv_event_t *e)
{
    uint32_t key = lv_event_get_key(e);
    lv_dir_t dir = 0;
    switch(key) {
        case LV_KEY_LEFT:  dir = LV_DIR_LEFT;   break;
        case LV_KEY_RIGHT: dir = LV_DIR_RIGHT;  break;
        case LV_KEY_UP:    dir = LV_DIR_TOP;    break;
        case LV_KEY_DOWN:  dir = LV_DIR_BOTTOM; break;
        default: return;
    }
    do_move(dir);
}

static void new_game_cb(lv_event_t *e)
{
    (void)e;
    hide_overlay();
    game_init();
}

static void overlay_btn_cb(lv_event_t *e)
{
    (void)e;
    /* If won, allow continue playing */
    if(g_game.game_won && !g_game.game_over) {
        hide_overlay();
        return;
    }
    /* Otherwise reset */
    hide_overlay();
    game_init();
}

/*----------- OVERLAY -----------*/

static void show_overlay(const char *msg)
{
    if(g_game.overlay != NULL) {
        lv_obj_delete(g_game.overlay);
        g_game.overlay = NULL;
    }

    /* Semi-transparent overlay covering the board */
    g_game.overlay = lv_obj_create(g_game.parent);
    lv_obj_set_size(g_game.overlay, lv_pct(100), lv_pct(100));
    lv_obj_center(g_game.overlay);
    lv_obj_set_style_bg_color(g_game.overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(g_game.overlay, LV_OPA_0, 0);
    lv_obj_set_style_border_width(g_game.overlay, 0, 0);
    lv_obj_set_style_radius(g_game.overlay, 0, 0);
    lv_obj_set_flex_flow(g_game.overlay, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(g_game.overlay, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(g_game.overlay, 20, 0);

    /* Fade in */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, g_game.overlay);
    lv_anim_set_values(&a, LV_OPA_0, LV_OPA_70);
    lv_anim_set_time(&a, 300);
    lv_anim_set_exec_cb(&a, anim_opa_cb);
    lv_anim_start(&a);

    /* Message label */
    lv_obj_t *lbl = lv_label_create(g_game.overlay);
    lv_label_set_text(lbl, msg);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_28, 0);

    /* Button */
    lv_obj_t *btn = lv_button_create(g_game.overlay);
    lv_obj_set_size(btn, 160, 44);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x8F7A66), 0);
    lv_obj_set_style_radius(btn, 6, 0);
    lv_obj_add_event_cb(btn, overlay_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_lbl = lv_label_create(btn);
    if(g_game.game_won && !g_game.game_over) {
        lv_label_set_text(btn_lbl, "Continue");
    } else {
        lv_label_set_text(btn_lbl, "Try Again");
    }
    lv_obj_set_style_text_color(btn_lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(btn_lbl, &lv_font_montserrat_16, 0);
    lv_obj_center(btn_lbl);
}

static void hide_overlay(void)
{
    if(g_game.overlay != NULL) {
        lv_obj_delete(g_game.overlay);
        g_game.overlay = NULL;
    }
}

/*----------- PUBLIC API -----------*/

void game_2048_create(lv_obj_t *parent)
{
    memset(&g_game, 0, sizeof(g_game));
    g_game.parent = parent;

    /* Root screen style */
    lv_obj_set_style_bg_color(parent, lv_color_hex(0xFAF8EF), 0);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(parent, 10, 0);
    lv_obj_set_style_pad_row(parent, 8, 0);

    /*--- Header row ---*/
    lv_obj_t *header = lv_obj_create(parent);
    lv_obj_set_size(header, BOARD_SIZE + 2 * BOARD_PAD, 60);
    lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);
    lv_obj_set_scrollbar_mode(header, LV_SCROLLBAR_MODE_OFF);

    /* Title */
    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "2048");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x776E65), 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 4, 0);

    /* Score */
    g_game.score_label = lv_label_create(header);
    lv_label_set_text(g_game.score_label, "Score: 0");
    lv_obj_set_style_text_font(g_game.score_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(g_game.score_label, lv_color_hex(0x776E65), 0);
    lv_obj_align(g_game.score_label, LV_ALIGN_TOP_RIGHT, -4, 4);

    /* New Game button */
    lv_obj_t *btn = lv_button_create(header);
    lv_obj_set_size(btn, 90, 28);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x8F7A66), 0);
    lv_obj_set_style_radius(btn, 4, 0);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -4, -2);
    lv_obj_add_event_cb(btn, new_game_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_lbl = lv_label_create(btn);
    lv_label_set_text(btn_lbl, "New Game");
    lv_obj_set_style_text_color(btn_lbl, lv_color_hex(0xF9F6F2), 0);
    lv_obj_set_style_text_font(btn_lbl, &lv_font_montserrat_12, 0);
    lv_obj_center(btn_lbl);

    /*--- Game board ---*/
    g_game.board_cont = lv_obj_create(parent);
    lv_obj_set_size(g_game.board_cont, BOARD_SIZE + 2 * BOARD_PAD, BOARD_SIZE + 2 * BOARD_PAD);
    lv_obj_set_style_bg_color(g_game.board_cont, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_radius(g_game.board_cont, 8, 0);
    lv_obj_set_style_border_width(g_game.board_cont, 0, 0);
    lv_obj_set_style_pad_all(g_game.board_cont, BOARD_PAD, 0);
    lv_obj_set_scrollbar_mode(g_game.board_cont, LV_SCROLLBAR_MODE_OFF);

    /* Create tiles */
    for(int r = 0; r < GRID_SIZE; r++) {
        for(int c = 0; c < GRID_SIZE; c++) {
            int x = TILE_GAP + c * (TILE_SIZE + TILE_GAP);
            int y = TILE_GAP + r * (TILE_SIZE + TILE_GAP);

            lv_obj_t *tile = lv_obj_create(g_game.board_cont);
            lv_obj_set_size(tile, TILE_SIZE, TILE_SIZE);
            lv_obj_set_pos(tile, x, y);
            lv_obj_set_style_radius(tile, 4, 0);
            lv_obj_set_style_border_width(tile, 0, 0);
            lv_obj_set_style_bg_color(tile, tile_color(0), 0);
            lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, 0);
            lv_obj_set_scrollbar_mode(tile, LV_SCROLLBAR_MODE_OFF);
            lv_obj_set_style_pad_all(tile, 0, 0);
            /* Set transform pivot to center for pop animation */
            lv_obj_set_style_transform_pivot_x(tile, TILE_SIZE / 2, 0);
            lv_obj_set_style_transform_pivot_y(tile, TILE_SIZE / 2, 0);
            lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t *lbl = lv_label_create(tile);
            lv_label_set_text(lbl, "");
            lv_obj_center(lbl);

            g_game.tiles[r][c]       = tile;
            g_game.tile_labels[r][c] = lbl;
        }
    }

    /* Gesture event on board */
    lv_obj_add_event_cb(g_game.board_cont, gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(g_game.board_cont, LV_OBJ_FLAG_GESTURE_BUBBLE);

    /* Keyboard event — add to parent so it gets key events */
    lv_obj_add_event_cb(parent, key_cb, LV_EVENT_KEY, NULL);
    lv_group_add_obj(lv_group_get_default(), parent);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);

    /*--- Instructions label ---*/
    lv_obj_t *hint = lv_label_create(parent);
    lv_label_set_text(hint, LV_SYMBOL_LEFT " " LV_SYMBOL_RIGHT " " LV_SYMBOL_UP " " LV_SYMBOL_DOWN "  or swipe to play");
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(0x9E9486), 0);

    /* Initialize game */
    game_init();
}
