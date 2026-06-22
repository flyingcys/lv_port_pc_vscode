#include "design_fruit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "design_fruit_assets.h"
#include "design_fruit_model.h"

#define APP_W 680
#define APP_H 480
#define SIDE_W 200
#define BOARD_W 480
#define CELL_SIZE 60
#define SWAP_ANIM_MS 140
#define BLAST_ANIM_MS 300
#define DROP_MS_PER_CELL 60

LV_FONT_DECLARE(design_fruit_font_12);
LV_FONT_DECLARE(design_fruit_font_16);

typedef struct {
    design_fruit_model_t model;
    lv_obj_t *root;
    lv_obj_t *app;
    lv_obj_t *score_label;
    lv_obj_t *status_label;
    lv_obj_t *cells[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];
    lv_obj_t *cell_images[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];
    uint8_t selected_row;
    uint8_t selected_col;
    bool has_selection;
    lv_point_t press_point;
    bool pressing;
    bool animating;
    int32_t screen_w;
    int32_t screen_h;
} design_fruit_ui_t;

static design_fruit_ui_t s_ui;

static const char *const k_fruit_paths[DESIGN_FRUIT_TYPES + 1] = {
    "",
    "png/fruit/grape.png",
    "png/fruit/apple.png",
    "png/fruit/banana.png",
    "png/fruit/orange.png",
    "png/fruit/pear.png",
    "png/fruit/watermelon.png",
    "png/fruit/coconut.png",
};

static void create_layout(design_fruit_ui_t *ui, lv_obj_t *parent);
static void create_left_panel(design_fruit_ui_t *ui);
static void create_board(design_fruit_ui_t *ui);
static void refresh_board(design_fruit_ui_t *ui);
static void refresh_board_to_matrix(design_fruit_ui_t *ui,
                                    const uint8_t board[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS]);
static void refresh_score(design_fruit_ui_t *ui);
static void reset_game(design_fruit_ui_t *ui);
static void try_swap(design_fruit_ui_t *ui, uint8_t row_a, uint8_t col_a, uint8_t row_b, uint8_t col_b);
static void play_swap_plan(design_fruit_ui_t *ui, const design_fruit_swap_plan_t *plan);
static void set_selected(design_fruit_ui_t *ui, uint8_t row, uint8_t col);
static void clear_selected(design_fruit_ui_t *ui);
static uint32_t seed_from_tick(void);
static void cell_event_cb(lv_event_t *e);
static void reset_event_cb(lv_event_t *e);
static void placeholder_event_cb(lv_event_t *e);
static void anim_set_x(void *obj, int32_t value);
static void anim_set_y(void *obj, int32_t value);
static void anim_set_scale(void *obj, int32_t value);
static void anim_set_blast_scale(void *obj, int32_t value);
static void start_obj_anim(lv_obj_t *obj,
                           lv_anim_exec_xcb_t exec_cb,
                           int32_t from,
                           int32_t to,
                           uint32_t duration,
                           uint32_t delay);
static void pump_lvgl_for(uint32_t duration_ms);
static uint32_t round_drop_duration(const design_fruit_round_plan_t *round);
static void reset_image_visual(lv_obj_t *img);

lv_obj_t *design_fruit_create(lv_obj_t *parent, int32_t screen_w, int32_t screen_h)
{
    if(s_ui.root != NULL) return s_ui.root;
    if(parent == NULL) parent = lv_screen_active();

    memset(&s_ui, 0, sizeof(s_ui));
    s_ui.screen_w = screen_w;
    s_ui.screen_h = screen_h;
    design_fruit_assets_init(NULL);
    create_layout(&s_ui, parent);
    reset_game(&s_ui);
    return s_ui.root;
}

void design_fruit_start(void)
{
    if(s_ui.root != NULL) reset_game(&s_ui);
}

void design_fruit_stop(void)
{
    if(s_ui.root != NULL) lv_obj_delete(s_ui.root);
    memset(&s_ui, 0, sizeof(s_ui));
}

