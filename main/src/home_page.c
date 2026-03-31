/**
 * @file home_page.c
 * @brief MasterGo home page static layout (background, weather, clock, Wi-Fi, pager)
 * @version 1.0
 * @date 2026-03-30
 * @copyright Copyright (c) 2026
 */

#include "home_page.h"
#include "home_assets.h"

/* ---------------------------------------------------------------------------
 * Layout constants (480px stage; ratios from design-preview/home/styles.css)
 * --------------------------------------------------------------------------- */
#define HOME_LAY_WEATHER_L      28
#define HOME_LAY_WEATHER_T      28
#define HOME_LAY_WEATHER_W      128
#define HOME_LAY_WEATHER_H      85
#define HOME_LAY_CLOCK_L        28
#define HOME_LAY_CLOCK_T        113
#define HOME_LAY_CLOCK_W        278
#define HOME_LAY_CLOCK_H        132
#define HOME_LAY_WEATHER_ICON_X 0
#define HOME_LAY_WEATHER_ICON_Y 0
#define HOME_LAY_WEATHER_ICON_W 40
#define HOME_LAY_WEATHER_ICON_H 35
#define HOME_LAY_WEATHER_TEMP_X 46
#define HOME_LAY_WEATHER_TEMP_Y 0
#define HOME_LAY_WEATHER_TEMP_W 60
#define HOME_LAY_WEATHER_TEMP_H 39
#define HOME_LAY_WEATHER_UNIT_X 101
#define HOME_LAY_WEATHER_UNIT_Y 6
#define HOME_LAY_WEATHER_UNIT_W 18
#define HOME_LAY_WEATHER_UNIT_H 22
#define HOME_LAY_WEATHER_DATE_X 1
#define HOME_LAY_WEATHER_DATE_Y 53
#define HOME_LAY_WEATHER_DATE_W 42
#define HOME_LAY_WEATHER_DATE_H 25
#define HOME_LAY_WEATHER_WEEKDAY_X 54
#define HOME_LAY_WEATHER_WEEKDAY_Y 53
#define HOME_LAY_WEATHER_WEEKDAY_W 42
#define HOME_LAY_WEATHER_WEEKDAY_H 25
#define HOME_LAY_CLOCK_HOUR_X   0
#define HOME_LAY_CLOCK_HOUR_Y   0
#define HOME_LAY_CLOCK_HOUR_W   114
#define HOME_LAY_CLOCK_HOUR_H   132
#define HOME_LAY_CLOCK_COLON_X  113
#define HOME_LAY_CLOCK_COLON_Y  40
#define HOME_LAY_CLOCK_COLON_W  10
#define HOME_LAY_CLOCK_COLON_H  28
#define HOME_LAY_CLOCK_MIN_X    128
#define HOME_LAY_CLOCK_MIN_Y    0
#define HOME_LAY_CLOCK_MIN_W    120
#define HOME_LAY_CLOCK_MIN_H    132

#define HOME_LAY_WIFI_SIZE      28
#define HOME_LAY_WIFI_X         432
#define HOME_LAY_WIFI_Y         20
#define HOME_LAY_PAGER_L        226
#define HOME_LAY_PAGER_T        462
#define HOME_LAY_PAGER_ACTIVE_W 18
#define HOME_LAY_PAGER_DOT_X    22
#define HOME_LAY_PAGER_DOT_W    6
#define HOME_LAY_PAGER_H        6

/* ---------------------------------------------------------------------------
 * Switch card layout (DSL: sa281:8209 / sa281:8217)
 * --------------------------------------------------------------------------- */
/** Card size – both cards are 204 x 140 */
#define HOME_LAY_CARD_W         204
#define HOME_LAY_CARD_H         140
/** Absolute X positions on the 480px stage */
#define HOME_LAY_CARD1_X        28
#define HOME_LAY_CARD2_X        248
/** Shared Y position */
#define HOME_LAY_CARD_Y         298
/** Border radius (token: borderRadius 20px) */
#define HOME_LAY_CARD_RADIUS    20
/** Indicator strip: 28 x 4, offset from card top-left */
#define HOME_LAY_CARD_IND_W     28
#define HOME_LAY_CARD_IND_H     4
#define HOME_LAY_CARD_IND_X     88
#define HOME_LAY_CARD_IND_Y     120
/** Label area relative to card */
#define HOME_LAY_CARD_LBL_X     69
#define HOME_LAY_CARD_LBL_Y     77
#define HOME_LAY_CARD_LBL_W     66
#define HOME_LAY_CARD_LBL_H     33

