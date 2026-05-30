#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/v9_music_player/music_player_shell.h"

#define TEST_WIDTH 800
#define TEST_HEIGHT 480

static uint8_t * g_last_flushed_buf;

static void flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map)
{
    LV_UNUSED(area);
    g_last_flushed_buf = px_map;
    lv_display_flush_ready(display);
}

static void assert_shell_tree(const music_player_app_t * app)
{
    assert(app->screen != 0);
    assert(app->root != 0);
    assert(app->sidebar != 0);
    assert(app->content_shell != 0);
    assert(app->content_host != 0);
    assert(app->mini_player != 0);
    assert(lv_obj_get_child_count(app->root) == 2);
    assert(lv_obj_get_child_count(app->content_shell) == 2);
    assert(lv_obj_get_child_count(app->sidebar) >= music_player_shell_nav_count());
    assert(lv_obj_get_child_count(app->content_host) == 1);
    assert(lv_obj_get_child_count(app->mini_player) == 3);
}

static void assert_nonblank_framebuffer(const lv_color32_t * framebuffer)
{
    const uint32_t total = TEST_WIDTH * TEST_HEIGHT;
    const lv_color32_t first = framebuffer[0];
    uint32_t different = 0;

    for(uint32_t i = 1; i < total; ++i) {
        if(memcmp(&framebuffer[i], &first, sizeof(first)) != 0) {
            different++;
            if(different > 128U) {
                return;
            }
        }
    }

    assert(!"framebuffer should contain visible variation");
}

static lv_obj_t * sidebar_nav_button_at(const music_player_app_t * app, size_t nav_index)
{
    size_t button_index = 0;
    const uint32_t child_count = lv_obj_get_child_count(app->sidebar);

    for(uint32_t i = 0; i < child_count; ++i) {
        lv_obj_t * child = lv_obj_get_child(app->sidebar, i);
        if(lv_obj_check_type(child, &lv_button_class)) {
            if(button_index == nav_index) {
                return child;
            }
            button_index++;
        }
    }

    return NULL;
}

static void click_sidebar_nav(music_player_app_t * app, size_t nav_index)
{
    lv_obj_t * button = sidebar_nav_button_at(app, nav_index);
    assert(button != 0);
    lv_obj_send_event(button, LV_EVENT_CLICKED, NULL);
}

static lv_obj_t * content_page(const music_player_app_t * app)
{
    assert(lv_obj_get_child_count(app->content_host) == 1);
    return lv_obj_get_child(app->content_host, 0);
}

static lv_obj_t * settings_group_button_at(const music_player_app_t * app, size_t group_index)
{
    lv_obj_t * page = content_page(app);
    assert(lv_obj_get_child_count(page) >= 2);
    lv_obj_t * settings_nav = lv_obj_get_child(page, 1);
    assert(group_index < lv_obj_get_child_count(settings_nav));
    lv_obj_t * button = lv_obj_get_child(settings_nav, (uint32_t)group_index);
    assert(lv_obj_check_type(button, &lv_button_class));
    return button;
}

static lv_obj_t * settings_accent_swatch_at(const music_player_app_t * app, size_t accent_index)
{
    lv_obj_t * page = content_page(app);
    assert(lv_obj_get_child_count(page) >= 3);
    lv_obj_t * main = lv_obj_get_child(page, 2);
    assert(lv_obj_get_child_count(main) >= 4);
    lv_obj_t * swatches = lv_obj_get_child(main, 3);
    assert(accent_index < lv_obj_get_child_count(swatches));
    lv_obj_t * swatch = lv_obj_get_child(swatches, (uint32_t)accent_index);
    assert(lv_obj_check_type(swatch, &lv_button_class));
    return swatch;
}

static void click_settings_group(music_player_app_t * app, music_player_settings_group_t group)
{
    lv_obj_t * button = settings_group_button_at(app, (size_t)group);
    lv_obj_send_event(button, LV_EVENT_CLICKED, NULL);
}

static void click_settings_accent(music_player_app_t * app, music_player_theme_accent_t accent)
{
    lv_obj_t * swatch = settings_accent_swatch_at(app, (size_t)accent);
    lv_obj_send_event(swatch, LV_EVENT_CLICKED, NULL);
}

static void write_framebuffer_ppm(const lv_color32_t * framebuffer, const char * path)
{
    FILE * file = fopen(path, "wb");
    assert(file != 0);
    fprintf(file, "P6\n%d %d\n255\n", TEST_WIDTH, TEST_HEIGHT);

    const uint32_t total = TEST_WIDTH * TEST_HEIGHT;
    for(uint32_t i = 0; i < total; ++i) {
        fputc(framebuffer[i].red, file);
        fputc(framebuffer[i].green, file);
        fputc(framebuffer[i].blue, file);
    }

    assert(fclose(file) == 0);
}

