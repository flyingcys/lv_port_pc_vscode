#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "lvgl.h"

#include "../desktop/desktop.h"
#include "../desktop/desktop_data.h"
#include "../desktop/desktop_layout.h"
#include "../desktop/desktop_metrics.h"

static uint32_t g_draw_buf[800 * 480];

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
}

static bool is_app_name(const char *text)
{
    for(uint32_t i = 0; i < desktop_app_count; i++) {
        if(strcmp(text, desktop_apps[i].name) == 0) {
            return true;
        }
    }

    return false;
}

static uint32_t count_app_labels(lv_obj_t *obj)
{
    uint32_t count = 0;
    uint32_t child_count = lv_obj_get_child_count(obj);

    if(lv_obj_check_type(obj, &lv_label_class) && is_app_name(lv_label_get_text(obj))) {
        count++;
    }

    for(uint32_t i = 0; i < child_count; i++) {
        count += count_app_labels(lv_obj_get_child(obj, i));
    }

    return count;
}

int main(void)
{
    uint32_t expected_app_labels = 0;

    for(uint32_t i = 0; i < desktop_app_count; i++) {
        if(desktop_apps[i].page > 0 && desktop_apps[i].page < DESKTOP_PAGE_COUNT) {
            expected_app_labels++;
        }
    }

    lv_init();
    lv_display_t *disp = lv_display_create(800, 480);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, g_draw_buf, NULL, sizeof(g_draw_buf), LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_default(disp);
    desktop_set_resolution(800, 480);

    desktop_run();
    lv_obj_update_layout(lv_screen_active());

    assert(count_app_labels(lv_screen_active()) == expected_app_labels);

    lv_display_delete(disp);
    return 0;
}
