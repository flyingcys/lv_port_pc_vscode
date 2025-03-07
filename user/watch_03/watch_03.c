#include "system_app.h"
#include "desktop.h"

static const char *ui_name = "qf_watch_03"; // 表盘名称

LV_IMG_DECLARE(watch_03_img_bg);
LV_IMG_DECLARE(watch_03_img_time);
LV_IMG_DECLARE(watch_03_img_time_1);
LV_IMG_DECLARE(watch_03_img_time_2);
// LV_IMG_DECLARE(watch_03_img_time_3);
LV_IMG_DECLARE(watch_03_img_time_4);
LV_IMG_DECLARE(watch_03_img_week);

typedef struct
{
    lv_obj_t *img_week;
    lv_obj_t *img_hour;
    lv_obj_t *img_hour_1;
    lv_obj_t *img_min;
    lv_obj_t *img_min_1;
    lv_obj_t *img_sec;
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

    uint8_t offset[4];
    offset[0] = time.hour / 10;
    offset[1] = time.hour % 10;
    offset[2] = time.min / 10;
    offset[3] = time.min % 10;

    lv_img_set_offset_y(ref_objs->img_hour, offset[0] * -35);
    lv_img_set_offset_y(ref_objs->img_hour_1, offset[1] * -35);
    lv_img_set_offset_y(ref_objs->img_min, offset[2] * -35);
    lv_img_set_offset_y(ref_objs->img_min_1, offset[3] * -35);
    lv_img_set_offset_y(ref_objs->img_week, time.week * -9);

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
    lv_img_set_src(bg, &watch_03_img_bg);

    ref_objs->img_hour = lv_img_create(scr);
    ref_objs->img_hour_1 = lv_img_create(scr);
    ref_objs->img_min = lv_img_create(scr);
    ref_objs->img_min_1 = lv_img_create(scr);
    ref_objs->img_sec = lv_img_create(scr);
    ref_objs->img_week = lv_img_create(scr);

    lv_img_set_src(ref_objs->img_hour, &watch_03_img_time);
    lv_img_set_src(ref_objs->img_hour_1, &watch_03_img_time_1);
    lv_img_set_src(ref_objs->img_min, &watch_03_img_time_2);
    lv_img_set_src(ref_objs->img_min_1, &watch_03_img_time_2);
    lv_img_set_src(ref_objs->img_sec, &watch_03_img_time_4);
    lv_img_set_src(ref_objs->img_week, &watch_03_img_week);

    lv_obj_set_height(ref_objs->img_hour, 35);
    lv_obj_align(ref_objs->img_hour, LV_ALIGN_TOP_LEFT, 57, 37);
    lv_obj_set_height(ref_objs->img_hour_1, 35);
    lv_obj_align(ref_objs->img_hour_1, LV_ALIGN_TOP_LEFT, 87, 37);

    lv_obj_set_height(ref_objs->img_min, 35);
    lv_obj_align(ref_objs->img_min, LV_ALIGN_TOP_LEFT, 133, 37);
    lv_obj_set_height(ref_objs->img_min_1, 35);
    lv_obj_align(ref_objs->img_min_1, LV_ALIGN_TOP_LEFT, 163, 37);

    lv_obj_align(ref_objs->img_sec, LV_ALIGN_TOP_LEFT, 117, 42);
    lv_obj_set_height(ref_objs->img_week, 9);
    lv_obj_align(ref_objs->img_week, LV_ALIGN_TOP_LEFT, 106, 85);

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

void watch_03_install() // 安装表盘
{
    desktop_ui_t cfg = {
        .name = ui_name,
        .ui_close = scr_close,
        .ui_load = scr_load};
    desktop_add_ui(&cfg);
}
