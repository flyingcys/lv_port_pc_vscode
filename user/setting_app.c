#include "setting_app.h"
#include "system_app.h"
#include "desktop.h"
#include "qf_zero_v2_gui_api_public.h"

LV_IMG_DECLARE(setting_app_icon);
LV_IMG_DECLARE(setting_app_img_logo);
LV_IMG_DECLARE(setting_app_img_add);
LV_IMG_DECLARE(setting_app_img_minus);

LV_FONT_DECLARE(setting_app_font_24);
LV_FONT_DECLARE(setting_app_font_22);
LV_FONT_DECLARE(setting_app_font_16);

static lv_obj_t *about_page = NULL;

typedef enum
{
    img_btn_down_bcd,
    img_btn_up_bcd,
    scr_rest_time_bcd,
    btn_about_bcd,
} user_flag_bcd_t;

static void scr_event(lv_event_t *e)
{
    if (e->code == LV_EVENT_GESTURE)
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
    if (e->code == LV_EVENT_VALUE_CHANGED)
    {
    }
    if (e->code == LV_EVENT_CLICKED)
    {
        if (public_lv_obj_get_bcd(e->target) == btn_about_bcd)
            lv_obj_clear_flag(about_page, LV_OBJ_FLAG_HIDDEN);
        return;
    }
}

static void slider_label_cb(lv_event_t *e)
{
    uint8_t bcd = public_lv_obj_get_bcd(e->target);
    lv_label_set_text_fmt((lv_obj_t *)e->user_data, "%d", lv_slider_get_value(e->target));

    if (bcd == scr_rest_time_bcd)
    {
        system_set_screen_rest(lv_slider_get_value(e->target));
        return;
    }
}

static void slider_img_btn_cb(lv_event_t *e)
{
    lv_obj_t *slider = (lv_obj_t *)e->user_data;

    int val_tmp = lv_slider_get_value(slider);
    user_flag_bcd_t bcd = public_lv_obj_get_bcd(e->target);

    if (public_lv_obj_get_bcd(slider) == scr_rest_time_bcd) // 息屏时长
    {
        if (bcd == img_btn_up_bcd) // 加按钮
        {
            if (val_tmp >= 100)
                val_tmp += 20;
            else if (val_tmp >= 20)
                val_tmp += 10;
            else
                val_tmp++;
        }
        else if (bcd == img_btn_down_bcd) // 加按钮
        {
            if (val_tmp <= 20)
                val_tmp--;
            else if (val_tmp <= 100)
                val_tmp -= 10;
            else
                val_tmp -= 20;
        }
        if (val_tmp > 300 || val_tmp < 5)
            return;
    }
    lv_slider_set_value(slider, val_tmp, LV_ANIM_OFF);
    lv_event_send(slider, LV_EVENT_VALUE_CHANGED, NULL);
}

static lv_obj_t *create_option_number(lv_obj_t *parent, const char *fmt, int min, int max, int num)
{

    lv_obj_t *cont = public_create_col_label(parent, fmt, &setting_app_font_24);

    lv_obj_t *img_minus = lv_img_create(cont);
    lv_obj_t *label = lv_label_create(cont);
    lv_obj_t *img_add = lv_img_create(cont);
    lv_obj_t *slider = lv_slider_create(cont);

    lv_img_set_src(img_minus, &setting_app_img_minus);
    lv_img_set_src(img_add, &setting_app_img_add);

    lv_obj_add_flag(img_add, LV_OBJ_FLAG_CLICKABLE);
    public_lv_obj_set_bcd(img_add, img_btn_up_bcd);
    public_lv_obj_set_bcd(img_minus, img_btn_down_bcd);
    lv_obj_add_flag(img_minus, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img_add, slider_img_btn_cb, LV_EVENT_CLICKED, slider);
    lv_obj_add_event_cb(img_minus, slider_img_btn_cb, LV_EVENT_CLICKED, slider);

    lv_obj_remove_style_all(slider);
    lv_obj_add_flag(slider, LV_OBJ_FLAG_HIDDEN);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, num, LV_ANIM_OFF);

    lv_obj_set_style_text_font(label, &setting_app_font_22, 0);
    lv_label_set_text_fmt(label, "%d", num);

    lv_obj_add_event_cb(slider, slider_label_cb, LV_EVENT_VALUE_CHANGED, label);

    return slider;
}

static void setting_app_load(void *arg)
{
    lv_obj_t *scr = public_create_menu_list(NULL);
    lv_obj_set_style_pad_row(scr, -5, LV_PART_MAIN);

    lv_obj_t *tmp;

    tmp = create_option_number(scr, "息屏秒", 5, 300, system_get_screen_rest());
    public_lv_obj_set_bcd(tmp, scr_rest_time_bcd);
    public_create_col_line(scr, lv_color_hex(0x2D2D2D), 90);

    tmp = public_create_col_label(scr, "关于", &setting_app_font_24);
    public_lv_obj_set_bcd(tmp, btn_about_bcd);
    lv_obj_add_event_cb(tmp, scr_event, LV_EVENT_CLICKED, NULL);

    about_page = lv_obj_create(scr);
    lv_obj_remove_style_all(about_page);
    lv_obj_set_style_bg_opa(about_page, 255, 0);
    lv_obj_set_style_bg_color(about_page, lv_obj_get_style_bg_color(scr, 0), 0);
    lv_obj_set_size(about_page, LV_HOR_RES, LV_VER_RES);

    lv_obj_t *img = lv_img_create(about_page);

    lv_img_set_src(img, &setting_app_img_logo);
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *label = lv_label_create(about_page);
    lv_obj_set_style_text_font(label, &setting_app_font_16, 0);
    uint8_t ver[3];
    system_get_soft_version(ver);
    uint8_t hw_v[2] = {2, 2};
#if software_version_release
    lv_label_set_text_fmt(label, "启凡科创\nQF-ZERO-V2\n固件:V%d.%d.%d\n硬件:V%d.%d\n", ver[0], ver[1], ver[2], hw_v[0], hw_v[1]);
#else
    lv_label_set_text_fmt(label, "启凡科创\nQF-ZERO-V2\n固件:V%d.%d.%d Dev\n硬件:V%d.%d\n", ver[0], ver[1], ver[2], hw_v[0], hw_v[1]);
#endif
    lv_obj_align(label, LV_ALIGN_CENTER, 10, 75);

    lv_obj_add_event_cb(scr, scr_event, LV_EVENT_GESTURE, NULL);
    lv_event_send(scr, LV_EVENT_SCROLL, NULL);
    lv_obj_scroll_to_view(lv_obj_get_child(scr, 0), LV_ANIM_OFF);

    app_scr_load(scr, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0); // 加载APP界面
}

void setting_app_install()
{
    app_config_t cfg = {
        .app_close = NULL,
        .app_init = NULL,
        .app_kill = NULL,
        .app_load = setting_app_load,
        .app_power_off = NULL,
        .has_gui = 1,
        .icon = &setting_app_icon,
        .name = "设置",
        .name_font = &setting_app_font_24};
    app_install(&cfg, NULL);
}
