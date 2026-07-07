#ifndef AM_PLAYER_H
#define AM_PLAYER_H

#include "lvgl/lvgl.h"

#include "am_data.h"

typedef struct {
    lv_obj_t *title_label;
    lv_obj_t *subtitle_label;
    lv_obj_t *time_cur;
    lv_obj_t *time_total;
    lv_obj_t *progress_track;
    lv_obj_t *progress_fill;
    lv_obj_t *knob;
    lv_obj_t *play_icon;
    lv_obj_t *pause_icon;
    lv_obj_t *volume_icon;
    lv_obj_t *volume_track;
    lv_obj_t *volume_fill;
    lv_obj_t *playlist_popup;
    lv_obj_t *playlist_list;
    lv_obj_t *playlist_scroll_track;
    lv_obj_t *playlist_scroll_thumb;
    lv_obj_t *btn_mode;
    lv_obj_t *btn_prev;
    lv_obj_t *btn_play;
    lv_obj_t *btn_next;
    lv_obj_t *btn_playlist;
} am_miniplayer_handles_t;

void am_player_init(void);
void am_player_deinit(void);

void am_player_bind_miniplayer(const am_miniplayer_handles_t *h);
void am_player_set_sources(const am_local_item_t *locals, size_t local_count,
                           const am_radio_item_t *radios, size_t radio_count);

void am_player_play_local_index(size_t index);
void am_player_play_radio_index(size_t index);
void am_player_play_single_file(const char *path, const char *title);
void am_player_toggle_playback(void);
void am_player_prev(void);
void am_player_next(void);
void am_player_cycle_mode(void);
void am_player_set_volume_percent(uint8_t percent);
void am_player_seek_percent(uint8_t percent);
void am_player_toggle_mute(void);
void am_player_set_playlist_open(bool open);
bool am_player_playlist_open(void);

/* 弹层(播放列表)点击某项切歌后回调宿主(用于跳转/刷新"正在播放"页)。 */
typedef void (*am_player_pick_cb_t)(void);
void am_player_set_playlist_pick_cb(am_player_pick_cb_t cb);

am_source_kind_t am_player_source_kind(void);
uint32_t am_player_track_duration_ms(size_t index);
size_t am_player_current_local_index(void);
size_t am_player_current_radio_index(void);
bool am_player_is_playing(void);
bool am_player_is_single_file_mode(void);
uint8_t am_player_volume_percent(void);
void am_player_clear_single_file_mode(void);

void am_player_refresh_ui(void);

void am_player_on_play_pause(lv_event_t *e);
void am_player_on_prev(lv_event_t *e);
void am_player_on_next(lv_event_t *e);
void am_player_on_mode(lv_event_t *e);

#endif /* AM_PLAYER_H */
