#include "desktop.h"
#include "system_app.h"
#include "qf_zero_v2_gui_api_public.h"

#if !QF_ZERO_V2_DESKTOP_HIDDEN_EN
LV_IMG_DECLARE(icon_desktop);

#endif

static lv_obj_t *memu_cont = NULL;
static lv_obj_t *main_page = NULL;
static app_handle_t desktop_id = app_handle_none;

typedef struct _desktop_ui_list_t
{
    desktop_ui_t ui;
    size_t ui_id;
    struct _desktop_ui_list_t *next;
    struct _desktop_ui_list_t *last;
} desktop_ui_list_t;

static desktop_ui_list_t *head = NULL;
static desktop_ui_list_t *tail = NULL;
static desktop_ui_list_t *move = NULL;
static RTC_FAST_ATTR size_t desktop_ui_id_save = 0;
static size_t ui_cnt = 0;
static lv_obj_t *status_bar = NULL;
static uint8_t page_gesture_enable = 1;
static control_center_load_cb_t control_center_cb = NULL;

typedef enum
{
    ui_last,
    ui_next
} ui_switch_t;

static void gestrue_cb(lv_event_t *e);
static void desktop_load(void *arg);

void desktop_register_control_center(control_center_load_cb_t cb)
{
    control_center_cb = cb;
}

static void main_page_loaded_cb(lv_event_t *e)
{
    status_bar = NULL;
}

static desktop_ui_list_t *get_ui(size_t id)
{
    desktop_ui_list_t *tmp = head;
    for (size_t i = 0; i < id; i++)
    {
        tmp = tmp->next;
    }
    return tmp;
}

