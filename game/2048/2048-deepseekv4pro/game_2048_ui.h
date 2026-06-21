// game_2048_ui.h - 2048 LVGL UI layer
#ifndef GAME_2048_UI_H
#define GAME_2048_UI_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the 2048 game UI on the active screen.
 * @param keyboard_indev  SDL keyboard input device (from hal_init)
 * @param mouse_indev     SDL mouse input device (from hal_init)
 */
void game_2048_ui_init(lv_indev_t *keyboard_indev, lv_indev_t *mouse_indev);

#ifdef __cplusplus
}
#endif

#endif /* GAME_2048_UI_H */
