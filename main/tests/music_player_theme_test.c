#include <assert.h>

#include "../src/v9_music_player/music_player_theme.h"

static int is_ascii_text(const char * text)
{
    while(*text != '\0') {
        if(((unsigned char)*text) > 0x7fU) {
            return 0;
        }
        text++;
    }

    return 1;
}

int main(void)
{
    music_player_theme_state_t state = music_player_theme_default_state();
    assert(state.mode == MUSIC_PLAYER_THEME_LIGHT);
    assert(state.accent == MUSIC_PLAYER_ACCENT_CYAN);

    music_player_theme_set_accent(&state, MUSIC_PLAYER_ACCENT_MINT);
    assert(music_player_theme_accent_hex(&state) == 0x27B29B);
    assert(music_player_theme_hero_start_hex(&state) == 0x79DEC9);
    assert(music_player_theme_hero_end_hex(&state) == 0x2E9985);

    music_player_theme_set_accent(&state, MUSIC_PLAYER_ACCENT_AUTO_1);
    assert(music_player_theme_accent_hex(&state) == 0x7A8CF7);
    assert(music_player_theme_hero_start_hex(&state) == 0xA8B4FF);
    assert(music_player_theme_hero_end_hex(&state) == 0x6877D9);

    music_player_theme_set_auto_accent(&state, 0x8B9CF7);
    assert(music_player_theme_accent_hex(&state) == 0x8B9CF7);
    assert(music_player_theme_hero_start_hex(&state) == 0xB2BFFF);
    assert(music_player_theme_hero_end_hex(&state) == 0x7483D9);

    assert(music_player_theme_accent_count() == 4);
    assert(music_player_theme_accent_descriptor(MUSIC_PLAYER_ACCENT_CYAN)->accent ==
           MUSIC_PLAYER_ACCENT_CYAN);
    assert(music_player_theme_accent_descriptor(MUSIC_PLAYER_ACCENT_CYAN)->label[0] == 'C');
    assert(music_player_theme_accent_descriptor(MUSIC_PLAYER_ACCENT_BLUE)->accent ==
           MUSIC_PLAYER_ACCENT_BLUE);
    assert(music_player_theme_accent_descriptor(MUSIC_PLAYER_ACCENT_MINT)->accent ==
           MUSIC_PLAYER_ACCENT_MINT);
    assert(music_player_theme_accent_descriptor(MUSIC_PLAYER_ACCENT_AUTO_1)->accent ==
           MUSIC_PLAYER_ACCENT_AUTO_1);
    assert(music_player_theme_accent_descriptor(MUSIC_PLAYER_ACCENT_AUTO_1)->label[0] != '\0');
    assert(is_ascii_text(music_player_theme_accent_descriptor(MUSIC_PLAYER_ACCENT_AUTO_1)->label));
    assert(music_player_theme_accent_descriptor(MUSIC_PLAYER_ACCENT_AUTO_1)->preview_hex ==
           0x7A8CF7);
    return 0;
}
