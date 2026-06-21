/**
 * @file tetris.c
 * @brief Tetris game implementation using LVGL v9
 */

#include "tetris.h"
#include "lvgl/lvgl.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/
#define BOARD_WIDTH  10
#define BOARD_HEIGHT 20
#define BLOCK_SIZE   20  /* Size of each block in pixels */

/* Colors */
#define COLOR_I lv_palette_main(LV_PALETTE_CYAN)
#define COLOR_J lv_palette_main(LV_PALETTE_BLUE)
#define COLOR_L lv_palette_main(LV_PALETTE_ORANGE)
#define COLOR_O lv_palette_main(LV_PALETTE_YELLOW)
#define COLOR_S lv_palette_main(LV_PALETTE_GREEN)
#define COLOR_T lv_palette_main(LV_PALETTE_PURPLE)
#define COLOR_Z lv_palette_main(LV_PALETTE_RED)
#define COLOR_G lv_color_make(0x30, 0x30, 0x30) /* Grid/Empty color */

/*********************
 *      TYPEDEFS
 *********************/
typedef enum {
    BLOCK_I = 0,
    BLOCK_J,
    BLOCK_L,
    BLOCK_O,
    BLOCK_S,
    BLOCK_T,
    BLOCK_Z,
    BLOCK_count
} block_type_t;

typedef struct {
    int8_t x, y;
} point_t;

typedef struct {
    point_t coords[4]; /* 4 blocks relative to center */
    lv_color_t color;
} tetromino_t;

typedef enum {
    GAME_IDLE,
    GAME_PLAYING,
    GAME_PAUSED,
    GAME_OVER
} game_status_t;

typedef struct {
    uint8_t board[BOARD_HEIGHT][BOARD_WIDTH]; /* 0: empty, 1-7: color index + 1 */
    
    struct {
        int x, y;           /* Position of the piece (top-left bounding box or pivot) */
        int type;           /* Current piece type */
        int rotation;       /* 0-3 */
        point_t blocks[4];  /* Current relative coordinates of the 4 blocks */
    } current_piece;

    struct {
        int type;
    } next_piece;

    uint32_t score;
    uint32_t level;
    uint32_t lines_cleared;
    game_status_t status;
    
    /* UI Objects */
    lv_obj_t * screen;
    lv_obj_t * game_area;
    lv_obj_t * score_label;
    lv_obj_t * next_piece_area;
    lv_obj_t * overlay;
    lv_obj_t * overlay_label;
    lv_timer_t * game_timer;
} game_ctx_t;

/*********************
 *  STATIC PROTOTYPES
 *********************/
static void spawn_new_piece(void);
static void game_timer_cb(lv_timer_t * timer);
static void input_event_cb(lv_event_t * e);
static void draw_board_cb(lv_event_t * e);
static void draw_next_piece_cb(lv_event_t * e);
static void update_score_ui(void);
static void show_overlay(const char * text);
static void hide_overlay(void);
static bool check_collision(int start_x, int start_y, point_t * blocks);
static void rotate_piece(void);
static void lock_piece(void);
static void check_lines(void);
static void reset_game(void);

/*********************
 *  STATIC VARIABLES
 *********************/
static game_ctx_t game;

/* Standard Tetromino definitions (offsets from a center point) */
/* We'll use a simplified model where we store base shapes and rotate them mathematically */
static const point_t TETROMINO_SHAPES[BLOCK_count][4] = {
    {{-1, 0}, {0, 0}, {1, 0}, {2, 0}}, /* I */
    {{-1, -1}, {-1, 0}, {0, 0}, {1, 0}}, /* J */
    {{1, -1}, {-1, 0}, {0, 0}, {1, 0}}, /* L */
    {{0, 0}, {1, 0}, {0, 1}, {1, 1}}, /* O */
    {{0, 0}, {1, 0}, {-1, 1}, {0, 1}}, /* S */
    {{-1, 0}, {0, 0}, {1, 0}, {0, 1}}, /* T */
    {{-1, 0}, {0, 0}, {0, 1}, {1, 1}}  /* Z */
};

