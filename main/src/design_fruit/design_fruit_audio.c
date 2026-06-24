#define _DEFAULT_SOURCE

#include "design_fruit_audio.h"

#include "design_fruit_assets.h"

#include "lvgl/lvgl.h"
#include "player_controller.h"
#include "stream_player.h"

#include <string.h>

/*
 * 背景音乐复用 third-party/hls_player_demo 的 player_controller（与 main/src/local_music_demo
 * 的 music_player 同一套基础设施）：单曲 + PLAYER_REPEAT_ONE 实现循环播放，lv_timer 周期
 * 轮询驱动其状态推进。
 *
 * 注意：hls 的音频输出后端（audio_output_alsa / audio_output_coreaudio）是**进程级单例**，
 * 同一时刻只能播放一路音频，无法把背景音乐与点击/消除音效混音。因此本模块只承载背景音乐；
 * play_hit / play_over 在此后端下保持为安全的 no-op（保留接口，避免与上层耦合）。
 */

#define DESIGN_FRUIT_BGM_RELATIVE "mp3/bgm.mp3"
#define DESIGN_FRUIT_BGM_POLL_MS 100
#define DESIGN_FRUIT_BGM_VOLUME 0.6f

static bool g_ready = false;
static bool g_bgm_enabled = true;
static player_controller_t *g_bgm_ctrl = NULL;
static lv_timer_t *g_poll_timer = NULL;
static char g_bgm_url[512];

static void bgm_event_cb(player_controller_t *controller,
                         player_controller_event_t event,
                         const void *event_data,
                         void *user_data)
{
    (void)controller;
    (void)event;
    (void)event_data;
    (void)user_data;
    /* 循环播放由 PLAYER_REPEAT_ONE + 轮询在控制器内部处理，无需在此响应事件。*/
}

static void poll_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    if(g_bgm_ctrl != NULL) player_controller_poll(g_bgm_ctrl);
}

bool design_fruit_audio_init(void)
{
    stream_player_config_t config;
    const char *urls[1];

    if(g_ready) return true; /* 幂等 */

    (void)design_fruit_assets_init(NULL);
    if(!design_fruit_assets_build_audio_path(g_bgm_url, sizeof(g_bgm_url), DESIGN_FRUIT_BGM_RELATIVE)) {
        return false;
    }

    stream_player_reset_interrupt_state();

    g_bgm_ctrl = player_controller_create(bgm_event_cb, NULL);
    if(g_bgm_ctrl == NULL) return false;

    stream_player_get_default_config(&config);
    stream_player_apply_profile(&config, STREAM_PROFILE_BALANCED);
    strncpy(config.url, g_bgm_url, sizeof(config.url) - 1u);
    config.url[sizeof(config.url) - 1u] = '\0';
    config.volume = DESIGN_FRUIT_BGM_VOLUME;

    if(player_controller_set_stream_config(g_bgm_ctrl, &config) != 0) goto fail;

    urls[0] = g_bgm_url;
    if(player_controller_load_urls(g_bgm_ctrl, urls, 1u) != 0) goto fail;

    player_controller_set_repeat_mode(g_bgm_ctrl, PLAYER_REPEAT_ONE);
    player_controller_set_volume(g_bgm_ctrl, DESIGN_FRUIT_BGM_VOLUME);

    g_poll_timer = lv_timer_create(poll_timer_cb, DESIGN_FRUIT_BGM_POLL_MS, NULL);
    if(g_poll_timer == NULL) goto fail;

    g_ready = true;
    return true;

fail:
    if(g_poll_timer != NULL) {
        lv_timer_delete(g_poll_timer);
        g_poll_timer = NULL;
    }
    if(g_bgm_ctrl != NULL) {
        player_controller_destroy(g_bgm_ctrl);
        g_bgm_ctrl = NULL;
    }
    g_ready = false;
    return false;
}

void design_fruit_audio_deinit(void)
{
    if(g_poll_timer != NULL) {
        lv_timer_delete(g_poll_timer);
        g_poll_timer = NULL;
    }
    if(g_bgm_ctrl != NULL) {
        player_controller_stop(g_bgm_ctrl);
        player_controller_destroy(g_bgm_ctrl);
        g_bgm_ctrl = NULL;
    }
    g_ready = false;
}

void design_fruit_audio_play_bgm(void)
{
    if(!g_ready || !g_bgm_enabled || g_bgm_ctrl == NULL) return;
    player_controller_select(g_bgm_ctrl, 0u);
    player_controller_play(g_bgm_ctrl);
}

void design_fruit_audio_stop_bgm(void)
{
    if(!g_ready || g_bgm_ctrl == NULL) return;
    player_controller_pause(g_bgm_ctrl);
}

bool design_fruit_audio_toggle_bgm(void)
{
    g_bgm_enabled = !g_bgm_enabled;
    if(g_bgm_enabled) {
        design_fruit_audio_play_bgm();
    } else {
        design_fruit_audio_stop_bgm();
    }
    return g_bgm_enabled;
}

bool design_fruit_audio_bgm_enabled(void)
{
    return g_bgm_enabled;
}

void design_fruit_audio_play_hit(void)
{
    /* 单路音频后端无法与背景音乐混音，此处不发声（保留接口）。*/
}

void design_fruit_audio_play_over(void)
{
    /* 单路音频后端无法与背景音乐混音，此处不发声（保留接口）。*/
}
