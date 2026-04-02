#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "lvgl/lvgl.h"
#include "model/device_state.h"
#include "ui/home_page.h"
#include "ui/ui_fonts.h"

static void dummy_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *color_p)
{
    LV_UNUSED(area);
    LV_UNUSED(color_p);
    lv_display_flush_ready(disp);
}

static lv_display_t *create_test_display(void)
{
    static uint8_t draw_buf[480 * 480 * 4];
    lv_display_t *disp = lv_display_create(480, 480);
    if(disp == NULL) {
        return NULL;
    }

    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, dummy_flush_cb);
    return disp;
}

int main(void)
{
    device_state_t state;
    home_page_t page;
    lv_display_t *disp;

    lv_init();
    disp = create_test_display();
    assert(disp != NULL);

    device_state_init_defaults(&state);
    assert(ui_fonts_init());
    assert(home_page_create(&page, lv_screen_active(), &state));

    assert(home_page_get_root(&page) != NULL);
    assert(page.background_image != NULL);
    assert(strcmp(lv_label_get_text(page.clock_top_label), "09") == 0);
    assert(strcmp(lv_label_get_text(page.clock_bottom_label), "26") == 0);
    assert(strcmp(lv_label_get_text(page.meridiem_label), "AM") == 0);
    assert(strcmp(lv_label_get_text(page.switch_name_labels[0]), "Switch 1") == 0);
    assert(strcmp(lv_label_get_text(page.switch_name_labels[1]), "Switch 2") == 0);

    home_page_destroy(&page);
    ui_fonts_deinit();
    return 0;
}
