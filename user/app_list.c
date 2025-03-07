

// #include "weather_app.h"

// #include "test_app.h"
#include "clock_app.h"
#include "setting_app.h"
// #include "yuanshen_app.h"
// #include "compass_app.h"

// #include "watch_01.h"
// #include "watch_02.h"

// #include "album_app.h"
// #include "print_screen_app.h"


#include "control_center_app.h"

void app_startup_list() // APP初始化列表,按需裁剪，不需要的注释，可以大幅度减少烧录时间
{
    /////////////// 无UI底层APP//////////////////
    //weather_app_install();     // 天气管理app

    ////////////////////////////////////////


    // test_app_install();     // 测试APP
     //game_2048_install();    // 2048小游戏
     //game_minesweeper_install(); // 扫雷小游戏
    // album_app_install();    // 相册APP
     setting_app_install();  // 设置APP
     setting_app_install();  // 设置APP
     setting_app_install();  // 设置APP
     setting_app_install();  // 设置APP
     setting_app_install();  // 设置APP
    // compass_app_install();  // 指南针APP
    clock_app_install();    // 时钟APP
   // yuanshen_app_install(); // 原神启动器

/////////////// 有UI交互APP/////////////////
// 在桌面菜单内顺序可以通过更改这里的上下顺序自定义
#if QF_ZERO_V2_DEV_EN
    // 不会被static_include.h配置的开发模式隐藏的app
    // test_app_install();    // 测试APP

#else
    // 会被隐藏不启用的APP
    // test_app_install();     // 测试APP
    game_2048_install();        // 2048游戏
    game_minesweeper_install(); // 扫雷游戏
    yuanshen_app_install();     // 原神启动器
    // muyu_app_install();         // 电子木鱼APP
    // pop_cat_app_install();      // pop猫APP
    compass_app_install();      // 指南针APP
    clock_app_install();        // 时钟APP
    // app_mouse_install();        // 鼠标APP
    setting_app_install();      // 设置APP
#endif

    control_center_app_install(); // 控制中心APP
                                  /////////////////////////////////////
}

void desktop_ui_list() // 桌面UI初始化列表,按需裁剪，不需要的注释，可以大幅度减少烧录时间
{
#if QF_ZERO_V2_DESKTOP_HIDDEN_EN == 1
    // 不会被static_include.h配置的隐藏桌面表盘功能隐藏的表盘
    watch_01_install(); // 官方表盘5

#elif QF_ZERO_V2_DESKTOP_HIDDEN_EN == 0
    // 会被static_include.h配置的隐藏桌面表盘功能隐藏的表盘
    // watch_01_install(); // 官方表盘1
    // watch_02_install(); // 官方表盘2
    watch_01_install(); // 官方表盘1
    watch_02_install(); // 官方表盘2
    watch_03_install(); // 官方表盘3
    watch_04_install(); // 官方表盘4
    watch_05_install(); // 官方表盘5
#endif
}
