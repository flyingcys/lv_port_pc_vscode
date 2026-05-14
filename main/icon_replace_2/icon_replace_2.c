#include "lvgl.h"
#include "icon_replace_2.h"
#include "icon_replace_2_assets.h"
#include "icon_replace_2_desktop.h"
#include "icon_replace_2_layout.h"
#include "icon_replace_2_page_config.h"
#include "icon_replace_2_top_bar.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PAGE0_ICON_COUNT 13
#define PAGE1_ICON_COUNT 14
#define PAGE2_ICON_COUNT 15

typedef struct {
    lv_obj_t * icon;
    int page;
    int index;
} icon_type;

static const int page_icon_count_init[PAGE_COUNT] = {
    PAGE0_ICON_COUNT,
    PAGE1_ICON_COUNT,
    PAGE2_ICON_COUNT,
};

static int page_icon_count[PAGE_COUNT];
static icon_type icons[PAGE_COUNT][ICON_SLOT_COUNT];
static int offsetx;
static int offsety;
static int touching;
static int touch_time_count;
static int old_index;
static int new_index;
static int icon_shake;
static int border_lefttest_count;
static int border_righttest_count;
static lv_obj_t * page[PAGE_COUNT];
static lv_obj_t * screen;
static icon_replace_2_desktop_t * desktop;
static icon_replace_2_top_bar_t * top_bar;
static int drag_page;

static int clamp_index(int index);
static void clear_runtime_object_refs(void);
static void desktop_page_changed_cb(uint32_t page_index, void * user_data);
static void top_bar_deleted_cb(lv_event_t * e);
static icon_type * icon_get_meta(lv_obj_t * obj);
static void icon_set_meta(lv_obj_t * obj, icon_type * meta);
static int index_by_xy(const lv_point_t * click_point);
static int x_by_index(int index);
static int y_by_index(int index);
static void touching_cb(lv_event_t * e);
static void released_cb(lv_event_t * e);
static void set_x_cb(void * var, int32_t v);
static void set_y_cb(void * var, int32_t v);
static void icon_shake_cb(void * var, int32_t v);

void icon_replace_demo_2(void)
{
    int i;
    int j;

    memcpy(page_icon_count, page_icon_count_init, sizeof(page_icon_count));
    memset(icons, 0, sizeof(icons));

    offsetx = 0;
    offsety = 0;
    touching = 0;
    touch_time_count = 0;
    old_index = 0;
    new_index = 0;
    icon_shake = 0;
    border_lefttest_count = 0;
    border_righttest_count = 0;
    drag_page = 0;

    if(top_bar != NULL) {
        icon_replace_2_top_bar_destroy(top_bar);
        top_bar = NULL;
    }

    if(desktop != NULL) {
        icon_replace_2_desktop_destroy(desktop);
        desktop = NULL;
    }

    clear_runtime_object_refs();
    lv_obj_clean(lv_screen_active());

    desktop = icon_replace_2_desktop_create(lv_screen_active(), desktop_page_changed_cb, NULL);
    if(desktop == NULL) {
        return;
    }

    screen = icon_replace_2_desktop_get_tileview(desktop);

    for(j = 0; j < PAGE_COUNT; j++) {
        page[j] = icon_replace_2_desktop_get_page(desktop, j);

        for(i = 0; i < ICON_SLOT_COUNT; i++) {
            icons[j][i].icon = NULL;
            icons[j][i].page = j;
            icons[j][i].index = i;
        }
    }

    for(j = 0; j < PAGE_COUNT; j++) {
        for(i = 0; i < page_icon_count[j]; i++) {
            lv_obj_t * icon_obj = lv_image_create(page[j]);
            const lv_image_dsc_t * image = icon_replace_2_assets[i % icon_replace_2_asset_count];

            icons[j][i].icon = icon_obj;

            lv_image_set_src(icon_obj, image);
            lv_image_set_scale(icon_obj, LV_SCALE_NONE * 2);
            lv_obj_set_pos(icon_obj, x_by_index(i), y_by_index(i));
            lv_obj_add_flag(icon_obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(icon_obj, released_cb, LV_EVENT_RELEASED, NULL);
            lv_obj_add_event_cb(icon_obj, touching_cb, LV_EVENT_PRESSING, NULL);
            icon_set_meta(icon_obj, &icons[j][i]);
        }
    }

    top_bar = icon_replace_2_top_bar_create(lv_screen_active());
    if(top_bar == NULL) {
        return;
    }

    lv_obj_add_event_cb(icon_replace_2_top_bar_get_root(top_bar), top_bar_deleted_cb, LV_EVENT_DELETE, NULL);
    icon_replace_2_top_bar_apply(top_bar, icon_replace_2_get_page_config(0));
    icon_replace_2_top_bar_set_wifi_state(top_bar, WIFI_STATE_NORMAL);
    icon_replace_2_top_bar_start_minute_timer(top_bar);
}

static void clear_runtime_object_refs(void)
{
    uint32_t page_index;

    screen = NULL;

    for(page_index = 0; page_index < PAGE_COUNT; page_index++) {
        page[page_index] = NULL;
    }
}

static void desktop_page_changed_cb(uint32_t page_index, void * user_data)
{
    LV_UNUSED(user_data);

    if(top_bar == NULL) {
        return;
    }

    icon_replace_2_top_bar_apply(top_bar, icon_replace_2_get_page_config(page_index));
}

static void top_bar_deleted_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    top_bar = NULL;
}

