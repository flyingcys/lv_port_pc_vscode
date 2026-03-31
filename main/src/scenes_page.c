/**
 * @file scenes_page.c
 * @brief Scenes page minimal root implementation
 * @version 1.0
 * @date 2026-03-31
 * @copyright Copyright (c) 2026
 */

#include "scenes_page.h"
#include "scenes_assets.h"

/* ---------------------------------------------------------------------------
 * Macros
 * --------------------------------------------------------------------------- */
#define SCENES_TOP_BAR_X           16
#define SCENES_TOP_BAR_Y           15
#define SCENES_TOP_BAR_W           448
#define SCENES_TOP_BAR_H           32
#define SCENES_TOP_WEATHER_SIZE    32
#define SCENES_TOP_TEMP_X          40
#define SCENES_TOP_TEMP_Y          2
#define SCENES_TOP_TEMP_W          26
#define SCENES_TOP_TEMP_H          28
#define SCENES_TOP_TEMP_UNIT_X     65
#define SCENES_TOP_TEMP_UNIT_Y     7
#define SCENES_TOP_RIGHT_X         315
#define SCENES_TOP_RIGHT_Y         2
#define SCENES_TOP_WIFI_W          28
#define SCENES_TOP_WIFI_H          28
#define SCENES_TOP_META_X          39
#define SCENES_TOP_META_W          22
#define SCENES_TOP_META_H          24
#define SCENES_TOP_TIME_X          66
#define SCENES_TOP_TIME_W          67
#define SCENES_TOP_TIME_H          28

#define SCENES_TITLE_SCENES_X      16
#define SCENES_TITLE_SCENES_Y      70
#define SCENES_TITLE_SCENES_W      111
#define SCENES_TITLE_SCENES_H      48
#define SCENES_TITLE_DEVICES_X     144
#define SCENES_TITLE_DEVICES_Y     70
#define SCENES_TITLE_DEVICES_W     115
#define SCENES_TITLE_DEVICES_H     48

#define SCENES_BOTTOM_FADE_Y       428
#define SCENES_BOTTOM_FADE_H       52
#define SCENES_PAGER_X             226
#define SCENES_PAGER_Y             462
#define SCENES_PAGER_W             28
#define SCENES_PAGER_H             6
#define SCENES_PAGER_DOT_W         6
#define SCENES_PAGER_ACTIVE_W      18
#define SCENES_PAGER_ACTIVE_X      10

#define SCENES_CARD_W              219
#define SCENES_CARD_H              145
#define SCENES_CARD_RADIUS         16
#define SCENES_CARD_LEFT_X         16
#define SCENES_CARD_RIGHT_X        245
#define SCENES_CARD_TOP_Y          130
#define SCENES_CARD_BOTTOM_Y       285
#define SCENES_CARD_BG_OPA         38
#define SCENES_CARD_TEXT_OPA       230
#define SCENES_CARD_TITLE_X        16
#define SCENES_CARD_ICON_DEFAULT_X 16
#define SCENES_CARD_ICON_DEFAULT_Y 31
#define SCENES_CARD_ICON_HOME_Y    26
#define SCENES_CARD_ICON_W         48
#define SCENES_CARD_ICON_H         48

/* ---------------------------------------------------------------------------
 * Type definitions
 * --------------------------------------------------------------------------- */
typedef struct {
    SCENES_CARD_E card;
    int32_t x;
    int32_t y;
    int32_t icon_x;
    int32_t icon_y;
    int32_t title_x;
    int32_t title_y;
    int32_t title_w;
    int32_t title_h;
    const char * title;
} SCENES_CARD_CONFIG_T;

/* ---------------------------------------------------------------------------
 * File scope variables
 * --------------------------------------------------------------------------- */
static SCENES_PAGE_CTX_T s_scenes_ctx;

