#include "game_2048.h"

#include <stdlib.h>
#include <string.h>

#include "game_2048_core.h"

#define GAME_2048_ANIM_MOVE_MS 140U
#define GAME_2048_ANIM_FADE_MS 90U
#define GAME_2048_ANIM_POP_MS 130U

typedef struct {
    uint32_t value;
    uint32_t bg;
    uint32_t fg;
} tile_style_t;

typedef struct {
    game_2048_core_t game;
    lv_obj_t *root;
    lv_obj_t *board;
    lv_obj_t *score_label;
    lv_obj_t *best_label;
    lv_obj_t *status_label;
    lv_obj_t *tiles[GAME_2048_MAX_SIZE][GAME_2048_MAX_SIZE];
    lv_obj_t *size_buttons[3];
    lv_point_t press_point;
    uint8_t grid_size;
    bool pressing;
    bool animating;
    int32_t screen_w;
    int32_t screen_h;
    lv_coord_t board_size;
    lv_coord_t tile_size;
    lv_coord_t gap;
    lv_coord_t radius;
} game_2048_ui_t;

typedef struct {
    uint8_t row;
    uint8_t col;
    lv_obj_t *obj;
    uint32_t value;
    bool merged;
} survivor_info_t;

static const tile_style_t k_tile_styles[] = {
    {2U, 0xEEE4DAU, 0x776E65U},    {4U, 0xEDE0C8U, 0x776E65U},
    {8U, 0xF2B179U, 0xF9F6F2U},    {16U, 0xF59563U, 0xF9F6F2U},
    {32U, 0xF67C5FU, 0xF9F6F2U},   {64U, 0xF65E3BU, 0xF9F6F2U},
    {128U, 0xEDCF72U, 0xF9F6F2U},  {256U, 0xEDCC61U, 0xF9F6F2U},
    {512U, 0xEDC850U, 0xF9F6F2U},  {1024U, 0xEDC53FU, 0xF9F6F2U},
    {2048U, 0xEDC22EU, 0xF9F6F2U}
};

static game_2048_ui_t s_ui;
static uint8_t s_preferred_grid_size = 4U;

static void reset_game(game_2048_ui_t *ui);
static void rebuild_tiles(game_2048_ui_t *ui);
static void create_layout(game_2048_ui_t *ui, lv_obj_t *parent);
static void create_header(game_2048_ui_t *ui);
static void create_controls(game_2048_ui_t *ui);
static void create_board(game_2048_ui_t *ui);
static void create_background_tiles(game_2048_ui_t *ui);
static void update_score(game_2048_ui_t *ui);
static void update_status(game_2048_ui_t *ui);
static void update_size_buttons(game_2048_ui_t *ui);
static void apply_move_steps(game_2048_ui_t *ui, const game_2048_move_result_t *result);
static void move_by_dir(game_2048_ui_t *ui, game_2048_dir_t dir);
static void focus_board(game_2048_ui_t *ui);
static void handle_swipe_point(game_2048_ui_t *ui, const lv_point_t *point);
static lv_obj_t *create_tile(game_2048_ui_t *ui, uint8_t row, uint8_t col, uint32_t value);
static void apply_tile_style(game_2048_ui_t *ui, lv_obj_t *tile, uint32_t value);
static const lv_font_t *tile_font(game_2048_ui_t *ui, uint32_t value);
static lv_coord_t cell_pos(game_2048_ui_t *ui, uint8_t index);
static uint32_t seed_from_tick(void);

static void event_board_cb(lv_event_t *e);
static void event_key_cb(lv_event_t *e);
static void event_reset_cb(lv_event_t *e);
static void event_size_cb(lv_event_t *e);
static void anim_x_cb(void *obj, int32_t value);
static void anim_y_cb(void *obj, int32_t value);
static void anim_opa_cb(void *obj, int32_t value);
static void anim_transform_cb(void *obj, int32_t value);
static void anim_disappear_ready_cb(lv_anim_t *anim);
static void anim_transform_reset_cb(lv_anim_t *anim);
static void animate_tile_move(lv_obj_t *tile, lv_coord_t x_end, lv_coord_t y_end, bool disappear);
static void animate_tile_spawn(lv_obj_t *tile);
static void animate_tile_merge(lv_obj_t *tile);

