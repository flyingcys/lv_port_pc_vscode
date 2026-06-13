#define _DEFAULT_SOURCE
#include "music_player.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "lvgl/lvgl.h"
#include "lvgl/demos/music/lv_demo_music_main.h"
#include "player_controller.h"
#include "stream_player.h"

#define MUSIC_PLAYER_MAX_TRACKS 64U

/* forward declaration */
static uint32_t audio_probe_duration_ms(const char *url);

static char               *g_urls[MUSIC_PLAYER_MAX_TRACKS];
static size_t              g_url_count  = 0;
static player_controller_t *g_controller = NULL;
static lv_timer_t          *g_poll_timer = NULL;

static uint32_t g_audio_sample_rate   = 0;
static uint8_t  g_audio_channels      = 0;
static uint8_t  g_audio_bps           = 16;
static uint32_t g_current_duration_ms = 0;

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
    if(g_urls[g_url_count]) g_url_count++;
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
    {
        const player_playlist_item_t *first =
            player_controller_get_playlist_item(g_controller, 0U);
        g_current_duration_ms = (first && !first->is_live)
                                ? audio_probe_duration_ms(first->url) : 0U;
    }
    player_controller_play(g_controller);
    return;

cleanup:
    if(g_controller) { player_controller_destroy(g_controller); g_controller = NULL; }
    for(i = 0U; i < g_url_count; i++) { free(g_urls[i]); g_urls[i] = NULL; }
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
            g_current_duration_ms = (item && !item->is_live)
                                    ? audio_probe_duration_ms(item->url) : 0U;
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
    if(g_audio_sample_rate == 0U || g_audio_channels == 0U) return 0U;
    if(player_controller_get_stats(g_controller, &stats) != 0) return 0U;

    denom = g_audio_sample_rate * (uint32_t)g_audio_channels * ((uint32_t)g_audio_bps / 8U);
    if(denom == 0U) return 0U;
    return (uint32_t)(stats.pcm_bytes_written * 1000ULL / denom);
}

/* ── audio file duration probe ───────────────────────────────────────── */
static uint32_t audio_probe_duration_ms(const char *url) {
    FILE   *f;
    uint8_t buf[48];
    size_t  n;
    long    file_size;

    if(!url) return 0U;
    if(strstr(url, "://") && strncmp(url, "file://", 7) != 0) return 0U;
    {
        const char *path = (strncmp(url, "file://", 7) == 0) ? url + 7 : url;
        f = fopen(path, "rb");
        if(!f) return 0U;
        fseek(f, 0, SEEK_END);
        file_size = ftell(f);
        rewind(f);
        n = fread(buf, 1, sizeof(buf), f);
        fclose(f);
    }
    if(n < 12U) return 0U;

    /* WAV */
    if(n >= 44U &&
       buf[0]=='R' && buf[1]=='I' && buf[2]=='F' && buf[3]=='F' &&
       buf[8]=='W' && buf[9]=='A' && buf[10]=='V' && buf[11]=='E') {
        uint32_t byte_rate = (uint32_t)buf[28] | ((uint32_t)buf[29]<<8)
                           | ((uint32_t)buf[30]<<16) | ((uint32_t)buf[31]<<24);
        uint32_t data_size = (uint32_t)buf[40] | ((uint32_t)buf[41]<<8)
                           | ((uint32_t)buf[42]<<16) | ((uint32_t)buf[43]<<24);
        if(byte_rate == 0U) return 0U;
        return data_size / byte_rate * 1000U + (data_size % byte_rate) * 1000U / byte_rate;
    }

    /* FLAC */
    if(n >= 42U &&
       buf[0]=='f' && buf[1]=='L' && buf[2]=='a' && buf[3]=='C') {
        uint32_t sr = ((uint32_t)buf[18] << 12) | ((uint32_t)buf[19] << 4)
                    | ((uint32_t)buf[20] >> 4);
        uint64_t total = ((uint64_t)(buf[21] & 0x0FU) << 32)
                       | ((uint64_t)buf[22] << 24) | ((uint64_t)buf[23] << 16)
                       | ((uint64_t)buf[24] << 8)  |  (uint64_t)buf[25];
        if(sr == 0U) return 0U;
        return (uint32_t)(total * 1000ULL / sr);
    }

    /* MP3 CBR estimate */
    {
        static const uint32_t kbps_mpeg1_l3[16] = {
            0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0
        };
        uint32_t i;
        for(i = 0U; i + 3U < (uint32_t)n; i++) {
            if(buf[i] == 0xFFU && (buf[i+1U] & 0xE0U) == 0xE0U) {
                uint8_t version = (buf[i+1U] >> 3) & 0x03U;
                uint8_t layer   = (buf[i+1U] >> 1) & 0x03U;
                uint8_t br_idx  = (buf[i+2U] >> 4) & 0x0FU;
                if(version == 3U && layer == 1U && br_idx > 0U && br_idx < 15U) {
                    uint32_t bps2 = kbps_mpeg1_l3[br_idx] * 1000U;
                    if(bps2 == 0U) break;
                    uint32_t audio_bytes = (uint32_t)(file_size > 128L ? file_size - 128L : file_size);
                    return audio_bytes * 8U / (bps2 / 1000U);
                }
                break;
            }
        }
    }
    return 0U;
}

uint32_t music_player_get_duration_ms(void) {
    return g_current_duration_ms;
}

/* ── seek stub (full implementation in Task S3) ──────────────────────── */
void music_player_seek(uint32_t position_ms) {
    (void)position_ms;
}
