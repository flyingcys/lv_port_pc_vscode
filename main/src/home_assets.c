/**
 * @file home_assets.c
 * @brief Home screen asset paths and Tiny TTF helpers
 * @version 1.0
 * @date 2026-03-30
 * @copyright Copyright (c) 2026
 */

#include "home_assets.h"

/* ---------------------------------------------------------------------------
 * File scope variables
 * --------------------------------------------------------------------------- */
LV_FONT_DECLARE(lv_font_montserrat_48);
LV_FONT_DECLARE(lv_font_montserrat_32);
LV_FONT_DECLARE(lv_font_simsun_16_cjk);

static lv_font_t * s_cached_clock_ttf;
static lv_font_t * s_cached_temp_ttf;
static lv_font_t * s_cached_meta_ttf;
static lv_font_t * s_cached_card_label_ttf;

/* ---------------------------------------------------------------------------
 * Function implementations
 * --------------------------------------------------------------------------- */

/**
 * @brief Get LVGL filesystem path for the home background PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_background(void)
{
    return HOME_ASSETS_PATH_HOME_BG;
}

/**
 * @brief Get LVGL filesystem path for the home background glow overlay
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_background_overlay(void)
{
    return HOME_ASSETS_PATH_HOME_BG_OVERLAY;
}

/**
 * @brief Get LVGL filesystem path for the weather icon PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_weather_png(void)
{
    return HOME_ASSETS_PATH_WEATHER_PNG;
}

/**
 * @brief Get LVGL filesystem path for the composed weather block PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_weather_block_png(void)
{
    return HOME_ASSETS_PATH_WEATHER_BLOCK_PNG;
}

/**
 * @brief Get LVGL filesystem path for the Wi-Fi icon PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_wifi_png(void)
{
    return HOME_ASSETS_PATH_WIFI_PNG;
}

/**
 * @brief Get LVGL filesystem path for the switch card “off” PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_switch_off_png(void)
{
    return HOME_ASSETS_PATH_SWITCH_OFF_PNG;
}

/**
 * @brief Get LVGL filesystem path for the switch card “on” PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_switch_on_png(void)
{
    return HOME_ASSETS_PATH_SWITCH_ON_PNG;
}

/**
 * @brief Get LVGL filesystem path for the pager indicator PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_pager_png(void)
{
    return HOME_ASSETS_PATH_PAGER_PNG;
}

/**
 * @brief Get LVGL filesystem path for the clock colon PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_clock_colon_png(void)
{
    return HOME_ASSETS_PATH_CLOCK_COLON_PNG;
}

/**
 * @brief Get LVGL filesystem path for the composed clock group PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_clock_group_png(void)
{
    return HOME_ASSETS_PATH_CLOCK_GROUP_PNG;
}

/**
 * @brief Get filesystem path for Urbanist (bold / variable) font file
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_font_urbanist_bold(void)
{
    return HOME_ASSETS_PATH_FONT_URBANIST_BOLD;
}

/**
 * @brief Get filesystem path for Source Han Sans CN bold font file
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_font_source_han_cn_bold(void)
{
    return HOME_ASSETS_PATH_FONT_SOURCE_HAN_CN_BOLD;
}

/**
 * @brief Get filesystem path for Source Han Sans CN medium font file
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_font_source_han_cn_medium(void)
{
    return HOME_ASSETS_PATH_FONT_SOURCE_HAN_CN_MEDIUM;
}

/**
 * @brief Create Tiny TTF font from Urbanist file at given pixel size
 * @param[in] font_px Line height / size in pixels (must be > 0)
 * @return Font pointer, or @c NULL on failure
 */
lv_font_t * home_assets_font_create_urbanist_bold(int32_t font_px)
{
    if (font_px <= 0) {
        return NULL;
    }
    return lv_tiny_ttf_create_file(home_assets_get_path_font_urbanist_bold(), font_px);
}

/**
 * @brief Create Tiny TTF font from Source Han Sans CN bold file
 * @param[in] font_px Line height / size in pixels (must be > 0)
 * @return Font pointer, or @c NULL on failure
 */
lv_font_t * home_assets_font_create_source_han_cn_bold(int32_t font_px)
{
    if (font_px <= 0) {
        return NULL;
    }
    return lv_tiny_ttf_create_file(home_assets_get_path_font_source_han_cn_bold(), font_px);
}

/**
 * @brief Create Tiny TTF font from Source Han Sans CN medium file
 * @param[in] font_px Line height / size in pixels (must be > 0)
 * @return Font pointer, or @c NULL on failure
 */
lv_font_t * home_assets_font_create_source_han_cn_medium(int32_t font_px)
{
    if (font_px <= 0) {
        return NULL;
    }
    return lv_tiny_ttf_create_file(home_assets_get_path_font_source_han_cn_medium(), font_px);
}

/**
 * @brief Destroy a font created by @c home_assets_font_create_*()
 * @param[in] font Font returned by Tiny TTF create API, or @c NULL
 * @return none
 */
void home_assets_font_destroy(lv_font_t * font)
{
    if (font == NULL) {
        return;
    }
    lv_tiny_ttf_destroy(font);
}

/**
 * @brief Return cached Urbanist clock font (or built-in fallback)
 * @return Valid font pointer
 */
lv_font_t * home_assets_font_cached_clock(void)
{
    if (s_cached_clock_ttf == NULL) {
        s_cached_clock_ttf = home_assets_font_create_urbanist_bold(HOME_ASSETS_FONT_CLOCK_PX);
    }
    if (s_cached_clock_ttf != NULL) {
        return s_cached_clock_ttf;
    }
    return (lv_font_t *)&lv_font_montserrat_48;
}

/**
 * @brief Return cached Urbanist temperature font (or built-in fallback)
 * @return Valid font pointer
 */
lv_font_t * home_assets_font_cached_temp(void)
{
    if (s_cached_temp_ttf == NULL) {
        s_cached_temp_ttf = home_assets_font_create_urbanist_bold(HOME_ASSETS_FONT_TEMP_PX);
    }
    if (s_cached_temp_ttf != NULL) {
        return s_cached_temp_ttf;
    }
    return (lv_font_t *)&lv_font_montserrat_32;
}

/**
 * @brief Return cached Source Han meta font for date/weekday (or CJK fallback)
 * @return Valid font pointer
 */
lv_font_t * home_assets_font_cached_meta_cn(void)
{
    if (s_cached_meta_ttf == NULL) {
        s_cached_meta_ttf = home_assets_font_create_source_han_cn_medium(HOME_ASSETS_FONT_META_CN_PX);
    }
    if (s_cached_meta_ttf != NULL) {
        return s_cached_meta_ttf;
    }
    return (lv_font_t *)&lv_font_simsun_16_cjk;
}

/**
 * @brief Return cached Source Han Sans CN Bold 22px font for switch card labels
 * @return Valid font pointer
 * @note token: font_162:3518
 */
lv_font_t * home_assets_font_cached_card_label(void)
{
    if (s_cached_card_label_ttf == NULL) {
        s_cached_card_label_ttf = home_assets_font_create_source_han_cn_bold(HOME_ASSETS_FONT_CARD_LABEL_PX);
    }
    if (s_cached_card_label_ttf != NULL) {
        return s_cached_card_label_ttf;
    }
    return (lv_font_t *)&lv_font_montserrat_32;
}
