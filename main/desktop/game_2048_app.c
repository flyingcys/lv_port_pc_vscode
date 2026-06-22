#include "game_2048_app.h"

#include "desktop_app_launcher.h"
#include "desktop_metrics.h"
#include "../src/v9_2048/game_2048.h"

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
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(btn, 96, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 8, 0);
    lv_obj_set_style_shadow_opa(btn, 32, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_add_event_cb(btn, back_button_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_move_foreground(btn);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(label, lv_color_hex(0x776E65), 0);
    lv_obj_center(label);
}

static lv_obj_t *game_2048_builder(lv_obj_t *overlay, int32_t screen_w, int32_t screen_h)
{
    lv_obj_t *game = game_2048_create(overlay, screen_w, screen_h);
    create_back_button(overlay);
    game_2048_start();
    return game;
}

void game_2048_app_launch(void)
{
    const desktop_metrics_t *m = desktop_metrics();
    desktop_app_launcher_open_with_close(game_2048_builder,
                                         game_2048_app_close,
                                         m->screen_w,
                                         m->screen_h);
}

void game_2048_app_close(void)
{
    game_2048_stop();
}