int main(void)
{
    lv_init();
    static lv_color32_t framebuffer[(TEST_WIDTH + LV_DRAW_BUF_STRIDE_ALIGN - 1) * TEST_HEIGHT +
                                    LV_DRAW_BUF_ALIGN];
    lv_display_t * display = lv_display_create(TEST_WIDTH, TEST_HEIGHT);
    assert(display != 0);
    lv_display_set_buffers(display, lv_draw_buf_align(framebuffer, LV_COLOR_FORMAT_ARGB8888), NULL,
                           TEST_WIDTH * TEST_HEIGHT * 4, LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(display, flush_cb);

    music_player_app_t app;
    memset(&app, 0, sizeof(app));
    app.page = MUSIC_PLAYER_PAGE_HOME;
    app.settings_group = MUSIC_PLAYER_SETTINGS_APPEARANCE;
    app.theme = music_player_theme_default_state();

    music_player_shell_create(&app);
    lv_screen_load(app.screen);
    assert_shell_tree(&app);

    lv_obj_t * first_mini_player = app.mini_player;
    click_sidebar_nav(&app, 1);
    assert(app.page == MUSIC_PLAYER_PAGE_RADIO);
    assert(app.mini_player == first_mini_player);
    assert_shell_tree(&app);

    click_sidebar_nav(&app, music_player_shell_bottom_nav_index());
    assert(app.page == MUSIC_PLAYER_PAGE_SETTINGS);
    assert(app.settings_group == MUSIC_PLAYER_SETTINGS_APPEARANCE);
    assert(app.mini_player == first_mini_player);
    assert_shell_tree(&app);

    click_settings_group(&app, MUSIC_PLAYER_SETTINGS_PLAYBACK);
    assert(app.page == MUSIC_PLAYER_PAGE_SETTINGS);
    assert(app.settings_group == MUSIC_PLAYER_SETTINGS_PLAYBACK);
    assert_shell_tree(&app);

    click_settings_group(&app, MUSIC_PLAYER_SETTINGS_APPEARANCE);
    assert(app.settings_group == MUSIC_PLAYER_SETTINGS_APPEARANCE);
    assert_shell_tree(&app);

    click_settings_accent(&app, MUSIC_PLAYER_ACCENT_AUTO_1);
    assert(app.theme.accent == MUSIC_PLAYER_ACCENT_AUTO_1);
    assert(music_player_theme_accent_hex(&app.theme) == 0x8B9CF7);
    assert_shell_tree(&app);

    for(size_t i = 0; i < music_player_shell_nav_count(); ++i) {
        const music_player_nav_descriptor_t * nav = music_player_shell_nav_descriptor(i);
        assert(nav != 0);
        music_player_shell_set_page(&app, nav->page);
        assert(app.page == nav->page);
        assert_shell_tree(&app);
    }

    music_player_theme_set_accent(&app.theme, MUSIC_PLAYER_ACCENT_BLUE);
    music_player_shell_refresh(&app);
    assert(app.theme.accent == MUSIC_PLAYER_ACCENT_BLUE);
    assert_shell_tree(&app);

    music_player_theme_set_accent(&app.theme, MUSIC_PLAYER_ACCENT_AUTO_1);
    music_player_theme_set_auto_accent(&app.theme, 0x8B9CF7);
    music_player_shell_refresh(&app);
    assert(music_player_theme_accent_hex(&app.theme) == 0x8B9CF7);
    assert_shell_tree(&app);

    music_player_shell_set_page(&app, MUSIC_PLAYER_PAGE_HOME);
    assert(app.page == MUSIC_PLAYER_PAGE_HOME);
    assert_shell_tree(&app);

    music_player_app_t first_app;
    music_player_app_t second_app;
    memset(&first_app, 0, sizeof(first_app));
    memset(&second_app, 0, sizeof(second_app));
    first_app.page = MUSIC_PLAYER_PAGE_HOME;
    first_app.settings_group = MUSIC_PLAYER_SETTINGS_APPEARANCE;
    first_app.theme = music_player_theme_default_state();
    second_app.page = MUSIC_PLAYER_PAGE_HOME;
    second_app.settings_group = MUSIC_PLAYER_SETTINGS_APPEARANCE;
    second_app.theme = music_player_theme_default_state();
    music_player_shell_create(&first_app);
    music_player_shell_create(&second_app);

    click_sidebar_nav(&first_app, 1);
    assert(first_app.page == MUSIC_PLAYER_PAGE_RADIO);
    assert(second_app.page == MUSIC_PLAYER_PAGE_HOME);

    music_player_shell_set_page(&first_app, MUSIC_PLAYER_PAGE_SETTINGS);
    click_settings_group(&first_app, MUSIC_PLAYER_SETTINGS_PLAYBACK);
    assert(first_app.settings_group == MUSIC_PLAYER_SETTINGS_PLAYBACK);
    assert(second_app.settings_group == MUSIC_PLAYER_SETTINGS_APPEARANCE);

    click_settings_group(&first_app, MUSIC_PLAYER_SETTINGS_APPEARANCE);
    click_settings_accent(&first_app, MUSIC_PLAYER_ACCENT_AUTO_1);
    assert(first_app.theme.accent == MUSIC_PLAYER_ACCENT_AUTO_1);
    assert(second_app.theme.accent == MUSIC_PLAYER_ACCENT_CYAN);

    lv_obj_invalidate(app.screen);
    lv_refr_now(display);
    assert(g_last_flushed_buf != 0);
    const lv_color32_t * rendered_framebuffer =
        (const lv_color32_t *)lv_draw_buf_align(framebuffer, LV_COLOR_FORMAT_ARGB8888);
    assert_nonblank_framebuffer(rendered_framebuffer);
    write_framebuffer_ppm(rendered_framebuffer, "/tmp/music_player_lvgl_offscreen.ppm");

    return 0;
}
