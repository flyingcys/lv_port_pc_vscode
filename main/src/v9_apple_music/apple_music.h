/* main/src/v9_apple_music/apple_music.h */
#ifndef APPLE_MUSIC_H
#define APPLE_MUSIC_H

#include "lvgl/lvgl.h"

/* 在当前活动屏幕上构建整个 Apple Music UI(800x480) */
void apple_music_create(void);
void apple_music_create_in(lv_obj_t *parent);
void apple_music_destroy(void);

#endif /* APPLE_MUSIC_H */