/* Card 1 fill opacities */
/** paint_162:3498 bottom fill rgba(0,0,0,0.8)  → 0.8*255 = 204 */
#define HOME_CARD1_BG_OPA       204
/** paint_162:3498 top fill rgba(255,255,255,0.15) → 0.15*255 = 38 */
#define HOME_CARD1_OVL_OPA      38
/** Indicator rgba(255,255,255,0.2) * opacity 0.8 → 0.16*255 = 41 */
#define HOME_CARD1_IND_OPA      41

/** Card 2 indicator #0BCAD0 at opacity 0.8 → 204 */
#define HOME_CARD2_IND_OPA      204

/** Text label opacity: rgba(*,*,*,0.9) → 0.9*255 = 230 */
#define HOME_CARD_LBL_OPA       230
#define HOME_TEXT_PRIMARY_OPA   230
#define HOME_TEXT_SECONDARY_OPA 204

/** Background image opacity (~0.72 vs design ambient layer) */
#define HOME_BG_IMAGE_OPA       255
#define HOME_BG_OVERLAY_OPA     148

/* ---------------------------------------------------------------------------
 * File scope variables
 * --------------------------------------------------------------------------- */
static HOME_PAGE_CTX_T s_home_ctx;

/* ---------------------------------------------------------------------------
 * Forward declarations
 * --------------------------------------------------------------------------- */
static void __home_page_style_base(lv_obj_t * obj);
static void __home_page_add_background(lv_obj_t * parent);
static void __home_page_add_overlays(lv_obj_t * parent);
static void __home_page_add_weather(lv_obj_t * parent);
static void __home_page_add_clock(lv_obj_t * parent);
static void __home_page_add_wifi(lv_obj_t * parent);
static void __home_page_add_pager(lv_obj_t * parent);
static lv_obj_t * __home_page_make_card(lv_obj_t * parent, int32_t x, bool is_dark, const char * label_utf8);
static void __home_page_add_switch_cards(lv_obj_t * parent);

/* ---------------------------------------------------------------------------
 * Function implementations
 * --------------------------------------------------------------------------- */

/**
 * @brief Apply non-interactive full-bleed object defaults
 * @param[in] obj Widget to style
 * @return none
 */
static void __home_page_style_base(lv_obj_t * obj)
{
    if (obj == NULL) {
        return;
    }
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_outline_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(obj, 0, 0);
}

/**
 * @brief Add background PNG with cover fit and reduced opacity
 * @param[in] parent Root home container
 * @return none
 */
static void __home_page_add_background(lv_obj_t * parent)
{
    lv_obj_t * bg;
    lv_obj_t * overlay;

    bg = lv_image_create(parent);
    if (bg == NULL) {
        return;
    }
    __home_page_style_base(bg);
    lv_obj_set_size(bg, HOME_PAGE_WIDTH, HOME_PAGE_HEIGHT);
    lv_obj_set_pos(bg, 0, 0);
    lv_image_set_src(bg, home_assets_get_path_background());
    lv_image_set_inner_align(bg, LV_IMAGE_ALIGN_COVER);
    lv_obj_set_style_image_opa(bg, HOME_BG_IMAGE_OPA, 0);

    overlay = lv_image_create(parent);
    if (overlay == NULL) {
        return;
    }
    __home_page_style_base(overlay);
    lv_obj_set_size(overlay, HOME_PAGE_WIDTH, HOME_PAGE_HEIGHT);
    lv_obj_set_pos(overlay, 0, 0);
    lv_image_set_src(overlay, home_assets_get_path_background_overlay());
    lv_image_set_inner_align(overlay, LV_IMAGE_ALIGN_COVER);
    lv_obj_set_style_image_opa(overlay, HOME_BG_OVERLAY_OPA, 0);
}

/**
 * @brief Add darkening and vignette overlays for foreground contrast
 * @param[in] parent Root home container
 * @return none
 */
