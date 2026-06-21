#include "calc.h"
#include "calc_engine.h"
#include <math.h>
#include <string.h>

LV_FONT_DECLARE(desktop_font_calc_20);
LV_FONT_DECLARE(desktop_font_calc_48);
LV_FONT_DECLARE(desktop_font_calc_18);
LV_FONT_DECLARE(desktop_font_calc_16);

/* 关闭回调由 launcher 提供；calc 不直接依赖 launcher 头避免循环。
 * 通过 user_data 注入 close 函数。calc_close_cb 类型见 calc.h。 */

typedef struct {
    calc_engine_t engine;
    lv_obj_t * root;
    lv_obj_t * lbl_current;
    lv_obj_t * lbl_expr;
    lv_obj_t * panel_adv;
    lv_obj_t * btn_expand;
    bool adv_open;
    int32_t adv_h;          /* 科学面板展开高度 */
    int32_t screen_w;
    int32_t screen_h;
    calc_close_cb close_cb;
} calc_ui_t;

/* ---- 颜色（lv_color_hex 不能进静态初始化器，用运行时设置）---- */
#define COL_CARD     0x1c1c1e
#define COL_NUM      0x333333
#define COL_ACTION   0xa5a5a5
#define COL_OP       0xff9f0a
#define COL_ADV      0x3a3a3c
#define COL_EXPAND   0x2a2a2c
#define COL_RED      0xff5f56
#define COL_YELLOW   0xffbd2e
#define COL_GRAY     0x444446
#define COL_EXPR     0x8e8e93
#define COL_BORDER   0x333333

static void update_display(calc_ui_t * ui) {
    const char * s = ui->engine.current;
    size_t len = strlen(s);
    const lv_font_t * f;
    if(len > 12)      f = &desktop_font_calc_16;   /* 超长 → 更小 */
    else if(len > 8)  f = &desktop_font_calc_20;
    else              f = &desktop_font_calc_48;
    lv_obj_set_style_text_font(ui->lbl_current, f, 0);
    lv_label_set_text(ui->lbl_current, s);
    lv_label_set_text(ui->lbl_expr, ui->engine.expression);
}

static void btn_event_cb(lv_event_t * e) {
    calc_btn_t btn = (calc_btn_t)(intptr_t)lv_event_get_user_data(e);
    lv_obj_t * btn_obj = lv_event_get_target_obj(e);
    /* make_btn / adv_keys 均已 lv_obj_set_user_data(b, ui)，直取更稳，
     * 不依赖 grid 嵌套层级（科学面板按钮 vs 键盘按钮 parent 不同）。 */
    calc_ui_t * ui = lv_obj_get_user_data(btn_obj);
    (void)btn_obj;
    if(ui == NULL) return;
    calc_engine_press(&ui->engine, btn);
    update_display(ui);
}

static void expand_cb(lv_event_t * e) {
    LV_UNUSED(e);
    calc_ui_t * ui = lv_event_get_user_data(e);
    ui->adv_open = !ui->adv_open;
    /* 简化：直接 set height/opa，动画可选 */
    if(ui->adv_open) {
        lv_obj_set_height(ui->panel_adv, ui->adv_h);
        lv_obj_set_style_opa(ui->panel_adv, LV_OPA_COVER, 0);
    } else {
        lv_obj_set_height(ui->panel_adv, 0);
        lv_obj_set_style_opa(ui->panel_adv, LV_OPA_TRANSP, 0);
    }
}

static void close_cb(lv_event_t * e) {
    calc_ui_t * ui = lv_event_get_user_data(e);
    if(ui->close_cb) ui->close_cb();
}

static void calc_delete_cb(lv_event_t * e) {
    calc_ui_t * ui = lv_event_get_user_data(e);
    if(ui) lv_free(ui);
}

