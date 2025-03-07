#include "system_app.h"
#include "desktop.h"

static const char *ui_name = "qf_watch_05"; // 表盘名称

LV_IMG_DECLARE(watch_05_img_bg);
LV_FONT_DECLARE(watch_05_font_38);

static lv_timer_t *data_ref_timer = NULL;
static uint8_t data_ref_flg = 2;

static void _scr_info_ref_cb(lv_timer_t *e)
{
    static uint8_t last_min = 0;
    clock_time_t time;
    key_value_msg("sys_get_time", &time, sizeof(time));

    if (last_min == time.min && data_ref_flg == 0)
        return;
    last_min = time.min;

    lv_obj_t *label_time = (lv_obj_t *)e->user_data;

    lv_label_set_text_fmt(label_time, "%02d:%02d", time.hour, time.min);

    if (data_ref_flg)
    {
        data_ref_flg--;
        if (data_ref_flg == 0)
            lv_timer_set_period(data_ref_timer, 500);
    }
}

static void scr_close_cb(lv_event_t *e)
{
    lv_timer_pause(data_ref_timer);
}

static void scr_load_cb(lv_event_t *e)
{
    data_ref_flg++;
    lv_timer_resume(data_ref_timer);
}

////////////////////////////////////////////////////////////////////////////////////////

static lv_obj_t *scr_load() // 表盘加载函数，返回表盘对象
{

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    lv_obj_t *bg = lv_img_create(scr);
    lv_img_set_src(bg, &watch_05_img_bg);

    lv_obj_t *label_time = lv_label_create(scr);

    lv_obj_set_style_text_font(label_time, &watch_05_font_38, 0);

    lv_obj_set_style_text_color(label_time, lv_color_hex(0xffffff), 0);

    lv_obj_set_style_text_letter_space(label_time, 6, 0);

    lv_obj_align(label_time, LV_ALIGN_TOP_MID, 0, 41);

    data_ref_flg = 2;
    data_ref_timer = lv_timer_create(_scr_info_ref_cb, 20, label_time);
    lv_timer_ready(data_ref_timer);

    lv_obj_add_event_cb(scr, scr_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(scr, scr_close_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_timer_pause(data_ref_timer);

    return scr;
}

static void scr_close() // 正在使用的表盘切换到不使用
{
    lv_timer_del(data_ref_timer);
    data_ref_timer = NULL;
}

void watch_05_install() // 安装表盘
{
    desktop_ui_t cfg = {
        .name = ui_name,
        .ui_close = scr_close,
        .ui_load = scr_load};
    desktop_add_ui(&cfg);
}
