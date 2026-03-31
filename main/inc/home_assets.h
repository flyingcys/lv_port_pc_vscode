/**
 * @file home_assets.h
 * @brief Home screen asset paths, design tokens, and Tiny TTF helpers
 * @version 1.0
 * @date 2026-03-30
 * @copyright Copyright (c) 2026
 */
#ifndef HOME_ASSETS_H
#define HOME_ASSETS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"
#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Macros
 * --------------------------------------------------------------------------- */

/** LVGL stdio FS driver letter (see @c LV_FS_STDIO_LETTER in @c lv_conf.h) */
#define HOME_ASSETS_FS_LETTER 'A'

/** Run with working directory = project root so these paths resolve */
#define HOME_ASSETS_PATH_HOME_BG "A:main/assets/home/background-stage.jpg"
#define HOME_ASSETS_PATH_HOME_BG_OVERLAY "A:main/assets/home/background-overlay.png"
#define HOME_ASSETS_PATH_WEATHER_PNG "A:main/assets/home/weather-icon.png"
#define HOME_ASSETS_PATH_WEATHER_BLOCK_PNG "A:main/assets/home/weather-block.png"
#define HOME_ASSETS_PATH_WIFI_PNG "A:main/assets/home/wifi-icon.png"
#define HOME_ASSETS_PATH_SWITCH_OFF_PNG "A:main/assets/home/switch-card-off.png"
#define HOME_ASSETS_PATH_SWITCH_ON_PNG "A:main/assets/home/switch-card-on.png"
#define HOME_ASSETS_PATH_PAGER_PNG "A:main/assets/home/pager-indicator.png"
#define HOME_ASSETS_PATH_CLOCK_COLON_PNG "A:main/assets/home/clock-colon.png"
#define HOME_ASSETS_PATH_CLOCK_GROUP_PNG "A:main/assets/home/clock-group.png"

#define HOME_ASSETS_PATH_FONT_URBANIST_BOLD "A:main/assets/fonts/Urbanist-Bold.ttf"
#define HOME_ASSETS_PATH_FONT_SOURCE_HAN_CN_BOLD "A:main/assets/fonts/NotoSansCJKSC-Bold.ttc"
#define HOME_ASSETS_PATH_FONT_SOURCE_HAN_CN_MEDIUM "A:main/assets/fonts/NotoSansCJKSC-Regular.ttc"

/** Tiny TTF pixel sizes aligned with @c design-preview/home/styles.css (480px stage) */
#define HOME_ASSETS_FONT_CLOCK_PX       110
#define HOME_ASSETS_FONT_TEMP_PX        32
#define HOME_ASSETS_FONT_META_CN_PX     20
/** token: font_162:3518 – Source Han Sans CN Bold 22px (switch card labels) */
#define HOME_ASSETS_FONT_CARD_LABEL_PX  22

/** Stage size (matches MasterGo preview) */
#define HOME_PAGE_WIDTH 480
#define HOME_PAGE_HEIGHT 480

/** Design tokens (RGB888, use with @c lv_color_hex) */
#define HOME_COLOR_PAGE_BG 0x070709U
#define HOME_COLOR_WHITE 0xFFFFFFU
#define HOME_COLOR_MUTED 0xC3C3C5U
#define HOME_COLOR_ACCENT 0x0BCAD0U
#define HOME_COLOR_ACCENT_SOFT 0x41CDE2U
/** Active card label (design: rgba(0,0,0,0.9) → solid near-black) */
#define HOME_COLOR_ACTIVE_TEXT 0x000000U

/* ---------------------------------------------------------------------------
 * Function declarations
 * --------------------------------------------------------------------------- */

/**
 * @brief Get LVGL filesystem path for the home background image
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_background(void);

/**
 * @brief Get LVGL filesystem path for the home background glow overlay
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_background_overlay(void);

/**
 * @brief Get LVGL filesystem path for the weather icon PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_weather_png(void);

/**
 * @brief Get LVGL filesystem path for the composed weather block PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_weather_block_png(void);

/**
 * @brief Get LVGL filesystem path for the Wi-Fi icon PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_wifi_png(void);

/**
 * @brief Get LVGL filesystem path for the switch card “off” PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_switch_off_png(void);

/**
 * @brief Get LVGL filesystem path for the switch card “on” PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_switch_on_png(void);

/**
 * @brief Get LVGL filesystem path for the pager indicator PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_pager_png(void);

/**
 * @brief Get LVGL filesystem path for the clock colon PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_clock_colon_png(void);

/**
 * @brief Get LVGL filesystem path for the composed clock group PNG
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_clock_group_png(void);

/**
 * @brief Get filesystem path for Urbanist (bold / variable) font file
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_font_urbanist_bold(void);

/**
 * @brief Get filesystem path for Source Han Sans CN bold font file
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_font_source_han_cn_bold(void);

/**
 * @brief Get filesystem path for Source Han Sans CN medium font file
 * @return Pointer to static path string with @c A: prefix
 */
const char * home_assets_get_path_font_source_han_cn_medium(void);

/**
 * @brief Create Tiny TTF font from Urbanist file at given pixel size
 * @param[in] font_px Line height / size in pixels (must be > 0)
 * @return Font pointer, or @c NULL on failure
 */
lv_font_t * home_assets_font_create_urbanist_bold(int32_t font_px);

/**
 * @brief Create Tiny TTF font from Source Han Sans CN bold file
 * @param[in] font_px Line height / size in pixels (must be > 0)
 * @return Font pointer, or @c NULL on failure
 */
lv_font_t * home_assets_font_create_source_han_cn_bold(int32_t font_px);

/**
 * @brief Create Tiny TTF font from Source Han Sans CN medium file
 * @param[in] font_px Line height / size in pixels (must be > 0)
 * @return Font pointer, or @c NULL on failure
 */
lv_font_t * home_assets_font_create_source_han_cn_medium(int32_t font_px);

/**
 * @brief Destroy a font created by @c home_assets_font_create_*()
 * @param[in] font Font returned by Tiny TTF create API, or @c NULL
 * @return none
 */
void home_assets_font_destroy(lv_font_t * font);

/**
 * @brief Return cached Urbanist clock font (or built-in fallback)
 * @return Valid font pointer (never @c NULL when built-in fonts are enabled)
 */
lv_font_t * home_assets_font_cached_clock(void);

/**
 * @brief Return cached Urbanist temperature font (or built-in fallback)
 * @return Valid font pointer (never @c NULL when built-in fonts are enabled)
 */
lv_font_t * home_assets_font_cached_temp(void);

/**
 * @brief Return cached Source Han meta font for date/weekday (or CJK fallback)
 * @return Valid font pointer (never @c NULL when built-in fonts are enabled)
 */
lv_font_t * home_assets_font_cached_meta_cn(void);

/**
 * @brief Return cached Source Han Sans CN Bold 22px font for switch card labels
 * @return Valid font pointer (never @c NULL when built-in fonts are enabled)
 * @note token: font_162:3518
 */
lv_font_t * home_assets_font_cached_card_label(void);

#ifdef __cplusplus
}
#endif

#endif /* HOME_ASSETS_H */