static void __home_page_add_overlays(lv_obj_t * parent)
{
    lv_obj_t * wash;
    lv_obj_t * vignette;

    wash = lv_obj_create(parent);
    if (wash != NULL) {
        __home_page_style_base(wash);
        lv_obj_set_size(wash, HOME_PAGE_WIDTH, HOME_PAGE_HEIGHT);
        lv_obj_set_pos(wash, 0, 0);
        lv_obj_set_style_bg_color(wash, lv_color_hex(0x070709U), 0);
        lv_obj_set_style_bg_opa(wash, 20, 0);
    }

    vignette = lv_obj_create(parent);
    if (vignette != NULL) {
        __home_page_style_base(vignette);
        lv_obj_set_size(vignette, HOME_PAGE_WIDTH, HOME_PAGE_HEIGHT);
        lv_obj_set_pos(vignette, 0, 0);
        lv_obj_set_style_bg_color(vignette, lv_color_hex(0x070709U), 0);
        lv_obj_set_style_bg_grad_color(vignette, lv_color_hex(0x070709U), 0);
        lv_obj_set_style_bg_grad_dir(vignette, LV_GRAD_DIR_VER, 0);
        lv_obj_set_style_bg_main_opa(vignette, 0, 0);
        lv_obj_set_style_bg_grad_opa(vignette, 56, 0);
    }
}

/**
 * @brief Build weather icon, temperature, and date row (static copy)
 * @param[in] parent Root home container
 * @return none
 */
static void __home_page_add_weather(lv_obj_t * parent)
{
    lv_obj_t * container;
    lv_obj_t * icon;
    lv_obj_t * temp;
    lv_obj_t * unit;
    lv_obj_t * date;
    lv_obj_t * weekday;
    lv_font_t * temp_font;
    lv_font_t * meta_font;

    container = lv_obj_create(parent);
    if (container == NULL) {
        return;
    }
    __home_page_style_base(container);
    lv_obj_set_pos(container, HOME_LAY_WEATHER_L, HOME_LAY_WEATHER_T);
    lv_obj_set_size(container, HOME_LAY_WEATHER_W, HOME_LAY_WEATHER_H);
    s_home_ctx.weather_container = container;

    icon = lv_image_create(container);
    if (icon != NULL) {
        __home_page_style_base(icon);
        lv_obj_set_pos(icon, HOME_LAY_WEATHER_ICON_X, HOME_LAY_WEATHER_ICON_Y);
        lv_obj_set_size(icon, HOME_LAY_WEATHER_ICON_W, HOME_LAY_WEATHER_ICON_H);
        lv_image_set_src(icon, home_assets_get_path_weather_png());
        lv_image_set_inner_align(icon, LV_IMAGE_ALIGN_CONTAIN);
        s_home_ctx.weather_icon = icon;
    }

    temp_font = home_assets_font_cached_temp();
    temp = lv_label_create(container);
    if (temp != NULL) {
        __home_page_style_base(temp);
        lv_label_set_text(temp, "26");
        lv_obj_set_pos(temp, HOME_LAY_WEATHER_TEMP_X, HOME_LAY_WEATHER_TEMP_Y);
        lv_obj_set_size(temp, HOME_LAY_WEATHER_TEMP_W, HOME_LAY_WEATHER_TEMP_H);
        lv_obj_set_style_text_font(temp, temp_font, 0);
        lv_obj_set_style_text_color(temp, lv_color_hex(HOME_COLOR_WHITE), 0);
        lv_obj_set_style_text_opa(temp, HOME_TEXT_PRIMARY_OPA, 0);
        lv_obj_set_style_text_align(temp, LV_TEXT_ALIGN_LEFT, 0);
        s_home_ctx.weather_temp_label = temp;
    }

    unit = lv_label_create(container);
    if (unit != NULL) {
        __home_page_style_base(unit);
        lv_label_set_text(unit, "\xC2\xB0""C");
        lv_obj_set_pos(unit, HOME_LAY_WEATHER_UNIT_X, HOME_LAY_WEATHER_UNIT_Y);
        lv_obj_set_size(unit, HOME_LAY_WEATHER_UNIT_W, HOME_LAY_WEATHER_UNIT_H);
        lv_obj_set_style_text_font(unit, home_assets_font_cached_meta_cn(), 0);
        lv_obj_set_style_text_color(unit, lv_color_hex(HOME_COLOR_WHITE), 0);
        lv_obj_set_style_text_opa(unit, HOME_TEXT_SECONDARY_OPA, 0);
    }

    meta_font = home_assets_font_cached_meta_cn();

    date = lv_label_create(container);
    if (date != NULL) {
        __home_page_style_base(date);
        lv_label_set_text(date, "6/24");
        lv_obj_set_pos(date, HOME_LAY_WEATHER_DATE_X, HOME_LAY_WEATHER_DATE_Y);
        lv_obj_set_size(date, HOME_LAY_WEATHER_DATE_W, HOME_LAY_WEATHER_DATE_H);
        lv_obj_set_style_text_font(date, meta_font, 0);
        lv_obj_set_style_text_color(date, lv_color_hex(HOME_COLOR_WHITE), 0);
        lv_obj_set_style_text_opa(date, HOME_TEXT_SECONDARY_OPA, 0);
        s_home_ctx.weather_date_label = date;
    }

    weekday = lv_label_create(container);
    if (weekday != NULL) {
        __home_page_style_base(weekday);
        lv_label_set_text(weekday, "\xE5\x91\xA8\xE4\xB8\x80");
        lv_obj_set_pos(weekday, HOME_LAY_WEATHER_WEEKDAY_X, HOME_LAY_WEATHER_WEEKDAY_Y);
        lv_obj_set_size(weekday, HOME_LAY_WEATHER_WEEKDAY_W, HOME_LAY_WEATHER_WEEKDAY_H);
        lv_obj_set_style_text_font(weekday, meta_font, 0);
        lv_obj_set_style_text_color(weekday, lv_color_hex(HOME_COLOR_WHITE), 0);
        lv_obj_set_style_text_opa(weekday, HOME_TEXT_SECONDARY_OPA, 0);
        s_home_ctx.weather_weekday_label = weekday;
    }
}

