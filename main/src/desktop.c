/* main/src/desktop.c
 * 应用桌面启动器（移植自 desktop 分支 create_gui，重写为可用版）。
 *
 * 模型：懒构建 + 持久化 + 切屏
 *   - 桌面自身在初始活动屏上构建（保留鼠标光标）。
 *   - 首次点击“音乐/切水果”时，在新建 screen 上构建对应 app 并持久保留；
 *     再次进入只 lv_screen_load 切屏，不重建（避免重复 init / 悬挂 timer）。
 *   - lv_layer_top 上放一个全局“返回桌面”按钮，进入 app 时显示、回桌面时隐藏。
 */
#include "lvgl/lvgl.h"
#include "desktop.h"
#include "v9_apple_music/apple_music.h"
#include "v9-fruit_ninja/fruit_ninja.h"

/* 桌面中文标签字体（SourceHanSansSC 子集，见 desktop_font_18.c）*/
LV_FONT_DECLARE(desktop_font_18);

#define ICON_SIZE   88
#define ICON_GAP    18
#define STATUS_H    32

/* 主题色（与 apple_music 青绿调一致）*/
#define COL_BG      0xeaf4f2
#define COL_CARD    0xffffff
#define COL_ACCENT  0x2bb6a3
#define COL_TEXT    0x303030

static lv_obj_t *s_scr_desktop;   /* 桌面屏（= 初始活动屏）*/
static lv_obj_t *s_scr_music;     /* 音乐 app 屏，懒构建 */
static lv_obj_t *s_scr_fruit;     /* 切水果 app 屏，懒构建 */
static lv_obj_t *s_home_btn;      /* lv_layer_top 上的全局返回按钮 */

/* ── 返回桌面 ─────────────────────────────────────────────────────────── */

static void go_desktop(void)
{
    lv_screen_load(s_scr_desktop);
    if(s_home_btn) lv_obj_add_flag(s_home_btn, LV_OBJ_FLAG_HIDDEN);
}

static void home_btn_cb(lv_event_t *e)
{
    (void)e;
    go_desktop();
}

static void enter_app(void)
{
    if(s_home_btn) lv_obj_remove_flag(s_home_btn, LV_OBJ_FLAG_HIDDEN);
}

/* ── app 启动回调（懒构建一次，之后只切屏）───────────────────────────── */

static void open_music_cb(lv_event_t *e)
{
    (void)e;
    if(!s_scr_music) {
        s_scr_music = lv_obj_create(NULL);
        lv_screen_load(s_scr_music);
        apple_music_create();          /* 在当前活动屏(s_scr_music)上构建 */
    } else {
        lv_screen_load(s_scr_music);
    }
    enter_app();
}

static void open_fruit_cb(lv_event_t *e)
{
    (void)e;
    if(!s_scr_fruit) {
        s_scr_fruit = lv_obj_create(NULL);
        lv_screen_load(s_scr_fruit);
        fruit_ninja_start();           /* 在当前活动屏(s_scr_fruit)上构建 */
    } else {
        lv_screen_load(s_scr_fruit);
    }
    enter_app();
}

