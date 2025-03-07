#include "control_center_app.h"
#include "desktop.h"
#include "system_app.h"

LV_IMG_DECLARE(control_center_app_img_set_blk);
LV_IMG_DECLARE(control_center_app_img_prtsc);
// LV_IMG_DECLARE(control_center_app_img_temp);
// LV_IMG_DECLARE(control_center_app_img_e);
// LV_IMG_DECLARE(control_center_app_img_range);
LV_IMG_DECLARE(control_center_app_img_light);
// LV_IMG_DECLARE(control_center_app_img_max);
// LV_IMG_DECLARE(control_center_app_img_min);

LV_FONT_DECLARE(control_center_app_font_20);

typedef enum
{
    obj_id_light, // 按键
    obj_id_prtsc,

    obj_id_blk, // 滑动条
} obj_id_t;

#define page_num 1

static uint8_t chek_enable = 0;
static RTC_FAST_ATTR uint8_t load_page = 0;

static lv_obj_t *creat_page(uint8_t num);

static void lv_obj_set_bcd(lv_obj_t *obj, uint8_t bcd)
{
    if (bcd & 1)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_1);
    if (bcd & 2)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_2);
    if (bcd & 4)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_3);
    if (bcd & 8)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_4);
}

static uint8_t lv_obj_get_bcd(lv_obj_t *obj)
{
    uint8_t bcd = 0;
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_1))
        bcd |= 1;
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_2))
        bcd |= 2;
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_3))
        bcd |= 4;
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_4))
        bcd |= 8;
    return bcd;
}

static void btn_set_bg_color(lv_obj_t *obj, uint8_t chek_en)
{
    if (chek_en)
        lv_obj_set_style_bg_color(obj, lv_color_hex(0xF8B428), 0);
    else
        lv_obj_set_style_bg_color(obj, lv_color_hex(0X515151), 0);
}

static void light_event_cb(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_TOP || dir == LV_DIR_BOTTOM)
        return;

    uint8_t blk = *(uint8_t *)e->user_data;
    key_value_msg("sys_set_blk", &blk, 1);
    lv_obj_del(e->target);
}

static void slider_event_cb(lv_event_t *e)
{
    uint8_t id = lv_obj_get_bcd(e->target);
    lv_obj_t *child = (lv_obj_t *)e->user_data;

    if (chek_enable == 0) // 未滑动界面
    {
        if (id == obj_id_blk)
        {
            lv_slider_set_value(e->target, system_get_blk(), LV_ANIM_OFF);
            return;
        }

        return;
    }

    if (id == obj_id_blk)
    {
        uint16_t tmp = lv_slider_get_value(e->target);
        system_set_blk(tmp);
        lv_img_set_angle(child, tmp * 18);
        return;
    }
}

static void btn_click_cb(lv_event_t *e)
{
    if (system_get_tp_type())
        return;
    uint8_t id = lv_obj_get_bcd(e->target);

    if (id == obj_id_prtsc)
    {
        uint16_t delay = 30;
        key_value_msg("take_prtscr", &delay, sizeof(delay));

        lv_scr_load_anim_t anim = LV_SCR_LOAD_ANIM_NONE;
        key_value_msg("sys_home", &anim, sizeof(anim));

        return;
    }

    if (id == obj_id_light)
    {
        lv_obj_t *scr = lv_obj_create(lv_obj_get_parent(e->target));
        lv_obj_set_size(scr, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_outline_width(scr, 0, 0);
        lv_obj_set_style_border_width(scr, 0, 0);
        lv_obj_set_style_radius(scr, 0, 0);
        lv_obj_set_style_bg_color(scr, lv_color_hex(0xffffff), 0);
        lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(scr, LV_OBJ_FLAG_GESTURE_BUBBLE);
        lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);
        static uint8_t blk = 100;
        key_value_msg("sys_get_blk", &blk, 1);
        system_set_blk(100);
        lv_obj_add_event_cb(scr, light_event_cb, LV_EVENT_GESTURE, &blk);
        return;
    }
}

static void scr_event(lv_event_t *e)
{
    if (e->code == LV_EVENT_PRESSED)
    {
        if (system_get_tp_type())
            return;
        chek_enable = 1;
        return;
    }
    if (e->code == LV_EVENT_RELEASED)
    {
        chek_enable = 0;
        return;
    }

    if (e->code == LV_EVENT_GESTURE)
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        cst816_point_t point;
        cst816_read_point(&point);

        if (dir == LV_DIR_TOP && point.start_y > 220) // 上滑关闭
        {
            lv_scr_load_anim_t anim = LV_SCR_LOAD_ANIM_MOVE_TOP;
            app_load("desktop", app_handle_none, &anim);
            return;
        }
        // printf("%d\n",point.start_x);
        if (dir == LV_DIR_LEFT && point.start_x > 230)
        {
#if page_num > 1
            if (load_page >= (page_num - 1))
                return;
#endif
            load_page++;
            lv_scr_load_anim(creat_page(load_page), LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 0, 0);
            return;
        }
        if (dir == LV_DIR_RIGHT && point.start_x < 40)
        {
            if (load_page == 0)
                return;
            load_page--;
            lv_scr_load_anim(creat_page(load_page), LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 0, 0);
            return;
        }
    }
}

