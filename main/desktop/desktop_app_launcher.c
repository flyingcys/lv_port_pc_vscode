#include "desktop_app_launcher.h"

static lv_obj_t * s_overlay = NULL;

lv_obj_t * desktop_app_launcher_open(lv_obj_t *(*builder)(lv_obj_t *, int32_t, int32_t),
                                     int32_t screen_w, int32_t screen_h)
{
    if(s_overlay != NULL) {
        return s_overlay;
    }
    s_overlay = lv_obj_create(lv_screen_active());
    if(s_overlay == NULL) return NULL;
    lv_obj_remove_style_all(s_overlay);
    lv_obj_set_pos(s_overlay, 0, 0);
    lv_obj_set_size(s_overlay, screen_w, screen_h);
    lv_obj_set_style_bg_color(s_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_overlay, 160, 0);
    lv_obj_clear_flag(s_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(s_overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_move_foreground(s_overlay);
    if(builder != NULL) {
        lv_obj_t * app = builder(s_overlay, screen_w, screen_h);
        (void)app;
    }
    return s_overlay;
}

void desktop_app_launcher_close(void)
{
    if(s_overlay == NULL) return;
    lv_obj_delete(s_overlay);
    s_overlay = NULL;
}

bool desktop_app_launcher_is_open(void)
{
    return s_overlay != NULL;
}
