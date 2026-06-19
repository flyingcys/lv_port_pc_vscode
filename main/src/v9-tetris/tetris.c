/* main/src/v9-tetris/tetris.c
 * 竖屏俄罗斯方块,复刻 ds1.html。
 * 渲染:lv_canvas(RGB565),仅在状态变化时重绘。逻辑对齐 ds1.html。
 */
#include "lvgl/lvgl.h"
#include "tetris.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

LV_FONT_DECLARE(tetris_font_18);
LV_FONT_DECLARE(tetris_font_13);

#define T_COLS 10
#define T_ROWS 20

/* 颜色对齐 ds1.html COLORS(索引 1..7)*/
static const uint32_t T_COLORS[8] = {
    0x000000, 0xFF0D72, 0x0DC2FF, 0x0DFF72, 0xF538FF, 0xFF8E0D, 0xFFE138, 0x3877FF
};

typedef struct { int dim; int cells[4][4]; } shape_t;
/* 7 种方块(方阵),数字=颜色索引,对齐 ds1.html SHAPES */
static const shape_t T_SHAPES[8] = {
    {0, {{0}}},
    {4, {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}}, /* I */
    {3, {{2,0,0},{2,2,2},{0,0,0}}},                 /* J */
    {3, {{0,0,3},{3,3,3},{0,0,0}}},                 /* L */
    {2, {{4,4},{4,4}}},                             /* O */
    {3, {{0,5,5},{5,5,0},{0,0,0}}},                 /* S */
    {3, {{0,6,0},{6,6,6},{0,0,0}}},                 /* T */
    {3, {{7,7,0},{0,7,7},{0,0,0}}},                 /* Z */
};

typedef struct {
    int dim;
    int cells[4][4];
    int color_id;
    int x, y;
} piece_t;

typedef struct {
    int   board[T_ROWS][T_COLS];
    piece_t cur, next;
    int   score, level, lines;
    uint32_t drop_interval;
    bool  paused, game_over, started;

    lv_obj_t  *scr;
    lv_obj_t  *board_canvas, *next_canvas;
    uint8_t   *board_buf, *next_buf;
    int        block, nblock;
    lv_obj_t  *lbl_score, *lbl_level, *lbl_lines;
    lv_obj_t  *btn_sp, *lbl_sp;          /* 开始/暂停切换按钮 */
    lv_obj_t  *overlay, *lbl_final;
    lv_timer_t *timer;
    const lv_font_t *f_big, *f_small;
    bool  ready;
} tetris_t;

static tetris_t g;

/* ── 逻辑(对齐 ds1.html)──────────────────────────────────────────────── */

static void make_piece(piece_t *p)
{
    int id = rand() % 7 + 1;
    const shape_t *s = &T_SHAPES[id];
    p->dim = s->dim;
    memcpy(p->cells, s->cells, sizeof(p->cells));
    p->color_id = id;
    p->x = T_COLS / 2 - s->dim / 2;
    p->y = 0;
}

static bool collides(const piece_t *p)
{
    for(int y = 0; y < p->dim; y++)
        for(int x = 0; x < p->dim; x++) {
            if(!p->cells[y][x]) continue;
            int nx = p->x + x, ny = p->y + y;
            if(nx < 0 || nx >= T_COLS || ny >= T_ROWS) return true;
            if(ny >= 0 && g.board[ny][nx]) return true;
        }
    return false;
}

static void draw_board(void);
static void draw_next(void);
static void refresh_labels(void);
static void end_game(void);

static void update_score(int n)
{
    static const int pts[5] = {0, 100, 300, 600, 1000};
    g.score += pts[n] * g.level;
    g.lines += n;
    g.level = g.lines / 10 + 1;
    g.drop_interval = LV_MAX(100u, 1000u - (uint32_t)(g.level - 1) * 100u);
    if(g.timer) lv_timer_set_period(g.timer, g.drop_interval);
    refresh_labels();
}

static void check_lines(void)
{
    int cleared = 0;
    for(int y = T_ROWS - 1; y >= 0; y--) {
        bool full = true;
        for(int x = 0; x < T_COLS; x++) if(!g.board[y][x]) { full = false; break; }
        if(full) {
            for(int yy = y; yy > 0; yy--) memcpy(g.board[yy], g.board[yy - 1], sizeof(g.board[0]));
            memset(g.board[0], 0, sizeof(g.board[0]));
            cleared++;
            y++;  /* 重新检查当前行 */
        }
    }
    if(cleared > 0) update_score(cleared);
}

