/* main/src/v9_apple_music/am_player.h */
#ifndef AM_PLAYER_H
#define AM_PLAYER_H

#include "lvgl/lvgl.h"
#include <stddef.h>

/* mini-player 关键 widget 句柄 */
typedef struct {
    lv_obj_t *title_label;
    lv_obj_t *subtitle_label;
    lv_obj_t *time_cur;
    lv_obj_t *time_total;
    lv_obj_t *progress_fill;
    lv_obj_t *play_icon;
} am_miniplayer_handles_t;

void am_player_init(void);
void am_player_deinit(void);

/* mini-player 重建后立即调用（主题切换时也需调用）*/
void am_player_bind_miniplayer(const am_miniplayer_handles_t *h);

/* 本地页：替换整个播放列表并从 start_index 播放 */
void am_player_load_local(const char **urls, size_t count, size_t start_index);

/* 广播页：直接切换 HLS 流（不影响本地列表）*/
void am_player_play_stream(const char *url, const char *title);

/* 控制按钮 LVGL 事件回调 */
void am_player_on_play_pause(lv_event_t *e);
void am_player_on_prev(lv_event_t *e);
void am_player_on_next(lv_event_t *e);

#endif /* AM_PLAYER_H */