/* 占位 app：短暂 toast 提示“开发中” */
static void placeholder_cb(lv_event_t *e)
{
    const char *name = (const char *)lv_event_get_user_data(e);

    lv_obj_t *toast = lv_label_create(lv_layer_top());
    lv_label_set_text_fmt(toast, "%s 开发中", name ? name : "");
    lv_obj_set_style_text_font(toast, &desktop_font_18, 0);
    lv_obj_set_style_bg_opa(toast, LV_OPA_80, 0);
    lv_obj_set_style_bg_color(toast, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_color(toast, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_pad_all(toast, 10, 0);
    lv_obj_set_style_radius(toast, 8, 0);
    lv_obj_align(toast, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_delete_delayed(toast, 1200);   /* 自动消失，无需手动 timer */
}

/* ── 图标 ─────────────────────────────────────────────────────────────── */

static lv_obj_t *make_icon(lv_obj_t *parent, const char *symbol, const char *name,
                            lv_event_cb_t cb, void *user_data)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_size(btn, ICON_SIZE, ICON_SIZE);
    lv_obj_set_style_radius(btn, 18, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(COL_CARD), 0);
    lv_obj_set_style_shadow_width(btn, 12, 0);
    lv_obj_set_style_shadow_opa(btn, LV_OPA_20, 0);
    lv_obj_set_style_pad_all(btn, 6, 0);
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(btn, 6, 0);

    lv_obj_t *ic = lv_label_create(btn);
    lv_label_set_text(ic, symbol);
    lv_obj_set_style_text_font(ic, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(ic, lv_color_hex(COL_ACCENT), 0);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, name);
    lv_obj_set_style_text_font(lbl, &desktop_font_18, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(COL_TEXT), 0);

    if(cb) lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);
    return btn;
}

/* ── 桌面构建 ─────────────────────────────────────────────────────────── */

void desktop_create(void)
{
    s_scr_desktop = lv_screen_active();   /* 初始屏作为桌面（保留鼠标光标）*/
    lv_obj_set_style_bg_color(s_scr_desktop, lv_color_hex(COL_BG), 0);
    lv_obj_set_style_bg_opa(s_scr_desktop, LV_OPA_COVER, 0);
    lv_obj_clear_flag(s_scr_desktop, LV_OBJ_FLAG_SCROLLABLE);

    /* 状态栏 */
    lv_obj_t *status = lv_obj_create(s_scr_desktop);
    lv_obj_remove_style_all(status);
    lv_obj_set_size(status, LV_PCT(100), STATUS_H);
    lv_obj_align(status, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(status, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(status, LV_OPA_60, 0);
    lv_obj_set_style_pad_hor(status, 14, 0);
    lv_obj_clear_flag(status, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *clock = lv_label_create(status);
    lv_label_set_text(clock, "09:41");
    lv_obj_set_style_text_font(clock, &desktop_font_18, 0);
    lv_obj_set_style_text_color(clock, lv_color_hex(COL_TEXT), 0);
    lv_obj_align(clock, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *batt = lv_label_create(status);
    lv_label_set_text(batt, LV_SYMBOL_BATTERY_3 " 76%");
    lv_obj_set_style_text_font(batt, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(batt, lv_color_hex(COL_TEXT), 0);
    lv_obj_align(batt, LV_ALIGN_RIGHT_MID, 0, 0);

    /* 图标网格（flex 自动换行）*/
    lv_obj_t *grid = lv_obj_create(s_scr_desktop);
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, LV_PCT(100), LV_PCT(100));
    lv_obj_align(grid, LV_ALIGN_TOP_MID, 0, STATUS_H);
    lv_obj_set_style_pad_all(grid, ICON_GAP + 6, 0);
    lv_obj_set_style_pad_row(grid, ICON_GAP, 0);
    lv_obj_set_style_pad_column(grid, ICON_GAP, 0);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    /* 两个真实可用 app */
    make_icon(grid, LV_SYMBOL_AUDIO, "音乐",   open_music_cb, NULL);
    make_icon(grid, LV_SYMBOL_CUT,   "切水果", open_fruit_cb, NULL);
    /* 占位图标（点击提示开发中）*/
    make_icon(grid, LV_SYMBOL_DIRECTORY, "有道",   placeholder_cb, "有道");
    make_icon(grid, LV_SYMBOL_LIST,      "网易云", placeholder_cb, "网易云");
    make_icon(grid, LV_SYMBOL_DRIVE,     "百度网盘", placeholder_cb, "百度网盘");
    make_icon(grid, LV_SYMBOL_FILE,      "WPS",    placeholder_cb, "WPS");
    make_icon(grid, LV_SYMBOL_IMAGE,     "相册",   placeholder_cb, "相册");
    make_icon(grid, LV_SYMBOL_SETTINGS,  "设置",   placeholder_cb, "设置");

    /* 全局“返回桌面”按钮：lv_layer_top，进入 app 时显示 */
    s_home_btn = lv_button_create(lv_layer_top());
    lv_obj_set_size(s_home_btn, 44, 44);
    lv_obj_set_style_radius(s_home_btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(s_home_btn, lv_color_hex(COL_ACCENT), 0);
    lv_obj_set_style_bg_opa(s_home_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_home_btn, 2, 0);
    lv_obj_set_style_border_color(s_home_btn, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_shadow_width(s_home_btn, 10, 0);
    lv_obj_set_style_shadow_opa(s_home_btn, LV_OPA_40, 0);
    lv_obj_align(s_home_btn, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_add_event_cb(s_home_btn, home_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *home_ic = lv_label_create(s_home_btn);
    lv_label_set_text(home_ic, LV_SYMBOL_HOME);
    lv_obj_set_style_text_font(home_ic, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(home_ic, lv_color_hex(0xffffff), 0);
    lv_obj_center(home_ic);

    lv_obj_add_flag(s_home_btn, LV_OBJ_FLAG_HIDDEN);   /* 桌面上隐藏 */
}
