/**
 * @file scenes_assets.h
 * @brief Scenes page asset paths and design tokens
 * @version 1.0
 * @date 2026-03-31
 * @copyright Copyright (c) 2026
 */
#ifndef __SCENES_ASSETS_H__
#define __SCENES_ASSETS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "scenes_state.h"
#include <stdbool.h>
#include "lvgl/lvgl.h"

/* ---------------------------------------------------------------------------
 * Macros
 * --------------------------------------------------------------------------- */
#define SCENES_ASSETS_FS_LETTER 'A'

#define SCENES_PAGE_WIDTH 480
#define SCENES_PAGE_HEIGHT 480

#define SCENES_COLOR_PAGE_BG 0x000000U
#define SCENES_COLOR_TEXT 0xFFFFFFU
#define SCENES_COLOR_ACTIVE 0x41CDE2U
#define SCENES_COLOR_WEATHER_CLOUD 0xF0F2F5U
#define SCENES_COLOR_WEATHER_SUN 0xFF7B30U

#define SCENES_ASSETS_PATH_FONT_URBANIST_BOLD "A:main/assets/fonts/Urbanist-Bold.ttf"
#define SCENES_ASSETS_PATH_FONT_NOTO_BOLD "A:main/assets/fonts/NotoSansCJKSC-Bold.ttc"
#define SCENES_ASSETS_PATH_FONT_NOTO_REGULAR "A:main/assets/fonts/NotoSansCJKSC-Regular.ttc"

#define SCENES_ASSETS_PATH_WEATHER_PNG "A:main/assets/scenes/weather.png"
#define SCENES_ASSETS_PATH_WIFI_PNG "A:main/assets/scenes/wifi.png"
#define SCENES_ASSETS_PATH_CARD_HOME_PNG "A:main/assets/scenes/card-home.png"
#define SCENES_ASSETS_PATH_CARD_LEAVING_HOME_PNG "A:main/assets/scenes/card-leaving-home.png"
#define SCENES_ASSETS_PATH_CARD_MORNING_PNG "A:main/assets/scenes/card-morning.png"
#define SCENES_ASSETS_PATH_CARD_SELECTED_PNG "A:main/assets/scenes/card-selected.png"
#define SCENES_ASSETS_PATH_CARD_ALL_LIGHTS_ON_PNG "A:main/assets/scenes/card-all-lights-on.png"

#define SCENES_FONT_TITLE_PX 32
#define SCENES_FONT_TEMP_PX 22
#define SCENES_FONT_TIME_PX 26
#define SCENES_FONT_META_PX 10
#define SCENES_FONT_CARD_TITLE_PX 18

/* ---------------------------------------------------------------------------
 * Function declarations
 * --------------------------------------------------------------------------- */
/**
 * @brief Get LVGL filesystem path for the scenes weather icon
 * @return Pointer to static path string with A: prefix
 */
const char * scenes_assets_get_path_weather_png(void);

/**
 * @brief Get LVGL filesystem path for the scenes Wi-Fi icon
 * @return Pointer to static path string with A: prefix
 */
const char * scenes_assets_get_path_wifi_png(void);

/**
 * @brief Get LVGL filesystem path for a scene card icon
 * @param[in] card Card index to query
 * @param[in] selected Whether selected-state asset should be used
 * @return Pointer to static path string with A: prefix
 */
const char * scenes_assets_get_path_card_icon(SCENES_CARD_E card, bool selected);

/**
 * @brief Return cached bold title font for page headings
 * @return Valid font pointer
 */
lv_font_t * scenes_assets_font_cached_title_bold(void);

/**
 * @brief Return cached regular title font for inactive tab headings
 * @return Valid font pointer
 */
lv_font_t * scenes_assets_font_cached_title_regular(void);

/**
 * @brief Return cached Urbanist font for temperature text
 * @return Valid font pointer
 */
lv_font_t * scenes_assets_font_cached_temp(void);

/**
 * @brief Return cached Urbanist font for time text
 * @return Valid font pointer
 */
lv_font_t * scenes_assets_font_cached_time(void);

/**
 * @brief Return cached small meta font for AM and date text
 * @return Valid font pointer
 */
lv_font_t * scenes_assets_font_cached_meta(void);

/**
 * @brief Return cached card title font for scene cards
 * @return Valid font pointer
 */
lv_font_t * scenes_assets_font_cached_card_title(void);

#ifdef __cplusplus
}
#endif

#endif /* __SCENES_ASSETS_H__ */
