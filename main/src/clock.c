#include "../lvgl.h"

#include <sys/time.h>
#include <time.h>

#ifdef LV_USE_FREETYPE

#if LV_FREETYPE_USE_LVGL_PORT
    #define PATH_PREFIX "A:"
#else
    #define PATH_PREFIX "/home/share/samba/lvgl/lv_port_pc_vscode/"
#endif

lv_obj_t *clock_label;

void update_clock(lv_timer_t *timer)
{
    time_t now;
    struct tm timeinfo;
    char buf[9];

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);

    lv_label_set_text(clock_label, buf);
}


void lv_app_clock(void)
{
   /*Create a font*/
    lv_font_t * font = lv_freetype_font_create(PATH_PREFIX "lvgl/demos/multilang/assets/fonts/BebasNeue-Regular.ttf",
                                               LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                               100,
                                               LV_FREETYPE_FONT_STYLE_NORMAL);

    if(!font) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }

    /*Create style with the new font*/
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, font);
    lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER);

    /*Create a label with the new style*/
    clock_label = lv_label_create(lv_screen_active());
    lv_obj_add_style(clock_label, &style, 0);
    lv_label_set_text(clock_label, "");
    lv_obj_center(clock_label); 
    lv_obj_set_style_text_color(clock_label, lv_color_hex(0xFF0000), 0);

    lv_timer_create(update_clock, 1000, NULL);
}

#else




// void lv_app_clock(void)
// {
//     lv_obj_t *scr = lv_scr_act();
//     // lv_obj_t *label = lv_label_create(scr);
//     // lv_label_set_text(label, "Hello, world!");
//     // lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

//     // 创建一个标签用于显示时间
//     clock_label = lv_label_create(scr);
//     lv_obj_align(clock_label, LV_ALIGN_CENTER, 0, 20);
    
//     //设置文本大小 默认是lv_font_montserrat_14
//     lv_obj_set_style_text_font(clock_label, &lv_font_montserrat_48, 0);
//     lv_label_set_text(clock_label, "");

//     /* 改变文本颜色（所有） */
//     lv_obj_set_style_text_color(clock_label, lv_color_hex(0xFF0000), 0);
//     // 在主函数或初始化函数中设置定时器
//     lv_timer_create(update_clock, 1000, NULL);
// }
#endif