static const SCENES_CARD_CONFIG_T s_card_configs[SCENES_CARD_COUNT] = {
    {
        .card = SCENES_CARD_HOME,
        .x = SCENES_CARD_LEFT_X,
        .y = SCENES_CARD_TOP_Y,
        .icon_x = SCENES_CARD_ICON_DEFAULT_X,
        .icon_y = SCENES_CARD_ICON_HOME_Y,
        .title_x = 16,
        .title_y = 80,
        .title_w = 52,
        .title_h = 27,
        .title = "Home",
    },
    {
        .card = SCENES_CARD_LEAVING_HOME,
        .x = SCENES_CARD_LEFT_X,
        .y = SCENES_CARD_BOTTOM_Y,
        .icon_x = SCENES_CARD_ICON_DEFAULT_X,
        .icon_y = SCENES_CARD_ICON_DEFAULT_Y,
        .title_x = 16,
        .title_y = 87,
        .title_w = 123,
        .title_h = 27,
        .title = "Leaving Home",
    },
    {
        .card = SCENES_CARD_MORNING,
        .x = SCENES_CARD_RIGHT_X,
        .y = SCENES_CARD_TOP_Y,
        .icon_x = SCENES_CARD_ICON_DEFAULT_X,
        .icon_y = 26,
        .title_x = 16,
        .title_y = 82,
        .title_w = 72,
        .title_h = 27,
        .title = "Morning",
    },
    {
        .card = SCENES_CARD_ALL_LIGHTS_ON,
        .x = SCENES_CARD_RIGHT_X,
        .y = SCENES_CARD_BOTTOM_Y,
        .icon_x = SCENES_CARD_ICON_DEFAULT_X,
        .icon_y = SCENES_CARD_ICON_DEFAULT_Y,
        .title_x = 16,
        .title_y = 87,
        .title_w = 108,
        .title_h = 27,
        .title = "All Lights On",
    },
};

/* ---------------------------------------------------------------------------
 * Forward declarations
 * --------------------------------------------------------------------------- */
static void __scenes_page_style_base(lv_obj_t * obj);
static void __scenes_page_add_top_bar(lv_obj_t * parent);
static void __scenes_page_add_titles(lv_obj_t * parent);
static void __scenes_page_add_bottom_fade(lv_obj_t * parent);
static void __scenes_page_add_pager(lv_obj_t * parent);
static void __scenes_page_add_cards(lv_obj_t * parent);
static void __scenes_page_refresh_cards(VOID_T);
static void __scenes_page_card_event_cb(lv_event_t * e);

/* ---------------------------------------------------------------------------
 * Function implementations
 * --------------------------------------------------------------------------- */
/**
 * @brief Apply common non-scrollable base style to objects
 * @param[in] obj Object to initialize
 * @return none
 */
static void __scenes_page_style_base(lv_obj_t * obj)
{
    if (obj == NULL) {
        return;
    }
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_TRANSP, 0);
}

/**
 * @brief Add the top information bar with weather, temperature, Wi-Fi, and time
 * @param[in] parent Root scenes container
 * @return none
 */
