#include "music_player_shell.h"

typedef struct {
    music_player_app_t * app;
    music_player_theme_accent_t accent;
} music_player_theme_click_ctx_t;

typedef struct {
    music_player_app_t * app;
    music_player_settings_group_t group;
} music_player_settings_group_ctx_t;

static music_player_theme_click_ctx_t g_theme_click_ctx[] = {
    { NULL, MUSIC_PLAYER_ACCENT_CYAN },
    { NULL, MUSIC_PLAYER_ACCENT_BLUE },
    { NULL, MUSIC_PLAYER_ACCENT_MINT },
    { NULL, MUSIC_PLAYER_ACCENT_AUTO_1 },
};

static music_player_settings_group_ctx_t g_settings_group_ctx[] = {
    { NULL, MUSIC_PLAYER_SETTINGS_APPEARANCE },
    { NULL, MUSIC_PLAYER_SETTINGS_PLAYBACK },
    { NULL, MUSIC_PLAYER_SETTINGS_ABOUT },
};

static void on_theme_clicked(lv_event_t * e)
{
    music_player_theme_click_ctx_t * ctx =
        (music_player_theme_click_ctx_t *)lv_event_get_user_data(e);
    if(ctx == NULL || ctx->app == NULL) {
        return;
    }

    music_player_theme_set_accent(&ctx->app->theme, ctx->accent);
    if(ctx->accent == MUSIC_PLAYER_ACCENT_AUTO_1) {
        music_player_theme_set_auto_accent(&ctx->app->theme, 0x8B9CF7);
    }
    music_player_shell_refresh(ctx->app);
}

static void on_settings_group_clicked(lv_event_t * e)
{
    music_player_settings_group_ctx_t * ctx =
        (music_player_settings_group_ctx_t *)lv_event_get_user_data(e);
    if(ctx == NULL || ctx->app == NULL) {
        return;
    }

    ctx->app->settings_group = ctx->group;
    music_player_shell_refresh(ctx->app);
}

