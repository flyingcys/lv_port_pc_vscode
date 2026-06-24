#include "design_fruit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "design_fruit_assets.h"
#include "design_fruit_audio.h"
#include "design_fruit_model.h"

#define APP_W 680
#define APP_H 480
#define SIDE_W 200
#define BOARD_W 480
#define CELL 60

/* 动画时长（与 HTML 体感对齐：交换滑动 / 消除缩放 / 下落每格） */
#define SWAP_MS 150
#define BLAST_MS 260
#define DROP_MS_PER_CELL 55
#define DROP_MS_MIN 130
#define SEQ_MARGIN_MS 20

/* 缩放：256 = 100%；消除时先放大到 ~1.15x 再缩到 0 */
#define SCALE_100 256
#define SCALE_PEAK 294

#define COLOR_SELECT 0xFD7418 /* HTML 选中描边橙色 */
#define COLOR_BOARD_BG 0x7bd86f

LV_FONT_DECLARE(design_fruit_font_12);
LV_FONT_DECLARE(design_fruit_font_16);

/* 序列状态机：每个一次性定时器触发后要执行的下一步动作 */
typedef enum {
    SEQ_NONE = 0,
    SEQ_AFTER_SWAP_VALID,        /* 有效交换滑动结束 -> 进入消除 */
    SEQ_AFTER_SWAP_INVALID_FWD,  /* 无效交换正向滑动结束 -> 反向滑回 */
    SEQ_AFTER_SWAP_INVALID_BACK, /* 反向滑回结束 -> 收尾（复原） */
    SEQ_AFTER_BLAST,             /* 消除缩放结束 -> 进入下落 */
    SEQ_AFTER_DROP,              /* 下落结束 -> 下一连锁回合或收尾 */
} seq_action_t;

