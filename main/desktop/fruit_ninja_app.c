#include "fruit_ninja_app.h"

#include "desktop_app_launcher.h"
#include "desktop_metrics.h"
#include "../src/v9-fruit_ninja/fruit_ninja_assets.h"
#include "../src/v9-fruit_ninja/fruit_ninja.h"

static void back_button_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        desktop_app_launcher_close();
    }
}

static void create_back_button(lv_obj_t * overlay)
{
    lv_obj_t * btn = lv_button_create(overlay);
    lv_obj_t * icon;
    char icon_path[512];

    lv_obj_set_size(btn, 48, 48);
    lv_obj_set_pos(btn, 16, 56);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_add_event_cb(btn, back_button_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_move_foreground(btn);

    icon = lv_image_create(btn);
    if(fruit_ninja_assets_build_image_path(icon_path, sizeof(icon_path), "images/back.png")) {
        lv_image_set_src(icon, icon_path);
    }
    lv_obj_center(icon);
}

static void create_overlay_background(lv_obj_t * overlay, int32_t screen_w, int32_t screen_h)
{
    lv_obj_t * bg = lv_image_create(overlay);
    char bg_path[512];

    lv_obj_set_size(bg, screen_w, screen_h);
    lv_obj_set_pos(bg, 0, 0);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);

    if(fruit_ninja_assets_build_image_path(bg_path, sizeof(bg_path), "images/background.jpg")) {
        lv_image_set_src(bg, bg_path);
        lv_image_set_inner_align(bg, LV_IMAGE_ALIGN_STRETCH);
    }
    lv_obj_move_background(bg);
}

static lv_obj_t * fruit_ninja_builder(lv_obj_t * overlay, int32_t screen_w, int32_t screen_h)
{
    lv_obj_set_style_bg_opa(overlay, LV_OPA_COVER, 0);
    lv_obj_t * game = fruit_ninja_create(overlay, screen_w, screen_h);
    create_overlay_background(overlay, screen_w, screen_h);
    lv_obj_move_foreground(game);
    create_back_button(overlay);
    return game;
}

void fruit_ninja_app_launch(void)
{
    const desktop_metrics_t * m = desktop_metrics();
    desktop_app_launcher_open_with_close(fruit_ninja_builder, fruit_ninja_app_close, m->screen_w, m->screen_h);
}

void fruit_ninja_app_close(void)
{
    fruit_ninja_stop();
}
