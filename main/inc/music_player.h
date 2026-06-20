/* main/inc/music_player.h */
#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

void   music_player_init(const char **urls, size_t count);
void   music_player_deinit(void);

void   music_player_play(void);
void   music_player_pause(void);
void   music_player_resume(void);
void   music_player_next(void);
void   music_player_prev(void);
void   music_player_select(size_t index);

size_t music_player_get_count(void);
size_t music_player_get_current_index(void);
const char *music_player_get_title(size_t index);
bool   music_player_is_live(size_t index);
bool   music_player_is_playing(void);

void     music_player_seek(uint32_t position_ms);
uint32_t music_player_get_duration_ms(void);
uint32_t music_player_get_track_duration_ms(size_t index);
uint32_t music_player_get_position_ms(void);

#endif /* MUSIC_PLAYER_H */
