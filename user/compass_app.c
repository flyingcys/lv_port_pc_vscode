#include "compass_app.h"
#include "system_app.h"
#include "desktop.h"

LV_IMG_DECLARE(compass_app_img_icon);
LV_IMG_DECLARE(compass_app_img_bg);
LV_FONT_DECLARE(compass_app_font_24);

static uint8_t act_flg = 0;
static uint8_t calibration_flg = 0;
static key_value_handle_t kv_handle = NULL;
static const char *rotation_ch[] = {
    "西南%d°",
    "西%d°",
    "西北%d°",
    "北%d°",
    "东北%d°",
    "东%d°",
    "东南%d°",
    "南%d°"};

enum
{
    label_start_calibration,
    label_x_min,
    label_y_min,
    label_z_min,
    label_x_max,
    label_y_max,
    label_z_max,
    label_tip,
    label_angle,
    label_cnt
};
static lv_obj_t *label[label_cnt];

static void ui_event(lv_event_t *e)
{
    if (e->code == LV_EVENT_GESTURE)
    {
        if (lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT || lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT)
        {
            if (calibration_flg)
            {
                lv_obj_clear_flag(lv_obj_get_child(lv_scr_act(), 0), LV_OBJ_FLAG_HIDDEN);
                lv_obj_del(lv_obj_get_child(e->target, lv_obj_get_child_cnt(e->target) - 1));
                calibration_flg = 0;
            }
            else
            {
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
        }
    }
}

static void lv_timer_cb(lv_timer_t *e)
{

    if (act_flg == 0)
    {
        lv_timer_del(e);
        return;
    }

    if (calibration_flg)
    {
        qmc_xyz_range_t range;
        static qmc_xyz_range_t last_range;
        key_value_msg("qmc_get_range", &range, sizeof(range));

        if (last_range.x_min != range.x_min)
            lv_label_set_text_fmt(label[label_x_min], "%d", range.x_min);
        if (last_range.x_max != range.x_max)
            lv_label_set_text_fmt(label[label_x_max], "%d", range.x_max);
        if (last_range.y_min != range.y_min)
            lv_label_set_text_fmt(label[label_y_min], "%d", range.y_min);
        if (last_range.y_max != range.y_max)
            lv_label_set_text_fmt(label[label_y_max], "%d", range.y_max);
        if (last_range.z_min != range.z_min)
            lv_label_set_text_fmt(label[label_z_min], "%d", range.z_min);
        if (last_range.z_max != range.z_max)
            lv_label_set_text_fmt(label[label_z_max], "%d", range.z_max);

        last_range = range;
        return;
    }

    lv_obj_t *img = (lv_obj_t *)e->user_data;

    int16_t angle = 0;
    key_value_msg("qmc_get_angle", &angle, sizeof(angle));

    static int16_t now_angle = 0;
    if (now_angle == angle)
        return;

    if (now_angle < angle)
    {
        int16_t tmp = angle - now_angle;

        if (tmp < 180)
        {
            int16_t cnt = tmp / 10;
            if (tmp == 0)
                tmp++;
            now_angle += cnt;
        }
        else
        {
            tmp = 180 + 179 - angle + now_angle;
            int16_t cnt = tmp / 10;
            if (tmp == 0)
                tmp++;
            now_angle -= cnt;
        }
    }
    else
    {
        int16_t tmp = now_angle - angle;
        if (tmp < 180)
        {
            int16_t cnt = tmp / 10;
            now_angle -= cnt;
        }
        else
        {
            tmp = 180 + 179 + angle - now_angle;
            int16_t cnt = tmp / 10;
            if (tmp == 0)
                tmp++;
            now_angle += cnt;
        }
    }

    if (now_angle > 180)
        now_angle = -179 + (now_angle - 180);
    if (now_angle < -179)
        now_angle = 180 - (-179 - now_angle);

    lv_img_set_angle(img, now_angle * 10);

    uint8_t tmp = 0;
    if (now_angle < -157)
        tmp = 7;
    else if (now_angle < -112)
        tmp = 6;
    else if (now_angle < -67)
        tmp = 5;
    else if (now_angle < -22)
        tmp = 4;
    else if (now_angle < 22)
        tmp = 3;
    else if (now_angle < 67)
        tmp = 2;
    else if (now_angle < 112)
        tmp = 1;
    else if (now_angle < 157)
        tmp = 0;
    else
        tmp = 7;

    int16_t angle_t = now_angle;
    if (angle_t < 0)
        angle_t += 360;
    angle_t = -angle_t + 360;

    lv_label_set_text_fmt(label[label_angle], rotation_ch[tmp], angle_t);
}

static void label_click_cb(lv_event_t *e)
{
    lv_obj_t *label_btn = e->target;
    if (lv_obj_has_flag(label_btn, LV_OBJ_FLAG_USER_1)) // 开始校准
    {
        lv_label_set_text(label_btn, "保存参数");
        lv_label_set_text(label[label_tip], "无死角转动");
        lv_obj_clear_flag(label_btn, LV_OBJ_FLAG_USER_1);
        uint8_t en = 1;
        key_value_msg("qmc_calibration", &en, 1);
    }
    else // 校准完毕
    {
        uint8_t en = 0;
        key_value_msg("qmc_calibration", &en, 1);
        lv_label_set_text(label_btn, "开始校准");
        lv_label_set_text(label[label_tip], "");
        lv_obj_add_flag(label_btn, LV_OBJ_FLAG_USER_1);
    }
}

static void calibration_cb(void *value, size_t lenth)
{
    if (calibration_flg == 1)
        return;
    calibration_flg = 1;
    lv_obj_t *cali_scr = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(cali_scr);
    lv_obj_clear_flag(cali_scr, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(cali_scr, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_size(cali_scr, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_opa(cali_scr, 255, 0);
    lv_obj_add_flag(lv_obj_get_child(lv_scr_act(), 0), LV_OBJ_FLAG_HIDDEN);

    for (size_t i = 0; i <= label_tip; i++)
    {
        label[i] = lv_label_create(cali_scr);
        lv_obj_set_style_text_font(label[i], &compass_app_font_24, 0);
        lv_obj_set_style_text_color(label[i], lv_color_hex(0xffffff), 0);
    }

    lv_label_set_text(label[label_start_calibration], "开始校准");
    lv_obj_align(label[label_start_calibration], LV_ALIGN_BOTTOM_MID, 0, -25);
    lv_obj_add_flag(label[label_start_calibration], LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(label[label_start_calibration], LV_OBJ_FLAG_USER_1);
    lv_obj_add_event_cb(label[label_start_calibration], label_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_outline_width(label[label_start_calibration], 3, 0);
    lv_obj_set_style_outline_color(label[label_start_calibration], lv_color_hex(0xB300F0), 0);
    lv_obj_set_style_outline_opa(label[label_start_calibration], 255, 0);
    lv_obj_set_style_radius(label[label_start_calibration], 5, 0);

    lv_obj_set_style_border_width(label[label_start_calibration], 3, 0);
    lv_obj_set_style_border_opa(label[label_start_calibration], 0, 0);

    lv_obj_set_style_outline_color(label[label_start_calibration], lv_color_hex(0x57FA57), LV_STATE_PRESSED);

    lv_obj_align(label[label_tip], LV_ALIGN_TOP_MID, 0, 25);
    lv_label_set_text(label[label_tip], "");

    for (size_t i = label_x_min; i <= label_z_max; i++)
    {
        lv_label_set_text(label[i], "0");
    }

    for (size_t i = label_x_min; i <= label_z_min; i++)
    {
        lv_obj_align(label[i], LV_ALIGN_LEFT_MID, 40, (i - 2) * (40));
    }
    for (size_t i = label_x_max; i <= label_z_max; i++)
    {
        lv_obj_align(label[i], LV_ALIGN_RIGHT_MID, -40, (i - 5) * (40));
    }
}

static void compass_app_load(void *arg)
{
    lv_obj_t *scr = lv_obj_create(NULL);

    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);

    uint8_t charge_flg = 0;
    key_value_msg("sys_usb_sta", &charge_flg, 1);
    if (charge_flg)
    {
        lv_obj_t *label_tmp = lv_label_create(scr);
        lv_obj_set_style_text_font(label_tmp, &compass_app_font_24, 0);
        lv_obj_set_style_text_color(label_tmp, lv_color_hex(0xFFFFFF), 0);
        lv_obj_center(label_tmp);
        lv_label_set_text(label_tmp, "充电时不支持此软件");

        lv_obj_add_event_cb(scr, ui_event, LV_EVENT_ALL, NULL);
        app_scr_load(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0); // 加载APP界面
        return;
    }

    lv_obj_t *img = lv_img_create(scr);
    lv_img_set_src(img, &compass_app_img_bg);
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    label[label_angle] = lv_label_create(scr);
    lv_obj_set_style_text_font(label[label_angle], &compass_app_font_24, 0);
    lv_obj_set_style_text_color(label[label_angle], lv_color_hex(0xA0A6A5), 0);
    lv_obj_center(label[label_angle]);
    lv_label_set_text(label[label_angle], "北0°");

    lv_timer_t *timer = lv_timer_create(lv_timer_cb, 16, img);
    lv_timer_set_repeat_count(timer, -1);

    lv_obj_add_event_cb(scr, ui_event, LV_EVENT_ALL, NULL);
    app_scr_load(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0); // 加载APP界面

    key_value_register(&kv_handle, "btn_click", calibration_cb);

    uint8_t hold_lcd = 1;
    key_value_msg("sys_always_on", &hold_lcd, 1);

    act_flg = 1;
}

static void compass_app_close(void *arg)
{
    if (act_flg == 0)
        return;
    key_value_del(kv_handle);
    act_flg = 0;
    calibration_flg = 0;
    uint8_t hold_lcd = 0;
    key_value_msg("sys_always_on", &hold_lcd, 1);
}

void compass_app_install()
{
    app_config_t cfg = {
        .app_close = compass_app_close,
        .app_init = NULL,
        .app_kill = NULL,
        .app_load = compass_app_load,
        .app_power_off = NULL,
        .has_gui = 1,
        .icon = &compass_app_img_icon,
        .name = "指南针",
        .name_font = &compass_app_font_24};
    app_install(&cfg, NULL);
}