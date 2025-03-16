#include "lvgl.h"

#define GRID_COLUMN_NUM 4
#define ICON_SIZE 60
#define ICON_SPACING 20

/* 状态栏样式 */
static lv_style_t status_bar_style;
/* 图标按钮样式 */
static lv_style_t icon_style;

/* 状态栏控件 */
lv_obj_t* status_bar;
lv_obj_t* time_label;
lv_obj_t* battery_label;
lv_obj_t* signal_icon;

/* 生成示例图标 */
void create_app_icon(lv_obj_t* parent, const char* name, const char* symbol)
{
    // 创建按钮容器
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, ICON_SIZE, ICON_SIZE);
    lv_obj_add_style(btn, &icon_style, 0);

    // 添加图标
    lv_obj_t* icon = lv_label_create(btn);
    lv_label_set_text(icon, symbol);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_24, 0);
    lv_obj_center(icon);

    // 添加文字标签
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, name);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -5);
}

void create_gui(void)
{
    // 初始化样式
    lv_style_init(&status_bar_style);
    lv_style_set_bg_color(&status_bar_style, lv_color_hex(0xf0f0f0));
    lv_style_set_pad_all(&status_bar_style, 5);
    
    lv_style_init(&icon_style);
    lv_style_set_radius(&icon_style, 8);
    lv_style_set_bg_color(&icon_style, lv_color_hex(0xffffff));
    lv_style_set_shadow_width(&icon_style, 10);
    lv_style_set_shadow_spread(&icon_style, 5);

    /* 创建状态栏 */
    status_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(status_bar, LV_HOR_RES, 30);
    lv_obj_add_style(status_bar, &status_bar_style, 0);
    
    // 时间显示
    time_label = lv_label_create(status_bar);
    lv_label_set_text(time_label, "00:22");
    lv_obj_align(time_label, LV_ALIGN_LEFT_MID, 10, 0);
    
    // 电量显示
    battery_label = lv_label_create(status_bar);
    lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_3 "76%");
    lv_obj_align(battery_label, LV_ALIGN_RIGHT_MID, -10, 0);

    /* 创建主图标容器 */
    lv_obj_t* main_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_cont, LV_HOR_RES, LV_VER_RES - 60);
    lv_obj_align(main_cont, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_all(main_cont, ICON_SPACING, 0);

    /* 生成示例图标（按图片内容排列） */
    create_app_icon(main_cont, "有道", LV_SYMBOL_DIRECTORY);
    create_app_icon(main_cont, "交管12123", LV_SYMBOL_WARNING);
    create_app_icon(main_cont, "网易云", LV_SYMBOL_AUDIO);
    // create_app_icon(main_cont, "反诈中心", LV_SYMBOL_SHIELD);
    create_app_icon(main_cont, "百度网盘", LV_SYMBOL_DRIVE);
    create_app_icon(main_cont, "WPS", LV_SYMBOL_FILE);
    // 继续添加其他图标...

    /* 创建底部导航栏 */
    lv_obj_t* nav_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(nav_bar, LV_HOR_RES, 50);
    lv_obj_align(nav_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(nav_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(nav_bar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
}