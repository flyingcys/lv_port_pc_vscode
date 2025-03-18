
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

#if 0
// 声明两个屏幕对象
lv_obj_t *scr_home;
lv_obj_t *scr_settings;

// 切换到设置界面的回调函数
void btn_click_cb(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    lv_scr_load(scr_settings);
}

void icon_test(void)
{
      // 创建主界面
      scr_home = lv_scr_act();

      // 创建设置界面
      scr_settings = lv_obj_create(NULL);
  
      // 在主界面上添加一个图标按钮
      lv_obj_t *icon_btn = lv_img_create(scr_home);
      lv_img_set_src(icon_btn, LV_SYMBOL_SETTINGS); // 使用LVGL内置的设置图标
      lv_obj_align(icon_btn, LV_ALIGN_CENTER, 0, 0); // 将按钮居中对齐
  
      // 设置按钮的点击事件回调函数
      lv_obj_add_event_cb(icon_btn, btn_click_cb, LV_EVENT_CLICKED, NULL);
  
}
#endif
// 创建两个屏幕
lv_obj_t * screen1;
lv_obj_t * screen2;

// 图片对象
lv_obj_t * img;

// 加载自定义图片
LV_IMG_DECLARE(IMG_icon); // 假设你已经定义了这个图片

// 切换屏幕的回调函数
void switch_to_screen2(lv_event_t * e) {
    // lv_scr_load(screen2);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}
void icon_test(void)
{
    // 创建屏幕1
    screen1 = lv_scr_act();
    lv_obj_clean(screen1);

    // 创建屏幕2
    screen2 = lv_obj_create(NULL);
    lv_obj_set_size(screen2, LV_PCT(100), LV_PCT(100));

    // 在屏幕1上添加图片
    img = lv_img_create(screen1);
    lv_img_set_src(img, &IMG_icon);
    lv_obj_center(img);

    // 为图片添加点击事件
    lv_obj_add_event_cb(img, switch_to_screen2, LV_EVENT_ALL, NULL);
}

#if 0
void icon_test(void)
{
    // LV_IMAGE_DECLARE(IMG_icon);
    //   // 创建主屏幕
    //   lv_obj_t * main_scr = lv_scr_act();
    
    //   // 创建图标按钮
    //   lv_obj_t * icon = lv_imgbtn_create(main_scr);
    //   // 设置图标图片（需先载入图像）
    //   // lv_img_set_src(icon, "/home/share/samba/lv_port_pc_vscode/icon.png"); // 或使用 LV_SYMBOL_* 符号
    //   lv_image_set_src(icon, &IMG_icon);
    //   lv_obj_align(icon, LV_ALIGN_CENTER, 0, 0);
      
    //   // 添加点击事件
    //   lv_obj_add_event_cb(icon, icon_event_handler, LV_EVENT_CLICKED, NULL);

  // LV_IMAGE_DECLARE(IMG_icon);
  // // 创建主屏幕
  // lv_obj_t * icon = lv_image_create(lv_screen_active());
  // // 设置图标图片（需先载入图像）
  // // lv_img_set_src(icon, "/home/share/samba/lv_port_pc_vscode/icon.png"); // 或使用 LV_SYMBOL_* 符号
  // lv_image_set_src(icon, &IMG_icon);
  // lv_obj_align(icon, LV_ALIGN_CENTER, 0, 0);

  // 添加点击事件
  // lv_obj_add_event_cb(icon, icon_event_handler, LV_EVENT_CLICKED, NULL);

#if 0
  static lv_style_t obj_layout_sytle;
  lv_style_init(&obj_layout_sytle);
  lv_style_set_pad_all(&obj_layout_sytle, 0);
  lv_style_set_bg_color(&obj_layout_sytle, lv_color_hex(0x000000));
  lv_style_set_bg_opa(&obj_layout_sytle, 50);
#endif

  #if 0
  static lv_style_t obj_layout_style;   // 容器的样式

  lv_style_init(&obj_layout_style);

  lv_style_set_pad_all(&obj_layout_style, 0);

  lv_style_set_bg_opa(&obj_layout_style, 50);

  lv_style_set_text_font(&obj_layout_style, &lv_font_montserrat_14);

  lv_style_set_border_opa(&obj_layout_style, 0);

  lv_style_set_radius(&obj_layout_style, 0);

  lv_style_set_text_color(&obj_layout_style, lv_color_hex(0xB10417));

  lv_obj_t * panel = lv_obj_create(lv_screen_active());

  lv_obj_set_size(panel,  240, 30);

  lv_obj_add_style(panel, &obj_layout_style, 0);

  lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, 0);
#endif

static lv_style_t obj_layout_style;   // 容器的样式

    lv_style_init(&obj_layout_style);

    lv_style_set_pad_all(&obj_layout_style, 0);

    lv_style_set_bg_opa(&obj_layout_style, 50);

    lv_style_set_text_font(&obj_layout_style, &lv_font_montserrat_14);

    lv_style_set_border_opa(&obj_layout_style, 0);

    lv_style_set_radius(&obj_layout_style, 0);

    lv_style_set_text_color(&obj_layout_style, lv_color_hex(0xB10417));

    lv_obj_t * panel = lv_obj_create(lv_screen_active());

    lv_obj_set_size(panel,  240, 30);

    lv_obj_add_style(panel, &obj_layout_style, 0);

    lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, 0);

        /* 右上角小图标 */

    lv_obj_t * panel_icon = lv_obj_create(panel);

    lv_obj_set_size(panel_icon,  240, 25);

    lv_obj_set_layout(panel_icon, LV_LAYOUT_FLEX);

    lv_obj_set_style_base_dir(panel_icon, LV_BASE_DIR_RTL, 0);

    lv_obj_set_flex_flow(panel_icon, LV_FLEX_FLOW_ROW);

    lv_obj_align(panel_icon, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_add_style(panel_icon, &obj_layout_style, 0);

    lv_obj_t * label = lv_label_create(panel_icon);

    lv_label_set_text(label,  "00");

    lv_obj_t * label_bat = lv_label_create(panel_icon);

    lv_label_set_text(label_bat,  LV_SYMBOL_BATTERY_EMPTY);

    lv_obj_t * label_batchar = lv_label_create(label_bat);

    lv_obj_set_style_text_font(label_batchar, &lv_font_montserrat_14, 0);

    lv_label_set_text(label_batchar,  LV_SYMBOL_CHARGE);

    lv_obj_center(label_batchar);

    lv_obj_t * label_wifi = lv_label_create(panel_icon);

    lv_label_set_text(label_wifi, LV_SYMBOL_WIFI);

    lv_obj_t * label_time = lv_label_create(panel);

    lv_label_set_text(label_time, "12-34");

    lv_obj_align(label_time, LV_ALIGN_LEFT_MID, 0, 0);

    // lv_timer_t * timer = lv_timer_create(lv_timer_update_time, 1000,  label_time);



static lv_style_t cont_style, style_tabview_desktop;
lv_style_init(&cont_style);                           

lv_style_set_bg_opa(&cont_style, 0);                

//lv_style_set_border_width(&cont_style,15);            

lv_style_set_border_opa(&cont_style, 60);                

lv_style_set_pad_column(&cont_style, 15);   //列 间距

lv_style_set_pad_row(&cont_style, 25);      //行 间距

lv_style_set_pad_all(&cont_style, 0);       //全部区域用于显示

lv_style_set_layout(&cont_style, LV_LAYOUT_FLEX); //布局  弹性布局 Flex

lv_style_set_base_dir(&cont_style, LV_BASE_DIR_LTR);//方向 right to left方向 从左向右

lv_style_set_flex_flow(&cont_style, LV_FLEX_FLOW_ROW_WRAP);//设置

lv_style_init(&style_tabview_desktop);                          

lv_style_set_bg_opa(&style_tabview_desktop, 0);     

/*选项卡视图*/

lv_obj_t * tabview_desktop = lv_tabview_create(lv_scr_act()); //创建选项卡 视图

lv_obj_add_style(tabview_desktop, &style_tabview_desktop, 0);     //添加 视图样式

lv_tabview_set_act(tabview_desktop,1,LV_ANIM_OFF);

/*选项卡视图 添加一个选项卡容器*/

lv_obj_t * tab_main  = lv_tabview_add_tab(tabview_desktop, "hello");       

lv_obj_set_scrollbar_mode(tab_main,LV_SCROLLBAR_MODE_OFF);             

lv_obj_clear_flag(tab_main, LV_OBJ_FLAG_SCROLLABLE  );              //禁用点击

lv_obj_t * icon_cont_main = lv_obj_create(tab_main);   

lv_obj_set_size(icon_cont_main, 400, 200);                        //设置显示区域大小

lv_obj_add_style(icon_cont_main, &cont_style, 0);                   

lv_obj_center(icon_cont_main); 


// LV_IMG_DECLARE(PenWu_APP_icon);              

// LV_IMG_DECLARE(FYF5_icon);                 

// LV_IMG_DECLARE(DBUG_Assist_icon);               

// LV_IMG_DECLARE(Radio_Beacon_icon);           

// LV_IMG_DECLARE(Wired_Network_icon);            

// LV_IMG_DECLARE(sys_set_icon);                   
LV_IMG_DECLARE(IMG_icon);

// static const lv_img_dsc_t *APP_icon[] = {

//                                             &amp;PenWu_APP_icon ,

//                                             &amp;FYF5_icon,

//                                             &amp;DBUG_Assist_icon,

//                                             &amp;sys_set_icon,

//                                             &amp;Wired_Network_icon,

//                                             &amp;Radio_Beacon_icon,

//                                         };

  for (uint8_t i = 0; i < 4; i ++) {
    lv_obj_t * img_icon = lv_img_create(icon_cont_main);                   //在icon_cont_main 上 填充内容

          lv_img_set_src(img_icon, &IMG_icon);
  }

  lv_obj_t * label_icon_name = lv_label_create(tab_main);                
  lv_obj_set_style_text_font(label_icon_name, &lv_font_montserrat_26, 0);

   lv_obj_set_style_text_color(label_icon_name, lv_color_hex(0xffffff), 0);

   lv_obj_set_style_text_align(label_icon_name, LV_TEXT_ALIGN_CENTER, 0);

  lv_label_set_text(label_icon_name, "1234");
}
#endif
void symbol_test(void)
{
  // lv_obj_t * main_scr = lv_scr_act();

  // lv_obj_t * label = lv_label_create(lv_screen_active());
  // // lv_label_set_text(label, LV_SYMBOL_HOME);
  // lv_image_set_src(label, LV_SYMBOL_HOME);

  lv_obj_t *label_1 = lv_label_create(lv_screen_active());
  lv_image_set_src(label_1, LV_SYMBOL_WIFI);
}


int main(int argc, char **argv)
{
  (void)argc; /*Unused*/
  (void)argv; /*Unused*/

  /*Initialize LVGL*/
  lv_init();

  /*Initialize the HAL (display, input devices, tick) for LVGL*/
  hal_init(800, 480);

  #if LV_USE_OS == LV_OS_NONE
 
  // create_gui();
  icon_test();
  // lv_example_win_1();
  // lv_example_button_1();

  // lv_demo_smartwatch();
  // lv_example_image_1();
  // lv_example_image_4();

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