#include "fruit_ninja_audio.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "fruit_ninja_assets.h"
#include "lvgl/lvgl.h"
#include "player_controller.h"
#include "stream_player.h"

typedef enum {
    FRUIT_NINJA_AUDIO_MENU = 0,
    FRUIT_NINJA_AUDIO_START,
    FRUIT_NINJA_AUDIO_THROW,
    FRUIT_NINJA_AUDIO_SLICE,
    FRUIT_NINJA_AUDIO_BOOM,
    FRUIT_NINJA_AUDIO_GAME_OVER,
    FRUIT_NINJA_AUDIO_COUNT,
} fruit_ninja_audio_track_t;

typedef struct {
    bool initialized;
    player_controller_t * controller;
    size_t current_track;
} fruit_ninja_audio_state_t;

static const char * const g_audio_relative_paths[FRUIT_NINJA_AUDIO_COUNT] = {
    "sound/menu.mp3",
    "sound/start.mp3",
    "sound/throw.mp3",
    "sound/splatter.mp3",
    "sound/boom.mp3",
    "sound/over.mp3",
};

static fruit_ninja_audio_state_t g_audio = {
    .current_track = FRUIT_NINJA_AUDIO_COUNT,
};

static char g_audio_paths[FRUIT_NINJA_AUDIO_COUNT][512];
static const char * g_audio_urls[FRUIT_NINJA_AUDIO_COUNT];

static bool build_audio_playlist(void)
{
    for(size_t i = 0; i < FRUIT_NINJA_AUDIO_COUNT; ++i) {
        if(!fruit_ninja_assets_build_audio_path(g_audio_paths[i],
                                                sizeof(g_audio_paths[i]),
                                                g_audio_relative_paths[i])) {
            LV_LOG_WARN("failed to build Fruit Ninja audio path: %s", g_audio_relative_paths[i]);
            return false;
        }
        g_audio_urls[i] = g_audio_paths[i];
    }

    return true;
}

static void play_track(fruit_ninja_audio_track_t track, player_repeat_mode_t repeat_mode)
{
    if(!g_audio.initialized || g_audio.controller == NULL) return;

    if(track == FRUIT_NINJA_AUDIO_MENU &&
       g_audio.current_track == (size_t)track &&
       player_controller_get_state(g_audio.controller) == PLAYER_CONTROLLER_STATE_PLAYING) {
        return;
    }

    player_controller_set_repeat_mode(g_audio.controller, repeat_mode);
    (void)player_controller_stop(g_audio.controller);
    if(player_controller_select(g_audio.controller, (size_t)track) != 0) {
        LV_LOG_WARN("failed to select Fruit Ninja audio track: %u", (unsigned)track);
        return;
    }
    if(player_controller_play(g_audio.controller) != 0) {
        LV_LOG_WARN("failed to play Fruit Ninja audio track: %u", (unsigned)track);
        return;
    }

    g_audio.current_track = (size_t)track;
}

bool fruit_ninja_audio_init(void)
{
    stream_player_config_t config;

    if(g_audio.initialized) return true;

    if(!build_audio_playlist()) return false;

    stream_player_reset_interrupt_state();
    g_audio.controller = player_controller_create(NULL, NULL);
    if(g_audio.controller == NULL) {
        LV_LOG_WARN("failed to create Fruit Ninja audio controller");
        return false;
    }

    stream_player_get_default_config(&config);
    stream_player_apply_profile(&config, STREAM_PROFILE_BALANCED);
    (void)strncpy(config.url, g_audio_urls[FRUIT_NINJA_AUDIO_MENU], sizeof(config.url) - 1U);
    config.url[sizeof(config.url) - 1U] = '\0';

    if(player_controller_set_stream_config(g_audio.controller, &config) != 0 ||
       player_controller_load_urls(g_audio.controller, g_audio_urls, FRUIT_NINJA_AUDIO_COUNT) != 0) {
        LV_LOG_WARN("failed to load Fruit Ninja audio playlist");
        player_controller_destroy(g_audio.controller);
        g_audio.controller = NULL;
        return false;
    }

    player_controller_set_repeat_mode(g_audio.controller, PLAYER_REPEAT_OFF);
    g_audio.current_track = FRUIT_NINJA_AUDIO_COUNT;
    g_audio.initialized = true;
    return true;
}

void fruit_ninja_audio_poll(void)
{
    if(g_audio.initialized && g_audio.controller != NULL) {
        player_controller_poll(g_audio.controller);
    }
}

void fruit_ninja_audio_shutdown(void)
{
    if(!g_audio.initialized) return;

    if(g_audio.controller != NULL) {
        (void)player_controller_stop(g_audio.controller);
        player_controller_destroy(g_audio.controller);
    }
    memset(&g_audio, 0, sizeof(g_audio));
    g_audio.current_track = FRUIT_NINJA_AUDIO_COUNT;
}

void fruit_ninja_audio_play_menu_music(void)
{
    play_track(FRUIT_NINJA_AUDIO_MENU, PLAYER_REPEAT_ONE);
}

void fruit_ninja_audio_play_start(void)
{
    play_track(FRUIT_NINJA_AUDIO_START, PLAYER_REPEAT_OFF);
}

void fruit_ninja_audio_play_throw(void)
{
    play_track(FRUIT_NINJA_AUDIO_THROW, PLAYER_REPEAT_OFF);
}

void fruit_ninja_audio_play_slice(void)
{
    play_track(FRUIT_NINJA_AUDIO_SLICE, PLAYER_REPEAT_OFF);
}

void fruit_ninja_audio_play_boom(void)
{
    play_track(FRUIT_NINJA_AUDIO_BOOM, PLAYER_REPEAT_OFF);
}

void fruit_ninja_audio_play_game_over(void)
{
    play_track(FRUIT_NINJA_AUDIO_GAME_OVER, PLAYER_REPEAT_OFF);
}

void fruit_ninja_audio_stop_music(void)
{
    if(g_audio.initialized && g_audio.controller != NULL) {
        (void)player_controller_stop(g_audio.controller);
        g_audio.current_track = FRUIT_NINJA_AUDIO_COUNT;
    }
}
