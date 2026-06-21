// game_2048_ui.c - 2048 LVGL UI layer
#include "game_2048_ui.h"
#include "game_2048.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

/* ---- layout constants ---- */
#define TILE_SIZE   95
#define TILE_GAP    7
#define GRID_X      39    /* (480 - (4*95 + 3*7)) / 2 */
#define GRID_Y      80
#define HEADER_H    75

/* ---- game state ---- */
static game_2048_t game;
static lv_obj_t *tile_objs[4][4];     /* container objects for each cell */
static lv_obj_t *tile_labels[4][4];   /* labels inside containers */
static lv_obj_t *score_val_label;
static lv_obj_t *best_val_label;
static lv_obj_t *overlay;             /* win/lose overlay */
static lv_obj_t *overlay_label;

/* ---- forward decls ---- */
static void create_header(lv_obj_t *parent);
static void create_grid(lv_obj_t *parent);
static void create_overlay(lv_obj_t *screen);
static void sync_tiles_to_engine(void);
static void animate_move(int r, int c, int from_grid_x, int from_grid_y);
static void animate_merge(int r, int c);
static void animate_spawn(int r, int c);
static void do_move(game_dir_t dir);
static void show_overlay(const char *msg, lv_color_t color);
static void hide_overlay(void);
static void reset_game(void);
static void input_event_cb(lv_event_t *e);
static void reset_btn_cb(lv_event_t *e);

/* ---- public API ---- */

void game_2048_ui_init(lv_indev_t *keyboard_indev, lv_indev_t *mouse_indev)
{
    /* seed random from LVGL tick */
    srand(lv_tick_get());

    /* init engine */
    game_2048_init(&game);

    /* get active screen */
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFAF8EF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

    create_header(scr);
    create_grid(scr);
    create_overlay(scr);

    /* bind keyboard input */
    lv_indev_add_event_cb(keyboard_indev, input_event_cb, LV_EVENT_KEY, NULL);

    /* bind mouse gesture on screen */
    lv_obj_add_event_cb(scr, input_event_cb, LV_EVENT_GESTURE, NULL);

    /* initial render */
    sync_tiles_to_engine();
}

/* ---- header widget ---- */

