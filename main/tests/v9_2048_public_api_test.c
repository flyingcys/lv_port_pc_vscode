#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "lvgl.h"

#include "../src/v9_2048/game_2048.h"

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

    lv_obj_t *root = game_2048_create(parent, 800, 480);
    assert(root != NULL);
    assert(lv_obj_get_parent(root) == parent);

    lv_group_t *group = lv_group_get_default();
    if(group == NULL) {
        group = lv_group_create();
        lv_group_set_default(group);
    }
    lv_obj_t *dummy = lv_button_create(parent);
    lv_group_add_obj(group, dummy);
    lv_group_focus_obj(dummy);
    game_2048_focus();
    assert(lv_group_get_focused(group) != NULL);
    assert(lv_group_get_focused(group) != dummy);

    lv_group_focus_obj(dummy);
    game_2048_set_grid_size(5);
    assert(game_2048_get_grid_size() == 5);
    assert(lv_group_get_focused(group) != NULL);
    assert(lv_group_get_focused(group) != dummy);
    game_2048_set_grid_size(6);
    assert(game_2048_get_grid_size() == 6);
    game_2048_set_grid_size(7);
    assert(game_2048_get_grid_size() == 6);
    game_2048_set_grid_size(4);
    assert(game_2048_get_grid_size() == 4);

    game_2048_start();
    for(int i = 0; i < 5; i++) {
        lv_timer_handler();
        lv_tick_inc(50);
    }
    game_2048_stop();

    lv_display_delete(disp);
    return 0;
}
