#include "am_player.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "am_icons.h"
#include "am_metrics.h"
#include "am_theme.h"
#include "am_widgets.h"
#include "music_player.h"

#define AM_PLAYER_LOCAL_MAX 128

static am_miniplayer_handles_t g_h;
static am_source_kind_t g_source_kind = AM_SOURCE_NONE;
static const am_local_item_t *g_locals = NULL;
static size_t g_local_count = 0U;
static const am_radio_item_t *g_radios = NULL;
static size_t g_radio_count = 0U;
static size_t g_current_local = 0U;
static size_t g_current_radio = 0U;
static bool g_playlist_open = false;
static uint8_t g_volume = 65U;
static lv_timer_t *g_timer = NULL;
static bool g_local_engine_ready = false;   /* 本地播放列表已 init(避免每次切歌 deinit/init) */

static void am_progress_track_cb(lv_event_t *e);
static void am_volume_track_cb(lv_event_t *e);
static void am_speaker_cb(lv_event_t *e);

static const char *am_current_title(void)
{
    if(g_source_kind == AM_SOURCE_RADIO && g_radios != NULL && g_current_radio < g_radio_count) {
        return g_radios[g_current_radio].title;
    }
    if(g_source_kind == AM_SOURCE_LOCAL && g_locals != NULL && g_current_local < g_local_count) {
        return g_locals[g_current_local].title;
    }
    return "未播放";
}

static const char *am_current_subtitle(void)
{
    if(g_source_kind == AM_SOURCE_RADIO) return "直播流";
    if(g_source_kind == AM_SOURCE_LOCAL) return "本地资料库";
    return "选择本地音乐或广播电台开始";
}

static void am_format_time(uint32_t ms, char *buf, size_t size)
{
    unsigned sec = ms / 1000U;
    snprintf(buf, size, "%u:%02u", sec / 60U, sec % 60U);
}

