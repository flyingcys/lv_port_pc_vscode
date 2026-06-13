#define _DEFAULT_SOURCE
#include "music_player.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "lvgl/lvgl.h"
#include "lvgl/demos/music/lv_demo_music_main.h"
#include "player_controller.h"
#include "stream_player.h"

#define MUSIC_PLAYER_MAX_TRACKS 64U

static char               *g_urls[MUSIC_PLAYER_MAX_TRACKS];
static size_t              g_url_count  = 0;
static player_controller_t *g_controller = NULL;
static lv_timer_t          *g_poll_timer = NULL;

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
        case PLAYER_CONTROLLER_EVENT_TRACK_CHANGED:
            _lv_demo_music_play((uint32_t)arg->track_index);
            break;
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
