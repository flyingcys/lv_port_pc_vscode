#include "music_player_theme.h"

static const music_player_theme_accent_descriptor_t g_accent_descriptors[] = {
    { MUSIC_PLAYER_ACCENT_CYAN, "Cyan", 0x4BBCAE },
    { MUSIC_PLAYER_ACCENT_BLUE, "Blue", 0x4C7DFF },
    { MUSIC_PLAYER_ACCENT_MINT, "Mint", 0x27B29B },
    { MUSIC_PLAYER_ACCENT_AUTO_1, "Auto 1", 0x7A8CF7 },
};

static uint32_t music_player_theme_builtin_hex(music_player_theme_accent_t accent)
{
    switch(accent) {
        case MUSIC_PLAYER_ACCENT_CYAN:
            return 0x4BBCAE;
        case MUSIC_PLAYER_ACCENT_BLUE:
            return 0x4C7DFF;
        case MUSIC_PLAYER_ACCENT_MINT:
            return 0x27B29B;
        case MUSIC_PLAYER_ACCENT_AUTO_1:
            return 0x7A8CF7;
        default:
            return 0x4BBCAE;
    }
}

static uint32_t music_player_theme_builtin_hero_start_hex(music_player_theme_accent_t accent)
{
    switch(accent) {
        case MUSIC_PLAYER_ACCENT_CYAN:
            return 0xA8EBE0;
        case MUSIC_PLAYER_ACCENT_BLUE:
            return 0x7BA2FF;
        case MUSIC_PLAYER_ACCENT_MINT:
            return 0x79DEC9;
        case MUSIC_PLAYER_ACCENT_AUTO_1:
            return 0xA8B4FF;
        default:
            return 0xA8EBE0;
    }
}

static uint32_t music_player_theme_builtin_hero_end_hex(music_player_theme_accent_t accent)
{
    switch(accent) {
        case MUSIC_PLAYER_ACCENT_CYAN:
            return 0x59C7BA;
        case MUSIC_PLAYER_ACCENT_BLUE:
            return 0x405FDB;
        case MUSIC_PLAYER_ACCENT_MINT:
            return 0x2E9985;
        case MUSIC_PLAYER_ACCENT_AUTO_1:
            return 0x6877D9;
        default:
            return 0x59C7BA;
    }
}

music_player_theme_state_t music_player_theme_default_state(void)
{
    music_player_theme_state_t state;
    state.mode = MUSIC_PLAYER_THEME_LIGHT;
    state.accent = MUSIC_PLAYER_ACCENT_CYAN;
    state.auto_accent_hex = 0x7A8CF7;
    return state;
}

void music_player_theme_set_accent(music_player_theme_state_t * state,
                                   music_player_theme_accent_t accent)
{
    if(state == 0) {
        return;
    }

    state->accent = accent;
}

void music_player_theme_set_auto_accent(music_player_theme_state_t * state, uint32_t accent_hex)
{
    if(state == 0) {
        return;
    }

    state->auto_accent_hex = accent_hex;
}

uint32_t music_player_theme_accent_hex(const music_player_theme_state_t * state)
{
    if(state == 0) {
        return 0x4BBCAE;
    }

    if(state->accent == MUSIC_PLAYER_ACCENT_AUTO_1) {
        return state->auto_accent_hex;
    }

    return music_player_theme_builtin_hex(state->accent);
}

uint32_t music_player_theme_hero_start_hex(const music_player_theme_state_t * state)
{
    if(state == 0) {
        return 0xA8EBE0;
    }

    if(state->accent == MUSIC_PLAYER_ACCENT_AUTO_1 && state->auto_accent_hex == 0x8B9CF7) {
        return 0xB2BFFF;
    }

    return music_player_theme_builtin_hero_start_hex(state->accent);
}

uint32_t music_player_theme_hero_end_hex(const music_player_theme_state_t * state)
{
    if(state == 0) {
        return 0x59C7BA;
    }

    if(state->accent == MUSIC_PLAYER_ACCENT_AUTO_1 && state->auto_accent_hex == 0x8B9CF7) {
        return 0x7483D9;
    }

    return music_player_theme_builtin_hero_end_hex(state->accent);
}

size_t music_player_theme_accent_count(void)
{
    return sizeof(g_accent_descriptors) / sizeof(g_accent_descriptors[0]);
}

const music_player_theme_accent_descriptor_t *
music_player_theme_accent_descriptor(music_player_theme_accent_t accent)
{
    size_t i;

    for(i = 0; i < music_player_theme_accent_count(); ++i) {
        if(g_accent_descriptors[i].accent == accent) {
            return &g_accent_descriptors[i];
        }
    }

    return &g_accent_descriptors[0];
}
