/* main/src/v9_apple_music/am_data.h */
#ifndef AM_DATA_H
#define AM_DATA_H
#include <stddef.h>

typedef struct { const char *id; const char *label; const char *icon; } am_nav_item_t;
typedef struct { const char *id; const char *label; const char *desc; } am_theme_preset_t;
typedef struct { const char *id; const char *title; const char *desc; } am_settings_tab_t;

typedef struct { const char *title; const char *subtitle; } am_simple_item_t;     /* recommends */
typedef struct { const char *kind; const char *title; const char *subtitle; } am_recent_item_t;
typedef struct { const char *label; const char *value; } am_metric_t;
typedef struct {
    const char *topline; const char *title; const char *desc; const char *badge;
} am_hero_t;

typedef struct {
    const char *kind;    /* album/radio/playlist/song -> 封面渐变色 */
    const char *title; const char *subtitle; const char *meta; const char *aux;
    const char *url;     /* 播放地址：本地文件路径或流 URL；NULL = 不可点击播放 */
} am_media_item_t;
typedef struct { const char *title; const char *subtitle; } am_queue_item_t;

/* 列表项点击播放语义 */
typedef enum {
    AM_LIST_PLAY_NONE = 0,   /* 条目仅展示，不可点击播放（如歌单页）*/
    AM_LIST_PLAY_LOCAL,      /* 点击 -> am_player_load_local（整列表 + 起始索引）*/
    AM_LIST_PLAY_STREAM,     /* 点击 -> am_player_play_stream（单条流）*/
} am_list_play_mode_t;

typedef struct {
    const char *eyebrow, *title, *subtitle;
    const char *banner_title, *banner_desc;
    const char *badges[3];
    const char *queue_label;
    am_media_item_t list[4];
    am_queue_item_t queue[3];
    am_list_play_mode_t play_mode;   /* 列表项点击播放语义，默认 NONE */
} am_list_page_t;

typedef struct { const char *title, *subtitle, *current, *total; int progress_pct; } am_mini_player_t;

extern const am_nav_item_t      am_nav_items[5];
extern const am_theme_preset_t  am_theme_presets[4];
extern const am_settings_tab_t  am_settings_tabs[3];
extern const am_hero_t          am_home_hero;
extern const am_simple_item_t   am_home_recommends[3];
extern const am_recent_item_t   am_home_recent[3];
extern const am_metric_t        am_home_metrics[3];
extern const am_list_page_t     am_page_radio;
extern const am_list_page_t     am_page_local;
extern const am_list_page_t     am_page_playlist;
extern const am_mini_player_t   am_mini;

#endif /* AM_DATA_H */
