#include <lvgl/lvgl.h>
#include "game_2048_improved.h"

// 测试函数
void test_2048_improved(void)
{
    // 获取当前屏幕
    lv_obj_t *screen = lv_screen_active();
    
    // 创建改进版2048游戏
    game_2048_improved(screen);
    
    printf("改进版2048游戏测试启动成功\n");
} 