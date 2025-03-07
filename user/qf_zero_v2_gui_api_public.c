#include "qf_zero_v2_gui_api_public.h"

void public_lv_obj_set_bcd(lv_obj_t *obj, uint8_t bcd)
{
    if (bcd & 1)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_1);
    else
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_USER_1);
    if (bcd & 2)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_2);
    else
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_USER_2);
    if (bcd & 4)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_3);
    else
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_USER_3);
    if (bcd & 8)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_4);
    else
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_USER_4);
}

uint8_t public_lv_obj_get_bcd(lv_obj_t *obj)
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

lv_obj_t *public_create_col(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_opa(cont, 0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_outline_width(cont, 0, 0);
    lv_obj_set_size(cont, 220, LV_SIZE_CONTENT);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    return cont;
}

lv_obj_t *public_create_col_label(lv_obj_t *parent, const char *fmt, const lv_font_t *font)
{
    lv_obj_t *cont = public_create_col(parent);

    lv_obj_t *label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, font, 0);
    lv_label_set_text(label, fmt);

    return cont;
}

lv_obj_t *public_create_col_img_label(lv_obj_t *parent, const void *img, const char *fmt, const lv_font_t *font)
{
    lv_obj_t *cont = public_create_col(parent);
    lv_obj_t *_img = lv_img_create(cont);
    lv_img_set_src(_img, img);
    if (fmt != NULL && font != NULL)
    {
        lv_obj_t *label = lv_label_create(cont);
        lv_obj_set_style_text_font(label, font, 0);
        lv_label_set_text(label, fmt);
    }
    return cont;
}

lv_obj_t *public_create_col_line(lv_obj_t *parent, lv_color_t color, uint8_t width)
{
    lv_obj_t *bar = lv_bar_create(parent);
    lv_obj_set_size(bar, lv_pct(width), 1);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 8);
    lv_bar_set_range(bar, 0, 1);
    lv_bar_set_value(bar, 1, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, 255, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, 0, LV_PART_INDICATOR);
    lv_obj_set_style_outline_width(bar, 0, 0);
    lv_obj_set_style_border_width(bar, 0, 0);

    return bar;
}

lv_obj_t *public_create_col_btn(lv_obj_t *parent, const char *fmt, const lv_font_t *font)
{
    lv_obj_t *cont = public_create_col_label(parent, fmt, font);
    return cont;
}

lv_obj_t *public_create_sw(lv_obj_t *parent, const char *fmt, const lv_font_t *font)
{
    lv_obj_t *cont = public_create_col_label(parent, fmt, font);

    lv_obj_t *_switch = lv_switch_create(cont);
    lv_obj_set_size(_switch, 48, 24);
    lv_obj_set_ext_click_area(_switch, 20);
    lv_obj_set_style_bg_color(_switch, lv_color_hex(0xF6B428), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_align(_switch, LV_ALIGN_RIGHT_MID, 0, 0);

    return _switch;
}

lv_obj_t *public_create_dp(lv_obj_t *parent, const char *fmt, const char *list, lv_font_t *name_font, lv_font_t *list_font)
{

    lv_obj_t *cont = public_create_col_label(parent, fmt, name_font);

    lv_obj_t *dp = lv_dropdown_create(cont);
    lv_obj_set_style_text_font(dp, list_font, 0);
    lv_dropdown_set_symbol(dp, "▽");

    lv_obj_t *list_obj = lv_dropdown_get_list(dp);
    lv_obj_set_style_text_font(list_obj, list_font, 0);

    lv_obj_set_size(dp, 80, 32);
    lv_obj_set_ext_click_area(dp, 20);
    lv_obj_set_style_outline_color(dp, lv_color_hex(0xF6B428), 0);
    lv_obj_set_style_outline_width(dp, 3, 0);
    lv_dropdown_set_options(dp, list);
    lv_obj_align(dp, LV_ALIGN_RIGHT_MID, 0, 0);

    return dp;
}

static void public_list_scroll_event_cb(lv_event_t *e)
{
    lv_obj_t *cont = lv_event_get_target(e);

    lv_area_t cont_a;
    lv_obj_get_coords(cont, &cont_a);
    lv_coord_t cont_y_center = cont_a.y1 + lv_area_get_height(&cont_a) / 2;

    lv_coord_t r = lv_obj_get_height(cont) * 7 / 10;
    uint32_t i;
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    for (i = 0; i < child_cnt; i++)
    {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        lv_area_t child_a;
        lv_obj_get_coords(child, &child_a);

        lv_coord_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;

        lv_coord_t diff_y = child_y_center - cont_y_center;
        diff_y = LV_ABS(diff_y);

        /*Get the x of diff_y on a circle.*/
        lv_coord_t x;
        /*If diff_y is out of the circle use the last point of the circle (the radius)*/
        if (diff_y >= r)
        {
            x = r;
        }
        else
        {
            /*Use Pythagoras theorem to get x from radius and y*/
            uint32_t x_sqr = r * r - diff_y * diff_y;
            lv_sqrt_res_t res;
            lv_sqrt(x_sqr, &res, 0x8000); /*Use lvgl's built in sqrt root function*/
            x = r - res.i;
        }

        /*Translate the item by the calculated X coordinate*/
        lv_obj_set_style_translate_x(child, x, 0);

        /*Use some opacity with larger translations*/
        lv_opa_t opa = lv_map(x, 0, r, LV_OPA_TRANSP, LV_OPA_COVER);
        lv_obj_set_style_opa(child, LV_OPA_COVER - opa, 0);
    }
}

lv_obj_t *public_create_menu_list(lv_obj_t *parent)
{

    lv_obj_t *scr = lv_obj_create(parent);
#if GUI_SIMULATOR_SPACE
    lv_obj_set_style_radius(scr, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(scr, true, 0);
#endif
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_size(scr, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_pad_all(scr, 5, 0);
    lv_obj_set_style_pad_row(scr, -10, LV_PART_MAIN);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_ON);
    lv_obj_add_event_cb(scr, public_list_scroll_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_set_scroll_dir(scr, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(scr, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);
    return scr;
}
