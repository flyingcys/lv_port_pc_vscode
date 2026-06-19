/* main/src/v9_apple_music/am_page_local.h */
#ifndef AM_PAGE_LOCAL_H
#define AM_PAGE_LOCAL_H

#include "lvgl/lvgl.h"
#include "am_config.h"

/* 扫描 cfg->local_dir，构建本地文件列表页（点击播放）*/
lv_obj_t *am_page_local_create(lv_obj_t *content_parent, const am_config_t *cfg);

/* 从 cfg->radio[] 构建广播频道列表页（点击切流）*/
lv_obj_t *am_page_radio_create(lv_obj_t *content_parent, const am_config_t *cfg);

#endif /* AM_PAGE_LOCAL_H */
