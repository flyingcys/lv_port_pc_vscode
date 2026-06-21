#include "lvgl.h"
#include "desktop.h"
#include "desktop_data.h"
#include "desktop_home.h"
#include "desktop_metrics.h"
#include "desktop_page_config.h"
#include "desktop_theme.h"
#include "desktop_top_bar.h"
#include "desktop_widgets.h"
#include "desktop_panels.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* -----------------------------------------------------------------------
 * 边缘手势感应带
 * 顶部条：按下后跟手下滑 → 实时拉出控制中心，松手按位置/甩动吸附
 * 底部条：按下后跟手上滑 → 实时拉出通知中心，松手按位置/甩动吸附
 * （吸附阈值在 panels 模块的 desktop_panel_snap_open 内处理）
 * ----------------------------------------------------------------------- */
#define EDGE_SENSOR_H   28      /* 感应带高度（px）；勿超过最小 top_bar_h（480x272=28） */

LV_IMAGE_DECLARE(img_wallpaper_800x480);
LV_IMAGE_DECLARE(img_wallpaper_640x480);
LV_IMAGE_DECLARE(img_wallpaper_480x272);

static const lv_image_dsc_t * wallpaper_for(const desktop_metrics_t * m)
{
    if(m->screen_w == 640) return &img_wallpaper_640x480;
    if(m->screen_w == 480) return &img_wallpaper_480x272;
    return &img_wallpaper_800x480;
}

typedef struct {
    lv_obj_t * icon;
    int page;
    int index;
} icon_type;

/* page_0: 锁屏 (0 个 app); page_1: 10 个 app; page_2: 2 个 app */
static const int page_icon_count_init[DESKTOP_PAGE_COUNT] = { 0, 10, 2 };

static int page_icon_count[DESKTOP_PAGE_COUNT];
static icon_type icons[DESKTOP_PAGE_COUNT][DESKTOP_SLOT_COUNT];
static int offsetx;
static int offsety;
static int touching;
static int touch_time_count;
static int old_index;
static int new_index;
static int icon_shake;
static int border_lefttest_count;
static int border_righttest_count;
static lv_obj_t * page[DESKTOP_PAGE_COUNT];
static lv_obj_t * screen;
static desktop_home_t * desktop;
static desktop_top_bar_t * top_bar;
static lv_obj_t * pager_dots;
static int drag_page;
static desktop_panels_t * panels;

/* 边缘感应条：挂在 screen_active，跟随 lv_obj_clean 自动回收，无残留风险 */
static lv_obj_t * edge_top;     /* 顶部感应条 */
static lv_obj_t * edge_bot;     /* 底部感应条 */
/* 按下时记录起始 Y（屏幕坐标），用于判断位移方向与幅度 */
static int32_t    edge_top_press_y;
static int32_t    edge_bot_press_y;
/* 边缘条跟手拖拽进行中标志（避免无 begin 的 pressing/released 误触发） */
static int        edge_top_dragging;
static int        edge_bot_dragging;

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
/* 边缘感应条事件 */
static void edge_top_pressed_cb(lv_event_t * e);
static void edge_top_pressing_cb(lv_event_t * e);
static void edge_bot_pressed_cb(lv_event_t * e);
static void edge_bot_pressing_cb(lv_event_t * e);
static void edge_top_released_cb(lv_event_t * e);
static void edge_bot_released_cb(lv_event_t * e);