lv_obj_t * game_2048_create(lv_obj_t *parent, int32_t screen_w, int32_t screen_h)
{
    if(s_ui.root != NULL) return s_ui.root;
    if(parent == NULL) parent = lv_screen_active();

    memset(&s_ui, 0, sizeof(s_ui));
    s_ui.screen_w = screen_w;
    s_ui.screen_h = screen_h;
    s_ui.grid_size = s_preferred_grid_size;
    if(s_ui.grid_size < GAME_2048_MIN_SIZE || s_ui.grid_size > GAME_2048_MAX_SIZE) {
        s_ui.grid_size = 4U;
    }

    create_layout(&s_ui, parent);
    reset_game(&s_ui);
    return s_ui.root;
}

void game_2048_start(void)
{
    if(s_ui.root != NULL) {
        reset_game(&s_ui);
        focus_board(&s_ui);
    }
}

void game_2048_stop(void)
{
    if(s_ui.root != NULL) {
        lv_obj_delete(s_ui.root);
    }
    memset(&s_ui, 0, sizeof(s_ui));
}

void game_2048_set_grid_size(uint8_t size)
{
    if(size < GAME_2048_MIN_SIZE || size > GAME_2048_MAX_SIZE) return;
    s_preferred_grid_size = size;
    if(s_ui.root != NULL && s_ui.grid_size != size) {
        s_ui.grid_size = size;
        reset_game(&s_ui);
    }
}

uint8_t game_2048_get_grid_size(void)
{
    return s_ui.root != NULL ? s_ui.grid_size : s_preferred_grid_size;
}

void game_2048_focus(void)
{
    focus_board(&s_ui);
}

