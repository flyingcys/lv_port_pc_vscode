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
static am_player_pick_cb_t g_pick_cb = NULL;
static size_t g_pending_pick = 0U;           /* 弹层待切歌的目标索引(见 am_playlist_pick_async) */
static bool g_single_file_mode = false;
static char g_single_file_path[1024];
static char g_single_file_title[256];

/* 播放列表弹层已构建的内容指纹。am_player_refresh_ui 每 120ms 被定时器调用,
 * 若每次都 lv_obj_clean+重建列表,会(1)把滚动位置重置回顶部→看不到后面的曲目,
 * (2)在用户按下某行的瞬间销毁该行→CLICKED 丢失→点了切不了歌。
 * 故仅当来源/当前索引/条目数变化时才重建,静止播放时保留现有列表。 */
static bool g_pl_built = false;
static am_source_kind_t g_pl_kind = AM_SOURCE_NONE;
static size_t g_pl_index = (size_t)-1;
static size_t g_pl_count = (size_t)-1;

static void am_progress_track_cb(lv_event_t *e);
static void am_volume_track_cb(lv_event_t *e);
static void am_speaker_cb(lv_event_t *e);
static void am_playlist_item_click_cb(lv_event_t *e);
static void am_playlist_pick_async(void *data);
static void am_playlist_scrollbar_sync(void);
static void am_playlist_list_scroll_cb(lv_event_t *e);
static void am_playlist_thumb_event_cb(lv_event_t *e);
static int32_t am_playlist_content_range(const lv_obj_t *list);
static void am_playlist_track_sync_geometry(lv_obj_t *track, lv_obj_t *list);
static void am_copy_text(char *dst, size_t dst_size, const char *src);

static bool g_playlist_thumb_dragging = false;
static int32_t g_playlist_thumb_press_ofs_y = 0;
static int32_t g_playlist_thumb_track_top = 0;

static void am_copy_text(char *dst, size_t dst_size, const char *src)
{
    size_t len;

    if(dst == NULL || dst_size == 0U) return;
    if(src == NULL) src = "";
    len = strlen(src);
    if(len >= dst_size) len = dst_size - 1U;
    memcpy(dst, src, len);
    dst[len] = '\0';
}

void am_player_set_playlist_pick_cb(am_player_pick_cb_t cb)
{
    g_pick_cb = cb;
}

/* 推迟到事件返回后再切歌:切歌会经 am_player_refresh_ui → lv_obj_clean(playlist_list)
 * 删掉正处理点击的本行,同步执行会 UAF。目标索引存于 g_pending_pick(取最后一次点击)。 */
static void am_playlist_pick_async(void *data)
{
    LV_UNUSED(data);
    if(g_source_kind == AM_SOURCE_RADIO) am_player_play_radio_index(g_pending_pick);
    else                                 am_player_play_local_index(g_pending_pick);
    g_playlist_open = false;                 /* 选完关闭弹层(对齐 mockup) */
    if(g_pick_cb != NULL) g_pick_cb();        /* 通知宿主跳转/刷新正在播放页 */
    am_player_refresh_ui();
}

static void am_playlist_item_click_cb(lv_event_t *e)
{
    g_pending_pick = (size_t)(intptr_t)lv_event_get_user_data(e);
    lv_async_call_cancel(am_playlist_pick_async, NULL);   /* 连点只保留最后一次 */
    lv_async_call(am_playlist_pick_async, NULL);
}

static int32_t am_playlist_content_range(const lv_obj_t *list)
{
    if(list == NULL) return 0;
    return lv_obj_get_scroll_top((lv_obj_t *)list) + lv_obj_get_scroll_bottom((lv_obj_t *)list);
}

static void am_playlist_track_sync_geometry(lv_obj_t *track, lv_obj_t *list)
{
    if(track == NULL || list == NULL) return;

    lv_obj_update_layout(list);
    lv_obj_set_height(track, lv_obj_get_height(list));
    lv_obj_set_width(track, 8);
    lv_obj_align_to(track, list, LV_ALIGN_TOP_RIGHT, -6, 0);
}

