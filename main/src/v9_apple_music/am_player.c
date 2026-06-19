/* main/src/v9_apple_music/am_player.c */
#include "am_player.h"
#include "am_icons.h"
#include "music_player.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <libgen.h>

#define AM_PLAYER_LOCAL_MAX 64

typedef enum {
    AM_PLAYER_MODE_IDLE = 0,
    AM_PLAYER_MODE_LOCAL,
    AM_PLAYER_MODE_STREAM,
} am_player_mode_t;

static am_miniplayer_handles_t g_h;
static am_player_mode_t        g_mode          = AM_PLAYER_MODE_IDLE;
static char                    g_stream_title[128];
static char                   *g_local_urls[AM_PLAYER_LOCAL_MAX];
static size_t                  g_local_count   = 0;
static size_t                  g_last_idx      = (size_t)-1;
static bool                    g_last_playing  = false;
static lv_timer_t             *g_timer         = NULL;

/* ── helpers ──────────────────────────────────────────────────────────── */

static const char *current_display_title(void) {
    if(g_mode == AM_PLAYER_MODE_STREAM) return g_stream_title;
    if(g_mode == AM_PLAYER_MODE_LOCAL && g_local_count > 0) {
        size_t idx = music_player_get_current_index();
        if(idx < g_local_count && g_local_urls[idx]) {
            static char name_buf[128];
            char copy[512];
            strncpy(copy, g_local_urls[idx], sizeof(copy) - 1);
            copy[sizeof(copy) - 1] = '\0';
            char *b = basename(copy);
            strncpy(name_buf, b, sizeof(name_buf) - 1);
            name_buf[sizeof(name_buf) - 1] = '\0';
            char *dot = strrchr(name_buf, '.');
            if(dot) *dot = '\0';
            return name_buf;
        }
    }
    return "";
}

static void refresh_title(void) {
    if(!g_h.title_label) return;
    lv_label_set_text(g_h.title_label, current_display_title());
}

static void refresh_play_icon(bool playing) {
    if(!g_h.play_icon) return;
    lv_label_set_text(g_h.play_icon, playing ? AM_ICON_PAUSE : AM_ICON_PLAY);
}

static void refresh_progress(void) {
    uint32_t pos = music_player_get_position_ms();
    uint32_t dur = music_player_get_duration_ms();

    /* time_cur */
    if(g_h.time_cur) {
        if(g_mode == AM_PLAYER_MODE_STREAM && dur == 0) {
            lv_label_set_text(g_h.time_cur, "LIVE");
        } else if(dur == 0) {
            lv_label_set_text(g_h.time_cur, "--:--");
        } else {
            char buf[8];
            snprintf(buf, sizeof(buf), "%u:%02u", pos / 60000u, (pos / 1000u) % 60u);
            lv_label_set_text(g_h.time_cur, buf);
        }
    }

    /* time_total */
    if(g_h.time_total) {
        if(dur == 0) {
            lv_label_set_text(g_h.time_total, "--:--");
        } else {
            char buf[8];
            snprintf(buf, sizeof(buf), "%u:%02u", dur / 60000u, (dur / 1000u) % 60u);
            lv_label_set_text(g_h.time_total, buf);
        }
    }

    /* progress fill: 仅在 dur > 0 时更新宽度 */
    if(g_h.progress_fill && dur > 0) {
        int pct = (int)((uint64_t)pos * 100ull / dur);
        if(pct > 100) pct = 100;
        lv_obj_set_width(g_h.progress_fill, LV_PCT(pct));
    }
}

/* ── timer callback (主线程，100ms) ──────────────────────────────────── */

static void am_player_timer_cb(lv_timer_t *t) {
    (void)t;
    if(g_mode == AM_PLAYER_MODE_IDLE) return;

    /* 检测曲目切换 */
    size_t idx = music_player_get_current_index();
    if(idx != g_last_idx) {
        g_last_idx = idx;
        refresh_title();
    }

    /* 检测播放状态变化 */
    bool playing = music_player_is_playing();
    if(playing != g_last_playing) {
        g_last_playing = playing;
        refresh_play_icon(playing);
    }

    refresh_progress();
}

/* ── public API ───────────────────────────────────────────────────────── */

void am_player_init(void) {
    memset(&g_h, 0, sizeof(g_h));
    g_mode         = AM_PLAYER_MODE_IDLE;
    g_stream_title[0] = '\0';
    g_last_idx     = (size_t)-1;
    g_last_playing = false;
    if(g_timer) { lv_timer_delete(g_timer); g_timer = NULL; }
    g_timer = lv_timer_create(am_player_timer_cb, 100, NULL);
}

void am_player_deinit(void) {
    size_t i;
    if(g_timer) { lv_timer_delete(g_timer); g_timer = NULL; }
    music_player_deinit();
    for(i = 0; i < g_local_count; i++) { free(g_local_urls[i]); g_local_urls[i] = NULL; }
    g_local_count = 0;
    memset(&g_h, 0, sizeof(g_h));
}

void am_player_bind_miniplayer(const am_miniplayer_handles_t *h) {
    if(!h) { memset(&g_h, 0, sizeof(g_h)); return; }
    g_h = *h;
    /* 立即刷新 */
    refresh_title();
    refresh_play_icon(music_player_is_playing());
    refresh_progress();
}

void am_player_load_local(const char **urls, size_t count, size_t start_index) {
    size_t i;
    for(i = 0; i < g_local_count; i++) { free(g_local_urls[i]); g_local_urls[i] = NULL; }
    g_local_count = 0;
    for(i = 0; i < count && i < AM_PLAYER_LOCAL_MAX; i++) {
        g_local_urls[i] = strdup(urls[i]);
        if(g_local_urls[i]) g_local_count++;
    }
    g_mode     = AM_PLAYER_MODE_LOCAL;
    g_last_idx = (size_t)-1;
    music_player_deinit();
    if(g_local_count > 0) {
        music_player_init((const char **)g_local_urls, g_local_count);
        if(start_index > 0 && start_index < g_local_count) {
            music_player_select(start_index);
        }
    }
}

void am_player_play_stream(const char *url, const char *title) {
    const char *one[1];
    strncpy(g_stream_title, title ? title : "", sizeof(g_stream_title) - 1);
    g_stream_title[sizeof(g_stream_title) - 1] = '\0';
    g_mode     = AM_PLAYER_MODE_STREAM;
    g_last_idx = (size_t)-1;
    music_player_deinit();
    one[0] = url;
    music_player_init(one, 1);
}

void am_player_on_play_pause(lv_event_t *e) {
    (void)e;
    if(music_player_is_playing()) music_player_pause();
    else                          music_player_resume();
}

void am_player_on_prev(lv_event_t *e) { (void)e; music_player_prev(); }
void am_player_on_next(lv_event_t *e) { (void)e; music_player_next(); }
