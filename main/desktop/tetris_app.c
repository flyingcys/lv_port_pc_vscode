#include "tetris_app.h"

#include "desktop_app_launcher.h"
#include "desktop_metrics.h"
#include "../src/v9-tetris/tetris.h"

static void back_button_event_cb(lv_event_t *e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        desktop_app_launcher_close();
    }
}

static void create_back_button(lv_obj_t *overlay)
{
    lv_obj_t *btn = lv_button_create(overlay);
    lv_obj_t *label;

    lv_obj_set_size(btn, 44, 44);
    lv_obj_set_pos(btn, 12, 12);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(btn, 92, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_opa(btn, 128, 0);
    lv_obj_set_style_shadow_width(btn, 10, 0);
    lv_obj_set_style_shadow_opa(btn, 40, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_add_event_cb(btn, back_button_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_move_foreground(btn);

    label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(label, lv_color_hex(0x1f2937), 0);
    lv_obj_center(label);
}

static lv_obj_t * tetris_builder(lv_obj_t *overlay, int32_t screen_w, int32_t screen_h)
{
    lv_obj_t *game = tetris_create(overlay, screen_w, screen_h);
    create_back_button(overlay);
    tetris_start();
    return game;
}

void tetris_app_launch(void)
{
    const desktop_metrics_t *m = desktop_metrics();
    desktop_app_launcher_open_with_close(tetris_builder,
                                         tetris_app_close,
                                         m->screen_w,
                                         m->screen_h);
}

void tetris_app_close(void)
{
    tetris_stop();
}
