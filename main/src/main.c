
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include "lvgl.h"
#if defined(VENUS_CODEGEN_DEBUG)
#include "ui.h"
#else
#include "lvgl/examples/lv_examples.h"
#include "lvgl/demos/lv_demos.h"
#endif
#include "glob.h"

/*********************
 *      DEFINES
 *********************/
/* 分辨率：优先用 codegen 写入 lv_conf.h 的 LV_DISP_DEF_*；未定义时默认 320×480。 */
#ifndef LV_DISP_DEF_WIDTH
#define LV_DISP_DEF_WIDTH 320
#endif
#ifndef LV_DISP_DEF_HEIGHT
#define LV_DISP_DEF_HEIGHT 480
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_display_t * hal_init(int32_t w, int32_t h);
#ifdef __EMSCRIPTEN__
static void emscripten_loop(void * arg);
#endif
#if defined(VENUS_CODEGEN_DEBUG)
static void venus_dump_geom_after_layout(void);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

extern void freertos_main(void);

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int main(int argc, char **argv)
{
  (void)argc; /*Unused*/
  (void)argv; /*Unused*/

  /*Initialize LVGL*/
  lv_init();

  /*Initialize the HAL (display, input devices, tick) for LVGL*/
  hal_init(LV_DISP_DEF_WIDTH, LV_DISP_DEF_HEIGHT);

  #if defined(VENUS_CODEGEN_DEBUG)
  ui_create();
  #elif LV_USE_OS == LV_OS_NONE
  lv_demo_widgets();

  #elif LV_USE_OS == LV_OS_FREERTOS

  /* Run FreeRTOS and create lvgl task */
  freertos_main();

  #endif

  #if LV_USE_OS == LV_OS_NONE
  #ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(emscripten_loop, NULL, 0, true);
  #else
  while(1) {
    /* Periodically call the lv_task handler.
     * It could be done in a timer interrupt or an OS task too.*/
    lv_timer_handler();
    #if defined(VENUS_CODEGEN_DEBUG)
    venus_dump_geom_after_layout();
    #endif
    usleep(5 * 1000);
  }
  #endif

  #endif

  return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static lv_display_t * hal_init(int32_t w, int32_t h)
{

  lv_group_set_default(lv_group_create());

  lv_display_t * disp = lv_sdl_window_create(w, h);

  lv_indev_t * mouse = lv_sdl_mouse_create();
  lv_indev_set_group(mouse, lv_group_get_default());
  lv_indev_set_display(mouse, disp);
  lv_display_set_default(disp);

  #if !defined(VENUS_CODEGEN_DEBUG)
  LV_IMAGE_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
  lv_obj_t * cursor_obj;
  cursor_obj = lv_image_create(lv_screen_active()); /*Create an image object for the cursor */
  lv_image_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
  lv_indev_set_cursor(mouse, cursor_obj);             /*Connect the image  object to the driver*/
  #endif

  lv_indev_t * mousewheel = lv_sdl_mousewheel_create();
  lv_indev_set_display(mousewheel, disp);
  lv_indev_set_group(mousewheel, lv_group_get_default());

  lv_indev_t * kb = lv_sdl_keyboard_create();
  lv_indev_set_display(kb, disp);
  lv_indev_set_group(kb, lv_group_get_default());

  return disp;
}

#ifdef __EMSCRIPTEN__
static void emscripten_loop(void * arg)
{
  (void)arg;
  lv_timer_handler();
  #if defined(VENUS_CODEGEN_DEBUG)
  venus_dump_geom_after_layout();
  #endif
}
#endif

#if defined(VENUS_CODEGEN_DEBUG)
static void venus_dump_geom_after_layout(void)
{
  static int dumped;
  if(dumped) return;
  dumped = 1;
  venus_ui_dump_geom();
}
#endif