static void am_playlist_scrollbar_sync(void)
{
    lv_obj_t *list = g_h.playlist_list;
    lv_obj_t *track = g_h.playlist_scroll_track;
    lv_obj_t *thumb = g_h.playlist_scroll_thumb;
    int32_t viewport_h;
    int32_t scroll_y;
    int32_t content_range;
    int32_t content_h;
    int32_t track_h;
    int32_t thumb_h;
    int32_t track_range;
    int32_t thumb_y;

    if(list == NULL || track == NULL || thumb == NULL) return;

    lv_obj_update_layout(list);
    am_playlist_track_sync_geometry(track, list);
    viewport_h = lv_obj_get_content_height(list);
    content_range = am_playlist_content_range(list);
    content_h = viewport_h + content_range;
    scroll_y = lv_obj_get_scroll_y(list);

    if(content_range <= 0) {
        lv_obj_add_flag(track, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(track, LV_OBJ_FLAG_HIDDEN);
    lv_obj_update_layout(track);
    track_h = lv_obj_get_content_height(track);
    thumb_h = (track_h * viewport_h) / content_h;
    if(thumb_h < 24) thumb_h = 24;
    if(thumb_h > track_h) thumb_h = track_h;

    track_range = track_h - thumb_h;
    thumb_y = (content_range > 0 && track_range > 0)
        ? (track_range * scroll_y) / content_range
        : 0;

    lv_obj_set_height(thumb, thumb_h);
    lv_obj_set_width(thumb, 4);
    lv_obj_align(thumb, LV_ALIGN_TOP_MID, 0, thumb_y);
}

static void am_playlist_list_scroll_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    am_playlist_scrollbar_sync();
}

static void am_playlist_thumb_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *thumb = lv_event_get_target(e);
    lv_obj_t *track = g_h.playlist_scroll_track;
    lv_obj_t *list = g_h.playlist_list;
    lv_indev_t *indev = lv_event_get_indev(e);
    lv_point_t p;
    lv_area_t track_a;
    lv_area_t thumb_a;
    int32_t track_content_top;
    int32_t track_h;
    int32_t thumb_h;
    int32_t track_range;
    int32_t thumb_y;
    int32_t viewport_h;
    int32_t content_range;
    int32_t content_h;
    int32_t scroll_target;

    if(thumb == NULL || track == NULL || list == NULL) return;
    if(indev != NULL) lv_indev_get_point(indev, &p);

    if(code == LV_EVENT_PRESSED) {
        if(indev == NULL) return;
        am_playlist_scrollbar_sync();
        lv_obj_update_layout(thumb);
        lv_obj_get_coords(track, &track_a);
        lv_obj_get_coords(thumb, &thumb_a);
        g_playlist_thumb_dragging = true;
        g_playlist_thumb_track_top = track_a.y1 + lv_obj_get_style_pad_top(track, 0);
        g_playlist_thumb_press_ofs_y = p.y - thumb_a.y1;
        return;
    }

    if(code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        g_playlist_thumb_dragging = false;
        return;
    }

    if(code != LV_EVENT_PRESSING || !g_playlist_thumb_dragging || indev == NULL) return;

    am_playlist_track_sync_geometry(track, list);
    track_h = lv_obj_get_content_height(track);
    thumb_h = lv_obj_get_height(thumb);
    track_range = track_h - thumb_h;
    viewport_h = lv_obj_get_content_height(list);
    content_range = am_playlist_content_range(list);
    content_h = viewport_h + content_range;

    if(track_range <= 0 || content_range <= 0) return;

    lv_obj_get_coords(thumb, &thumb_a);
    track_content_top = g_playlist_thumb_track_top;
    thumb_y = p.y - track_content_top - g_playlist_thumb_press_ofs_y;
    if(thumb_y < 0) thumb_y = 0;
    if(thumb_y > track_range) thumb_y = track_range;

    scroll_target = (thumb_y * content_range) / track_range;
    lv_obj_scroll_to_y(list, scroll_target, LV_ANIM_OFF);
    am_playlist_scrollbar_sync();
}