void music_player_settings_view_build(lv_obj_t * parent, music_player_app_t * app)
{
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(page, LV_LAYOUT_GRID);
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    static const int32_t cols[] = { 180, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static const int32_t rows[] = { 68, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(page, cols, rows);
    lv_obj_set_style_pad_column(page, 14, 0);
    lv_obj_set_style_pad_row(page, 12, 0);

    lv_obj_t * head = lv_obj_create(page);
    lv_obj_remove_style_all(head);
    lv_obj_set_layout(head, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(head, LV_FLEX_FLOW_COLUMN);
    lv_label_set_text(lv_label_create(head), "Appearance Controls");
    lv_label_set_text(lv_label_create(head), "Settings");
    lv_label_set_text(lv_label_create(head), "Verify theme switching and shell stability first.");
    lv_obj_set_grid_cell(head, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START, 0, 1);

    lv_obj_t * nav = lv_obj_create(page);
    lv_obj_remove_style_all(nav);
    lv_obj_set_style_radius(nav, 20, 0);
    lv_obj_set_style_bg_color(nav, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_pad_all(nav, 12, 0);
    lv_obj_set_style_shadow_width(nav, 10, 0);
    lv_obj_set_style_shadow_color(nav, lv_color_hex(0xe7eded), 0);
    lv_obj_set_layout(nav, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(nav, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(nav, 8, 0);
    lv_obj_set_grid_cell(nav, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    for(size_t i = 0; i < music_player_shell_settings_group_count(); ++i) {
        lv_obj_t * group_btn = lv_button_create(nav);
        lv_obj_set_width(group_btn, LV_PCT(100));
        lv_obj_set_style_radius(group_btn, 14, 0);
        lv_obj_set_style_border_width(group_btn, 0, 0);
        if(app->settings_group == g_settings_group_ctx[i].group) {
            lv_obj_set_style_bg_color(group_btn,
                                      lv_color_hex(music_player_theme_accent_hex(&app->theme)), 0);
            lv_obj_set_style_text_color(group_btn, lv_color_white(), 0);
        }
        else {
            lv_obj_set_style_bg_color(group_btn, lv_color_hex(0xf3f8f8), 0);
        }
        g_settings_group_ctx[i].app = app;
        lv_obj_add_event_cb(group_btn, on_settings_group_clicked, LV_EVENT_CLICKED,
                            &g_settings_group_ctx[i]);
        lv_label_set_text(lv_label_create(group_btn),
                          music_player_shell_settings_group_label(g_settings_group_ctx[i].group));
    }

    lv_obj_t * main = lv_obj_create(page);
    lv_obj_remove_style_all(main);
    lv_obj_set_style_radius(main, 20, 0);
    lv_obj_set_style_bg_color(main, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_pad_all(main, 16, 0);
    lv_obj_set_style_shadow_width(main, 10, 0);
    lv_obj_set_style_shadow_color(main, lv_color_hex(0xe7eded), 0);
    lv_obj_set_layout(main, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(main, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(main, 10, 0);
    lv_obj_set_grid_cell(main, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    if(app->settings_group == MUSIC_PLAYER_SETTINGS_APPEARANCE) {
        lv_label_set_text(lv_label_create(main), "Theme Mode");
        lv_obj_t * mode_row = lv_obj_create(main);
        lv_obj_remove_style_all(mode_row);
        lv_obj_set_width(mode_row, LV_PCT(100));
        lv_obj_set_layout(mode_row, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(mode_row, LV_FLEX_FLOW_ROW_WRAP);
        lv_obj_set_style_pad_column(mode_row, 8, 0);
        lv_obj_set_style_pad_row(mode_row, 8, 0);
        lv_obj_t * chip = lv_button_create(mode_row);
        lv_label_set_text(lv_label_create(chip), "Light");
        chip = lv_button_create(mode_row);
        lv_label_set_text(lv_label_create(chip), "Dark");
        chip = lv_button_create(mode_row);
        lv_label_set_text(lv_label_create(chip), "System");

        lv_label_set_text(lv_label_create(main), "Accent");
        lv_obj_t * swatches = lv_obj_create(main);
        lv_obj_remove_style_all(swatches);
        lv_obj_set_width(swatches, LV_PCT(100));
        lv_obj_set_layout(swatches, LV_LAYOUT_GRID);
        static const int32_t sw_cols[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
        static const int32_t sw_rows[] = { LV_SIZE_CONTENT, LV_GRID_TEMPLATE_LAST };
        lv_obj_set_grid_dsc_array(swatches, sw_cols, sw_rows);
        lv_obj_set_style_pad_column(swatches, 8, 0);
        for(size_t i = 0; i < music_player_theme_accent_count(); ++i) {
            const music_player_theme_accent_descriptor_t * item =
                music_player_theme_accent_descriptor((music_player_theme_accent_t)i);
            lv_obj_t * swatch = lv_button_create(swatches);
            lv_obj_set_style_radius(swatch, 18, 0);
            lv_obj_set_style_border_width(swatch, 0, 0);
            if(item->accent == app->theme.accent) {
                lv_obj_set_style_bg_color(swatch,
                                          lv_color_hex(music_player_theme_accent_hex(&app->theme)), 0);
                lv_obj_set_style_text_color(swatch, lv_color_white(), 0);
            }
            else {
                lv_obj_set_style_bg_color(swatch, lv_color_hex(0xf3f8f8), 0);
            }
            lv_obj_set_grid_cell(swatch, LV_GRID_ALIGN_STRETCH, (int32_t)i, 1,
                                 LV_GRID_ALIGN_STRETCH, 0, 1);
            g_theme_click_ctx[i].app = app;
            g_theme_click_ctx[i].accent = item->accent;
            lv_obj_add_event_cb(swatch, on_theme_clicked, LV_EVENT_CLICKED, &g_theme_click_ctx[i]);
            lv_label_set_text(lv_label_create(swatch), item->label);
        }

        lv_label_set_text(lv_label_create(main), "Corner radius 62%");
        lv_label_set_text(lv_label_create(main), "Backdrop soft glow");
        if(app->theme.accent == MUSIC_PLAYER_ACCENT_AUTO_1) {
            char auto_label[32];
            lv_snprintf(auto_label, sizeof(auto_label), "Auto 1 #%06X", app->theme.auto_accent_hex);
            lv_label_set_text(lv_label_create(main), auto_label);
        }
    }
    else if(app->settings_group == MUSIC_PLAYER_SETTINGS_PLAYBACK) {
        lv_label_set_text(lv_label_create(main), "Playback");
        lv_label_set_text(lv_label_create(main), "Auto resume");
        lv_label_set_text(lv_label_create(main), "Crossfade 8s");
        lv_label_set_text(lv_label_create(main), "Radio-first queue");
    }
    else {
        lv_label_set_text(lv_label_create(main), "About");
        lv_label_set_text(lv_label_create(main), "SDL / LVGL prototype");
        lv_label_set_text(lv_label_create(main), "LVGL layout in progress");
        lv_label_set_text(lv_label_create(main), "Next step: visual polish");
    }
}