static void place_piece(void)
{
    for(int y = 0; y < g.cur.dim; y++)
        for(int x = 0; x < g.cur.dim; x++) {
            if(!g.cur.cells[y][x]) continue;
            int by = g.cur.y + y;
            if(by < 0) { end_game(); return; }
            g.board[by][g.cur.x + x] = g.cur.cells[y][x];
        }
    check_lines();
    g.cur = g.next;
    make_piece(&g.next);
    draw_next();
    if(collides(&g.cur)) end_game();
}

/* 返回 true 表示移动成功;dy>0 撞底时落子并返回 false */
static bool move_cur(int dx, int dy)
{
    if(g.paused || g.game_over || !g.started) return false;
    g.cur.x += dx; g.cur.y += dy;
    if(collides(&g.cur)) {
        g.cur.x -= dx; g.cur.y -= dy;
        if(dy > 0) { place_piece(); draw_board(); return false; }
    }
    draw_board();
    return true;
}

static void rotate_cur(void)
{
    if(g.paused || g.game_over || !g.started) return;
    int d = g.cur.dim, ox = g.cur.x;
    int old[4][4]; memcpy(old, g.cur.cells, sizeof(old));
    int nc[4][4] = {{0}};
    for(int y = 0; y < d; y++)
        for(int x = 0; x < d; x++)
            nc[x][d - 1 - y] = old[y][x];
    memcpy(g.cur.cells, nc, sizeof(nc));
    if(collides(&g.cur)) {
        g.cur.x--;
        if(collides(&g.cur)) {
            g.cur.x += 2;
            if(collides(&g.cur)) { g.cur.x = ox; memcpy(g.cur.cells, old, sizeof(old)); }
        }
    }
    draw_board();
}

static void hard_drop(void)
{
    if(g.paused || g.game_over || !g.started) return;
    while(move_cur(0, 1)) { }
}

/* ── 渲染 ─────────────────────────────────────────────────────────────── */

static void draw_cell(lv_layer_t *layer, int px, int py, int bs, lv_color_t color)
{
    int edge = LV_MAX(2, bs / 6);

    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.bg_color = color; d.bg_opa = LV_OPA_COVER;
    d.border_color = lv_color_black(); d.border_width = (bs >= 16) ? 2 : 1; d.border_opa = LV_OPA_COVER;
    lv_area_t a = {px, py, px + bs - 1, py + bs - 1};
    lv_draw_rect(layer, &d, &a);

    /* 左上高光 */
    lv_draw_rect_dsc_t hi; lv_draw_rect_dsc_init(&hi);
    hi.bg_color = lv_color_white(); hi.bg_opa = LV_OPA_20;
    lv_area_t top = {px, py, px + bs - 1, py + edge - 1};        lv_draw_rect(layer, &hi, &top);
    lv_area_t lft = {px, py, px + edge - 1, py + bs - 1};        lv_draw_rect(layer, &hi, &lft);
    /* 右下暗边 */
    lv_draw_rect_dsc_t sh; lv_draw_rect_dsc_init(&sh);
    sh.bg_color = lv_color_black(); sh.bg_opa = LV_OPA_20;
    lv_area_t bot = {px, py + bs - edge, px + bs - 1, py + bs - 1}; lv_draw_rect(layer, &sh, &bot);
    lv_area_t rgt = {px + bs - edge, py, px + bs - 1, py + bs - 1}; lv_draw_rect(layer, &sh, &rgt);
}

static void draw_board(void)
{
    if(!g.ready) return;
    int bs = g.block;
    lv_layer_t layer;
    lv_canvas_init_layer(g.board_canvas, &layer);

    /* 背景 #111 */
    lv_draw_rect_dsc_t bg; lv_draw_rect_dsc_init(&bg);
    bg.bg_color = lv_color_hex(0x111111); bg.bg_opa = LV_OPA_COVER;
    lv_area_t full = {0, 0, T_COLS * bs - 1, T_ROWS * bs - 1};
    lv_draw_rect(&layer, &bg, &full);

    /* 已固定方块 */
    for(int y = 0; y < T_ROWS; y++)
        for(int x = 0; x < T_COLS; x++)
            if(g.board[y][x])
                draw_cell(&layer, x * bs, y * bs, bs, lv_color_hex(T_COLORS[g.board[y][x]]));

    /* 当前方块 */
    if(g.started)
        for(int y = 0; y < g.cur.dim; y++)
            for(int x = 0; x < g.cur.dim; x++)
                if(g.cur.cells[y][x]) {
                    int gx = g.cur.x + x, gy = g.cur.y + y;
                    if(gy >= 0) draw_cell(&layer, gx * bs, gy * bs, bs, lv_color_hex(T_COLORS[g.cur.color_id]));
                }

    /* 网格线 white 10% */
    lv_draw_rect_dsc_t gl; lv_draw_rect_dsc_init(&gl);
    gl.bg_color = lv_color_white(); gl.bg_opa = LV_OPA_10;
    for(int x = 1; x < T_COLS; x++) { lv_area_t a = {x * bs, 0, x * bs, T_ROWS * bs - 1}; lv_draw_rect(&layer, &gl, &a); }
    for(int y = 1; y < T_ROWS; y++) { lv_area_t a = {0, y * bs, T_COLS * bs - 1, y * bs}; lv_draw_rect(&layer, &gl, &a); }

    lv_canvas_finish_layer(g.board_canvas, &layer);
    lv_obj_invalidate(g.board_canvas);
}

