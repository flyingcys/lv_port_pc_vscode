#include "music_player.h"
#include "music_player_shell.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define MUSIC_PLAYER_SCREEN_WIDTH 800
#define MUSIC_PLAYER_SCREEN_HEIGHT 480
#define MUSIC_PLAYER_SIDEBAR_WIDTH 164
#define MUSIC_PLAYER_MINI_PLAYER_HEIGHT 78

static music_player_app_t g_app;

static const music_player_nav_descriptor_t g_nav_items[] = {
    { MUSIC_PLAYER_PAGE_HOME, "Home", "H" },
    { MUSIC_PLAYER_PAGE_RADIO, "Radio", "R" },
    { MUSIC_PLAYER_PAGE_LOCAL, "Local", "L" },
    { MUSIC_PLAYER_PAGE_PLAYLIST, "Playlists", "P" },
    { MUSIC_PLAYER_PAGE_SETTINGS, "Settings", "S" },
};

typedef struct {
    music_player_app_t * app;
    music_player_page_t page;
} music_player_nav_click_ctx_t;

static music_player_nav_click_ctx_t g_nav_click_ctx[] = {
    { NULL, MUSIC_PLAYER_PAGE_HOME },
    { NULL, MUSIC_PLAYER_PAGE_RADIO },
    { NULL, MUSIC_PLAYER_PAGE_LOCAL },
    { NULL, MUSIC_PLAYER_PAGE_PLAYLIST },
    { NULL, MUSIC_PLAYER_PAGE_SETTINGS },
};

static const char * const g_settings_group_labels[] = {
    "Appearance",
    "Playback",
    "About",
};

static void clear_object(lv_obj_t * obj);
static void style_root_shell(music_player_app_t * app);
static void build_sidebar(music_player_app_t * app);
static void build_content(music_player_app_t * app);
static void build_mini_player(music_player_app_t * app);
static void build_page_content(music_player_app_t * app);
static void style_panel_card(lv_obj_t * obj, lv_color_t bg, lv_opa_t opa);
static lv_color_t accent_color(const music_player_app_t * app);
static lv_color_t accent_soft_color(const music_player_app_t * app);
static lv_color_t text_color(void);
static lv_color_t muted_color(void);
static lv_color_t shell_background_color(void);
static void on_nav_clicked(lv_event_t * e);
static void on_theme_accent_clicked(lv_event_t * e);
static void on_settings_group_clicked(lv_event_t * e);
static lv_obj_t * create_label(lv_obj_t * parent, const char * text, lv_color_t color,
                               const lv_font_t * font);
static lv_obj_t * create_chip_button(lv_obj_t * parent, const char * text, lv_color_t bg,
                                     lv_color_t color);

void music_player_start(void)
{
    memset(&g_app, 0, sizeof(g_app));
    g_app.page = MUSIC_PLAYER_PAGE_HOME;
    g_app.settings_group = MUSIC_PLAYER_SETTINGS_APPEARANCE;
    g_app.theme = music_player_theme_default_state();
    music_player_shell_create(&g_app);
    lv_screen_load(g_app.screen);
}

void music_player_shell_create(music_player_app_t * app)
{
    app->screen = lv_obj_create(NULL);
    app->root = lv_obj_create(app->screen);
    app->sidebar = lv_obj_create(app->root);
    app->content_shell = lv_obj_create(app->root);
    app->content_host = lv_obj_create(app->content_shell);
    app->mini_player = lv_obj_create(app->content_shell);

    style_root_shell(app);
    music_player_shell_refresh(app);
}

void music_player_shell_refresh(music_player_app_t * app)
{
    style_root_shell(app);
    build_sidebar(app);
    build_content(app);
    build_mini_player(app);
}

void music_player_shell_set_page(music_player_app_t * app, music_player_page_t page)
{
    if(app == NULL) {
        return;
    }

    app->page = page;
    music_player_shell_refresh(app);
}

size_t music_player_shell_nav_count(void)
{
    return sizeof(g_nav_items) / sizeof(g_nav_items[0]);
}

size_t music_player_shell_bottom_nav_index(void)
{
    return music_player_shell_nav_count() - 1U;
}

const music_player_nav_descriptor_t * music_player_shell_nav_descriptor(size_t index)
{
    if(index >= music_player_shell_nav_count()) {
        return NULL;
    }

    return &g_nav_items[index];
}

size_t music_player_shell_settings_group_count(void)
{
    return sizeof(g_settings_group_labels) / sizeof(g_settings_group_labels[0]);
}