static lv_obj_t *create_btn(lv_obj_t *par, const void *img, obj_id_t id, bool chek_en)
{
    lv_obj_t *btn = lv_btn_create(par);
    btn_set_bg_color(btn, chek_en);

    lv_obj_set_size(btn, 60, 60);
    lv_obj_set_style_radius(btn, 15, 0);
    lv_obj_t *_img = lv_img_create(btn);
    lv_img_set_src(_img, img);
    lv_obj_center(_img);

    lv_obj_set_bcd(btn, id);
    lv_obj_add_event_cb(btn, btn_click_cb, LV_EVENT_CLICKED, NULL);

    return btn;
}

static lv_obj_t *create_slider(lv_obj_t *par, const void *img, lv_obj_t **label, obj_id_t id, int min, int max, int value, uint16_t w, uint16_t h)
{
    lv_obj_t *slider = lv_slider_create(par);
    lv_obj_set_size(slider, w, h);

    lv_obj_t *_img = lv_img_create(slider);
    lv_img_set_src(_img, img);
    lv_obj_align(_img, LV_ALIGN_BOTTOM_MID, 0, -(w - 32) / 2);

    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, value, LV_ANIM_ON);
    lv_obj_set_style_bg_opa(slider, 0, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(slider, 255, LV_PART_MAIN);
    if (w < h)
        lv_obj_set_style_radius(slider, w / 4, LV_PART_MAIN);
    else
        lv_obj_set_style_radius(slider, h / 4, LV_PART_MAIN);
    lv_obj_set_style_radius(slider, 0, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0X515151), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0xffffff), LV_PART_INDICATOR);

    if (label != NULL)
    {
        *label = lv_label_create(slider);
        lv_obj_set_style_text_font(*label, &control_center_app_font_20, 0);
        lv_obj_set_style_text_color(*label, lv_color_hex(0X208CC0), 0);
        lv_obj_align(*label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, *label);
    }
    else
        lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, _img);

    lv_obj_add_event_cb(slider, scr_event, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(slider, scr_event, LV_EVENT_RELEASED, NULL);

    lv_obj_set_bcd(slider, id);
    return slider;
}

static void create_close_line(lv_obj_t *par, uint8_t num)
{
    lv_obj_t *cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, 60, 8);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, -9);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_flex_main_place(cont, LV_FLEX_ALIGN_SPACE_EVENLY, 0);

    // uint16_t width = (60 - (page_num - 1) * 15) / page_num;
    for (size_t i = 0; i < page_num; i++)
    {
        lv_obj_t *line = lv_obj_create(cont);
        lv_obj_remove_style_all(line);
        lv_obj_set_style_bg_opa(line, 255, 0);
        lv_obj_set_size(line, 8, 8);
        lv_obj_set_style_radius(line, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_scrollbar_mode(line, LV_SCROLLBAR_MODE_OFF);
        if (i == num)
            lv_obj_set_style_bg_color(line, lv_color_hex(0xF8B428), 0);
        else
            lv_obj_set_style_bg_color(line, lv_color_hex(0x808080), 0);
        lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE);

        // lv_obj_set_style_outline_opa(line,0,0);
        //  lv_obj_set_style_pad_row(line, 10, 0);
        //  lv_obj_set_style_pad_column(line, 10, 0);
    }
}

static void create_page0(lv_obj_t *scr)
{

    lv_obj_align(create_slider(scr, &control_center_app_img_set_blk, NULL, obj_id_blk, 0, 100, system_get_blk(), 60, 160), LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(create_btn(scr, &control_center_app_img_prtsc, obj_id_prtsc, 0), LV_ALIGN_RIGHT_MID, -15, 0);
    lv_obj_align(create_btn(scr, &control_center_app_img_light, obj_id_light, 0), LV_ALIGN_LEFT_MID, 15, 0);
}

static lv_obj_t *creat_page(uint8_t num)
{
    chek_enable = 0;

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_size(scr, LV_HOR_RES, LV_VER_RES);

    if (num == 0)
        create_page0(scr);

    create_close_line(scr, num);

    lv_obj_set_bcd(scr, num);

    lv_obj_add_event_cb(scr, scr_event, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(scr, scr_event, LV_EVENT_RELEASED, NULL);
    return scr;
}

static lv_obj_t *status_bar_load()
{
    return creat_page(load_page);
}

void control_center_app_install()
{
    desktop_register_control_center(status_bar_load);
}
