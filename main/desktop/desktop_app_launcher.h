#ifndef DESKTOP_APP_LAUNCHER_H
#define DESKTOP_APP_LAUNCHER_H
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * desktop_app_launcher_open(lv_obj_t *(*builder)(lv_obj_t * overlay,
                                                          int32_t screen_w,
                                                          int32_t screen_h),
                                     int32_t screen_w, int32_t screen_h);
lv_obj_t * desktop_app_launcher_open_with_close(lv_obj_t *(*builder)(lv_obj_t * overlay,
                                                                     int32_t screen_w,
                                                                     int32_t screen_h),
                                                void (*close_cb)(void),
                                                int32_t screen_w,
                                                int32_t screen_h);
void desktop_app_launcher_close(void);
bool desktop_app_launcher_is_open(void);

#ifdef __cplusplus
}
#endif
#endif