static void draw_next(void)
{
    if(!g.ready || !g.next_canvas) return;
    int bs = g.nblock;
    lv_layer_t layer;
    lv_canvas_init_layer(g.next_canvas, &layer);

    lv_draw_rect_dsc_t bg; lv_draw_rect_dsc_init(&bg);
    bg.bg_color = lv_color_hex(0x111111); bg.bg_opa = LV_OPA_COVER;
    lv_area_t full = {0, 0, 4 * bs - 1, 4 * bs - 1};
    lv_draw_rect(&layer, &bg, &full);

    int off_x = (4 - g.next.dim) / 2, off_y = (4 - g.next.dim) / 2;
    for(int y = 0; y < g.next.dim; y++)
        for(int x = 0; x < g.next.dim; x++)
            if(g.next.cells[y][x])
                draw_cell(&layer, (off_x + x) * bs, (off_y + y) * bs, bs, lv_color_hex(T_COLORS[g.next.color_id]));

    lv_canvas_finish_layer(g.next_canvas, &layer);
    lv_obj_invalidate(g.next_canvas);
}

static void refresh_labels(void)
{
    if(g.lbl_score) lv_label_set_text_fmt(g.lbl_score, "得分: %d", g.score);
    if(g.lbl_level) lv_label_set_text_fmt(g.lbl_level, "等级: %d", g.level);
    if(g.lbl_lines) lv_label_set_text_fmt(g.lbl_lines, "已消除: %d", g.lines);
}

static void update_sp_label(void)
{
    if(!g.lbl_sp) return;
    if(!g.started)      lv_label_set_text(g.lbl_sp, "开始");
    else if(g.paused)   lv_label_set_text(g.lbl_sp, "继续");
    else                lv_label_set_text(g.lbl_sp, "暂停");
}

/* ── 游戏控制 ─────────────────────────────────────────────────────────── */

static void reset_game(void)
{
    memset(g.board, 0, sizeof(g.board));
    make_piece(&g.cur);
    make_piece(&g.next);
    g.score = 0; g.level = 1; g.lines = 0;
    g.drop_interval = 1000;
    g.game_over = false; g.paused = true; g.started = false;
    if(g.timer) lv_timer_set_period(g.timer, g.drop_interval);
    if(g.overlay) lv_obj_add_flag(g.overlay, LV_OBJ_FLAG_HIDDEN);
    refresh_labels();
    update_sp_label();
    draw_board();
    draw_next();
}

static void end_game(void)
{
    g.game_over = true;
    if(g.lbl_final) lv_label_set_text_fmt(g.lbl_final, "得分: %d", g.score);
    if(g.overlay) lv_obj_remove_flag(g.overlay, LV_OBJ_FLAG_HIDDEN);
}

static void start_or_pause(void)
{
    if(g.game_over) { reset_game(); return; }
    if(!g.started) { g.started = true; g.paused = false; }
    else           { g.paused = !g.paused; }
    update_sp_label();
    draw_board();
}

static void drop_cb(lv_timer_t *t)
{
    (void)t;
    if(g.paused || g.game_over || !g.started) return;
    move_cur(0, 1);
}

/* ── 事件回调 ─────────────────────────────────────────────────────────── */

static void ev_left(lv_event_t *e)   { (void)e; move_cur(-1, 0); }
static void ev_right(lv_event_t *e)  { (void)e; move_cur(1, 0); }
static void ev_down(lv_event_t *e)   { (void)e; move_cur(0, 1); }
static void ev_rotate(lv_event_t *e) { (void)e; rotate_cur(); }
static void ev_drop(lv_event_t *e)   { (void)e; hard_drop(); }
static void ev_sp(lv_event_t *e)     { (void)e; start_or_pause(); }
static void ev_restart(lv_event_t *e){ (void)e; reset_game(); }

