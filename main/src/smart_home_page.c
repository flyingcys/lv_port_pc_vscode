#include "smart_home_page.h"

#include <stdint.h>

#include "smart_home_assets.h"

typedef enum {
    SMART_HOME_ICON_WEATHER,
    SMART_HOME_ICON_WIFI,
    SMART_HOME_ICON_HOME,
    SMART_HOME_ICON_CHECK,
    SMART_HOME_ICON_SCENE,
    SMART_HOME_ICON_BULB,
} smart_home_icon_t;

typedef enum {
    SMART_HOME_CARD_GLASS,
    SMART_HOME_CARD_GRADIENT,
} smart_home_card_style_t;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
    smart_home_card_style_t style;
    smart_home_icon_t icon;
    int32_t icon_x;
    int32_t icon_y;
    int32_t icon_w;
    int32_t icon_h;
    int32_t text_x;
    int32_t text_y;
    const char * label;
} smart_home_card_spec_t;

static lv_obj_t * s_root;
static lv_obj_t * s_placeholder;

static lv_color_t color_black(void) { return lv_color_hex(0x000000); }
static lv_color_t color_white(void) { return lv_color_hex(0xFFFFFF); }
static lv_color_t color_cloud(void) { return lv_color_hex(0xF0F2F5); }
static lv_color_t color_sun(void) { return lv_color_hex(0xFF7B30); }
static lv_color_t color_cyan(void) { return lv_color_hex(0x41CDE2); }
static lv_color_t color_blue_start(void) { return lv_color_hex(0x73BFFD); }
static lv_color_t color_blue_end(void) { return lv_color_hex(0x2D8CDB); }

static void draw_filled_circle(lv_layer_t * layer, int32_t cx, int32_t cy, int32_t radius, lv_color_t color, lv_opa_t opa)
{
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = color;
    dsc.bg_opa = opa;
    dsc.radius = LV_RADIUS_CIRCLE;
    dsc.border_width = 0;

    lv_area_t area = {cx - radius, cy - radius, cx + radius, cy + radius};
    lv_draw_rect(layer, &dsc, &area);
}

static void draw_round_rect(
    lv_layer_t * layer,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2,
    int32_t radius,
    lv_color_t color,
    lv_opa_t opa
)
{
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = color;
    dsc.bg_opa = opa;
    dsc.radius = radius;
    dsc.border_width = 0;

    lv_area_t area = {x1, y1, x2, y2};
    lv_draw_rect(layer, &dsc, &area);
}

static void draw_stroke_circle(
    lv_layer_t * layer,
    int32_t cx,
    int32_t cy,
    int32_t radius,
    int32_t width,
    lv_color_t color,
    lv_opa_t opa
)
{
    lv_draw_arc_dsc_t dsc;
    lv_draw_arc_dsc_init(&dsc);
    dsc.color = color;
    dsc.opa = opa;
    dsc.width = width;
    dsc.center.x = cx;
    dsc.center.y = cy;
    dsc.radius = radius;
    dsc.start_angle = 0;
    dsc.end_angle = 360;
    lv_draw_arc(layer, &dsc);
}

static void draw_line(
    lv_layer_t * layer,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2,
    int32_t width,
    lv_color_t color,
    lv_opa_t opa
)
{
    lv_draw_line_dsc_t dsc;
    lv_draw_line_dsc_init(&dsc);
    dsc.color = color;
    dsc.opa = opa;
    dsc.width = width;
    dsc.round_start = 1;
    dsc.round_end = 1;
    dsc.p1.x = x1;
    dsc.p1.y = y1;
    dsc.p2.x = x2;
    dsc.p2.y = y2;
    lv_draw_line(layer, &dsc);
}

static void draw_weather_icon(lv_layer_t * layer, const lv_area_t * area)
{
    const int32_t x = area->x1;
    const int32_t y = area->y1;

    draw_filled_circle(layer, x + 22, y + 10, 7, color_sun(), LV_OPA_COVER);
    draw_line(layer, x + 22, y + 1, x + 22, y + 5, 2, color_sun(), LV_OPA_COVER);
    draw_line(layer, x + 14, y + 10, x + 18, y + 10, 2, color_sun(), LV_OPA_COVER);
    draw_line(layer, x + 26, y + 10, x + 30, y + 10, 2, color_sun(), LV_OPA_COVER);

    draw_filled_circle(layer, x + 11, y + 18, 7, color_cloud(), LV_OPA_COVER);
    draw_filled_circle(layer, x + 18, y + 15, 8, color_cloud(), LV_OPA_COVER);
    draw_filled_circle(layer, x + 25, y + 18, 6, color_cloud(), LV_OPA_COVER);
    draw_round_rect(layer, x + 8, y + 18, x + 28, y + 25, LV_RADIUS_CIRCLE, color_cloud(), LV_OPA_COVER);
}