static const char *am_current_title(void)
{
    if(g_single_file_mode && g_single_file_title[0] != '\0') return g_single_file_title;
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
    if(g_single_file_mode) return "单个文件";
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

    /* 内容指纹未变则跳过重建,保住滚动位置、不吃点击(见 g_pl_* 说明) */
    {
        size_t cur_idx = (g_source_kind == AM_SOURCE_RADIO) ? g_current_radio : g_current_local;
        size_t cur_cnt = (g_source_kind == AM_SOURCE_RADIO) ? g_radio_count : g_local_count;
        if(g_pl_built && g_pl_kind == g_source_kind && g_pl_index == cur_idx && g_pl_count == cur_cnt) {
            return;
        }
        g_pl_built = true;
        g_pl_kind = g_source_kind;
        g_pl_index = cur_idx;
        g_pl_count = cur_cnt;
    }

    lv_obj_clean(g_h.playlist_list);

    if(g_single_file_mode) {
        lv_obj_t *item = lv_obj_create(g_h.playlist_list);
        lv_obj_t *info;

        lv_obj_remove_style_all(item);
        lv_obj_set_size(item, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(item, 8, 0);
        lv_obj_set_style_radius(item, 7, 0);
        lv_obj_set_style_bg_color(item, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(item, 12, 0);
        lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_column(item, 10, 0);
        am_text(item, "1", am_metrics()->f_label, lv_color_hex(0xfa2d48));

        info = lv_obj_create(item);
        lv_obj_remove_style_all(info);
        lv_obj_set_flex_grow(info, 1);
        lv_obj_set_height(info, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(info, LV_FLEX_FLOW_COLUMN);
        lv_obj_clear_flag(info, LV_OBJ_FLAG_CLICKABLE);
        am_text(info, am_current_title(), am_metrics()->f_body, lv_color_hex(0xfa2d48));
        am_text(info, am_current_subtitle(), am_metrics()->f_label, AM_MUTED);
        am_playlist_scrollbar_sync();
        return;
    }

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
        am_playlist_scrollbar_sync();
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
        lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);        /* 点击此行切歌 */
        lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(item, am_playlist_item_click_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);

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
        /* lv_obj 默认带 LV_OBJ_FLAG_CLICKABLE,会截获落在 info 区(行内大部分宽度)的点击,
         * 使 CLICKED 目标变成 info(无回调)而非绑了切歌回调的 item → 点标题区切不了歌。
         * 清掉 info 的 CLICKABLE,让点击命中冒泡回 item。 */
        lv_obj_clear_flag(info, LV_OBJ_FLAG_CLICKABLE);
        am_text(info, g_locals[i].title, am_metrics()->f_body, active ? lv_color_hex(0xfa2d48) : AM_TEXT);
        am_text(info, "本地音频", am_metrics()->f_label, AM_MUTED);
    }

    am_playlist_scrollbar_sync();
}

void am_player_refresh_ui(void)
{
    char cur_buf[16];
    char total_buf[16];
    uint32_t pos = music_player_get_position_ms();
    uint32_t dur = music_player_get_duration_ms();

    if(g_h.title_label != NULL) lv_label_set_text(g_h.title_label, am_current_title());
    if(g_h.subtitle_label != NULL) lv_label_set_text(g_h.subtitle_label, am_current_subtitle());
    if(g_h.play_icon != NULL || g_h.pause_icon != NULL) {
        bool playing = music_player_is_playing();
        if(g_h.play_icon != NULL) {
            if(playing) lv_obj_add_flag(g_h.play_icon, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_clear_flag(g_h.play_icon, LV_OBJ_FLAG_HIDDEN);
        }
        if(g_h.pause_icon != NULL) {
            if(playing) lv_obj_clear_flag(g_h.pause_icon, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_add_flag(g_h.pause_icon, LV_OBJ_FLAG_HIDDEN);
        }
    }
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
    am_playlist_scrollbar_sync();
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
    g_single_file_mode = false;
    g_single_file_path[0] = '\0';
    g_single_file_title[0] = '\0';
    g_pl_built = false;
    g_pl_kind = AM_SOURCE_NONE;
    g_pl_index = (size_t)-1;
    g_pl_count = (size_t)-1;
    if(g_timer != NULL) lv_timer_delete(g_timer);
    g_timer = lv_timer_create(am_player_timer_cb, 120, NULL);
}

void am_player_deinit(void)
{
    lv_async_call_cancel(am_playlist_pick_async, NULL);   /* 防止销毁后待处理的切歌回调解引用已释放数据 */
    if(g_timer != NULL) {
        lv_timer_delete(g_timer);
        g_timer = NULL;
    }
    music_player_deinit();
    memset(&g_h, 0, sizeof(g_h));
    g_source_kind = AM_SOURCE_NONE;
    g_local_engine_ready = false;
    g_single_file_mode = false;
    g_single_file_path[0] = '\0';
    g_single_file_title[0] = '\0';
    g_pl_built = false;
    g_locals = NULL;
    g_local_count = 0U;
    g_radios = NULL;
    g_radio_count = 0U;
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
    if(g_h.playlist_list != NULL) {
        lv_obj_add_event_cb(g_h.playlist_list, am_playlist_list_scroll_cb, LV_EVENT_SCROLL, NULL);
    }
    if(g_h.playlist_scroll_thumb != NULL) {
        lv_obj_add_flag(g_h.playlist_scroll_thumb, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(g_h.playlist_scroll_thumb, LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_clear_flag(g_h.playlist_scroll_thumb, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(g_h.playlist_scroll_thumb, am_playlist_thumb_event_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(g_h.playlist_scroll_thumb, am_playlist_thumb_event_cb, LV_EVENT_PRESSING, NULL);
        lv_obj_add_event_cb(g_h.playlist_scroll_thumb, am_playlist_thumb_event_cb, LV_EVENT_RELEASED, NULL);
        lv_obj_add_event_cb(g_h.playlist_scroll_thumb, am_playlist_thumb_event_cb, LV_EVENT_PRESS_LOST, NULL);
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

    am_player_clear_single_file_mode();
    g_source_kind = AM_SOURCE_LOCAL;
    g_current_local = index;

    /* 一次 init 整份本地列表,之后只 select 切歌(此前每次都 deinit/init,重探测所有时长,过重) */
    if(!g_local_engine_ready) {
        for(i = 0U; i < g_local_count; i++) urls[i] = g_locals[i].path;
        music_player_deinit();
        music_player_init(urls, g_local_count);   /* 固定从第 0 首起播 */
        music_player_set_volume(g_volume);
        g_local_engine_ready = true;
        /* init 固定起播第 0 首,且此刻引擎尚未进入 PLAYING(状态经异步事件才更新),
           故紧跟的 music_player_select 不会自动切歌(player_controller_select 仅在已
           active 时自动播)→ 首次选非 0 曲会停在第 0 首。目标非 0 时显式 select+play
           强制切到目标;目标为 0 时 init 已在播,无需再动(避免同曲重复 change_url)。 */
        if(index != 0U) {
            music_player_select(index);
            music_player_play();
        }
        am_player_refresh_ui();
        return;
    }
    music_player_select(index);   /* 引擎已 active,select 自动切并起播 */
    am_player_refresh_ui();
}

void am_player_play_radio_index(size_t index)
{
    const char *url_list[1];

    if(g_radios == NULL || g_radio_count == 0U || index >= g_radio_count) return;
    am_player_clear_single_file_mode();
    g_source_kind = AM_SOURCE_RADIO;
    g_current_radio = index;
    music_player_deinit();
    g_local_engine_ready = false;   /* 切电台重置了单例,本地需重新 init */
    url_list[0] = g_radios[index].url;
    music_player_init(url_list, 1U);
    music_player_set_volume(g_volume);
    am_player_refresh_ui();
}

void am_player_play_single_file(const char *path, const char *title)
{
    const char *url_list[1];

    if(path == NULL || path[0] == '\0') return;

    g_single_file_mode = true;
    am_copy_text(g_single_file_path, sizeof(g_single_file_path), path);
    am_copy_text(g_single_file_title, sizeof(g_single_file_title),
                 (title != NULL && title[0] != '\0') ? title : path);
    g_source_kind = AM_SOURCE_LOCAL;
    g_current_local = 0U;
    music_player_deinit();
    g_local_engine_ready = false;
    url_list[0] = g_single_file_path;
    music_player_init(url_list, 1U);
    music_player_set_volume(g_volume);
    am_player_refresh_ui();
}

/* 首次交互(播放/上一首/下一首)时若尚未选择任何来源,懒加载本地库并从当前曲开始,
 * 对齐 local_music_demo 的开箱即播:此前传输键仅转调 music_player_*,而引擎未 init
 * (g_controller==NULL)→ 全部 no-op,表现为"播放按钮没反应、没声音"。
 * 返回 true 表示本次调用刚启动播放,调用方无需再做暂停/切换。 */
static bool am_player_ensure_started(void)
{
    if(g_source_kind != AM_SOURCE_NONE) return false;
    if(g_locals == NULL || g_local_count == 0U) return false;
    am_player_play_local_index(g_current_local < g_local_count ? g_current_local : 0U);
    return true;
}

void am_player_toggle_playback(void)
{
    if(am_player_ensure_started()) return;   /* 首次按播放:刚起播即在放,无需再 toggle */
    if(music_player_is_playing()) music_player_pause();
    else music_player_resume();
    am_player_refresh_ui();
}

void am_player_prev(void)
{
    if(am_player_ensure_started()) return;   /* 未选歌时按上一首:先起播本地库 */
    if(g_single_file_mode) {
        am_player_refresh_ui();
        return;
    }
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
    if(am_player_ensure_started()) return;   /* 未选歌时按下一首:先起播本地库 */
    if(g_single_file_mode) {
        am_player_refresh_ui();
        return;
    }
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

uint32_t am_player_track_duration_ms(size_t index)
{
    return music_player_get_track_duration_ms(index);
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

bool am_player_is_single_file_mode(void)
{
    return g_single_file_mode;
}

uint8_t am_player_volume_percent(void)
{
    return g_volume;
}

void am_player_clear_single_file_mode(void)
{
    g_single_file_mode = false;
    g_single_file_path[0] = '\0';
    g_single_file_title[0] = '\0';
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
