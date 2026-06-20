#include "game_2048_ui.h"

#include <stdlib.h>
#include <string.h>

#include "lvgl/lvgl.h"

#include "game_2048.h"

typedef struct {
    game_2048_t game;
    lv_obj_t *root;
    lv_obj_t *board;
    lv_obj_t *score_label;
    lv_obj_t *status_label;
    lv_obj_t *tile_objs[GAME_2048_SIZE][GAME_2048_SIZE];
    lv_point_t drag_start;
    bool dragging;
    lv_coord_t tile_size;
    lv_coord_t gap;
    lv_coord_t tile_radius;
} game_2048_ui_t;

typedef struct {
    uint16_t value;
    uint32_t bg_color;
    uint32_t text_color;
} tile_color_t;

typedef struct {
    uint8_t row;
    uint8_t col;
    lv_obj_t *obj;
    uint16_t value;
    bool merged;
} survivor_info_t;

static game_2048_ui_t s_ui;

static const tile_color_t k_tile_colors[] = {
    {2U, 0xEEE4DAU, 0x776E65U},
    {4U, 0xEDE0C8U, 0x776E65U},
    {8U, 0xF2B179U, 0xF9F6F2U},
    {16U, 0xF59563U, 0xF9F6F2U},
    {32U, 0xF67C5FU, 0xF9F6F2U},
    {64U, 0xF65E3BU, 0xF9F6F2U},
    {128U, 0xEDCF72U, 0xF9F6F2U},
    {256U, 0xEDCC61U, 0xF9F6F2U},
    {512U, 0xEDC850U, 0xF9F6F2U},
    {1024U, 0xEDC53FU, 0xF9F6F2U},
    {2048U, 0xEDC22EU, 0xF9F6F2U}
};

static void init_layout(game_2048_ui_t *ui);
static void rebuild_tiles(game_2048_ui_t *ui);
static void board_event_cb(lv_event_t *e);
static void reset_btn_event_cb(lv_event_t *e);
static void key_event_cb(lv_event_t *e);
static void update_score(game_2048_ui_t *ui);
static void update_status(game_2048_ui_t *ui);
static void apply_move_steps(game_2048_ui_t *ui, const game_2048_move_result_t *result);
static void create_background_tiles(game_2048_ui_t *ui);
static lv_obj_t *create_tile(game_2048_ui_t *ui, uint8_t row, uint8_t col, uint16_t value);
static void apply_tile_style(game_2048_ui_t *ui, lv_obj_t *tile, uint16_t value);
static lv_coord_t cell_pos(game_2048_ui_t *ui, uint8_t index);
static void animate_tile_move(lv_obj_t *tile, lv_coord_t x_end, lv_coord_t y_end, bool disappear);
static void animate_tile_spawn(lv_obj_t *tile);
static void animate_tile_merge(lv_obj_t *tile);
static void tile_anim_exec_x(void *obj, int32_t value);
static void tile_anim_exec_y(void *obj, int32_t value);
static void tile_anim_exec_opa(void *obj, int32_t value);
static void tile_anim_exec_transform(void *obj, int32_t value);
static void tile_move_ready_cb(lv_anim_t *a);
static void tile_fade_ready_cb(lv_anim_t *a);
static void tile_transform_reset_cb(lv_anim_t *a);

void game_2048_ui_create(void)
{
    memset(&s_ui, 0, sizeof(s_ui));
    game_2048_init(&s_ui.game);
    init_layout(&s_ui);
    rebuild_tiles(&s_ui);
    update_score(&s_ui);
    update_status(&s_ui);
}

void game_2048_ui_reset(void)
{
    game_2048_reset(&s_ui.game);

    for(uint8_t row = 0U; row < GAME_2048_SIZE; row++) {
        for(uint8_t col = 0U; col < GAME_2048_SIZE; col++) {
            if(s_ui.tile_objs[row][col] != NULL) {
                lv_obj_delete(s_ui.tile_objs[row][col]);
                s_ui.tile_objs[row][col] = NULL;
            }
        }
    }

    rebuild_tiles(&s_ui);
    update_score(&s_ui);
    update_status(&s_ui);
}

