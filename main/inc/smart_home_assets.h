#ifndef SMART_HOME_ASSETS_H
#define SMART_HOME_ASSETS_H

#include <stdbool.h>

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

bool smart_home_assets_init(void);
void smart_home_assets_deinit(void);

const lv_font_t * smart_home_font_title_bold_32(void);
const lv_font_t * smart_home_font_title_regular_32(void);
const lv_font_t * smart_home_font_metric_bold_26(void);
const lv_font_t * smart_home_font_metric_bold_22(void);
const lv_font_t * smart_home_font_body_regular_18(void);
const lv_font_t * smart_home_font_body_regular_15(void);
const lv_font_t * smart_home_font_caption_bold_10(void);

#ifdef __cplusplus
}
#endif

#endif