static void draw_wifi_icon(lv_layer_t * layer, const lv_area_t * area)
{
    const int32_t cx = (area->x1 + area->x2) / 2;
    const int32_t cy = area->y1 + 19;

    lv_draw_arc_dsc_t dsc;
    lv_draw_arc_dsc_init(&dsc);
    dsc.color = color_white();
    dsc.opa = LV_OPA_COVER;
    dsc.width = 2;
    dsc.center.x = cx;
    dsc.center.y = cy;
    dsc.start_angle = 225;
    dsc.end_angle = 315;

    dsc.radius = 11;
    lv_draw_arc(layer, &dsc);
    dsc.radius = 7;
    lv_draw_arc(layer, &dsc);
    dsc.radius = 3;
    lv_draw_arc(layer, &dsc);

    draw_filled_circle(layer, cx, area->y1 + 21, 2, color_white(), LV_OPA_COVER);
}

static void draw_home_icon(lv_layer_t * layer, const lv_area_t * area)
{
    const int32_t x = area->x1;
    const int32_t y = area->y1;

    draw_line(layer, x + 11, y + 24, x + 24, y + 12, 4, color_white(), LV_OPA_COVER);
    draw_line(layer, x + 24, y + 12, x + 37, y + 24, 4, color_white(), LV_OPA_COVER);
    draw_line(layer, x + 14, y + 23, x + 14, y + 36, 4, color_white(), LV_OPA_COVER);
    draw_line(layer, x + 34, y + 23, x + 34, y + 36, 4, color_white(), LV_OPA_COVER);
    draw_line(layer, x + 14, y + 36, x + 34, y + 36, 4, color_white(), LV_OPA_COVER);
    draw_line(layer, x + 23, y + 36, x + 23, y + 27, 4, color_white(), LV_OPA_COVER);
}

static void draw_check_icon(lv_layer_t * layer, const lv_area_t * area)
{
    const int32_t x = area->x1;
    const int32_t y = area->y1;

    draw_line(layer, x + 14, y + 24, x + 21, y + 31, 5, color_white(), LV_OPA_COVER);
    draw_line(layer, x + 21, y + 31, x + 34, y + 17, 5, color_white(), LV_OPA_COVER);
}

static void draw_scene_icon(lv_layer_t * layer, const lv_area_t * area)
{
    static const lv_point_t offsets[] = {
        {0, -14}, {10, -10}, {14, 0}, {10, 10},
        {0, 14}, {-10, 10}, {-14, 0}, {-10, -10}
    };
    const int32_t cx = (area->x1 + area->x2) / 2;
    const int32_t cy = (area->y1 + area->y2) / 2;
    uint32_t i;

    draw_stroke_circle(layer, cx, cy, 10, 2, color_white(), LV_OPA_COVER);
    for(i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++) {
        draw_filled_circle(layer, cx + offsets[i].x, cy + offsets[i].y, 2, color_white(), LV_OPA_COVER);
    }
}

static void draw_bulb_icon(lv_layer_t * layer, const lv_area_t * area)
{
    const int32_t cx = (area->x1 + area->x2) / 2;
    const int32_t cy = area->y1 + 20;

    draw_stroke_circle(layer, cx, cy, 9, 3, color_white(), LV_OPA_COVER);
    draw_line(layer, cx - 4, area->y1 + 28, cx + 4, area->y1 + 28, 3, color_white(), LV_OPA_COVER);
    draw_line(layer, cx - 3, area->y1 + 32, cx + 3, area->y1 + 32, 3, color_white(), LV_OPA_COVER);
    draw_line(layer, cx, area->y1 + 24, cx, area->y1 + 31, 3, color_white(), LV_OPA_COVER);
}

