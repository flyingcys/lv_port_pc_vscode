#define _DEFAULT_SOURCE
#include "music_player.h"
#include <dirent.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "lvgl/lvgl.h"
#include "desktop_app_launcher.h"
#include "desktop_metrics.h"
#include "local_music_demo/lv_demo_music.h"
#include "local_music_demo/lv_demo_music_main.h"
#include "player_controller.h"
#include "stream_player.h"

#define MUSIC_PLAYER_MAX_TRACKS 64U

static char               *g_urls[MUSIC_PLAYER_MAX_TRACKS];
static uint32_t            g_url_duration_ms[MUSIC_PLAYER_MAX_TRACKS];
static size_t              g_url_count  = 0;
static player_controller_t *g_controller = NULL;
static lv_timer_t          *g_poll_timer = NULL;

static _Atomic uint32_t g_audio_sample_rate = 0U;
static _Atomic uint8_t  g_audio_channels    = 0U;
static _Atomic uint8_t  g_audio_bps         = 16U;
static uint32_t g_current_duration_ms = 0;

static music_play_mode_t g_play_mode = MP_MODE_SEQ;
static uint8_t  g_volume  = 100U;
static bool     g_muted   = false;

typedef struct {
    player_controller_event_t  event;
    size_t                     track_index;
    player_controller_state_t  state;
} music_player_async_arg_t;

static void on_player_event(player_controller_t *controller,
                            player_controller_event_t event,
                            const void *event_data,
                            void *user_data);
static void music_player_async_handler(void *data);
static void poll_timer_cb(lv_timer_t *t);
static void mp_apply_play_mode(music_play_mode_t mode);
static void mp_apply_volume(void);
static lv_obj_t * local_music_demo_builder(lv_obj_t *overlay, int32_t screen_w, int32_t screen_h);

static const char *s_local_music_demo_paths[] = {
    "third-party/hls_player_demo/test_file",
};

/* ── directory expansion helpers ─────────────────────────────────────── */

