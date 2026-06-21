#include "desktop_app_launcher.h"

static lv_obj_t * s_overlay = NULL;
static void (*s_close_cb)(void) = NULL;
static bool s_screen_was_scrollable = false;
static lv_scrollbar_mode_t s_screen_scrollbar_mode = LV_SCROLLBAR_MODE_AUTO;

lv_obj_t * desktop_app_launcher_open(lv_obj_t *(*builder)(lv_obj_t *, int32_t, int32_t),
                                     int32_t screen_w, int32_t screen_h)
{
    return desktop_app_launcher_open_with_close(builder, NULL, screen_w, screen_h);
}

lv_obj_t * desktop_app_launcher_open_with_close(lv_obj_t *(*builder)(lv_obj_t *, int32_t, int32_t),
                                                void (*close_cb)(void),
                                                int32_t screen_w,
                                                int32_t screen_h)
{
    if(s_overlay != NULL) {
        return s_overlay;
    }
    lv_obj_t * screen = lv_screen_active();
    s_screen_was_scrollable = lv_obj_has_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    s_screen_scrollbar_mode = lv_obj_get_scrollbar_mode(screen);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);

    s_overlay = lv_obj_create(screen);
    if(s_overlay == NULL) return NULL;
    lv_obj_remove_style_all(s_overlay);
    lv_obj_set_pos(s_overlay, 0, 0);
    lv_obj_set_size(s_overlay, screen_w, screen_h);
    lv_obj_set_style_bg_color(s_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_overlay, 160, 0);
    lv_obj_clear_flag(s_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_overlay, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(s_overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_move_foreground(s_overlay);
    s_close_cb = close_cb;
    if(builder != NULL) {
        lv_obj_t * app = builder(s_overlay, screen_w, screen_h);
        (void)app;
    }
    return s_overlay;
}

void desktop_app_launcher_close(void)
{
    if(s_overlay == NULL) return;
    if(s_close_cb != NULL) {
        s_close_cb();
        s_close_cb = NULL;
    }
    lv_obj_delete(s_overlay);
    s_overlay = NULL;
    if(s_screen_was_scrollable) {
        lv_obj_add_flag(lv_screen_active(), LV_OBJ_FLAG_SCROLLABLE);
    }
    lv_obj_set_scrollbar_mode(lv_screen_active(), s_screen_scrollbar_mode);
    s_screen_was_scrollable = false;
    s_screen_scrollbar_mode = LV_SCROLLBAR_MODE_AUTO;
}

bool desktop_app_launcher_is_open(void)
{
    return s_overlay != NULL;
}
