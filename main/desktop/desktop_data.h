#ifndef DESKTOP_DATA_H
#define DESKTOP_DATA_H
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    const lv_image_dsc_t * icon;   /* 真实图标 C 数组 */
    const char * name;             /* 中文名 */
    uint8_t page;                  /* 1 或 2（page_0 为锁屏） */
    void (*launch)(void);          /* 图标短按启动回调，可空 */
} desktop_app_t;
typedef struct { const char * title; const char * body; const char * time; } desktop_notify_t;

extern const desktop_app_t    desktop_apps[];
extern const uint32_t     desktop_app_count;      /* 12 */
extern const desktop_notify_t desktop_notifies[];
extern const uint32_t     desktop_notify_count;
#ifdef __cplusplus
}
#endif
#endif
