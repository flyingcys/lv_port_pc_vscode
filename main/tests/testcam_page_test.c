#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "lvgl/lvgl.h"
#include "ui/testcam_assets.h"
#include "ui/testcam_fonts.h"
#include "ui/testcam_page.h"

static void test_dummy_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(display);
}

static lv_display_t *test_display_create(void)
{
    static uint8_t draw_buf[TESTCAM_PAGE_WIDTH * TESTCAM_PAGE_HEIGHT * 4U];
    lv_display_t *display = lv_display_create(TESTCAM_PAGE_WIDTH, TESTCAM_PAGE_HEIGHT);

    assert(display != NULL);
    lv_display_set_buffers(display, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_flush_cb(display, test_dummy_flush_cb);

    return display;
}

static void assert_asset_decodes(testcam_asset_id_t id)
{
    lv_image_decoder_dsc_t decoder_dsc;
    lv_image_header_t header;
    const lv_image_dsc_t *src = testcam_assets_get_image_src(id);

    assert(src != NULL);
    assert(lv_image_decoder_get_info(src, &header) == LV_RESULT_OK);

    memset(&decoder_dsc, 0, sizeof(decoder_dsc));
    assert(lv_image_decoder_open(&decoder_dsc, src, NULL) == LV_RESULT_OK);
    lv_image_decoder_close(&decoder_dsc);
}

int main(void)
{
    testcam_page_t page;

    lv_init();
    (void)test_display_create();

    assert(testcam_fonts_init());
    assert_asset_decodes(TESTCAM_ASSET_SAFE_SHIELD_ICON);
    assert_asset_decodes(TESTCAM_ASSET_REMINDER_ICON);
    assert_asset_decodes(TESTCAM_ASSET_WEATHER_TODAY_ICON);
    assert_asset_decodes(TESTCAM_ASSET_WEATHER_TUE_ICON);
    assert_asset_decodes(TESTCAM_ASSET_WEATHER_WED_ICON);
    assert_asset_decodes(TESTCAM_ASSET_NAV_HOME_ICON);
    assert_asset_decodes(TESTCAM_ASSET_NAV_ASSIST_ICON);
    assert_asset_decodes(TESTCAM_ASSET_NAV_HEALTH_ICON);
    assert_asset_decodes(TESTCAM_ASSET_NAV_ALERTS_ICON);
    assert_asset_decodes(TESTCAM_ASSET_DEVICE_CARD_BACKGROUND);

    memset(&page, 0, sizeof(page));
    assert(testcam_page_create(&page, lv_screen_active()));
    assert(page.root != NULL);
    lv_obj_update_layout(page.root);
    assert(lv_obj_get_width(page.root) == TESTCAM_PAGE_WIDTH);
    assert(lv_obj_get_height(page.root) == TESTCAM_PAGE_HEIGHT);
    assert(strcmp(lv_label_get_text(page.time_label), "10:45") == 0);
    assert(strcmp(lv_label_get_text(page.date_label), "MONDAY, OCT 24") == 0);
    assert(testcam_page_get_active_nav_index(&page) == TESTCAM_NAV_HOME);
    assert(!testcam_page_is_reminder_taken(&page));

    lv_obj_send_event(page.reminder_button, LV_EVENT_CLICKED, NULL);
    assert(testcam_page_is_reminder_taken(&page));
    assert(strcmp(lv_label_get_text(page.reminder_button_label), "TAKEN") == 0);

    lv_obj_send_event(page.nav_buttons[TESTCAM_NAV_ASSIST], LV_EVENT_CLICKED, NULL);
    assert(testcam_page_get_active_nav_index(&page) == TESTCAM_NAV_ASSIST);

    testcam_page_destroy(&page);
    testcam_fonts_deinit();
    return 0;
}