static void __scenes_page_add_top_bar(lv_obj_t * parent)
{
    lv_obj_t * top_bar;
    lv_obj_t * weather_icon;
    lv_obj_t * temp_label;
    lv_obj_t * temp_unit_label;
    lv_obj_t * right_group;
    lv_obj_t * wifi_icon;
    lv_obj_t * meta_group;
    lv_obj_t * am_label;
    lv_obj_t * date_label;
    lv_obj_t * time_label;

    top_bar = lv_obj_create(parent);
    if (top_bar == NULL) {
        return;
    }
    __scenes_page_style_base(top_bar);
    lv_obj_set_pos(top_bar, SCENES_TOP_BAR_X, SCENES_TOP_BAR_Y);
    lv_obj_set_size(top_bar, SCENES_TOP_BAR_W, SCENES_TOP_BAR_H);

    weather_icon = lv_image_create(top_bar);
    if (weather_icon != NULL) {
        __scenes_page_style_base(weather_icon);
        lv_obj_set_pos(weather_icon, 0, 0);
        lv_obj_set_size(weather_icon, SCENES_TOP_WEATHER_SIZE, SCENES_TOP_WEATHER_SIZE);
        lv_image_set_src(weather_icon, scenes_assets_get_path_weather_png());
        lv_image_set_inner_align(weather_icon, LV_IMAGE_ALIGN_CONTAIN);
    }

    temp_label = lv_label_create(top_bar);
    if (temp_label != NULL) {
        lv_label_set_text(temp_label, "24");
        lv_obj_set_pos(temp_label, SCENES_TOP_TEMP_X, SCENES_TOP_TEMP_Y);
        lv_obj_set_size(temp_label, SCENES_TOP_TEMP_W, SCENES_TOP_TEMP_H);
        lv_obj_set_style_text_font(temp_label, scenes_assets_font_cached_temp(), 0);
        lv_obj_set_style_text_color(temp_label, lv_color_hex(SCENES_COLOR_TEXT), 0);
        lv_obj_set_style_text_align(temp_label, LV_TEXT_ALIGN_LEFT, 0);
    }

    temp_unit_label = lv_label_create(top_bar);
    if (temp_unit_label != NULL) {
        lv_label_set_text(temp_unit_label, "\xC2\xB0""C");
        lv_obj_set_pos(temp_unit_label, SCENES_TOP_TEMP_UNIT_X, SCENES_TOP_TEMP_UNIT_Y);
        lv_obj_set_style_text_font(temp_unit_label, scenes_assets_font_cached_meta(), 0);
        lv_obj_set_style_text_color(temp_unit_label, lv_color_hex(SCENES_COLOR_TEXT), 0);
        lv_obj_set_style_text_opa(temp_unit_label, 102, 0);
    }

    right_group = lv_obj_create(top_bar);
    if (right_group == NULL) {
        return;
    }
    __scenes_page_style_base(right_group);
    lv_obj_set_pos(right_group, SCENES_TOP_RIGHT_X, SCENES_TOP_RIGHT_Y);
    lv_obj_set_size(right_group, 133, 28);

    wifi_icon = lv_image_create(right_group);
    if (wifi_icon != NULL) {
        __scenes_page_style_base(wifi_icon);
        lv_obj_set_pos(wifi_icon, 0, 0);
        lv_obj_set_size(wifi_icon, SCENES_TOP_WIFI_W, SCENES_TOP_WIFI_H);
        lv_image_set_src(wifi_icon, scenes_assets_get_path_wifi_png());
        lv_image_set_inner_align(wifi_icon, LV_IMAGE_ALIGN_CONTAIN);
    }

    meta_group = lv_obj_create(right_group);
    if (meta_group != NULL) {
        __scenes_page_style_base(meta_group);
        lv_obj_set_pos(meta_group, SCENES_TOP_META_X, 0);
        lv_obj_set_size(meta_group, SCENES_TOP_META_W, SCENES_TOP_META_H);

        am_label = lv_label_create(meta_group);
        if (am_label != NULL) {
            lv_label_set_text(am_label, "AM");
            lv_obj_set_pos(am_label, 6, 0);
            lv_obj_set_style_text_font(am_label, scenes_assets_font_cached_meta(), 0);
            lv_obj_set_style_text_color(am_label, lv_color_hex(SCENES_COLOR_TEXT), 0);
        }

        date_label = lv_label_create(meta_group);
        if (date_label != NULL) {
            lv_label_set_text(date_label, "2/24");
            lv_obj_set_pos(date_label, 0, 12);
            lv_obj_set_style_text_font(date_label, scenes_assets_font_cached_meta(), 0);
            lv_obj_set_style_text_color(date_label, lv_color_hex(SCENES_COLOR_TEXT), 0);
        }
    }

    time_label = lv_label_create(right_group);
    if (time_label != NULL) {
        lv_label_set_text(time_label, "08:20");
        lv_obj_set_pos(time_label, SCENES_TOP_TIME_X, 0);
        lv_obj_set_size(time_label, SCENES_TOP_TIME_W, SCENES_TOP_TIME_H);
        lv_obj_set_style_text_font(time_label, scenes_assets_font_cached_time(), 0);
        lv_obj_set_style_text_color(time_label, lv_color_hex(SCENES_COLOR_TEXT), 0);
        lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_RIGHT, 0);
    }
}