static bool mp_path_is_directory(const char *path) {
    struct stat st;
    if(!path || path[0] == '\0') return false;
    if(stat(path, &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

static bool mp_path_is_regular_file(const char *path) {
    struct stat st;
    if(!path || path[0] == '\0') return false;
    if(stat(path, &st) != 0) return false;
    return S_ISREG(st.st_mode);
}

static bool mp_url_is_local(const char *url) {
    if(!url || url[0] == '\0') return false;
    if(strstr(url, "://") == NULL) return true;
    return strncmp(url, "file://", 7) == 0;
}

static const char *mp_local_path(const char *url) {
    if(!mp_url_is_local(url)) return NULL;
    if(strncmp(url, "file://", 7) == 0) return url + 7;
    return url;
}

static char *mp_join_path(const char *dir, const char *name) {
    size_t dlen = strlen(dir);
    size_t nlen = strlen(name);
    bool   sep  = (dlen > 0U && dir[dlen - 1U] != '/');
    char  *path = (char *)malloc(dlen + (sep ? 1U : 0U) + nlen + 1U);
    if(!path) return NULL;
    memcpy(path, dir, dlen);
    if(sep) path[dlen++] = '/';
    memcpy(path + dlen, name, nlen + 1U);
    return path;
}

static int mp_compare_strings(const void *a, const void *b) {
    return strcmp(*(const char * const *)a, *(const char * const *)b);
}

static void mp_expand_directory(const char *dir_path) {
    DIR           *d;
    struct dirent *entry;
    char          *entries[MUSIC_PLAYER_MAX_TRACKS];
    size_t         entry_count = 0U;
    size_t         i;

    d = opendir(dir_path);
    if(!d) return;

    while((entry = readdir(d)) != NULL) {
        char *path;
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        path = mp_join_path(dir_path, entry->d_name);
        if(!path) continue;
        if(!mp_path_is_regular_file(path) || !stream_player_is_supported_local_audio_file(path)) {
            free(path);
            continue;
        }
        if(entry_count >= MUSIC_PLAYER_MAX_TRACKS) { free(path); continue; }
        entries[entry_count++] = path;
    }
    closedir(d);

    if(entry_count == 0U) return;
    qsort(entries, entry_count, sizeof(entries[0]), mp_compare_strings);

    for(i = 0U; i < entry_count; i++) {
        if(g_url_count < MUSIC_PLAYER_MAX_TRACKS) {
            g_url_duration_ms[g_url_count] = 0U;
            g_urls[g_url_count++] = entries[i];
        } else {
            free(entries[i]);
        }
    }
}

static void mp_add_url(const char *url) {
    const char *local;
    if(g_url_count >= MUSIC_PLAYER_MAX_TRACKS) return;
    local = mp_local_path(url);
    if(local && mp_path_is_directory(local)) {
        mp_expand_directory(local);
        return;
    }
    g_urls[g_url_count] = strdup(url);
    if(g_urls[g_url_count]) {
        g_url_duration_ms[g_url_count] = 0U;
        g_url_count++;
    }
}

static void mp_refresh_duration_cache(void) {
    size_t i;
    if(!g_controller) return;
    for(i = 0U; i < g_url_count; i++) {
        const player_playlist_item_t *item =
            player_controller_get_playlist_item(g_controller, i);
        g_url_duration_ms[i] = (item && !item->is_live) ? stream_player_probe_file_duration_ms(item->url) : 0U;
    }
}

/* ── lifecycle ────────────────────────────────────────────────────────── */

void music_player_init(const char **urls, size_t count) {
    stream_player_config_t config;
    size_t i;

    if(!urls || count == 0U) return;
    if(g_controller) music_player_deinit();

    for(i = 0U; i < count; i++) mp_add_url(urls[i]);
    if(g_url_count == 0U) return;

    stream_player_reset_interrupt_state();

    g_controller = player_controller_create(on_player_event, NULL);
    if(!g_controller) goto cleanup;

    stream_player_get_default_config(&config);
    stream_player_apply_profile(&config, STREAM_PROFILE_BALANCED);
    strncpy(config.url, g_urls[0], sizeof(config.url) - 1U);
    config.url[sizeof(config.url) - 1U] = '\0';

    if(player_controller_set_stream_config(g_controller, &config) != 0) goto cleanup;
    if(player_controller_load_urls(g_controller, (const char * const *)g_urls, g_url_count) != 0) goto cleanup;

    g_poll_timer = lv_timer_create(poll_timer_cb, 100, NULL);
    if(!g_poll_timer) goto cleanup;
    mp_refresh_duration_cache();
    mp_apply_play_mode(g_play_mode);
    mp_apply_volume();
    g_current_duration_ms = g_url_duration_ms[0];
    player_controller_play(g_controller);
    return;

cleanup:
    if(g_controller) { player_controller_destroy(g_controller); g_controller = NULL; }
    for(i = 0U; i < g_url_count; i++) {
        free(g_urls[i]);
        g_urls[i] = NULL;
        g_url_duration_ms[i] = 0U;
    }
    g_url_count = 0U;
}

void music_player_deinit(void) {
    size_t i;
    lv_async_call_cancel(music_player_async_handler, NULL);
    if(g_poll_timer) { lv_timer_delete(g_poll_timer); g_poll_timer = NULL; }
    if(g_controller) { player_controller_destroy(g_controller); g_controller = NULL; }
    for(i = 0U; i < g_url_count; i++) { free(g_urls[i]); g_urls[i] = NULL; }
    g_url_count = 0U;
}

/* ── public API ───────────────────────────────────────────────────────── */

void music_player_play(void)              { if(g_controller) player_controller_play(g_controller); }
void music_player_pause(void)             { if(g_controller) player_controller_pause(g_controller); }
void music_player_resume(void)            { if(g_controller) player_controller_resume(g_controller); }
void music_player_next(void)              { if(g_controller) player_controller_next(g_controller); }
void music_player_prev(void)              { if(g_controller) player_controller_prev(g_controller); }
void music_player_select(size_t index)    { if(g_controller) player_controller_select(g_controller, index); }

size_t music_player_get_count(void) {
    return g_controller ? player_controller_get_playlist_count(g_controller) : 0U;
}
size_t music_player_get_current_index(void) {
    return g_controller ? player_controller_get_current_index(g_controller) : 0U;
}
const char *music_player_get_title(size_t index) {
    const player_playlist_item_t *item =
        g_controller ? player_controller_get_playlist_item(g_controller, index) : NULL;
    return item ? item->title : NULL;
}
bool music_player_is_live(size_t index) {
    const player_playlist_item_t *item =
        g_controller ? player_controller_get_playlist_item(g_controller, index) : NULL;
    return item ? item->is_live : false;
}
bool music_player_is_playing(void) {
    return g_controller &&
           (player_controller_get_state(g_controller) == PLAYER_CONTROLLER_STATE_PLAYING);
}

/* ── LVGL poll timer ─────────────────────────────────────────────────── */

static void poll_timer_cb(lv_timer_t *t) {
    (void)t;
    if(g_controller) player_controller_poll(g_controller);
}

/* ── cross-thread event handling ─────────────────────────────────────── */

static void music_player_async_handler(void *data) {
    music_player_async_arg_t *arg = (music_player_async_arg_t *)data;
    if(!arg) return;

    switch(arg->event) {
        case PLAYER_CONTROLLER_EVENT_TRACK_CHANGED: {
            const player_playlist_item_t *item =
                player_controller_get_playlist_item(g_controller, arg->track_index);
            g_audio_sample_rate   = 0U;
            g_audio_channels      = 0U;
            if(item && !item->is_live && arg->track_index < MUSIC_PLAYER_MAX_TRACKS &&
               g_url_duration_ms[arg->track_index] == 0U) {
                g_url_duration_ms[arg->track_index] = stream_player_probe_file_duration_ms(item->url);
            }
            g_current_duration_ms = (arg->track_index < MUSIC_PLAYER_MAX_TRACKS)
                                    ? g_url_duration_ms[arg->track_index] : 0U;
            _lv_demo_music_play((uint32_t)arg->track_index);
            break;
        }
        case PLAYER_CONTROLLER_EVENT_STATE_CHANGED:
            if(arg->state == PLAYER_CONTROLLER_STATE_PAUSED ||
               arg->state == PLAYER_CONTROLLER_STATE_STOPPED) {
                _lv_demo_music_pause();
            } else if(arg->state == PLAYER_CONTROLLER_STATE_PLAYING) {
                _lv_demo_music_resume();
            }
            break;
        case PLAYER_CONTROLLER_EVENT_PLAYLIST_END:
        case PLAYER_CONTROLLER_EVENT_ERROR:
            _lv_demo_music_pause();
            break;
        default:
            break;
    }
    free(arg);
}

static void on_player_event(player_controller_t *controller,
                            player_controller_event_t event,
                            const void *event_data,
                            void *user_data) {
    music_player_async_arg_t *arg;
    (void)user_data;

    switch(event) {
        case PLAYER_CONTROLLER_EVENT_TRACK_CHANGED:
        case PLAYER_CONTROLLER_EVENT_STATE_CHANGED:
        case PLAYER_CONTROLLER_EVENT_PLAYLIST_END:
        case PLAYER_CONTROLLER_EVENT_ERROR:
            break;
        default:
            return;
    }

    arg = (music_player_async_arg_t *)malloc(sizeof(*arg));
    if(!arg) return;

    arg->event       = event;
    arg->track_index = player_controller_get_current_index(controller);
    arg->state       = (event == PLAYER_CONTROLLER_EVENT_STATE_CHANGED && event_data)
                       ? *(const player_controller_state_t *)event_data
                       : PLAYER_CONTROLLER_STATE_IDLE;

    lv_async_call(music_player_async_handler, arg);
}

/* ── hls observer: capture actual audio output format ───────────────── */
void hls_observer_audio_output_started(uint32_t sample_rate,
                                       uint32_t channels,
                                       uint32_t bits_per_sample) {
    g_audio_sample_rate = sample_rate;
    g_audio_channels    = (uint8_t)channels;
    g_audio_bps         = (uint8_t)bits_per_sample;
}

/* ── real position from PCM bytes written ────────────────────────────── */
uint32_t music_player_get_position_ms(void) {
    stream_player_stats_t stats;
    uint32_t denom;

    if(!g_controller) return 0U;
    if(player_controller_get_stats(g_controller, &stats) == 0 && stats.position_ms > 0U) {
        return (uint32_t)stats.position_ms;
    }
    if(g_audio_sample_rate == 0U || g_audio_channels == 0U) return 0U;

    denom = g_audio_sample_rate * (uint32_t)g_audio_channels * ((uint32_t)g_audio_bps / 8U);
    if(denom == 0U) return 0U;
    return (uint32_t)(stats.pcm_bytes_written * 1000ULL / denom);
}

uint32_t music_player_get_duration_ms(void) {
    stream_player_stats_t stats;
    if(g_controller && player_controller_get_stats(g_controller, &stats) == 0 &&
       stats.duration_ms > 0U) {
        return (uint32_t)stats.duration_ms;
    }
    return g_current_duration_ms;
}

uint32_t music_player_get_track_duration_ms(size_t index) {
    if(index >= MUSIC_PLAYER_MAX_TRACKS) return 0U;
    return g_url_duration_ms[index];
}

/* ── seek (Task S3) ──────────────────────────────────────────────────── */
void music_player_seek(uint32_t position_ms) {
    if(g_controller) player_controller_seek(g_controller, position_ms);
}

/* ── play mode + volume ──────────────────────────────────────────────── */

static void mp_apply_play_mode(music_play_mode_t mode) {
    if(!g_controller) return;
    switch(mode) {
        case MP_MODE_SEQ:
            player_controller_set_repeat_mode(g_controller, PLAYER_REPEAT_OFF);
            player_controller_set_shuffle(g_controller, false);
            break;
        case MP_MODE_REPEAT_ONE:
            player_controller_set_repeat_mode(g_controller, PLAYER_REPEAT_ONE);
            player_controller_set_shuffle(g_controller, false);
            break;
        case MP_MODE_REPEAT_ALL:
            player_controller_set_repeat_mode(g_controller, PLAYER_REPEAT_ALL);
            player_controller_set_shuffle(g_controller, false);
            break;
        case MP_MODE_SHUFFLE:
            player_controller_set_repeat_mode(g_controller, PLAYER_REPEAT_OFF);
            player_controller_set_shuffle(g_controller, true);
            break;
        default:
            break;
    }
}

static void mp_apply_volume(void) {
    if(!g_controller) return;
    float v = g_muted ? 0.0f : (float)g_volume / 100.0f;
    player_controller_set_volume(g_controller, v);
}

void music_player_set_play_mode(music_play_mode_t mode) {
    g_play_mode = mode;
    mp_apply_play_mode(mode);
}

music_play_mode_t music_player_get_play_mode(void) {
    return g_play_mode;
}

music_play_mode_t music_player_cycle_play_mode(void) {
    music_play_mode_t next = (music_play_mode_t)((g_play_mode + 1) % 4);
    music_player_set_play_mode(next);
    return next;
}

void music_player_set_volume(uint8_t percent) {
    if(percent > 100U) percent = 100U;
    g_volume = percent;
    if(g_muted && percent > 0U) g_muted = false; /* raising volume unmutes */
    mp_apply_volume();
}

uint8_t music_player_get_volume(void) {
    return g_volume;
}

void music_player_mute_toggle(void) {
    g_muted = !g_muted;
    mp_apply_volume();
}

bool music_player_is_muted(void) {
    return g_muted;
}

/* ── desktop app entry ───────────────────────────────────────────────── */

static lv_obj_t * local_music_demo_builder(lv_obj_t *overlay, int32_t screen_w, int32_t screen_h) {
    (void)screen_w;
    (void)screen_h;

    music_player_init(s_local_music_demo_paths,
                      sizeof(s_local_music_demo_paths) / sizeof(s_local_music_demo_paths[0]));

    lv_demo_args_t args;
    lv_demo_args_init(&args);
    args.parent = overlay;
    lv_demo_music_with_args(&args);

    return overlay;
}

void local_music_demo_launch(void) {
    const desktop_metrics_t *m = desktop_metrics();
    desktop_app_launcher_open_with_close(local_music_demo_builder,
                                         local_music_demo_close,
                                         m->screen_w,
                                         m->screen_h);
}

void local_music_demo_close(void) {
    music_player_deinit();
}
