/* main/src/music_player.c */
#include "music_player.h"

void   music_player_init(const char **urls, size_t count) { (void)urls; (void)count; }
void   music_player_deinit(void) {}
void   music_player_play(void) {}
void   music_player_pause(void) {}
void   music_player_resume(void) {}
void   music_player_next(void) {}
void   music_player_prev(void) {}
void   music_player_select(size_t index) { (void)index; }
size_t music_player_get_count(void) { return 0; }
size_t music_player_get_current_index(void) { return 0; }
bool   music_player_is_playing(void) { return false; }