const char * music_player_shell_settings_group_label(music_player_settings_group_t group)
{
    if((size_t)group >= music_player_shell_settings_group_count()) {
        return "";
    }

    return g_settings_group_labels[group];
}

static void style_root_shell(music_player_app_t * app)
{
    lv_obj_remove_style_all(app->screen);
    lv_obj_set_size(app->screen, MUSIC_PLAYER_SCREEN_WIDTH, MUSIC_PLAYER_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(app->screen, shell_background_color(), 0);
    lv_obj_set_style_bg_grad_color(app->screen, lv_color_hex(0xf6fbfd), 0);
    lv_obj_set_style_bg_grad_dir(app->screen, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_pad_all(app->screen, 0, 0);

    lv_obj_remove_style_all(app->root);
    lv_obj_set_size(app->root, MUSIC_PLAYER_SCREEN_WIDTH - 16, MUSIC_PLAYER_SCREEN_HEIGHT - 16);
    lv_obj_center(app->root);
    lv_obj_set_style_radius(app->root, 24, 0);
    lv_obj_set_style_bg_color(app->root, lv_color_hex(0xfafcfc), 0);
    lv_obj_set_style_border_width(app->root, 0, 0);
    lv_obj_set_style_pad_all(app->root, 0, 0);
    lv_obj_set_style_shadow_width(app->root, 24, 0);
    lv_obj_set_style_shadow_color(app->root, lv_color_hex(0xd6e7e7), 0);
    lv_obj_set_layout(app->root, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(app->root, LV_FLEX_FLOW_ROW);

    lv_obj_remove_style_all(app->sidebar);
    lv_obj_set_size(app->sidebar, MUSIC_PLAYER_SIDEBAR_WIDTH, LV_PCT(100));
    lv_obj_set_style_bg_color(app->sidebar, lv_color_hex(0xf2fbfa), 0);
    lv_obj_set_style_bg_opa(app->sidebar, LV_OPA_100, 0);
    lv_obj_set_style_border_width(app->sidebar, 0, 0);
    lv_obj_set_style_pad_all(app->sidebar, 16, 0);
    lv_obj_set_style_pad_row(app->sidebar, 10, 0);
    lv_obj_set_style_radius(app->sidebar, 24, 0);
    lv_obj_set_layout(app->sidebar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(app->sidebar, LV_FLEX_FLOW_COLUMN);

    lv_obj_remove_style_all(app->content_shell);
    lv_obj_set_flex_grow(app->content_shell, 1);
    lv_obj_set_height(app->content_shell, LV_PCT(100));
    lv_obj_set_style_bg_opa(app->content_shell, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(app->content_shell, 0, 0);
    lv_obj_set_style_pad_all(app->content_shell, 16, 0);
    lv_obj_set_style_pad_left(app->content_shell, 18, 0);
    lv_obj_set_style_pad_right(app->content_shell, 18, 0);
    lv_obj_set_style_pad_top(app->content_shell, 18, 0);
    lv_obj_set_style_pad_bottom(app->content_shell, 14, 0);
    lv_obj_set_layout(app->content_shell, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(app->content_shell, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(app->content_shell, 12, 0);

    lv_obj_remove_style_all(app->content_host);
    lv_obj_set_width(app->content_host, LV_PCT(100));
    lv_obj_set_flex_grow(app->content_host, 1);
    lv_obj_set_style_bg_opa(app->content_host, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(app->content_host, 0, 0);
    lv_obj_set_style_pad_all(app->content_host, 0, 0);

    lv_obj_remove_style_all(app->mini_player);
    lv_obj_set_width(app->mini_player, LV_PCT(100));
    lv_obj_set_height(app->mini_player, MUSIC_PLAYER_MINI_PLAYER_HEIGHT);
    lv_obj_set_style_radius(app->mini_player, 20, 0);
    lv_obj_set_style_bg_color(app->mini_player, lv_color_hex(0xfefefe), 0);
    lv_obj_set_style_border_width(app->mini_player, 0, 0);
    lv_obj_set_style_pad_all(app->mini_player, 14, 0);
    lv_obj_set_style_shadow_width(app->mini_player, 12, 0);
    lv_obj_set_style_shadow_color(app->mini_player, lv_color_hex(0xe4eded), 0);
}

static void build_sidebar(music_player_app_t * app)
{
    size_t i;

    clear_object(app->sidebar);

    lv_obj_t * brand = lv_obj_create(app->sidebar);
    lv_obj_remove_style_all(brand);
    lv_obj_set_width(brand, LV_PCT(100));
    lv_obj_set_style_bg_opa(brand, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(brand, 0, 0);
    lv_obj_set_layout(brand, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(brand, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(brand, 10, 0);

    lv_obj_t * badge = lv_obj_create(brand);
    lv_obj_remove_style_all(badge);
    lv_obj_set_size(badge, 34, 34);
    lv_obj_set_style_radius(badge, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(badge, accent_color(app), 0);
    lv_obj_set_style_shadow_width(badge, 12, 0);
    lv_obj_set_style_shadow_color(badge, accent_soft_color(app), 0);
    create_label(badge, "M", lv_color_white(), LV_FONT_DEFAULT);

    lv_obj_t * brand_copy = lv_obj_create(brand);
    lv_obj_remove_style_all(brand_copy);
    lv_obj_set_flex_grow(brand_copy, 1);
    lv_obj_set_height(brand_copy, LV_SIZE_CONTENT);
    lv_obj_set_layout(brand_copy, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(brand_copy, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_opa(brand_copy, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(brand_copy, 0, 0);
    create_label(brand_copy, "Broadcast First", text_color(), LV_FONT_DEFAULT);
    create_label(brand_copy, "UI mockup", muted_color(), LV_FONT_DEFAULT);

    create_label(app->sidebar, "NAV", muted_color(), LV_FONT_DEFAULT);

    for(i = 0; i < (sizeof(g_nav_items) / sizeof(g_nav_items[0])) - 1U; ++i) {
        lv_obj_t * btn = lv_button_create(app->sidebar);
        lv_obj_set_width(btn, LV_PCT(100));
        lv_obj_set_height(btn, 40);
        lv_obj_set_style_radius(btn, 16, 0);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_set_style_bg_color(btn,
                                  g_nav_items[i].page == app->page ? accent_soft_color(app) :
                                                                     lv_color_hex(0xeff7f6),
                                  0);
        lv_obj_set_style_bg_opa(btn,
                                g_nav_items[i].page == app->page ? LV_OPA_100 : LV_OPA_60,
                                0);
        lv_obj_set_style_pad_left(btn, 12, 0);
        lv_obj_set_style_pad_right(btn, 12, 0);
        lv_obj_set_style_pad_top(btn, 0, 0);
        lv_obj_set_style_pad_bottom(btn, 0, 0);
        g_nav_click_ctx[i].app = app;
        g_nav_click_ctx[i].page = g_nav_items[i].page;
        lv_obj_add_event_cb(btn, on_nav_clicked, LV_EVENT_CLICKED, &g_nav_click_ctx[i]);

        lv_obj_t * row = lv_obj_create(btn);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, LV_PCT(100), LV_PCT(100));
        lv_obj_set_layout(row, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_style_pad_column(row, 10, 0);
        lv_obj_set_style_align(row, LV_ALIGN_LEFT_MID, 0);

        lv_obj_t * dot = lv_obj_create(row);
        lv_obj_remove_style_all(dot);
        lv_obj_set_size(dot, 24, 24);
        lv_obj_set_style_radius(dot, 12, 0);
        lv_obj_set_style_bg_color(dot,
                                  g_nav_items[i].page == app->page ? accent_color(app) :
                                                                     lv_color_hex(0xffffff),
                                  0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_100, 0);
        create_label(dot, g_nav_items[i].glyph,
                     g_nav_items[i].page == app->page ? lv_color_white() : muted_color(),
                     LV_FONT_DEFAULT);

        create_label(row, g_nav_items[i].label,
                     g_nav_items[i].page == app->page ? accent_color(app) : text_color(),
                     LV_FONT_DEFAULT);
    }

    lv_obj_t * spacer = lv_obj_create(app->sidebar);
    lv_obj_remove_style_all(spacer);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);

    const music_player_nav_descriptor_t * settings = &g_nav_items[music_player_shell_bottom_nav_index()];
    lv_obj_t * btn = lv_button_create(app->sidebar);
    lv_obj_set_width(btn, LV_PCT(100));
    lv_obj_set_height(btn, 40);
    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_bg_color(btn,
                              settings->page == app->page ? accent_soft_color(app) :
                                                             lv_color_hex(0xeff7f6),
                              0);
    lv_obj_set_style_bg_opa(btn,
                            settings->page == app->page ? LV_OPA_100 : LV_OPA_60,
                            0);
    lv_obj_set_style_pad_left(btn, 12, 0);
    lv_obj_set_style_pad_right(btn, 12, 0);
    lv_obj_set_style_pad_top(btn, 0, 0);
    lv_obj_set_style_pad_bottom(btn, 0, 0);
    g_nav_click_ctx[music_player_shell_bottom_nav_index()].app = app;
    g_nav_click_ctx[music_player_shell_bottom_nav_index()].page = settings->page;
    lv_obj_add_event_cb(btn, on_nav_clicked, LV_EVENT_CLICKED,
                        &g_nav_click_ctx[music_player_shell_bottom_nav_index()]);

    lv_obj_t * row = lv_obj_create(btn);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_column(row, 10, 0);
    lv_obj_set_style_align(row, LV_ALIGN_LEFT_MID, 0);

    lv_obj_t * dot = lv_obj_create(row);
    lv_obj_remove_style_all(dot);
    lv_obj_set_size(dot, 24, 24);
    lv_obj_set_style_radius(dot, 12, 0);
    lv_obj_set_style_bg_color(dot,
                              settings->page == app->page ? accent_color(app) :
                                                             lv_color_hex(0xffffff),
                              0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_100, 0);
    create_label(dot, settings->glyph,
                 settings->page == app->page ? lv_color_white() : muted_color(),
                 LV_FONT_DEFAULT);

    create_label(row, settings->label,
                 settings->page == app->page ? accent_color(app) : text_color(),
                 LV_FONT_DEFAULT);
}

static void build_content(music_player_app_t * app)
{
    clear_object(app->content_host);
    build_page_content(app);
}

static void build_mini_player(music_player_app_t * app)
{
    lv_obj_t * art;
    lv_obj_t * left;
    lv_obj_t * center;
    lv_obj_t * right;
    lv_obj_t * slider;

    clear_object(app->mini_player);
    lv_obj_set_layout(app->mini_player, LV_LAYOUT_GRID);

    static const int32_t cols[] = { 180, 120, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static const int32_t rows[] = { LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(app->mini_player, cols, rows);

    left = lv_obj_create(app->mini_player);
    lv_obj_remove_style_all(left);
    lv_obj_set_layout(left, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(left, 0, 0);
    lv_obj_set_style_pad_column(left, 10, 0);
    lv_obj_set_grid_cell(left, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    art = lv_obj_create(left);
    lv_obj_remove_style_all(art);
    lv_obj_set_size(art, 52, 52);
    lv_obj_set_style_radius(art, 16, 0);
    lv_obj_set_style_bg_color(art, accent_color(app), 0);
    lv_obj_set_style_bg_grad_color(art,
                                   lv_color_hex(music_player_theme_hero_start_hex(&app->theme)),
                                   0);
    lv_obj_set_style_bg_grad_dir(art, LV_GRAD_DIR_VER, 0);

    lv_obj_t * copy = lv_obj_create(left);
    lv_obj_remove_style_all(copy);
    lv_obj_set_flex_grow(copy, 1);
    lv_obj_set_layout(copy, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(copy, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(copy, 0, 0);
    create_label(copy, "Lo-Fi Morning Radio", text_color(), LV_FONT_DEFAULT);
    create_label(copy, "Morning Transit Session", muted_color(), LV_FONT_DEFAULT);

    center = lv_obj_create(app->mini_player);
    lv_obj_remove_style_all(center);
    lv_obj_set_layout(center, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(center, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(center, 0, 0);
    lv_obj_set_style_pad_column(center, 10, 0);
    lv_obj_set_grid_cell(center, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    create_chip_button(center, "<<", lv_color_hex(0xf1f7f7), muted_color());
    create_chip_button(center, ">", accent_color(app), lv_color_white());
    create_chip_button(center, ">>", lv_color_hex(0xf1f7f7), muted_color());

    right = lv_obj_create(app->mini_player);
    lv_obj_remove_style_all(right);
    lv_obj_set_layout(right, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_all(right, 0, 0);
    lv_obj_set_grid_cell(right, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    static const int32_t progress_cols[] = { 36, LV_GRID_FR(1), 88, 36, LV_GRID_TEMPLATE_LAST };
    static const int32_t progress_rows[] = { LV_SIZE_CONTENT, LV_SIZE_CONTENT, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(right, progress_cols, progress_rows);

    create_label(right, "01:42", muted_color(), LV_FONT_DEFAULT);
    lv_obj_set_grid_cell(lv_obj_get_child(right, 0), LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 2);
    slider = lv_slider_create(right);
    lv_obj_set_grid_cell(slider, LV_GRID_ALIGN_STRETCH, 1, 2, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 44, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0xe6ecec), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, accent_color(app), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, accent_color(app), LV_PART_KNOB);
    lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider, 0, LV_PART_KNOB);
    lv_obj_set_height(slider, 6);

    create_label(right, "Now Playing", muted_color(), LV_FONT_DEFAULT);
    lv_obj_set_grid_cell(lv_obj_get_child(right, 2), LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_START, 0, 1);
    create_label(right, "03:58", muted_color(), LV_FONT_DEFAULT);
    lv_obj_set_grid_cell(lv_obj_get_child(right, 3), LV_GRID_ALIGN_END, 3, 1, LV_GRID_ALIGN_CENTER, 0, 2);
}

static void build_page_content(music_player_app_t * app)
{
    switch(app->page) {
        case MUSIC_PLAYER_PAGE_HOME:
            music_player_home_view_build(app->content_host, app);
            break;
        case MUSIC_PLAYER_PAGE_RADIO:
            music_player_radio_view_build(app->content_host, app);
            break;
        case MUSIC_PLAYER_PAGE_LOCAL:
            music_player_local_view_build(app->content_host, app);
            break;
        case MUSIC_PLAYER_PAGE_PLAYLIST:
            music_player_playlist_view_build(app->content_host, app);
            break;
        case MUSIC_PLAYER_PAGE_SETTINGS:
            music_player_settings_view_build(app->content_host, app);
            break;
        default:
            music_player_home_view_build(app->content_host, app);
            break;
    }
}

static void clear_object(lv_obj_t * obj)
{
    while(lv_obj_get_child_count(obj) > 0) {
        lv_obj_delete(lv_obj_get_child(obj, 0));
    }
}

static void style_panel_card(lv_obj_t * obj, lv_color_t bg, lv_opa_t opa)
{
    lv_obj_remove_style_all(obj);
    lv_obj_set_style_radius(obj, 20, 0);
    lv_obj_set_style_bg_color(obj, bg, 0);
    lv_obj_set_style_bg_opa(obj, opa, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_shadow_width(obj, 12, 0);
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0xe6eeee), 0);
}

static lv_color_t accent_color(const music_player_app_t * app)
{
    return lv_color_hex(music_player_theme_accent_hex(&app->theme));
}

static lv_color_t accent_soft_color(const music_player_app_t * app)
{
    uint32_t accent = music_player_theme_accent_hex(&app->theme);
    uint8_t r = (accent >> 16) & 0xffU;
    uint8_t g = (accent >> 8) & 0xffU;
    uint8_t b = accent & 0xffU;
    r = (uint8_t)((r + 255U) / 2U);
    g = (uint8_t)((g + 255U) / 2U);
    b = (uint8_t)((b + 255U) / 2U);
    return lv_color_make(r, g, b);
}

static lv_color_t text_color(void)
{
    return lv_color_hex(0x20242d);
}

static lv_color_t muted_color(void)
{
    return lv_color_hex(0x72808f);
}

static lv_color_t shell_background_color(void)
{
    return lv_color_hex(0xe8f4f3);
}

static void on_nav_clicked(lv_event_t * e)
{
    music_player_nav_click_ctx_t * ctx = (music_player_nav_click_ctx_t *)lv_event_get_user_data(e);
    if(ctx == NULL || ctx->app == NULL) {
        return;
    }

    music_player_shell_set_page(ctx->app, ctx->page);
}

static void on_theme_accent_clicked(lv_event_t * e)
{
    music_player_app_t * app = &g_app;
    music_player_theme_accent_t accent = (music_player_theme_accent_t)(uintptr_t)lv_event_get_user_data(e);
    music_player_theme_set_accent(&app->theme, accent);
    music_player_shell_refresh(app);
}

static void on_settings_group_clicked(lv_event_t * e)
{
    LV_UNUSED(e);
}

static lv_obj_t * create_label(lv_obj_t * parent, const char * text, lv_color_t color,
                               const lv_font_t * font)
{
    lv_obj_t * label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, color, 0);
    if(font != NULL) {
        lv_obj_set_style_text_font(label, font, 0);
    }
    return label;
}

static lv_obj_t * create_chip_button(lv_obj_t * parent, const char * text, lv_color_t bg,
                                     lv_color_t color)
{
    lv_obj_t * btn = lv_button_create(parent);
    lv_obj_set_size(btn, 34, 34);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn, bg, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    create_label(btn, text, color, LV_FONT_DEFAULT);
    return btn;
}
