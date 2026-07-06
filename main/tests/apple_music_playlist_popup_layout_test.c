#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "lvgl.h"
#include "../src/v9_apple_music/am_metrics.h"
#include "../src/v9_apple_music/am_shell.h"

static uint32_t g_draw_buf[800 * 480];

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
}

static int failf(const char *msg, int value)
{
    fprintf(stderr, "%s: %d\n", msg, value);
    return 1;
}

int main(void)
{
    lv_display_t *disp;
    lv_obj_t *root;
    lv_obj_t *player;
    am_miniplayer_handles_t handles;

    lv_init();
    disp = lv_display_create(800, 480);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, g_draw_buf, NULL, sizeof(g_draw_buf), LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_default(disp);

    am_metrics_init(800, 480);

    root = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(root);
    lv_obj_set_pos(root, 0, 0);
    lv_obj_set_size(root, 604, 480);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    player = lv_obj_create(root);
    lv_obj_remove_style_all(player);
    lv_obj_set_size(player, 604, am_metrics()->player_h);
    lv_obj_align(player, LV_ALIGN_BOTTOM_MID, 0, 0);

    handles = am_shell_build_miniplayer(player, NULL, NULL, NULL, NULL, NULL);
    lv_obj_update_layout(root);

    if(handles.playlist_popup == NULL) return failf("playlist popup missing", 0);
    if(handles.playlist_scroll_track == NULL) {
        return failf("playlist scroll track missing", 0);
    }
    if(handles.playlist_scroll_thumb == NULL) {
        return failf("playlist scroll thumb missing", 0);
    }
    if(lv_obj_get_width(handles.playlist_scroll_track) <= 0) {
        return failf("playlist scroll track width invalid",
                     lv_obj_get_width(handles.playlist_scroll_track));
    }
    if(lv_obj_get_height(handles.playlist_scroll_thumb) <= 0) {
        return failf("playlist scroll thumb height invalid",
                     lv_obj_get_height(handles.playlist_scroll_thumb));
    }
    if(!lv_obj_has_flag(handles.playlist_scroll_track, LV_OBJ_FLAG_HIDDEN)) {
        return failf("playlist scroll track should start hidden", 0);
    }
    if(lv_obj_get_style_pad_right(handles.playlist_list, LV_PART_MAIN) != 16) {
        return failf("playlist list pad right unexpected",
                     lv_obj_get_style_pad_right(handles.playlist_list, LV_PART_MAIN));
    }

    lv_display_delete(disp);
    return 0;
}
