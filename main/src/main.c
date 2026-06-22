
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
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "lvgl/lvgl.h"
#include "lvgl/src/draw/snapshot/lv_snapshot.h"
#include "desktop.h"
#include "calc/calc.h"
#include "calc/calc_app.h"
#include "fruit_ninja_app.h"
#include "game_2048_app.h"
#include "music_player.h"


/*********************
 *      DEFINES
 *********************/
/* 截图前预热若干帧，等待布局/动画完成，约 400ms */
#define SNAPSHOT_WARMUP_FRAMES 200

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_display_t * hal_init(int32_t w, int32_t h);
static void resolve_resolution(int32_t * w, int32_t * h);
static void maybe_take_snapshot(void);
static void maybe_run_calc_standalone(int32_t w, int32_t h);

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
  int32_t scr_w, scr_h;
  resolve_resolution(&scr_w, &scr_h);
  desktop_set_resolution(scr_w, scr_h);
  hal_init(scr_w, scr_h);

  #if LV_USE_OS == LV_OS_NONE

  maybe_run_calc_standalone(scr_w, scr_h);

  desktop_run();

  /* AM_APP=calc/2048/fruit_ninja/netease_music/local_music：直接打开 app 用于无头截图比对 */
  {
    const char * app = getenv("AM_APP");
    if(app && strcmp(app, "calc") == 0) {
      fprintf(stderr, "DBG: launching calc\n"); fflush(stderr);
      calc_app_launch();
      fprintf(stderr, "DBG: calc launched\n"); fflush(stderr);
    } else if(app && strcmp(app, "2048") == 0) {
      fprintf(stderr, "DBG: launching 2048\n"); fflush(stderr);
      game_2048_app_launch();
      fprintf(stderr, "DBG: 2048 launched\n"); fflush(stderr);
    } else if(app && strcmp(app, "fruit_ninja") == 0) {
      fprintf(stderr, "DBG: launching fruit_ninja\n"); fflush(stderr);
      fruit_ninja_app_launch();
      fprintf(stderr, "DBG: fruit_ninja launched\n"); fflush(stderr);
    } else if(app && (strcmp(app, "netease_music") == 0 || strcmp(app, "local_music") == 0)) {
      fprintf(stderr, "DBG: launching local music\n"); fflush(stderr);
      local_music_demo_launch();
      fprintf(stderr, "DBG: local music launched\n"); fflush(stderr);
    }
  }

  maybe_take_snapshot();   /* 设置 AM_SHOT 时截图并退出，供自动化比对使用 */

  while(1) {
    /* Periodically call the lv_task handler.
     * It could be done in a timer interrupt or an OS task too.*/
    lv_timer_handler();
    usleep(5 * 1000);
  }

  #elif LV_USE_OS == LV_OS_FREERTOS

  /* Run FreeRTOS and create lvgl task */
  freertos_main();  

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

  LV_IMAGE_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
  lv_obj_t * cursor_obj;
  cursor_obj = lv_image_create(lv_screen_active()); /*Create an image object for the cursor */
  lv_image_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
  lv_indev_set_cursor(mouse, cursor_obj);             /*Connect the image  object to the driver*/

  lv_indev_t * mousewheel = lv_sdl_mousewheel_create();
  lv_indev_set_display(mousewheel, disp);
  lv_indev_set_group(mousewheel, lv_group_get_default());

  lv_indev_t * kb = lv_sdl_keyboard_create();
  lv_indev_set_display(kb, disp);
  lv_indev_set_group(kb, lv_group_get_default());

  return disp;
}

/* 解析分辨率，可通过环境变量 AM_RES 选择，默认 800x480 */
static void resolve_resolution(int32_t * w, int32_t * h)
{
    const char * res = getenv("AM_RES");
    if(res && strcmp(res, "640x480") == 0)      { *w = 640; *h = 480; }
    else if(res && strcmp(res, "480x272") == 0) { *w = 480; *h = 272; }
    else                                        { *w = 800; *h = 480; }
}

static void maybe_run_calc_standalone(int32_t w, int32_t h)
{
    const char * app = getenv("AM_APP");
    if(app == NULL || strcmp(app, "calc_standalone") != 0) return;

    setenv("AM_CALC_HTML_PARITY", "1", 1);

    lv_obj_t * screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x667eea), 0);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x764ba2), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);

    calc_create(screen, w, h);
    maybe_take_snapshot();

    while(1) {
        lv_timer_handler();
        usleep(5 * 1000);
    }
}

static void maybe_take_snapshot(void)
{
    const char * out = getenv("AM_SHOT");
    const char * frames_env = getenv("AM_SHOT_FRAMES");
    int frames = SNAPSHOT_WARMUP_FRAMES;
    if(out == NULL) return;
    if(frames_env != NULL) {
        int parsed = atoi(frames_env);
        if(parsed > 0 && parsed < 10000) frames = parsed;
    }

    fprintf(stderr, "DBG: snapshot warmup start\n"); fflush(stderr);

    /* 等待若干帧，让布局/动画稳定 */
    for(int i = 0; i < frames; i++) { lv_timer_handler(); usleep(2 * 1000); }

    lv_draw_buf_t * snap = lv_snapshot_take(lv_screen_active(), LV_COLOR_FORMAT_ARGB8888);
    if(snap == NULL) { fprintf(stderr, "snapshot failed\n"); exit(2); }   /* 2 = 截图失败 */

    int32_t w = snap->header.w, h = snap->header.h;
    size_t stride = snap->header.stride;
    FILE * f = fopen(out, "wb");
    if(f == NULL) {
        fprintf(stderr, "cannot open snapshot output: %s\n", out);
        lv_draw_buf_destroy(snap);
        exit(3);   /* 3 = 打开输出文件失败 */
    }
    fprintf(f, "P6\n%d %d\n255\n", (int)w, (int)h);
    for(int32_t y = 0; y < h; y++) {
        const uint8_t * row = snap->data + (size_t)y * stride;
        for(int32_t x = 0; x < w; x++) {
            const uint8_t * px = row + (size_t)x * 4;   /* B,G,R,A 内存序 */
            uint8_t rgb[3] = { px[2], px[1], px[0] };
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
    lv_draw_buf_destroy(snap);
    exit(0);   /* 0 = 截图成功 */
}
