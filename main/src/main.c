
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
static void gesture_event_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);

  if(code == LV_EVENT_GESTURE) {
      lv_indev_t * indev = lv_indev_active();
      if(indev) {
          switch(lv_indev_get_gesture_dir(indev)) {
              case LV_DIR_LEFT:  printf("左滑\n"); break;
              case LV_DIR_RIGHT: printf("右滑\n"); break;
              case LV_DIR_TOP:   printf("上滑\n"); break;
              case LV_DIR_BOTTOM:printf("下滑\n"); break;
              default: break;
          }
      }
  }
}

#if 0
// 1. 创建基础动画结构
lv_anim_t merge_anim;
lv_anim_init(&merge_anim);
lv_anim_set_var(&merge_anim, target_label); // 目标数字标签
lv_anim_set_time(&merge_anim, 300); // 动画时长300ms

// 2. 设置缩放动画（合并效果）
lv_anim_set_values(&merge_anim, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 1.5); 
lv_anim_set_exec_cb(&merge_anim, (lv_anim_exec_xcb_t)lv_obj_set_zoom);

// 3. 设置透明度变化（可选）
lv_anim_t fade_anim;
lv_anim_init(&fade_anim);
lv_anim_set_var(&fade_anim, source_label); // 被合并的数字标签
lv_anim_set_values(&fade_anim, LV_OPA_100, LV_OPA_0);
lv_anim_set_exec_cb(&fade_anim, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);

// 4. 使用动画时间线同步
lv_anim_timeline_t * timeline = lv_anim_timeline_create();
lv_anim_timeline_add(timeline, 0, &merge_anim); 
lv_anim_timeline_add(timeline, 0, &fade_anim);
#endif

void lv_text_demo(void)
{
  lv_obj_t * label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, "Hello animations!");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
  lv_obj_set_pos(label, 100, 10);

}
void lv_font_test(void)
{
    lv_obj_t* label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label, & aka_font, LV_STATE_DEFAULT);
    lv_label_set_text(label,"风扇监控!!!!");

    lv_obj_t * screen = lv_screen_active();
    lv_obj_add_event_cb(screen, gesture_event_cb, LV_EVENT_ALL, NULL);
}

// 全局变量存储按键显示标签
static lv_obj_t * key_display_label;
static void keyboard_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    printf("keyboard event: %d\n", code);
    if(code == LV_EVENT_KEY) {
        uint32_t key = *((uint32_t *)lv_event_get_param(e));
        char key_str[32] = {0};
        
        switch(key) {
            case 'w':
            case 'W':
                snprintf(key_str, sizeof(key_str), "W");
                break;
            case 'a':
            case 'A':
                snprintf(key_str, sizeof(key_str), "A");
                break;
            case 's':
            case 'S':
                snprintf(key_str, sizeof(key_str), "S");
                break;
            case 'd':
            case 'D':
                snprintf(key_str, sizeof(key_str), "D");
                break;
            case 'j':
            case 'J':
                snprintf(key_str, sizeof(key_str), "J");
                break;
            case 'k':
            case 'K':
                snprintf(key_str, sizeof(key_str), "K");
                break;
            default:
                snprintf(key_str, sizeof(key_str), "按键: %c (0x%X)", (char)key, key);
                break;
        }
        
        // 更新显示
        lv_label_set_text(key_display_label, key_str);
    }
}

void create_key_display_ui(void) {
    // 创建显示按键的标签
    key_display_label = lv_label_create(lv_scr_act());
    lv_label_set_text(key_display_label, "test");
    lv_obj_set_style_text_font(key_display_label, &lv_font_montserrat_24, 0);
    lv_obj_align(key_display_label, LV_ALIGN_CENTER, 0, 0);
    
    // 添加键盘事件回调
    lv_obj_add_event_cb(lv_scr_act(), keyboard_event_cb, LV_EVENT_KEY, NULL);
}


int main(int argc, char **argv)
{
  (void)argc; /*Unused*/
  (void)argv; /*Unused*/

  /*Initialize LVGL*/
  lv_init();

  /*Initialize the HAL (display, input devices, tick) for LVGL*/
  hal_init(640, 480);

  #if LV_USE_OS == LV_OS_NONE
 
//   lv_demo_widgets();
  // lv_font_test();
  // lv_text_demo();
  lv_100ask_nes_simple_test();
  // lv_app_clock();
  
  // game_2048_create(lv_screen_active());
  
  // game_2048(lv_screen_active());
  // create_clock(lv_screen_active());

//   create_key_display_ui();

  // lv_example_grid_4();
  // lv_demo_benchmark();

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