void desktop_run(void)
{
    int i;
    int j;

    memcpy(page_icon_count, page_icon_count_init, sizeof(page_icon_count));
    page_icon_count[0] = 0;   /* page_0 为锁屏页，不放 app 图标 */
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
        desktop_top_bar_destroy(top_bar);
        top_bar = NULL;
    }

    if(desktop != NULL) {
        desktop_home_destroy(desktop);
        desktop = NULL;
    }

    if(panels != NULL) {
        desktop_panels_destroy(panels);
        panels = NULL;
    }

    clear_runtime_object_refs();
    lv_obj_clean(lv_screen_active());

    desktop = desktop_home_create(lv_screen_active(), desktop_page_changed_cb, NULL);
    if(desktop == NULL) {
        return;
    }

    screen = desktop_home_get_tileview(desktop);

    for(j = 0; j < DESKTOP_PAGE_COUNT; j++) {
        page[j] = desktop_home_get_page(desktop, j);

        for(i = 0; i < DESKTOP_SLOT_COUNT; i++) {
            icons[j][i].icon = NULL;
            icons[j][i].page = j;
            icons[j][i].index = i;
        }
    }

    /* 铺全屏壁纸作最底层背景 */
    {
        const desktop_metrics_t * m = desktop_metrics();
        lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), 0);
        lv_obj_t * wp = lv_image_create(lv_screen_active());
        lv_image_set_src(wp, wallpaper_for(m));
        lv_obj_set_pos(wp, 0, 0);
        desktop_make_decorative(wp);
        lv_obj_move_background(wp);
    }

    /* 让 tileview 与各 page 背景透明，使壁纸透出 */
    lv_obj_set_style_bg_opa(desktop_home_get_tileview(desktop), LV_OPA_TRANSP, 0);
    for(j = 0; j < DESKTOP_PAGE_COUNT; j++) {
        lv_obj_t * pg = desktop_home_get_page(desktop, j);
        if(pg) lv_obj_set_style_bg_opa(pg, LV_OPA_TRANSP, 0);
    }

    {
        /* 按 desktop_apps[] 的 page 字段把真实瓷砖填入对应页 */
        const int32_t row_h = desktop_grid_row_h();
        int slot_k[DESKTOP_PAGE_COUNT];   /* 各页已填槽位计数 */
        for(j = 0; j < DESKTOP_PAGE_COUNT; j++) slot_k[j] = 0;

        for(i = 0; i < (int)desktop_app_count; i++) {
            int pg_idx = (int)desktop_apps[i].page;  /* 1 或 2 */
            if(pg_idx < 0 || pg_idx >= DESKTOP_PAGE_COUNT) continue;
            int k = slot_k[pg_idx];
            if(k >= DESKTOP_SLOT_COUNT) continue;   /* 超出槽位则跳过 */

            lv_obj_t * tile = desktop_widget_app_tile(page[pg_idx], &desktop_apps[i]);
            lv_obj_set_size(tile, desktop_grid_col_w(), row_h);
            lv_obj_set_pos(tile, x_by_index(k), y_by_index(k));
            lv_obj_add_event_cb(tile, released_cb, LV_EVENT_RELEASED, NULL);
            lv_obj_add_event_cb(tile, touching_cb, LV_EVENT_PRESSING, NULL);

            icons[pg_idx][k].icon = tile;
            icon_set_meta(tile, &icons[pg_idx][k]);

            slot_k[pg_idx]++;
        }
    }

    /* page_0 锁屏：居中大时钟 */
    {
        const desktop_metrics_t * mm = desktop_metrics();
        lv_obj_t * big = lv_label_create(page[0]);
        lv_obj_set_style_text_font(big, mm->font_big_clock, 0);
        lv_obj_set_style_text_color(big, desktop_theme()->text_primary, 0);
        lv_label_set_text(big, "09:41");
        lv_obj_center(big);
        desktop_make_decorative(big);
    }

    top_bar = desktop_top_bar_create(lv_screen_active());
    if(top_bar == NULL) {
        return;
    }

    lv_obj_add_event_cb(desktop_top_bar_get_root(top_bar), top_bar_deleted_cb, LV_EVENT_DELETE, NULL);
    desktop_top_bar_apply(top_bar, desktop_get_page_config(0));
    desktop_top_bar_set_wifi_state(top_bar, WIFI_STATE_NORMAL);
    desktop_top_bar_start_minute_timer(top_bar);

    /* 底部分页圆点：仅代表 2 个 app 页（page_1/2），锁屏页隐藏 */
    pager_dots = desktop_widget_dots(lv_screen_active(), 2, 0);
    lv_obj_align(pager_dots, LV_ALIGN_BOTTOM_MID, 0, -6);
    lv_obj_add_flag(pager_dots, LV_OBJ_FLAG_HIDDEN);   /* 初始在 page_0 锁屏，先隐藏 */

    /* 截图钩子：按 AM_PAGE 切到指定页（无动画），便于无头逐页出图 */
    {
        const char * am_page = getenv("AM_PAGE");
        if(am_page != NULL) {
            int idx = atoi(am_page);
            if(idx >= 0 && idx < DESKTOP_PAGE_COUNT && page[idx] != NULL) {
                lv_tileview_set_tile(screen, page[idx], LV_ANIM_OFF);
                /* 同步分页圆点状态（VALUE_CHANGED 可能在帧后触发，截图钩子提前补偿） */
                desktop_page_changed_cb((uint32_t)idx, NULL);
            }
        }
    }

    panels = desktop_panels_create(lv_screen_active());

    /* 截图钩子：按 AM_PANEL 展开指定面板（无动画） */
    {
        const char * am_panel = getenv("AM_PANEL");
        if(am_panel != NULL) desktop_panels_apply_initial(panels, am_panel);
    }

    /* ---------------------------------------------------------------
     * 边缘感应条：在 panels 之后创建，位于 screen_active 子列表最末
     * （最后创建 = 最高渲染层级），仅覆盖屏幕上/下各 EDGE_SENSOR_H px，
     * 不遮挡桌面图标区域（图标位于 top_bar_h 以下，远离边缘）。
     * 透明无背景、CLICKABLE，GESTURE_BUBBLE 关闭（自己处理，不向上传播）。
     * --------------------------------------------------------------- */
    {
        const desktop_metrics_t * em = desktop_metrics();

        /* 顶部感应条 */
        edge_top = lv_obj_create(lv_screen_active());
        lv_obj_remove_style_all(edge_top);
        lv_obj_set_size(edge_top, em->screen_w, EDGE_SENSOR_H);
        lv_obj_set_pos(edge_top, 0, 0);
        lv_obj_set_style_bg_opa(edge_top, LV_OPA_TRANSP, 0);
        lv_obj_add_flag(edge_top, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_flag(edge_top, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(edge_top, LV_OBJ_FLAG_GESTURE_BUBBLE);
        lv_obj_add_event_cb(edge_top, edge_top_pressed_cb,  LV_EVENT_PRESSED,  NULL);
        lv_obj_add_event_cb(edge_top, edge_top_pressing_cb, LV_EVENT_PRESSING, NULL);
        lv_obj_add_event_cb(edge_top, edge_top_released_cb, LV_EVENT_RELEASED, NULL);

        /* 底部感应条 */
        edge_bot = lv_obj_create(lv_screen_active());
        lv_obj_remove_style_all(edge_bot);
        lv_obj_set_size(edge_bot, em->screen_w, EDGE_SENSOR_H);
        lv_obj_set_pos(edge_bot, 0, em->screen_h - EDGE_SENSOR_H);
        lv_obj_set_style_bg_opa(edge_bot, LV_OPA_TRANSP, 0);
        lv_obj_add_flag(edge_bot, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_flag(edge_bot, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(edge_bot, LV_OBJ_FLAG_GESTURE_BUBBLE);
        lv_obj_add_event_cb(edge_bot, edge_bot_pressed_cb,  LV_EVENT_PRESSED,  NULL);
        lv_obj_add_event_cb(edge_bot, edge_bot_pressing_cb, LV_EVENT_PRESSING, NULL);
        lv_obj_add_event_cb(edge_bot, edge_bot_released_cb, LV_EVENT_RELEASED, NULL);

        /* 面板置于感应带之上：全屏展开时把手才点得到（关闭时面板在屏外不挡感应带） */
        desktop_panels_bring_to_front(panels);
    }
}

static void clear_runtime_object_refs(void)
{
    uint32_t page_index;

    screen = NULL;
    pager_dots = NULL;
    panels = NULL;
    /* 边缘条随 lv_obj_clean(lv_screen_active()) 一起被删除，清空指针即可 */
    edge_top = NULL;
    edge_bot = NULL;
    edge_top_press_y = 0;
    edge_bot_press_y = 0;
    edge_top_dragging = 0;
    edge_bot_dragging = 0;

    for(page_index = 0; page_index < DESKTOP_PAGE_COUNT; page_index++) {
        page[page_index] = NULL;
    }
}

static void desktop_page_changed_cb(uint32_t page_index, void * user_data)
{
    LV_UNUSED(user_data);

    if(top_bar != NULL) {
        desktop_top_bar_apply(top_bar, desktop_get_page_config(page_index));
    }

    if(pager_dots != NULL) {
        if(page_index == 0) {
            lv_obj_add_flag(pager_dots, LV_OBJ_FLAG_HIDDEN);   /* 锁屏页隐藏 */
        } else {
            lv_obj_remove_flag(pager_dots, LV_OBJ_FLAG_HIDDEN);
            desktop_widget_dots_set_active(pager_dots, page_index - 1);  /* page_1→0, page_2→1 */
        }
    }
}

static void top_bar_deleted_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    top_bar = NULL;
}

static int32_t cell_w(void) { return desktop_grid_col_w(); }
static int32_t cell_h(void) { return desktop_grid_row_h(); }

static int x_by_index(int index)
{
    return (index % DESKTOP_GRID_COLS) * cell_w();
}

static int y_by_index(int index)
{
    return (index / DESKTOP_GRID_COLS) * cell_h();
}

static int clamp_index(int index)
{
    if(index < 0) {
        return 0;
    }

    if(index >= DESKTOP_SLOT_COUNT) {
        return DESKTOP_SLOT_COUNT - 1;
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
    int col = click_point->x / cell_w();
    int row = click_point->y / cell_h();
    if(col < 0) col = 0;
    if(col >= DESKTOP_GRID_COLS) col = DESKTOP_GRID_COLS - 1;
    if(row < 0) row = 0;
    if(row >= DESKTOP_GRID_ROWS) row = DESKTOP_GRID_ROWS - 1;
    return clamp_index(row * DESKTOP_GRID_COLS + col);
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
        if(!desktop_home_screen_to_local(desktop, &screen_point, &local_point)) {
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
        if(!desktop_home_screen_to_local(desktop, &screen_point, &local_point)) {
            return;
        }

        lv_obj_set_pos(target, local_point.x - offsetx, local_point.y - offsety);

        {
            const desktop_metrics_t * mb = desktop_metrics();
            int32_t half_icon = mb->icon_size / 2;
            if(local_point.x < half_icon) {
                border_lefttest_count++;
            }
            else if(local_point.x > mb->screen_w - half_icon) {
                border_righttest_count++;
            }
            else {
                border_lefttest_count = 0;
                border_righttest_count = 0;
            }
        }

        if(border_lefttest_count > 70) {
            drag_page = drag_page > 1 ? drag_page - 1 : 1;   /* page_0 为锁屏，不可拖入 */
            lv_obj_set_parent(target, page[drag_page]);
            lv_tileview_set_tile(screen, page[drag_page], LV_ANIM_ON);
            border_lefttest_count = 0;
        }

        if(border_righttest_count > 70) {
            drag_page = drag_page < DESKTOP_PAGE_COUNT - 1 ? drag_page + 1 : DESKTOP_PAGE_COUNT - 1;
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

    for(j = 0; j < DESKTOP_PAGE_COUNT; j++) {
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

    for(j = 0; j < DESKTOP_PAGE_COUNT; j++) {
        for(i = 0; i < page_icon_count[j]; i++) {
            lv_obj_set_style_transform_rotation(icons[j][i].icon, 0, 0);
        }
    }

    if(icon_shake && drag_page == meta->page) {
        lv_point_t screen_point;
        lv_point_t local_point;

        lv_indev_get_point(lv_indev_get_act(), &screen_point);
        if(!desktop_home_screen_to_local(desktop, &screen_point, &local_point)) {
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
        if(!desktop_home_screen_to_local(desktop, &screen_point, &local_point)) {
            return;
        }

        icon_shake = 0;
        new_index = index_by_xy(&local_point);

        if(icons[drag_page][DESKTOP_SLOT_COUNT - 1].icon == NULL) {
            /* 防 NULL 空洞：插入位置不得越过当前页末尾（insert 分支已保证未满，上界即 count） */
            if(new_index > page_icon_count[drag_page]) {
                new_index = page_icon_count[drag_page];
            }
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
    /* 瓷砖是 lv_obj 容器，不是 lv_image；用 style transform 旋转 */
    lv_obj_set_style_transform_rotation((lv_obj_t *)var, v, 0);
}

/* -----------------------------------------------------------------------
 * 边缘感应条回调
 * ----------------------------------------------------------------------- */

static void edge_top_pressed_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(desktop_panels_active(panels) != 0) { edge_top_dragging = 0; return; }
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    edge_top_press_y = pt.y;
    edge_top_dragging = 1;
}

static void edge_top_pressing_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(!edge_top_dragging) return;
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    if(pt.y - edge_top_press_y > 50) {
        desktop_panels_show_control(panels, true);
        edge_top_dragging = 0;
    }
}

static void edge_top_released_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    edge_top_dragging = 0;
}

static void edge_bot_pressed_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(desktop_panels_active(panels) != 0) { edge_bot_dragging = 0; return; }
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    edge_bot_press_y = pt.y;
    edge_bot_dragging = 1;
}

static void edge_bot_pressing_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(!edge_bot_dragging) return;
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    if(edge_bot_press_y - pt.y > 50) {
        desktop_panels_show_notify(panels, true);
        edge_bot_dragging = 0;
    }
}

static void edge_bot_released_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    edge_bot_dragging = 0;
}