typedef struct {
    design_fruit_model_t model;
    lv_obj_t *root;
    lv_obj_t *app;
    lv_obj_t *board;
    lv_obj_t *score_label;
    lv_obj_t *status_label;
    lv_obj_t *cells[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];   /* 透明按钮：输入 + 选中描边（在精灵之上） */
    lv_obj_t *sprites[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS]; /* 水果精灵：board 直接子对象，可跨格自由移动 */
    lv_obj_t *gameover_modal;

    uint8_t selected_row;
    uint8_t selected_col;
    bool has_selection;

    lv_point_t press_point;
    bool pressing;

    bool animating;
    design_fruit_swap_plan_t plan;
    uint8_t cur_round;
    uint8_t swap_ar, swap_ac, swap_br, swap_bc;
    seq_action_t next_action;
    lv_timer_t *seq_timer;

    uint32_t total_cleared; /* 本局累计消除水果数 */
    uint8_t max_single;     /* 单回合最多消除数 */

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
static void render_board(design_fruit_ui_t *ui, const uint8_t board[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS]);
static void sprite_clear_anims(lv_obj_t *sprite);
static void update_selection_visual(design_fruit_ui_t *ui);
static void refresh_score(design_fruit_ui_t *ui);
static void reset_game(design_fruit_ui_t *ui);

static void begin_swap(design_fruit_ui_t *ui, uint8_t ar, uint8_t ac, uint8_t br, uint8_t bc);
static void enter_blast(design_fruit_ui_t *ui);
static void enter_drop(design_fruit_ui_t *ui);
static void finish_valid(design_fruit_ui_t *ui);
static void finish_invalid(design_fruit_ui_t *ui);
static void seq_timer_cb(lv_timer_t *timer);
static void schedule_seq(design_fruit_ui_t *ui, uint32_t ms, seq_action_t action);
static void cancel_animation(design_fruit_ui_t *ui);
static uint32_t drop_duration(const design_fruit_round_plan_t *round);

static void set_selected(design_fruit_ui_t *ui, uint8_t row, uint8_t col);
static void clear_selected(design_fruit_ui_t *ui);

static void show_gameover(design_fruit_ui_t *ui);
static void close_gameover(design_fruit_ui_t *ui);

static uint32_t seed_from_tick(void);
static void cell_event_cb(lv_event_t *e);
static void reset_event_cb(lv_event_t *e);
static void music_event_cb(lv_event_t *e);
static void placeholder_event_cb(lv_event_t *e);
static void gameover_again_cb(lv_event_t *e);

static void anim_set_x(void *obj, int32_t value);
static void anim_set_y(void *obj, int32_t value);
static void anim_blast(void *obj, int32_t value);
static void start_anim(lv_obj_t *obj, lv_anim_exec_xcb_t exec_cb, int32_t from, int32_t to, uint32_t duration);

static inline int32_t home_x(uint8_t col) { return (int32_t)col * CELL; }
static inline int32_t home_y(uint8_t row) { return (int32_t)row * CELL; }

lv_obj_t *design_fruit_create(lv_obj_t *parent, int32_t screen_w, int32_t screen_h)
{
    if(s_ui.root != NULL) return s_ui.root;
    if(parent == NULL) parent = lv_screen_active();

    memset(&s_ui, 0, sizeof(s_ui));
    s_ui.screen_w = screen_w;
    s_ui.screen_h = screen_h;
    design_fruit_assets_init(NULL);
    design_fruit_audio_init();
    create_layout(&s_ui, parent);
    reset_game(&s_ui);
    if(design_fruit_audio_bgm_enabled()) design_fruit_audio_play_bgm();
    return s_ui.root;
}

void design_fruit_start(void)
{
    if(s_ui.root != NULL) reset_game(&s_ui);
}

void design_fruit_stop(void)
{
    if(s_ui.root != NULL) {
        cancel_animation(&s_ui);
        design_fruit_audio_deinit();
        lv_obj_delete(s_ui.root);
    }
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
        lv_obj_clear_flag(bg, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_move_background(bg);
    }

    lv_obj_t *logo = lv_image_create(panel);
    if(design_fruit_assets_build_image_path(path, sizeof(path), "png/logo_200.png")) {
        lv_image_set_src(logo, path);
    }
    lv_obj_set_pos(logo, 0, 16);
    lv_obj_clear_flag(logo, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *score_circle = lv_obj_create(panel);
    lv_obj_set_size(score_circle, 128, 128);
    lv_obj_set_pos(score_circle, 36, 86);
    lv_obj_set_style_radius(score_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(score_circle, lv_color_hex(0x5d9cec), 0);
    lv_obj_set_style_bg_opa(score_circle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(score_circle, 4, 0);
    lv_obj_set_style_border_color(score_circle, lv_color_hex(0xffffff), 0);
    lv_obj_clear_flag(score_circle, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(score_circle, LV_OBJ_FLAG_CLICKABLE);

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
    create_small_button(panel, "背景音乐", 12, 436, music_event_cb, ui);
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
    lv_obj_set_style_bg_color(board, lv_color_hex(COLOR_BOARD_BG), 0);
    lv_obj_set_style_bg_opa(board, LV_OPA_COVER, 0);
    /* 关键：裁切子对象到棋盘范围，使从顶部之上落入的新水果被棋盘上边缘裁切（同 HTML canvas 顶部） */
    lv_obj_set_style_clip_corner(board, true, 0);
    lv_obj_clear_flag(board, LV_OBJ_FLAG_SCROLLABLE);
    ui->board = board;

    if(design_fruit_assets_build_image_path(path, sizeof(path), "png/bg.png")) {
        lv_obj_t *bg = lv_image_create(board);
        lv_image_set_src(bg, path);
        lv_image_set_inner_align(bg, LV_IMAGE_ALIGN_TILE);
        lv_obj_set_size(bg, BOARD_W, BOARD_W);
        lv_obj_set_pos(bg, 0, 0);
        lv_obj_clear_flag(bg, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_move_background(bg);
    }

    /* 先建精灵层（在底，承载水果与动画），再建透明按钮层（在上，承载输入与选中描边） */
    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
            lv_obj_t *sprite = lv_image_create(board);
            lv_obj_set_size(sprite, CELL, CELL);
            lv_obj_set_pos(sprite, home_x(col), home_y(row));
            lv_image_set_pivot(sprite, CELL / 2, CELL / 2);
            lv_obj_clear_flag(sprite, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_clear_flag(sprite, LV_OBJ_FLAG_SCROLLABLE);
            ui->sprites[row][col] = sprite;
        }
    }

    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
            lv_obj_t *cell = lv_button_create(board);
            lv_obj_remove_style_all(cell);
            lv_obj_set_size(cell, CELL, CELL);
            lv_obj_set_pos(cell, home_x(col), home_y(row));
            lv_obj_set_style_radius(cell, 6, 0);
            lv_obj_set_style_bg_opa(cell, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_color(cell, lv_color_hex(COLOR_SELECT), 0);
            lv_obj_set_style_border_width(cell, 0, 0);
            lv_obj_set_user_data(cell, (void *)(uintptr_t)((row << 4) | col));
            lv_obj_add_event_cb(cell, cell_event_cb, LV_EVENT_PRESSED, ui);
            lv_obj_add_event_cb(cell, cell_event_cb, LV_EVENT_RELEASED, ui);
            lv_obj_add_event_cb(cell, cell_event_cb, LV_EVENT_CLICKED, ui);
            lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
            ui->cells[row][col] = cell;
        }
    }
}

static void sprite_clear_anims(lv_obj_t *sprite)
{
    lv_anim_delete(sprite, anim_set_x);
    lv_anim_delete(sprite, anim_set_y);
    lv_anim_delete(sprite, anim_blast);
}

static void render_board(design_fruit_ui_t *ui, const uint8_t board[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS])
{
    char path[512];

    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
            lv_obj_t *sprite = ui->sprites[row][col];
            uint8_t value = board[row][col];

            sprite_clear_anims(sprite);
            if(value >= 1u && value <= DESIGN_FRUIT_TYPES &&
               design_fruit_assets_build_image_path(path, sizeof(path), k_fruit_paths[value])) {
                lv_image_set_src(sprite, path);
                lv_obj_clear_flag(sprite, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(sprite, LV_OBJ_FLAG_HIDDEN);
            }
            lv_image_set_scale(sprite, SCALE_100);
            lv_obj_set_pos(sprite, home_x(col), home_y(row));
        }
    }
    update_selection_visual(ui);
}

static void update_selection_visual(design_fruit_ui_t *ui)
{
    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
            bool sel = ui->has_selection && ui->selected_row == row && ui->selected_col == col;
            lv_obj_set_style_border_width(ui->cells[row][col], sel ? 3 : 0, 0);
        }
    }
}

static void refresh_score(design_fruit_ui_t *ui)
{
    char text[32];
    snprintf(text, sizeof(text), "%u", (unsigned)design_fruit_model_score(&ui->model));
    lv_label_set_text(ui->score_label, text);
}

static void reset_game(design_fruit_ui_t *ui)
{
    cancel_animation(ui);
    close_gameover(ui);
    design_fruit_model_init(&ui->model, seed_from_tick());
    ui->total_cleared = 0;
    ui->max_single = 0;
    clear_selected(ui);
    lv_label_set_text(ui->status_label, "开始");
    render_board(ui, ui->model.board);
    refresh_score(ui);
}

/* ── 动画序列 ──────────────────────────────────────────────────────────── */

static uint32_t drop_duration(const design_fruit_round_plan_t *round)
{
    uint8_t max_cells = 1;
    uint32_t dur;
    for(uint16_t i = 0; i < round->move_count; i++) {
        uint8_t cells = (uint8_t)(round->moves[i].to_row - round->moves[i].from_row);
        if(cells > max_cells) max_cells = cells;
    }
    for(uint16_t i = 0; i < round->spawn_count; i++) {
        if(round->spawns[i].drop_cells > max_cells) max_cells = round->spawns[i].drop_cells;
    }
    dur = (uint32_t)max_cells * DROP_MS_PER_CELL;
    return dur < DROP_MS_MIN ? DROP_MS_MIN : dur;
}

static void schedule_seq(design_fruit_ui_t *ui, uint32_t ms, seq_action_t action)
{
    ui->next_action = action;
    ui->seq_timer = lv_timer_create(seq_timer_cb, ms, ui);
    lv_timer_set_repeat_count(ui->seq_timer, 1); /* 一次性：触发后由 LVGL 自动删除 */
}

static void seq_timer_cb(lv_timer_t *timer)
{
    design_fruit_ui_t *ui = (design_fruit_ui_t *)lv_timer_get_user_data(timer);
    seq_action_t action = ui->next_action;

    ui->seq_timer = NULL; /* 此定时器即将被 LVGL 自动删除 */
    ui->next_action = SEQ_NONE;

    switch(action) {
        case SEQ_AFTER_SWAP_VALID:
            ui->cur_round = 0;
            enter_blast(ui);
            break;

        case SEQ_AFTER_SWAP_INVALID_FWD: {
            lv_obj_t *sa = ui->sprites[ui->swap_ar][ui->swap_ac];
            lv_obj_t *sb = ui->sprites[ui->swap_br][ui->swap_bc];
            start_anim(sa, anim_set_x, home_x(ui->swap_bc), home_x(ui->swap_ac), SWAP_MS);
            start_anim(sa, anim_set_y, home_y(ui->swap_br), home_y(ui->swap_ar), SWAP_MS);
            start_anim(sb, anim_set_x, home_x(ui->swap_ac), home_x(ui->swap_bc), SWAP_MS);
            start_anim(sb, anim_set_y, home_y(ui->swap_ar), home_y(ui->swap_br), SWAP_MS);
            lv_label_set_text(ui->status_label, "不能交换");
            schedule_seq(ui, SWAP_MS + SEQ_MARGIN_MS, SEQ_AFTER_SWAP_INVALID_BACK);
            break;
        }

        case SEQ_AFTER_SWAP_INVALID_BACK:
            finish_invalid(ui);
            break;

        case SEQ_AFTER_BLAST:
            enter_drop(ui);
            break;

        case SEQ_AFTER_DROP:
            ui->cur_round++;
            if(ui->cur_round < ui->plan.round_count) {
                enter_blast(ui);
            } else {
                finish_valid(ui);
            }
            break;

        case SEQ_NONE:
        default:
            break;
    }
}

static void begin_swap(design_fruit_ui_t *ui, uint8_t ar, uint8_t ac, uint8_t br, uint8_t bc)
{
    bool accepted;
    lv_obj_t *sa;
    lv_obj_t *sb;

    if(ui->animating) return;

    clear_selected(ui);
    ui->animating = true;
    ui->swap_ar = ar;
    ui->swap_ac = ac;
    ui->swap_br = br;
    ui->swap_bc = bc;

    accepted = design_fruit_model_swap_with_plan(&ui->model, ar, ac, br, bc, &ui->plan);

    /* 两个水果对滑交换（视觉） */
    sa = ui->sprites[ar][ac];
    sb = ui->sprites[br][bc];
    start_anim(sa, anim_set_x, home_x(ac), home_x(bc), SWAP_MS);
    start_anim(sa, anim_set_y, home_y(ar), home_y(br), SWAP_MS);
    start_anim(sb, anim_set_x, home_x(bc), home_x(ac), SWAP_MS);
    start_anim(sb, anim_set_y, home_y(br), home_y(ar), SWAP_MS);

    if(accepted) {
        lv_label_set_text(ui->status_label, "消除成功");
        schedule_seq(ui, SWAP_MS + SEQ_MARGIN_MS, SEQ_AFTER_SWAP_VALID);
    } else {
        schedule_seq(ui, SWAP_MS + SEQ_MARGIN_MS, SEQ_AFTER_SWAP_INVALID_FWD);
    }
}

static void enter_blast(design_fruit_ui_t *ui)
{
    const design_fruit_round_plan_t *round = &ui->plan.rounds[ui->cur_round];

    render_board(ui, round->board_before);
    ui->total_cleared += round->match_count;
    if(round->match_count > ui->max_single) ui->max_single = round->match_count;

    design_fruit_audio_play_hit();

    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
            if(round->marks[row][col]) {
                start_anim(ui->sprites[row][col], anim_blast, 0, (int32_t)BLAST_MS, BLAST_MS);
            }
        }
    }
    schedule_seq(ui, BLAST_MS + SEQ_MARGIN_MS, SEQ_AFTER_BLAST);
}

