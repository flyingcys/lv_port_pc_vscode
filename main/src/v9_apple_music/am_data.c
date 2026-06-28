#include "am_data.h"
#include "am_icons.h"

const am_nav_item_t am_nav_items[AM_VIEW_COUNT] = {
    {"now", "正在播放", AM_ICON_PLAY},
    {"radio", "广播电台", AM_ICON_RADIO},
    {"favorites", "我的收藏", AM_ICON_HEART},
};

const char *am_recent_titles[4] = {
    "火力全开 (DJ版)",
    "光年之外 (Remix)",
    "起风了 (DJ版)",
    "海阔天空 (摇滚版)",
};

const char *am_mock_lyrics[4] = {
    "把播放器壳层先搭稳，再把真实数据接进来。",
    "本地资料库走目录扫描，广播列表走 sources.csv。",
    "当前行高亮、底部条状态、播放列表弹层保持同步。",
    "视觉结构以 macOS Apple Music 轻主题为基准。",
};

const am_theme_preset_t am_theme_presets[4] = {
    {"cyan", "Cyan", "冷白底配青色强调"},
    {"blue", "Blue", "偏蓝的桌面中性调"},
    {"mint", "Mint", "默认薄荷绿强调"},
    {"orange", "Orange", "暖调橙色强调"},
};

const am_settings_tab_t am_settings_tabs[3] = {
    {"appearance", "外观", "保留旧模块编译所需常量"},
    {"playback", "播放", "本轮入口不再使用设置页"},
    {"about", "关于", "仅用于兼容旧源码编译"},
};

const char *am_view_label(am_view_t view)
{
    if(view < 0 || view >= AM_VIEW_COUNT) return "正在播放";
    return am_nav_items[view].label;
}

const char *am_source_badge(am_source_kind_t kind)
{
    switch(kind) {
        case AM_SOURCE_LOCAL:
            return "本地音频";
        case AM_SOURCE_RADIO:
            return "LIVE";
        case AM_SOURCE_NONE:
        default:
            return "未播放";
    }
}
