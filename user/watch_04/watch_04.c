#include "system_app.h"
#include "desktop.h"

static const char *ui_name = "qf_watch_04"; // 表盘名称

LV_IMG_DECLARE(watch_04_img_bg);
LV_FONT_DECLARE(watch_04_font_18);
LV_FONT_DECLARE(watch_04_font_48);

static const char *week_str[] = {
    "SON.",
    "MON.",
    "TUE.",
    "WED.",
    "THU.",
    "FRI.",
    "SAT."};

typedef struct
{
    lv_obj_t *label_week;
    lv_obj_t *label_time;
    lv_obj_t *label_month;
} ref_objs_t;

static lv_timer_t *data_ref_timer = NULL;
static ref_objs_t *ref_objs = NULL;
static uint8_t data_ref_flg = 2;

static void _scr_info_ref_cb(lv_timer_t *e)
{
    static uint8_t last_min = 0;
    clock_time_t time;
    key_value_msg("sys_get_time", &time, sizeof(time));

    if (last_min == time.min && data_ref_flg == 0)
        return;
    last_min = time.min;

    lv_label_set_text_fmt(ref_objs->label_month, "%02d/%02d", time.month, time.day);
    lv_label_set_text_fmt(ref_objs->label_week, "%s", week_str[time.week]);
    lv_label_set_text_fmt(ref_objs->label_time, "%02d:%02d", time.hour, time.min);

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
    ref_objs = lv_mem_alloc(sizeof(ref_objs_t));

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    lv_obj_t *bg = lv_img_create(scr);
    lv_img_set_src(bg, &watch_04_img_bg);

    ref_objs->label_month = lv_label_create(scr);
    ref_objs->label_time = lv_label_create(scr);
    ref_objs->label_week = lv_label_create(scr);

    lv_obj_set_style_text_font(ref_objs->label_month, &watch_04_font_18, 0);
    lv_obj_set_style_text_font(ref_objs->label_week, &watch_04_font_18, 0);
    lv_obj_set_style_text_font(ref_objs->label_time, &watch_04_font_48, 0);

    lv_obj_set_style_text_color(ref_objs->label_month, lv_color_hex(0xfffefe), 0);
    lv_obj_set_style_text_color(ref_objs->label_week, lv_color_hex(0xfffefe), 0);
    lv_obj_set_style_text_color(ref_objs->label_time, lv_color_hex(0xff0000), 0);

    lv_obj_align(ref_objs->label_month, LV_ALIGN_TOP_LEFT, 187, 120);
    lv_obj_align(ref_objs->label_week, LV_ALIGN_TOP_LEFT, 18, 121);
    lv_obj_align(ref_objs->label_time, LV_ALIGN_TOP_MID, 1, 48);

    data_ref_flg = 2;
    data_ref_timer = lv_timer_create(_scr_info_ref_cb, 20, NULL);
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

    lv_mem_free(ref_objs);
    ref_objs = NULL;
}

void watch_04_install() // 安装表盘
{
    desktop_ui_t cfg = {
        .name = ui_name,
        .ui_close = scr_close,
        .ui_load = scr_load};
    desktop_add_ui(&cfg);
}