static void enter_drop(design_fruit_ui_t *ui)
{
    const design_fruit_round_plan_t *round = &ui->plan.rounds[ui->cur_round];
    uint32_t dur = drop_duration(round);

    render_board(ui, round->board_after);

    /* 棋盘内幸存水果向下滑落 */
    for(uint16_t i = 0; i < round->move_count; i++) {
        const design_fruit_move_t *m = &round->moves[i];
        lv_obj_t *sprite = ui->sprites[m->to_row][m->to_col];
        int32_t start_y = home_y(m->to_row) - (int32_t)(m->to_row - m->from_row) * CELL;
        lv_obj_set_y(sprite, start_y);
        start_anim(sprite, anim_set_y, start_y, home_y(m->to_row), dur);
    }
    /* 新水果从棋盘顶部之上落入（被棋盘上边缘裁切） */
    for(uint16_t i = 0; i < round->spawn_count; i++) {
        const design_fruit_spawn_t *sp = &round->spawns[i];
        lv_obj_t *sprite = ui->sprites[sp->row][sp->col];
        int32_t start_y = home_y(sp->row) - (int32_t)sp->drop_cells * CELL;
        lv_obj_set_y(sprite, start_y);
        start_anim(sprite, anim_set_y, start_y, home_y(sp->row), dur);
    }
    schedule_seq(ui, dur + SEQ_MARGIN_MS, SEQ_AFTER_DROP);
}

