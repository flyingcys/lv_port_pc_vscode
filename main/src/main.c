
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
#include "lvgl/lvgl.h"
#include "lvgl/examples/lv_examples.h"
#include "lvgl/demos/lv_demos.h"
#include "glob.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_display_t * hal_init(int32_t w, int32_t h);

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
// 返回主界面的事件回调
static void back_event_handler(lv_event_t * e) {
  // lv_obj_t * main_scr = (lv_obj_t *)e->user_data;
  // lv_scr_load(main_scr);
}

// 图标点击事件回调
static void icon_event_handler(lv_event_t * e) {
  lv_obj_t * sub_scr = lv_obj_create(NULL); // 创建子屏幕
  create_sub_screen(sub_scr);
  lv_scr_load(sub_scr); // 切换到子屏幕
}

// 创建子屏幕的函数
void create_sub_screen(lv_obj_t * parent_screen) {
  lv_obj_t * sub_scr = lv_obj_create(parent_screen);
  lv_obj_t * label = lv_label_create(sub_scr);
  lv_label_set_text(label, "这是子界面");
  lv_obj_center(label);
  
  // 添加返回按钮（可选）
  lv_obj_t * btn_back = lv_btn_create(sub_scr);
  lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_t * lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, "返回");
  lv_obj_add_event_cb(btn_back, back_event_handler, LV_EVENT_CLICKED, parent_screen);
}


void icon_test(void)
{
      // 创建主屏幕
      lv_obj_t * main_scr = lv_scr_act();
    
      // 创建图标按钮
      lv_obj_t * icon = lv_imgbtn_create(main_scr);
      // 设置图标图片（需先载入图像）
      lv_img_set_src(icon, "/home/share/samba/lv_port_pc_vscode/icon.png"); // 或使用 LV_SYMBOL_* 符号
      lv_obj_align(icon, LV_ALIGN_CENTER, 0, 0);
      
      // 添加点击事件
      lv_obj_add_event_cb(icon, icon_event_handler, LV_EVENT_CLICKED, NULL);
}
int main(int argc, char **argv)
{
  (void)argc; /*Unused*/
  (void)argv; /*Unused*/

  /*Initialize LVGL*/
  lv_init();

  /*Initialize the HAL (display, input devices, tick) for LVGL*/
  hal_init(320, 480);

  #if LV_USE_OS == LV_OS_NONE
 
  // create_gui();
  icon_test();

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