static void init_layout(game_2048_ui_t *ui)
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);

    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFAF8EFU), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

    ui->root = lv_obj_create(screen);
    lv_obj_set_size(ui->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(ui->root, 16, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui->root, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui->root, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(ui->root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(ui->root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui->root, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_BETWEEN);

    lv_obj_t *header = lv_obj_create(ui->root);
    lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(header, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(header, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(header, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(header, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "LVGL 2048");
    lv_obj_set_style_text_color(title, lv_color_hex(0x776E65U), LV_PART_MAIN);

    ui->score_label = lv_label_create(header);
    lv_obj_set_style_text_color(ui->score_label, lv_color_hex(0x776E65U), LV_PART_MAIN);

    lv_obj_t *reset_btn = lv_button_create(header);
    lv_obj_set_style_bg_color(reset_btn, lv_color_hex(0x8F7A66U), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(reset_btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(reset_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(reset_btn, 8, LV_PART_MAIN);
    lv_obj_add_event_cb(reset_btn, reset_btn_event_cb, LV_EVENT_CLICKED, ui);

    lv_obj_t *btn_label = lv_label_create(reset_btn);
    lv_label_set_text(btn_label, "重置");
    lv_obj_center(btn_label);
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0xF9F6F2U), LV_PART_MAIN);

    ui->status_label = lv_label_create(ui->root);
    lv_obj_set_style_text_color(ui->status_label, lv_color_hex(0x776E65U), LV_PART_MAIN);
    lv_label_set_text(ui->status_label, "" );

    lv_display_t *disp = lv_display_get_default();
    lv_coord_t hor = lv_display_get_horizontal_resolution(disp);
    lv_coord_t ver = lv_display_get_vertical_resolution(disp);
    lv_coord_t shorter = (hor < ver) ? hor : ver;
    lv_coord_t board_size = shorter - 60;
    if(board_size < 220) {
        board_size = shorter - 30;
    }
    if(board_size < 200) {
        board_size = 200;
    }

    ui->gap = board_size / (GAME_2048_SIZE * 8);
    if(ui->gap < 6) {
        ui->gap = 6;
    }

    ui->tile_size = (board_size - ui->gap * (GAME_2048_SIZE + 1)) / GAME_2048_SIZE;
    if(ui->tile_size < 40) {
        ui->tile_size = 40;
    }

    board_size = ui->tile_size * GAME_2048_SIZE + ui->gap * (GAME_2048_SIZE + 1);
    ui->tile_radius = ui->tile_size / 6;

    ui->board = lv_obj_create(ui->root);
    lv_obj_set_size(ui->board, board_size, board_size);
    lv_obj_set_style_bg_color(ui->board, lv_color_hex(0xBBADA0U), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui->board, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(ui->board, ui->tile_radius, LV_PART_MAIN);
    lv_obj_set_style_pad_all(ui->board, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui->board, 0, LV_PART_MAIN);
    lv_obj_clear_flag(ui->board, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(ui->board, LV_LAYOUT_NONE);
    lv_obj_center(ui->board);

    create_background_tiles(ui);

    lv_obj_add_event_cb(ui->board, board_event_cb, LV_EVENT_PRESSED, ui);
    lv_obj_add_event_cb(ui->board, board_event_cb, LV_EVENT_RELEASED, ui);

    lv_group_t *group = lv_group_get_default();
    if(group != NULL) {
        lv_group_add_obj(group, ui->board);
        lv_group_focus_obj(ui->board);
    }
    lv_obj_add_event_cb(ui->board, key_event_cb, LV_EVENT_KEY, ui);
}

static void rebuild_tiles(game_2048_ui_t *ui)
{
    for(uint8_t row = 0U; row < GAME_2048_SIZE; row++) {
        for(uint8_t col = 0U; col < GAME_2048_SIZE; col++) {
            uint16_t value = game_2048_get_cell(&ui->game, row, col);
            if(value != 0U) {
                lv_obj_t *tile = create_tile(ui, row, col, value);
                apply_tile_style(ui, tile, value);
                ui->tile_objs[row][col] = tile;
            } else {
                ui->tile_objs[row][col] = NULL;
            }
        }
    }
}

static void board_event_cb(lv_event_t *e)
{
    game_2048_ui_t *ui = (game_2048_ui_t *)lv_event_get_user_data(e);
    if(ui == NULL) {
        return;
    }

    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_PRESSED) {
        lv_point_t point = {0, 0};
        lv_indev_t *indev = lv_event_get_indev(e);
        if(indev != NULL) {
            lv_indev_get_point(indev, &point);
        }
        ui->drag_start = point;
        ui->dragging = true;
        return;
    }

    if(code != LV_EVENT_RELEASED) {
        return;
    }

    if(!ui->dragging) {
        return;
    }
    ui->dragging = false;

    lv_point_t point = {0, 0};
    lv_indev_t *indev = lv_event_get_indev(e);
    if(indev != NULL) {
        lv_indev_get_point(indev, &point);
    }
    lv_coord_t dx = point.x - ui->drag_start.x;
    lv_coord_t dy = point.y - ui->drag_start.y;

    lv_coord_t threshold = ui->tile_size / 4;
    if(threshold < 12) {
        threshold = 12;
    }

    if((abs((int)dx) < threshold) && (abs((int)dy) < threshold)) {
        return;
    }

    game_2048_dir_t dir;
    if(abs((int)dx) >= abs((int)dy)) {
        dir = (dx > 0) ? GAME_2048_DIR_RIGHT : GAME_2048_DIR_LEFT;
    } else {
        dir = (dy > 0) ? GAME_2048_DIR_DOWN : GAME_2048_DIR_UP;
    }

    game_2048_status_t status = game_2048_get_status(&ui->game);
    if(status == GAME_2048_STATUS_OVER) {
        return;
    }

    game_2048_move_result_t result;
    bool moved = game_2048_move(&ui->game, dir, &result);
    if(!moved) {
        return;
    }

    apply_move_steps(ui, &result);

    if(result.has_new_tile) {
        uint8_t row = result.new_tile_row;
        uint8_t col = result.new_tile_col;
        uint16_t value = result.new_tile_value;
        lv_obj_t *tile = create_tile(ui, row, col, value);
        apply_tile_style(ui, tile, value);
        lv_obj_set_style_opa(tile, LV_OPA_TRANSP, LV_PART_MAIN);
        animate_tile_spawn(tile);
        ui->tile_objs[row][col] = tile;
    }

    update_score(ui);
    update_status(ui);
}

static void reset_btn_event_cb(lv_event_t *e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    game_2048_ui_reset();
}

static void key_event_cb(lv_event_t *e)
{
    if(lv_event_get_code(e) != LV_EVENT_KEY) {
        return;
    }

    game_2048_ui_t *ui = (game_2048_ui_t *)lv_event_get_user_data(e);
    if(ui == NULL) {
        return;
    }

    uint32_t key = lv_event_get_key(e);
    game_2048_dir_t dir;
    bool valid = true;

    switch(key) {
        case LV_KEY_LEFT:
            dir = GAME_2048_DIR_LEFT;
            break;
        case LV_KEY_RIGHT:
            dir = GAME_2048_DIR_RIGHT;
            break;
        case LV_KEY_UP:
            dir = GAME_2048_DIR_UP;
            break;
        case LV_KEY_DOWN:
            dir = GAME_2048_DIR_DOWN;
            break;
        default:
            valid = false;
            break;
    }

    if(!valid) {
        return;
    }

    game_2048_status_t status = game_2048_get_status(&ui->game);
    if(status == GAME_2048_STATUS_OVER) {
        return;
    }

    game_2048_move_result_t result;
    bool moved = game_2048_move(&ui->game, dir, &result);
    if(!moved) {
        return;
    }

    apply_move_steps(ui, &result);

    if(result.has_new_tile) {
        uint8_t row = result.new_tile_row;
        uint8_t col = result.new_tile_col;
        uint16_t value = result.new_tile_value;
        lv_obj_t *tile = create_tile(ui, row, col, value);
        apply_tile_style(ui, tile, value);
        lv_obj_set_style_opa(tile, LV_OPA_TRANSP, LV_PART_MAIN);
        animate_tile_spawn(tile);
        ui->tile_objs[row][col] = tile;
    }

    update_score(ui);
    update_status(ui);
}

static void update_score(game_2048_ui_t *ui)
{
    uint32_t score = game_2048_get_score(&ui->game);
    lv_label_set_text_fmt(ui->score_label, "分数: %lu", (unsigned long)score);
}

static void update_status(game_2048_ui_t *ui)
{
    game_2048_status_t status = game_2048_get_status(&ui->game);
    switch(status) {
        case GAME_2048_STATUS_WON:
            lv_label_set_text(ui->status_label, "目标达成！继续挑战更高分数。");
            lv_obj_set_style_text_color(ui->status_label, lv_color_hex(0x8F7A66U), LV_PART_MAIN);
            break;
        case GAME_2048_STATUS_OVER:
            lv_label_set_text(ui->status_label, "游戏结束，点击重置再来一局。");
            lv_obj_set_style_text_color(ui->status_label, lv_color_hex(0xC0392BU), LV_PART_MAIN);
            break;
        default:
            lv_label_set_text(ui->status_label, "通过鼠标或方向键滑动方块。加油！");
            lv_obj_set_style_text_color(ui->status_label, lv_color_hex(0x776E65U), LV_PART_MAIN);
            break;
    }
}

static void apply_move_steps(game_2048_ui_t *ui, const game_2048_move_result_t *result)
{
    survivor_info_t survivors[GAME_2048_SIZE * GAME_2048_SIZE];
    uint8_t survivor_count = 0U;

    for(uint8_t i = 0U; i < result->step_count; i++) {
        const game_2048_move_step_t *step = &result->steps[i];
        lv_obj_t *tile = ui->tile_objs[step->from_row][step->from_col];
        if(tile == NULL) {
            continue;
        }

        ui->tile_objs[step->from_row][step->from_col] = NULL;

        lv_coord_t x_end = cell_pos(ui, step->to_col);
        lv_coord_t y_end = cell_pos(ui, step->to_row);

        animate_tile_move(tile, x_end, y_end, step->disappear);

        if(step->disappear) {
            continue;
        }

        survivors[survivor_count].row = step->to_row;
        survivors[survivor_count].col = step->to_col;
        survivors[survivor_count].obj = tile;
        survivors[survivor_count].value = step->value_to;
        survivors[survivor_count].merged = step->merged;
        survivor_count++;
    }

    for(uint8_t i = 0U; i < survivor_count; i++) {
        const survivor_info_t *info = &survivors[i];
        ui->tile_objs[info->row][info->col] = info->obj;
        apply_tile_style(ui, info->obj, info->value);

        lv_obj_t *label = lv_obj_get_child(info->obj, 0);
        if(label != NULL) {
            lv_label_set_text_fmt(label, "%u", info->value);
        }

        if(info->merged) {
            animate_tile_merge(info->obj);
        }
    }

    for(uint8_t row = 0U; row < GAME_2048_SIZE; row++) {
        for(uint8_t col = 0U; col < GAME_2048_SIZE; col++) {
            if((game_2048_get_cell(&ui->game, row, col) == 0U) && (ui->tile_objs[row][col] != NULL)) {
                lv_obj_delete(ui->tile_objs[row][col]);
                ui->tile_objs[row][col] = NULL;
            }
        }
    }
}

static void create_background_tiles(game_2048_ui_t *ui)
{
    for(uint8_t row = 0U; row < GAME_2048_SIZE; row++) {
        for(uint8_t col = 0U; col < GAME_2048_SIZE; col++) {
            lv_obj_t *base = lv_obj_create(ui->board);
            lv_obj_set_size(base, ui->tile_size, ui->tile_size);
            lv_obj_set_style_bg_color(base, lv_color_hex(0xCDC1B4U), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(base, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_border_width(base, 0, LV_PART_MAIN);
            lv_obj_set_style_radius(base, ui->tile_radius, LV_PART_MAIN);
            lv_obj_set_pos(base, cell_pos(ui, col), cell_pos(ui, row));
            lv_obj_add_flag(base, LV_OBJ_FLAG_IGNORE_LAYOUT);
        }
    }
}

static lv_obj_t *create_tile(game_2048_ui_t *ui, uint8_t row, uint8_t col, uint16_t value)
{
    lv_obj_t *tile = lv_obj_create(ui->board);
    lv_obj_add_flag(tile, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(tile, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(tile, ui->tile_radius, LV_PART_MAIN);
    lv_obj_set_style_pad_all(tile, 0, LV_PART_MAIN);
    lv_obj_set_style_transform_width(tile, 0, LV_PART_MAIN);
    lv_obj_set_style_transform_height(tile, 0, LV_PART_MAIN);
    lv_obj_set_style_opa(tile, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(tile, ui->tile_size, ui->tile_size);
    lv_obj_set_pos(tile, cell_pos(ui, col), cell_pos(ui, row));

    lv_obj_t *label = lv_label_create(tile);
    lv_obj_center(label);
    lv_label_set_text_fmt(label, "%u", value);
    lv_obj_set_style_text_color(label, lv_color_hex(0x776E65U), LV_PART_MAIN);

    return tile;
}

static void apply_tile_style(game_2048_ui_t *ui, lv_obj_t *tile, uint16_t value)
{
    (void)ui;

    lv_color_t bg = lv_color_hex(0x3C3A32U);
    lv_color_t txt = lv_color_hex(0xF9F6F2U);

    for(size_t i = 0U; i < (sizeof(k_tile_colors) / sizeof(k_tile_colors[0])); i++) {
        if(k_tile_colors[i].value == value) {
            bg = lv_color_hex(k_tile_colors[i].bg_color);
            txt = lv_color_hex(k_tile_colors[i].text_color);
            break;
        }
    }

    if(value <= 4U) {
        txt = lv_color_hex(0x776E65U);
    }

    lv_obj_set_style_bg_color(tile, bg, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_t *label = lv_obj_get_child(tile, 0);
    if(label != NULL) {
        lv_obj_set_style_text_color(label, txt, LV_PART_MAIN);
    }
}

static lv_coord_t cell_pos(game_2048_ui_t *ui, uint8_t index)
{
    return (lv_coord_t)(ui->gap + index * (ui->tile_size + ui->gap));
}

static void animate_tile_move(lv_obj_t *tile, lv_coord_t x_end, lv_coord_t y_end, bool disappear)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, tile);
    lv_anim_set_exec_cb(&anim, tile_anim_exec_x);
    lv_anim_set_time(&anim, 140);
    lv_anim_set_values(&anim, lv_obj_get_x(tile), x_end);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
    lv_anim_start(&anim);

    lv_anim_init(&anim);
    lv_anim_set_var(&anim, tile);
    lv_anim_set_exec_cb(&anim, tile_anim_exec_y);
    lv_anim_set_time(&anim, 140);
    lv_anim_set_values(&anim, lv_obj_get_y(tile), y_end);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
    if(disappear) {
        lv_anim_set_ready_cb(&anim, tile_move_ready_cb);
    }
    lv_anim_start(&anim);
}

static void animate_tile_spawn(lv_obj_t *tile)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, tile);
    lv_anim_set_exec_cb(&anim, tile_anim_exec_opa);
    lv_anim_set_time(&anim, 160);
    lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_start(&anim);

    lv_anim_init(&anim);
    lv_anim_set_var(&anim, tile);
    lv_anim_set_exec_cb(&anim, tile_anim_exec_transform);
    lv_anim_set_time(&anim, 160);
    lv_anim_set_values(&anim, 10, 0);
    lv_anim_set_ready_cb(&anim, tile_transform_reset_cb);
    lv_anim_start(&anim);
}

static void animate_tile_merge(lv_obj_t *tile)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, tile);
    lv_anim_set_exec_cb(&anim, tile_anim_exec_transform);
    lv_anim_set_time(&anim, 160);
    lv_anim_set_values(&anim, 0, 12);
    lv_anim_set_playback_time(&anim, 120);
    lv_anim_set_playback_delay(&anim, 10);
    lv_anim_set_ready_cb(&anim, tile_transform_reset_cb);
    lv_anim_start(&anim);
}

static void tile_anim_exec_x(void *obj, int32_t value)
{
    lv_obj_set_x((lv_obj_t *)obj, (lv_coord_t)value);
}

static void tile_anim_exec_y(void *obj, int32_t value)
{
    lv_obj_set_y((lv_obj_t *)obj, (lv_coord_t)value);
}

static void tile_anim_exec_opa(void *obj, int32_t value)
{
    lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)value, LV_PART_MAIN);
}

static void tile_anim_exec_transform(void *obj, int32_t value)
{
    lv_obj_set_style_transform_width((lv_obj_t *)obj, value, LV_PART_MAIN);
    lv_obj_set_style_transform_height((lv_obj_t *)obj, value, LV_PART_MAIN);
}

static void tile_move_ready_cb(lv_anim_t *a)
{
    lv_obj_t *tile = (lv_obj_t *)a->var;
    if(tile == NULL) {
        return;
    }

    lv_anim_t fade;
    lv_anim_init(&fade);
    lv_anim_set_var(&fade, tile);
    lv_anim_set_exec_cb(&fade, tile_anim_exec_opa);
    lv_anim_set_time(&fade, 100);
    lv_anim_set_values(&fade, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_ready_cb(&fade, tile_fade_ready_cb);
    lv_anim_start(&fade);
}

static void tile_fade_ready_cb(lv_anim_t *a)
{
    lv_obj_t *tile = (lv_obj_t *)a->var;
    if(tile != NULL) {
        lv_obj_delete(tile);
    }
}

static void tile_transform_reset_cb(lv_anim_t *a)
{
    lv_obj_t *tile = (lv_obj_t *)a->var;
    if(tile == NULL) {
        return;
    }
    lv_obj_set_style_transform_width(tile, 0, LV_PART_MAIN);
    lv_obj_set_style_transform_height(tile, 0, LV_PART_MAIN);
}
