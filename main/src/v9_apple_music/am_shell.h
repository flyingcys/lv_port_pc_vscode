/* main/src/v9_apple_music/am_shell.h */
#ifndef AM_SHELL_H
#define AM_SHELL_H
#include "lvgl/lvgl.h"

/* 导航点击回调:传出被点项索引(0..4 对应 am_nav_items) */
typedef void (*am_nav_cb_t)(int nav_index, void *user);

/* 在 sidebar 容器内构建侧栏(品牌/导航/偏好/listener 卡);active_nav 高亮项;cb 用于切页 */
void am_shell_build_sidebar(lv_obj_t *sidebar, int active_nav, am_nav_cb_t cb, void *user);

/* 在 player 容器内构建迷你播放条 */
void am_shell_build_miniplayer(lv_obj_t *player);

#endif /* AM_SHELL_H */