static void finish_valid(design_fruit_ui_t *ui)
{
    render_board(ui, ui->plan.final_board);
    refresh_score(ui);
    ui->animating = false;
    ui->next_action = SEQ_NONE;

    if(!design_fruit_model_has_available_move(&ui->model)) {
        design_fruit_audio_play_over();
        show_gameover(ui);
    }
}

static void finish_invalid(design_fruit_ui_t *ui)
{
    render_board(ui, ui->model.board); /* 模型在拒绝时已复原为原棋盘 */
    refresh_score(ui);
    ui->animating = false;
    ui->next_action = SEQ_NONE;
}

static void cancel_animation(design_fruit_ui_t *ui)
{
    if(ui->seq_timer != NULL) {
        lv_timer_delete(ui->seq_timer);
        ui->seq_timer = NULL;
    }
    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
            if(ui->sprites[row][col] != NULL) lv_anim_delete(ui->sprites[row][col], NULL);
        }
    }
    ui->animating = false;
    ui->next_action = SEQ_NONE;
}

/* ── 选中 ──────────────────────────────────────────────────────────────── */

static void set_selected(design_fruit_ui_t *ui, uint8_t row, uint8_t col)
{
    ui->selected_row = row;
    ui->selected_col = col;
    ui->has_selection = true;
    update_selection_visual(ui);
}

