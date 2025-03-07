#include "test_app.h"
#include "system_app.h"
#include "desktop.h"

LV_IMG_DECLARE(test_app_icon);
LV_FONT_DECLARE(test_app_font_24);

static lv_obj_t *obj_test = NULL;
static lv_timer_t *test_timer = NULL;

typedef struct userdata
{
    lv_obj_t *label_bat;
    lv_obj_t *label_time;
    lv_obj_t *label_usb;
} userdata_t;

static void gestrue_cb(lv_event_t *e)
{
    // key_value_msg("lv_gestrue", NULL, 0);
    switch (lv_indev_get_gesture_dir(lv_indev_get_act()))
    {
    case LV_DIR_LEFT:
        key_value_msg("sys_home", NULL, 0);

        break;
    case LV_DIR_RIGHT:
        key_value_msg("sys_home", NULL, 0);

        break;
    default:
        break;
    }
}

void test_cb(lv_timer_t *e)
{
    userdata_t *tmp = (userdata_t *)e->user_data;

    cw2015_bat_info_t bat_info;
    system_get_bat(&bat_info);
    lv_label_set_text_fmt(tmp->label_bat, "%d%%  %dmV", bat_info.soc, bat_info.voltage);

    clock_time_t time;
    system_get_time(&time);

    lv_label_set_text_fmt(tmp->label_time, "%d-%d  %d  %d-%d-%d", time.month, time.day, time.week, time.hour, time.min, time.sec);

    static uint8_t usb_sta = 0;
    if (system_get_usb_sta() != usb_sta)
    {
        usb_sta = system_get_usb_sta();
        if (usb_sta)
            lv_label_set_text(tmp->label_usb, "usb online");
        else
            lv_label_set_text(tmp->label_usb, "usb offline");
    }
}

void slider_cb(lv_event_t *e)
{
    uint8_t blk = lv_slider_get_value(e->target);
    key_value_msg("sys_set_blk", &blk, 1);
}

static void test_app_load(void *arg)
{
    static RTC_FAST_ATTR uint8_t reset_count = 0;
    static userdata_t labels;

    reset_count++;

    obj_test = lv_obj_create(NULL);
    lv_obj_set_size(obj_test, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_scrollbar_mode(obj_test, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(obj_test, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *slider = lv_slider_create(obj_test);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, system_get_blk(), LV_ANIM_ON);
    lv_obj_set_size(slider, 200, 30);
    lv_obj_center(slider);
    lv_obj_add_event_cb(slider, slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *label = lv_label_create(obj_test);
    lv_obj_align_to(label, slider, LV_ALIGN_OUT_BOTTOM_MID, -20, 10);
    labels.label_usb = label;

    lv_obj_t *label_reset = lv_label_create(obj_test);
    lv_label_set_text_fmt(label_reset, "reset:%d", reset_count);
    lv_obj_align_to(label_reset, slider, LV_ALIGN_OUT_TOP_MID, 0, -80);

    lv_obj_t *label_bat = lv_label_create(obj_test);
    lv_obj_align_to(label_bat, slider, LV_ALIGN_OUT_TOP_MID, -40, -50);
    labels.label_bat = label_bat;

    lv_obj_t *label_time = lv_label_create(obj_test);
    lv_obj_align_to(label_time, slider, LV_ALIGN_OUT_TOP_MID, -40, -10);
    labels.label_time = label_time;

    lv_obj_add_event_cb(obj_test, gestrue_cb, LV_EVENT_GESTURE, NULL);

    test_timer = lv_timer_create(test_cb, 10, &labels);

    app_scr_load(obj_test, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0);
}

static void test_app_close(void *arg)
{
    lv_timer_del(test_timer);
}

void test_app_install()
{
    app_config_t cfg = {
        .app_close = test_app_close,
        .app_init = NULL,
        .app_kill = NULL,
        .app_load = test_app_load,
        .app_power_off = NULL,
        .has_gui = 1,
        .icon = &test_app_icon,
        .name = "临时测试",
        .name_font = &test_app_font_24};
    app_install(&cfg, NULL);
}