/**
 * @brief Add page titles for Scenes and Devices tabs
 * @param[in] parent Root scenes container
 * @return none
 */
static void __scenes_page_add_titles(lv_obj_t * parent)
{
    lv_obj_t * scenes_label;
    lv_obj_t * devices_label;

    scenes_label = lv_label_create(parent);
    if (scenes_label != NULL) {
        lv_label_set_text(scenes_label, "Scenes");
        lv_obj_set_pos(scenes_label, SCENES_TITLE_SCENES_X, SCENES_TITLE_SCENES_Y);
        lv_obj_set_size(scenes_label, SCENES_TITLE_SCENES_W, SCENES_TITLE_SCENES_H);
        lv_obj_set_style_text_font(scenes_label, scenes_assets_font_cached_title_bold(), 0);
        lv_obj_set_style_text_color(scenes_label, lv_color_hex(SCENES_COLOR_TEXT), 0);
        lv_obj_set_style_text_opa(scenes_label, 230, 0);
    }

    devices_label = lv_label_create(parent);
    if (devices_label != NULL) {
        lv_label_set_text(devices_label, "Devices");
        lv_obj_set_pos(devices_label, SCENES_TITLE_DEVICES_X, SCENES_TITLE_DEVICES_Y);
        lv_obj_set_size(devices_label, SCENES_TITLE_DEVICES_W, SCENES_TITLE_DEVICES_H);
        lv_obj_set_style_text_font(devices_label, scenes_assets_font_cached_title_regular(), 0);
        lv_obj_set_style_text_color(devices_label, lv_color_hex(SCENES_COLOR_TEXT), 0);
        lv_obj_set_style_text_opa(devices_label, 128, 0);
    }
}

/**
 * @brief Add the bottom vertical fade overlay
 * @param[in] parent Root scenes container
 * @return none
 */