static void clear_selected(design_fruit_ui_t *ui)
{
    ui->has_selection = false;
    update_selection_visual(ui);
}

/* ── 游戏结束弹窗 ─────────────────────────────────────────────────────── */

static void show_gameover(design_fruit_ui_t *ui)
{
    char body[96];

    close_gameover(ui);

    lv_obj_t *modal = lv_obj_create(ui->root);
    lv_obj_remove_style_all(modal);
    lv_obj_set_size(modal, ui->screen_w, ui->screen_h);
    lv_obj_set_pos(modal, 0, 0);
    lv_obj_set_style_bg_color(modal, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(modal, 140, 0);
    lv_obj_add_flag(modal, LV_OBJ_FLAG_CLICKABLE); /* 拦截弹窗后面的点击 */
    lv_obj_clear_flag(modal, LV_OBJ_FLAG_SCROLLABLE);
    ui->gameover_modal = modal;

    lv_obj_t *panel = lv_obj_create(modal);
    lv_obj_set_size(panel, 320, 210);
    lv_obj_center(panel);
    lv_obj_set_style_radius(panel, 10, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 16, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(panel);
    lv_label_set_text(title, "游戏结束");
    lv_obj_set_style_text_color(title, lv_color_hex(0x333333), 0);
    lv_obj_set_style_text_font(title, &design_fruit_font_16, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

    snprintf(body, sizeof(body), "无法移动\n本局得分 %u\n共消除 %u 个\n单次最多 %u 个",
             (unsigned)design_fruit_model_score(&ui->model),
             (unsigned)ui->total_cleared,
             (unsigned)ui->max_single);
    lv_obj_t *info = lv_label_create(panel);
    lv_label_set_text(info, body);
    lv_obj_set_style_text_align(info, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(info, lv_color_hex(0x555555), 0);
    lv_obj_set_style_text_font(info, &design_fruit_font_16, 0);
    lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 34);

    lv_obj_t *again = lv_button_create(panel);
    lv_obj_set_size(again, 140, 38);
    lv_obj_align(again, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_radius(again, 4, 0);
    lv_obj_set_style_bg_color(again, lv_color_hex(0x5d9cec), 0);
    lv_obj_add_event_cb(again, gameover_again_cb, LV_EVENT_CLICKED, ui);

    lv_obj_t *again_lbl = lv_label_create(again);
    lv_label_set_text(again_lbl, "再来一局");
    lv_obj_set_style_text_color(again_lbl, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(again_lbl, &design_fruit_font_16, 0);
    lv_obj_center(again_lbl);
}

static void close_gameover(design_fruit_ui_t *ui)
{
    if(ui->gameover_modal != NULL) {
        lv_obj_delete(ui->gameover_modal);
        ui->gameover_modal = NULL;
    }
}

/* ── 事件回调 ─────────────────────────────────────────────────────────── */

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

static void anim_blast(void *obj, int32_t value)
{
    int32_t grow = BLAST_MS * 35 / 100;
    int32_t scale;
    if(value <= grow) {
        scale = SCALE_100 + (SCALE_PEAK - SCALE_100) * value / grow;
    } else {
        int32_t shrink = BLAST_MS - grow;
        int32_t elapsed = value - grow;
        scale = SCALE_PEAK - SCALE_PEAK * elapsed / shrink;
    }
    if(scale < 1) scale = 1;
    lv_image_set_scale((lv_obj_t *)obj, (uint16_t)scale);
}

static void start_anim(lv_obj_t *obj, lv_anim_exec_xcb_t exec_cb, int32_t from, int32_t to, uint32_t duration)
{
    lv_anim_t anim;
    lv_anim_delete(obj, exec_cb);
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_exec_cb(&anim, exec_cb);
    lv_anim_set_values(&anim, from, to);
    lv_anim_set_duration(&anim, duration);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_start(&anim);
}

static void cell_event_cb(lv_event_t *e)
{
    design_fruit_ui_t *ui = (design_fruit_ui_t *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target_obj(e);
    uint8_t packed = (uint8_t)(uintptr_t)lv_obj_get_user_data(target);
    uint8_t row = packed >> 4;
    uint8_t col = packed & 0x0fu;

    if(ui->animating || ui->gameover_modal != NULL) return;

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
            if(dx > 0 && col + 1u < DESIGN_FRUIT_COLS) begin_swap(ui, row, col, row, (uint8_t)(col + 1u));
            else if(dx < 0 && col > 0u) begin_swap(ui, row, col, row, (uint8_t)(col - 1u));
        } else {
            if(dy > 0 && row + 1u < DESIGN_FRUIT_ROWS) begin_swap(ui, row, col, (uint8_t)(row + 1u), col);
            else if(dy < 0 && row > 0u) begin_swap(ui, row, col, (uint8_t)(row - 1u), col);
        }
        return;
    }

    if(code == LV_EVENT_CLICKED) {
        if(ui->has_selection) {
            uint8_t prev_row = ui->selected_row;
            uint8_t prev_col = ui->selected_col;
            if(prev_row == row && prev_col == col) {
                clear_selected(ui);
            } else if((abs((int)prev_row - (int)row) + abs((int)prev_col - (int)col)) == 1) {
                begin_swap(ui, prev_row, prev_col, row, col);
            } else {
                set_selected(ui, row, col);
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

static void music_event_cb(lv_event_t *e)
{
    design_fruit_ui_t *ui = (design_fruit_ui_t *)lv_event_get_user_data(e);
    if(lv_event_get_code(e) == LV_EVENT_CLICKED && ui != NULL) {
        bool on = design_fruit_audio_toggle_bgm();
        lv_label_set_text(ui->status_label, on ? "音乐开启" : "音乐关闭");
    }
}

static void placeholder_event_cb(lv_event_t *e)
{
    design_fruit_ui_t *ui = (design_fruit_ui_t *)lv_event_get_user_data(e);
    if(lv_event_get_code(e) == LV_EVENT_CLICKED && ui != NULL) {
        lv_label_set_text(ui->status_label, "占位功能");
    }
}

static void gameover_again_cb(lv_event_t *e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        reset_game((design_fruit_ui_t *)lv_event_get_user_data(e));
    }
}

/* ── 测试 / 无头快照演示钩子 ─────────────────────────────────────────── */

void design_fruit_test_set_board(const uint8_t *board8x8)
{
    uint8_t board[DESIGN_FRUIT_ROWS][DESIGN_FRUIT_COLS];
    if(s_ui.root == NULL || board8x8 == NULL) return;

    cancel_animation(&s_ui);
    for(uint8_t row = 0; row < DESIGN_FRUIT_ROWS; row++) {
        for(uint8_t col = 0; col < DESIGN_FRUIT_COLS; col++) {
            board[row][col] = board8x8[row * DESIGN_FRUIT_COLS + col];
        }
    }
    design_fruit_model_set_board(&s_ui.model, board);
    clear_selected(&s_ui);
    render_board(&s_ui, s_ui.model.board);
}

void design_fruit_test_swap(uint8_t row_a, uint8_t col_a, uint8_t row_b, uint8_t col_b)
{
    if(s_ui.root == NULL) return;
    begin_swap(&s_ui, row_a, col_a, row_b, col_b);
}

bool design_fruit_test_is_animating(void)
{
    return s_ui.root != NULL && s_ui.animating;
}

void design_fruit_test_show_gameover(void)
{
    if(s_ui.root == NULL) return;
    s_ui.total_cleared = 12;
    s_ui.max_single = 5;
    show_gameover(&s_ui);
}
