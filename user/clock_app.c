#include "clock_app.h"
#include "system_app.h"
#include "desktop.h"
#include "qf_zero_v2_gui_api_public.h"

LV_IMG_DECLARE(clock_app_icon);
LV_IMG_DECLARE(clock_app_img_alarm);
LV_IMG_DECLARE(clock_app_img_stopwatch);
LV_IMG_DECLARE(clock_app_img_day);
LV_IMG_DECLARE(clock_app_img_back);
LV_IMG_DECLARE(clock_app_img_sure);

LV_FONT_DECLARE(clock_app_font_24);

static lv_obj_t *scr_main = NULL;

static clock_time_t time_tmp;
static uint8_t set_num_tmp = 0;

static void scr_event_cb(lv_event_t *e)
{

    if (e->code != LV_EVENT_GESTURE)
        return;

    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_RIGHT || dir == LV_DIR_LEFT)
    {
        lv_scr_load_anim_t anim;
        if (dir == LV_DIR_LEFT)
            anim = LV_SCR_LOAD_ANIM_MOVE_LEFT;
        else if (dir == LV_DIR_RIGHT)

            anim = LV_SCR_LOAD_ANIM_MOVE_RIGHT;

        key_value_msg("sys_home", &anim, sizeof(anim));
        return;
    }
}

static lv_obj_t *create_label_white(lv_obj_t *pare, const lv_font_t *font, const char *str)
{
    lv_obj_t *label = lv_label_create(pare);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(label, font, 0);
    lv_label_set_text(label, str);
    return label;
}

static void set_time_ref_label(lv_obj_t *label, uint8_t var)
{
    lv_label_set_text_fmt(label, "%d", var);
    lv_obj_center(label);
}

static void num_change_cb(lv_event_t *e)
{
    lv_label_set_text_fmt((lv_obj_t *)e->user_data, "%d", lv_arc_get_value(e->target));
}

static void img_btn_cb(lv_event_t *e)
{
    lv_obj_t *pare = lv_obj_get_parent(e->target);
    lv_obj_t *arc = lv_obj_get_child(pare, 0);
    lv_obj_t *label = (lv_obj_t *)e->user_data;

    if (lv_obj_has_flag(e->target, LV_OBJ_FLAG_USER_1) == 0)
    {
        lv_obj_del(pare);
        return;
    }

    uint8_t var = lv_arc_get_value(arc);

    if (set_num_tmp == 0)
    {
        time_tmp.year = var;
    }
    else if (set_num_tmp == 1)
    {
        time_tmp.month = var;
    }
    else if (set_num_tmp == 2)
    {
        time_tmp.day = var;
    }
    else if (set_num_tmp == 3)
    {
        time_tmp.hour = var;
    }
    else if (set_num_tmp == 4)
    {
        time_tmp.min = var;
    }
    else if (set_num_tmp == 5)
    {
        time_tmp.sec = var;
    }
    set_time_ref_label(label, var);

    lv_obj_del(pare);
}

