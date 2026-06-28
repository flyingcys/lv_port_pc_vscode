#include "am_page_list.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "am_icons.h"
#include "am_metrics.h"
#include "am_theme.h"
#include "am_widgets.h"

static const am_metrics_t *am_list_metrics(void)
{
    return am_metrics();
}

typedef struct {
    size_t index;
    void *user;
    am_radio_select_cb_t on_radio;
    am_local_select_cb_t on_local;
} am_row_ctx_t;

static void am_row_click_cb(lv_event_t *e)
{
    am_row_ctx_t *ctx = (am_row_ctx_t *)lv_event_get_user_data(e);
    if(ctx == NULL) return;
    if(ctx->on_radio != NULL) ctx->on_radio(ctx->index, ctx->user);
    if(ctx->on_local != NULL) ctx->on_local(ctx->index, ctx->user);
}

static void am_row_ctx_free_cb(lv_event_t *e)
{
    lv_free(lv_event_get_user_data(e));
}

static lv_obj_t *am_list_row_base(lv_obj_t *parent, bool active)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_radius(row, 8, 0);
    lv_obj_set_style_pad_all(row, 8, 0);
    lv_obj_set_style_pad_column(row, 12, 0);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(row, active ? 12 : LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    return row;
}

static void am_bind_row(lv_obj_t *row, size_t index, void *user,
                        am_radio_select_cb_t on_radio, am_local_select_cb_t on_local)
{
    am_row_ctx_t *ctx = lv_malloc(sizeof(*ctx));
    if(ctx == NULL) return;
    ctx->index = index;
    ctx->user = user;
    ctx->on_radio = on_radio;
    ctx->on_local = on_local;
    lv_obj_add_event_cb(row, am_row_click_cb, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(row, am_row_ctx_free_cb, LV_EVENT_DELETE, ctx);
}

static lv_obj_t *am_build_list_root(lv_obj_t *parent, const char *title)
{
    const am_metrics_t *m = am_metrics();
    lv_obj_t *root = lv_obj_create(parent);
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_top(root, 14, 0);
    lv_obj_set_style_pad_left(root, 24, 0);
    lv_obj_set_style_pad_right(root, 24, 0);
    lv_obj_set_style_pad_bottom(root, 10, 0);
    lv_obj_set_style_pad_row(root, 10, 0);
    lv_obj_set_scroll_dir(root, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_AUTO);

    am_text(root, title, m->f_h2, AM_TEXT);
    return root;
}

lv_obj_t *am_page_list_build_radio(lv_obj_t *parent,
                                   const am_radio_item_t *items,
                                   size_t count,
                                   size_t current_index,
                                   am_radio_select_cb_t on_select,
                                   void *user)
{
    const am_metrics_t *m = am_metrics();
    lv_obj_t *root = am_build_list_root(parent, "广播电台");
    size_t i;

    for(i = 0; i < count; i++) {
        lv_obj_t *row = am_list_row_base(root, i == current_index);
        lv_obj_t *art;
        lv_obj_t *info;
        char sub[96];

        am_bind_row(row, i, user, on_select, NULL);

        art = lv_obj_create(row);
        lv_obj_remove_style_all(art);
        lv_obj_set_size(art, 42, 42);
        lv_obj_set_style_radius(art, 8, 0);
        lv_obj_set_style_bg_color(art, lv_color_hex(0xfa2d48), 0);
        lv_obj_set_style_bg_opa(art, LV_OPA_COVER, 0);
        lv_obj_clear_flag(art, LV_OBJ_FLAG_SCROLLABLE);
        {
            lv_obj_t *icon = am_text(art, AM_ICON_RADIO, m->f_icon, AM_WHITE);
            lv_obj_center(icon);
        }

        info = lv_obj_create(row);
        lv_obj_remove_style_all(info);
        lv_obj_set_flex_grow(info, 1);
        lv_obj_set_height(info, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(info, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(info, 2, 0);
        lv_obj_clear_flag(info, LV_OBJ_FLAG_SCROLLABLE);

        {
            lv_obj_t *name = am_text(info, items[i].title, m->f_body,
                                     (i == current_index) ? lv_color_hex(0xfa2d48) : AM_TEXT);
            if(i == current_index) lv_obj_set_style_text_font(name, m->f_strong, 0);
            lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
            lv_obj_set_width(name, LV_PCT(100));
        }

        snprintf(sub, sizeof(sub), "缓存 %ums", (unsigned)items[i].network_cache_ms);
        {
            lv_obj_t *label = am_text(info, sub, m->f_label, AM_MUTED);
            lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
            lv_obj_set_width(label, LV_PCT(100));
        }

        am_text(row, "LIVE", m->f_label, lv_color_hex(0xfa2d48));
    }

    return root;
}

lv_obj_t *am_page_list_build_favorites(lv_obj_t *parent,
                                       const am_local_item_t *items,
                                       size_t count,
                                       size_t current_index,
                                       am_local_select_cb_t on_select,
                                       void *user)
{
    const am_metrics_t *m = am_metrics();
    lv_obj_t *root = am_build_list_root(parent, "我的收藏");
    size_t i;

    for(i = 0; i < count; i++) {
        lv_obj_t *row;
        lv_obj_t *art;
        lv_obj_t *info;

        if(!items[i].favorite) continue;

        row = am_list_row_base(root, i == current_index);
        am_bind_row(row, i, user, NULL, on_select);

        art = lv_obj_create(row);
        lv_obj_remove_style_all(art);
        lv_obj_set_size(art, 42, 42);
        lv_obj_set_style_radius(art, 8, 0);
        lv_obj_set_style_bg_color(art, lv_color_hex(0x1d1d1f), 0);
        lv_obj_set_style_bg_opa(art, LV_OPA_COVER, 0);
        {
            lv_obj_t *icon = am_text(art, AM_ICON_HEART, m->f_icon, AM_WHITE);
            lv_obj_center(icon);
        }

        info = lv_obj_create(row);
        lv_obj_remove_style_all(info);
        lv_obj_set_flex_grow(info, 1);
        lv_obj_set_height(info, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(info, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(info, 2, 0);
        lv_obj_clear_flag(info, LV_OBJ_FLAG_SCROLLABLE);

        {
            lv_obj_t *name = am_text(info, items[i].title, m->f_body,
                                     (i == current_index) ? lv_color_hex(0xfa2d48) : AM_TEXT);
            if(i == current_index) lv_obj_set_style_text_font(name, m->f_strong, 0);
            lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
            lv_obj_set_width(name, LV_PCT(100));
        }
        {
            lv_obj_t *sub = am_text(info, items[i].path, m->f_label, AM_MUTED);
            lv_label_set_long_mode(sub, LV_LABEL_LONG_DOT);
            lv_obj_set_width(sub, LV_PCT(100));
        }

        am_text(row, AM_ICON_HEART, m->f_label, lv_color_hex(0xfa2d48));
    }

    return root;
}