static void create_layout(design_fruit_ui_t *ui, lv_obj_t *parent)
{
    int32_t app_x = (ui->screen_w - APP_W) / 2;
    if(app_x < 0) app_x = 0;

    ui->root = lv_obj_create(parent);
    lv_obj_remove_style_all(ui->root);
    lv_obj_set_size(ui->root, ui->screen_w, ui->screen_h);
    lv_obj_set_pos(ui->root, 0, 0);
    lv_obj_set_style_bg_color(ui->root, lv_color_hex(0x12201b), 0);
    lv_obj_set_style_bg_opa(ui->root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(ui->root, LV_OBJ_FLAG_SCROLLABLE);

    ui->app = lv_obj_create(ui->root);
    lv_obj_remove_style_all(ui->app);
    lv_obj_set_size(ui->app, APP_W, APP_H);
    lv_obj_set_pos(ui->app, app_x, 0);
    lv_obj_clear_flag(ui->app, LV_OBJ_FLAG_SCROLLABLE);

    create_left_panel(ui);
    create_board(ui);
}

static lv_obj_t *create_text_button(lv_obj_t *parent, const char *text, int32_t y, lv_event_cb_t cb, void *user_data)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_size(btn, 150, 34);
    lv_obj_set_pos(btn, 12, y);
    lv_obj_set_style_radius(btn, 4, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x5d9cec), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x242424), 0);
    lv_obj_set_style_shadow_width(btn, 2, 0);
    lv_obj_set_style_shadow_opa(btn, 80, 0);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(label, &design_fruit_font_16, 0);
    lv_obj_center(label);
    return btn;
}

static lv_obj_t *create_small_button(lv_obj_t *parent,
                                     const char *text,
                                     int32_t x,
                                     int32_t y,
                                     lv_event_cb_t cb,
                                     void *user_data)
{
    lv_obj_t *btn = create_text_button(parent, text, y, cb, user_data);
    lv_obj_set_size(btn, 84, 34);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x656d78), 0);
    return btn;
}