static void create_header(lv_obj_t *parent)
{
    /* header background bar */
    lv_obj_t *header = lv_obj_create(parent);
    lv_obj_set_size(header, 480, HEADER_H);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(header, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(header, 0, LV_PART_MAIN);

    /* title "2048" */
    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "2048");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(0x776E65), LV_PART_MAIN);
    lv_obj_set_pos(title, 10, 10);

    /* best score box (right side) */
    lv_obj_t *best_box = lv_obj_create(header);
    lv_obj_set_size(best_box, 100, 55);
    lv_obj_set_pos(best_box, 365, 10);
    lv_obj_set_style_bg_color(best_box, lv_color_hex(0xBBADA0), LV_PART_MAIN);
    lv_obj_set_style_radius(best_box, 5, LV_PART_MAIN);
    lv_obj_set_style_border_width(best_box, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(best_box, 0, LV_PART_MAIN);

    lv_obj_t *best_title = lv_label_create(best_box);
    lv_label_set_text(best_title, "BEST");
    lv_obj_set_style_text_font(best_title, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(best_title, lv_color_hex(0xEEE4DA), LV_PART_MAIN);
    lv_obj_align(best_title, LV_ALIGN_TOP_MID, 0, 4);

    best_val_label = lv_label_create(best_box);
    lv_label_set_text(best_val_label, "0");
    lv_obj_set_style_text_font(best_val_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_color(best_val_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(best_val_label, LV_ALIGN_BOTTOM_MID, 0, -4);

    /* score box (next to best) */
    lv_obj_t *score_box = lv_obj_create(header);
    lv_obj_set_size(score_box, 100, 55);
    lv_obj_set_pos(score_box, 260, 10);
    lv_obj_set_style_bg_color(score_box, lv_color_hex(0xBBADA0), LV_PART_MAIN);
    lv_obj_set_style_radius(score_box, 5, LV_PART_MAIN);
    lv_obj_set_style_border_width(score_box, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(score_box, 0, LV_PART_MAIN);

    lv_obj_t *score_title = lv_label_create(score_box);
    lv_label_set_text(score_title, "SCORE");
    lv_obj_set_style_text_font(score_title, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(score_title, lv_color_hex(0xEEE4DA), LV_PART_MAIN);
    lv_obj_align(score_title, LV_ALIGN_TOP_MID, 0, 4);

    score_val_label = lv_label_create(score_box);
    lv_label_set_text(score_val_label, "0");
    lv_obj_set_style_text_font(score_val_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_color(score_val_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(score_val_label, LV_ALIGN_BOTTOM_MID, 0, -4);

    /* reset button */
    lv_obj_t *reset_btn = lv_button_create(header);
    lv_obj_set_size(reset_btn, 55, 55);
    lv_obj_set_pos(reset_btn, 420, 10);
    lv_obj_set_style_bg_color(reset_btn, lv_color_hex(0xBBADA0), LV_PART_MAIN);
    lv_obj_set_style_radius(reset_btn, 5, LV_PART_MAIN);

    lv_obj_t *reset_label = lv_label_create(reset_btn);
    lv_label_set_text(reset_label, LV_SYMBOL_REFRESH);
    lv_obj_set_style_text_font(reset_label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(reset_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(reset_label);

    lv_obj_add_event_cb(reset_btn, reset_btn_cb, LV_EVENT_CLICKED, NULL);
}

/* ---- grid widget ---- */

static void create_grid(lv_obj_t *parent)
{
    /* grid background */
    lv_obj_t *grid_bg = lv_obj_create(parent);
    int grid_size = 4 * TILE_SIZE + 3 * TILE_GAP;
    lv_obj_set_size(grid_bg, grid_size + 14, grid_size + 14);
    lv_obj_set_pos(grid_bg, GRID_X - 7, GRID_Y - 7);
    lv_obj_set_style_bg_color(grid_bg, lv_color_hex(0xBBADA0), LV_PART_MAIN);
    lv_obj_set_style_radius(grid_bg, 8, LV_PART_MAIN);
    lv_obj_set_style_border_width(grid_bg, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(grid_bg, 0, LV_PART_MAIN);

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int x = GRID_X + c * (TILE_SIZE + TILE_GAP);
            int y = GRID_Y + r * (TILE_SIZE + TILE_GAP);

            /* background cell (empty tile placeholder) */
            lv_obj_t *cell_bg = lv_obj_create(parent);
            lv_obj_set_size(cell_bg, TILE_SIZE, TILE_SIZE);
            lv_obj_set_pos(cell_bg, x, y);
            lv_obj_set_style_bg_color(cell_bg, lv_color_hex(0xCDC1B4), LV_PART_MAIN);
            lv_obj_set_style_radius(cell_bg, 5, LV_PART_MAIN);
            lv_obj_set_style_border_width(cell_bg, 0, LV_PART_MAIN);
            lv_obj_set_style_pad_all(cell_bg, 0, LV_PART_MAIN);

            /* tile container (on top, visible when cell is non-zero) */
            lv_obj_t *tile = lv_obj_create(parent);
            lv_obj_set_size(tile, TILE_SIZE, TILE_SIZE);
            lv_obj_set_pos(tile, x, y);
            lv_obj_set_style_bg_color(tile, lv_color_hex(0xEEE4DA), LV_PART_MAIN);
            lv_obj_set_style_radius(tile, 5, LV_PART_MAIN);
            lv_obj_set_style_border_width(tile, 0, LV_PART_MAIN);
            lv_obj_set_style_pad_all(tile, 0, LV_PART_MAIN);
            lv_obj_set_style_transform_pivot_x(tile, TILE_SIZE / 2, LV_PART_MAIN);
            lv_obj_set_style_transform_pivot_y(tile, TILE_SIZE / 2, LV_PART_MAIN);
            tile_objs[r][c] = tile;

            /* number label */
            lv_obj_t *label = lv_label_create(tile);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
            lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            lv_obj_center(label);
            tile_labels[r][c] = label;

            /* start hidden (empty cells) */
            lv_obj_add_flag(tile, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

/* ---- overlay widget ---- */

static void create_overlay(lv_obj_t *screen)
{
    overlay = lv_obj_create(screen);
    lv_obj_set_size(overlay, 480, 480);
    lv_obj_set_pos(overlay, 0, 0);
    lv_obj_set_style_bg_color(overlay, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(overlay, 0, LV_PART_MAIN);
    lv_obj_add_flag(overlay, LV_OBJ_FLAG_CLICKABLE); /* block input to grid */
    lv_obj_add_flag(overlay, LV_OBJ_FLAG_HIDDEN);

    overlay_label = lv_label_create(overlay);
    lv_obj_set_style_text_font(overlay_label, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_obj_center(overlay_label);
}

/* ---- tile color lookup ---- */

typedef struct {
    uint32_t hex_bg;
    uint32_t hex_fg;
} tile_color_t;

static tile_color_t get_tile_colors(uint16_t value)
{
    switch (value) {
    case 0:     return (tile_color_t){0xCDC1B4, 0x000000};
    case 2:     return (tile_color_t){0xEEE4DA, 0x776E65};
    case 4:     return (tile_color_t){0xEDE0C8, 0x776E65};
    case 8:     return (tile_color_t){0xF2B179, 0xFFFFFF};
    case 16:    return (tile_color_t){0xF59563, 0xFFFFFF};
    case 32:    return (tile_color_t){0xF67C5F, 0xFFFFFF};
    case 64:    return (tile_color_t){0xF65E3B, 0xFFFFFF};
    case 128:   return (tile_color_t){0xEDCF72, 0xFFFFFF};
    case 256:   return (tile_color_t){0xEDCC61, 0xFFFFFF};
    case 512:   return (tile_color_t){0xEDC850, 0xFFFFFF};
    case 1024:  return (tile_color_t){0xEDC53F, 0xFFFFFF};
    case 2048:  return (tile_color_t){0xEDC22E, 0xFFFFFF};
    default:    return (tile_color_t){0x3C3A32, 0xFFFFFF}; /* 4096+ super tile */
    }
}

/* ---- sync tiles to engine ---- */

static void sync_tiles_to_engine(void)
{
    char buf[8];

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            lv_obj_t *tile = tile_objs[r][c];
            lv_obj_t *label = tile_labels[r][c];
            uint16_t val = game.cells[r][c];

            if (val == 0) {
                lv_obj_add_flag(tile, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_remove_flag(tile, LV_OBJ_FLAG_HIDDEN);

                tile_color_t tc = get_tile_colors(val);
                lv_obj_set_style_bg_color(tile, lv_color_hex(tc.hex_bg), LV_PART_MAIN);
                lv_obj_set_style_text_color(label, lv_color_hex(tc.hex_fg), LV_PART_MAIN);

                snprintf(buf, sizeof(buf), "%u", val);
                lv_label_set_text(label, buf);

                /* reset tile position to grid anchor */
                int x = GRID_X + c * (TILE_SIZE + TILE_GAP);
                int y = GRID_Y + r * (TILE_SIZE + TILE_GAP);
                lv_obj_set_pos(tile, x, y);

                /* reset transform scale */
                lv_obj_set_style_transform_scale_x(tile, LV_SCALE_NONE, LV_PART_MAIN);
                lv_obj_set_style_transform_scale_y(tile, LV_SCALE_NONE, LV_PART_MAIN);
            }
        }
    }

    /* update score labels */
    char score_buf[16];
    snprintf(score_buf, sizeof(score_buf), "%" PRIu32, game.score);
    lv_label_set_text(score_val_label, score_buf);

    snprintf(score_buf, sizeof(score_buf), "%" PRIu32, game.best_score);
    lv_label_set_text(best_val_label, score_buf);
}

/* ---- animation callbacks ---- */

static void anim_pos_x_cb(void *var, int32_t v)
{
    lv_obj_set_x((lv_obj_t *)var, v);
}

static void anim_pos_y_cb(void *var, int32_t v)
{
    lv_obj_set_y((lv_obj_t *)var, v);
}

static void anim_scale_x_cb(void *var, int32_t v)
{
    lv_obj_set_style_transform_scale_x((lv_obj_t *)var, v, LV_PART_MAIN);
}

static void anim_scale_y_cb(void *var, int32_t v)
{
    lv_obj_set_style_transform_scale_y((lv_obj_t *)var, v, LV_PART_MAIN);
}

static void anim_opa_cb(void *var, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)var, (lv_opa_t)v, LV_PART_MAIN);
}

/* ---- animate_move ---- */

static void animate_move(int r, int c, int from_x, int from_y)
{
    lv_obj_t *tile = tile_objs[r][c];
    int to_x = GRID_X + c * (TILE_SIZE + TILE_GAP);
    int to_y = GRID_Y + r * (TILE_SIZE + TILE_GAP);

    /* cancel any existing anim on this tile */
    lv_anim_delete(tile, anim_pos_x_cb);
    lv_anim_delete(tile, anim_pos_y_cb);

    /* set to start position */
    lv_obj_set_pos(tile, from_x, from_y);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, tile);
    lv_anim_set_duration(&a, 120);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);

    /* animate x */
    lv_anim_set_exec_cb(&a, anim_pos_x_cb);
    lv_anim_set_values(&a, from_x, to_x);
    lv_anim_start(&a);

    /* animate y */
    lv_anim_set_exec_cb(&a, anim_pos_y_cb);
    lv_anim_set_values(&a, from_y, to_y);
    lv_anim_start(&a);
}

/* ---- animate_merge ---- */

static void animate_merge(int r, int c)
{
    lv_obj_t *tile = tile_objs[r][c];

    lv_anim_delete(tile, anim_scale_x_cb);
    lv_anim_delete(tile, anim_scale_y_cb);

    /* start from normal scale */
    lv_obj_set_style_transform_scale_x(tile, LV_SCALE_NONE, LV_PART_MAIN);
    lv_obj_set_style_transform_scale_y(tile, LV_SCALE_NONE, LV_PART_MAIN);

    /* pop: 1.0 -> 1.18 -> 1.0 */
    int32_t peak = (int32_t)(LV_SCALE_NONE * 1.18f);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, tile);
    lv_anim_set_duration(&a, 100);
    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);

    lv_anim_set_exec_cb(&a, anim_scale_x_cb);
    lv_anim_set_values(&a, LV_SCALE_NONE, peak);
    lv_anim_start(&a);

    /* chain back to normal */
    lv_anim_set_values(&a, peak, LV_SCALE_NONE);
    lv_anim_set_duration(&a, 80);
    lv_anim_start(&a);

    /* same for y */
    lv_anim_set_exec_cb(&a, anim_scale_y_cb);
    lv_anim_set_values(&a, LV_SCALE_NONE, peak);
    lv_anim_set_duration(&a, 100);
    lv_anim_start(&a);

    lv_anim_set_values(&a, peak, LV_SCALE_NONE);
    lv_anim_set_duration(&a, 80);
    lv_anim_start(&a);
}

/* ---- animate_spawn ---- */

static void animate_spawn(int r, int c)
{
    lv_obj_t *tile = tile_objs[r][c];

    lv_anim_delete(tile, anim_scale_x_cb);
    lv_anim_delete(tile, anim_scale_y_cb);

    /* start invisible (scale 0) */
    lv_obj_set_style_transform_scale_x(tile, 0, LV_PART_MAIN);
    lv_obj_set_style_transform_scale_y(tile, 0, LV_PART_MAIN);
    lv_obj_remove_flag(tile, LV_OBJ_FLAG_HIDDEN);

    /* grow to normal */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, tile);
    lv_anim_set_duration(&a, 120);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);

    lv_anim_set_exec_cb(&a, anim_scale_x_cb);
    lv_anim_set_values(&a, 0, LV_SCALE_NONE);
    lv_anim_start(&a);

    lv_anim_set_exec_cb(&a, anim_scale_y_cb);
    lv_anim_set_values(&a, 0, LV_SCALE_NONE);
    lv_anim_start(&a);
}

/* ---- do_move: animation orchestration ---- */

static void do_move(game_dir_t dir)
{
    if (game.state == STATE_LOST) return;

    /* save old board for animation tracking */
    uint16_t old_cells[4][4];
    memcpy(old_cells, game.cells, sizeof(old_cells));

    /* execute engine move */
    game_2048_move(&game, dir);

    if (!game.moved) return;  /* nothing changed */

    /* update score labels immediately */
    char buf[16];
    snprintf(buf, sizeof(buf), "%" PRIu32, game.score);
    lv_label_set_text(score_val_label, buf);
    snprintf(buf, sizeof(buf), "%" PRIu32, game.best_score);
    lv_label_set_text(best_val_label, buf);

    /* for each cell, update content and decide animation */
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            lv_obj_t *tile = tile_objs[r][c];
            lv_obj_t *label = tile_labels[r][c];
            uint16_t val = game.cells[r][c];

            if (val == 0) {
                lv_obj_add_flag(tile, LV_OBJ_FLAG_HIDDEN);
                continue;
            }

            /* update label and colors */
            tile_color_t tc = get_tile_colors(val);
            lv_obj_set_style_bg_color(tile, lv_color_hex(tc.hex_bg), LV_PART_MAIN);
            lv_obj_set_style_text_color(label, lv_color_hex(tc.hex_fg), LV_PART_MAIN);
            lv_label_set_text_fmt(label, "%u", val);

            /* determine animation type */
            if (game.merged[r][c]) {
                /* merged result - do pop animation at destination */
                animate_merge(r, c);
            } else if (old_cells[r][c] == 0) {
                /* new tile spawned here */
                animate_spawn(r, c);
            } else if (old_cells[r][c] == val && !game.merged[r][c]) {
                /* same value at same position - tile did not move, reset to grid position */
                int grid_x = GRID_X + c * (TILE_SIZE + TILE_GAP);
                int grid_y = GRID_Y + r * (TILE_SIZE + TILE_GAP);
                lv_obj_set_pos(tile, grid_x, grid_y);
                lv_obj_remove_flag(tile, LV_OBJ_FLAG_HIDDEN);
            } else {
                /* tile slid into this position from elsewhere - find source */
                int found_r = r, found_c = c;
                bool found = false;
                for (int sr = 0; sr < 4 && !found; sr++) {
                    for (int sc = 0; sc < 4 && !found; sc++) {
                        if (old_cells[sr][sc] == val && !(sr == r && sc == c)) {
                            found_r = sr;
                            found_c = sc;
                            found = true;
                        }
                    }
                }
                lv_obj_remove_flag(tile, LV_OBJ_FLAG_HIDDEN);
                int from_x = GRID_X + found_c * (TILE_SIZE + TILE_GAP);
                int from_y = GRID_Y + found_r * (TILE_SIZE + TILE_GAP);
                animate_move(r, c, from_x, from_y);
            }
        }
    }

    /* handle state overlay */
    if (game.state == STATE_WON) {
        show_overlay("YOU WIN!", lv_color_hex(0xF9F6F2));
    } else if (game.state == STATE_LOST) {
        show_overlay("GAME OVER", lv_color_hex(0xF9F6F2));
    }
}

/* ---- input handling ---- */

static void input_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        switch (key) {
        case LV_KEY_UP:    do_move(DIR_UP);    break;
        case LV_KEY_DOWN:  do_move(DIR_DOWN);  break;
        case LV_KEY_LEFT:  do_move(DIR_LEFT);  break;
        case LV_KEY_RIGHT: do_move(DIR_RIGHT); break;
        default: break;
        }
    } else if (code == LV_EVENT_GESTURE) {
        lv_indev_t *indev = lv_event_get_indev(e);
        if (indev == NULL) return;
        lv_dir_t dir = lv_indev_get_gesture_dir(indev);
        switch (dir) {
        case LV_DIR_LEFT:  do_move(DIR_LEFT);  break;
        case LV_DIR_RIGHT: do_move(DIR_RIGHT); break;
        case LV_DIR_TOP:   do_move(DIR_UP);    break;
        case LV_DIR_BOTTOM:do_move(DIR_DOWN);  break;
        default: break;
        }
    }
}

/* ---- overlay show/hide ---- */

static void show_overlay(const char *msg, lv_color_t color)
{
    lv_label_set_text(overlay_label, msg);
    lv_obj_set_style_text_color(overlay_label, color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_50, LV_PART_MAIN);
    lv_obj_remove_flag(overlay, LV_OBJ_FLAG_HIDDEN);

    /* animate fade in */
    lv_obj_set_style_opa(overlay, LV_OPA_0, LV_PART_MAIN);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, overlay);
    lv_anim_set_duration(&a, 200);
    lv_anim_set_exec_cb(&a, anim_opa_cb);
    lv_anim_set_values(&a, LV_OPA_0, LV_OPA_50);
    lv_anim_start(&a);
}

static void hide_overlay(void)
{
    lv_obj_add_flag(overlay, LV_OBJ_FLAG_HIDDEN);
}

/* ---- reset button handler ---- */

static void reset_btn_cb(lv_event_t *e)
{
    (void)e;
    game_2048_reset(&game);
    hide_overlay();
    sync_tiles_to_engine();
}
