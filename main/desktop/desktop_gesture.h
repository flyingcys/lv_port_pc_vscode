#ifndef DESKTOP_GESTURE_H
#define DESKTOP_GESTURE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DESKTOP_GESTURE_AXIS_PX 20
#define DESKTOP_GESTURE_TAP_CANCEL_PX 8

bool desktop_gesture_is_vertical(int32_t dx, int32_t dy);
bool desktop_gesture_cancels_tap(int32_t dx, int32_t dy);

#ifdef __cplusplus
}
#endif

#endif
