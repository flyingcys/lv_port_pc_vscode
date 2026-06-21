/**
 * @file lv_demo_music.h
 *
 */

#ifndef LV_DEMO_MUSIC_H
#define LV_DEMO_MUSIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

#ifndef LOCAL_LV_USE_DEMO_MUSIC
#define LOCAL_LV_USE_DEMO_MUSIC 1
#endif

#ifndef LV_DEMO_MUSIC_SQUARE
#define LV_DEMO_MUSIC_SQUARE 0
#endif

#ifndef LV_DEMO_MUSIC_LANDSCAPE
#define LV_DEMO_MUSIC_LANDSCAPE 0
#endif

#ifndef LV_DEMO_MUSIC_ROUND
#define LV_DEMO_MUSIC_ROUND 0
#endif

#ifndef LV_DEMO_MUSIC_LARGE
#define LV_DEMO_MUSIC_LARGE 0
#endif

#ifndef LV_DEMO_MUSIC_AUTO_PLAY
#define LV_DEMO_MUSIC_AUTO_PLAY 0
#endif

#if LOCAL_LV_USE_DEMO_MUSIC

/*********************
 *      DEFINES
 *********************/

#if LV_DEMO_MUSIC_LARGE
#  define LV_DEMO_MUSIC_HANDLE_SIZE  40
#else
#  define LV_DEMO_MUSIC_HANDLE_SIZE  20
#endif

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_obj_t * parent;
} lv_demo_args_t;

static inline void lv_demo_args_init(lv_demo_args_t * args)
{
    if(args) args->parent = lv_screen_active();
}

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_demo_music(void);
/**
 * Create the music demo with custom arguments.
 * @param args Pointer to demo arguments structure containing the parent widget and other options.
 */
void lv_demo_music_with_args(const lv_demo_args_t * args);
const char * lv_demo_music_get_title(uint32_t track_id);
const char * lv_demo_music_get_artist(uint32_t track_id);
const char * lv_demo_music_get_genre(uint32_t track_id);
uint32_t lv_demo_music_get_track_length(uint32_t track_id);

/**********************
 *      MACROS
 **********************/

#endif /*LOCAL_LV_USE_DEMO_MUSIC*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DEMO_MUSIC_H*/
