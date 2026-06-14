/* main/src/v9_apple_music/am_page_list.h
 * 共用列表页模板 —— 广播/本地/歌单三页复用同一布局，内容来自 am_list_page_t。
 * 对应 HTML renderListPage()，设计规格见 §6.3。
 */
#ifndef AM_PAGE_LIST_H
#define AM_PAGE_LIST_H
#include "lvgl/lvgl.h"
#include "am_data.h"

/**
 * 在 content_parent 内创建列表页。
 * page 自身高度为 LV_SIZE_CONTENT，由调用方负责滚动。
 * @param content_parent  可滚动的父容器（am_shell 内容区）
 * @param data            页面数据（&am_page_radio / &am_page_local / &am_page_playlist）
 * @return                page 根对象
 */
lv_obj_t *am_page_list_create(lv_obj_t *content_parent, const am_list_page_t *data);

#endif /* AM_PAGE_LIST_H */