static int x_by_index(int index)
{
    return (index % ICON_MAX_COL) * ICON_X_DISTANCE + ICON_START_X;
}

static int y_by_index(int index)
{
    return (index / ICON_MAX_COL) * ICON_Y_DISTANCE + ICON_START_Y;
}

static int clamp_index(int index)
{
    if(index < 0) {
        return 0;
    }

    if(index >= ICON_SLOT_COUNT) {
        return ICON_SLOT_COUNT - 1;
    }

    return index;
}

static icon_type * icon_get_meta(lv_obj_t * obj)
{
    return (icon_type *)lv_obj_get_user_data(obj);
}

static void icon_set_meta(lv_obj_t * obj, icon_type * meta)
{
    lv_obj_set_user_data(obj, meta);
}

static int index_by_xy(const lv_point_t * click_point)
{
    int index = (click_point->x - ICON_START_X) / ICON_X_DISTANCE +
                ((click_point->y - ICON_START_Y) / ICON_Y_DISTANCE) * ICON_MAX_COL;

    return clamp_index(index);
}

static void touching_cb(lv_event_t * e)
{
    int i;
    int j;
    lv_point_t screen_point;
    lv_point_t local_point;
    lv_obj_t * target = lv_event_get_target(e);
    icon_type * meta = icon_get_meta(target);

    if(meta == NULL) {
        return;
    }

    lv_obj_move_foreground(target);

    if(touching == 0) {
        lv_indev_get_point(lv_indev_get_act(), &screen_point);
        if(!icon_replace_2_desktop_screen_to_local(desktop, &screen_point, &local_point)) {
            return;
        }

        offsetx = local_point.x - lv_obj_get_x(target);
        offsety = local_point.y - lv_obj_get_y(target);
        touching = 1;
        drag_page = meta->page;
        return;
    }

    if(icon_shake) {
        lv_indev_get_point(lv_indev_get_act(), &screen_point);
        if(!icon_replace_2_desktop_screen_to_local(desktop, &screen_point, &local_point)) {
            return;
        }

        lv_obj_set_pos(target, local_point.x - offsetx, local_point.y - offsety);

        if(local_point.x < ICON_SIZE / 2) {
            border_lefttest_count++;
        }
        else if(local_point.x > PAGE_WIDTH - ICON_SIZE / 2) {
            border_righttest_count++;
        }
        else {
            border_lefttest_count = 0;
            border_righttest_count = 0;
        }

        if(border_lefttest_count > 70) {
            drag_page = drag_page > 0 ? drag_page - 1 : 0;
            lv_obj_set_parent(target, page[drag_page]);
            lv_tileview_set_tile(screen, page[drag_page], LV_ANIM_ON);
            border_lefttest_count = 0;
        }

        if(border_righttest_count > 70) {
            drag_page = drag_page < PAGE_COUNT - 1 ? drag_page + 1 : PAGE_COUNT - 1;
            lv_obj_set_parent(target, page[drag_page]);
            lv_tileview_set_tile(screen, page[drag_page], LV_ANIM_ON);
            border_righttest_count = 0;
        }

        return;
    }

    touch_time_count++;
    if(touch_time_count <= 70) {
        return;
    }

    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    if(icon_shake) {
        return;
    }

    icon_shake = 1;

    for(j = 0; j < PAGE_COUNT; j++) {
        for(i = 0; i < page_icon_count[j]; i++) {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, icons[j][i].icon);
            lv_anim_set_exec_cb(&a, icon_shake_cb);
            lv_anim_set_time(&a, 100);
            lv_anim_set_delay(&a, rand() % 100);
            lv_anim_set_playback_time(&a, 100);
            lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
            lv_anim_set_values(&a, -50, 50);
            lv_anim_start(&a);
        }
    }
}