/**
 * @brief Add static clock 09:26 with colon image
 * @param[in] parent Root home container
 * @return none
 */
static void __home_page_add_clock(lv_obj_t * parent)
{
    lv_obj_t * container;
    lv_obj_t * hours;
    lv_obj_t * colon;
    lv_obj_t * minutes;
    lv_font_t * clock_font;

    container = lv_obj_create(parent);
    if (container == NULL) {
        return;
    }
    __home_page_style_base(container);
    lv_obj_set_pos(container, HOME_LAY_CLOCK_L, HOME_LAY_CLOCK_T);
    lv_obj_set_size(container, HOME_LAY_CLOCK_W, HOME_LAY_CLOCK_H);
    s_home_ctx.clock_container = container;

    clock_font = home_assets_font_cached_clock();

    hours = lv_label_create(container);
    if (hours != NULL) {
        __home_page_style_base(hours);
        lv_label_set_text(hours, "09");
        lv_obj_set_pos(hours, HOME_LAY_CLOCK_HOUR_X, HOME_LAY_CLOCK_HOUR_Y);
        lv_obj_set_size(hours, HOME_LAY_CLOCK_HOUR_W, HOME_LAY_CLOCK_HOUR_H);
        lv_obj_set_style_text_font(hours, clock_font, 0);
        lv_obj_set_style_text_color(hours, lv_color_hex(HOME_COLOR_WHITE), 0);
        lv_obj_set_style_text_opa(hours, HOME_TEXT_PRIMARY_OPA, 0);
        lv_obj_set_style_text_align(hours, LV_TEXT_ALIGN_LEFT, 0);
        s_home_ctx.clock_hour_label = hours;
    }

    colon = lv_image_create(container);
    if (colon != NULL) {
        __home_page_style_base(colon);
        lv_obj_set_pos(colon, HOME_LAY_CLOCK_COLON_X, HOME_LAY_CLOCK_COLON_Y);
        lv_obj_set_size(colon, HOME_LAY_CLOCK_COLON_W, HOME_LAY_CLOCK_COLON_H);
        lv_image_set_src(colon, home_assets_get_path_clock_colon_png());
        lv_image_set_inner_align(colon, LV_IMAGE_ALIGN_CONTAIN);
        s_home_ctx.clock_colon_image = colon;
    }

    minutes = lv_label_create(container);
    if (minutes != NULL) {
        __home_page_style_base(minutes);
        lv_label_set_text(minutes, "26");
        lv_obj_set_pos(minutes, HOME_LAY_CLOCK_MIN_X, HOME_LAY_CLOCK_MIN_Y);
        lv_obj_set_size(minutes, HOME_LAY_CLOCK_MIN_W, HOME_LAY_CLOCK_MIN_H);
        lv_obj_set_style_text_font(minutes, clock_font, 0);
        lv_obj_set_style_text_color(minutes, lv_color_hex(HOME_COLOR_WHITE), 0);
        lv_obj_set_style_text_opa(minutes, HOME_TEXT_PRIMARY_OPA, 0);
        lv_obj_set_style_text_align(minutes, LV_TEXT_ALIGN_LEFT, 0);
        s_home_ctx.clock_minute_label = minutes;
    }
}

