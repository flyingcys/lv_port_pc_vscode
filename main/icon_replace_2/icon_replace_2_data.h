#ifndef ICON_REPLACE_2_DATA_H
#define ICON_REPLACE_2_DATA_H
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    const lv_image_dsc_t * icon;   /* 真实图标 C 数组 */
    const char * name;             /* 中文名 */
    uint8_t page;                  /* 1 或 2（page_0 为锁屏） */
} ir2_app_t;
typedef struct { const char * title; const char * body; const char * time; } ir2_notify_t;

extern const ir2_app_t    ir2_apps[];
extern const uint32_t     ir2_app_count;      /* 12 */
extern const ir2_notify_t ir2_notifies[];
extern const uint32_t     ir2_notify_count;
#ifdef __cplusplus
}
#endif
#endif
