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

/** Background image opacity (~0.72 vs design ambient layer) */
#define HOME_BG_IMAGE_OPA       255

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
    lv_obj_set_style_image_opa(overlay, 148, 0);
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
    lv_obj_t * block;

    block = lv_image_create(parent);
    if (block == NULL) {
        return;
    }
    __home_page_style_base(block);
    lv_obj_set_pos(block, HOME_LAY_WEATHER_L, HOME_LAY_WEATHER_T);
    lv_obj_set_size(block, HOME_LAY_WEATHER_W, HOME_LAY_WEATHER_H);
    lv_image_set_src(block, home_assets_get_path_weather_block_png());
    lv_image_set_inner_align(block, LV_IMAGE_ALIGN_CONTAIN);
}

/**
 * @brief Add static clock 09:26 with colon image
 * @param[in] parent Root home container
 * @return none
 */
static void __home_page_add_clock(lv_obj_t * parent)
{
    lv_obj_t * panel;

    panel = lv_image_create(parent);
    if (panel == NULL) {
        return;
    }
    __home_page_style_base(panel);
    lv_obj_set_pos(panel, HOME_LAY_CLOCK_L, HOME_LAY_CLOCK_T);
    lv_obj_set_size(panel, HOME_LAY_CLOCK_W, HOME_LAY_CLOCK_H);
    lv_image_set_src(panel, home_assets_get_path_clock_group_png());
    lv_image_set_inner_align(panel, LV_IMAGE_ALIGN_CONTAIN);
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