static void create_layout(game_2048_ui_t *ui, lv_obj_t *parent)
{
    ui->root = lv_obj_create(parent);
    lv_obj_remove_style_all(ui->root);
    lv_obj_set_size(ui->root, ui->screen_w, ui->screen_h);
    lv_obj_set_pos(ui->root, 0, 0);
    lv_obj_set_style_bg_color(ui->root, lv_color_hex(0xFAF8EFU), 0);
    lv_obj_set_style_bg_opa(ui->root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(ui->root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(ui->root, LV_OBJ_FLAG_CLICKABLE);

    create_header(ui);
    create_controls(ui);
    create_board(ui);

    focus_board(ui);
}

static void create_header(game_2048_ui_t *ui)
{
    int32_t margin = ui->screen_w < 520 ? 24 : 48;
    lv_obj_t *title = lv_label_create(ui->root);
    lv_label_set_text(title, "2048");
    lv_obj_set_style_text_color(title, lv_color_hex(0x776E65U), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_48, 0);
    lv_obj_set_pos(title, margin, 30);

    lv_obj_t *score_box = lv_obj_create(ui->root);
    lv_obj_set_size(score_box, 88, 58);
    lv_obj_set_pos(score_box, ui->screen_w - margin - 190, 36);
    lv_obj_set_style_bg_color(score_box, lv_color_hex(0xBBADA0U), 0);
    lv_obj_set_style_bg_opa(score_box, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(score_box, 8, 0);
    lv_obj_set_style_border_width(score_box, 0, 0);
    lv_obj_clear_flag(score_box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *score_title = lv_label_create(score_box);
    lv_label_set_text(score_title, "SCORE");
    lv_obj_set_style_text_color(score_title, lv_color_hex(0xEEE4DAU), 0);
    lv_obj_set_style_text_font(score_title, &lv_font_montserrat_12, 0);
    lv_obj_align(score_title, LV_ALIGN_TOP_MID, 0, 5);

    ui->score_label = lv_label_create(score_box);
    lv_obj_set_style_text_color(ui->score_label, lv_color_hex(0xFFFFFFU), 0);
    lv_obj_set_style_text_font(ui->score_label, &lv_font_montserrat_20, 0);
    lv_obj_align(ui->score_label, LV_ALIGN_BOTTOM_MID, 0, -5);

    lv_obj_t *best_box = lv_obj_create(ui->root);
    lv_obj_set_size(best_box, 96, 58);
    lv_obj_set_pos(best_box, ui->screen_w - margin - 96, 36);
    lv_obj_set_style_bg_color(best_box, lv_color_hex(0xBBADA0U), 0);
    lv_obj_set_style_bg_opa(best_box, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(best_box, 8, 0);
    lv_obj_set_style_border_width(best_box, 0, 0);
    lv_obj_clear_flag(best_box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *best_title = lv_label_create(best_box);
    lv_label_set_text(best_title, "BEST");
    lv_obj_set_style_text_color(best_title, lv_color_hex(0xEEE4DAU), 0);
    lv_obj_set_style_text_font(best_title, &lv_font_montserrat_12, 0);
    lv_obj_align(best_title, LV_ALIGN_TOP_MID, 0, 5);

    ui->best_label = lv_label_create(best_box);
    lv_obj_set_style_text_color(ui->best_label, lv_color_hex(0xFFFFFFU), 0);
    lv_obj_set_style_text_font(ui->best_label, &lv_font_montserrat_20, 0);
    lv_obj_align(ui->best_label, LV_ALIGN_BOTTOM_MID, 0, -5);
}

static void create_controls(game_2048_ui_t *ui)
{
    const char *labels[] = {"4x4", "5x5", "6x6"};
    lv_obj_t *row = lv_obj_create(ui->root);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, 250, 44);
    lv_obj_set_pos(row, (ui->screen_w - 250) / 2, 112);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 8, 0);

    for(uint8_t i = 0; i < 3U; i++) {
        lv_obj_t *btn = lv_button_create(row);
        lv_obj_set_size(btn, 72, 38);
        lv_obj_set_style_radius(btn, 8, 0);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_set_user_data(btn, (void *)(uintptr_t)(i + 4U));
        lv_obj_add_event_cb(btn, event_size_cb, LV_EVENT_CLICKED, ui);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, labels[i]);
        lv_obj_center(label);
        ui->size_buttons[i] = btn;
    }

    lv_obj_t *reset_btn = lv_button_create(ui->root);
    lv_obj_set_size(reset_btn, 118, 42);
    lv_obj_set_pos(reset_btn, ui->screen_w - 168, 112);
    lv_obj_set_style_bg_color(reset_btn, lv_color_hex(0x8F7A66U), 0);
    lv_obj_set_style_radius(reset_btn, 8, 0);
    lv_obj_set_style_border_width(reset_btn, 0, 0);
    lv_obj_add_event_cb(reset_btn, event_reset_cb, LV_EVENT_CLICKED, ui);

    lv_obj_t *reset_label = lv_label_create(reset_btn);
    lv_label_set_text(reset_label, "New Game");
    lv_obj_set_style_text_color(reset_label, lv_color_hex(0xF9F6F2U), 0);
    lv_obj_center(reset_label);
}

static void create_board(game_2048_ui_t *ui)
{
    int32_t usable_w = ui->screen_w - 72;
    int32_t usable_h = ui->screen_h - 190;
    int32_t board_size = usable_w < usable_h ? usable_w : usable_h;
    if(board_size > 520) board_size = 520;
    if(board_size < 240) board_size = 240;

    ui->board_size = board_size;
    ui->board = lv_obj_create(ui->root);
    lv_obj_remove_style_all(ui->board);
    lv_obj_set_size(ui->board, ui->board_size, ui->board_size);
    lv_obj_set_pos(ui->board, (ui->screen_w - ui->board_size) / 2, 170);
    lv_obj_set_style_bg_color(ui->board, lv_color_hex(0xBBADA0U), 0);
    lv_obj_set_style_bg_opa(ui->board, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ui->board, 8, 0);
    lv_obj_clear_flag(ui->board, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(ui->board, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(ui->board, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_event_cb(ui->board, event_board_cb, LV_EVENT_PRESSED, ui);
    lv_obj_add_event_cb(ui->board, event_board_cb, LV_EVENT_PRESSING, ui);
    lv_obj_add_event_cb(ui->board, event_board_cb, LV_EVENT_RELEASED, ui);
    lv_obj_add_event_cb(ui->board, event_key_cb, LV_EVENT_KEY, ui);

    ui->status_label = lv_label_create(ui->root);
    lv_obj_set_style_text_color(ui->status_label, lv_color_hex(0x776E65U), 0);
    lv_obj_set_style_text_font(ui->status_label, &lv_font_montserrat_16, 0);
    lv_obj_align_to(ui->status_label, ui->board, LV_ALIGN_OUT_BOTTOM_MID, 0, 18);
}

static void reset_board_children(game_2048_ui_t *ui)
{
    lv_obj_clean(ui->board);
    memset(ui->tiles, 0, sizeof(ui->tiles));

    ui->gap = ui->board_size / (ui->grid_size * 10);
    if(ui->gap < 6) ui->gap = 6;
    if(ui->gap > 14) ui->gap = 14;
    ui->tile_size = (ui->board_size - ui->gap * (ui->grid_size + 1)) / ui->grid_size;
    ui->radius = ui->tile_size / 12;
    if(ui->radius < 4) ui->radius = 4;

    create_background_tiles(ui);
}

static void reset_game(game_2048_ui_t *ui)
{
    game_2048_core_init(&ui->game, ui->grid_size, seed_from_tick());
    reset_board_children(ui);
    rebuild_tiles(ui);
    update_score(ui);
    update_status(ui);
    update_size_buttons(ui);
    ui->animating = false;
    ui->pressing = false;
    focus_board(ui);
}

static void create_background_tiles(game_2048_ui_t *ui)
{
    for(uint8_t row = 0U; row < ui->grid_size; row++) {
        for(uint8_t col = 0U; col < ui->grid_size; col++) {
            lv_obj_t *base = lv_obj_create(ui->board);
            lv_obj_remove_style_all(base);
            lv_obj_set_size(base, ui->tile_size, ui->tile_size);
            lv_obj_set_pos(base, cell_pos(ui, col), cell_pos(ui, row));
            lv_obj_set_style_bg_color(base, lv_color_hex(0xCDC1B4U), 0);
            lv_obj_set_style_bg_opa(base, LV_OPA_COVER, 0);
            lv_obj_set_style_radius(base, ui->radius, 0);
            lv_obj_clear_flag(base, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(base, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(base, LV_OBJ_FLAG_EVENT_BUBBLE);
        }
    }
}

static void rebuild_tiles(game_2048_ui_t *ui)
{
    for(uint8_t row = 0U; row < ui->grid_size; row++) {
        for(uint8_t col = 0U; col < ui->grid_size; col++) {
            uint32_t value = game_2048_core_cell(&ui->game, row, col);
            if(value == 0U) continue;
            lv_obj_t *tile = create_tile(ui, row, col, value);
            ui->tiles[row][col] = tile;
        }
    }
}

static lv_obj_t *create_tile(game_2048_ui_t *ui, uint8_t row, uint8_t col, uint32_t value)
{
    lv_obj_t *tile = lv_obj_create(ui->board);
    lv_obj_remove_style_all(tile);
    lv_obj_set_size(tile, ui->tile_size, ui->tile_size);
    lv_obj_set_pos(tile, cell_pos(ui, col), cell_pos(ui, row));
    lv_obj_set_style_radius(tile, ui->radius, 0);
    lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, 0);
    lv_obj_set_style_opa(tile, LV_OPA_COVER, 0);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(tile, LV_OBJ_FLAG_EVENT_BUBBLE);

    lv_obj_t *label = lv_label_create(tile);
    lv_label_set_text_fmt(label, "%lu", (unsigned long)value);
    lv_obj_add_flag(label, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_center(label);
    apply_tile_style(ui, tile, value);
    return tile;
}

static void apply_tile_style(game_2048_ui_t *ui, lv_obj_t *tile, uint32_t value)
{
    uint32_t bg = 0x3C3A32U;
    uint32_t fg = 0xF9F6F2U;
    for(size_t i = 0; i < sizeof(k_tile_styles) / sizeof(k_tile_styles[0]); i++) {
        if(k_tile_styles[i].value == value) {
            bg = k_tile_styles[i].bg;
            fg = k_tile_styles[i].fg;
            break;
        }
    }

    lv_obj_set_style_bg_color(tile, lv_color_hex(bg), 0);
    lv_obj_t *label = lv_obj_get_child(tile, 0);
    if(label != NULL) {
        lv_obj_set_style_text_color(label, lv_color_hex(fg), 0);
        lv_obj_set_style_text_font(label, tile_font(ui, value), 0);
        lv_label_set_text_fmt(label, "%lu", (unsigned long)value);
        lv_obj_center(label);
    }
}

static const lv_font_t *tile_font(game_2048_ui_t *ui, uint32_t value)
{
    if(ui->tile_size >= 90) {
        if(value < 100U) return &lv_font_montserrat_40;
        if(value < 1000U) return &lv_font_montserrat_32;
        return &lv_font_montserrat_24;
    }
    if(ui->tile_size >= 66) {
        if(value < 100U) return &lv_font_montserrat_32;
        if(value < 1000U) return &lv_font_montserrat_24;
        return &lv_font_montserrat_20;
    }
    if(value < 100U) return &lv_font_montserrat_24;
    if(value < 1000U) return &lv_font_montserrat_20;
    return &lv_font_montserrat_16;
}

static void update_score(game_2048_ui_t *ui)
{
    lv_label_set_text_fmt(ui->score_label, "%lu", (unsigned long)ui->game.score);
    lv_label_set_text_fmt(ui->best_label, "%lu", (unsigned long)ui->game.best_score);
}

static void update_status(game_2048_ui_t *ui)
{
    switch(game_2048_core_status(&ui->game)) {
        case GAME_2048_STATUS_WON:
            lv_label_set_text(ui->status_label, "You win. Keep going.");
            break;
        case GAME_2048_STATUS_OVER:
            lv_label_set_text(ui->status_label, "Game over");
            break;
        default:
            lv_label_set_text(ui->status_label, "Use arrow keys or swipe");
            break;
    }
}

static void update_size_buttons(game_2048_ui_t *ui)
{
    for(uint8_t i = 0; i < 3U; i++) {
        uint8_t size = (uint8_t)(i + 4U);
        bool active = size == ui->grid_size;
        lv_obj_set_style_bg_color(ui->size_buttons[i], lv_color_hex(active ? 0x8F7A66U : 0xBBADA0U), 0);
        lv_obj_set_style_bg_opa(ui->size_buttons[i], LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(ui->size_buttons[i], 0, 0);
        lv_obj_t *label = lv_obj_get_child(ui->size_buttons[i], 0);
        if(label != NULL) {
            lv_obj_set_style_text_color(label, lv_color_hex(active ? 0xF9F6F2U : 0x776E65U), 0);
        }
    }
}

static void apply_move_steps(game_2048_ui_t *ui, const game_2048_move_result_t *result)
{
    survivor_info_t survivors[GAME_2048_MAX_SIZE * GAME_2048_MAX_SIZE];
    uint8_t survivor_count = 0U;

    for(uint8_t i = 0U; i < result->step_count; i++) {
        const game_2048_move_step_t *step = &result->steps[i];
        lv_obj_t *tile = ui->tiles[step->from_row][step->from_col];
        if(tile == NULL) continue;

        ui->tiles[step->from_row][step->from_col] = NULL;
        animate_tile_move(tile, cell_pos(ui, step->to_col), cell_pos(ui, step->to_row), step->disappear);
        if(step->disappear) continue;

        survivors[survivor_count].row = step->to_row;
        survivors[survivor_count].col = step->to_col;
        survivors[survivor_count].obj = tile;
        survivors[survivor_count].value = step->value_to;
        survivors[survivor_count].merged = step->merged;
        survivor_count++;
    }

    for(uint8_t i = 0U; i < survivor_count; i++) {
        const survivor_info_t *info = &survivors[i];
        ui->tiles[info->row][info->col] = info->obj;
        apply_tile_style(ui, info->obj, info->value);
        if(info->merged) animate_tile_merge(info->obj);
    }

    if(result->has_new_tile) {
        lv_obj_t *tile = create_tile(ui, result->new_tile_row, result->new_tile_col, result->new_tile_value);
        lv_obj_set_style_opa(tile, LV_OPA_TRANSP, 0);
        ui->tiles[result->new_tile_row][result->new_tile_col] = tile;
        animate_tile_spawn(tile);
    }
}

static void move_by_dir(game_2048_ui_t *ui, game_2048_dir_t dir)
{
    if(ui->animating) return;
    focus_board(ui);
    game_2048_move_result_t result;
    if(!game_2048_core_move(&ui->game, dir, &result)) return;

    ui->animating = true;
    apply_move_steps(ui, &result);
    update_score(ui);
    update_status(ui);
    ui->animating = false;
}

static void focus_board(game_2048_ui_t *ui)
{
    if(ui == NULL || ui->board == NULL) return;

    lv_group_t *group = lv_group_get_default();
    if(group == NULL) {
        group = lv_group_create();
        lv_group_set_default(group);
    }

    if(lv_obj_get_group(ui->board) != group) {
        lv_group_add_obj(group, ui->board);
    }
    lv_group_focus_obj(ui->board);
}

static void handle_swipe_point(game_2048_ui_t *ui, const lv_point_t *point)
{
    if(ui == NULL || point == NULL || !ui->pressing) return;

    int32_t dx = point->x - ui->press_point.x;
    int32_t dy = point->y - ui->press_point.y;
    int32_t threshold = ui->tile_size / 4;
    if(threshold < 16) threshold = 16;
    if(abs(dx) < threshold && abs(dy) < threshold) return;

    ui->pressing = false;
    if(abs(dx) >= abs(dy)) {
        move_by_dir(ui, dx > 0 ? GAME_2048_DIR_RIGHT : GAME_2048_DIR_LEFT);
    } else {
        move_by_dir(ui, dy > 0 ? GAME_2048_DIR_DOWN : GAME_2048_DIR_UP);
    }
}

static void event_board_cb(lv_event_t *e)
{
    game_2048_ui_t *ui = (game_2048_ui_t *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_PRESSED) {
        focus_board(ui);
        lv_indev_t *indev = lv_event_get_indev(e);
        if(indev != NULL) {
            lv_indev_get_point(indev, &ui->press_point);
            ui->pressing = true;
        }
        return;
    }

    lv_point_t point = {0, 0};
    lv_indev_t *indev = lv_event_get_indev(e);
    if(indev != NULL) lv_indev_get_point(indev, &point);

    if(code == LV_EVENT_PRESSING) {
        handle_swipe_point(ui, &point);
    } else if(code == LV_EVENT_RELEASED) {
        handle_swipe_point(ui, &point);
        ui->pressing = false;
    }
}

static void event_key_cb(lv_event_t *e)
{
    game_2048_ui_t *ui = (game_2048_ui_t *)lv_event_get_user_data(e);
    if(lv_event_get_code(e) != LV_EVENT_KEY) return;

    switch(lv_event_get_key(e)) {
        case LV_KEY_LEFT:
            move_by_dir(ui, GAME_2048_DIR_LEFT);
            break;
        case LV_KEY_RIGHT:
            move_by_dir(ui, GAME_2048_DIR_RIGHT);
            break;
        case LV_KEY_UP:
            move_by_dir(ui, GAME_2048_DIR_UP);
            break;
        case LV_KEY_DOWN:
            move_by_dir(ui, GAME_2048_DIR_DOWN);
            break;
        default:
            break;
    }
}

static void event_reset_cb(lv_event_t *e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        game_2048_ui_t *ui = (game_2048_ui_t *)lv_event_get_user_data(e);
        reset_game(ui);
        focus_board(ui);
    }
}

static void event_size_cb(lv_event_t *e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    game_2048_ui_t *ui = (game_2048_ui_t *)lv_event_get_user_data(e);
    uint8_t size = (uint8_t)(uintptr_t)lv_obj_get_user_data(lv_event_get_target(e));
    if(size < GAME_2048_MIN_SIZE || size > GAME_2048_MAX_SIZE || size == ui->grid_size) return;
    ui->grid_size = size;
    s_preferred_grid_size = size;
    reset_game(ui);
    focus_board(ui);
}

static lv_coord_t cell_pos(game_2048_ui_t *ui, uint8_t index)
{
    return (lv_coord_t)(ui->gap + index * (ui->tile_size + ui->gap));
}

static uint32_t seed_from_tick(void)
{
    uint32_t seed = lv_tick_get();
    return seed == 0U ? 1U : seed;
}

static void animate_tile_move(lv_obj_t *tile, lv_coord_t x_end, lv_coord_t y_end, bool disappear)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, tile);
    lv_anim_set_exec_cb(&anim, anim_x_cb);
    lv_anim_set_values(&anim, lv_obj_get_x(tile), x_end);
    lv_anim_set_time(&anim, GAME_2048_ANIM_MOVE_MS);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
    lv_anim_start(&anim);

    lv_anim_init(&anim);
    lv_anim_set_var(&anim, tile);
    lv_anim_set_exec_cb(&anim, anim_y_cb);
    lv_anim_set_values(&anim, lv_obj_get_y(tile), y_end);
    lv_anim_set_time(&anim, GAME_2048_ANIM_MOVE_MS);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
    if(disappear) lv_anim_set_ready_cb(&anim, anim_disappear_ready_cb);
    lv_anim_start(&anim);
}

static void animate_tile_spawn(lv_obj_t *tile)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, tile);
    lv_anim_set_exec_cb(&anim, anim_opa_cb);
    lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_time(&anim, GAME_2048_ANIM_POP_MS);
    lv_anim_start(&anim);

    lv_anim_init(&anim);
    lv_anim_set_var(&anim, tile);
    lv_anim_set_exec_cb(&anim, anim_transform_cb);
    lv_anim_set_values(&anim, 12, 0);
    lv_anim_set_time(&anim, GAME_2048_ANIM_POP_MS);
    lv_anim_set_ready_cb(&anim, anim_transform_reset_cb);
    lv_anim_start(&anim);
}

static void animate_tile_merge(lv_obj_t *tile)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, tile);
    lv_anim_set_exec_cb(&anim, anim_transform_cb);
    lv_anim_set_values(&anim, 0, 14);
    lv_anim_set_time(&anim, GAME_2048_ANIM_POP_MS);
    lv_anim_set_playback_time(&anim, 90);
    lv_anim_set_ready_cb(&anim, anim_transform_reset_cb);
    lv_anim_start(&anim);
}

static void anim_x_cb(void *obj, int32_t value)
{
    lv_obj_set_x((lv_obj_t *)obj, (lv_coord_t)value);
}

static void anim_y_cb(void *obj, int32_t value)
{
    lv_obj_set_y((lv_obj_t *)obj, (lv_coord_t)value);
}

static void anim_opa_cb(void *obj, int32_t value)
{
    lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)value, 0);
}

