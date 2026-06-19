/* main/src/v9_apple_music/am_page_local.c */
#include "am_page_local.h"
#include "am_player.h"
#include "am_widgets.h"
#include "am_theme.h"
#include "am_metrics.h"
#include "stream_player.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

/* lv_event_cb_t wrapper：释放 lv_event_get_user_data 指向的 lv_malloc 块 */
static void am_obj_user_data_free_cb(lv_event_t *e)
{
    lv_free(lv_event_get_user_data(e));
}

/* ─── 共用 helper：构建简单列表行 ──────────────────────────────────── */

static lv_obj_t *make_list_row(lv_obj_t *parent,
                                const char *title, const char *subtitle,
                                const am_metrics_t *m)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(row, 10, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(row, 3, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(row, LV_OPA_20, 0);
    lv_obj_set_style_radius(row, 10, 0);
    lv_obj_set_style_border_width(row, 0, 0);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, title);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
    lv_obj_set_width(lbl, LV_PCT(100));
    if(m->f_body) lv_obj_set_style_text_font(lbl, m->f_body, 0);

    if(subtitle && subtitle[0]) {
        lv_obj_t *sub = lv_label_create(row);
        lv_label_set_text(sub, subtitle);
        lv_label_set_long_mode(sub, LV_LABEL_LONG_DOT);
        lv_obj_set_width(sub, LV_PCT(100));
        lv_obj_set_style_text_opa(sub, LV_OPA_60, 0);
        if(m->f_label) lv_obj_set_style_text_font(sub, m->f_label, 0);
    }
    return row;
}

/* ─── 本地页 ────────────────────────────────────────────────────────── */

typedef struct {
    char **urls;
    size_t count;
} local_page_ctx_t;

typedef struct {
    local_page_ctx_t *page;
    size_t            index;
    /* 注意：page 的生命周期必须长于 row；row 只应通过删除 page 来触发，
     * page 的 DELETE 回调先于子对象 DELETE 执行，不要在 page 之外单独删除 row。 */
} local_item_ctx_t;

static void local_page_delete_cb(lv_event_t *e)
{
    local_page_ctx_t *ctx = (local_page_ctx_t *)lv_event_get_user_data(e);
    size_t i;
    for(i = 0; i < ctx->count; i++) free(ctx->urls[i]);
    free(ctx->urls);
    /* urls[i] 由 strdup(malloc) 分配用 free 释放；ctx 由 lv_malloc 分配用 lv_free 释放。
     * PC 模拟器下 lv_malloc == malloc，混用安全；移植到自定义堆目标时需重新评估。 */
    lv_free(ctx);
}

static void local_item_click_cb(lv_event_t *e)
{
    local_item_ctx_t *ctx = (local_item_ctx_t *)lv_event_get_user_data(e);
    am_player_load_local((const char **)ctx->page->urls, ctx->page->count, ctx->index);
}

static int cmp_str(const void *a, const void *b)
{
    return strcmp(*(const char * const *)a, *(const char * const *)b);
}

