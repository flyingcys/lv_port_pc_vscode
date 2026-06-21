#include "desktop_panels_geom.h"

int32_t desktop_panel_drag_y(int which, int32_t reveal, int32_t panel_h, int32_t screen_h)
{
    if(reveal < 0)        reveal = 0;
    if(reveal > panel_h)  reveal = panel_h;
    if(which == DESKTOP_PANEL_CONTROL) {
        return -panel_h + reveal;       /* [-panel_h, 0] */
    }
    return screen_h - reveal;           /* [screen_h - panel_h, screen_h]，notify */
}

bool desktop_panel_snap_open(int32_t reveal, int32_t panel_h, int32_t last_delta)
{
    if(last_delta >= DESKTOP_PANEL_FLICK_PX)  return true;
    if(last_delta <= -DESKTOP_PANEL_FLICK_PX) return false;
    if(panel_h <= 0)                      return false;
    return reveal * 2 >= panel_h;        /* reveal/panel_h >= 0.5 */
}
