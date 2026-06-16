#ifndef FRUIT_NINJA_INTERNAL_H
#define FRUIT_NINJA_INTERNAL_H

#include "lvgl/lvgl.h"
#include "fruit_ninja_model.h"
#include "fruit_ninja_state.h"
#include "fruit_ninja_viewport.h"

/* —— 时间/物理常量(原 scene.c 顶部)—— */
#define FRUIT_NINJA_UPDATE_MS            16U
#define FRUIT_NINJA_EXPLODING_MS         4000U
#define FRUIT_NINJA_FLASH_MS             200U
#define FRUIT_NINJA_DROP_TIME_MS         1200U
#define FRUIT_NINJA_THROW_START_Y        560.0f
#define FRUIT_NINJA_JS_START_Y           600.0f
#define FRUIT_NINJA_GRAVITY              0.32f
#define FRUIT_NINJA_HOME_BG_COLOR        0x111111
#define FRUIT_NINJA_HOME_FLOAT_AMPLITUDE 8.0f
#define FRUIT_NINJA_HOME_SLICE_FEEDBACK_MS 240U
#define FRUIT_NINJA_SCORE_PULSE_MS       90U

/* —— 共享 helper(实现留在 scene.c)—— */
lv_obj_t * fruit_ninja_create_layer(lv_obj_t * parent);
bool       fruit_ninja_make_image_path(char * out, size_t out_size, const char * relative_path);
lv_obj_t * fruit_ninja_create_file_image(lv_obj_t * parent, const char * relative_path);
/* 几何 helper:入参为逻辑坐标(640x480 系),内部经 viewport 映射到物理屏。
 * 位置/尺寸均 letterbox 等比缩放,并对 lv_image 应用 viewport scale。*/
void       fruit_ninja_set_image_geometry(lv_obj_t * obj, int32_t x, int32_t y, int32_t w, int32_t h);
/* place_logic:统一"逻辑坐标 -> 物理映射 + 缩放 + 定位"。
 * lx/ly 为逻辑坐标;centered=true 时 lx/ly 视为对象中心(自动减去缩放后半宽高),
 * 否则视为左上角。scale_image=true 时对 lv_image 应用 viewport scale。*/
void       fruit_ninja_place_logic(lv_obj_t * obj, float lx, float ly, bool centered, bool scale_image);
void       fruit_ninja_hide_obj(lv_obj_t * obj);
void       fruit_ninja_show_obj(lv_obj_t * obj);
void       fruit_ninja_destroy_if_present(lv_obj_t ** obj);


#endif