lv_obj_t *am_page_local_create(lv_obj_t *content_parent, const am_config_t *cfg)
{
    const am_metrics_t *m = am_metrics();

    lv_obj_t *page = lv_obj_create(content_parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(page, m->page_gap, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    /* section title */
    am_section_title(page, "本地音乐");

    if(!cfg || !cfg->local_dir[0]) {
        lv_obj_t *ph = lv_label_create(page);
        lv_label_set_text(ph, "请在 am_config.json 中配置 local.dir");
        return page;
    }

    DIR *d = opendir(cfg->local_dir);
    if(!d) {
        lv_obj_t *ph = lv_label_create(page);
        lv_label_set_text(ph, "目录不存在或无法读取");
        return page;
    }

    char **file_urls = NULL;
    size_t file_count = 0, file_cap = 0;
    struct dirent *entry;

    while((entry = readdir(d)) != NULL) {
        char full[1024];
        if(entry->d_name[0] == '.') continue;
        snprintf(full, sizeof(full), "%s/%s", cfg->local_dir, entry->d_name);
        if(!stream_player_is_supported_local_audio_file(full)) continue;
        if(file_count >= file_cap) {
            size_t new_cap = (file_cap == 0) ? 16 : file_cap * 2;
            char **tmp = (char **)realloc(file_urls, new_cap * sizeof(char *));
            if(!tmp) break;
            file_urls = tmp;
            file_cap  = new_cap;
        }
        file_urls[file_count] = strdup(full);
        if(!file_urls[file_count]) break;  /* OOM 时中止，避免 NULL 空洞 */
        file_count++;
    }
    closedir(d);

    if(file_count == 0) {
        free(file_urls);
        lv_obj_t *ph = lv_label_create(page);
        lv_label_set_text(ph, "暂无支持的音频文件");
        return page;
    }

    qsort(file_urls, file_count, sizeof(char *), cmp_str);

    local_page_ctx_t *page_ctx = (local_page_ctx_t *)lv_malloc(sizeof(local_page_ctx_t));
    page_ctx->urls  = file_urls;
    page_ctx->count = file_count;
    lv_obj_add_event_cb(page, local_page_delete_cb, LV_EVENT_DELETE, page_ctx);

    size_t i;
    for(i = 0; i < file_count; i++) {
        char copy[512];
        strncpy(copy, file_urls[i], sizeof(copy) - 1);
        copy[sizeof(copy) - 1] = '\0';
        char *b = basename(copy);
        char  name[128];
        strncpy(name, b, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        char *dot = strrchr(name, '.');
        if(dot) *dot = '\0';

        lv_obj_t *row = make_list_row(page, name, file_urls[i], m);

        local_item_ctx_t *ictx = (local_item_ctx_t *)lv_malloc(sizeof(local_item_ctx_t));
        ictx->page  = page_ctx;
        ictx->index = i;
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(row, local_item_click_cb,      LV_EVENT_CLICKED, ictx);
        lv_obj_add_event_cb(row, am_obj_user_data_free_cb, LV_EVENT_DELETE,  ictx);
    }

    return page;
}

/* ─── 广播页 ────────────────────────────────────────────────────────── */

typedef struct {
    char url[512];
    char title[128];
} radio_item_ctx_t;

static void radio_item_click_cb(lv_event_t *e)
{
    radio_item_ctx_t *ctx = (radio_item_ctx_t *)lv_event_get_user_data(e);
    am_player_play_stream(ctx->url, ctx->title);
}

lv_obj_t *am_page_radio_create(lv_obj_t *content_parent, const am_config_t *cfg)
{
    const am_metrics_t *m = am_metrics();

    lv_obj_t *page = lv_obj_create(content_parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(page, m->page_gap, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    am_section_title(page, "广播频道");

    if(!cfg || cfg->radio_count == 0) {
        lv_obj_t *ph = lv_label_create(page);
        lv_label_set_text(ph, "请在 am_config.json 中配置 radio 列表");
        return page;
    }

    int i;
    for(i = 0; i < cfg->radio_count; i++) {
        lv_obj_t *row = make_list_row(page,
                                      cfg->radio[i].title,
                                      cfg->radio[i].subtitle,
                                      m);

        radio_item_ctx_t *ictx = (radio_item_ctx_t *)lv_malloc(sizeof(radio_item_ctx_t));
        strncpy(ictx->url,   cfg->radio[i].url,   sizeof(ictx->url)   - 1);
        strncpy(ictx->title, cfg->radio[i].title, sizeof(ictx->title) - 1);
        ictx->url[sizeof(ictx->url) - 1]     = '\0';
        ictx->title[sizeof(ictx->title) - 1] = '\0';

        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(row, radio_item_click_cb,      LV_EVENT_CLICKED, ictx);
        lv_obj_add_event_cb(row, am_obj_user_data_free_cb, LV_EVENT_DELETE,  ictx);
    }

    return page;
}
