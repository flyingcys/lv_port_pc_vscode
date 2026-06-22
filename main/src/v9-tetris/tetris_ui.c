/**
 * @file tetris_ui.c
 * @brief Tetris UI layer implementation - LVGL v9 interface
 */

/*********************
 *      INCLUDES
 *********************/
#include "tetris_ui.h"
#include <stdio.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/
#define BOARD_CANVAS_WIDTH  (BOARD_WIDTH * CELL_SIZE)
#define BOARD_CANVAS_HEIGHT (BOARD_HEIGHT * CELL_SIZE)
#define NEXT_CANVAS_SIZE    (TETROMINO_SIZE * CELL_SIZE)

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void draw_game_board(tetris_ui_t *ui);
static void draw_next_piece(tetris_ui_t *ui);
static void draw_cell(tetris_ui_t *ui, lv_draw_buf_t *draw_buf, int x, int y, uint8_t type);
static void start_btn_event_cb(lv_event_t *e);
static void pause_btn_event_cb(lv_event_t *e);
static void reset_btn_event_cb(lv_event_t *e);
static void game_over_msgbox_event_cb(lv_event_t *e);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void tetris_ui_init(tetris_ui_t *ui,
                    tetris_game_t *game,
                    lv_obj_t *parent,
                    int32_t screen_w,
                    int32_t screen_h)
{
    if(parent == NULL) {
        parent = lv_screen_active();
    }

    ui->game = game;
    ui->block_style = TETRIS_BLOCK_STYLE_SOLID;
    
    /* Create main container */
    ui->main_container = lv_obj_create(parent);
    lv_obj_set_size(ui->main_container, screen_w, screen_h);
    lv_obj_set_pos(ui->main_container, 0, 0);
    lv_obj_set_flex_flow(ui->main_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui->main_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(ui->main_container, 10, 0);
    lv_obj_set_style_pad_gap(ui->main_container, 15, 0);
    lv_obj_set_style_bg_color(ui->main_container, lv_color_hex(0x1a1a1a), 0);
    lv_obj_clear_flag(ui->main_container, LV_OBJ_FLAG_SCROLLABLE);
    
    /* Create game board canvas */
    ui->game_canvas = lv_canvas_create(ui->main_container);
    ui->game_draw_buf = lv_draw_buf_create(BOARD_CANVAS_WIDTH, BOARD_CANVAS_HEIGHT, LV_COLOR_FORMAT_RGB565, 0);
    lv_canvas_set_draw_buf(ui->game_canvas, ui->game_draw_buf);
    lv_obj_set_style_border_width(ui->game_canvas, 2, 0);
    lv_obj_set_style_border_color(ui->game_canvas, lv_color_hex(0x666666), 0);
    lv_canvas_fill_bg(ui->game_canvas, lv_color_hex(0x000000), LV_OPA_COVER);
    
    /* Create side panel */
    lv_obj_t *side_panel = lv_obj_create(ui->main_container);
    lv_obj_set_size(side_panel, 150, BOARD_CANVAS_HEIGHT);
    lv_obj_set_flex_flow(side_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(side_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(side_panel, 10, 0);
    lv_obj_set_style_pad_gap(side_panel, 10, 0);
    lv_obj_set_style_bg_color(side_panel, lv_color_hex(0x2a2a2a), 0);
    
    /* Score display */
    lv_obj_t *score_title = lv_label_create(side_panel);
    lv_label_set_text(score_title, "SCORE");
    lv_obj_set_style_text_color(score_title, lv_color_hex(0xffffff), 0);
    
    ui->score_label = lv_label_create(side_panel);
    lv_label_set_text(ui->score_label, "0");
    lv_obj_set_style_text_font(ui->score_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(ui->score_label, lv_color_hex(0x00ff00), 0);
    
    /* Lines display */
    lv_obj_t *lines_title = lv_label_create(side_panel);
    lv_label_set_text(lines_title, "LINES");
    lv_obj_set_style_text_color(lines_title, lv_color_hex(0xffffff), 0);
    
    ui->lines_label = lv_label_create(side_panel);
    lv_label_set_text(ui->lines_label, "0");
    lv_obj_set_style_text_font(ui->lines_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(ui->lines_label, lv_color_hex(0xffff00), 0);
    
    /* Level display */
    lv_obj_t *level_title = lv_label_create(side_panel);
    lv_label_set_text(level_title, "LEVEL");
    lv_obj_set_style_text_color(level_title, lv_color_hex(0xffffff), 0);
    
    ui->level_label = lv_label_create(side_panel);
    lv_label_set_text(ui->level_label, "1");
    lv_obj_set_style_text_font(ui->level_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(ui->level_label, lv_color_hex(0xff00ff), 0);
    
    /* Next piece preview */
    lv_obj_t *next_title = lv_label_create(side_panel);
    lv_label_set_text(next_title, "NEXT");
    lv_obj_set_style_text_color(next_title, lv_color_hex(0xffffff), 0);
    
    ui->next_canvas = lv_canvas_create(side_panel);
    ui->next_draw_buf = lv_draw_buf_create(NEXT_CANVAS_SIZE, NEXT_CANVAS_SIZE, LV_COLOR_FORMAT_RGB565, 0);
    lv_canvas_set_draw_buf(ui->next_canvas, ui->next_draw_buf);
    lv_obj_set_style_border_width(ui->next_canvas, 2, 0);
    lv_obj_set_style_border_color(ui->next_canvas, lv_color_hex(0x666666), 0);
    lv_canvas_fill_bg(ui->next_canvas, lv_color_hex(0x000000), LV_OPA_COVER);
    
    /* Control buttons */
    ui->start_btn = lv_button_create(side_panel);
    lv_obj_set_width(ui->start_btn, LV_PCT(100));
    lv_obj_t *start_label = lv_label_create(ui->start_btn);
    lv_label_set_text(start_label, "START");
    lv_obj_center(start_label);
    lv_obj_set_style_bg_color(ui->start_btn, lv_color_hex(0x00aa00), 0);
    lv_obj_add_event_cb(ui->start_btn, start_btn_event_cb, LV_EVENT_CLICKED, ui);
    
    ui->pause_btn = lv_button_create(side_panel);
    lv_obj_set_width(ui->pause_btn, LV_PCT(100));
    lv_obj_t *pause_label = lv_label_create(ui->pause_btn);
    lv_label_set_text(pause_label, "PAUSE");
    lv_obj_center(pause_label);
    lv_obj_set_style_bg_color(ui->pause_btn, lv_color_hex(0xaaaa00), 0);
    lv_obj_add_event_cb(ui->pause_btn, pause_btn_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_flag(ui->pause_btn, LV_OBJ_FLAG_HIDDEN); /* Hidden initially */
    
    ui->reset_btn = lv_button_create(side_panel);
    lv_obj_set_width(ui->reset_btn, LV_PCT(100));
    lv_obj_t *reset_label = lv_label_create(ui->reset_btn);
    lv_label_set_text(reset_label, "RESET");
    lv_obj_center(reset_label);
    lv_obj_set_style_bg_color(ui->reset_btn, lv_color_hex(0xaa0000), 0);
    lv_obj_add_event_cb(ui->reset_btn, reset_btn_event_cb, LV_EVENT_CLICKED, ui);
    
    ui->game_over_msgbox = NULL;
}

void tetris_ui_update(tetris_ui_t *ui)
{
    /* Update score, lines, and level */
    char buf[32];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)ui->game->stats.score);
    lv_label_set_text(ui->score_label, buf);
    
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)ui->game->stats.lines);
    lv_label_set_text(ui->lines_label, buf);
    
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)ui->game->stats.level);
    lv_label_set_text(ui->level_label, buf);
    
    /* Update button visibility based on game state */
    if (ui->game->state == GAME_STATE_IDLE) {
        lv_obj_clear_flag(ui->start_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->pause_btn, LV_OBJ_FLAG_HIDDEN);
    } else if (ui->game->state == GAME_STATE_PLAYING) {
        lv_obj_add_flag(ui->start_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui->pause_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_t *pause_label = lv_obj_get_child(ui->pause_btn, 0);
        lv_label_set_text(pause_label, "PAUSE");
    } else if (ui->game->state == GAME_STATE_PAUSED) {
        lv_obj_t *pause_label = lv_obj_get_child(ui->pause_btn, 0);
        lv_label_set_text(pause_label, "RESUME");
    }
    
    /* Draw game board and next piece */
    draw_game_board(ui);
    draw_next_piece(ui);
    
    /* Show game over dialog if needed */
    if (ui->game->state == GAME_STATE_GAME_OVER && ui->game_over_msgbox == NULL) {
        lv_obj_t *msg_parent = ui->main_container ? ui->main_container : lv_screen_active();
        ui->game_over_msgbox = lv_msgbox_create(msg_parent);
        lv_msgbox_add_title(ui->game_over_msgbox, "GAME OVER");
        
        char msg[64];
        snprintf(msg, sizeof(msg), "Score: %lu\nLines: %lu\nLevel: %lu",
                 (unsigned long)ui->game->stats.score,
                 (unsigned long)ui->game->stats.lines,
                 (unsigned long)ui->game->stats.level);
        lv_msgbox_add_text(ui->game_over_msgbox, msg);
        
        lv_obj_t *btn = lv_msgbox_add_footer_button(ui->game_over_msgbox, "Play Again");
        lv_obj_add_event_cb(btn, game_over_msgbox_event_cb, LV_EVENT_CLICKED, ui);
        
        lv_obj_center(ui->game_over_msgbox);
    }
}

void tetris_ui_cleanup(tetris_ui_t *ui)
{
    if(ui == NULL) return;

    if (ui->game_draw_buf) {
        lv_draw_buf_destroy(ui->game_draw_buf);
        ui->game_draw_buf = NULL;
    }
    if (ui->next_draw_buf) {
        lv_draw_buf_destroy(ui->next_draw_buf);
        ui->next_draw_buf = NULL;
    }
    ui->main_container = NULL;
    ui->game_canvas = NULL;
    ui->next_canvas = NULL;
    ui->score_label = NULL;
    ui->lines_label = NULL;
    ui->level_label = NULL;
    ui->start_btn = NULL;
    ui->pause_btn = NULL;
    ui->reset_btn = NULL;
    ui->game_over_msgbox = NULL;
    ui->game = NULL;
}

void tetris_ui_next_block_style(tetris_ui_t *ui)
{
    if(ui == NULL) return;

    ui->block_style = (tetris_block_style_t)((ui->block_style + 1) % TETRIS_BLOCK_STYLE_COUNT);
    tetris_ui_update(ui);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void draw_game_board(tetris_ui_t *ui)
{
    /* Clear canvas */
    lv_canvas_fill_bg(ui->game_canvas, lv_color_hex(0x000000), LV_OPA_COVER);
    
    lv_draw_buf_t *draw_buf = ui->game_draw_buf;
    
    /* Draw placed pieces */
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            uint8_t cell = ui->game->board[y][x];
            if (cell != 0) {
                draw_cell(ui, draw_buf, x, y, cell);
            }
        }
    }
    
    /* Draw current piece */
    if (ui->game->state == GAME_STATE_PLAYING || ui->game->state == GAME_STATE_PAUSED) {
        const uint8_t (*shape)[TETROMINO_SIZE] = tetris_get_shape(
            ui->game->current_piece.type,
            ui->game->current_piece.rotation
        );
        
        if (shape) {
            for (int y = 0; y < TETROMINO_SIZE; y++) {
                for (int x = 0; x < TETROMINO_SIZE; x++) {
                    if (shape[y][x]) {
                        int board_x = ui->game->current_piece.x + x;
                        int board_y = ui->game->current_piece.y + y;
                        
                        if (board_x >= 0 && board_x < BOARD_WIDTH &&
                            board_y >= 0 && board_y < BOARD_HEIGHT) {
                            draw_cell(ui, draw_buf, board_x, board_y, ui->game->current_piece.type + 1);
                        }
                    }
                }
            }
        }
    }
    
    /* Draw grid lines using rectangles */
    lv_layer_t layer;
    lv_canvas_init_layer(ui->game_canvas, &layer);
    
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_color = lv_color_hex(0x333333);
    rect_dsc.bg_opa = LV_OPA_COVER;
    
    /* Vertical lines */
    for (int x = 0; x <= BOARD_WIDTH; x++) {
        lv_area_t area = {
            .x1 = x * CELL_SIZE,
            .y1 = 0,
            .x2 = x * CELL_SIZE,
            .y2 = BOARD_CANVAS_HEIGHT - 1
        };
        lv_draw_rect(&layer, &rect_dsc, &area);
    }
    
    /* Horizontal lines */
    for (int y = 0; y <= BOARD_HEIGHT; y++) {
        lv_area_t area = {
            .x1 = 0,
            .y1 = y * CELL_SIZE,
            .x2 = BOARD_CANVAS_WIDTH - 1,
            .y2 = y * CELL_SIZE
        };
        lv_draw_rect(&layer, &rect_dsc, &area);
    }
    
    lv_canvas_finish_layer(ui->game_canvas, &layer);
}

static void draw_next_piece(tetris_ui_t *ui)
{
    /* Clear canvas */
    lv_canvas_fill_bg(ui->next_canvas, lv_color_hex(0x000000), LV_OPA_COVER);
    
    lv_draw_buf_t *draw_buf = ui->next_draw_buf;
    
    /* Draw next piece centered */
    const uint8_t (*shape)[TETROMINO_SIZE] = tetris_get_shape(
        ui->game->next_piece.type,
        ROTATION_0
    );
    
    if (shape) {
        for (int y = 0; y < TETROMINO_SIZE; y++) {
            for (int x = 0; x < TETROMINO_SIZE; x++) {
                if (shape[y][x]) {
                    draw_cell(ui, draw_buf, x, y, ui->game->next_piece.type + 1);
                }
            }
        }
    }
}

static void draw_cell(tetris_ui_t *ui, lv_draw_buf_t *draw_buf, int x, int y, uint8_t type)
{
    (void)draw_buf; /* Not used with canvas API */
    
    uint8_t r, g, b;
    tetris_get_color(type, &r, &g, &b);
    
    lv_color_t color = lv_color_make(r, g, b);
    
    /* Draw filled rectangle for the cell using canvas API */
    /* Note: This function is called during canvas layer drawing,
     * so we need to use a different approach - just fill the buffer directly
     * but in a safer way */
    
    /* Calculate buffer dimensions */
    uint32_t buf_w_stride = lv_draw_buf_width_to_stride(
        draw_buf->header.w, 
        LV_COLOR_FORMAT_RGB565
    );
    uint16_t *buf_data = (uint16_t *)draw_buf->data;
    uint16_t color_u16 = lv_color_to_u16(color);
    
    /* Draw cell with 1px border (leave edges empty for grid lines) */
    int inset = (ui->block_style == TETRIS_BLOCK_STYLE_INSET) ? 4 : 1;
    int start_x = x * CELL_SIZE + inset;
    int start_y = y * CELL_SIZE + inset;
    int end_x = (x + 1) * CELL_SIZE - inset;
    int end_y = (y + 1) * CELL_SIZE - inset;
    
    /* Bounds checking */
    if (start_x < 0 || start_y < 0) return;
    if (end_x > (int)draw_buf->header.w || end_y > (int)draw_buf->header.h) return;
    
    /* Fill the cell area */
    for (int py = start_y; py < end_y; py++) {
        for (int px = start_x; px < end_x; px++) {
            if(ui->block_style == TETRIS_BLOCK_STYLE_OUTLINE) {
                bool edge = (px - start_x < 3) ||
                            (end_x - px <= 3) ||
                            (py - start_y < 3) ||
                            (end_y - py <= 3);
                if(!edge) continue;
            }

            uint32_t idx = py * (buf_w_stride / 2) + px;
            if (idx < (draw_buf->data_size / 2)) {
                buf_data[idx] = color_u16;
            }
        }
    }
}

static void start_btn_event_cb(lv_event_t *e)
{
    tetris_ui_t *ui = (tetris_ui_t *)lv_event_get_user_data(e);
    tetris_game_start(ui->game);
    tetris_ui_update(ui);
}

static void pause_btn_event_cb(lv_event_t *e)
{
    tetris_ui_t *ui = (tetris_ui_t *)lv_event_get_user_data(e);
    tetris_game_toggle_pause(ui->game);
    tetris_ui_update(ui);
}

static void reset_btn_event_cb(lv_event_t *e)
{
    tetris_ui_t *ui = (tetris_ui_t *)lv_event_get_user_data(e);
    
    /* Close game over dialog if open */
    if (ui->game_over_msgbox) {
        lv_msgbox_close(ui->game_over_msgbox);
        ui->game_over_msgbox = NULL;
    }
    
    tetris_game_init(ui->game);
    tetris_ui_update(ui);
}

static void game_over_msgbox_event_cb(lv_event_t *e)
{
    tetris_ui_t *ui = (tetris_ui_t *)lv_event_get_user_data(e);
    
    /* Close dialog */
    lv_msgbox_close(ui->game_over_msgbox);
    ui->game_over_msgbox = NULL;
    
    /* Start new game */
    tetris_game_start(ui->game);
    tetris_ui_update(ui);
}