static void am_refresh_playlist_popup(void)
{
    size_t i;

    if(g_h.playlist_list == NULL) return;
    lv_obj_clean(g_h.playlist_list);

    if(g_source_kind == AM_SOURCE_RADIO) {
        if(g_radios != NULL && g_current_radio < g_radio_count) {
            lv_obj_t *item = lv_obj_create(g_h.playlist_list);
            lv_obj_remove_style_all(item);
            lv_obj_set_size(item, LV_PCT(100), LV_SIZE_CONTENT);
            lv_obj_set_style_pad_all(item, 8, 0);
            lv_obj_set_style_radius(item, 7, 0);
            lv_obj_set_style_bg_color(item, lv_color_hex(0x000000), 0);
            lv_obj_set_style_bg_opa(item, 12, 0);
            lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
            lv_obj_set_style_pad_column(item, 10, 0);
            am_text(item, "1", am_metrics()->f_label, AM_MUTED);
            {
                lv_obj_t *info = lv_obj_create(item);
                lv_obj_remove_style_all(info);
                lv_obj_set_flex_grow(info, 1);
                lv_obj_set_height(info, LV_SIZE_CONTENT);
                lv_obj_set_flex_flow(info, LV_FLEX_FLOW_COLUMN);
                am_text(info, g_radios[g_current_radio].title, am_metrics()->f_body, lv_color_hex(0xfa2d48));
                am_text(info, "LIVE", am_metrics()->f_label, AM_MUTED);
            }
        }
        return;
    }

    for(i = 0U; i < g_local_count; i++) {
        lv_obj_t *item = lv_obj_create(g_h.playlist_list);
        lv_obj_t *info;
        bool active = (i == g_current_local);
        lv_obj_remove_style_all(item);
        lv_obj_set_size(item, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(item, 8, 0);
        lv_obj_set_style_radius(item, 7, 0);
        lv_obj_set_style_bg_color(item, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(item, active ? 12 : LV_OPA_TRANSP, 0);
        lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_column(item, 10, 0);

        {
            char idx_buf[8];
            snprintf(idx_buf, sizeof(idx_buf), "%u", (unsigned)(i + 1U));
            am_text(item, idx_buf, am_metrics()->f_label, active ? lv_color_hex(0xfa2d48) : AM_MUTED);
        }

        info = lv_obj_create(item);
        lv_obj_remove_style_all(info);
        lv_obj_set_flex_grow(info, 1);
        lv_obj_set_height(info, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(info, LV_FLEX_FLOW_COLUMN);
        am_text(info, g_locals[i].title, am_metrics()->f_body, active ? lv_color_hex(0xfa2d48) : AM_TEXT);
        am_text(info, "本地音频", am_metrics()->f_label, AM_MUTED);
    }
}

void am_player_refresh_ui(void)
{
    char cur_buf[16];
    char total_buf[16];
    uint32_t pos = music_player_get_position_ms();
    uint32_t dur = music_player_get_duration_ms();

    if(g_h.title_label != NULL) lv_label_set_text(g_h.title_label, am_current_title());
    if(g_h.subtitle_label != NULL) lv_label_set_text(g_h.subtitle_label, am_current_subtitle());
    if(g_h.play_icon != NULL) lv_label_set_text(g_h.play_icon, music_player_is_playing() ? AM_ICON_PAUSE : AM_ICON_PLAY);
    if(g_h.time_cur != NULL) {
        if(g_source_kind == AM_SOURCE_RADIO && dur == 0U) lv_label_set_text(g_h.time_cur, "LIVE");
        else {
            am_format_time(pos, cur_buf, sizeof(cur_buf));
            lv_label_set_text(g_h.time_cur, cur_buf);
        }
    }
    if(g_h.time_total != NULL) {
        if(g_source_kind == AM_SOURCE_RADIO && dur == 0U) lv_label_set_text(g_h.time_total, "--:--");
        else {
            am_format_time(dur, total_buf, sizeof(total_buf));
            lv_label_set_text(g_h.time_total, total_buf);
        }
    }
    {
        int pct = 0;
        if(dur > 0U) pct = (int)((uint64_t)pos * 100ULL / dur);
        if(pct < 0) pct = 0;
        if(pct > 100) pct = 100;
        if(g_h.progress_fill != NULL) lv_obj_set_width(g_h.progress_fill, LV_PCT(pct));
        if(g_h.knob != NULL && g_h.progress_track != NULL) {
            lv_coord_t tw = lv_obj_get_width(g_h.progress_track);
            lv_coord_t x = (lv_coord_t)((int)tw * pct / 100) - 6;   /* 半个 knob 宽 */
            if(x < -6) x = -6;
            lv_obj_align(g_h.knob, LV_ALIGN_LEFT_MID, x, 0);
        }
    }
    if(g_h.volume_fill != NULL) {
        lv_coord_t width = (lv_coord_t)((70U * g_volume) / 100U);
        lv_obj_set_width(g_h.volume_fill, width);
    }
    if(g_h.playlist_popup != NULL) {
        if(g_playlist_open) lv_obj_clear_flag(g_h.playlist_popup, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_h.playlist_popup, LV_OBJ_FLAG_HIDDEN);
    }

    am_refresh_playlist_popup();
}

static void am_player_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    if(g_source_kind == AM_SOURCE_LOCAL) {
        size_t idx = music_player_get_current_index();
        if(idx < g_local_count) g_current_local = idx;
    }
    am_player_refresh_ui();
}

void am_player_init(void)
{
    memset(&g_h, 0, sizeof(g_h));
    g_source_kind = AM_SOURCE_NONE;
    g_locals = NULL;
    g_local_count = 0U;
    g_radios = NULL;
    g_radio_count = 0U;
    g_current_local = 0U;
    g_current_radio = 0U;
    g_playlist_open = false;
    g_volume = 65U;
    g_local_engine_ready = false;
    if(g_timer != NULL) lv_timer_delete(g_timer);
    g_timer = lv_timer_create(am_player_timer_cb, 120, NULL);
}

void am_player_deinit(void)
{
    if(g_timer != NULL) {
        lv_timer_delete(g_timer);
        g_timer = NULL;
    }
    music_player_deinit();
    memset(&g_h, 0, sizeof(g_h));
    g_source_kind = AM_SOURCE_NONE;
    g_local_engine_ready = false;
}

void am_player_bind_miniplayer(const am_miniplayer_handles_t *h)
{
    if(h == NULL) {
        memset(&g_h, 0, sizeof(g_h));
        return;
    }
    g_h = *h;

    /* 进度轨道点击 → seek;音量轨道点击 → 设音量;喇叭点击 → 静音 */
    if(g_h.progress_track != NULL) {
        lv_obj_add_flag(g_h.progress_track, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(g_h.progress_track, am_progress_track_cb, LV_EVENT_CLICKED, NULL);
    }
    if(g_h.volume_track != NULL) {
        lv_obj_add_flag(g_h.volume_track, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(g_h.volume_track, am_volume_track_cb, LV_EVENT_CLICKED, NULL);
    }
    if(g_h.volume_icon != NULL) {
        lv_obj_add_flag(g_h.volume_icon, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(g_h.volume_icon, am_speaker_cb, LV_EVENT_CLICKED, NULL);
    }
    am_player_refresh_ui();
}

void am_player_set_sources(const am_local_item_t *locals, size_t local_count,
                           const am_radio_item_t *radios, size_t radio_count)
{
    g_locals = locals;
    g_local_count = local_count;
    g_radios = radios;
    g_radio_count = radio_count;
    am_player_refresh_ui();
}

void am_player_play_local_index(size_t index)
{
    const char *urls[AM_PLAYER_LOCAL_MAX];
    size_t i;

    if(g_locals == NULL || g_local_count == 0U || index >= g_local_count) return;
    if(g_local_count > AM_PLAYER_LOCAL_MAX) return;

    g_source_kind = AM_SOURCE_LOCAL;
    g_current_local = index;

    /* 一次 init 整份本地列表,之后只 select 切歌(此前每次都 deinit/init,重探测所有时长,过重) */
    if(!g_local_engine_ready) {
        for(i = 0U; i < g_local_count; i++) urls[i] = g_locals[i].path;
        music_player_deinit();
        music_player_init(urls, g_local_count);
        music_player_set_volume(g_volume);
        g_local_engine_ready = true;
    }
    music_player_select(index);
    am_player_refresh_ui();
}

void am_player_play_radio_index(size_t index)
{
    const char *url_list[1];

    if(g_radios == NULL || g_radio_count == 0U || index >= g_radio_count) return;
    g_source_kind = AM_SOURCE_RADIO;
    g_current_radio = index;
    music_player_deinit();
    g_local_engine_ready = false;   /* 切电台重置了单例,本地需重新 init */
    url_list[0] = g_radios[index].url;
    music_player_init(url_list, 1U);
    music_player_set_volume(g_volume);
    am_player_refresh_ui();
}

void am_player_toggle_playback(void)
{
    if(music_player_is_playing()) music_player_pause();
    else music_player_resume();
    am_player_refresh_ui();
}

void am_player_prev(void)
{
    if(g_source_kind == AM_SOURCE_RADIO) {
        if(g_radio_count == 0U) return;
        if(g_current_radio == 0U) g_current_radio = g_radio_count - 1U;
        else g_current_radio--;
        am_player_play_radio_index(g_current_radio);
        return;
    }
    music_player_prev();
    if(g_current_local > 0U) g_current_local--;
    am_player_refresh_ui();
}

void am_player_next(void)
{
    if(g_source_kind == AM_SOURCE_RADIO) {
        if(g_radio_count == 0U) return;
        g_current_radio = (g_current_radio + 1U) % g_radio_count;
        am_player_play_radio_index(g_current_radio);
        return;
    }
    music_player_next();
    if(g_local_count > 0U) g_current_local = (g_current_local + 1U) % g_local_count;
    am_player_refresh_ui();
}

void am_player_cycle_mode(void)
{
    music_player_cycle_play_mode();
    am_player_refresh_ui();
}

void am_player_set_volume_percent(uint8_t percent)
{
    if(percent > 100U) percent = 100U;
    g_volume = percent;
    music_player_set_volume(percent);
    am_player_refresh_ui();
}

void am_player_seek_percent(uint8_t percent)
{
    uint32_t dur;
    if(g_source_kind != AM_SOURCE_LOCAL) return;   /* 电台直播不支持 seek */
    dur = music_player_get_duration_ms();
    if(dur == 0U) return;
    if(percent > 100U) percent = 100U;
    music_player_seek((uint32_t)((uint64_t)dur * percent / 100U));
    am_player_refresh_ui();
}

void am_player_toggle_mute(void)
{
    music_player_mute_toggle();
    am_player_refresh_ui();
}

/* 轨道点击/拖动 → 计算命中比例 */
static uint8_t am_track_ratio(lv_event_t *e)
{
    lv_indev_t *indev = lv_event_get_indev(e);
    lv_obj_t *track = lv_event_get_target(e);
    lv_point_t p;
    lv_area_t a;
    int32_t w, rel, pct;

    if(indev == NULL || track == NULL) return 0U;
    lv_indev_get_point(indev, &p);
    lv_obj_get_coords(track, &a);
    w = lv_area_get_width(&a);
    if(w <= 0) return 0U;
    rel = p.x - a.x1;
    pct = rel * 100 / w;
    if(pct < 0) pct = 0;
    if(pct > 100) pct = 100;
    return (uint8_t)pct;
}

static void am_progress_track_cb(lv_event_t *e)
{
    am_player_seek_percent(am_track_ratio(e));
}

static void am_volume_track_cb(lv_event_t *e)
{
    am_player_set_volume_percent(am_track_ratio(e));
}

static void am_speaker_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    am_player_toggle_mute();
}

void am_player_set_playlist_open(bool open)
{
    g_playlist_open = open;
    am_player_refresh_ui();
}

bool am_player_playlist_open(void)
{
    return g_playlist_open;
}

am_source_kind_t am_player_source_kind(void)
{
    return g_source_kind;
}

size_t am_player_current_local_index(void)
{
    return g_current_local;
}

size_t am_player_current_radio_index(void)
{
    return g_current_radio;
}

bool am_player_is_playing(void)
{
    return music_player_is_playing();
}

uint8_t am_player_volume_percent(void)
{
    return g_volume;
}

void am_player_on_play_pause(lv_event_t *e)
{
    LV_UNUSED(e);
    am_player_toggle_playback();
}

void am_player_on_prev(lv_event_t *e)
{
    LV_UNUSED(e);
    am_player_prev();
}

void am_player_on_next(lv_event_t *e)
{
    LV_UNUSED(e);
    am_player_next();
}

void am_player_on_mode(lv_event_t *e)
{
    LV_UNUSED(e);
    am_player_cycle_mode();
}