static void anim_transform_cb(void *obj, int32_t value)
{
    lv_obj_set_style_transform_width((lv_obj_t *)obj, value, 0);
    lv_obj_set_style_transform_height((lv_obj_t *)obj, value, 0);
}

static void anim_disappear_ready_cb(lv_anim_t *anim)
{
    lv_obj_t *tile = (lv_obj_t *)anim->var;
    if(tile == NULL) return;

    lv_anim_t fade;
    lv_anim_init(&fade);
    lv_anim_set_var(&fade, tile);
    lv_anim_set_exec_cb(&fade, anim_opa_cb);
    lv_anim_set_values(&fade, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_time(&fade, GAME_2048_ANIM_FADE_MS);
    lv_anim_set_ready_cb(&fade, anim_transform_reset_cb);
    lv_anim_start(&fade);
}

static void anim_transform_reset_cb(lv_anim_t *anim)
{
    lv_obj_t *tile = (lv_obj_t *)anim->var;
    if(tile == NULL) return;
    if(lv_obj_get_style_opa(tile, 0) == LV_OPA_TRANSP) {
        lv_obj_delete(tile);
        return;
    }
    lv_obj_set_style_transform_width(tile, 0, 0);
    lv_obj_set_style_transform_height(tile, 0, 0);
}