/**
 * @brief Add Wi-Fi status icon (top-right)
 * @param[in] parent Root home container
 * @return none
 */
static void __home_page_add_wifi(lv_obj_t * parent)
{
    lv_obj_t * wifi;

    wifi = lv_image_create(parent);
    if (wifi == NULL) {
        return;
    }
    __home_page_style_base(wifi);
    lv_obj_set_size(wifi, HOME_LAY_WIFI_SIZE, HOME_LAY_WIFI_SIZE);
    lv_obj_set_pos(wifi, HOME_LAY_WIFI_X, HOME_LAY_WIFI_Y);
    lv_image_set_src(wifi, home_assets_get_path_wifi_png());
    lv_image_set_inner_align(wifi, LV_IMAGE_ALIGN_CONTAIN);
}

/**
 * @brief Add pager strip near bottom center
 * @param[in] parent Root home container
 * @return none
 */
static void __home_page_add_pager(lv_obj_t * parent)
{
    lv_obj_t * pager;
    lv_obj_t * active;
    lv_obj_t * dot;

    pager = lv_obj_create(parent);
    if (pager == NULL) {
        return;
    }
    __home_page_style_base(pager);
    lv_obj_set_size(pager, HOME_LAY_WIFI_SIZE, HOME_LAY_PAGER_H);
    lv_obj_set_pos(pager, HOME_LAY_PAGER_L, HOME_LAY_PAGER_T);

    active = lv_obj_create(pager);
    if (active != NULL) {
        __home_page_style_base(active);
        lv_obj_set_size(active, HOME_LAY_PAGER_ACTIVE_W, HOME_LAY_PAGER_H);
        lv_obj_set_pos(active, 0, 0);
        lv_obj_set_style_radius(active, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(active, lv_color_hex(HOME_COLOR_ACCENT_SOFT), 0);
        lv_obj_set_style_bg_opa(active, LV_OPA_COVER, 0);
    }

    dot = lv_obj_create(pager);
    if (dot != NULL) {
        __home_page_style_base(dot);
        lv_obj_set_size(dot, HOME_LAY_PAGER_DOT_W, HOME_LAY_PAGER_H);
        lv_obj_set_pos(dot, HOME_LAY_PAGER_DOT_X, 0);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(dot, lv_color_hex(HOME_COLOR_WHITE), 0);
        lv_obj_set_style_bg_opa(dot, 128, 0);
    }
}

/**
 * @brief Build one switch card with layered fills, indicator strip, and label
 * @param[in] parent Root home container
 * @param[in] x      Absolute X position of the card on the 480px stage
 * @param[in] is_dark true = Card 1 dark style; false = Card 2 light gradient style
 * @param[in] label_utf8 UTF-8 encoded label string
 * @return Pointer to the outer card container, or @c NULL on allocation failure
 */
static lv_obj_t * __home_page_make_card(lv_obj_t * parent, int32_t x,
                                         bool is_dark, const char * label_utf8)
{
    lv_obj_t * card;
    lv_obj_t * fill;
    lv_obj_t * overlay;
    lv_obj_t * ind;
    lv_obj_t * lbl;
    lv_font_t * f;

    /* --- outer clipping container (paint_162:3498 / paint_161:3037) --- */
    card = lv_obj_create(parent);
    if (card == NULL) {
        return NULL;
    }
    lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(card, HOME_LAY_CARD_W, HOME_LAY_CARD_H);
    lv_obj_set_pos(card, x, HOME_LAY_CARD_Y);
    lv_obj_set_style_bg_opa(card, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_set_style_radius(card, HOME_LAY_CARD_RADIUS, 0);
    lv_obj_set_style_clip_corner(card, true, 0);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);

    if (is_dark) {
        /* Layer 1 – rgba(0,0,0,0.8): token paint_162:3498 bottom fill */
        fill = lv_obj_create(card);
        if (fill != NULL) {
            __home_page_style_base(fill);
            lv_obj_set_size(fill, HOME_LAY_CARD_W, HOME_LAY_CARD_H);
            lv_obj_set_pos(fill, 0, 0);
            lv_obj_set_style_bg_color(fill, lv_color_hex(0x000000U), 0);
            lv_obj_set_style_bg_opa(fill, HOME_CARD1_BG_OPA, 0);
        }
        /* Layer 2 – rgba(255,255,255,0.15): token paint_162:3498 top fill */
        overlay = lv_obj_create(card);
        if (overlay != NULL) {
            __home_page_style_base(overlay);
            lv_obj_set_size(overlay, HOME_LAY_CARD_W, HOME_LAY_CARD_H);
            lv_obj_set_pos(overlay, 0, 0);
            lv_obj_set_style_bg_color(overlay, lv_color_hex(0xFFFFFFU), 0);
            lv_obj_set_style_bg_opa(overlay, HOME_CARD1_OVL_OPA, 0);
        }
    } else {
        /* Single gradient fill – linear-gradient(123deg, #F5F9FD -1%, #C3D2E1 98%)
         * token paint_161:3037; LVGL approximates with vertical gradient */
        fill = lv_obj_create(card);
        if (fill != NULL) {
            __home_page_style_base(fill);
            lv_obj_set_size(fill, HOME_LAY_CARD_W, HOME_LAY_CARD_H);
            lv_obj_set_pos(fill, 0, 0);
            lv_obj_set_style_bg_color(fill, lv_color_hex(0xF5F9FDU), 0);
            lv_obj_set_style_bg_grad_color(fill, lv_color_hex(0xC3D2E1U), 0);
            lv_obj_set_style_bg_grad_dir(fill, LV_GRAD_DIR_VER, 0);
            lv_obj_set_style_bg_opa(fill, LV_OPA_COVER, 0);
        }
    }

    /* --- indicator strip (DSL: Rectangle 19705, 28×4 at x=88 y=120) --- */
    ind = lv_obj_create(card);
    if (ind != NULL) {
        __home_page_style_base(ind);
        lv_obj_set_size(ind, HOME_LAY_CARD_IND_W, HOME_LAY_CARD_IND_H);
        lv_obj_set_pos(ind, HOME_LAY_CARD_IND_X, HOME_LAY_CARD_IND_Y);
        lv_obj_set_style_radius(ind, 8, 0);
        if (is_dark) {
            /* rgba(255,255,255,0.2) at opacity 0.8 → combined opa 41 */
            lv_obj_set_style_bg_color(ind, lv_color_hex(0xFFFFFFU), 0);
            lv_obj_set_style_bg_opa(ind, HOME_CARD1_IND_OPA, 0);
        } else {
            /* #0BCAD0 (token Color/Dark/Main/M1_主色) at opacity 0.8 */
            lv_obj_set_style_bg_color(ind, lv_color_hex(HOME_COLOR_ACCENT), 0);
            lv_obj_set_style_bg_opa(ind, HOME_CARD2_IND_OPA, 0);
        }
    }

    /* --- text label (DSL: font_162:3518 – Source Han Sans CN Bold 22px) --- */
    f = home_assets_font_cached_card_label();
    lbl = lv_label_create(card);
    if (lbl != NULL) {
        lv_label_set_text(lbl, label_utf8);
        lv_obj_set_size(lbl, HOME_LAY_CARD_LBL_W, HOME_LAY_CARD_LBL_H);
        lv_obj_set_pos(lbl, HOME_LAY_CARD_LBL_X, HOME_LAY_CARD_LBL_Y);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_font(lbl, f, 0);
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
        if (is_dark) {
            /* paint_3:8068 rgba(255,255,255,0.9) */
            lv_obj_set_style_text_color(lbl, lv_color_hex(HOME_COLOR_WHITE), 0);
            lv_obj_set_style_text_opa(lbl, HOME_CARD_LBL_OPA, 0);
        } else {
            /* paint_161:3055 rgba(0,0,0,0.9) → token Color/Light/Grey/N10_000_90 */
            lv_obj_set_style_text_color(lbl, lv_color_hex(HOME_COLOR_ACTIVE_TEXT), 0);
            lv_obj_set_style_text_opa(lbl, HOME_CARD_LBL_OPA, 0);
        }
    }

    return card;
}

/**
 * @brief Add both switch cards to the home page
 * @param[in] parent Root home container
 * @return none
 */
static void __home_page_add_switch_cards(lv_obj_t * parent)
{
    /* Card 1 – dark style, label "开关一" (UTF-8: E5 BC 80  E5 85 B3  E4 B8 80) */
    s_home_ctx.switch_one_bg = __home_page_make_card(
        parent, HOME_LAY_CARD1_X, true,
        "\xE5\xBC\x80\xE5\x85\xB3\xE4\xB8\x80");

    /* Card 2 – light gradient style, label "开关二" (UTF-8: E5 BC 80  E5 85 B3  E4 BA 8C) */
    s_home_ctx.switch_two_bg = __home_page_make_card(
        parent, HOME_LAY_CARD2_X, false,
        "\xE5\xBC\x80\xE5\x85\xB3\xE4\xBA\x8C");
}

/**
 * @brief Create the home page root container on the active display screen
 * @return @c OPRT_OK on success, @c OPRT_INVALID_PARM if LVGL is not ready
 */
OPERATE_RET home_page_create(VOID_T)
{
    lv_obj_t * act;

    act = lv_screen_active();
    if (act == NULL) {
        return OPRT_INVALID_PARM;
    }

    s_home_ctx.switch_one_bg = NULL;
    s_home_ctx.switch_two_bg = NULL;
    s_home_ctx.weather_container = NULL;
    s_home_ctx.weather_icon = NULL;
    s_home_ctx.weather_temp_label = NULL;
    s_home_ctx.weather_date_label = NULL;
    s_home_ctx.weather_weekday_label = NULL;
    s_home_ctx.clock_container = NULL;
    s_home_ctx.clock_hour_label = NULL;
    s_home_ctx.clock_colon_image = NULL;
    s_home_ctx.clock_minute_label = NULL;
    s_home_ctx.switch_one_btn = NULL;
    s_home_ctx.switch_two_btn = NULL;
    s_home_ctx.switch_one_label = NULL;
    s_home_ctx.switch_two_label = NULL;
    s_home_ctx.active_index = 0;

    s_home_ctx.screen = lv_obj_create(act);
    if (s_home_ctx.screen == NULL) {
        return OPRT_INVALID_PARM;
    }

    lv_obj_set_size(s_home_ctx.screen, HOME_PAGE_WIDTH, HOME_PAGE_HEIGHT);
    lv_obj_set_style_bg_color(s_home_ctx.screen, lv_color_hex(HOME_COLOR_PAGE_BG), 0);
    lv_obj_set_style_bg_opa(s_home_ctx.screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_home_ctx.screen, 0, 0);
    lv_obj_set_style_pad_all(s_home_ctx.screen, 0, 0);
    lv_obj_set_style_radius(s_home_ctx.screen, 0, 0);
    lv_obj_set_style_clip_corner(s_home_ctx.screen, false, 0);
    lv_obj_remove_flag(s_home_ctx.screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_home_ctx.screen, LV_SCROLLBAR_MODE_OFF);

    __home_page_add_background(s_home_ctx.screen);
    __home_page_add_overlays(s_home_ctx.screen);
    __home_page_add_weather(s_home_ctx.screen);
    __home_page_add_clock(s_home_ctx.screen);
    __home_page_add_switch_cards(s_home_ctx.screen);
    __home_page_add_wifi(s_home_ctx.screen);
    __home_page_add_pager(s_home_ctx.screen);

    return OPRT_OK;
}
