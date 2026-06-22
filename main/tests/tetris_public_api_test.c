#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "lvgl.h"

#include "../src/v9-tetris/tetris.h"

static uint32_t g_draw_buf[800 * 480];

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
}

int main(void)
{
    lv_init();
    lv_display_t *disp = lv_display_create(800, 480);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, g_draw_buf, NULL, sizeof(g_draw_buf), LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_default(disp);

    lv_obj_t *parent = lv_obj_create(lv_screen_active());
    lv_obj_set_size(parent, 800, 480);

    lv_obj_t *root = tetris_create(parent, 800, 480);
    assert(root != NULL);
    assert(lv_obj_get_parent(root) == parent);

    tetris_start();
    for(int i = 0; i < 5; i++) {
        lv_timer_handler();
        lv_tick_inc(50);
    }
    tetris_stop();

    lv_display_delete(disp);
    return 0;
}