static void create_left_panel(design_fruit_ui_t *ui)
{
    char path[512];
    lv_obj_t *panel = lv_obj_create(ui->app);
    lv_obj_remove_style_all(panel);
    lv_obj_set_size(panel, SIDE_W, APP_H);
    lv_obj_set_pos(panel, 0, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xccd1d9), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    if(design_fruit_assets_build_image_path(path, sizeof(path), "png/bg.png")) {
        lv_obj_t *bg = lv_image_create(panel);
        lv_image_set_src(bg, path);
        lv_image_set_inner_align(bg, LV_IMAGE_ALIGN_STRETCH);
        lv_obj_set_size(bg, SIDE_W, APP_H);
        lv_obj_set_pos(bg, 0, 0);
        lv_obj_move_background(bg);
    }

    lv_obj_t *logo = lv_image_create(panel);
    if(design_fruit_assets_build_image_path(path, sizeof(path), "png/logo_200.png")) {
        lv_image_set_src(logo, path);
    }
    lv_obj_set_pos(logo, 0, 16);

    lv_obj_t *score_circle = lv_obj_create(panel);
    lv_obj_set_size(score_circle, 128, 128);
    lv_obj_set_pos(score_circle, 36, 86);
    lv_obj_set_style_radius(score_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(score_circle, lv_color_hex(0x5d9cec), 0);
    lv_obj_set_style_bg_opa(score_circle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(score_circle, 4, 0);
    lv_obj_set_style_border_color(score_circle, lv_color_hex(0xffffff), 0);
    lv_obj_clear_flag(score_circle, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *score_title = lv_label_create(score_circle);
    lv_label_set_text(score_title, "SCORE");
    lv_obj_set_style_text_color(score_title, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(score_title, &lv_font_montserrat_14, 0);
    lv_obj_align(score_title, LV_ALIGN_TOP_MID, 0, 26);

    ui->score_label = lv_label_create(score_circle);
    lv_obj_set_style_text_color(ui->score_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(ui->score_label, &lv_font_montserrat_24, 0);
    lv_obj_align(ui->score_label, LV_ALIGN_CENTER, 0, 18);

    create_text_button(panel, "新游戏", 236, reset_event_cb, ui);
    create_small_button(panel, "正常模式", 12, 286, placeholder_event_cb, ui);
    create_small_button(panel, "计时模式", 104, 286, placeholder_event_cb, ui);
    create_text_button(panel, "提示", 336, placeholder_event_cb, ui);
    create_small_button(panel, "排行榜", 12, 386, placeholder_event_cb, ui);
    create_small_button(panel, "历史数据", 104, 386, placeholder_event_cb, ui);
    create_small_button(panel, "背景音乐", 12, 436, placeholder_event_cb, ui);
    create_small_button(panel, "帮助", 104, 436, placeholder_event_cb, ui);

    ui->status_label = lv_label_create(panel);
    lv_label_set_text(ui->status_label, "");
    lv_obj_set_width(ui->status_label, 170);
    lv_obj_set_pos(ui->status_label, 15, 218);
    lv_obj_set_style_text_align(ui->status_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(ui->status_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(ui->status_label, &design_fruit_font_12, 0);
}

static void create_board(design_fruit_ui_t *ui)
{
    char path[512];
    lv_obj_t *board = lv_obj_create(ui->app);
    lv_obj_remove_style_all(board);
    lv_obj_set_size(board, BOARD_W, BOARD_W);
    lv_obj_set_pos(board, SIDE_W, 0);
    lv_obj_set_style_bg_color(board, lv_color_hex(0x7bd86f), 0);
    lv_obj_set_style_bg_opa(board, LV_OPA_COVER, 0);
    lv_obj_clear_flag(board, LV_OBJ_FLAG_SCROLLABLE);

    if(design_fruit_assets_build_image_path(path, sizeof(path), "png/bg.png")) {
        lv_obj_t *bg = lv_image_create(board);
        lv_image_set_src(bg, path);
        lv_image_set_inner_align(bg, LV_IMAGE_ALIGN_TILE);
        lv_obj_set_size(bg, BOARD_W, BOARD_W);
        lv_obj_set_pos(bg, 0, 0);
        lv_obj_clear_flag(bg, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);
    }

    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
            lv_obj_t *cell = lv_button_create(board);
            lv_obj_set_size(cell, CELL_SIZE, CELL_SIZE);
            lv_obj_set_pos(cell, col * CELL_SIZE, row * CELL_SIZE);
            lv_obj_set_style_radius(cell, 8, 0);
            lv_obj_set_style_bg_opa(cell, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(cell, 1, 0);
            lv_obj_set_style_border_color(cell, lv_color_hex(0x4f9f45), 0);
            lv_obj_set_style_pad_all(cell, 4, 0);
            lv_obj_set_user_data(cell, (void *)(uintptr_t)((row << 4) | col));
            lv_obj_add_event_cb(cell, cell_event_cb, LV_EVENT_PRESSED, ui);
            lv_obj_add_event_cb(cell, cell_event_cb, LV_EVENT_RELEASED, ui);
            lv_obj_add_event_cb(cell, cell_event_cb, LV_EVENT_CLICKED, ui);

            lv_obj_t *img = lv_image_create(cell);
            lv_obj_center(img);
            ui->cells[row][col] = cell;
            ui->cell_images[row][col] = img;
        }
    }
}

static void refresh_board(design_fruit_ui_t *ui)
{
    refresh_board_to_matrix(ui, ui->model.board);
}

static void refresh_board_to_matrix(design_fruit_ui_t *ui, const uint8_t board[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS])
{
    char path[512];

    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
            uint8_t value = board[row][col];
            if(value <= DESIGN_FRUIT_TYPES &&
               design_fruit_assets_build_image_path(path, sizeof(path), k_fruit_paths[value])) {
                lv_image_set_src(ui->cell_images[row][col], path);
            }
            reset_image_visual(ui->cell_images[row][col]);
            lv_obj_set_style_border_width(ui->cells[row][col],
                                          ui->has_selection && ui->selected_row == row && ui->selected_col == col ? 4 : 1,
                                          0);
            lv_obj_set_style_border_color(ui->cells[row][col],
                                          ui->has_selection && ui->selected_row == row && ui->selected_col == col
                                              ? lv_color_hex(0xffd166)
                                              : lv_color_hex(0x4f9f45),
                                          0);
        }
    }
}

static void reset_image_visual(lv_obj_t *img)
{
    lv_anim_delete(img, anim_set_x);
    lv_anim_delete(img, anim_set_y);
    lv_anim_delete(img, anim_set_scale);
    lv_anim_delete(img, anim_set_blast_scale);
    lv_obj_set_pos(img, 0, 0);
    lv_obj_center(img);
    lv_image_set_scale(img, 256);
    lv_obj_clear_flag(img, LV_OBJ_FLAG_HIDDEN);
}

static void refresh_score(design_fruit_ui_t *ui)
{
    char text[32];
    snprintf(text, sizeof(text), "%u", (unsigned)design_fruit_model_score(&ui->model));
    lv_label_set_text(ui->score_label, text);
}

static void reset_game(design_fruit_ui_t *ui)
{
    design_fruit_model_init(&ui->model, seed_from_tick());
    clear_selected(ui);
    lv_label_set_text(ui->status_label, "开始");
    refresh_score(ui);
    refresh_board(ui);
}

static void try_swap(design_fruit_ui_t *ui, uint8_t row_a, uint8_t col_a, uint8_t row_b, uint8_t col_b)
{
    design_fruit_swap_plan_t plan;

    if(ui->animating) return;
    ui->animating = true;
    clear_selected(ui);

    if(design_fruit_model_swap_with_plan(&ui->model, row_a, col_a, row_b, col_b, &plan)) {
        lv_label_set_text(ui->status_label, "消除成功");
        play_swap_plan(ui, &plan);
    } else {
        int32_t dx = ((int32_t)col_b - (int32_t)col_a) * CELL_SIZE;
        int32_t dy = ((int32_t)row_b - (int32_t)row_a) * CELL_SIZE;
        start_obj_anim(ui->cell_images[row_a][col_a], anim_set_x, 0, dx, SWAP_ANIM_MS / 2, 0);
        start_obj_anim(ui->cell_images[row_a][col_a], anim_set_y, 0, dy, SWAP_ANIM_MS / 2, 0);
        start_obj_anim(ui->cell_images[row_b][col_b], anim_set_x, 0, -dx, SWAP_ANIM_MS / 2, 0);
        start_obj_anim(ui->cell_images[row_b][col_b], anim_set_y, 0, -dy, SWAP_ANIM_MS / 2, 0);
        pump_lvgl_for(SWAP_ANIM_MS / 2 + 20);
        start_obj_anim(ui->cell_images[row_a][col_a], anim_set_x, dx, 0, SWAP_ANIM_MS / 2, 0);
        start_obj_anim(ui->cell_images[row_a][col_a], anim_set_y, dy, 0, SWAP_ANIM_MS / 2, 0);
        start_obj_anim(ui->cell_images[row_b][col_b], anim_set_x, -dx, 0, SWAP_ANIM_MS / 2, 0);
        start_obj_anim(ui->cell_images[row_b][col_b], anim_set_y, -dy, 0, SWAP_ANIM_MS / 2, 0);
        pump_lvgl_for(SWAP_ANIM_MS / 2 + 20);
        lv_label_set_text(ui->status_label, "不能交换");
        refresh_board(ui);
    }
    refresh_score(ui);
    ui->animating = false;
}

static void play_swap_plan(design_fruit_ui_t *ui, const design_fruit_swap_plan_t *plan)
{
    int32_t dx = ((int32_t)plan->to_col - (int32_t)plan->from_col) * CELL_SIZE;
    int32_t dy = ((int32_t)plan->to_row - (int32_t)plan->from_row) * CELL_SIZE;

    start_obj_anim(ui->cell_images[plan->from_row][plan->from_col], anim_set_x, 0, dx, SWAP_ANIM_MS, 0);
    start_obj_anim(ui->cell_images[plan->from_row][plan->from_col], anim_set_y, 0, dy, SWAP_ANIM_MS, 0);
    start_obj_anim(ui->cell_images[plan->to_row][plan->to_col], anim_set_x, 0, -dx, SWAP_ANIM_MS, 0);
    start_obj_anim(ui->cell_images[plan->to_row][plan->to_col], anim_set_y, 0, -dy, SWAP_ANIM_MS, 0);
    pump_lvgl_for(SWAP_ANIM_MS + 20);

    for(uint8_t round_i = 0; round_i < plan->round_count; round_i++) {
        const design_fruit_round_plan_t *round = &plan->rounds[round_i];
        uint32_t drop_duration = round_drop_duration(round);

        refresh_board_to_matrix(ui, round->board_before);
        for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
            for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
                if(round->marks[row][col]) {
                    start_obj_anim(ui->cell_images[row][col], anim_set_blast_scale, 0, (int32_t)BLAST_ANIM_MS,
                                   BLAST_ANIM_MS, 0);
                }
            }
        }
        pump_lvgl_for(BLAST_ANIM_MS + 20);

        refresh_board_to_matrix(ui, round->board_after);
        for(uint16_t i = 0; i < round->move_count; i++) {
            const design_fruit_move_t *move = &round->moves[i];
            lv_obj_t *img = ui->cell_images[move->to_row][move->to_col];
            int32_t start_y = ((int32_t)move->from_row - (int32_t)move->to_row) * CELL_SIZE;
            reset_image_visual(img);
            start_obj_anim(img, anim_set_y, start_y, 0, drop_duration, 0);
        }
        for(uint16_t i = 0; i < round->spawn_count; i++) {
            const design_fruit_spawn_t *spawn = &round->spawns[i];
            lv_obj_t *img = ui->cell_images[spawn->row][spawn->col];
            int32_t start_y = -((int32_t)spawn->drop_cells * CELL_SIZE);
            reset_image_visual(img);
            start_obj_anim(img, anim_set_y, start_y, 0, drop_duration, 0);
        }
        pump_lvgl_for(drop_duration + 20);
        refresh_board_to_matrix(ui, round->board_after);
    }
    refresh_board_to_matrix(ui, plan->final_board);
}

static void set_selected(design_fruit_ui_t *ui, uint8_t row, uint8_t col)
{
    ui->selected_row = row;
    ui->selected_col = col;
    ui->has_selection = true;
    refresh_board(ui);
}

static void clear_selected(design_fruit_ui_t *ui)
{
    ui->has_selection = false;
}

static uint32_t seed_from_tick(void)
{
    uint32_t tick = lv_tick_get();
    return tick != 0u ? tick : 1u;
}

static void anim_set_x(void *obj, int32_t value)
{
    lv_obj_set_x((lv_obj_t *)obj, value);
}

static void anim_set_y(void *obj, int32_t value)
{
    lv_obj_set_y((lv_obj_t *)obj, value);
}

static void anim_set_scale(void *obj, int32_t value)
{
    lv_image_set_scale((lv_obj_t *)obj, (uint16_t)value);
}

static void anim_set_blast_scale(void *obj, int32_t value)
{
    int32_t scale;
    int32_t grow_ms = BLAST_ANIM_MS / 3;
    if(value <= grow_ms) {
        scale = 256 + ((294 - 256) * value) / grow_ms;
    } else {
        int32_t shrink_ms = BLAST_ANIM_MS - grow_ms;
        int32_t elapsed = value - grow_ms;
        scale = 294 - (294 * elapsed) / shrink_ms;
    }
    if(scale < 0) scale = 0;
    lv_image_set_scale((lv_obj_t *)obj, (uint16_t)scale);
}

static void start_obj_anim(lv_obj_t *obj,
                           lv_anim_exec_xcb_t exec_cb,
                           int32_t from,
                           int32_t to,
                           uint32_t duration,
                           uint32_t delay)
{
    lv_anim_t anim;
    lv_anim_delete(obj, exec_cb);
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_exec_cb(&anim, exec_cb);
    lv_anim_set_values(&anim, from, to);
    lv_anim_set_duration(&anim, duration);
    lv_anim_set_delay(&anim, delay);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_start(&anim);
}

static void pump_lvgl_for(uint32_t duration_ms)
{
    uint32_t start = lv_tick_get();
    while(lv_tick_elaps(start) < duration_ms) {
        lv_timer_handler();
        lv_tick_inc(5);
        usleep(5 * 1000);
    }
    lv_timer_handler();
}

static uint32_t round_drop_duration(const design_fruit_round_plan_t *round)
{
    uint8_t max_cells = 1;
    for(uint16_t i = 0; i < round->move_count; i++) {
        uint8_t cells = (uint8_t)(round->moves[i].to_row - round->moves[i].from_row);
        if(cells > max_cells) max_cells = cells;
    }
    for(uint16_t i = 0; i < round->spawn_count; i++) {
        if(round->spawns[i].drop_cells > max_cells) max_cells = round->spawns[i].drop_cells;
    }
    return (uint32_t)max_cells * DROP_MS_PER_CELL;
}

static void cell_event_cb(lv_event_t *e)
{
    design_fruit_ui_t *ui = (design_fruit_ui_t *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target_obj(e);
    uint8_t packed = (uint8_t)(uintptr_t)lv_obj_get_user_data(target);
    uint8_t row = packed >> 4;
    uint8_t col = packed & 0x0fu;

    if(ui->animating) return;

    if(code == LV_EVENT_PRESSED) {
        lv_indev_t *indev = lv_event_get_indev(e);
        if(indev != NULL) lv_indev_get_point(indev, &ui->press_point);
        ui->pressing = true;
        return;
    }

    if(code == LV_EVENT_RELEASED && ui->pressing) {
        lv_indev_t *indev = lv_event_get_indev(e);
        lv_point_t point = ui->press_point;
        int32_t dx;
        int32_t dy;

        ui->pressing = false;
        if(indev == NULL) return;
        lv_indev_get_point(indev, &point);
        dx = point.x - ui->press_point.x;
        dy = point.y - ui->press_point.y;
        if(dx * dx + dy * dy < 18 * 18) return;

        if(abs(dx) > abs(dy)) {
            if(dx > 0 && col + 1u < DESIGN_FRUIT_COLS) try_swap(ui, row, col, row, (uint8_t)(col + 1u));
            else if(dx < 0 && col > 0u) try_swap(ui, row, col, row, (uint8_t)(col - 1u));
        } else {
            if(dy > 0 && row + 1u < DESIGN_FRUIT_ROWS) try_swap(ui, row, col, (uint8_t)(row + 1u), col);
            else if(dy < 0 && row > 0u) try_swap(ui, row, col, (uint8_t)(row - 1u), col);
        }
        return;
    }

    if(code == LV_EVENT_CLICKED) {
        if(ui->has_selection) {
            uint8_t prev_row = ui->selected_row;
            uint8_t prev_col = ui->selected_col;
            if(prev_row == row && prev_col == col) {
                clear_selected(ui);
                refresh_board(ui);
            } else {
                try_swap(ui, prev_row, prev_col, row, col);
            }
        } else {
            set_selected(ui, row, col);
        }
    }
}

static void reset_event_cb(lv_event_t *e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        reset_game((design_fruit_ui_t *)lv_event_get_user_data(e));
    }
}

static void placeholder_event_cb(lv_event_t *e)
{
    design_fruit_ui_t *ui = (design_fruit_ui_t *)lv_event_get_user_data(e);
    if(lv_event_get_code(e) == LV_EVENT_CLICKED && ui != NULL) {
        lv_label_set_text(ui->status_label, "占位功能");
    }
}
