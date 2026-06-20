/**
 * @file tetris_input.c
 * @brief Tetris input handling implementation - Keyboard controls
 */

/*********************
 *      INCLUDES
 *********************/
#include "tetris_input.h"

/*********************
 *      DEFINES
 *********************/
#define KEY_REPEAT_DELAY    150  /* ms before key repeat starts */
#define KEY_REPEAT_RATE     50   /* ms between key repeats */

/**********************
 *  STATIC VARIABLES
 **********************/
static tetris_game_t *s_game = NULL;
static tetris_ui_t *s_ui = NULL;
static uint32_t s_last_key_time[256] = {0};
static bool s_key_pressed[256] = {false};

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void keyboard_event_cb(lv_event_t *e);
static bool should_process_key(uint32_t key, uint32_t current_time);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void tetris_input_init(tetris_game_t *game, tetris_ui_t *ui)
{
    s_game = game;
    s_ui = ui;
    
    /* Get keyboard input device and add event handler */
    lv_indev_t *kb = lv_indev_get_next(NULL);
    while (kb) {
        if (lv_indev_get_type(kb) == LV_INDEV_TYPE_KEYPAD) {
            lv_obj_add_event_cb(lv_screen_active(), keyboard_event_cb, LV_EVENT_KEY, NULL);
            break;
        }
        kb = lv_indev_get_next(kb);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void keyboard_event_cb(lv_event_t *e)
{
    if (!s_game || !s_ui) {
        return;
    }
    
    uint32_t key = lv_event_get_key(e);
    uint32_t current_time = lv_tick_get();
    
    /* Handle key press */
    if (lv_event_get_code(e) == LV_EVENT_KEY) {
        bool key_handled = false;
        
        /* Check if we should process this key (for debouncing and repeat) */
        if (should_process_key(key, current_time)) {
            switch (key) {
                case LV_KEY_LEFT:
                    if (tetris_game_move_left(s_game)) {
                        key_handled = true;
                    }
                    break;
                    
                case LV_KEY_RIGHT:
                    if (tetris_game_move_right(s_game)) {
                        key_handled = true;
                    }
                    break;
                    
                case LV_KEY_UP:
                    if (tetris_game_rotate(s_game)) {
                        key_handled = true;
                    }
                    s_key_pressed[key] = false; /* No repeat for rotation */
                    break;
                    
                case LV_KEY_DOWN:
                    tetris_game_set_soft_drop(s_game, true);
                    key_handled = true;
                    break;
                    
                case ' ':  /* Space key for pause */
                    tetris_game_toggle_pause(s_game);
                    key_handled = true;
                    s_key_pressed[key] = false; /* No repeat for pause */
                    break;
                    
                default:
                    break;
            }
            
            if (key_handled) {
                tetris_ui_update(s_ui);
            }
        }
    }
    /* Handle key release */
    else if (lv_event_get_code(e) == LV_EVENT_RELEASED) {
        if (key == LV_KEY_DOWN) {
            tetris_game_set_soft_drop(s_game, false);
        }
        s_key_pressed[key] = false;
    }
}

static bool should_process_key(uint32_t key, uint32_t current_time)
{
    if (key >= 256) {
        return false;
    }
    
    /* First press */
    if (!s_key_pressed[key]) {
        s_key_pressed[key] = true;
        s_last_key_time[key] = current_time;
        return true;
    }
    
    /* Key repeat */
    uint32_t time_since_last = current_time - s_last_key_time[key];
    
    /* Initial delay before repeat starts */
    if (time_since_last < KEY_REPEAT_DELAY) {
        return false;
    }
    
    /* Check repeat rate */
    if (time_since_last >= KEY_REPEAT_DELAY + KEY_REPEAT_RATE) {
        s_last_key_time[key] = current_time;
        return true;
    }
    
    return false;
}
