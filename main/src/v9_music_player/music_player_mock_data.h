#ifndef MUSIC_PLAYER_MOCK_DATA_H
#define MUSIC_PLAYER_MOCK_DATA_H

#include <stddef.h>

typedef struct {
    const char * title;
    const char * subtitle;
} music_player_station_t;

typedef struct {
    const char * title;
    const char * subtitle;
} music_player_recent_item_t;

typedef struct {
    const char * title;
    const char * subtitle;
} music_player_playlist_item_t;

size_t music_player_radio_count(void);
size_t music_player_recent_count(void);
size_t music_player_playlist_count(void);

const music_player_station_t * music_player_radio_items(void);
const music_player_recent_item_t * music_player_recent_items(void);
const music_player_playlist_item_t * music_player_playlist_items(void);

#endif
