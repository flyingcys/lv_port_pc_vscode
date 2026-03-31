#include "home_assets.h"
#include "home_page.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

static void dummy_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
}

static bool tree_has_image_src(lv_obj_t * root, const char * expected_src)
{
    uint32_t i;

    if(root == NULL || expected_src == NULL) {
        return false;
    }

    if(lv_obj_check_type(root, &lv_image_class)) {
        const char * src = (const char *)lv_image_get_src(root);
        if(src != NULL && strcmp(src, expected_src) == 0) {
            return true;
        }
    }

    for(i = 0; i < lv_obj_get_child_count(root); i++) {
        if(tree_has_image_src(lv_obj_get_child(root, (int32_t)i), expected_src)) {
            return true;
        }
    }

    return false;
}

static uint32_t tree_count_label_text(lv_obj_t * root, const char * expected_text)
{
    uint32_t count;
    uint32_t i;

    if(root == NULL || expected_text == NULL) {
        return 0;
    }

    count = 0;
    if(lv_obj_check_type(root, &lv_label_class)) {
        const char * text = lv_label_get_text(root);
        if(text != NULL && strcmp(text, expected_text) == 0) {
            count++;
        }
    }

    for(i = 0; i < lv_obj_get_child_count(root); i++) {
        count += tree_count_label_text(lv_obj_get_child(root, (int32_t)i), expected_text);
    }

    return count;
}

int main(void)
{
    lv_display_t * disp;
    lv_draw_buf_t * draw_buf;
    lv_obj_t * screen;

    lv_init();

    disp = lv_display_create(HOME_PAGE_WIDTH, HOME_PAGE_HEIGHT);
    assert(disp != NULL);
    lv_display_set_default(disp);
    lv_display_set_flush_cb(disp, dummy_flush_cb);

    draw_buf = lv_draw_buf_create(HOME_PAGE_WIDTH, HOME_PAGE_HEIGHT, LV_COLOR_FORMAT_NATIVE, 0);
    assert(draw_buf != NULL);
    lv_display_set_draw_buffers(disp, draw_buf, NULL);

    assert(home_page_create() == OPRT_OK);

    screen = lv_screen_active();
    assert(screen != NULL);

    assert(tree_has_image_src(screen, home_assets_get_path_weather_png()));
    assert(tree_has_image_src(screen, home_assets_get_path_clock_colon_png()));
    assert(tree_count_label_text(screen, "26") >= 2U);
    assert(tree_count_label_text(screen, "6/24") >= 1U);
    assert(tree_count_label_text(screen, "09") >= 1U);

    lv_display_delete(disp);
    lv_draw_buf_destroy(draw_buf);
    return 0;
}