static lv_obj_t * make_btn(lv_obj_t * parent, uint32_t color, const char * txt,
                           const lv_font_t * font, calc_btn_t btn, calc_ui_t * ui) {
    lv_obj_t * b = lv_button_create(parent);
    lv_obj_set_size(b, 65, 65);
    lv_obj_set_style_radius(b, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(b, 0, 0);
    lv_obj_set_style_shadow_width(b, 0, 0);
    lv_obj_set_style_pad_all(b, 0, 0);
    lv_obj_set_style_text_font(b, font, 0);
    lv_obj_set_style_text_color(b,
        color == COL_ACTION ? lv_color_black() : lv_color_white(), 0);
    lv_label_set_text(lv_label_create(b), txt);
    lv_obj_set_user_data(b, ui);
    lv_obj_add_event_cb(b, btn_event_cb, LV_EVENT_CLICKED, (void *)(intptr_t)btn);
    return b;
}

/* mockup 5x4 主键盘，data-action/value 逐字对应 */
static const struct { const char * txt; uint32_t color; calc_btn_t btn; } keypad[20] = {
    {"DEL", COL_ACTION, CALC_BTN_DELETE}, {"AC", COL_ACTION, CALC_BTN_CLEAR},
    {"%",   COL_ACTION, CALC_BTN_PERCENT}, {"÷", COL_OP, CALC_BTN_DIV},
    {"7",   COL_NUM, CALC_BTN_7}, {"8", COL_NUM, CALC_BTN_8},
    {"9",   COL_NUM, CALC_BTN_9}, {"×", COL_OP, CALC_BTN_MUL},
    {"4",   COL_NUM, CALC_BTN_4}, {"5", COL_NUM, CALC_BTN_5},
    {"6",   COL_NUM, CALC_BTN_6}, {"-", COL_OP, CALC_BTN_SUB},
    {"1",   COL_NUM, CALC_BTN_1}, {"2", COL_NUM, CALC_BTN_2},
    {"3",   COL_NUM, CALC_BTN_3}, {"+", COL_OP, CALC_BTN_ADD},
    {"+/-", COL_ACTION, CALC_BTN_SIGN}, {"0", COL_NUM, CALC_BTN_0},
    {".",   COL_NUM, CALC_BTN_DOT}, {"=", COL_OP, CALC_BTN_EQ},
};

static const struct { const char * txt; calc_btn_t btn; } adv_keys[12] = {
    {"sin", CALC_BTN_SIN}, {"cos", CALC_BTN_COS}, {"tan", CALC_BTN_TAN}, {"log", CALC_BTN_LOG},
    {"ln", CALC_BTN_LN}, {"√x", CALC_BTN_SQRT}, {"x²", CALC_BTN_SQUARE}, {"x³", CALC_BTN_CUBE},
    {"(", CALC_BTN_LPAREN}, {")", CALC_BTN_RPAREN}, {"x^y", CALC_BTN_POW}, {"n!", CALC_BTN_FACTORIAL},
};

lv_obj_t * calc_create(lv_obj_t * parent, int32_t screen_w, int32_t screen_h)
{
    calc_ui_t * ui = lv_malloc_zeroed(sizeof(calc_ui_t));
    if(ui == NULL) return NULL;
    calc_engine_init(&ui->engine);
    ui->screen_w = screen_w;
    ui->screen_h = screen_h;
    ui->adv_open = false;

    /* 卡片根：360 宽，圆角20 */
    lv_obj_t * card = lv_obj_create(parent);
    ui->root = card;
    lv_obj_remove_style_all(card);
    lv_obj_set_width(card, 360);
    lv_obj_set_height(card, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(card, 20, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(COL_CARD), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_left(card, 20, 0);
    lv_obj_set_style_pad_right(card, 20, 0);
    lv_obj_set_style_pad_top(card, 20, 0);
    lv_obj_set_style_pad_bottom(card, 25, 0);
    lv_obj_set_style_pad_row(card, 10, 0);
    lv_obj_set_style_pad_column(card, 0, 0);
    lv_obj_set_style_layout(card, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_user_data(card, ui);

    /* 窗口头 */
    lv_obj_t * header = lv_obj_create(card);
    lv_obj_remove_style_all(header);
    lv_obj_set_width(header, 320);
    lv_obj_set_height(header, 30);
    lv_obj_set_style_pad_all(header, 0, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_layout(header, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * dots = lv_obj_create(header);
    lv_obj_remove_style_all(dots);
    lv_obj_set_size(dots, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(dots, 0, 0);
    lv_obj_set_style_pad_column(dots, 8, 0);
    lv_obj_clear_flag(dots, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_layout(dots, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(dots, LV_FLEX_FLOW_ROW);
    uint32_t dot_cols[3] = {COL_RED, COL_YELLOW, COL_GRAY};
    for(int i = 0; i < 3; i++) {
        lv_obj_t * d = lv_obj_create(dots);
        lv_obj_remove_style_all(d);
        lv_obj_set_size(d, 12, 12);
        lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(d, lv_color_hex(dot_cols[i]), 0);
        lv_obj_set_style_bg_opa(d, LV_OPA_COVER, 0);
        lv_obj_clear_flag(d, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
        if(i == 0) lv_obj_add_event_cb(d, close_cb, LV_EVENT_CLICKED, ui);  /* 红点=关闭 */
        else lv_obj_add_flag(d, LV_OBJ_FLAG_CLICKABLE);  /* 黄灰点吞点击不关 */
    }

    lv_obj_t * expand = lv_button_create(header);
    ui->btn_expand = expand;
    lv_obj_set_size(expand, 32, 32);
    lv_obj_set_style_radius(expand, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(expand, lv_color_hex(COL_EXPAND), 0);
    lv_obj_set_style_bg_opa(expand, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(expand, 0, 0);
    lv_obj_set_style_shadow_width(expand, 0, 0);
    lv_obj_set_style_pad_all(expand, 0, 0);
    lv_obj_set_style_text_font(expand, &desktop_font_calc_18, 0);
    lv_obj_set_style_text_color(expand, lv_color_hex(0xa0a0a0), 0);
    lv_label_set_text(lv_label_create(expand), "\xE2\x89\xA1");  /* ≡ U+2261 */
    lv_obj_add_event_cb(expand, expand_cb, LV_EVENT_CLICKED, ui);

    /* 显示屏 */
    lv_obj_t * display = lv_obj_create(card);
    lv_obj_remove_style_all(display);
    lv_obj_set_width(display, 320);
    lv_obj_set_height(display, 100);
    lv_obj_set_style_pad_all(display, 5, 0);
    lv_obj_set_style_pad_bottom(display, 15, 0);
    lv_obj_set_style_border_side(display, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(display, 1, 0);
    lv_obj_set_style_border_color(display, lv_color_hex(COL_BORDER), 0);
    lv_obj_clear_flag(display, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_layout(display, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(display, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(display, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);

    ui->lbl_expr = lv_label_create(display);
    lv_obj_set_style_text_font(ui->lbl_expr, &desktop_font_calc_18, 0);
    lv_obj_set_style_text_color(ui->lbl_expr, lv_color_hex(COL_EXPR), 0);
    lv_label_set_text(ui->lbl_expr, "");

    ui->lbl_current = lv_label_create(display);
    lv_obj_set_style_text_font(ui->lbl_current, &desktop_font_calc_48, 0);
    lv_obj_set_style_text_color(ui->lbl_current, lv_color_white(), 0);
    lv_label_set_text(ui->lbl_current, "0");

    /* 科学面板（默认折叠） */
    lv_obj_t * adv = lv_obj_create(card);
    ui->panel_adv = adv;
    lv_obj_remove_style_all(adv);
    lv_obj_set_width(adv, 320);
    lv_obj_set_height(adv, 0);
    lv_obj_set_style_bg_opa(adv, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(adv, 0, 0);
    lv_obj_set_style_pad_row(adv, 12, 0);
    lv_obj_set_style_pad_column(adv, 12, 0);
    lv_obj_clear_flag(adv, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_layout(adv, LV_LAYOUT_GRID, 0);
    static int32_t adv_col[4] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1)};
    static int32_t adv_row[3] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT};
    lv_obj_set_grid_dsc_array(adv, adv_col, adv_row);
    for(int i = 0; i < 12; i++) {
        lv_obj_t * b = lv_button_create(adv);
        lv_obj_set_size(b, 65, 40);
        lv_obj_set_style_radius(b, 12, 0);
        lv_obj_set_style_bg_color(b, lv_color_hex(COL_ADV), 0);
        lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(b, 0, 0);
        lv_obj_set_style_shadow_width(b, 0, 0);
        lv_obj_set_style_pad_all(b, 0, 0);
        lv_obj_set_style_text_font(b, &desktop_font_calc_16, 0);
        lv_obj_set_style_text_color(b, lv_color_white(), 0);
        lv_label_set_text(lv_label_create(b), adv_keys[i].txt);
        lv_obj_set_user_data(b, ui);
        lv_obj_add_event_cb(b, btn_event_cb, LV_EVENT_CLICKED, (void *)(intptr_t)adv_keys[i].btn);
        lv_obj_set_grid_cell(b, LV_GRID_ALIGN_STRETCH, i % 4, 1,
                                 LV_GRID_ALIGN_START, i / 4, 1);
    }
    ui->adv_h = 3 * 40 + 2 * 12;   /* 3行 高40 + 2 gap12 = 144 */

    /* 主键盘 grid */
    lv_obj_t * keypad_grid = lv_obj_create(card);
    lv_obj_remove_style_all(keypad_grid);
    lv_obj_set_width(keypad_grid, 320);
    lv_obj_set_height(keypad_grid, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(keypad_grid, 0, 0);
    lv_obj_set_style_pad_row(keypad_grid, 15, 0);
    lv_obj_set_style_pad_column(keypad_grid, 15, 0);
    lv_obj_clear_flag(keypad_grid, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_layout(keypad_grid, LV_LAYOUT_GRID, 0);
    static int32_t kp_col[4] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1)};
    static int32_t kp_row[5] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT};
    lv_obj_set_grid_dsc_array(keypad_grid, kp_col, kp_row);
    for(int i = 0; i < 20; i++) {
        lv_obj_t * b = make_btn(keypad_grid, keypad[i].color, keypad[i].txt,
                                &desktop_font_calc_20, keypad[i].btn, ui);
        lv_obj_set_grid_cell(b, LV_GRID_ALIGN_STRETCH, i % 4, 1,
                                 LV_GRID_ALIGN_START, i / 4, 1);
    }

    update_display(ui);

    /* 自适应缩放：取实际高，transform_scale 整体缩 */
    lv_obj_update_layout(card);
    int32_t card_h = lv_obj_get_height(card);
    int32_t target_h = (int32_t)(screen_h * 0.95);
    int32_t scale_w = (int32_t)((int64_t)screen_w * 256 / 360);
    int32_t scale_h = (int32_t)((int64_t)target_h * 256 / card_h);
    int32_t scale = scale_w < scale_h ? scale_w : scale_h;
    if(scale > 256) scale = 256;   /* 不放大 */
    if(scale < 60 * 256 / 100) {
        /* 小屏档命中区对策：缩按钮尺寸（见 spec §8） */
        /* 简化首版：保持 transform_scale，按钮命中区重叠风险留实测 */
    }
    lv_obj_set_style_transform_scale(card, (uint32_t)scale, 0);
    lv_obj_center(card);

    /* 回收 ui：launcher close 销毁 card 时触发，防止泄漏 */
    lv_obj_add_event_cb(card, calc_delete_cb, LV_EVENT_DELETE, ui);

    return card;
}

/* 供 calc_app 设置关闭回调 */
void calc_set_close_cb(lv_obj_t * card, calc_close_cb cb) {
    calc_ui_t * ui = lv_obj_get_user_data(card);
    if(ui) ui->close_cb = cb;
}
