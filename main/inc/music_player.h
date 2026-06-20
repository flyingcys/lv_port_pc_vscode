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

/* Play mode: unified 4-state mapping to backend repeat+shuffle.
 * SEQ -> repeat OFF + shuffle off
 * REPEAT_ONE -> repeat ONE + shuffle off
 * REPEAT_ALL -> repeat ALL + shuffle off
 * SHUFFLE -> repeat OFF + shuffle on
 */
typedef enum {
    MP_MODE_SEQ = 0,
    MP_MODE_REPEAT_ONE,
    MP_MODE_REPEAT_ALL,
    MP_MODE_SHUFFLE,
} music_play_mode_t;

void              music_player_set_play_mode(music_play_mode_t mode);
music_play_mode_t music_player_get_play_mode(void);
music_play_mode_t music_player_cycle_play_mode(void); /* SEQ->ONE->ALL->SHUFFLE->SEQ */

/* Volume: percent 0~100. Mute is a separate flag that overrides volume to 0
 * without losing the stored volume. */
void    music_player_set_volume(uint8_t percent);
uint8_t music_player_get_volume(void);
void    music_player_mute_toggle(void);
bool    music_player_is_muted(void);

#endif /* MUSIC_PLAYER_H */
