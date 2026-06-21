#include "desktop_gesture.h"

static int32_t iabs32(int32_t v)
{
    return v < 0 ? -v : v;
}

bool desktop_gesture_is_vertical(int32_t dx, int32_t dy)
{
    int32_t ax = iabs32(dx);
    int32_t ay = iabs32(dy);

    return ay > ax && ay > DESKTOP_GESTURE_AXIS_PX;
}

bool desktop_gesture_cancels_tap(int32_t dx, int32_t dy)
{
    return iabs32(dx) > DESKTOP_GESTURE_TAP_CANCEL_PX ||
           iabs32(dy) > DESKTOP_GESTURE_TAP_CANCEL_PX;
}
