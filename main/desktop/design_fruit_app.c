#include "design_fruit_app.h"

#include "desktop_app_launcher.h"
#include "desktop_metrics.h"
#include "../src/design_fruit/design_fruit.h"

static void back_button_event_cb(lv_event_t *e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        desktop_app_launcher_close();
    }
}

static void create_back_button(lv_obj_t *overlay)
{
    lv_obj_t *btn = lv_button_create(overlay);
    lv_obj_set_size(btn, 44, 44);
    lv_obj_set_pos(btn, 12, 12);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(btn, 96, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_opa(btn, 128, 0);
    lv_obj_set_style_shadow_width(btn, 8, 0);
    lv_obj_set_style_shadow_opa(btn, 40, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_add_event_cb(btn, back_button_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_move_foreground(btn);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(label, lv_color_hex(0x1f2937), 0);
    lv_obj_center(label);
}

static lv_obj_t *design_fruit_builder(lv_obj_t *overlay, int32_t screen_w, int32_t screen_h)
{
    lv_obj_set_style_bg_opa(overlay, LV_OPA_COVER, 0);
    lv_obj_t *game = design_fruit_create(overlay, screen_w, screen_h);
    create_back_button(overlay);
    return game;
}

void design_fruit_app_launch(void)
{
    const desktop_metrics_t *m = desktop_metrics();
    desktop_app_launcher_open_with_close(design_fruit_builder,
                                         design_fruit_app_close,
                                         m->screen_w,
                                         m->screen_h);
}

void design_fruit_app_close(void)
{
    design_fruit_stop();
}
