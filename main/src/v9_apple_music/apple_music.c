/* main/src/v9_apple_music/apple_music.c */
#include "apple_music.h"
#include "am_shell.h"
#include "lvgl/lvgl.h"

static void on_nav(int idx, void *u){ LV_UNUSED(u); LV_LOG_USER("nav %d", idx); }

#define AM_SIDEBAR_W   164
#define AM_PLAYER_H    78

/* 根 grid:列[164, FR1] 行[FR1, 78],侧栏跨两行 */
static const int32_t col_dsc[] = {AM_SIDEBAR_W, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static const int32_t row_dsc[] = {LV_GRID_FR(1), AM_PLAYER_H, LV_GRID_TEMPLATE_LAST};

void apple_music_create(void)
{
    lv_obj_t *root = lv_screen_active();
    lv_obj_remove_style_all(root);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF); /* 全屏壳,禁滚动条避免亚像素溢出闪烁 */
    lv_obj_set_style_bg_color(root, lv_color_hex(0xedf8f4), 0); /* 全屏背景底色(侧栏 OPA_80 叠加其上) */
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_grid_dsc_array(root, col_dsc, row_dsc);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_pad_gap(root, 0, 0);

    /* 侧栏:col0 row0 跨两行 */
    lv_obj_t *sidebar = lv_obj_create(root);
    lv_obj_remove_style_all(sidebar);
    lv_obj_set_grid_cell(sidebar, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
    am_shell_build_sidebar(sidebar, 0, on_nav, NULL);

    /* 内容区占位:col1 row0 */
    lv_obj_t *content = lv_obj_create(root);
    lv_obj_remove_style_all(content);
    lv_obj_set_style_bg_color(content, lv_color_hex(0xf6f7fa), 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_COVER, 0);
    lv_obj_set_grid_cell(content, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    /* 迷你播放条:col1 row1 */
    lv_obj_t *player = lv_obj_create(root);
    lv_obj_remove_style_all(player);
    lv_obj_set_grid_cell(player, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    am_shell_build_miniplayer(player);
}
