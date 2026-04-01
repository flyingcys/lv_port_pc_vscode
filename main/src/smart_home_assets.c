#include "smart_home_assets.h"

#include "lvgl/src/libs/tiny_ttf/lv_tiny_ttf.h"

#define SMART_HOME_FONT_REGULAR_PATH "A:main/assets/smart_home/fonts/dejavu_sans.ttf"
#define SMART_HOME_FONT_BOLD_PATH "A:main/assets/smart_home/fonts/dejavu_sans_bold.ttf"

typedef struct {
    lv_font_t * title_bold_32;
    lv_font_t * title_regular_32;
    lv_font_t * metric_bold_26;
    lv_font_t * metric_bold_22;
    lv_font_t * body_regular_18;
    lv_font_t * body_regular_15;
    lv_font_t * caption_bold_10;
    bool initialized;
} smart_home_font_store_t;

static smart_home_font_store_t s_fonts;

static lv_font_t * load_font(const char * path, int32_t size)
{
    return lv_tiny_ttf_create_file(path, size);
}

static const lv_font_t * fallback_font(int32_t size)
{
    switch(size) {
        case 10:
            return &lv_font_montserrat_10;
        case 18:
            return &lv_font_montserrat_18;
        case 22:
            return &lv_font_montserrat_22;
        case 26:
            return &lv_font_montserrat_26;
        case 32:
            return &lv_font_montserrat_32;
        default:
            return LV_FONT_DEFAULT;
    }
}

bool smart_home_assets_init(void)
{
    if(s_fonts.initialized) {
        return true;
    }

    s_fonts.title_bold_32 = load_font(SMART_HOME_FONT_BOLD_PATH, 32);
    s_fonts.title_regular_32 = load_font(SMART_HOME_FONT_REGULAR_PATH, 32);
    s_fonts.metric_bold_26 = load_font(SMART_HOME_FONT_BOLD_PATH, 26);
    s_fonts.metric_bold_22 = load_font(SMART_HOME_FONT_BOLD_PATH, 22);
    s_fonts.body_regular_18 = load_font(SMART_HOME_FONT_REGULAR_PATH, 18);
    s_fonts.body_regular_15 = load_font(SMART_HOME_FONT_REGULAR_PATH, 15);
    s_fonts.caption_bold_10 = load_font(SMART_HOME_FONT_BOLD_PATH, 10);
    s_fonts.initialized = true;

    return true;
}

void smart_home_assets_deinit(void)
{
    if(!s_fonts.initialized) {
        return;
    }

    if(s_fonts.title_bold_32) lv_tiny_ttf_destroy(s_fonts.title_bold_32);
    if(s_fonts.title_regular_32) lv_tiny_ttf_destroy(s_fonts.title_regular_32);
    if(s_fonts.metric_bold_26) lv_tiny_ttf_destroy(s_fonts.metric_bold_26);
    if(s_fonts.metric_bold_22) lv_tiny_ttf_destroy(s_fonts.metric_bold_22);
    if(s_fonts.body_regular_18) lv_tiny_ttf_destroy(s_fonts.body_regular_18);
    if(s_fonts.body_regular_15) lv_tiny_ttf_destroy(s_fonts.body_regular_15);
    if(s_fonts.caption_bold_10) lv_tiny_ttf_destroy(s_fonts.caption_bold_10);

    lv_memzero(&s_fonts, sizeof(s_fonts));
}

const lv_font_t * smart_home_font_title_bold_32(void)
{
    return s_fonts.title_bold_32 ? s_fonts.title_bold_32 : fallback_font(32);
}

const lv_font_t * smart_home_font_title_regular_32(void)
{
    return s_fonts.title_regular_32 ? s_fonts.title_regular_32 : fallback_font(32);
}

const lv_font_t * smart_home_font_metric_bold_26(void)
{
    return s_fonts.metric_bold_26 ? s_fonts.metric_bold_26 : fallback_font(26);
}

const lv_font_t * smart_home_font_metric_bold_22(void)
{
    return s_fonts.metric_bold_22 ? s_fonts.metric_bold_22 : fallback_font(22);
}

const lv_font_t * smart_home_font_body_regular_18(void)
{
    return s_fonts.body_regular_18 ? s_fonts.body_regular_18 : fallback_font(18);
}

const lv_font_t * smart_home_font_body_regular_15(void)
{
    return s_fonts.body_regular_15 ? s_fonts.body_regular_15 : LV_FONT_DEFAULT;
}

const lv_font_t * smart_home_font_caption_bold_10(void)
{
    return s_fonts.caption_bold_10 ? s_fonts.caption_bold_10 : fallback_font(10);
}
