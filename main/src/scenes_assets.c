/**
 * @file scenes_assets.c
 * @brief Scenes page asset path helpers
 * @version 1.0
 * @date 2026-03-31
 * @copyright Copyright (c) 2026
 */

#include "scenes_assets.h"

/* ---------------------------------------------------------------------------
 * File scope variables
 * --------------------------------------------------------------------------- */
LV_FONT_DECLARE(lv_font_montserrat_12);
LV_FONT_DECLARE(lv_font_montserrat_22);
LV_FONT_DECLARE(lv_font_montserrat_26);
LV_FONT_DECLARE(lv_font_montserrat_32);

static lv_font_t * s_title_bold_ttf;
static lv_font_t * s_title_regular_ttf;
static lv_font_t * s_temp_ttf;
static lv_font_t * s_time_ttf;
static lv_font_t * s_meta_ttf;
static lv_font_t * s_card_title_ttf;

/* ---------------------------------------------------------------------------
 * Forward declarations
 * --------------------------------------------------------------------------- */
static lv_font_t * __scenes_assets_create_font(const char * path, int32_t font_px);

/* ---------------------------------------------------------------------------
 * Function implementations
 * --------------------------------------------------------------------------- */
/**
 * @brief Create Tiny TTF font from file path and size
 * @param[in] path Font file path with LVGL filesystem prefix
 * @param[in] font_px Requested font size in pixels
 * @return Font pointer, or NULL on failure
 */
static lv_font_t * __scenes_assets_create_font(const char * path, int32_t font_px)
{
    if (path == NULL || font_px <= 0) {
        return NULL;
    }
    return lv_tiny_ttf_create_file(path, font_px);
}

/**
 * @brief Get LVGL filesystem path for the scenes weather icon
 * @return Pointer to static path string with A: prefix
 */
const char * scenes_assets_get_path_weather_png(void)
{
    return SCENES_ASSETS_PATH_WEATHER_PNG;
}

/**
 * @brief Get LVGL filesystem path for the scenes Wi-Fi icon
 * @return Pointer to static path string with A: prefix
 */
const char * scenes_assets_get_path_wifi_png(void)
{
    return SCENES_ASSETS_PATH_WIFI_PNG;
}

/**
 * @brief Get LVGL filesystem path for a scene card icon
 * @param[in] card Card index to query
 * @param[in] selected Whether selected-state asset should be used
 * @return Pointer to static path string with A: prefix
 */
const char * scenes_assets_get_path_card_icon(SCENES_CARD_E card, bool selected)
{
    if (selected) {
        return SCENES_ASSETS_PATH_CARD_SELECTED_PNG;
    }

    switch (card) {
        case SCENES_CARD_HOME:
            return SCENES_ASSETS_PATH_CARD_HOME_PNG;
        case SCENES_CARD_LEAVING_HOME:
            return SCENES_ASSETS_PATH_CARD_LEAVING_HOME_PNG;
        case SCENES_CARD_MORNING:
            return SCENES_ASSETS_PATH_CARD_MORNING_PNG;
        case SCENES_CARD_ALL_LIGHTS_ON:
            return SCENES_ASSETS_PATH_CARD_ALL_LIGHTS_ON_PNG;
        case SCENES_CARD_COUNT:
        default:
            return SCENES_ASSETS_PATH_CARD_HOME_PNG;
    }
}

/**
 * @brief Return cached bold title font for page headings
 * @return Valid font pointer
 */
lv_font_t * scenes_assets_font_cached_title_bold(void)
{
    if (s_title_bold_ttf == NULL) {
        s_title_bold_ttf = __scenes_assets_create_font(
            SCENES_ASSETS_PATH_FONT_NOTO_BOLD,
            SCENES_FONT_TITLE_PX
        );
    }
    if (s_title_bold_ttf != NULL) {
        return s_title_bold_ttf;
    }
    return (lv_font_t *)&lv_font_montserrat_32;
}

/**
 * @brief Return cached regular title font for inactive tab headings
 * @return Valid font pointer
 */
lv_font_t * scenes_assets_font_cached_title_regular(void)
{
    if (s_title_regular_ttf == NULL) {
        s_title_regular_ttf = __scenes_assets_create_font(
            SCENES_ASSETS_PATH_FONT_NOTO_REGULAR,
            SCENES_FONT_TITLE_PX
        );
    }
    if (s_title_regular_ttf != NULL) {
        return s_title_regular_ttf;
    }
    return (lv_font_t *)&lv_font_montserrat_32;
}

/**
 * @brief Return cached Urbanist font for temperature text
 * @return Valid font pointer
 */
lv_font_t * scenes_assets_font_cached_temp(void)
{
    if (s_temp_ttf == NULL) {
        s_temp_ttf = __scenes_assets_create_font(
            SCENES_ASSETS_PATH_FONT_URBANIST_BOLD,
            SCENES_FONT_TEMP_PX
        );
    }
    if (s_temp_ttf != NULL) {
        return s_temp_ttf;
    }
    return (lv_font_t *)&lv_font_montserrat_22;
}

/**
 * @brief Return cached Urbanist font for time text
 * @return Valid font pointer
 */
lv_font_t * scenes_assets_font_cached_time(void)
{
    if (s_time_ttf == NULL) {
        s_time_ttf = __scenes_assets_create_font(
            SCENES_ASSETS_PATH_FONT_URBANIST_BOLD,
            SCENES_FONT_TIME_PX
        );
    }
    if (s_time_ttf != NULL) {
        return s_time_ttf;
    }
    return (lv_font_t *)&lv_font_montserrat_26;
}

/**
 * @brief Return cached small meta font for AM and date text
 * @return Valid font pointer
 */
lv_font_t * scenes_assets_font_cached_meta(void)
{
    if (s_meta_ttf == NULL) {
        s_meta_ttf = __scenes_assets_create_font(
            SCENES_ASSETS_PATH_FONT_NOTO_REGULAR,
            SCENES_FONT_META_PX
        );
    }
    if (s_meta_ttf != NULL) {
        return s_meta_ttf;
    }
    return (lv_font_t *)&lv_font_montserrat_12;
}

/**
 * @brief Return cached card title font for scene cards
 * @return Valid font pointer
 */
lv_font_t * scenes_assets_font_cached_card_title(void)
{
    if (s_card_title_ttf == NULL) {
        s_card_title_ttf = __scenes_assets_create_font(
            SCENES_ASSETS_PATH_FONT_NOTO_REGULAR,
            SCENES_FONT_CARD_TITLE_PX
        );
    }
    if (s_card_title_ttf != NULL) {
        return s_card_title_ttf;
    }
    return (lv_font_t *)&lv_font_montserrat_22;
}
