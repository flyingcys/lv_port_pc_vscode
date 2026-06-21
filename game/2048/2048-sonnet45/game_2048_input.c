/**
 * @file game_2048_input.c
 * @brief 2048 Game Input Handler Implementation
 */

/*********************
 *      INCLUDES
 *********************/
#include "game_2048_input.h"
#include "game_2048.h"
#include <stdlib.h>
#include <stdio.h>

/*********************
 *      DEFINES
 *********************/
#define SWIPE_THRESHOLD 30  // Minimum pixels for swipe detection

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void key_event_cb(lv_event_t * e);
static void touch_event_cb(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_point_t press_point = {0, 0};
static bool input_locked = false;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void game_2048_input_init(lv_obj_t * container)
{
    if (container == NULL) {
        return;
    }
    
    // Enable keyboard input
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);  // Disable scrolling
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_add_event_cb(container, key_event_cb, LV_EVENT_KEY, NULL);
    
    // Enable touch/mouse gestures - use PRESSING to capture drag movements
    lv_obj_add_event_cb(container, touch_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(container, touch_event_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(container, touch_event_cb, LV_EVENT_RELEASED, NULL);
    
    // Add container to default input group for keyboard focus
    lv_group_t * g = lv_group_get_default();
    if (g) {
        lv_group_add_obj(g, container);
        lv_group_focus_obj(container);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Keyboard input event handler
 */
static void key_event_cb(lv_event_t * e)
{
    if (input_locked) {
        return;
    }
    
    uint32_t key = lv_event_get_key(e);
    direction_t dir;
    bool valid_key = true;
    
    switch (key) {
        case LV_KEY_UP:
            dir = DIR_UP;
            break;
        case LV_KEY_DOWN:
            dir = DIR_DOWN;
            break;
        case LV_KEY_LEFT:
            dir = DIR_LEFT;
            break;
        case LV_KEY_RIGHT:
            dir = DIR_RIGHT;
            break;
        default:
            valid_key = false;
            break;
    }
    
    if (valid_key) {
        // Lock input temporarily to prevent rapid repeated moves
        input_locked = true;
        
        // Execute move
        bool moved = game_2048_move(dir);
        
        // Unlock input after a short delay (simulating animation time)
        if (moved) {
            // In a real implementation with animations, unlock after animation completes
            // For now, unlock immediately
            input_locked = false;
        } else {
            input_locked = false;
        }
    }
}

/**
 * Touch/Mouse gesture event handler
 */
static void touch_event_cb(lv_event_t * e)
{
    if (input_locked) {
        return;
    }
    
    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t * indev = lv_event_get_indev(e);
    
    if (code == LV_EVENT_PRESSED) {
        // Record press position
        lv_indev_get_point(indev, &press_point);
        printf("[2048] Mouse pressed at (%d, %d)\n", press_point.x, press_point.y);
    } else if (code == LV_EVENT_PRESSING) {
        // Update press point during drag (for continuous tracking)
        // This helps with more accurate gesture detection
        (void)indev; // Not needed for PRESSING, just tracking
    } else if (code == LV_EVENT_RELEASED) {
        // Get release position
        lv_point_t release_point;
        lv_indev_get_point(indev, &release_point);
        
        printf("[2048] Mouse released at (%d, %d)\n", release_point.x, release_point.y);
        
        // Calculate delta
        int32_t dx = release_point.x - press_point.x;
        int32_t dy = release_point.y - press_point.y;
        
        printf("[2048] Delta: dx=%d, dy=%d\n", dx, dy);
        
        // Determine if swipe is significant enough
        int32_t abs_dx = (dx >= 0) ? dx : -dx;
        int32_t abs_dy = (dy >= 0) ? dy : -dy;
        
        if (abs_dx < SWIPE_THRESHOLD && abs_dy < SWIPE_THRESHOLD) {
            // Not a swipe, ignore
            printf("[2048] Not a swipe (below threshold)\n");
            return;
        }
        
        // Determine dominant direction
        direction_t dir;
        const char* dir_name;
        if (abs_dx > abs_dy) {
            // Horizontal swipe
            dir = (dx > 0) ? DIR_RIGHT : DIR_LEFT;
            dir_name = (dx > 0) ? "RIGHT" : "LEFT";
        } else {
            // Vertical swipe
            dir = (dy > 0) ? DIR_DOWN : DIR_UP;
            dir_name = (dy > 0) ? "DOWN" : "UP";
        }
        
        printf("[2048] Swipe direction: %s\n", dir_name);
        
        // Lock input temporarily
        input_locked = true;
        
        // Execute move
        bool moved = game_2048_move(dir);
        
        printf("[2048] Move result: %s\n", moved ? "SUCCESS" : "FAILED");
        
        // Unlock input
        if (moved) {
            // In a real implementation with animations, unlock after animation completes
            input_locked = false;
        } else {
            input_locked = false;
        }
    }
}