static void smart_home_icon_draw_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_DRAW_MAIN) {
        return;
    }

    lv_obj_t * obj = lv_event_get_target(e);
    lv_layer_t * layer = lv_event_get_layer(e);
    const lv_area_t * area = &obj->coords;
    const smart_home_icon_t icon = (smart_home_icon_t)(uintptr_t)lv_event_get_user_data(e);

    switch(icon) {
        case SMART_HOME_ICON_WEATHER:
            draw_weather_icon(layer, area);
            break;
        case SMART_HOME_ICON_WIFI:
            draw_wifi_icon(layer, area);
            break;
        case SMART_HOME_ICON_HOME:
            draw_home_icon(layer, area);
            break;
        case SMART_HOME_ICON_CHECK:
            draw_check_icon(layer, area);
            break;
        case SMART_HOME_ICON_SCENE:
            draw_scene_icon(layer, area);
            break;
        case SMART_HOME_ICON_BULB:
            draw_bulb_icon(layer, area);
            break;
    }
}

static lv_obj_t * create_label(
    lv_obj_t * parent,
    int32_t x,
    int32_t y,
    const char * text,
    const lv_font_t * font,
    lv_color_t color,
    lv_opa_t opa
)
{
    lv_obj_t * label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_style_text_opa(label, opa, 0);
    lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(label, 0, 0);
    lv_obj_set_style_pad_all(label, 0, 0);
    lv_obj_set_pos(label, x, y);
    return label;
}

static lv_obj_t * create_icon(lv_obj_t * parent, int32_t x, int32_t y, int32_t w, int32_t h, smart_home_icon_t icon)
{
    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_add_event_cb(obj, smart_home_icon_draw_event_cb, LV_EVENT_DRAW_MAIN, (void *)(uintptr_t)icon);
    return obj;
}

static void apply_card_style(lv_obj_t * card, smart_home_card_style_t style)
{
    lv_obj_remove_style_all(card);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_set_style_clip_corner(card, true, 0);
    lv_obj_set_style_shadow_color(card, color_black(), 0);
    lv_obj_set_style_shadow_width(card, 12, 0);
    lv_obj_set_style_shadow_opa(card, LV_OPA_30, 0);
    lv_obj_set_style_shadow_offset_x(card, 0, 0);
    lv_obj_set_style_shadow_offset_y(card, 2, 0);

    if(style == SMART_HOME_CARD_GRADIENT) {
        lv_obj_set_style_bg_color(card, color_blue_start(), 0);
        lv_obj_set_style_bg_grad_color(card, color_blue_end(), 0);
        lv_obj_set_style_bg_grad_dir(card, LV_GRAD_DIR_HOR, 0);
        lv_obj_set_style_bg_main_stop(card, 0, 0);
        lv_obj_set_style_bg_grad_stop(card, 255, 0);
        lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    }
    else {
        lv_obj_set_style_bg_color(card, color_white(), 0);
        lv_obj_set_style_bg_opa(card, 38, 0);
        lv_obj_set_style_border_width(card, 1, 0);
        lv_obj_set_style_border_color(card, color_white(), 0);
        lv_obj_set_style_border_opa(card, 28, 0);
    }
}

static lv_obj_t * create_card(lv_obj_t * parent, const smart_home_card_spec_t * spec)
{
    lv_obj_t * card = lv_obj_create(parent);
    apply_card_style(card, spec->style);
    lv_obj_set_pos(card, spec->x, spec->y);
    lv_obj_set_size(card, spec->w, spec->h);

    create_icon(card, spec->icon_x, spec->icon_y, spec->icon_w, spec->icon_h, spec->icon);
    create_label(card, spec->text_x, spec->text_y, spec->label, smart_home_font_body_regular_18(), color_white(), 230);

    return card;
}

static void create_status_bar(lv_obj_t * parent)
{
    lv_obj_t * status_bar = lv_obj_create(parent);
    lv_obj_remove_style_all(status_bar);
    lv_obj_set_pos(status_bar, 16, 15);
    lv_obj_set_size(status_bar, 448, 32);

    create_icon(status_bar, 0, 0, 32, 32, SMART_HOME_ICON_WEATHER);
    create_label(status_bar, 39, 2, "24", smart_home_font_metric_bold_22(), color_white(), LV_OPA_COVER);
    create_label(status_bar, 65, 7, "°C", smart_home_font_body_regular_15(), color_white(), 102);

    create_icon(status_bar, 315, 2, 28, 28, SMART_HOME_ICON_WIFI);
    create_label(status_bar, 353, 0, "AM", smart_home_font_caption_bold_10(), color_white(), LV_OPA_COVER);
    create_label(status_bar, 347, 12, "2/24", smart_home_font_caption_bold_10(), color_white(), LV_OPA_COVER);
    create_label(status_bar, 380, 0, "08:20", smart_home_font_metric_bold_26(), color_white(), LV_OPA_COVER);
}