static void ui_switch(ui_switch_t type)
{

    if (move == NULL)
        return;

    void (*close_func)() = move->ui.ui_close;

    if (type == ui_last)
    {
        if (move->last == NULL)
            return;
        move = move->last;
    }
    else
    {
        if (move->next == NULL)
            return;
        move = move->next;
    }

    main_page = move->ui.ui_load();
    lv_obj_add_event_cb(main_page, gestrue_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(main_page, main_page_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    desktop_load(NULL);
    close_func();
}

// static void gestrue_start_xy(lv_event_t *e)
// {
//     lv_indev_get_point(lv_indev_get_act(), &start_point);
// }

static void gestrue_cb(lv_event_t *e)
{
    if (page_gesture_enable == 0)
        return;

    switch (lv_indev_get_gesture_dir(lv_indev_get_act()))
    {
    case LV_DIR_LEFT:
        if (lv_scr_act() == main_page)
        {
            ui_switch(ui_next);
            return;
        }

        lv_scr_load_anim(main_page, LV_SCR_LOAD_ANIM_MOVE_LEFT, desktop_load_move_time, 0, 0);

        break;
    case LV_DIR_RIGHT:
        if (lv_scr_act() == main_page)
        {
            ui_switch(ui_last);
            return;
        }

        lv_scr_load_anim(main_page, LV_SCR_LOAD_ANIM_MOVE_RIGHT, desktop_load_move_time, 0, 0);

        break;
    case LV_DIR_TOP:
        if (lv_scr_act() == main_page)
        {
            lv_scr_load_anim(memu_cont, LV_SCR_LOAD_ANIM_MOVE_TOP, desktop_load_move_time, 0, 0);
            return;
        }
        // if (status_bar == NULL)
        //     return;
        // status_bar_del();

        break;
    case LV_DIR_BOTTOM:
        // if (lv_scr_act() == main_page)
        // {
        //     status_bar_init();
        // }
        if (control_center_cb == NULL)
            return;
        cst816_point_t point;

        cst816_read_point(&point);
        if (point.start_y > control_center_area_width)
            return;
        app_scr_load(control_center_cb(), LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 200, 0);

        break;
    default:
        break;
    }
}

static void desktop_home_cb(void *value, size_t lenth)
{
    if (lv_scr_act() == main_page)
    {
        return;
    }
    app_load("desktop", app_handle_none, value);
}

static void app_click_cb(lv_event_t *e)
{
    uint8_t tmp;
    key_value_msg("tp_type", &tmp, 1); // 获取屏幕操作是否为点击
    if (tmp)                           // 如果有滑动
    {
        return; // 丢弃CLICK操作
    }
    // printf("app name:%s\n", (const char *)e->user_data);
    app_load((const char *)e->user_data, app_handle_none, NULL);
}

lv_img_dsc_t desk_img = {
    .header.cf = LV_IMG_CF_TRUE_COLOR,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 280,
    .header.h = 240,
    .data_size = 280 * 240 * LV_COLOR_SIZE / 8,
    .data = NULL,
};
static void menu_load(void)
{
    memu_cont = public_create_menu_list(NULL);
   // lv_obj_set_style_pad_row(memu_cont, -15, LV_PART_MAIN);

#if !QF_ZERO_V2_DESKTOP_HIDDEN_EN
#if desktop_bg_default != 0
    lv_obj_set_style_bg_img_src(memu_cont, &desktop_bg_default, LV_PART_MAIN);
#endif
#endif

    uint32_t i;
    size_t ui_app_cnt = 0;
    for (i = 0; i < (app_get_cnt() - 1); i++)
    {
        app_obj_t *app = app_get(NULL, i);
        if (app->has_gui == 0)
            continue;

        ui_app_cnt++;

        lv_obj_t *btn = public_create_col_img_label(memu_cont, app->icon, app->name, (const lv_font_t *)app->name_font);

        if (i < (app_get_cnt() - 2))
            public_create_col_line(memu_cont, lv_color_hex(0x2D2D2D), 90);

        lv_obj_add_event_cb(btn, app_click_cb, LV_EVENT_CLICKED, (void *)app->name);
    }

    if (ui_app_cnt)
    {
        if (ui_app_cnt == 1)
        {
            lv_obj_set_style_bg_opa(public_create_col_line(memu_cont, lv_color_hex(0x2D2D2D), 90), 0, 0);
        }
        lv_event_send(memu_cont, LV_EVENT_SCROLL, NULL);
        lv_obj_scroll_to_view(lv_obj_get_child(memu_cont, 0), LV_ANIM_OFF);
    }

    lv_obj_add_event_cb(memu_cont, gestrue_cb, LV_EVENT_GESTURE, NULL);
}

static void desktop_init(void *arg)
{
    desktop_ui_list();
    if (ui_cnt == 0)
    {
        main_page = lv_obj_create(NULL);
    }
    else
    {
        move = get_ui(desktop_ui_id_save);
        main_page = move->ui.ui_load();
    }

    lv_obj_add_event_cb(main_page, gestrue_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(main_page, main_page_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    menu_load();
    key_value_register(NULL, "sys_home", desktop_home_cb);
}

static void desktop_load(void *arg)
{
    if (arg == NULL)
    {
        lv_scr_load_anim(main_page, LV_SCR_LOAD_ANIM_FADE_ON, desktop_load_fade_time, 0, 1);
    }
    else
    {
        lv_scr_load_anim_t anim = *(lv_scr_load_anim_t *)arg;
        if (anim >= LV_SCR_LOAD_ANIM_FADE_IN && anim <= LV_SCR_LOAD_ANIM_FADE_OUT)
            lv_scr_load_anim(main_page, anim, desktop_load_fade_time, 0, 1);
        else if (anim != LV_SCR_LOAD_ANIM_NONE)
            lv_scr_load_anim(main_page, anim, desktop_load_move_time, 0, 1);
        else
            lv_scr_load_anim(main_page, anim, 0, 0, 1);
    }
}

static void desktop_close(void *arg)
{

    // printf("fun2:%d,%d\n", (uint32_t)move->ui->ui_close, (uint32_t)move);

    // vTaskDelay(100);

    // if (move != NULL && move->ui.ui_close != NULL)
    //     move->ui.ui_close();
}

static void desktop_kill(void *arg)
{
    if (move == NULL)
        return;
    desktop_ui_id_save = move->ui_id;
    printf("desktop kill\n");
}

static void desktop_power_off()
{
    printf("desktop power off\n");
}

void app_scr_load(lv_obj_t *scr, lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay)
{
    lv_scr_load_anim(scr, anim_type, time, delay, false); // 加载界面
}

void desktop_app_install()
{
    app_config_t cfg = {
        .app_close = desktop_close,
        .app_init = desktop_init,
        .app_kill = desktop_kill,
        .app_load = desktop_load,
        .app_power_off = desktop_power_off,
        .has_gui = 1,
#if QF_ZERO_V2_DESKTOP_HIDDEN_EN
        .icon = NULL,
#else
        .icon = &icon_desktop,
#endif
        .name = "desktop",
        .name_font = NULL};
    desktop_id = app_install(&cfg, NULL);
}

void desktop_add_ui(desktop_ui_t *ui)
{
    desktop_ui_list_t *tmp = malloc(sizeof(desktop_ui_list_t));
    if (tmp == NULL)
        return;
    tmp->ui = *ui;
    tmp->next = NULL;
    tmp->last = tail;
    tmp->ui_id = ui_cnt++;
    if (tail != NULL)
    {
        tail->next = tmp;
    }
    tail = tmp;
    if (head == NULL)
    {
        head = tmp;
    }
}