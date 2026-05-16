#ifndef FRUIT_NINJA_AUDIO_H
#define FRUIT_NINJA_AUDIO_H

#include <stdbool.h>

bool fruit_ninja_audio_init(void);
void fruit_ninja_audio_shutdown(void);
void fruit_ninja_audio_play_menu_music(void);
void fruit_ninja_audio_play_start(void);
void fruit_ninja_audio_play_throw(void);
void fruit_ninja_audio_play_slice(void);
void fruit_ninja_audio_play_boom(void);
void fruit_ninja_audio_play_game_over(void);
void fruit_ninja_audio_stop_music(void);

#endif
