/**
 * @file home_page.h
 * @brief MasterGo home page module (layout skeleton)
 * @version 1.0
 * @date 2026-03-30
 * @copyright Copyright (c) 2026
 */
#ifndef HOME_PAGE_H
#define HOME_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"
#include <stdint.h>

typedef void VOID_T;
typedef int OPERATE_RET;

#define OPRT_OK 0
#define OPRT_INVALID_PARM (-1)

/* ---------------------------------------------------------------------------
 * Type definitions
 * --------------------------------------------------------------------------- */

/**
 * @brief Runtime state for the home page (two switch cards and root container)
 */
typedef struct {
    lv_obj_t * screen;
    lv_obj_t * weather_container;
    lv_obj_t * weather_icon;
    lv_obj_t * weather_temp_label;
    lv_obj_t * weather_date_label;
    lv_obj_t * weather_weekday_label;
    lv_obj_t * clock_container;
    lv_obj_t * clock_hour_label;
    lv_obj_t * clock_colon_image;
    lv_obj_t * clock_minute_label;
    lv_obj_t * switch_one_bg;
    lv_obj_t * switch_two_bg;
    lv_obj_t * switch_one_btn;
    lv_obj_t * switch_two_btn;
    lv_obj_t * switch_one_label;
    lv_obj_t * switch_two_label;
    uint8_t active_index;
} HOME_PAGE_CTX_T;

/* ---------------------------------------------------------------------------
 * Function declarations
 * --------------------------------------------------------------------------- */

/**
 * @brief Create the home page root container on the active display screen
 * @return @c OPRT_OK on success, @c OPRT_INVALID_PARM if LVGL is not ready
 */
OPERATE_RET home_page_create(VOID_T);

#ifdef __cplusplus
}
#endif

#endif /* HOME_PAGE_H */
