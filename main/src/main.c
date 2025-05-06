
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
void lv_ui_test(void)
{
  lv_obj_t * obj = lv_obj_create(lv_scr_act());
  lv_obj_set_size(obj, 300, 300);
  // lv_obj_set_pos(obj, 100, 100);
  lv_obj_set_style_bg_color(obj, lv_color_hex(0xff0000), 0);
  lv_obj_set_align(obj, LV_ALIGN_CENTER);

  lv_obj_t * label = lv_label_create(obj);
  lv_label_set_text(label, "Hello world");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

lv_draw_buf_t draw_buf_;
lv_obj_t* status_bar_ = NULL;
lv_obj_t* content_ = NULL;
lv_obj_t* container_ = NULL;
lv_obj_t* side_bar_ = NULL;

lv_display_t *display_ = NULL;

lv_obj_t *emotion_label_ = NULL;
lv_obj_t *network_label_ = NULL;
lv_obj_t *status_label_ = NULL;
lv_obj_t *notification_label_ = NULL;
lv_obj_t *mute_label_ = NULL;
lv_obj_t *battery_label_ = NULL;
lv_obj_t* chat_message_label_ = NULL;
lv_obj_t* low_battery_popup_ = NULL;
lv_obj_t* low_battery_label_ = NULL;


// Color definitions for dark theme
#define DARK_BACKGROUND_COLOR       lv_color_hex(0x121212)     // Dark background
#define DARK_TEXT_COLOR             lv_color_white()           // White text
#define DARK_CHAT_BACKGROUND_COLOR  lv_color_hex(0x1E1E1E)     // Slightly lighter than background
#define DARK_USER_BUBBLE_COLOR      lv_color_hex(0x1A6C37)     // Dark green
#define DARK_ASSISTANT_BUBBLE_COLOR lv_color_hex(0x333333)     // Dark gray
#define DARK_SYSTEM_BUBBLE_COLOR    lv_color_hex(0x2A2A2A)     // Medium gray
#define DARK_SYSTEM_TEXT_COLOR      lv_color_hex(0xAAAAAA)     // Light gray text
#define DARK_BORDER_COLOR           lv_color_hex(0x333333)     // Dark gray border
#define DARK_LOW_BATTERY_COLOR      lv_color_hex(0xFF0000)     // Red for dark mode

// Color definitions for light theme
#define LIGHT_BACKGROUND_COLOR       lv_color_white()           // White background
#define LIGHT_TEXT_COLOR             lv_color_black()           // Black text
#define LIGHT_CHAT_BACKGROUND_COLOR  lv_color_hex(0xE0E0E0)     // Light gray background
#define LIGHT_USER_BUBBLE_COLOR      lv_color_hex(0x95EC69)     // WeChat green
#define LIGHT_ASSISTANT_BUBBLE_COLOR lv_color_white()           // White
#define LIGHT_SYSTEM_BUBBLE_COLOR    lv_color_hex(0xE0E0E0)     // Light gray
#define LIGHT_SYSTEM_TEXT_COLOR      lv_color_hex(0x666666)     // Dark gray text
#define LIGHT_BORDER_COLOR           lv_color_hex(0xE0E0E0)     // Light gray border
#define LIGHT_LOW_BATTERY_COLOR      lv_color_black()           // Black for light mode

struct ThemeColors {
  lv_color_t background;
  lv_color_t text;
  lv_color_t chat_background;
  lv_color_t user_bubble;
  lv_color_t assistant_bubble;
  lv_color_t system_bubble;
  lv_color_t system_text;
  lv_color_t border;
  lv_color_t low_battery;
} ;

struct ThemeColors current_theme;

struct DisplayFonts {
  lv_font_t* text_font;
  lv_font_t* icon_font;
  lv_font_t* emoji_font;
};

LV_FONT_DECLARE(font_puhui_20_4);
LV_FONT_DECLARE(font_awesome_20_4);
LV_FONT_DECLARE(font_awesome_30_4);

struct DisplayFonts fonts_ = {
  .text_font = &font_puhui_20_4,
  .icon_font = &font_awesome_20_4,
};
void set_theme(bool dark)
{ 
  if (dark) {
    current_theme.background = DARK_BACKGROUND_COLOR;
    current_theme.text = DARK_TEXT_COLOR;
    current_theme.chat_background = DARK_CHAT_BACKGROUND_COLOR;
    current_theme.user_bubble = DARK_USER_BUBBLE_COLOR;
    current_theme.assistant_bubble = DARK_ASSISTANT_BUBBLE_COLOR;
    current_theme.system_bubble = DARK_SYSTEM_BUBBLE_COLOR;
    current_theme.system_text = DARK_SYSTEM_TEXT_COLOR;
    current_theme.border = DARK_BORDER_COLOR;
    current_theme.low_battery = DARK_LOW_BATTERY_COLOR;

  } else {
    current_theme.background = LIGHT_BACKGROUND_COLOR;
    current_theme.text = LIGHT_TEXT_COLOR;
    current_theme.chat_background = LIGHT_CHAT_BACKGROUND_COLOR;
    current_theme.user_bubble = LIGHT_USER_BUBBLE_COLOR;
    current_theme.assistant_bubble = LIGHT_ASSISTANT_BUBBLE_COLOR;
    current_theme.system_bubble = LIGHT_SYSTEM_BUBBLE_COLOR;
    current_theme.system_text = LIGHT_SYSTEM_TEXT_COLOR;
    current_theme.border = LIGHT_BORDER_COLOR;
    current_theme.low_battery = LIGHT_LOW_BATTERY_COLOR;
  }
}

#include "font_awesome_symbols.h"
const char* INITIALIZING = "正在初始化...";
const char* BATTERY_NEED_CHARGE = "电量低，请充电";
const char* BATTERY_CHARGING = "正在充电";
void lv_example_xiaozhi(void)
{
  set_theme(false);

  lv_obj_t * screen = lv_screen_active();
  lv_obj_set_style_text_font(screen, fonts_.text_font, 0);
  lv_obj_set_style_text_color(screen, current_theme.text, 0);
  lv_obj_set_style_bg_color(screen, current_theme.background, 0);

  /* Container */
  container_ = lv_obj_create(screen);
  lv_obj_set_size(container_, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);        // 垂直布局
  lv_obj_set_style_pad_all(container_, 0, 0);                   // 无内边距
  lv_obj_set_style_border_width(container_, 0, 0);              // 无边框
  lv_obj_set_style_pad_row(container_, 0, 0);                   // 设置行间距
  lv_obj_set_style_bg_color(container_, current_theme.background, 0);       // 设置背景颜色属性
  lv_obj_set_style_border_color(container_, current_theme.border, 0);       // 设置边框的颜色属性

  /* Status bar */
  // status_bar_ = lv_obj_create(container_);
  // lv_obj_set_size(status_bar_, LV_HOR_RES, fonts_.text_font->line_height);
  // lv_obj_set_style_radius(status_bar_, 0, 0);
  // lv_obj_set_style_bg_color(status_bar_, current_theme.background, 0);
  // lv_obj_set_style_text_color(status_bar_, current_theme.text, 0);
  
  // lv_obj_set_flex_flow(status_bar_, LV_FLEX_FLOW_ROW);
  // lv_obj_set_style_pad_all(status_bar_, 0, 0);
  // lv_obj_set_style_border_width(status_bar_, 0, 0);
  // lv_obj_set_style_pad_column(status_bar_, 0, 0);
  // lv_obj_set_style_pad_left(status_bar_, 2, 0);
  // lv_obj_set_style_pad_right(status_bar_, 2, 0);

  // network_label_ = lv_label_create(status_bar_);
  // lv_label_set_text(network_label_, LV_SYMBOL_WIFI);
  // lv_obj_set_style_text_font(network_label_, fonts_.icon_font, 0);
  // lv_obj_set_style_text_color(network_label_, current_theme.text, 0);

  // notification_label_ = lv_label_create(status_bar_);
  // lv_obj_set_flex_grow(notification_label_, 1);
  // lv_obj_set_style_text_align(notification_label_, LV_TEXT_ALIGN_CENTER, 0);
  // lv_obj_set_style_text_color(notification_label_, current_theme.text, 0);
  // lv_label_set_text(notification_label_, "");
  // lv_obj_add_flag(notification_label_, LV_OBJ_FLAG_HIDDEN);

  // status_label_ = lv_label_create(status_bar_);
  // lv_obj_set_flex_grow(status_label_, 1);
  // lv_label_set_long_mode(status_label_, LV_LABEL_LONG_SCROLL_CIRCULAR);
  // lv_obj_set_style_text_align(status_label_, LV_TEXT_ALIGN_CENTER, 0);
  // lv_obj_set_style_text_color(status_label_, current_theme.text, 0);
  // lv_label_set_text(status_label_, INITIALIZING);

  // mute_label_ = lv_label_create(status_bar_);
  // lv_label_set_text(mute_label_, "");
  // lv_obj_set_style_text_font(mute_label_, fonts_.icon_font, 0);
  // lv_obj_set_style_text_color(mute_label_, current_theme.text, 0);

  // battery_label_ = lv_label_create(status_bar_);
  // lv_label_set_text(battery_label_, LV_SYMBOL_BATTERY_FULL);
  // lv_obj_set_style_text_font(battery_label_, fonts_.icon_font, 0);
  // lv_obj_set_style_text_color(battery_label_, current_theme.text, 0);

  low_battery_popup_ = lv_obj_create(screen);
  lv_obj_set_scrollbar_mode(low_battery_popup_, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_size(low_battery_popup_, LV_HOR_RES * 0.9, fonts_.text_font->line_height * 2);
  lv_obj_align(low_battery_popup_, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_bg_color(low_battery_popup_, current_theme.low_battery, 0);
  lv_obj_set_style_radius(low_battery_popup_, 10, 0);                         // 设置圆角半径为10像素

  low_battery_label_ = lv_label_create(low_battery_popup_);
  lv_label_set_text(low_battery_label_, BATTERY_NEED_CHARGE);
  lv_obj_set_style_text_color(low_battery_label_, lv_color_white(), 0);
  lv_obj_center(low_battery_label_);
  // lv_obj_add_flag(low_battery_popup_, LV_OBJ_FLAG_HIDDEN);                 // 设置为隐藏
    
  /* Content */
  content_ = lv_obj_create(container_);
  lv_obj_set_scrollbar_mode(content_, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_radius(content_, 0, 0);
  lv_obj_set_width(content_, LV_HOR_RES);
  lv_obj_set_flex_grow(content_, 1);                                         // 设置为填充剩余空间
  lv_obj_set_style_pad_all(content_, 5, 0);                                 //设置四个方向的填充为5像素
  lv_obj_set_style_bg_color(content_, current_theme.chat_background, 0);
  lv_obj_set_style_border_color(content_, current_theme.border, 0); // Border color for content

  lv_obj_set_flex_flow(content_, LV_FLEX_FLOW_COLUMN);                                                      // 垂直布局（从上到下）
  lv_obj_set_flex_align(content_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);  // 子对象居中对齐，等距分布

  emotion_label_ = lv_label_create(content_);
  lv_obj_set_style_text_font(emotion_label_, &font_awesome_30_4, 0);
  lv_obj_set_style_text_color(emotion_label_, current_theme.text, 0);
  lv_label_set_text(emotion_label_, FONT_AWESOME_AI_CHIP);

  chat_message_label_ = lv_label_create(content_);
  lv_label_set_text(chat_message_label_, "message");
  lv_obj_set_width(chat_message_label_, LV_HOR_RES * 0.9); // 限制宽度为屏幕宽度的 90%
  lv_label_set_long_mode(chat_message_label_, LV_LABEL_LONG_WRAP); // 设置为自动换行模式
  lv_obj_set_style_text_align(chat_message_label_, LV_TEXT_ALIGN_CENTER, 0); // 设置文本居中对齐
  lv_obj_set_style_text_color(chat_message_label_, current_theme.text, 0);
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
 
  // lv_demo_widgets();
  // lv_example_flex_2();
  lv_example_xiaozhi();
  // lv_ui_test();
  // lv_example_anim_1();

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