static void set_time_btn_cb(lv_event_t *e)
{
    set_num_tmp = 0;
    if (lv_obj_has_flag(e->target, LV_OBJ_FLAG_USER_1))
        set_num_tmp |= 0x01;
    if (lv_obj_has_flag(e->target, LV_OBJ_FLAG_USER_2))
        set_num_tmp |= 0x02;
    if (lv_obj_has_flag(e->target, LV_OBJ_FLAG_USER_3))
        set_num_tmp |= 0x04;

    lv_obj_t *bg = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(bg);
    lv_obj_set_size(bg, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_scrollbar_mode(bg, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(bg, lv_color_hex(0), 0);
    lv_obj_set_style_bg_opa(bg, 255, 0);

    lv_obj_t *arc = lv_arc_create(bg);
    lv_obj_set_size(arc, LV_HOR_RES, LV_VER_RES);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_obj_center(arc);
    lv_obj_set_style_arc_color(arc, lv_color_hex(clock_app_arc_main_color), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(clock_app_arc_indicator_color), LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(arc, 20, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 20, LV_PART_INDICATOR);

    lv_obj_t *label = create_label_white(bg, &clock_app_font_24, NULL);
    lv_obj_add_event_cb(arc, num_change_cb, LV_EVENT_VALUE_CHANGED, label);
    lv_obj_center(label);

    if (set_num_tmp == 0)
    {
        lv_arc_set_range(arc, 0, 99);
        lv_arc_set_value(arc, time_tmp.year);
    }
    else if (set_num_tmp == 1)
    {
        lv_arc_set_bg_angles(arc, 30, 240);
        lv_arc_set_range(arc, 1, 12);
        lv_arc_set_value(arc, time_tmp.month);
    }
    else if (set_num_tmp == 2)
    {
        lv_arc_set_range(arc, 1, 31);
        lv_arc_set_value(arc, time_tmp.day);
    }
    else if (set_num_tmp == 3)
    {
        lv_arc_set_range(arc, 0, 23);
        lv_arc_set_value(arc, time_tmp.hour);
    }
    else if (set_num_tmp == 4)
    {
        lv_arc_set_range(arc, 0, 59);
        lv_arc_set_value(arc, time_tmp.min);
    }
    else if (set_num_tmp == 5)
    {
        lv_arc_set_range(arc, 0, 59);
        lv_arc_set_value(arc, time_tmp.sec);
    }

    lv_obj_t *img_btn = lv_img_create(bg);
    lv_img_set_src(img_btn, &clock_app_img_back);
    lv_obj_align(img_btn, LV_ALIGN_CENTER, 0, -50);
    lv_obj_add_flag(img_btn, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_add_event_cb(img_btn, img_btn_cb, LV_EVENT_CLICKED, e->user_data);

    img_btn = lv_img_create(bg);
    lv_img_set_src(img_btn, &clock_app_img_sure);
    lv_obj_align(img_btn, LV_ALIGN_CENTER, 0, 50);
    lv_obj_add_flag(img_btn, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_USER_1);

    lv_obj_add_event_cb(img_btn, img_btn_cb, LV_EVENT_CLICKED, e->user_data);

    lv_event_send(arc, LV_EVENT_VALUE_CHANGED, NULL);
}

static void img_btn_cb_set_time(lv_event_t *e)
{
    if (lv_obj_has_flag(e->target, LV_OBJ_FLAG_USER_1))
    {
        key_value_msg("sys_set_time", &time_tmp, sizeof(time_tmp));
    }

    lv_scr_load_anim(scr_main, LV_SCR_LOAD_ANIM_NONE, 0, 0, 1); // 返回APP主界面
}

static void set_day_cb(lv_event_t *e)
{
    uint8_t tmp;
    key_value_msg("tp_type", &tmp, 1); // 获取屏幕操作是否为点击
    if (tmp)                           // 如果有滑动
        return;                        // 丢弃CLICK操作

    lv_obj_t *cont = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0), 0);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *btn[6];
    lv_obj_t *label[6];
    lv_obj_t *label_ch[6];

    for (size_t i = 0; i < 6; i++)
    {
        btn[i] = lv_btn_create(cont);
        label[i] = lv_label_create(btn[i]);
        label_ch[i] = lv_label_create(cont);
        lv_obj_set_width(btn[i], 50);
        lv_obj_set_style_bg_color(btn[i], lv_color_hex(0x626973), 0);
        lv_obj_set_style_text_font(label[i], &clock_app_font_24, 0);
        lv_obj_set_style_text_font(label_ch[i], &clock_app_font_24, 0);
        lv_obj_set_style_text_color(label[i], lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_color(label_ch[i], lv_color_hex(0xffffff), 0);
        if (i & 0x01)
            lv_obj_add_flag(btn[i], LV_OBJ_FLAG_USER_1);
        if (i & 0x02)
            lv_obj_add_flag(btn[i], LV_OBJ_FLAG_USER_2);
        if (i & 0x04)
            lv_obj_add_flag(btn[i], LV_OBJ_FLAG_USER_3);
        lv_obj_add_event_cb(btn[i], set_time_btn_cb, LV_EVENT_CLICKED, label[i]);
    }

    lv_obj_align(btn[0], LV_ALIGN_LEFT_MID, 25, -25);
    lv_obj_align(btn[1], LV_ALIGN_CENTER, 0, -25);
    lv_obj_align(btn[2], LV_ALIGN_RIGHT_MID, -25, -25);
    lv_obj_align(btn[3], LV_ALIGN_LEFT_MID, 25, 25);
    lv_obj_align(btn[4], LV_ALIGN_CENTER, 0, 25);
    lv_obj_align(btn[5], LV_ALIGN_RIGHT_MID, -25, 25);

    lv_label_set_text(label_ch[0], "年");
    lv_label_set_text(label_ch[1], "月");
    lv_label_set_text(label_ch[2], "日");
    lv_label_set_text(label_ch[3], "时");
    lv_label_set_text(label_ch[4], "分");
    lv_label_set_text(label_ch[5], "秒");

    lv_obj_align(label_ch[0], LV_ALIGN_LEFT_MID, 38, -64);
    lv_obj_align(label_ch[1], LV_ALIGN_CENTER, 0, -64);
    lv_obj_align(label_ch[2], LV_ALIGN_RIGHT_MID, -38, -64);
    lv_obj_align(label_ch[3], LV_ALIGN_LEFT_MID, 38, 65);
    lv_obj_align(label_ch[4], LV_ALIGN_CENTER, 0, 65);
    lv_obj_align(label_ch[5], LV_ALIGN_RIGHT_MID, -38, 65);

    key_value_msg("sys_get_time", &time_tmp, sizeof(time_tmp));

    set_time_ref_label(label[0], time_tmp.year);
    set_time_ref_label(label[1], time_tmp.month);
    set_time_ref_label(label[2], time_tmp.day);
    set_time_ref_label(label[3], time_tmp.hour);
    set_time_ref_label(label[4], time_tmp.min);
    set_time_ref_label(label[5], time_tmp.sec);

    lv_obj_t *img_btn = lv_img_create(cont);
    lv_img_set_src(img_btn, &clock_app_img_back);
    lv_obj_align(img_btn, LV_ALIGN_TOP_MID, 0, 3);
    lv_obj_add_flag(img_btn, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_add_event_cb(img_btn, img_btn_cb_set_time, LV_EVENT_CLICKED, NULL);

    img_btn = lv_img_create(cont);
    lv_img_set_src(img_btn, &clock_app_img_sure);
    lv_obj_align(img_btn, LV_ALIGN_BOTTOM_MID, 0, -3);
    lv_obj_add_flag(img_btn, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_USER_1);

    lv_obj_add_event_cb(img_btn, img_btn_cb_set_time, LV_EVENT_CLICKED, NULL);

    app_scr_load(cont, LV_SCR_LOAD_ANIM_NONE, 0, 0); // 加载界面
}

static void clock_load_set_time()
{
    lv_obj_t *btn = public_create_col_img_label(scr_main, &clock_app_img_day, "时间日期", &clock_app_font_24);
    lv_obj_add_event_cb(btn, set_day_cb, LV_EVENT_CLICKED, NULL);
}

static void clock_stopwatch_load()
{
    // lv_obj_t *btn =
    public_create_col_img_label(scr_main, &clock_app_img_stopwatch, "秒表", &clock_app_font_24);
    // TO DO
}

static void clock_alarm_load()
{
    // lv_obj_t *btn =
    public_create_col_img_label(scr_main, &clock_app_img_alarm, "闹钟", &clock_app_font_24);
    // TO DO
}

static void clock_app_load(void *arg)
{
    scr_main = public_create_menu_list(NULL);
    //lv_obj_set_style_pad_row(scr_main, -25, LV_PART_MAIN);

    clock_load_set_time();  // 设置时间功能
    public_create_col_line(scr_main, lv_color_hex(0x2D2D2D), 90);
    clock_stopwatch_load(); // 秒表
    public_create_col_line(scr_main, lv_color_hex(0x2D2D2D), 90);
    clock_alarm_load();     // 闹钟

    lv_event_send(scr_main, LV_EVENT_SCROLL, NULL);
    lv_obj_scroll_to_view(lv_obj_get_child(scr_main, 0), LV_ANIM_OFF);

    lv_obj_add_event_cb(scr_main, scr_event_cb, LV_EVENT_GESTURE, NULL);
    app_scr_load(scr_main, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0); // 加载APP界面
}

void clock_app_install()
{
    app_config_t cfg = {
        .app_close = NULL,
        .app_init = NULL,
        .app_kill = NULL,
        .app_load = clock_app_load,
        .app_power_off = NULL,
        .has_gui = 1,
        .icon = &clock_app_icon,
        .name = "时钟",
        .name_font = &clock_app_font_24};
    app_install(&cfg, NULL);
}