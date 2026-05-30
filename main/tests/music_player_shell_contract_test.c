#include <assert.h>
#include <string.h>

#include "../src/v9_music_player/music_player_shell.h"

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
    assert(music_player_shell_nav_count() == 5);
    assert(music_player_shell_bottom_nav_index() == 4);

    const music_player_nav_descriptor_t * settings =
        music_player_shell_nav_descriptor(music_player_shell_bottom_nav_index());
    assert(settings != 0);
    assert(settings->page == MUSIC_PLAYER_PAGE_SETTINGS);
    assert(strcmp(settings->label, "Settings") == 0);

    for(size_t i = 0; i < music_player_shell_nav_count(); ++i) {
        const music_player_nav_descriptor_t * nav = music_player_shell_nav_descriptor(i);
        assert(nav != 0);
        assert(is_ascii_text(nav->label));
        assert(is_ascii_text(nav->glyph));
    }

    assert(music_player_shell_settings_group_count() == 3);
    assert(strcmp(music_player_shell_settings_group_label(MUSIC_PLAYER_SETTINGS_APPEARANCE),
                  "Appearance") == 0);
    assert(strcmp(music_player_shell_settings_group_label(MUSIC_PLAYER_SETTINGS_PLAYBACK),
                  "Playback") == 0);
    assert(strcmp(music_player_shell_settings_group_label(MUSIC_PLAYER_SETTINGS_ABOUT),
                  "About") == 0);

    for(size_t i = 0; i < music_player_shell_settings_group_count(); ++i) {
        assert(is_ascii_text(music_player_shell_settings_group_label((music_player_settings_group_t)i)));
    }

    return 0;
}
