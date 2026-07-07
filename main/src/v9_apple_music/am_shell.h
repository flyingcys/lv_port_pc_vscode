#ifndef AM_SHELL_H
#define AM_SHELL_H

#include "lvgl/lvgl.h"

#include "am_data.h"
#include "am_player.h"

typedef void (*am_nav_cb_t)(am_view_t view, void *user);
typedef void (*am_simple_event_cb_t)(lv_event_t *e);

typedef struct {
    lv_obj_t *root;
    lv_obj_t *sidebar;
    lv_obj_t *header;
    lv_obj_t *crumb;
    lv_obj_t *content;
    lv_obj_t *player;
} am_shell_handles_t;

void am_shell_build_sidebar(lv_obj_t *sidebar,
                            am_view_t active_view,
                            const am_local_item_t *locals,
                            size_t local_count,
                            am_nav_cb_t cb,
                            void *user);
void am_shell_set_header_title(lv_obj_t *header_label, const char *title);
am_miniplayer_handles_t am_shell_build_miniplayer(lv_obj_t *player,
                                                  am_simple_event_cb_t on_mode,
                                                  am_simple_event_cb_t on_prev,
                                                  am_simple_event_cb_t on_play,
                                                  am_simple_event_cb_t on_next,
                                                  am_simple_event_cb_t on_open_file,
                                                  am_simple_event_cb_t on_playlist);

#endif /* AM_SHELL_H */
