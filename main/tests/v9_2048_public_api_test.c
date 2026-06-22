#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "lvgl.h"

#include "../src/v9_2048/game_2048.h"

static uint32_t g_draw_buf[800 * 480];

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
}

static lv_obj_t *find_label(lv_obj_t *root, const char *text)
{
    uint32_t count = lv_obj_get_child_count(root);
    if(lv_obj_check_type(root, &lv_label_class) && strcmp(lv_label_get_text(root), text) == 0) {
        return root;
    }
    for(uint32_t i = 0; i < count; i++) {
        lv_obj_t *found = find_label(lv_obj_get_child(root, i), text);
        if(found != NULL) return found;
    }
    return NULL;
}

static lv_obj_t *find_square_board(lv_obj_t *root)
{
    uint32_t count = lv_obj_get_child_count(root);
    for(uint32_t i = 0; i < count; i++) {
        lv_obj_t *child = lv_obj_get_child(root, i);
        int32_t w = lv_obj_get_width(child);
        int32_t h = lv_obj_get_height(child);
        if(w == h && w >= 240 && lv_obj_get_x(child) > 100 && lv_obj_get_y(child) > 100) return child;
    }
    return NULL;
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
    lv_obj_update_layout(root);

    lv_obj_t *title = find_label(root, "2048");
    lv_obj_t *size_button = find_label(root, "4x4");
    lv_obj_t *score = find_label(root, "SCORE");
    lv_obj_t *best = find_label(root, "BEST");
    lv_obj_t *board = find_square_board(root);
    assert(title != NULL);
    assert(size_button != NULL);
    assert(score != NULL);
    assert(best != NULL);
    assert(board != NULL);
    assert(lv_obj_get_y(lv_obj_get_parent(size_button)) <= lv_obj_get_y(title) + 30);
    assert(lv_obj_get_y(lv_obj_get_parent(score)) == lv_obj_get_y(lv_obj_get_parent(best)));
    assert(lv_obj_get_width(lv_obj_get_parent(score)) >= 98);
    assert(lv_obj_get_width(lv_obj_get_parent(best)) >= 98);
    lv_obj_t *score_value = lv_obj_get_child(lv_obj_get_parent(score), 0);
    lv_obj_t *best_value = lv_obj_get_child(lv_obj_get_parent(best), 0);
    assert(score_value != NULL);
    assert(best_value != NULL);
    lv_obj_update_layout(lv_obj_get_parent(score));
    lv_obj_update_layout(lv_obj_get_parent(best));
    assert(lv_obj_get_y(score) >= lv_obj_get_y(score_value) + lv_obj_get_height(score_value) + 4);
    assert(lv_obj_get_y(best) >= lv_obj_get_y(best_value) + lv_obj_get_height(best_value) + 4);
    assert(lv_obj_get_width(board) >= 330);
    assert(lv_obj_get_y(board) + lv_obj_get_height(board) <= 480);

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
