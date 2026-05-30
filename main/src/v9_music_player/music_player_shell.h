#ifndef MUSIC_PLAYER_SHELL_H
#define MUSIC_PLAYER_SHELL_H

#include "music_player_mock_data.h"
#include "music_player_theme.h"
#include "lvgl/lvgl.h"

typedef enum {
    MUSIC_PLAYER_PAGE_HOME = 0,
    MUSIC_PLAYER_PAGE_RADIO,
    MUSIC_PLAYER_PAGE_LOCAL,
    MUSIC_PLAYER_PAGE_PLAYLIST,
    MUSIC_PLAYER_PAGE_SETTINGS,
} music_player_page_t;

typedef struct music_player_app music_player_app_t;

typedef enum {
    MUSIC_PLAYER_SETTINGS_APPEARANCE = 0,
    MUSIC_PLAYER_SETTINGS_PLAYBACK,
    MUSIC_PLAYER_SETTINGS_ABOUT,
} music_player_settings_group_t;

typedef struct {
    music_player_page_t page;
    const char * label;
    const char * glyph;
} music_player_nav_descriptor_t;

typedef struct {
    music_player_page_t id;
    const char * label;
    const char * subtitle;
    const char * badge;
    const music_player_station_t * stations;
    size_t station_count;
    const music_player_recent_item_t * recents;
    size_t recent_count;
    const music_player_playlist_item_t * playlists;
    size_t playlist_count;
} music_player_page_model_t;

struct music_player_app {
    lv_obj_t * screen;
    lv_obj_t * root;
    lv_obj_t * sidebar;
    lv_obj_t * content_shell;
    lv_obj_t * content_host;
    lv_obj_t * mini_player;
    music_player_page_t page;
    music_player_settings_group_t settings_group;
    music_player_theme_state_t theme;
};

void music_player_shell_create(music_player_app_t * app);
void music_player_shell_refresh(music_player_app_t * app);
void music_player_shell_set_page(music_player_app_t * app, music_player_page_t page);
size_t music_player_shell_nav_count(void);
size_t music_player_shell_bottom_nav_index(void);
const music_player_nav_descriptor_t * music_player_shell_nav_descriptor(size_t index);
size_t music_player_shell_settings_group_count(void);
const char * music_player_shell_settings_group_label(music_player_settings_group_t group);

void music_player_home_view_build(lv_obj_t * parent, music_player_app_t * app);
void music_player_radio_view_build(lv_obj_t * parent, music_player_app_t * app);
void music_player_local_view_build(lv_obj_t * parent, music_player_app_t * app);
void music_player_playlist_view_build(lv_obj_t * parent, music_player_app_t * app);
void music_player_settings_view_build(lv_obj_t * parent, music_player_app_t * app);

#endif