static const lv_color_t PIECE_COLORS[BLOCK_count] = {
    {0x00, 0xFF, 0xFF}, /* I - Cyan (Approximation, corrected by macro below) */
    {0x00, 0x00, 0xFF}, /* J */
    {0xFF, 0xA5, 0x00}, /* L */
    {0xFF, 0xFF, 0x00}, /* O */
    {0x00, 0x80, 0x00}, /* S */
    {0x80, 0x00, 0x80}, /* T */
    {0xFF, 0x00, 0x00}  /* Z */
}; // Note: We will use the LVGL palette macros in the unified draw function for better colors

static lv_color_t get_piece_color(int type) {
    switch(type) {
        case BLOCK_I: return COLOR_I;
        case BLOCK_J: return COLOR_J;
        case BLOCK_L: return COLOR_L;
        case BLOCK_O: return COLOR_O;
        case BLOCK_S: return COLOR_S;
        case BLOCK_T: return COLOR_T;
        case BLOCK_Z: return COLOR_Z;
        default: return lv_color_white();
    }
}

/*********************
 *   GLOBAL FUNCTIONS
 *********************/

void tetris_init(void)
{
    srand(time(NULL));

    /* Create main screen */
    game.screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(game.screen, lv_color_hex(0x101010), 0);
    lv_screen_load(game.screen);

    /* Game Area (Board) */
    game.game_area = lv_obj_create(game.screen);
    lv_obj_set_size(game.game_area, BOARD_WIDTH * BLOCK_SIZE, BOARD_HEIGHT * BLOCK_SIZE);
    lv_obj_set_style_bg_color(game.game_area, lv_color_black(), 0);
    lv_obj_set_style_border_width(game.game_area, 2, 0);
    lv_obj_set_style_border_color(game.game_area, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_align(game.game_area, LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_add_event_cb(game.game_area, draw_board_cb, LV_EVENT_DRAW_MAIN, NULL);

    /* Info Panel */
    lv_obj_t * panel = lv_obj_create(game.screen);
    lv_obj_set_size(panel, 120, BOARD_HEIGHT * BLOCK_SIZE);
    lv_obj_align(panel, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_bg_opa(panel, 0, 0);
    lv_obj_set_style_border_width(panel, 0, 0);

    /* Score */
    lv_obj_t * score_title = lv_label_create(panel);
    lv_label_set_text(score_title, "SCORE");
    lv_obj_set_style_text_color(score_title, lv_color_white(), 0);
    lv_obj_align(score_title, LV_ALIGN_TOP_MID, 0, 20);

    game.score_label = lv_label_create(panel);
    lv_label_set_text(game.score_label, "0");
    lv_obj_set_style_text_color(game.score_label, lv_palette_main(LV_PALETTE_LIME), 0);
    lv_obj_set_style_text_font(game.score_label, &lv_font_montserrat_20, 0);
    lv_obj_align(game.score_label, LV_ALIGN_TOP_MID, 0, 50);

    /* Next Piece */
    lv_obj_t * next_title = lv_label_create(panel);
    lv_label_set_text(next_title, "NEXT");
    lv_obj_set_style_text_color(next_title, lv_color_white(), 0);
    lv_obj_align(next_title, LV_ALIGN_TOP_MID, 0, 100);

    game.next_piece_area = lv_obj_create(panel);
    lv_obj_set_size(game.next_piece_area, 80, 80);
    lv_obj_align(game.next_piece_area, LV_ALIGN_TOP_MID, 0, 130);
    lv_obj_set_style_bg_color(game.next_piece_area, lv_color_hex(0x202020), 0);
    lv_obj_add_event_cb(game.next_piece_area, draw_next_piece_cb, LV_EVENT_DRAW_MAIN, NULL);
    lv_obj_remove_flag(game.next_piece_area, LV_OBJ_FLAG_SCROLLABLE);

    /* Controls Hint */
    lv_obj_t * help_label = lv_label_create(panel);
    lv_label_set_text(help_label, "Controls:\n\nARROWS\nMove/Rot\n\nSPACE\nPause");
    lv_obj_set_style_text_color(help_label, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_align(help_label, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_text_align(help_label, LV_TEXT_ALIGN_CENTER, 0);

    /* Overlay */
    game.overlay = lv_obj_create(game.screen);
    lv_obj_set_size(game.overlay, 200, 100);
    lv_obj_center(game.overlay);
    lv_obj_set_style_bg_color(game.overlay, lv_color_black(), 0);
    lv_obj_set_style_border_color(game.overlay, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_border_width(game.overlay, 2, 0);
    
    game.overlay_label = lv_label_create(game.overlay);
    lv_obj_center(game.overlay_label);
    lv_obj_set_style_text_font(game.overlay_label, &lv_font_montserrat_20, 0);

    /* Input Handling using a Key Listener on the screen/group */
    lv_group_t * g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_t * indev = lv_indev_get_next(NULL);
    while(indev) {
        if(lv_indev_get_type(indev) == LV_INDEV_TYPE_KEYPAD || lv_indev_get_type(indev) == LV_INDEV_TYPE_ENCODER) {
            lv_indev_set_group(indev, g);
        }
        indev = lv_indev_get_next(indev);
    }
    
    /* Create a dummy object to catch keys if needed, or attach event to screen */
    lv_obj_add_event_cb(game.screen, input_event_cb, LV_EVENT_KEY, NULL);
    lv_group_add_obj(g, game.screen);
    lv_group_focus_obj(game.screen);

    /* Initialize Game State */
    reset_game();
    
    /* Timer */
    game.game_timer = lv_timer_create(game_timer_cb, 500, NULL);
    lv_timer_pause(game.game_timer);

    show_overlay("PRESS SPACE\nTO START");
}

/*********************
 *   STATIC FUNCTIONS
 *********************/

static void reset_game(void)
{
    memset(game.board, 0, sizeof(game.board));
    game.score = 0;
    game.level = 1;
    game.lines_cleared = 0;
    game.status = GAME_IDLE;
    game.next_piece.type = rand() % BLOCK_count;
    
    update_score_ui();
    lv_obj_invalidate(game.game_area);
}

static void start_game(void)
{
    reset_game();
    game.status = GAME_PLAYING;
    spawn_new_piece();
    lv_timer_resume(game.game_timer);
    hide_overlay();
}

static void pause_game(void)
{
    if (game.status == GAME_PLAYING) {
        game.status = GAME_PAUSED;
        lv_timer_pause(game.game_timer);
        show_overlay("PAUSED");
    } else if (game.status == GAME_PAUSED) {
        game.status = GAME_PLAYING;
        lv_timer_resume(game.game_timer);
        hide_overlay();
    }
}

static void game_over(void)
{
    game.status = GAME_OVER;
    lv_timer_pause(game.game_timer);
    char buf[32];
    snprintf(buf, sizeof(buf), "GAME OVER\nScore: %u", (unsigned int)game.score);
    show_overlay(buf);
}

static void show_overlay(const char * text)
{
    lv_label_set_text(game.overlay_label, text);
    lv_obj_remove_flag(game.overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(game.overlay);
}

static void hide_overlay(void)
{
    lv_obj_add_flag(game.overlay, LV_OBJ_FLAG_HIDDEN);
}

static void spawn_new_piece(void)
{
    game.current_piece.type = game.next_piece.type;
    game.next_piece.type = rand() % BLOCK_count;
    
    game.current_piece.x = BOARD_WIDTH / 2; /* Center roughly */
    game.current_piece.y = 0;
    game.current_piece.rotation = 0;
    
    /* Load shape */
    for(int i=0; i<4; i++) {
        game.current_piece.blocks[i] = TETROMINO_SHAPES[game.current_piece.type][i];
    }

    /* Check immediate collision (Game Over condition) */
    if (check_collision(game.current_piece.x, game.current_piece.y, game.current_piece.blocks)) {
        game_over();
    }

    lv_obj_invalidate(game.next_piece_area);
    lv_obj_invalidate(game.game_area);
}

static bool check_collision(int start_x, int start_y, point_t * blocks)
{
    for(int i=0; i<4; i++) {
        int x = start_x + blocks[i].x;
        int y = start_y + blocks[i].y;

        if (x < 0 || x >= BOARD_WIDTH || y >= BOARD_HEIGHT) return true;
        if (y >= 0 && game.board[y][x] != 0) return true;
    }
    return false;
}

static void rotate_piece(void)
{
    if (game.current_piece.type == BLOCK_O) return; /* O doesn't rotate */

    point_t new_blocks[4];
    for(int i=0; i<4; i++) {
        /* 90 degree rotation: (x, y) -> (-y, x) */
        new_blocks[i].x = -game.current_piece.blocks[i].y;
        new_blocks[i].y = game.current_piece.blocks[i].x;
    }

    if (!check_collision(game.current_piece.x, game.current_piece.y, new_blocks)) {
        for(int i=0; i<4; i++) game.current_piece.blocks[i] = new_blocks[i];
        lv_obj_invalidate(game.game_area);
    }
}

static void lock_piece(void)
{
    for(int i=0; i<4; i++) {
        int x = game.current_piece.x + game.current_piece.blocks[i].x;
        int y = game.current_piece.y + game.current_piece.blocks[i].y;
        
        if (x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT) {
            game.board[y][x] = game.current_piece.type + 1;
        }
    }
    check_lines();
    spawn_new_piece();
}

static void check_lines(void)
{
    int lines = 0;
    for(int y=BOARD_HEIGHT-1; y>=0; y--) {
        bool full = true;
        for(int x=0; x<BOARD_WIDTH; x++) {
            if (game.board[y][x] == 0) {
                full = false;
                break;
            }
        }

        if (full) {
            lines++;
            /* Move everything down */
            for(int k=y; k>0; k--) {
                for(int x=0; x<BOARD_WIDTH; x++) {
                    game.board[k][x] = game.board[k-1][x];
                }
            }
            /* Clear top row */
            for(int x=0; x<BOARD_WIDTH; x++) game.board[0][x] = 0;
            
            y++; /* Check same row again as it's now new content */
        }
    }

    if (lines > 0) {
        /* Scoring: 100, 300, 600, 1000 */
        uint32_t points = 0;
        switch(lines) {
            case 1: points = 100; break;
            case 2: points = 300; break;
            case 3: points = 600; break;
            case 4: points = 1000; break;
        }
        game.score += points;
        game.lines_cleared += lines;
        update_score_ui();
        
        /* Speed up */
        if (game.lines_cleared % 10 == 0) {
            game.level++;
            uint32_t period = 500 - (game.level * 20);
            if (period < 100) period = 100;
            lv_timer_set_period(game.game_timer, period);
        }
    }
}

static void input_event_cb(lv_event_t * e)
{
    uint32_t key = lv_event_get_key(e);
    
    if (key == LV_KEY_ESC) {
        /* Optional: Exit game */
    }
    else if (key == ' ' || key == LV_KEY_ENTER) {
        if (game.status == GAME_IDLE || game.status == GAME_OVER) {
            start_game();
        } else {
            pause_game();
        }
    }
    else if (game.status == GAME_PLAYING) {
        if (key == LV_KEY_UP) {
            rotate_piece();
        }
        else if (key == LV_KEY_LEFT) {
            if (!check_collision(game.current_piece.x - 1, game.current_piece.y, game.current_piece.blocks)) {
                game.current_piece.x--;
                lv_obj_invalidate(game.game_area);
            }
        }
        else if (key == LV_KEY_RIGHT) {
            if (!check_collision(game.current_piece.x + 1, game.current_piece.y, game.current_piece.blocks)) {
                game.current_piece.x++;
                lv_obj_invalidate(game.game_area);
            }
        }
        else if (key == LV_KEY_DOWN) {
            if (!check_collision(game.current_piece.x, game.current_piece.y + 1, game.current_piece.blocks)) {
                game.current_piece.y++;
                lv_obj_invalidate(game.game_area);
            } else {
                lock_piece();
            }
        }
    }
}

static void game_timer_cb(lv_timer_t * timer)
{
    (void)timer;
    if (game.status != GAME_PLAYING) return;

    if (!check_collision(game.current_piece.x, game.current_piece.y + 1, game.current_piece.blocks)) {
        game.current_piece.y++;
        lv_obj_invalidate(game.game_area);
    } else {
        lock_piece();
    }
}

static void draw_board_cb(lv_event_t * e)
{
    lv_layer_t * layer = lv_event_get_layer(e);
    lv_draw_rect_dsc_t draw_dsc;
    lv_draw_rect_dsc_init(&draw_dsc);
    
    /* Draw Fixed Blocks */
    lv_area_t obj_coords;
    lv_obj_get_coords(game.game_area, &obj_coords);

    for(int y=0; y<BOARD_HEIGHT; y++) {
        for(int x=0; x<BOARD_WIDTH; x++) {
            if (game.board[y][x] != 0) {
                draw_dsc.bg_color = get_piece_color(game.board[y][x] - 1);
                lv_area_t coords;
                coords.x1 = obj_coords.x1 + x * BLOCK_SIZE + 2; /* 2px border offset handled by parent align ideally, but here explicit */
                coords.y1 = obj_coords.y1 + y * BLOCK_SIZE + 2;
                coords.x2 = coords.x1 + BLOCK_SIZE - 2; /* -2 for grid gap */
                coords.y2 = coords.y1 + BLOCK_SIZE - 2;
                
                lv_draw_rect(layer, &draw_dsc, &coords);
            }
        }
    }

    /* Draw Active Piece */
    if (game.status == GAME_PLAYING || game.status == GAME_PAUSED) {
        draw_dsc.bg_color = get_piece_color(game.current_piece.type);
        for(int i=0; i<4; i++) {
            int x = game.current_piece.x + game.current_piece.blocks[i].x;
            int y = game.current_piece.y + game.current_piece.blocks[i].y;
            
            if (y >= 0) { /* Don't draw if above board */
                lv_area_t coords;
                coords.x1 = obj_coords.x1 + x * BLOCK_SIZE + 2;
                coords.y1 = obj_coords.y1 + y * BLOCK_SIZE + 2;
                coords.x2 = coords.x1 + BLOCK_SIZE - 2;
                coords.y2 = coords.y1 + BLOCK_SIZE - 2;
                
                lv_draw_rect(layer, &draw_dsc, &coords);
            }
        }
    }
}

static void draw_next_piece_cb(lv_event_t * e)
{
    lv_layer_t * layer = lv_event_get_layer(e);
    lv_draw_rect_dsc_t draw_dsc;
    lv_draw_rect_dsc_init(&draw_dsc);
    draw_dsc.bg_color = get_piece_color(game.next_piece.type);

    /* Center in the 80x80 box. Blocks are 20px */
    /* Base shape center is usually around 0,0. We need to shift to 40,40 */
    lv_area_t obj_coords;
    lv_obj_get_coords(game.next_piece_area, &obj_coords);
    int offset_x = obj_coords.x1 + 30; /* 30 to center 20px block approx */
    int offset_y = obj_coords.y1 + 30;

    for(int i=0; i<4; i++) {
        lv_area_t coords;
        point_t p = TETROMINO_SHAPES[game.next_piece.type][i];
        coords.x1 = offset_x + p.x * BLOCK_SIZE + 1;
        coords.y1 = offset_y + p.y * BLOCK_SIZE + 1;
        coords.x2 = coords.x1 + BLOCK_SIZE - 2;
        coords.y2 = coords.y1 + BLOCK_SIZE - 2;
        lv_draw_rect(layer, &draw_dsc, &coords);
    }
}

static void update_score_ui(void)
{
    lv_label_set_text_fmt(game.score_label, "%u", (unsigned int)game.score);
}
