#include "music_player_shell.h"

static void build_list_page(lv_obj_t * parent, music_player_app_t * app, const char * eyebrow,
                            const char * title, const char * subtitle, const char * banner_title,
                            const char * banner_desc, const char * side_title);

void music_player_radio_view_build(lv_obj_t * parent, music_player_app_t * app)
{
    build_list_page(parent, app, "RADIO BROWSE", "Radio",
                    "Expand the radio-first home into a station browser.",
                    "Station Picks", "Confirm hierarchy first, wire real data later.",
                    "Queue: 4 items");
}

static void build_list_page(lv_obj_t * parent, music_player_app_t * app, const char * eyebrow,
                            const char * title, const char * subtitle, const char * banner_title,
                            const char * banner_desc, const char * side_title)
{
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(page, LV_LAYOUT_GRID);
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_set_style_pad_row(page, 12, 0);
    static const int32_t cols[] = { LV_GRID_FR(1), 236, LV_GRID_TEMPLATE_LAST };
    static const int32_t rows[] = { 68, 112, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(page, cols, rows);
    lv_obj_set_style_pad_column(page, 14, 0);

    lv_obj_t * head = lv_obj_create(page);
    lv_obj_remove_style_all(head);
    lv_obj_set_layout(head, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(head, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_opa(head, LV_OPA_TRANSP, 0);
    lv_label_set_text(lv_label_create(head), eyebrow);
    lv_label_set_text(lv_label_create(head), title);
    lv_label_set_text(lv_label_create(head), subtitle);
    lv_obj_set_grid_cell(head, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START, 0, 1);

    lv_obj_t * banner = lv_obj_create(page);
    lv_obj_remove_style_all(banner);
    lv_obj_set_style_radius(banner, 24, 0);
    lv_obj_set_style_bg_color(banner, lv_color_hex(music_player_theme_hero_end_hex(&app->theme)), 0);
    lv_obj_set_style_bg_grad_color(banner,
                                   lv_color_hex(music_player_theme_hero_start_hex(&app->theme)),
                                   0);
    lv_obj_set_style_bg_grad_dir(banner, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_pad_all(banner, 18, 0);
    lv_obj_set_layout(banner, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(banner, LV_FLEX_FLOW_COLUMN);
    lv_label_set_text(lv_label_create(banner), banner_title);
    lv_label_set_text(lv_label_create(banner), banner_desc);
    lv_obj_set_grid_cell(banner, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t * list = lv_obj_create(page);
    lv_obj_remove_style_all(list);
    lv_obj_set_style_radius(list, 20, 0);
    lv_obj_set_style_bg_color(list, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_pad_all(list, 14, 0);
    lv_obj_set_style_shadow_width(list, 10, 0);
    lv_obj_set_style_shadow_color(list, lv_color_hex(0xe7eded), 0);
    lv_obj_set_layout(list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(list, 8, 0);
    lv_obj_set_grid_cell(list, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
    for(size_t i = 0; i < music_player_radio_count(); ++i) {
        const music_player_station_t * item = &music_player_radio_items()[i];
        lv_obj_t * row = lv_obj_create(list);
        lv_obj_remove_style_all(row);
        lv_obj_set_width(row, LV_PCT(100));
        lv_obj_set_style_radius(row, 16, 0);
        lv_obj_set_style_bg_color(row, lv_color_hex(0xf4f8f8), 0);
        lv_obj_set_style_pad_all(row, 10, 0);
        lv_obj_set_layout(row, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
        lv_label_set_text(lv_label_create(row), item->title);
        lv_label_set_text(lv_label_create(row), item->subtitle);
    }

    lv_obj_t * side = lv_obj_create(page);
    lv_obj_remove_style_all(side);
    lv_obj_set_style_radius(side, 20, 0);
    lv_obj_set_style_bg_color(side, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_pad_all(side, 14, 0);
    lv_obj_set_style_shadow_width(side, 10, 0);
    lv_obj_set_style_shadow_color(side, lv_color_hex(0xe7eded), 0);
    lv_obj_set_layout(side, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(side, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(side, 8, 0);
    lv_obj_set_grid_cell(side, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
    lv_label_set_text(lv_label_create(side), side_title);
    lv_label_set_text(lv_label_create(side), "Host intro");
    lv_label_set_text(lv_label_create(side), "New tracks");
    lv_label_set_text(lv_label_create(side), "DJ note");
}
