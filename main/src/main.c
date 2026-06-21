
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

/*********************
 *      DEFINES
 *********************/
/* ����ǰ��������֡�ò���/������֡��ɣ�Լ 400ms�� */
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
 
  desktop_run();
  maybe_take_snapshot();   /* �� AM_SHOT ��λ���ͼ���˳���������� */

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

/* �����ֱ��ʣ��ɻ������� AM_RES ѡ��Ĭ�� 800x480 */
static void resolve_resolution(int32_t * w, int32_t * h)
{
    const char * res = getenv("AM_RES");
    if(res && strcmp(res, "640x480") == 0)      { *w = 640; *h = 480; }
    else if(res && strcmp(res, "480x272") == 0) { *w = 480; *h = 272; }
    else                                        { *w = 800; *h = 480; }
}

static void maybe_take_snapshot(void)
{
    const char * out = getenv("AM_SHOT");
    if(out == NULL) return;

    /* ����֡�ò���/������֡��� */
    for(int i = 0; i < SNAPSHOT_WARMUP_FRAMES; i++) { lv_timer_handler(); usleep(2 * 1000); }

    lv_draw_buf_t * snap = lv_snapshot_take(lv_screen_active(), LV_COLOR_FORMAT_ARGB8888);
    if(snap == NULL) { fprintf(stderr, "snapshot failed\n"); exit(2); }   /* 2 = ����ʧ�� */

    int32_t w = snap->header.w, h = snap->header.h;
    size_t stride = snap->header.stride;
    FILE * f = fopen(out, "wb");
    if(f == NULL) {
        fprintf(stderr, "cannot open snapshot output: %s\n", out);
        lv_draw_buf_destroy(snap);
        exit(3);   /* 3 = ����ļ���ʧ�� */
    }
    fprintf(f, "P6\n%d %d\n255\n", (int)w, (int)h);
    for(int32_t y = 0; y < h; y++) {
        const uint8_t * row = snap->data + (size_t)y * stride;
        for(int32_t x = 0; x < w; x++) {
            const uint8_t * px = row + (size_t)x * 4;   /* B,G,R,A �ڴ��� */
            uint8_t rgb[3] = { px[2], px[1], px[0] };
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
    lv_draw_buf_destroy(snap);
    exit(0);   /* 0 = ��ͼ�ɹ� */
}
