#include <assert.h>
#include <stdint.h>

#include "lvgl.h"

LV_IMAGE_DECLARE(img_clock_alarm);
LV_IMAGE_DECLARE(img_clock_thermometer);
LV_IMAGE_DECLARE(img_clock_drop);
LV_IMAGE_DECLARE(img_clock_smile);

static uint32_t g_draw_buf[800 * 480];

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
}

static void check_image(const lv_image_dsc_t *dsc, const char *name)
{
    assert(dsc != NULL);
    assert(dsc->header.w > 0);

    lv_obj_t *img = lv_image_create(lv_screen_active());
    assert(img != NULL);
    lv_image_set_src(img, dsc);
    assert(lv_image_get_src_width(img) == (int32_t)dsc->header.w);
    assert(lv_image_get_src_width(img) > 0);

    lv_obj_delete(img);
    (void)name;
}

int main(void)
{
    lv_init();
    lv_display_t *disp = lv_display_create(800, 480);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, g_draw_buf, NULL, sizeof(g_draw_buf),
                              LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_default(disp);

    check_image(&img_clock_alarm, "alarm");
    check_image(&img_clock_thermometer, "thermometer");
    check_image(&img_clock_drop, "drop");
    check_image(&img_clock_smile, "smile");

    lv_display_delete(disp);
    return 0;
}