static void released_cb(lv_event_t * e)
{
    int i;
    int j;
    lv_obj_t * target = lv_event_get_target(e);
    icon_type * meta = icon_get_meta(target);

    if(meta == NULL) {
        return;
    }

    old_index = meta->index;

    touch_time_count = 0;
    touching = 0;
    border_lefttest_count = 0;
    border_righttest_count = 0;

    lv_obj_add_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_anim_del(NULL, icon_shake_cb);

    for(j = 0; j < PAGE_COUNT; j++) {
        for(i = 0; i < page_icon_count[j]; i++) {
            lv_image_set_rotation(icons[j][i].icon, 0);
        }
    }

    if(icon_shake && drag_page == meta->page) {
        lv_point_t screen_point;
        lv_point_t local_point;

        lv_indev_get_point(lv_indev_get_act(), &screen_point);
        if(!icon_replace_2_desktop_screen_to_local(desktop, &screen_point, &local_point)) {
            return;
        }

        icon_shake = 0;

        meta->icon = NULL;
        new_index = index_by_xy(&local_point);
        if(new_index > page_icon_count[drag_page] - 1) {
            new_index = page_icon_count[drag_page] - 1;
        }

        j = drag_page;
        if(new_index < old_index) {
            for(i = old_index; i > new_index; i--) {
                icons[j][i].icon = icons[j][i - 1].icon;
                icon_set_meta(icons[j][i].icon, &icons[j][i]);

                lv_anim_t a;
                lv_anim_t a1;

                lv_anim_init(&a);
                lv_anim_set_var(&a, icons[j][i - 1].icon);
                lv_anim_set_exec_cb(&a, set_x_cb);
                lv_anim_set_time(&a, 300);
                lv_anim_set_values(&a, x_by_index(i - 1), x_by_index(i));
                lv_anim_start(&a);

                lv_anim_init(&a1);
                lv_anim_set_var(&a1, icons[j][i - 1].icon);
                lv_anim_set_exec_cb(&a1, set_y_cb);
                lv_anim_set_time(&a1, 300);
                lv_anim_set_values(&a1, y_by_index(i - 1), y_by_index(i));
                lv_anim_start(&a1);
            }

            icons[j][new_index].icon = target;
            icon_set_meta(target, &icons[j][new_index]);
            lv_obj_set_pos(target, x_by_index(new_index), y_by_index(new_index));
            return;
        }

        for(i = old_index; i < new_index; i++) {
            icons[j][i].icon = icons[j][i + 1].icon;
            icon_set_meta(icons[j][i].icon, &icons[j][i]);

            lv_anim_t a;
            lv_anim_t a1;

            lv_anim_init(&a);
            lv_anim_set_var(&a, icons[j][i].icon);
            lv_anim_set_exec_cb(&a, set_x_cb);
            lv_anim_set_time(&a, 300);
            lv_anim_set_values(&a, x_by_index(i + 1), x_by_index(i));
            lv_anim_start(&a);

            lv_anim_init(&a1);
            lv_anim_set_var(&a1, icons[j][i].icon);
            lv_anim_set_exec_cb(&a1, set_y_cb);
            lv_anim_set_time(&a1, 300);
            lv_anim_set_values(&a1, y_by_index(i + 1), y_by_index(i));
            lv_anim_start(&a1);
        }

        icons[j][new_index].icon = target;
        icon_set_meta(target, &icons[j][new_index]);
        lv_obj_set_pos(target, x_by_index(new_index), y_by_index(new_index));
        return;
    }

    if(icon_shake && drag_page != meta->page) {
        lv_point_t screen_point;
        lv_point_t local_point;

        lv_indev_get_point(lv_indev_get_act(), &screen_point);
        if(!icon_replace_2_desktop_screen_to_local(desktop, &screen_point, &local_point)) {
            return;
        }

        icon_shake = 0;
        new_index = index_by_xy(&local_point);

        if(icons[drag_page][ICON_SLOT_COUNT - 1].icon == NULL) {
            j = drag_page;
            for(i = page_icon_count[drag_page]; i > new_index; i--) {
                icons[j][i].icon = icons[j][i - 1].icon;
                icon_set_meta(icons[j][i].icon, &icons[j][i]);

                lv_anim_t a;
                lv_anim_t a1;

                lv_anim_init(&a);
                lv_anim_set_var(&a, icons[j][i - 1].icon);
                lv_anim_set_exec_cb(&a, set_x_cb);
                lv_anim_set_time(&a, 300);
                lv_anim_set_values(&a, x_by_index(i - 1), x_by_index(i));
                lv_anim_start(&a);

                lv_anim_init(&a1);
                lv_anim_set_var(&a1, icons[j][i - 1].icon);
                lv_anim_set_exec_cb(&a1, set_y_cb);
                lv_anim_set_time(&a1, 300);
                lv_anim_set_values(&a1, y_by_index(i - 1), y_by_index(i));
                lv_anim_start(&a1);
            }

            j = meta->page;
            for(i = old_index; i < page_icon_count[j] - 1; i++) {
                icons[j][i].icon = icons[j][i + 1].icon;
                icon_set_meta(icons[j][i].icon, &icons[j][i]);
                lv_obj_set_pos(icons[j][i].icon, x_by_index(i), y_by_index(i));
            }

            icons[j][page_icon_count[j] - 1].icon = NULL;

            page_icon_count[drag_page]++;
            page_icon_count[meta->page]--;
            icons[drag_page][new_index].icon = target;
            icon_set_meta(target, &icons[drag_page][new_index]);
            lv_obj_set_pos(target, x_by_index(new_index), y_by_index(new_index));
            return;
        }

        {
            int old_page = meta->page;
            int old_page_index = meta->index;

            icons[old_page][old_page_index].icon = icons[drag_page][new_index].icon;

            lv_obj_set_parent(icons[old_page][old_page_index].icon, page[old_page]);
            lv_obj_set_pos(icons[old_page][old_page_index].icon,
                           x_by_index(old_page_index),
                           y_by_index(old_page_index));
            icon_set_meta(icons[drag_page][new_index].icon, &icons[drag_page][new_index]);
            icon_set_meta(icons[old_page][old_page_index].icon, &icons[old_page][old_page_index]);

            icons[drag_page][new_index].icon = target;
            icon_set_meta(target, &icons[drag_page][new_index]);
            lv_obj_set_pos(target, x_by_index(new_index), y_by_index(new_index));
            return;
        }
    }
}

static void set_x_cb(void * var, int32_t v)
{
    lv_obj_set_x((lv_obj_t *)var, v);
}

static void set_y_cb(void * var, int32_t v)
{
    lv_obj_set_y((lv_obj_t *)var, v);
}

static void icon_shake_cb(void * var, int32_t v)
{
    lv_image_set_rotation((lv_obj_t *)var, v);
}
