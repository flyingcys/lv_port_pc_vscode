#pragma once

#include "static_include.h"

/**
 * @brief 公共常用APP GUI 开发接口
 */

/**
 * @brief 设置对象的LV_OBJ_FLAG_USERX
 *
 * @param obj 对象
 * @param bcd 0-15
 */
void public_lv_obj_set_bcd(lv_obj_t *obj, uint8_t bcd);

/**
 * @brief 获取对象的LV_OBJ_FLAG_USERX
 *
 * @param obj 对象
 * @return uint8_t 0-15
 */
uint8_t public_lv_obj_get_bcd(lv_obj_t *obj);

/**
 * @brief 创建一行空容器
 *
 * @param parent 父屏对象
 * @return lv_obj_t* 容器
 */
lv_obj_t *public_create_col(lv_obj_t *parent);

/**
 * @brief 创建带文字的一行
 *
 * @param parent 父对象
 * @param fmt 文字
 * @param font 字体
 * @return lv_obj_t* 行对象
 */
lv_obj_t *public_create_col_label(lv_obj_t *parent, const char *fmt, const lv_font_t *font);

/**
 * @brief 创建图标文字
 *
 * @param parent 父对象
 * @param img 图标
 * @param fmt 文本
 * @param font 字体
 * @return lv_obj_t*
 */
lv_obj_t *public_create_col_img_label(lv_obj_t *parent, const void *img, const char *fmt, const lv_font_t *font);

/**
 * @brief 创建隔断线
 *
 * @param parent 父对象
 * @param color 颜色
 * @param width 宽度百分比
 * @return lv_obj_t* 隔断线对象,lv_bar
 */
lv_obj_t *public_create_col_line(lv_obj_t *parent, lv_color_t color, uint8_t width);

/**
 * @brief 创建按钮
 *
 * @param parent 父对象
 * @param fmt 文本
 * @param font 字体
 * @return lv_obj_t* 按钮对象
 */
lv_obj_t *public_create_col_btn(lv_obj_t *parent, const char *fmt, const lv_font_t *font);

/**
 * @brief 创建switch开关
 *
 * @param parent 父对象
 * @param fmt 文本
 * @param font 字体
 * @return lv_obj_t* 开关对象
 */
lv_obj_t *public_create_sw(lv_obj_t *parent, const char *fmt, const lv_font_t *font);

/**
 * @brief 创建下拉列表
 *
 * @param parent 父对象
 * @param fmt 文本
 * @param list 列表
 * @param name_font 行字体
 * @param list_font 列表字体
 * @return lv_obj_t* 下拉对象
 */
lv_obj_t *public_create_dp(lv_obj_t *parent, const char *fmt, const char *list, lv_font_t *name_font, lv_font_t *list_font);

/**
 * @brief 创建菜单
 *
 * @param parent 父对象，一般使用NULL创建顶层屏
 * @return lv_obj_t* 菜单
 */
lv_obj_t *public_create_menu_list(lv_obj_t *parent);