static void __scenes_page_add_bottom_fade(lv_obj_t * parent)
{
    lv_obj_t * fade;

    fade = lv_obj_create(parent);
    if (fade == NULL) {
        return;
    }
    __scenes_page_style_base(fade);
    lv_obj_set_pos(fade, 0, SCENES_BOTTOM_FADE_Y);
    lv_obj_set_size(fade, SCENES_PAGE_WIDTH, SCENES_BOTTOM_FADE_H);
    lv_obj_set_style_bg_color(fade, lv_color_hex(SCENES_COLOR_PAGE_BG), 0);
    lv_obj_set_style_bg_grad_color(fade, lv_color_hex(SCENES_COLOR_PAGE_BG), 0);
    lv_obj_set_style_bg_grad_dir(fade, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_main_opa(fade, 0, 0);
    lv_obj_set_style_bg_grad_opa(fade, LV_OPA_COVER, 0);
}

/**
 * @brief Add the bottom pager indicator
 * @param[in] parent Root scenes container
 * @return none
 */
static void __scenes_page_add_pager(lv_obj_t * parent)
{
    lv_obj_t * pager;
    lv_obj_t * dot;
    lv_obj_t * active;

    pager = lv_obj_create(parent);
    if (pager == NULL) {
        return;
    }
    __scenes_page_style_base(pager);
    lv_obj_set_pos(pager, SCENES_PAGER_X, SCENES_PAGER_Y);
    lv_obj_set_size(pager, SCENES_PAGER_W, SCENES_PAGER_H);

    dot = lv_obj_create(pager);
    if (dot != NULL) {
        __scenes_page_style_base(dot);
        lv_obj_set_pos(dot, 0, 0);
        lv_obj_set_size(dot, SCENES_PAGER_DOT_W, SCENES_PAGER_H);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(dot, lv_color_hex(SCENES_COLOR_TEXT), 0);
        lv_obj_set_style_bg_opa(dot, 128, 0);
    }

    active = lv_obj_create(pager);
    if (active != NULL) {
        __scenes_page_style_base(active);
        lv_obj_set_pos(active, SCENES_PAGER_ACTIVE_X, 0);
        lv_obj_set_size(active, SCENES_PAGER_ACTIVE_W, SCENES_PAGER_H);
        lv_obj_set_style_radius(active, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(active, lv_color_hex(SCENES_COLOR_ACTIVE), 0);
        lv_obj_set_style_bg_opa(active, LV_OPA_COVER, 0);
    }
}

/**
 * @brief Refresh card backgrounds and icon/title styles from current state
 * @return none
 */
static void __scenes_page_refresh_cards(VOID_T)
{
    uint32_t index;
    SCENES_CARD_E selected;

    selected = scenes_state_get_selected(&s_scenes_ctx.state);

    for (index = 0; index < SCENES_CARD_COUNT; index++) {
        lv_obj_t * card;
        lv_obj_t * title;
        lv_obj_t * icon;
        bool is_selected;

        card = s_scenes_ctx.cards[index];
        title = s_scenes_ctx.card_titles[index];
        icon = s_scenes_ctx.card_icons[index];
        if (card == NULL) {
            continue;
        }

        is_selected = ((SCENES_CARD_E)index == selected);
        lv_obj_set_style_radius(card, SCENES_CARD_RADIUS, 0);
        lv_obj_set_style_bg_opa(card, is_selected ? LV_OPA_COVER : SCENES_CARD_BG_OPA, 0);
        lv_obj_set_style_border_width(card, 0, 0);

        if (is_selected) {
            lv_obj_set_style_bg_color(card, lv_color_hex(0x2D8CDBU), 0);
            lv_obj_set_style_bg_grad_color(card, lv_color_hex(0x73BFFDU), 0);
            lv_obj_set_style_bg_grad_dir(card, LV_GRAD_DIR_HOR, 0);
        } else {
            lv_obj_set_style_bg_color(card, lv_color_hex(SCENES_COLOR_TEXT), 0);
            lv_obj_set_style_bg_grad_color(card, lv_color_hex(SCENES_COLOR_TEXT), 0);
            lv_obj_set_style_bg_grad_dir(card, LV_GRAD_DIR_NONE, 0);
        }

        if (title != NULL) {
            lv_obj_set_style_text_color(title, lv_color_hex(SCENES_COLOR_TEXT), 0);
            lv_obj_set_style_text_opa(title, SCENES_CARD_TEXT_OPA, 0);
        }

        if (icon != NULL) {
            lv_image_set_src(icon, scenes_assets_get_path_card_icon((SCENES_CARD_E)index, is_selected));
        }
    }
}

/**
 * @brief Handle click events on scene cards
 * @param[in] e LVGL event descriptor
 * @return none
 */
static void __scenes_page_card_event_cb(lv_event_t * e)
{
    uint32_t card_index;

    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    card_index = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
    scenes_state_select(&s_scenes_ctx.state, (SCENES_CARD_E)card_index);
    __scenes_page_refresh_cards();
}

/**
 * @brief Add four scene cards to the grid area
 * @param[in] parent Root scenes container
 * @return none
 */
static void __scenes_page_add_cards(lv_obj_t * parent)
{
    uint32_t index;

    for (index = 0; index < SCENES_CARD_COUNT; index++) {
        const SCENES_CARD_CONFIG_T * config;
        lv_obj_t * card;
        lv_obj_t * icon;
        lv_obj_t * title;

        config = &s_card_configs[index];
        card = lv_button_create(parent);
        if (card == NULL) {
            continue;
        }

        lv_obj_set_pos(card, config->x, config->y);
        lv_obj_set_size(card, SCENES_CARD_W, SCENES_CARD_H);
        lv_obj_set_style_pad_all(card, 0, 0);
        lv_obj_set_style_shadow_opa(card, LV_OPA_TRANSP, 0);
        lv_obj_set_style_outline_width(card, 0, 0);
        lv_obj_set_style_radius(card, SCENES_CARD_RADIUS, 0);
        lv_obj_set_style_border_width(card, 0, 0);
        lv_obj_add_event_cb(
            card,
            __scenes_page_card_event_cb,
            LV_EVENT_CLICKED,
            (void *)(uintptr_t)config->card
        );

        icon = lv_image_create(card);
        if (icon != NULL) {
            lv_obj_set_pos(icon, config->icon_x, config->icon_y);
            lv_obj_set_size(icon, SCENES_CARD_ICON_W, SCENES_CARD_ICON_H);
            lv_image_set_inner_align(icon, LV_IMAGE_ALIGN_CONTAIN);
        }

        title = lv_label_create(card);
        if (title != NULL) {
            lv_label_set_text(title, config->title);
            lv_obj_set_pos(title, config->title_x, config->title_y);
            lv_obj_set_size(title, config->title_w, config->title_h);
            lv_obj_set_style_text_font(title, scenes_assets_font_cached_card_title(), 0);
            lv_obj_set_style_text_color(title, lv_color_hex(SCENES_COLOR_TEXT), 0);
            lv_obj_set_style_text_opa(title, SCENES_CARD_TEXT_OPA, 0);
            lv_label_set_long_mode(title, LV_LABEL_LONG_CLIP);
        }

        s_scenes_ctx.cards[index] = card;
        s_scenes_ctx.card_icons[index] = icon;
        s_scenes_ctx.card_titles[index] = title;
    }

    __scenes_page_refresh_cards();
}

/**
 * @brief Create the scenes page root container on the active display screen
 * @return OPRT_OK on success, OPRT_INVALID_PARM if LVGL is not ready
 */
OPERATE_RET scenes_page_create(VOID_T)
{
    lv_obj_t * act;

    act = lv_screen_active();
    if (act == NULL) {
        return OPRT_INVALID_PARM;
    }

    scenes_state_init(&s_scenes_ctx.state);
    lv_memzero(s_scenes_ctx.cards, sizeof(s_scenes_ctx.cards));
    lv_memzero(s_scenes_ctx.card_titles, sizeof(s_scenes_ctx.card_titles));
    lv_memzero(s_scenes_ctx.card_icons, sizeof(s_scenes_ctx.card_icons));

    s_scenes_ctx.screen = lv_obj_create(act);
    if (s_scenes_ctx.screen == NULL) {
        return OPRT_INVALID_PARM;
    }

    lv_obj_set_size(s_scenes_ctx.screen, SCENES_PAGE_WIDTH, SCENES_PAGE_HEIGHT);
    lv_obj_set_style_bg_color(s_scenes_ctx.screen, lv_color_hex(SCENES_COLOR_PAGE_BG), 0);
    lv_obj_set_style_bg_opa(s_scenes_ctx.screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_scenes_ctx.screen, 0, 0);
    lv_obj_set_style_pad_all(s_scenes_ctx.screen, 0, 0);
    lv_obj_set_style_radius(s_scenes_ctx.screen, 0, 0);
    lv_obj_remove_flag(s_scenes_ctx.screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_scenes_ctx.screen, LV_SCROLLBAR_MODE_OFF);

    __scenes_page_add_top_bar(s_scenes_ctx.screen);
    __scenes_page_add_titles(s_scenes_ctx.screen);
    __scenes_page_add_cards(s_scenes_ctx.screen);
    __scenes_page_add_bottom_fade(s_scenes_ctx.screen);
    __scenes_page_add_pager(s_scenes_ctx.screen);

    return OPRT_OK;
}
