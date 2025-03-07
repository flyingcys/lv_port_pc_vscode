#ifndef desktop_app_H
#define desktop_app_H

#include "static_include.h"

// 桌面壁纸声明
//LV_IMG_DECLARE(desktop_bg_01);
// LV_IMG_DECLARE(desktop_bg_02);
// LV_IMG_DECLARE(desktop_bg_03);

#define desktop_bg_default 0    // SFIFFS_PATH "yuan.bmp" // 默认桌面壁纸 0不启用
#define control_center_gestrue LV_DIR_BOTTOM // 呼出控制中心手势
#define control_center_area_width 30         // 呼出控制中心宽度
#define desktop_load_move_time 200           // 加载桌面所需移动动画时长
#define desktop_load_fade_time 500           // 加载桌面所需渐变动画时长

typedef struct
{
    const char *name;       // 界面名称
    lv_obj_t *(*ui_load)(); // 初始化界面并返回表盘LVGL screen对象
    void (*ui_close)();     // 关闭（禁用)界面
} desktop_ui_t;             // 界面初始化结构体

// 初始化控制中心回调函数格式
typedef lv_obj_t *(*control_center_load_cb_t)();

/**
 * @brief 初始化桌面APP
 */
void desktop_app_install();

/**
 * 添加界面风格
 */
void desktop_add_ui(desktop_ui_t *ui);

/**
 * @brief 注册控制中心
 *
 * @param cb
 */
void desktop_register_control_center(control_center_load_cb_t cb);

/**
 * @brief APP加载界面
 *
 * @param scr 界面对象
 * @param anim_type 动画类型
 * @param time 过渡时长ms
 * @param delay 等待时长ms
 */
void app_scr_load(lv_obj_t *scr, lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay);

//////////////////////////////////////////////////////////
void desktop_ui_list();

#endif