/* ── UI 构建 ──────────────────────────────────────────────────────────── */

static uint8_t *alloc_canvas_buf(int w, int h)
{
    /* RGB565: 2 字节/像素;留足对齐余量 */
    size_t sz = (size_t)w * h * 2 + (size_t)w * 8 + 256;
    return (uint8_t *)malloc(sz);
}

static lv_obj_t *make_btn(lv_obj_t *parent, const char *text, const lv_font_t *font,
                          uint32_t color, lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_style_radius(btn, 6, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(color), 0);
    lv_obj_set_flex_grow(btn, 1);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, font, 0);
    lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    lv_obj_center(lbl);
    if(cb) lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    return btn;
}

void tetris_start(void)
{
    static bool seeded = false;
    if(!seeded) { srand((unsigned)time(NULL)); seeded = true; }

    memset(&g, 0, sizeof(g));
    g.f_big   = &tetris_font_18;
    g.f_small = &tetris_font_13;

    lv_display_t *disp = lv_display_get_default();
    int W = (int)lv_display_get_horizontal_resolution(disp);
    int H = (int)lv_display_get_vertical_resolution(disp);
    bool small = (W <= 320);
    const lv_font_t *fl = small ? g.f_small : g.f_big;

    int top_h = small ? 64 : 104;
    int btn_h = small ? 92 : 132;
    int avail_h = H - top_h - btn_h - 16;
    int avail_w = W - 16;
    g.block  = LV_MAX(8, LV_MIN(avail_w / T_COLS, avail_h / T_ROWS));
    g.nblock = LV_MAX(6, g.block * 3 / 5);

    /* 根屏:深色容器 */
    g.scr = lv_screen_active();
    lv_obj_clean(g.scr);
    lv_obj_set_style_bg_color(g.scr, lv_color_hex(0x11141f), 0);
    lv_obj_set_style_bg_opa(g.scr, LV_OPA_COVER, 0);
    lv_obj_set_flex_flow(g.scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(g.scr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(g.scr, small ? 4 : 8, 0);
    lv_obj_set_style_pad_row(g.scr, small ? 4 : 8, 0);
    lv_obj_clear_flag(g.scr, LV_OBJ_FLAG_SCROLLABLE);

    /* 顶部信息行:得分/等级/已消除 + 下一个预览 */
    lv_obj_t *top = lv_obj_create(g.scr);
    lv_obj_remove_style_all(top);
    lv_obj_set_width(top, LV_PCT(100));
    lv_obj_set_height(top, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(top, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(top, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *info = lv_obj_create(top);
    lv_obj_remove_style_all(info);
    lv_obj_set_size(info, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(info, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(info, 2, 0);
    lv_obj_clear_flag(info, LV_OBJ_FLAG_SCROLLABLE);
    g.lbl_score = lv_label_create(info);
    g.lbl_level = lv_label_create(info);
    g.lbl_lines = lv_label_create(info);
    lv_obj_t *infos[3] = {g.lbl_score, g.lbl_level, g.lbl_lines};
    for(int i = 0; i < 3; i++) {
        lv_obj_set_style_text_font(infos[i], fl, 0);
        lv_obj_set_style_text_color(infos[i], lv_color_hex(0x2ecc71), 0);
    }

    /* 下一个预览 */
    lv_obj_t *nbox = lv_obj_create(top);
    lv_obj_remove_style_all(nbox);
    lv_obj_set_size(nbox, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(nbox, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(nbox, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(nbox, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *nlbl = lv_label_create(nbox);
    lv_label_set_text(nlbl, "下一个");
    lv_obj_set_style_text_font(nlbl, fl, 0);
    lv_obj_set_style_text_color(nlbl, lv_color_hex(0xe74c3c), 0);
    g.next_buf = alloc_canvas_buf(4 * g.nblock, 4 * g.nblock);
    g.next_canvas = lv_canvas_create(nbox);
    lv_canvas_set_buffer(g.next_canvas, g.next_buf, 4 * g.nblock, 4 * g.nblock, LV_COLOR_FORMAT_RGB565);
    lv_obj_set_style_border_color(g.next_canvas, lv_color_hex(0xe74c3c), 0);
    lv_obj_set_style_border_width(g.next_canvas, 2, 0);

    /* 棋盘(放入容器以承载 game-over 遮罩)*/
    lv_obj_t *bwrap = lv_obj_create(g.scr);
    lv_obj_remove_style_all(bwrap);
    lv_obj_set_size(bwrap, T_COLS * g.block + 6, T_ROWS * g.block + 6);
    lv_obj_set_style_border_color(bwrap, lv_color_hex(0x3498db), 0);
    lv_obj_set_style_border_width(bwrap, 3, 0);
    lv_obj_set_style_radius(bwrap, 4, 0);
    lv_obj_clear_flag(bwrap, LV_OBJ_FLAG_SCROLLABLE);

    g.board_buf = alloc_canvas_buf(T_COLS * g.block, T_ROWS * g.block);
    g.board_canvas = lv_canvas_create(bwrap);
    lv_canvas_set_buffer(g.board_canvas, g.board_buf, T_COLS * g.block, T_ROWS * g.block, LV_COLOR_FORMAT_RGB565);
    lv_obj_center(g.board_canvas);

    /* game-over 遮罩 */
    g.overlay = lv_obj_create(bwrap);
    lv_obj_remove_style_all(g.overlay);
    lv_obj_set_size(g.overlay, LV_PCT(100), LV_PCT(100));
    lv_obj_center(g.overlay);
    lv_obj_set_style_bg_color(g.overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(g.overlay, LV_OPA_80, 0);
    lv_obj_set_flex_flow(g.overlay, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(g.overlay, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(g.overlay, 10, 0);
    lv_obj_clear_flag(g.overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *go = lv_label_create(g.overlay);
    lv_label_set_text(go, "游戏结束");
    lv_obj_set_style_text_font(go, fl, 0);
    lv_obj_set_style_text_color(go, lv_color_hex(0xe74c3c), 0);
    g.lbl_final = lv_label_create(g.overlay);
    lv_obj_set_style_text_font(g.lbl_final, fl, 0);
    lv_obj_set_style_text_color(g.lbl_final, lv_color_white(), 0);
    make_btn(g.overlay, "重新开始", fl, 0xe74c3c, ev_restart);
    lv_obj_add_flag(g.overlay, LV_OBJ_FLAG_HIDDEN);

    /* 控制按钮:方向行 + 功能行 */
    lv_obj_t *row1 = lv_obj_create(g.scr);
    lv_obj_remove_style_all(row1);
    lv_obj_set_width(row1, LV_PCT(100));
    lv_obj_set_height(row1, small ? 40 : 56);
    lv_obj_set_flex_flow(row1, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(row1, small ? 4 : 8, 0);
    lv_obj_clear_flag(row1, LV_OBJ_FLAG_SCROLLABLE);
    /* 方向/旋转用 montserrat 符号字体(中文子集字体不含 FontAwesome 符号)*/
    const lv_font_t *fsym = small ? &lv_font_montserrat_14 : &lv_font_montserrat_16;
    make_btn(row1, LV_SYMBOL_LEFT,    fsym, 0x3498db, ev_left);
    make_btn(row1, LV_SYMBOL_REFRESH, fsym, 0x9b59b6, ev_rotate);
    make_btn(row1, LV_SYMBOL_RIGHT,   fsym, 0x3498db, ev_right);
    make_btn(row1, LV_SYMBOL_DOWN,    fsym, 0x3498db, ev_down);
    make_btn(row1, "落下",            fl,   0x16a085, ev_drop);

    lv_obj_t *row2 = lv_obj_create(g.scr);
    lv_obj_remove_style_all(row2);
    lv_obj_set_width(row2, LV_PCT(100));
    lv_obj_set_height(row2, small ? 40 : 56);
    lv_obj_set_flex_flow(row2, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(row2, small ? 4 : 8, 0);
    lv_obj_clear_flag(row2, LV_OBJ_FLAG_SCROLLABLE);
    g.btn_sp = make_btn(row2, "开始", fl, 0xf39c12, ev_sp);
    g.lbl_sp = lv_obj_get_child(g.btn_sp, 0);
    make_btn(row2, "重新开始", fl, 0xe74c3c, ev_restart);

    /* 计时器(常驻,drop_cb 内判断状态)*/
    g.timer = lv_timer_create(drop_cb, 1000, NULL);

    g.ready = true;
    reset_game();
}

void tetris_set_active(bool active)
{
    if(!g.timer) return;
    if(active) lv_timer_resume(g.timer);
    else       lv_timer_pause(g.timer);
}
