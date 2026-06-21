#include "music_player_shell.h"

static lv_obj_t * create_title_block(lv_obj_t * parent, const char * eyebrow, const char * title,
                                     const char * subtitle);
static lv_obj_t * create_info_card(lv_obj_t * parent, const char * title);
static void fill_station_rows(lv_obj_t * list, const music_player_station_t * items, size_t count);
static void fill_recent_rows(lv_obj_t * list, const music_player_recent_item_t * items, size_t count);

void music_player_home_view_build(lv_obj_t * parent, music_player_app_t * app)
{
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_t * hero;
    lv_obj_t * aside;
    lv_obj_t * card;
    lv_obj_t * list;
    lv_obj_t * metrics;

    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_set_layout(page, LV_LAYOUT_GRID);

    static const int32_t cols[] = { LV_GRID_FR(5), LV_GRID_FR(3), LV_GRID_TEMPLATE_LAST };
    static const int32_t rows[] = { 68, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(page, cols, rows);
    lv_obj_set_style_pad_column(page, 14, 0);
    lv_obj_set_style_pad_row(page, 14, 0);

    create_title_block(page, "RADIO FIRST", "Home",
                       "Start with radio, then return to recent plays.");
    lv_obj_set_grid_cell(lv_obj_get_child(page, 0), LV_GRID_ALIGN_STRETCH, 0, 2,
                         LV_GRID_ALIGN_START, 0, 1);

    hero = lv_obj_create(page);
    lv_obj_remove_style_all(hero);
    lv_obj_set_grid_cell(hero, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_style_radius(hero, 28, 0);
    lv_obj_set_style_bg_color(hero, lv_color_hex(music_player_theme_hero_end_hex(&app->theme)), 0);
    lv_obj_set_style_bg_grad_color(hero,
                                   lv_color_hex(music_player_theme_hero_start_hex(&app->theme)),
                                   0);
    lv_obj_set_style_bg_grad_dir(hero, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_pad_all(hero, 22, 0);
    lv_obj_set_layout(hero, LV_LAYOUT_GRID);
    static const int32_t hero_cols[] = { LV_GRID_FR(3), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST };
    static const int32_t hero_rows[] = { LV_SIZE_CONTENT, LV_SIZE_CONTENT, LV_GRID_FR(1), LV_SIZE_CONTENT, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(hero, hero_cols, hero_rows);

    lv_obj_t * eyebrow = lv_label_create(hero);
    lv_label_set_text(eyebrow, "LIVE RADIO SELECTION");
    lv_obj_set_style_text_color(eyebrow, lv_color_white(), 0);
    lv_obj_set_style_text_letter_space(eyebrow, 2, 0);
    lv_obj_set_grid_cell(eyebrow, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);

    lv_obj_t * pill = lv_button_create(hero);
    lv_obj_set_style_radius(pill, 16, 0);
    lv_obj_set_style_bg_color(pill, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(pill, LV_OPA_20, 0);
    lv_obj_set_style_border_width(pill, 0, 0);
    lv_obj_set_style_pad_hor(pill, 10, 0);
    lv_obj_set_style_pad_ver(pill, 8, 0);
    lv_obj_set_grid_cell(pill, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_START, 0, 1);
    lv_label_set_text(lv_label_create(pill), "Featured");
    lv_obj_set_style_text_color(lv_obj_get_child(pill, 0), lv_color_white(), 0);

    lv_obj_t * title = lv_label_create(hero);
    lv_label_set_text(title, "Start with radio, tempo already set");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, LV_FONT_DEFAULT, 0);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(title, 250);
    lv_obj_set_grid_cell(title, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 1, 1);

    lv_obj_t * desc = lv_label_create(hero);
    lv_label_set_text(desc, "Hear featured stations first, then jump back to recents.");
    lv_obj_set_style_text_color(desc, lv_color_hex(0xf4fffe), 0);
    lv_label_set_long_mode(desc, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(desc, 220);
    lv_obj_set_grid_cell(desc, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 2, 1);

    lv_obj_t * action_row = lv_obj_create(hero);
    lv_obj_remove_style_all(action_row);
    lv_obj_set_layout(action_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(action_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(action_row, 0, 0);
    lv_obj_set_style_pad_column(action_row, 10, 0);
    lv_obj_set_grid_cell(action_row, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_END, 3, 1);
    lv_obj_t * btn_primary = lv_button_create(action_row);
    lv_obj_set_style_radius(btn_primary, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_primary, lv_color_white(), 0);
    lv_obj_set_style_border_width(btn_primary, 0, 0);
    lv_obj_set_style_pad_hor(btn_primary, 16, 0);
    lv_obj_set_style_pad_ver(btn_primary, 10, 0);
    lv_label_set_text(lv_label_create(btn_primary), "Resume");
    lv_obj_t * btn_secondary = lv_button_create(action_row);
    lv_obj_set_style_radius(btn_secondary, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_secondary, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(btn_secondary, LV_OPA_20, 0);
    lv_obj_set_style_border_width(btn_secondary, 0, 0);
    lv_obj_set_style_pad_hor(btn_secondary, 16, 0);
    lv_obj_set_style_pad_ver(btn_secondary, 10, 0);
    lv_label_set_text(lv_label_create(btn_secondary), "Explore");
    lv_obj_set_style_text_color(lv_obj_get_child(btn_secondary, 0), lv_color_white(), 0);

    lv_obj_t * ring = lv_obj_create(hero);
    lv_obj_remove_style_all(ring);
    lv_obj_set_size(ring, 122, 122);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(ring, 16, 0);
    lv_obj_set_style_border_color(ring, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_opa(ring, LV_OPA_20, 0);
    lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, 0);
    lv_obj_set_grid_cell(ring, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_END, 1, 3);

    aside = lv_obj_create(page);
    lv_obj_remove_style_all(aside);
    lv_obj_set_style_bg_opa(aside, LV_OPA_TRANSP, 0);
    lv_obj_set_grid_cell(aside, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_layout(aside, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(aside, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(aside, 0, 0);
    lv_obj_set_style_pad_row(aside, 12, 0);

    card = create_info_card(aside, "Radio Picks");
    list = lv_obj_get_child(card, 1);
    fill_station_rows(list, music_player_radio_items(), music_player_radio_count());

    card = create_info_card(aside, "Recent Plays");
    list = lv_obj_get_child(card, 1);
    fill_recent_rows(list, music_player_recent_items(), music_player_recent_count());

    metrics = lv_obj_create(card);
    lv_obj_remove_style_all(metrics);
    lv_obj_set_width(metrics, LV_PCT(100));
    lv_obj_set_layout(metrics, LV_LAYOUT_GRID);
    static const int32_t metric_cols[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static const int32_t metric_rows[] = { LV_SIZE_CONTENT, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(metrics, metric_cols, metric_rows);
    lv_obj_set_style_pad_column(metrics, 8, 0);
    lv_obj_set_style_pad_top(metrics, 8, 0);
    for(size_t i = 0; i < 3; ++i) {
        lv_obj_t * metric = lv_obj_create(metrics);
        lv_obj_remove_style_all(metric);
        lv_obj_set_style_radius(metric, 14, 0);
        lv_obj_set_style_bg_color(metric, lv_color_hex(0xf3f8f8), 0);
        lv_obj_set_style_pad_all(metric, 10, 0);
        lv_obj_set_grid_cell(metric, LV_GRID_ALIGN_STRETCH, (int32_t)i, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
        lv_obj_set_layout(metric, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(metric, LV_FLEX_FLOW_COLUMN);
        if(i == 0) {
            lv_label_set_text(lv_label_create(metric), "Saved");
            lv_label_set_text(lv_label_create(metric), "12");
        }
        else if(i == 1) {
            lv_label_set_text(lv_label_create(metric), "Session");
            lv_label_set_text(lv_label_create(metric), "48m");
        }
        else {
            lv_label_set_text(lv_label_create(metric), "Today");
            lv_label_set_text(lv_label_create(metric), "06");
        }
    }
}

static lv_obj_t * create_title_block(lv_obj_t * parent, const char * eyebrow, const char * title,
                                     const char * subtitle)
{
    lv_obj_t * wrap = lv_obj_create(parent);
    lv_obj_remove_style_all(wrap);
    lv_obj_set_layout(wrap, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(wrap, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_opa(wrap, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(wrap, 0, 0);
    lv_obj_set_style_pad_row(wrap, 4, 0);
    lv_label_set_text(lv_label_create(wrap), eyebrow);
    lv_label_set_text(lv_label_create(wrap), title);
    lv_label_set_text(lv_label_create(wrap), subtitle);
    return wrap;
}

static lv_obj_t * create_info_card(lv_obj_t * parent, const char * title)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_remove_style_all(card);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_style_radius(card, 22, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_pad_all(card, 16, 0);
    lv_obj_set_style_shadow_width(card, 12, 0);
    lv_obj_set_style_shadow_color(card, lv_color_hex(0xe4eded), 0);
    lv_obj_set_layout(card, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(card, 10, 0);
    lv_label_set_text(lv_label_create(card), title);

    lv_obj_t * list = lv_obj_create(card);
    lv_obj_remove_style_all(list);
    lv_obj_set_width(list, LV_PCT(100));
    lv_obj_set_layout(list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_style_pad_row(list, 8, 0);

    return card;
}

static void fill_station_rows(lv_obj_t * list, const music_player_station_t * items, size_t count)
{
    for(size_t i = 0; i < count; ++i) {
        lv_obj_t * row = lv_obj_create(list);
        lv_obj_remove_style_all(row);
        lv_obj_set_width(row, LV_PCT(100));
        lv_obj_set_style_radius(row, 16, 0);
        lv_obj_set_style_bg_color(row, lv_color_hex(0xf3f8f8), 0);
        lv_obj_set_style_pad_all(row, 10, 0);
        lv_obj_set_layout(row, LV_LAYOUT_GRID);
        static const int32_t cols[] = { 42, LV_GRID_FR(1), 28, LV_GRID_TEMPLATE_LAST };
        static const int32_t rows[] = { LV_SIZE_CONTENT, LV_SIZE_CONTENT, LV_GRID_TEMPLATE_LAST };
        lv_obj_set_grid_dsc_array(row, cols, rows);

        lv_obj_t * thumb = lv_obj_create(row);
        lv_obj_remove_style_all(thumb);
        lv_obj_set_size(thumb, 42, 42);
        lv_obj_set_style_radius(thumb, 14, 0);
        lv_obj_set_style_bg_color(thumb, lv_color_hex(0xd8f4ee), 0);
        lv_obj_set_grid_cell(thumb, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 2);
        lv_label_set_text(lv_label_create(row), items[i].title);
        lv_obj_set_grid_cell(lv_obj_get_child(row, 1), LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 0, 1);
        lv_label_set_text(lv_label_create(row), items[i].subtitle);
        lv_obj_set_grid_cell(lv_obj_get_child(row, 2), LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 1, 1);
        lv_obj_t * play = lv_button_create(row);
        lv_obj_set_style_radius(play, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(play, lv_color_hex(0xdff5f1), 0);
        lv_obj_set_style_border_width(play, 0, 0);
        lv_obj_set_size(play, 28, 28);
        lv_label_set_text(lv_label_create(play), ">");
        lv_obj_set_grid_cell(play, LV_GRID_ALIGN_END, 2, 1, LV_GRID_ALIGN_CENTER, 0, 2);
    }
}

static void fill_recent_rows(lv_obj_t * list, const music_player_recent_item_t * items, size_t count)
{
    for(size_t i = 0; i < count; ++i) {
        lv_obj_t * row = lv_obj_create(list);
        lv_obj_remove_style_all(row);
        lv_obj_set_width(row, LV_PCT(100));
        lv_obj_set_style_radius(row, 16, 0);
        lv_obj_set_style_bg_color(row, lv_color_hex(0xf5f7fa), 0);
        lv_obj_set_style_pad_all(row, 10, 0);
        lv_obj_set_layout(row, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(row, 4, 0);
        lv_label_set_text(lv_label_create(row), items[i].title);
        lv_label_set_text(lv_label_create(row), items[i].subtitle);
    }
}
