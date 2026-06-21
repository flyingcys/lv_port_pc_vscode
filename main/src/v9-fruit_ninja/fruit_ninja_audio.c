#include "fruit_ninja_audio.h"

#include <stdint.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "fruit_ninja_assets.h"
#include "lvgl/lvgl.h"

#define FRUIT_NINJA_AUDIO_FLAGS (MIX_INIT_OGG | MIX_INIT_MP3)

typedef struct {
    bool initialized;
    Mix_Music * menu_music;
    Mix_Chunk * start_sound;
    Mix_Chunk * throw_sound;
    Mix_Chunk * slice_sound;
    Mix_Chunk * boom_sound;
    Mix_Chunk * game_over_sound;
} fruit_ninja_audio_state_t;

static fruit_ninja_audio_state_t g_audio;

static bool load_music(Mix_Music ** music, const char * ogg_rel_path, const char * mp3_rel_path)
{
    char path[512];

    fruit_ninja_assets_build_audio_path(path, sizeof(path), ogg_rel_path);
    if(SDL_RWFromFile(path, "rb") != NULL) {
        *music = Mix_LoadMUS(path);
        if(*music != NULL) return true;
    }

    fruit_ninja_assets_build_audio_path(path, sizeof(path), mp3_rel_path);
    *music = Mix_LoadMUS(path);
    return *music != NULL;
}

static bool load_chunk(Mix_Chunk ** chunk, const char * ogg_rel_path, const char * mp3_rel_path)
{
    char path[512];

    fruit_ninja_assets_build_audio_path(path, sizeof(path), ogg_rel_path);
    *chunk = Mix_LoadWAV(path);
    if(*chunk != NULL) return true;

    fruit_ninja_assets_build_audio_path(path, sizeof(path), mp3_rel_path);
    *chunk = Mix_LoadWAV(path);
    return *chunk != NULL;
}

bool fruit_ninja_audio_init(void)
{
    if(g_audio.initialized) return true;

    if(Mix_Init(FRUIT_NINJA_AUDIO_FLAGS) == 0) {
        LV_LOG_WARN("SDL_mixer codec init failed: %s", Mix_GetError());
    }

    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        LV_LOG_ERROR("Mix_OpenAudio failed: %s", Mix_GetError());
        return false;
    }

    Mix_AllocateChannels(8);
    memset(&g_audio, 0, sizeof(g_audio));
    g_audio.initialized = true;

    if(!load_music(&g_audio.menu_music, "sound/menu.ogg", "sound/menu.mp3")) {
        LV_LOG_WARN("failed to load menu music: %s", Mix_GetError());
    }
    if(!load_chunk(&g_audio.start_sound, "sound/start.ogg", "sound/start.mp3")) {
        LV_LOG_WARN("failed to load start sound: %s", Mix_GetError());
    }
    if(!load_chunk(&g_audio.throw_sound, "sound/throw.ogg", "sound/throw.mp3")) {
        LV_LOG_WARN("failed to load throw sound: %s", Mix_GetError());
    }
    if(!load_chunk(&g_audio.slice_sound, "sound/splatter.ogg", "sound/splatter.mp3")) {
        LV_LOG_WARN("failed to load slice sound: %s", Mix_GetError());
    }
    if(!load_chunk(&g_audio.boom_sound, "sound/boom.ogg", "sound/boom.mp3")) {
        LV_LOG_WARN("failed to load boom sound: %s", Mix_GetError());
    }
    if(!load_chunk(&g_audio.game_over_sound, "sound/over.ogg", "sound/over.mp3")) {
        LV_LOG_WARN("failed to load game-over sound: %s", Mix_GetError());
    }

    return true;
}

void fruit_ninja_audio_shutdown(void)
{
    if(!g_audio.initialized) return;

    Mix_HaltMusic();
    Mix_HaltChannel(-1);
    Mix_FreeMusic(g_audio.menu_music);
    Mix_FreeChunk(g_audio.start_sound);
    Mix_FreeChunk(g_audio.throw_sound);
    Mix_FreeChunk(g_audio.slice_sound);
    Mix_FreeChunk(g_audio.boom_sound);
    Mix_FreeChunk(g_audio.game_over_sound);
    memset(&g_audio, 0, sizeof(g_audio));
    Mix_CloseAudio();
    Mix_Quit();
}

void fruit_ninja_audio_play_menu_music(void)
{
    if(g_audio.initialized && g_audio.menu_music != NULL && Mix_PlayingMusic() == 0) {
        Mix_PlayMusic(g_audio.menu_music, -1);
    }
}

void fruit_ninja_audio_play_start(void)
{
    if(g_audio.initialized && g_audio.start_sound != NULL) {
        Mix_PlayChannel(-1, g_audio.start_sound, 0);
    }
}

void fruit_ninja_audio_play_throw(void)
{
    if(g_audio.initialized && g_audio.throw_sound != NULL) {
        Mix_PlayChannel(-1, g_audio.throw_sound, 0);
    }
}

void fruit_ninja_audio_play_slice(void)
{
    if(g_audio.initialized && g_audio.slice_sound != NULL) {
        Mix_PlayChannel(-1, g_audio.slice_sound, 0);
    }
}

void fruit_ninja_audio_play_boom(void)
{
    if(g_audio.initialized && g_audio.boom_sound != NULL) {
        Mix_PlayChannel(-1, g_audio.boom_sound, 0);
    }
}

void fruit_ninja_audio_play_game_over(void)
{
    if(g_audio.initialized && g_audio.game_over_sound != NULL) {
        Mix_PlayChannel(-1, g_audio.game_over_sound, 0);
    }
}

void fruit_ninja_audio_stop_music(void)
{
    if(g_audio.initialized) {
        Mix_HaltMusic();
    }
}
