#include "am_data.h"

const am_nav_item_t am_nav_items[5] = {
    {"home",     "主页", "\xE2\x8C\x82"},
    {"radio",    "广播", "\xE2\x97\x89"},
    {"local",    "本地", "\xE2\x99\xAB"},
    {"playlist", "歌单", "\xE2\x89\xA3"},
    {"settings", "设置", "\xE2\x9A\x99"},
};

const am_theme_preset_t am_theme_presets[4] = {
    {"cyan",   "Cyan",   "淡青主调"},
    {"blue",   "Blue",   "夜间轻冷"},
    {"mint",   "Mint",   "清透早晨"},
    {"orange", "Orange", "暖调黄金时刻"},
};

const am_settings_tab_t am_settings_tabs[3] = {
    {"appearance", "外观", "主题、颜色与氛围"},
    {"playback",   "播放", "默认队列与过渡策略"},
    {"about",      "关于", "设备信息与版本说明"},
};

const am_hero_t am_home_hero = {
    "Live Radio Selection",
    "从广播开始，今天节奏已经选好",
    "先听推荐广播，再回到最近播放。",
    "今日主打",
};

const am_simple_item_t am_home_recommends[3] = {
    {"Lo-Fi 早班电台",   "27 min continuous mix"},
    {"City Pop Avenue", "东京夜风与合成器"},
    {"Pure Piano Focus","安静但不失推进感"},
};

const am_recent_item_t am_home_recent[3] = {
    {"广播", "New Music Daily",        "Apple Music 1"},
    {"歌单", "Coding Without Hurry",   "18 首歌曲"},
    {"单曲", "Golden Hour",            "JVKE"},
};

const am_metric_t am_home_metrics[3] = {
    {"广播收藏", "12"},
    {"连续收听", "48m"},
    {"今日推荐", "06"},
};

const am_mini_player_t am_mini = {
    "Lo-Fi 早班电台",
    "Morning Transit Session",
    "01:42",
    "03:58",
    44,
};

const am_list_page_t am_page_radio = {
    .eyebrow      = "广播目录",
    .title        = "广播",
    .subtitle     = "把首页广播优先的心智展开成频道页，适合快速挑到当下想听的气氛。",
    .banner_title = "Apple Music 电台精选",
    .banner_desc  = "从编辑推荐、DJ 专栏到 mood station，全部先做静态视觉确认，再迁回 LVGL 结构。",
    .badges       = {"24h Live", "Editor Picks", "Trending"},
    .queue_label  = "正在预排 4 个节目",
    .list = {
        {"radio", "The New Music Station", "全球新歌 / 每小时刷新", "现在开始", "LIVE"},
        {"radio", "Chill Sunday",          "低压氛围 / 柔和女声",   "42 分钟",  "Mix"},
        {"radio", "After Midnight Jazz",   "夜色铜管 / 慢速鼓刷",   "28 分钟",  "HD"},
        {"radio", "Electro Run Club",      "高 BPM / 晨跑编排",     "56 分钟",  "New"},
    },
    .queue = {
        {"主持人开场", "02:14 后切到主节目"},
        {"新歌连播",   "含 3 首首发曲目"},
        {"DJ 访谈",    "片段预留"},
    },
};

const am_list_page_t am_page_local = {
    .eyebrow      = "本地资料库",
    .title        = "本地",
    .subtitle     = "先验证列表型页面在 800x480 下的信息密度、层级和留白控制。",
    .banner_title = "最近导入的专辑与单曲",
    .banner_desc  = "不接真实扫描逻辑，只用静态假数据模拟专辑、单曲和收藏混合视图。",
    .badges       = {"Albums", "Lossless", "Recently Added"},
    .queue_label  = "资料库总量 148 项",
    .list = {
        {"album", "In Rainbows",         "Radiohead / 专辑",         "10 tracks", "Album"},
        {"album", "Souvlaki",            "Slowdive / 专辑",           "9 tracks",  "Album"},
        {"song",  "Nights",              "Frank Ocean / 单曲",        "5:07",      "Song"},
        {"song",  "Sunset Rollercoaster","My Jinji / 单曲",           "4:26",      "Fav"},
    },
    .queue = {
        {"下载完成", "3 张专辑可离线播放"},
        {"上次播放", "昨晚 22:14 停在 track 07"},
        {"同步状态", "iCloud 占位 UI"},
    },
};

const am_list_page_t am_page_playlist = {
    .eyebrow      = "歌单集合",
    .title        = "歌单",
    .subtitle     = "用更轻的卡片感展示个人歌单与推荐歌单，观察标题长度和二级信息的压缩方式。",
    .banner_title = "你的歌单今天适合这样排",
    .banner_desc  = "先把结构与节奏做顺，再决定 LVGL 里是 grid 卡片还是列表卡片混排。",
    .badges       = {"Curated", "Personal", "Smart Mix"},
    .queue_label  = "收藏歌单 9 个",
    .list = {
        {"playlist", "晨间编译",     "15 首 / 稳定推进型",     "35 min",  "Pinned"},
        {"playlist", "夜行公路",     "22 首 / 合成器和鼓机",   "1h 24m",  "Road"},
        {"playlist", "低功耗工作流", "13 首 / 人声克制",       "48 min",  "Focus"},
        {"playlist", "广播回放收藏", "8 集 / 长节目",          "2h 03m",  "Radio"},
    },
    .queue = {
        {"智能续播", "按最近收听顺序推荐"},
        {"封面候选", "后续可接主题色联动"},
        {"共享入口", "本轮只保留视觉占位"},
    },
};
