#ifndef MUSIC_PLAYER_THEME_H
#define MUSIC_PLAYER_THEME_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    MUSIC_PLAYER_THEME_LIGHT = 0,
    MUSIC_PLAYER_THEME_DARK,
} music_player_theme_mode_t;

typedef enum {
    MUSIC_PLAYER_ACCENT_CYAN = 0,
    MUSIC_PLAYER_ACCENT_BLUE,
    MUSIC_PLAYER_ACCENT_MINT,
    MUSIC_PLAYER_ACCENT_AUTO_1,
} music_player_theme_accent_t;

typedef struct {
    music_player_theme_mode_t mode;
    music_player_theme_accent_t accent;
    uint32_t auto_accent_hex;
} music_player_theme_state_t;

typedef struct {
    music_player_theme_accent_t accent;
    const char * label;
    uint32_t preview_hex;
} music_player_theme_accent_descriptor_t;

music_player_theme_state_t music_player_theme_default_state(void);
void music_player_theme_set_accent(music_player_theme_state_t * state,
                                   music_player_theme_accent_t accent);
void music_player_theme_set_auto_accent(music_player_theme_state_t * state, uint32_t accent_hex);
uint32_t music_player_theme_accent_hex(const music_player_theme_state_t * state);
uint32_t music_player_theme_hero_start_hex(const music_player_theme_state_t * state);
uint32_t music_player_theme_hero_end_hex(const music_player_theme_state_t * state);
size_t music_player_theme_accent_count(void);
const music_player_theme_accent_descriptor_t *
music_player_theme_accent_descriptor(music_player_theme_accent_t accent);

#endif