static void create_tabs(lv_obj_t * parent)
{
    create_label(parent, 16, 70, "Scenes", smart_home_font_title_bold_32(), color_white(), 230);
    create_label(parent, 144, 70, "Devices", smart_home_font_title_regular_32(), color_white(), 128);
}

static void create_bottom_overlay(lv_obj_t * parent)
{
    lv_obj_t * overlay = lv_obj_create(parent);
    lv_obj_remove_style_all(overlay);
    lv_obj_set_pos(overlay, 0, 428);
    lv_obj_set_size(overlay, 480, 52);
    lv_obj_set_style_bg_color(overlay, color_black(), 0);
    lv_obj_set_style_bg_opa(overlay, 0, 0);
    lv_obj_set_style_bg_grad_color(overlay, color_black(), 0);
    lv_obj_set_style_bg_grad_dir(overlay, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_main_stop(overlay, 0, 0);
    lv_obj_set_style_bg_grad_stop(overlay, 255, 0);
    lv_obj_set_style_bg_grad_opa(overlay, LV_OPA_COVER, 0);
}

static void create_page_indicator(lv_obj_t * parent)
{
    lv_obj_t * indicator = lv_obj_create(parent);
    lv_obj_remove_style_all(indicator);
    lv_obj_set_pos(indicator, 226, 462);
    lv_obj_set_size(indicator, 28, 6);

    lv_obj_t * dot = lv_obj_create(indicator);
    lv_obj_remove_style_all(dot);
    lv_obj_set_pos(dot, 0, 0);
    lv_obj_set_size(dot, 6, 6);
    lv_obj_set_style_bg_color(dot, color_white(), 0);
    lv_obj_set_style_bg_opa(dot, 128, 0);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);

    lv_obj_t * active = lv_obj_create(indicator);
    lv_obj_remove_style_all(active);
    lv_obj_set_pos(active, 10, 0);
    lv_obj_set_size(active, 18, 6);
    lv_obj_set_style_bg_color(active, color_cyan(), 0);
    lv_obj_set_style_bg_opa(active, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(active, LV_RADIUS_CIRCLE, 0);
}

lv_obj_t * smart_home_page_create(void)
{
    static const smart_home_card_spec_t cards[] = {
        {16, 130, 219, 145, SMART_HOME_CARD_GLASS,    SMART_HOME_ICON_HOME,  16, 31, 48, 48, 16, 80, "Home"},
        {245, 130, 219, 135, SMART_HOME_CARD_GRADIENT, SMART_HOME_ICON_CHECK, 16, 26, 48, 48, 16, 82, "Morning"},
        {16, 285, 219, 145, SMART_HOME_CARD_GRADIENT, SMART_HOME_ICON_SCENE, 16, 31, 48, 48, 16, 87, "Morning"},
        {245, 285, 219, 145, SMART_HOME_CARD_GLASS,   SMART_HOME_ICON_BULB,  16, 31, 48, 48, 16, 87, "All Lights On"},
    };
    uint32_t i;

    if(s_root) {
        lv_screen_load(s_root);
        return s_root;
    }

    smart_home_assets_init();

    s_root = lv_obj_create(NULL);
    lv_obj_remove_style_all(s_root);
    lv_obj_set_size(s_root, 480, 480);
    lv_obj_set_style_bg_color(s_root, color_black(), 0);
    lv_obj_set_style_bg_opa(s_root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(s_root, LV_OBJ_FLAG_SCROLLABLE);

    create_status_bar(s_root);
    create_tabs(s_root);

    for(i = 0; i < sizeof(cards) / sizeof(cards[0]); i++) {
        create_card(s_root, &cards[i]);
    }

    create_bottom_overlay(s_root);
    create_page_indicator(s_root);

    lv_screen_load(s_root);
    return s_root;
}

void smart_home_page_destroy(void)
{
    if(s_root) {
        if(s_root == lv_screen_active()) {
            if(s_placeholder == NULL) {
                s_placeholder = lv_obj_create(NULL);
                lv_obj_remove_style_all(s_placeholder);
                lv_obj_set_style_bg_color(s_placeholder, color_black(), 0);
                lv_obj_set_style_bg_opa(s_placeholder, LV_OPA_COVER, 0);
            }
            lv_screen_load(s_placeholder);
        }
        lv_obj_delete(s_root);
        s_root = NULL;
    }

    smart_home_assets_deinit();
}
