#include "desktop_data.h"
#include "apple_music_app.h"
#include "calc/calc_app.h"
#include "fruit_ninja_app.h"
#include "music_player.h"
LV_IMAGE_DECLARE(img_app_clock);   LV_IMAGE_DECLARE(img_app_photos);
LV_IMAGE_DECLARE(img_app_calc);    LV_IMAGE_DECLARE(img_app_calc_plus);
LV_IMAGE_DECLARE(img_app_files);   LV_IMAGE_DECLARE(img_app_netease);
LV_IMAGE_DECLARE(img_app_qqmusic); LV_IMAGE_DECLARE(img_app_applemusic);
LV_IMAGE_DECLARE(img_app_2048);    LV_IMAGE_DECLARE(img_app_blockpuzzle);
LV_IMAGE_DECLARE(img_app_blockblast); LV_IMAGE_DECLARE(img_app_fruitninja);

const desktop_app_t desktop_apps[] = {
    { &img_app_clock,      "时钟",        1, NULL },
    { &img_app_photos,     "相册",        1, NULL },
    { &img_app_calc,       "计算器",      1, calc_app_launch },
    { &img_app_calc_plus,  "计算器+",     1, NULL },
    { &img_app_files,      "文件管理",    1, NULL },
    { &img_app_netease,    "网易云音乐",  1, local_music_demo_launch },
    { &img_app_qqmusic,    "QQ音乐",      1, NULL },
    { &img_app_applemusic, "Apple Music", 1, apple_music_app_launch },
    { &img_app_2048,       "2048",        1, NULL },
    { &img_app_blockpuzzle,"方块拼图",    1, NULL },
    { &img_app_blockblast, "方块爆炸",    2, NULL },
    { &img_app_fruitninja, "水果忍者",    2, fruit_ninja_app_launch },
};
const uint32_t desktop_app_count = sizeof(desktop_apps)/sizeof(desktop_apps[0]);

const desktop_notify_t desktop_notifies[] = {
    { "系统消息", "欢迎使用全新横屏系统UI！左右滑动切换应用，上下滑动调出面板。", "刚刚" },
    { "日程提醒", "下午 2:00 有一个产品设计评审会议。", "1小时前" },
};
const uint32_t desktop_notify_count = sizeof(desktop_notifies)/sizeof(desktop_notifies[0]);
