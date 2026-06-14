# Apple Music LVGL v9 高保真复刻 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 LVGL v9 + SDL 下新建独立模块,纯视觉复刻 `third-party/apple_music_player_mockup/` 的 Apple Music 样机(800×480,4 主题,5 页,玻璃拟态)。

**Architecture:** 方案 A —— 侧栏+迷你播放条常驻;切页只重建内容区;换肤整壳重建。约 10 个高内聚模块,依赖单向:`data/theme/fonts/icons → widgets → shell/pages → apple_music(控制器)`。零图片资源,全用 LVGL 原生渐变/阴影/圆角/半透明近似。

**Tech Stack:** C99、LVGL v9、SDL2、CMake、`lv_font_conv`(子集字体)。

**唯一事实来源:** 设计 spec `docs/superpowers/specs/2026-06-14-apple-music-lvgl-design.md`(含 4 主题完整色值表、逐区布局尺寸);视觉对照 `third-party/apple_music_player_mockup/`(index.html / styles.css / app.js)。

---

## 约定(每个任务通用)

**构建**(改了 `CMakeLists.txt` / `lv_conf.h` 后需重新配置):
```bash
cmake -S . -B build && cmake --build build -j"$(nproc)"
```

**运行**(可执行因 `EXECUTABLE_OUTPUT_PATH=${PROJECT_SOURCE_DIR}/bin` 落在项目根 `bin/`):
```bash
./bin/main          # 弹出 800×480 SDL 窗口
```

**逻辑单测**:
```bash
cd build && ctest -R apple_music --output-on-failure    # 或直接运行 ./bin/<test_name>
```

**视觉验证**:运行 `./bin/main`,与浏览器打开的 `third-party/apple_music_player_mockup/index.html` 逐区对照;可调用 `verify` 技能截图比对。**SDL 窗口无指针 hover,故 mockup 的 hover/transition 不复刻**(见 spec §11)。

**验证方式说明**:纯逻辑模块(`am_data`/`am_theme`)走 TDD(先写失败测试);UI 模块走"实现→构建→运行截图比对→提交"(LVGL 渲染无法用单测断言,强行写假测试是表演,不做)。

**项目规约(CLAUDE.md)**:**修改已存在文件前**(`main.c`/`CMakeLists.txt`/`lv_conf.h`)先用 `gitnexus_impact` 或 `get_impact_radius` 看影响面;**每次提交前**用 `detect_changes` 确认改动范围符合预期。新建文件无既有符号,无需 impact。

**提交粒度**:每个 Task 末尾一次提交;commit message 用 `feat(music): ...` 前缀,结尾带:
```
Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
```

---

## 文件结构(decomposition)

| 文件 | 职责 |
|------|------|
| `main/src/v9_apple_music/apple_music.h/.c` | 入口 `apple_music_create()` + 控制器:状态机、根 grid、shell 装配、重建编排、事件回调 |
| `main/src/v9_apple_music/am_data.h/.c` | 静态假数据(镜像 app.js) |
| `main/src/v9_apple_music/am_theme.h/.c` | 4 主题 token + 共享色常量 + 当前主题访问器 |
| `main/src/v9_apple_music/am_fonts.h` (+ `fonts/*.c`) | 子集字体声明与位图 |
| `main/src/v9_apple_music/am_icons.h` | 图标字形 UTF-8 宏 |
| `main/src/v9_apple_music/am_widgets.h/.c` | 可复用工厂:渐变助手/玻璃面板/卡片/胶囊/nav_item/封面/item_action/小标题/按钮/eyebrow/search_chip |
| `main/src/v9_apple_music/am_shell.h/.c` | 常驻 chrome:侧栏 + 迷你播放条 |
| `main/src/v9_apple_music/am_page_home.h/.c` | 主页 |
| `main/src/v9_apple_music/am_page_list.h/.c` | 列表页模板(广播/本地/歌单数据驱动) |
| `main/src/v9_apple_music/am_page_settings.h/.c` | 设置页 + 3 tab |
| `main/tests/apple_music_data_test.c` | am_data 计数单测 |
| `main/tests/apple_music_theme_test.c` | am_theme 取值单测 |

**修改**:`CMakeLists.txt`(源码/测试注册)、`lv_conf.h`(渐变档数+字体声明)、`main/src/main.c`(分辨率+入口切换)。

**并行性**:Task 8/9/10(三个页面)写面互不重叠,可并行(均依赖 Task 1-7)。

---

## Task 1: 骨架 + 构建接入 + 800×480 空壳冒烟

**Files:**
- Create: `main/src/v9_apple_music/apple_music.h`
- Create: `main/src/v9_apple_music/apple_music.c`
- Modify: `lv_conf.h`(`LV_GRADIENT_MAX_STOPS`)
- Modify: `CMakeLists.txt`(新增 GLOB 并加入 main 目标)
- Modify: `main/src/main.c`(分辨率 + 入口)

- [ ] **Step 1: `lv_conf.h` 开启三段渐变**

定位到 `LV_DRAW_SW_COMPLEX` 附近(约 205 行)的渐变配置区,在 `LV_USE_DRAW_SW_COMPLEX_GRADIENTS` 之后新增一行(若已存在 `LV_GRADIENT_MAX_STOPS` 则改其值):
```c
#define LV_GRADIENT_MAX_STOPS 4
```

- [ ] **Step 2: 写 `apple_music.h`**

```c
/* main/src/v9_apple_music/apple_music.h */
#ifndef APPLE_MUSIC_H
#define APPLE_MUSIC_H

/* 在当前活动屏幕上构建整个 Apple Music UI(800x480) */
void apple_music_create(void);

#endif /* APPLE_MUSIC_H */
```

- [ ] **Step 3: 写 `apple_music.c` 最小骨架(根 grid + 3 个着色占位区)**

```c
/* main/src/v9_apple_music/apple_music.c */
#include "apple_music.h"
#include "lvgl/lvgl.h"

#define AM_SIDEBAR_W   164
#define AM_PLAYER_H    78

/* 根 grid:列[164, FR1] 行[FR1, 78],侧栏跨两行 */
static const int32_t col_dsc[] = {AM_SIDEBAR_W, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static const int32_t row_dsc[] = {LV_GRID_FR(1), AM_PLAYER_H, LV_GRID_TEMPLATE_LAST};

void apple_music_create(void)
{
    lv_obj_t *root = lv_screen_active();
    lv_obj_remove_style_all(root);
    lv_obj_set_style_bg_color(root, lv_color_hex(0xedf8f4), 0); /* mint bg-top 占位 */
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_grid_dsc_array(root, col_dsc, row_dsc);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_pad_gap(root, 0, 0);

    /* 侧栏占位:col0 row0 跨两行 */
    lv_obj_t *sidebar = lv_obj_create(root);
    lv_obj_remove_style_all(sidebar);
    lv_obj_set_style_bg_color(sidebar, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(sidebar, LV_OPA_80, 0);
    lv_obj_set_grid_cell(sidebar, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 2);

    /* 内容区占位:col1 row0 */
    lv_obj_t *content = lv_obj_create(root);
    lv_obj_remove_style_all(content);
    lv_obj_set_style_bg_color(content, lv_color_hex(0xf6f7fa), 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_COVER, 0);
    lv_obj_set_grid_cell(content, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    /* 迷你播放条占位:col1 row1 */
    lv_obj_t *player = lv_obj_create(root);
    lv_obj_remove_style_all(player);
    lv_obj_set_style_bg_color(player, lv_color_hex(0xf9fafc), 0);
    lv_obj_set_style_bg_opa(player, LV_OPA_COVER, 0);
    lv_obj_set_grid_cell(player, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
}
```

- [ ] **Step 4: `CMakeLists.txt` 接入新模块**

在 `file(GLOB FRUIT_NINJA_SOURCES ...)`(约 51 行)之后新增:
```cmake
file(GLOB APPLE_MUSIC_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/*.c")
file(GLOB APPLE_MUSIC_FONTS CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/fonts/*.c")
```
在 `target_sources(main PRIVATE ${FRUIT_NINJA_SOURCES})`(约 151 行)之后新增:
```cmake
target_sources(main PRIVATE ${APPLE_MUSIC_SOURCES} ${APPLE_MUSIC_FONTS})
```

- [ ] **Step 5: 改 `main/src/main.c`**(先跑 `gitnexus_impact({target:"main", direction:"upstream"})`)

```c
#include "apple_music.h"   /* 替换/新增,放在 #include "music_player.h" 处 */
```
`main()` 内:
- `hal_init(640, 480);` → `hal_init(800, 480);`
- 注释掉 `music_player_init(...);`(纯视觉无音频,避免后端回调进已不存在的 demo UI)
- `lv_demo_music();` → `apple_music_create();`(原 `lv_demo_music();` 与已注释的 `fruit_ninja_start();` 保留为注释)
- 注释掉 `music_player_deinit();`

- [ ] **Step 6: 构建**

Run: `cmake -S . -B build && cmake --build build -j"$(nproc)"`
Expected: 编译链接成功,无 error。

- [ ] **Step 7: 运行冒烟**

Run: `./bin/main`
Expected: 800×480 窗口;左侧 164px 半透明白色侧栏占满整高;右上为内容区;右下 78px 为播放条占位;三区位置与 `index.html` 的 `.app-shell` grid 一致。

- [ ] **Step 8: 提交**(先 `detect_changes`)
```bash
git add main/src/v9_apple_music/ CMakeLists.txt lv_conf.h main/src/main.c
git commit -m "feat(music): scaffold apple_music module + 800x480 shell grid"
```

---

## Task 2: `am_data` 静态数据(TDD)

**Files:**
- Create: `main/src/v9_apple_music/am_data.h`, `am_data.c`
- Test: `main/tests/apple_music_data_test.c`
- Modify: `CMakeLists.txt`(注册测试)

- [ ] **Step 1: 写 `am_data.h`(完整结构定义 + extern 计数)**

```c
/* main/src/v9_apple_music/am_data.h */
#ifndef AM_DATA_H
#define AM_DATA_H
#include <stddef.h>

typedef struct { const char *id; const char *label; const char *icon; } am_nav_item_t;
typedef struct { const char *id; const char *label; const char *desc; } am_theme_preset_t;
typedef struct { const char *id; const char *title; const char *desc; } am_settings_tab_t;

typedef struct { const char *title; const char *subtitle; } am_simple_item_t;     /* recommends */
typedef struct { const char *kind; const char *title; const char *subtitle; } am_recent_item_t;
typedef struct { const char *label; const char *value; } am_metric_t;
typedef struct {
    const char *topline; const char *title; const char *desc; const char *badge;
} am_hero_t;

typedef struct {
    const char *kind;    /* album/radio/playlist/song -> 封面渐变色 */
    const char *title; const char *subtitle; const char *meta; const char *aux;
} am_media_item_t;
typedef struct { const char *title; const char *subtitle; } am_queue_item_t;

typedef struct {
    const char *eyebrow, *title, *subtitle;
    const char *banner_title, *banner_desc;
    const char *badges[3];
    const char *queue_label;
    am_media_item_t list[4];
    am_queue_item_t queue[3];
} am_list_page_t;

typedef struct { const char *title, *subtitle, *current, *total; int progress_pct; } am_mini_player_t;

extern const am_nav_item_t      am_nav_items[5];
extern const am_theme_preset_t  am_theme_presets[4];
extern const am_settings_tab_t  am_settings_tabs[3];
extern const am_hero_t          am_home_hero;
extern const am_simple_item_t   am_home_recommends[3];
extern const am_recent_item_t   am_home_recent[3];
extern const am_metric_t        am_home_metrics[3];
extern const am_list_page_t     am_page_radio;
extern const am_list_page_t     am_page_local;
extern const am_list_page_t     am_page_playlist;
extern const am_mini_player_t   am_mini;

#endif /* AM_DATA_H */
```

- [ ] **Step 2: 写失败测试 `main/tests/apple_music_data_test.c`**

```c
#include <assert.h>
#include <string.h>
#include "../src/v9_apple_music/am_data.h"

static void test_counts(void)
{
    assert(sizeof(am_nav_items)/sizeof(am_nav_items[0]) == 5);
    assert(sizeof(am_theme_presets)/sizeof(am_theme_presets[0]) == 4);
    assert(sizeof(am_settings_tabs)/sizeof(am_settings_tabs[0]) == 3);
}
static void test_first_values(void)
{
    assert(strcmp(am_nav_items[0].id, "home") == 0);
    assert(strcmp(am_theme_presets[2].id, "mint") == 0);   /* 默认主题 */
    assert(am_mini.progress_pct == 44);
    assert(strcmp(am_page_radio.list[0].kind, "radio") == 0);
}
int main(void){ test_counts(); test_first_values(); return 0; }
```

- [ ] **Step 3: 注册测试到 `CMakeLists.txt`**(在 fruit_ninja_model_test 注册块之后)

```cmake
add_executable(apple_music_data_test
    ${PROJECT_SOURCE_DIR}/main/tests/apple_music_data_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_data.c
)
target_include_directories(apple_music_data_test PRIVATE ${PROJECT_SOURCE_DIR})
add_test(NAME apple_music_data_test COMMAND $<TARGET_FILE:apple_music_data_test>)
set_tests_properties(apple_music_data_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
```

- [ ] **Step 4: 构建并确认测试失败**

Run: `cmake -S . -B build && cmake --build build --target apple_music_data_test -j"$(nproc)"`
Expected: 链接失败(`am_data.c` 不存在 / 未定义符号)。

- [ ] **Step 5: 写 `am_data.c`(逐字转写 app.js)**

按 app.js 1-121 行**逐字**转写为 C 初始化器。nav/themes/settings/hero/recommends/recent/metrics/mini 全部转写;`am_page_radio/local/playlist` 分别对应 app.js 的 `pageData.radio`(53-72)、`local`(73-92)、`playlist`(93-112)。示例(radio,其余同构):
```c
#include "am_data.h"
const am_nav_item_t am_nav_items[5] = {
    {"home","主页","\xE2\x8C\x82"}, {"radio","广播","\xE2\x97\x89"},
    {"local","本地","\xE2\x99\xAB"}, {"playlist","歌单","\xE2\x89\xA3"},
    {"settings","设置","\xE2\x9A\x99"},
};
const am_theme_preset_t am_theme_presets[4] = {
    {"cyan","Cyan","淡青主调"}, {"blue","Blue","夜间轻冷"},
    {"mint","Mint","清透早晨"}, {"orange","Orange","暖调黄金时刻"},
};
const am_mini_player_t am_mini = {"Lo-Fi 早班电台","Morning Transit Session","01:42","03:58",44};
const am_list_page_t am_page_radio = {
    .eyebrow="广播目录", .title="广播",
    .subtitle="把首页广播优先的心智展开成频道页,适合快速挑到当下想听的气氛。",
    .banner_title="Apple Music 电台精选",
    .banner_desc="从编辑推荐、DJ 专栏到 mood station,全部先做静态视觉确认,再迁回 LVGL 结构。",
    .badges={"24h Live","Editor Picks","Trending"}, .queue_label="正在预排 4 个节目",
    .list={
        {"radio","The New Music Station","全球新歌 / 每小时刷新","现在开始","LIVE"},
        {"radio","Chill Sunday","低压氛围 / 柔和女声","42 分钟","Mix"},
        {"radio","After Midnight Jazz","夜色铜管 / 慢速鼓刷","28 分钟","HD"},
        {"radio","Electro Run Club","高 BPM / 晨跑编排","56 分钟","New"},
    },
    .queue={
        {"主持人开场","02:14 后切到主节目"},{"新歌连播","含 3 首首发曲目"},{"DJ 访谈","片段预留"},
    },
};
/* am_home_hero / am_home_recommends / am_home_recent / am_home_metrics
   及 am_page_local / am_page_playlist 同样逐字转写自 app.js 对应字段 */
```
> nav 图标用 UTF-8 字节(⌂=E2 8C 82, ◉=E2 97 89, ♫=E2 99 AB, ≣=E2 89 A3, ⚙=E2 9A 99);也可在 Task 5 的 `am_icons.h` 定义后改用宏。

- [ ] **Step 6: 构建并确认测试通过**

Run: `cmake --build build --target apple_music_data_test -j"$(nproc)" && ./bin/apple_music_data_test && echo OK`
Expected: 退出码 0,打印 `OK`。

- [ ] **Step 7: 提交**
```bash
git add main/src/v9_apple_music/am_data.* main/tests/apple_music_data_test.c CMakeLists.txt
git commit -m "feat(music): add am_data static mock data + counts test"
```

---

## Task 3: `am_theme` 主题 token(TDD)

**Files:**
- Create: `main/src/v9_apple_music/am_theme.h`, `am_theme.c`
- Test: `main/tests/apple_music_theme_test.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 写 `am_theme.h`**

```c
/* main/src/v9_apple_music/am_theme.h */
#ifndef AM_THEME_H
#define AM_THEME_H
#include "lvgl/lvgl.h"

typedef enum { AM_THEME_CYAN, AM_THEME_BLUE, AM_THEME_MINT, AM_THEME_ORANGE, AM_THEME_COUNT } am_theme_id_t;

typedef struct {
    const char *id;
    lv_color_t bg_top, bg_bottom;
    lv_color_t accent, accent_soft;
    lv_color_t hero_a, hero_b, hero_c;
    lv_color_t tile_a, tile_b, tile_c;
} am_theme_t;

/* 共享色(不随主题变) */
#define AM_TEXT          lv_color_hex(0x21242e)
#define AM_MUTED         lv_color_hex(0x7a8194)
#define AM_MUTED_STRONG  lv_color_hex(0x5f6678)
#define AM_WHITE         lv_color_hex(0xffffff)
/* rgba 透明度:bg_color + bg_opa,如 surface=白62% -> AM_OPA_SURFACE */
#define AM_OPA_SURFACE        158  /* 0.62*255 */
#define AM_OPA_SURFACE_STRONG 219  /* 0.86 */
#define AM_OPA_SURFACE_SOFT   112  /* 0.44 */
#define AM_OPA_BORDER         173  /* 0.68 */
#define AM_OPA_ACCENT_12       31  /* 0.12 */

const am_theme_t *am_theme_get(am_theme_id_t id);
am_theme_id_t     am_theme_current(void);
void              am_theme_set(am_theme_id_t id);

#endif /* AM_THEME_H */
```

- [ ] **Step 2: 写失败测试 `main/tests/apple_music_theme_test.c`**

```c
#include <assert.h>
#include <string.h>
#include "../src/v9_apple_music/am_theme.h"

int main(void)
{
    const am_theme_t *mint = am_theme_get(AM_THEME_MINT);
    assert(strcmp(mint->id, "mint") == 0);
    assert(lv_color_to_u32(mint->accent) == lv_color_to_u32(lv_color_hex(0x23b497)));
    assert(lv_color_to_u32(mint->hero_c) == lv_color_to_u32(lv_color_hex(0x1a6662)));
    am_theme_set(AM_THEME_ORANGE);
    assert(am_theme_current() == AM_THEME_ORANGE);
    assert(lv_color_to_u32(am_theme_get(am_theme_current())->accent)
           == lv_color_to_u32(lv_color_hex(0xf08d3c)));
    return 0;
}
```

- [ ] **Step 3: 注册测试**(`CMakeLists.txt`,需链接 lvgl 以用 `lv_color_*`;若过重,可改测试只比较自定义 hex 而不 include lvgl —— 但 lv_color_hex 需 lvgl。链接 `lvgl`):

```cmake
add_executable(apple_music_theme_test
    ${PROJECT_SOURCE_DIR}/main/tests/apple_music_theme_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_theme.c
)
target_include_directories(apple_music_theme_test PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(apple_music_theme_test PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(apple_music_theme_test lvgl)
add_test(NAME apple_music_theme_test COMMAND $<TARGET_FILE:apple_music_theme_test>)
set_tests_properties(apple_music_theme_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
```

- [ ] **Step 4: 构建确认失败**

Run: `cmake -S . -B build && cmake --build build --target apple_music_theme_test -j"$(nproc)"`
Expected: 链接失败(`am_theme.c` 未实现)。

- [ ] **Step 5: 写 `am_theme.c`(4 主题表 = spec §5.1)**

```c
#include "am_theme.h"
static const am_theme_t g_themes[AM_THEME_COUNT] = {
 [AM_THEME_CYAN]={"cyan",  lv_color_hex(0xeef8f5),lv_color_hex(0xe3edf1),lv_color_hex(0x4bbcae),lv_color_hex(0xd7f5ef),
                  lv_color_hex(0xa8ebe0),lv_color_hex(0x59c7ba),lv_color_hex(0x2b7373),
                  lv_color_hex(0xdaf6f0),lv_color_hex(0xf1fffc),lv_color_hex(0xdff4fb)},
 [AM_THEME_BLUE]={"blue",  lv_color_hex(0xeef4ff),lv_color_hex(0xe5ebfb),lv_color_hex(0x4c78ff),lv_color_hex(0xdbe5ff),
                  lv_color_hex(0x91b3ff),lv_color_hex(0x567cff),lv_color_hex(0x2b3b81),
                  lv_color_hex(0xdce8ff),lv_color_hex(0xeff3ff),lv_color_hex(0xdff6ff)},
 [AM_THEME_MINT]={"mint",  lv_color_hex(0xedf8f4),lv_color_hex(0xe5eff2),lv_color_hex(0x23b497),lv_color_hex(0xd4f7ee),
                  lv_color_hex(0x7ae1cc),lv_color_hex(0x28b89c),lv_color_hex(0x1a6662),
                  lv_color_hex(0xd9f8ef),lv_color_hex(0xf0fffb),lv_color_hex(0xddf4ff)},
 [AM_THEME_ORANGE]={"orange",lv_color_hex(0xfff3e7),lv_color_hex(0xf2ebea),lv_color_hex(0xf08d3c),lv_color_hex(0xffe6cf),
                  lv_color_hex(0xffc66f),lv_color_hex(0xf18b47),lv_color_hex(0x8f4a32),
                  lv_color_hex(0xffe5cc),lv_color_hex(0xfff6ef),lv_color_hex(0xffeedc)},
};
static am_theme_id_t g_current = AM_THEME_MINT;  /* 默认 mint */
const am_theme_t *am_theme_get(am_theme_id_t id){ return &g_themes[id]; }
am_theme_id_t am_theme_current(void){ return g_current; }
void am_theme_set(am_theme_id_t id){ if(id < AM_THEME_COUNT) g_current = id; }
```

- [ ] **Step 6: 构建确认通过**

Run: `cmake --build build --target apple_music_theme_test -j"$(nproc)" && ./bin/apple_music_theme_test && echo OK`
Expected: 退出码 0,`OK`。

- [ ] **Step 7: 提交**
```bash
git add main/src/v9_apple_music/am_theme.* main/tests/apple_music_theme_test.c CMakeLists.txt
git commit -m "feat(music): add am_theme tokens (4 themes) + lookup test"
```

---

## Task 4: 子集字体生成(外部工具)

> ⚠️ 外部依赖:`lv_font_conv`(`npm i -g lv_font_conv` 或 `npx`)+ 源字体 TTF(网络获取)。若环境受限,先确认可用再开工。

**Files:**
- Create: `main/src/v9_apple_music/fonts/am_font_{11,12,13,14,16,18,24,34}.c`(各字号一个)
- Create: `main/src/v9_apple_music/am_fonts.h`
- Modify: `lv_conf.h`(`LV_FONT_CUSTOM_DECLARE`)

- [ ] **Step 1: 准备源字体**

下载到 `main/src/v9_apple_music/fonts/src/`:
- 拉丁:`PlusJakartaSans-SemiBold.ttf`(Google Fonts,OFL)
- 中文:`NotoSansSC-Regular.otf` / `.ttf`(Google Fonts,OFL)
- 符号:`NotoSansSymbols2-Regular.ttf`(覆盖 ⌂◉♫≣⚙♪⌕◌▶⏮⏭↺)

- [ ] **Step 2: 提取字符集**

```bash
python3 - <<'PY'
import pathlib,re
js=pathlib.Path('third-party/apple_music_player_mockup/app.js').read_text(encoding='utf-8')
# 只取字符串字面量里的可见字符 + 显式图标
strs=re.findall(r'"([^"]*)"|`([^`]*)`',js)
text=''.join(a+b for a,b in strs)
glyphs='⌂◉♫≣⚙♪⌕◌▶⏮⏭↺'
keep=set(ch for ch in text+glyphs if 0x20<ord(ch))
print(''.join(sorted(keep)))
PY
```
把输出存为 `main/src/v9_apple_music/fonts/charset.txt`,**人工核对**覆盖全部可见文案。

- [ ] **Step 3: 逐字号生成**(以 14px 为例,其余字号把 `--size`/输出名换成 11/12/13/16/18/24/34)

```bash
SYMS="$(cat main/src/v9_apple_music/fonts/charset.txt)"
npx lv_font_conv --no-compress --format lvgl --bpp 4 --size 14 \
  --font main/src/v9_apple_music/fonts/src/PlusJakartaSans-SemiBold.ttf -r 0x20-0x7F \
  --font main/src/v9_apple_music/fonts/src/NotoSansSC-Regular.ttf --symbols "$SYMS" \
  --font main/src/v9_apple_music/fonts/src/NotoSansSymbols2-Regular.ttf --symbols "⌂◉♫≣⚙♪⌕◌▶⏮⏭↺" \
  -o main/src/v9_apple_music/fonts/am_font_14.c --force-fast-kern-format
```
生成的 C 文件里字体变量名形如 `am_font_14`(由 `-o` 文件名决定;如不符可加 `--lv-font-name am_font_14`)。

- [ ] **Step 4: 写 `am_fonts.h`**

```c
/* main/src/v9_apple_music/am_fonts.h */
#ifndef AM_FONTS_H
#define AM_FONTS_H
#include "lvgl/lvgl.h"
LV_FONT_DECLARE(am_font_11) LV_FONT_DECLARE(am_font_12) LV_FONT_DECLARE(am_font_13)
LV_FONT_DECLARE(am_font_14) LV_FONT_DECLARE(am_font_16) LV_FONT_DECLARE(am_font_18)
LV_FONT_DECLARE(am_font_24) LV_FONT_DECLARE(am_font_34)
#endif
```

- [ ] **Step 5: `lv_conf.h` 声明自定义字体**

将约 693 行的 `#define LV_FONT_CUSTOM_DECLARE` 改为:
```c
#define LV_FONT_CUSTOM_DECLARE \
  LV_FONT_DECLARE(am_font_11) LV_FONT_DECLARE(am_font_12) LV_FONT_DECLARE(am_font_13) \
  LV_FONT_DECLARE(am_font_14) LV_FONT_DECLARE(am_font_16) LV_FONT_DECLARE(am_font_18) \
  LV_FONT_DECLARE(am_font_24) LV_FONT_DECLARE(am_font_34)
```

- [ ] **Step 6: 构建 + 临时验证中文/符号能渲染**

在 `apple_music.c` 的内容区临时加一个 `lv_label`(`lv_obj_set_style_text_font(lbl,&am_font_14,0); lv_label_set_text(lbl,"主页 广播 ▶");`),构建运行确认中文与 ▶ 正常显示,然后删除临时 label。
Run: `cmake -S . -B build && cmake --build build -j"$(nproc)" && ./bin/main`
Expected: 窗口里中文与符号清晰渲染(无方框/缺字)。

- [ ] **Step 7: 提交**
```bash
git add main/src/v9_apple_music/fonts/ main/src/v9_apple_music/am_fonts.h lv_conf.h
git commit -m "feat(music): add subset CJK+latin+symbol fonts (8 sizes)"
```

---

## Task 5: `am_icons.h` 图标宏

**Files:** Create `main/src/v9_apple_music/am_icons.h`

- [ ] **Step 1: 写头文件(UTF-8 字面量)**

```c
/* main/src/v9_apple_music/am_icons.h */
#ifndef AM_ICONS_H
#define AM_ICONS_H
#define AM_ICON_HOME      "\xE2\x8C\x82"  /* ⌂ */
#define AM_ICON_RADIO     "\xE2\x97\x89"  /* ◉ */
#define AM_ICON_LOCAL     "\xE2\x99\xAB"  /* ♫ */
#define AM_ICON_PLAYLIST  "\xE2\x89\xA3"  /* ≣ */
#define AM_ICON_SETTINGS  "\xE2\x9A\x99"  /* ⚙ */
#define AM_ICON_BRAND     "\xE2\x99\xAA"  /* ♪ */
#define AM_ICON_SEARCH    "\xE2\x8C\x95"  /* ⌕ */
#define AM_ICON_SEARCH2   "\xE2\x97\x8C"  /* ◌ */
#define AM_ICON_PLAY      "\xE2\x96\xB6"  /* ▶ */
#define AM_ICON_PREV      "\xE2\x8F\xAE"  /* ⏮ */
#define AM_ICON_NEXT      "\xE2\x8F\xAD"  /* ⏭ */
#define AM_ICON_REPLAY    "\xE2\x86\xBA"  /* ↺ */
#endif
```

- [ ] **Step 2: 构建确认无误**(头文件被后续引用;此处仅 `cmake --build build` 确认不破坏构建)

- [ ] **Step 3: 提交**
```bash
git add main/src/v9_apple_music/am_icons.h
git commit -m "feat(music): add am_icons glyph macros"
```

---

## Task 6: `am_widgets` 可复用工厂(视觉)

**Files:** Create `main/src/v9_apple_music/am_widgets.h`, `am_widgets.c`

依据 spec §6 的尺寸/色值与 §5 的 token。提供渐变助手 + 工厂函数。

- [ ] **Step 1: 写 `am_widgets.h`(完整签名)**

```c
/* main/src/v9_apple_music/am_widgets.h */
#ifndef AM_WIDGETS_H
#define AM_WIDGETS_H
#include "lvgl/lvgl.h"

/* 竖向 2 段渐变(by-value 存色,无生命周期问题):top -> bottom */
void       am_fill_grad2(lv_obj_t *o, lv_color_t top, lv_color_t bottom);
/* 竖向 3 段渐变(hero/播放头像/色卡预览):持久 dsc,随对象删除自动释放 */
void       am_fill_grad3(lv_obj_t *o, lv_color_t c1, lv_color_t c2, lv_color_t c3);
/* 玻璃面板:radius22 + surface(白62%) + 1px 白边 + 柔阴影 */
lv_obj_t  *am_panel(lv_obj_t *parent);
/* 卡片:radius20 + 白56% */
lv_obj_t  *am_card(lv_obj_t *parent, int radius, lv_opa_t bg_opa);
/* 胶囊标签 */
lv_obj_t  *am_pill(lv_obj_t *parent, const char *text, bool active);
/* 小标题(10px/800 muted 大写字距) */
lv_obj_t  *am_section_title(lv_obj_t *parent, const char *text);
/* 封面块:size×size,radius16,tile_a→tile_c 竖向渐变 + 两个波纹白圆 */
lv_obj_t  *am_cover(lv_obj_t *parent, int size, lv_color_t a, lv_color_t b);
/* 圆形动作按钮(accent@14% 底 + accent 图标) */
lv_obj_t  *am_item_action(lv_obj_t *parent, const char *glyph);
/* 侧栏导航项;active 时 accent 高亮;返回按钮对象,cb 由调用方挂 */
lv_obj_t  *am_nav_item(lv_obj_t *parent, const char *icon, const char *label, bool active);
/* 标签文本助手:设字号与颜色 */
lv_obj_t  *am_text(lv_obj_t *parent, const char *txt, const lv_font_t *font, lv_color_t color);

#endif
```

- [ ] **Step 2: 写 `am_widgets.c` —— 渐变助手 + 封面(波纹)+ nav_item(关键模式,完整)**

```c
#include "am_widgets.h"
#include "am_theme.h"
#include "am_fonts.h"

/* 2 段:用 by-value 样式 API,颜色按值存入样式,无指针生命周期问题 */
void am_fill_grad2(lv_obj_t *o, lv_color_t top, lv_color_t bottom)
{
    lv_obj_set_style_bg_color(o, top, 0);
    lv_obj_set_style_bg_grad_color(o, bottom, 0);
    lv_obj_set_style_bg_grad_dir(o, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
}

/* 3 段:lv_grad_dsc_t 按指针被样式引用,必须持久;此处每对象 malloc 一份,
   并在对象删除时释放,避免悬空指针与多对象共享同一渐变 */
static void am_free_grad_cb(lv_event_t *e){ lv_free(lv_event_get_user_data(e)); }
void am_fill_grad3(lv_obj_t *o, lv_color_t c1, lv_color_t c2, lv_color_t c3)
{
    lv_grad_dsc_t *d = lv_malloc(sizeof(lv_grad_dsc_t));
    lv_memzero(d, sizeof(*d));
    d->dir = LV_GRAD_DIR_VER; d->stops_count = 3;
    d->stops[0].color = c1; d->stops[0].frac = 0;   d->stops[0].opa = LV_OPA_COVER;
    d->stops[1].color = c2; d->stops[1].frac = 108; d->stops[1].opa = LV_OPA_COVER;  /* ~42% */
    d->stops[2].color = c3; d->stops[2].frac = 255; d->stops[2].opa = LV_OPA_COVER;
    lv_obj_set_style_bg_grad(o, d, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(o, am_free_grad_cb, LV_EVENT_DELETE, d);
}
```
> 选型:封面/侧栏 tint/banner/进度条 fill 等用 `am_fill_grad2`;hero 卡、播放头像、主播放键、色卡预览这类需要三段(`hero_a/b/c`)的用 `am_fill_grad3`。

封面(含 cover-wave 两个白圆,绝对定位):
```c
lv_obj_t *am_cover(lv_obj_t *parent, int size, lv_color_t a, lv_color_t b)
{
    lv_obj_t *cov = lv_obj_create(parent);
    lv_obj_remove_style_all(cov);
    lv_obj_set_size(cov, size, size);
    lv_obj_set_style_radius(cov, 16, 0);
    lv_obj_set_style_clip_corner(cov, true, 0);
    am_fill_grad2(cov, a, b);                        /* tile_a -> tile_c */
    /* ::before 大圆 左下 */
    lv_obj_t *w1 = lv_obj_create(cov); lv_obj_remove_style_all(w1);
    lv_obj_set_size(w1, 54, 54); lv_obj_set_style_radius(w1, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(w1, AM_WHITE, 0); lv_obj_set_style_bg_opa(w1, 148, 0);
    lv_obj_align(w1, LV_ALIGN_BOTTOM_LEFT, -8, 24);
    /* ::after 小圆 右上 */
    lv_obj_t *w2 = lv_obj_create(cov); lv_obj_remove_style_all(w2);
    lv_obj_set_size(w2, 24, 24); lv_obj_set_style_radius(w2, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(w2, AM_WHITE, 0); lv_obj_set_style_bg_opa(w2, 148, 0);
    lv_obj_align(w2, LV_ALIGN_TOP_RIGHT, 8, 8);
    return cov;
}
```

nav_item(spec §6.1:h42 r16 pad0/12 gap10;icon 28×28 r12 白60%;label 14/700;active=accent 文字+accent@12%底+内描边):
```c
lv_obj_t *am_nav_item(lv_obj_t *parent, const char *icon, const char *label, bool active)
{
    const am_theme_t *t = am_theme_get(am_theme_current());
    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, LV_PCT(100), 42);
    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(btn, 12, 0);
    lv_obj_set_style_pad_column(btn, 10, 0);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    if(active){
        lv_obj_set_style_bg_color(btn, t->accent, 0);
        lv_obj_set_style_bg_opa(btn, AM_OPA_ACCENT_12, 0);
    }
    lv_obj_t *ic = lv_obj_create(btn); lv_obj_remove_style_all(ic);
    lv_obj_set_size(ic, 28, 28); lv_obj_set_style_radius(ic, 12, 0);
    lv_obj_set_style_bg_color(ic, AM_WHITE, 0); lv_obj_set_style_bg_opa(ic, 153, 0);
    lv_obj_t *icl = lv_label_create(ic); lv_label_set_text(icl, icon);
    lv_obj_center(icl); lv_obj_set_style_text_font(icl, &am_font_14, 0);
    lv_obj_set_style_text_color(icl, active ? t->accent : AM_MUTED_STRONG, 0);
    lv_obj_t *lbl = lv_label_create(btn); lv_label_set_text(lbl, label);
    lv_obj_set_style_text_font(lbl, &am_font_14, 0);
    lv_obj_set_style_text_color(lbl, active ? t->accent : AM_MUTED_STRONG, 0);
    return btn;
}
```

- [ ] **Step 3: 实现其余工厂**

`am_panel`/`am_card`/`am_pill`/`am_section_title`/`am_item_action`/`am_text` 按相同模式(`lv_obj_create`+`lv_obj_remove_style_all`+设 radius/bg_color/bg_opa/pad/font/color)实现,数值取 spec §6 与 §5:
- `am_panel`:radius22、bg=AM_WHITE/AM_OPA_SURFACE、`lv_obj_set_style_border_color(.,AM_WHITE,0)`+`border_opa=AM_OPA_BORDER`+`border_width=1`、阴影 `shadow_width≈30 shadow_ofs_y≈12 shadow_color=0x363446 shadow_opa≈26`。
- `am_pill`:radius999、active 时 accent@12% 底 + accent 文字,否则白70% 底 + muted_strong;font am_font_12;pad 9/12。
- `am_item_action`:30×30 圆、accent@14% 底、accent 图标、font am_font_12。
- `am_section_title`:font am_font_11(近似 10px)、color AM_MUTED、`lv_obj_set_style_text_letter_space(.,2,0)`。

- [ ] **Step 4: 临时拼装验证**

在 `apple_music.c` 内容区临时放一行 nav_item、一个 panel、一个 cover、几个 pill,构建运行,与 mockup 对照外观(圆角/阴影/渐变/颜色)。确认后删除临时代码。
Run: `cmake --build build -j"$(nproc)" && ./bin/main`
Expected: 各组件外观接近 mockup。

- [ ] **Step 5: 提交**
```bash
git add main/src/v9_apple_music/am_widgets.*
git commit -m "feat(music): add am_widgets factories (panel/card/pill/cover/nav_item/...)"
```

---

## Task 7: `am_shell` 侧栏 + 迷你播放条(视觉)

**Files:** Create `main/src/v9_apple_music/am_shell.h`, `am_shell.c`;Modify `apple_music.c`(调用 shell 构造,移除占位 bg)

- [ ] **Step 1: 写 `am_shell.h`**

```c
/* main/src/v9_apple_music/am_shell.h */
#ifndef AM_SHELL_H
#define AM_SHELL_H
#include "lvgl/lvgl.h"
/* 导航点击回调:传出被点项索引(0..4 对应 nav_items) */
typedef void (*am_nav_cb_t)(int nav_index, void *user);
/* 在 sidebar 容器内构建侧栏;nav 点击回调用于切页 */
void am_shell_build_sidebar(lv_obj_t *sidebar, int active_nav, am_nav_cb_t cb, void *user);
/* 在 player 容器内构建迷你播放条 */
void am_shell_build_miniplayer(lv_obj_t *player);
#endif
```

- [ ] **Step 2: 写 `am_shell.c`**

按 spec §6.1 装配侧栏:`am_fill_grad2` 做 sidebar-tint(近白竖向)→ 顶边 padding(18/14/18/16,flex 列 gap14)→ brand(badge 36×36 r14 hero 渐变 + `AM_ICON_BRAND` + 文案两行)→ `am_section_title("导航")` → 4 个 `am_nav_item`(i==active 高亮,每个 `lv_obj_add_event_cb(item, on_nav, LV_EVENT_CLICKED, ...)`,用 `lv_obj_set_user_data(item,(void*)(intptr_t)i)`,回调里取 index 调 `cb`)→ flex-grow 空白 → footer(顶边线:`lv_obj_set_style_border_side(.,LV_BORDER_SIDE_TOP,0)` + `am_section_title("偏好")` + 设置 nav_item(index 4)+ listener 卡:`am_card` 内三段文本)。
迷你播放条按 spec §6.5:player 容器设 grid `[1.15FR,auto,1FR]`,gap16,pad12/20/14/14;三格:now-playing(`am_cover` 52 或 hero 渐变小块 + 标题/副标)、控制键(`am_item_action`/圆按钮:⏮ + 主播放 42×42 hero 渐变 + ⏭)、progress-cluster(progress-meta 三段文本 + progress-bar:rail 底 6px r999 + 子对象宽 44% 的 accent 渐变 fill)。
文案全部取 `am_mini` / 共享色;字号见 spec §7.1。

- [ ] **Step 3: `apple_music.c` 调用 shell**

把 Task 1 占位区的 bg 设置替换为:sidebar 调 `am_shell_build_sidebar(sidebar, /*active*/0, on_nav, NULL)`;player 调 `am_shell_build_miniplayer(player)`。`on_nav` 暂时 `LV_LOG_USER` 打印 index(Task 11 接真正切页)。

- [ ] **Step 4: 构建 + 视觉验证**

Run: `cmake --build build -j"$(nproc)" && ./bin/main`
Expected:侧栏(品牌/导航 4 项/偏好+设置/listener 卡)与底部迷你播放条与 mockup 一致(mint 主题);点击导航项控制台打印 index。

- [ ] **Step 5: 提交**
```bash
git add main/src/v9_apple_music/am_shell.* main/src/v9_apple_music/apple_music.c
git commit -m "feat(music): build persistent sidebar + mini-player shell"
```

---

## Task 8: `am_page_home` 主页(视觉,可与 9/10 并行)

**Files:** Create `main/src/v9_apple_music/am_page_home.h`, `am_page_home.c`

接口:`lv_obj_t *am_page_home_create(lv_obj_t *content_parent);`(返回 page 根,调用方负责挂载/销毁)。

- [ ] **Step 1: 写 `.h`**(签名如上)
- [ ] **Step 2: 写 `.c`** 按 spec §6.2 + app.js `renderHomePage`(189-255):
  - page 容器:flex 列 gap16,`lv_obj_set_height(page, LV_PCT(100))`
  - page-header:flex 行 space-between;左 = `am_eyebrow`(此处用 `am_pill` 变体:dot 圆 + "广播优先首页")+ title `am_text("主页",&am_font_34,AM_TEXT)` + subtitle `am_font_13/AM_MUTED`;右 = search-chip(`am_card` 圆角 + `AM_ICON_SEARCH` + 文案)
  - home-grid:grid 列 `[LV_GRID_FR(125), LV_GRID_FR(92)]` gap16
    - **hero-card**:`am_panel` 改 radius28 + `am_fill_grad3(hero, hero_a, hero_b, hero_c)` + 白字;hero-copy(topline 12/700 大写、title `am_font_24` 白、desc 12);`hero-actions-docked` 用 `lv_obj_align(.,LV_ALIGN_BOTTOM_LEFT,..)` 放两个按钮(继续收听 = 白底深字、探索更多 = 白16%底白字);floating-pill `LV_ALIGN_TOP_RIGHT`;hero-visual(右下:vinyl-ring = 描边圆 + vinyl-core 白圆;径向高光 = 半透明白圆 + `shadow_spread`)
    - side-stack:flex 列 gap14 → recommend-panel(`am_panel`:header + 3× recommend-item)+ recent-panel(header + 3× recent-item + recent-metrics 3 列 `am_card`)
  - recommend-item:flex/grid `[44,FR1,auto]`:`am_cover(44, tile_a, tile_c)` + 标题/副标 + `am_item_action(AM_ICON_PLAY)`;recent-item 同构,action 用 `AM_ICON_REPLAY`,副标=`kind · subtitle`
- [ ] **Step 3: 临时挂载验证**:在 `apple_music.c` 内容区调 `am_page_home_create(content)`,构建运行,对照 mockup 主页。
  Run: `cmake --build build -j"$(nproc)" && ./bin/main`
  Expected:主页 hero/推荐/最近播放/指标与 mockup 一致。
- [ ] **Step 4: 提交**
```bash
git add main/src/v9_apple_music/am_page_home.*
git commit -m "feat(music): implement home page"
```

---

## Task 9: `am_page_list` 列表页模板(视觉,可与 8/10 并行)

**Files:** Create `main/src/v9_apple_music/am_page_list.h`, `am_page_list.c`

接口:`lv_obj_t *am_page_list_create(lv_obj_t *content_parent, const am_list_page_t *data);`

- [ ] **Step 1: 写 `.h`**
- [ ] **Step 2: 写 `.c`** 按 spec §6.3 + app.js `renderListPage`(283-345):
  - page-header(同主页结构,文案来自 `data->eyebrow/title/subtitle`)
  - feature-banner:grid `[FR120,FR80]` pad18 r26 + `am_fill_grad2(banner, accent, accent_soft)`(近似 accent 90%→58% 的明度过渡)白字;左 = h3(`data->banner_title` 24)+ p(`banner_desc`)+ 3× `am_pill`(badges,白18%底);右 = 2× feature-cardlet(`am_card` 白16%:queue_label / "视觉重点")
  - content-grid:grid `[FR1, 244]` gap14
    - catalog-panel(`am_panel`):header(`{title} 列表` + "4 项假数据")+ 4× media-item:`[44,FR1,auto]`;封面色按 `kind`(album=#dce8ff→#b3c6ff、radio=#f8d9ec→#ffd2cf、playlist=#d3f8f0→#d7ecff、song/default=tile_a→tile_b);meta span + aux 标签
    - queue-panel(`am_panel` 244 宽):headline + `am_pill`(queue_label,accent@12%)+ 3× queue-item(`am_card`:strong+span)
- [ ] **Step 3: 临时验证**(内容区调 `am_page_list_create(content, &am_page_radio)`,再分别试 local/playlist)
  Expected:三套数据下列表页与 mockup 对应页一致;封面色随 kind 变化。
- [ ] **Step 4: 提交**
```bash
git add main/src/v9_apple_music/am_page_list.*
git commit -m "feat(music): implement shared list page (radio/local/playlist)"
```

---

## Task 10: `am_page_settings` 设置页(视觉,可与 8/9 并行)

**Files:** Create `main/src/v9_apple_music/am_page_settings.h`, `am_page_settings.c`

接口:
```c
typedef void (*am_theme_pick_cb_t)(int theme_index, void *user);
typedef void (*am_tab_pick_cb_t)(int tab_index, void *user);
lv_obj_t *am_page_settings_create(lv_obj_t *content_parent, int active_tab,
                                  am_theme_pick_cb_t on_theme, am_tab_pick_cb_t on_tab, void *user);
```

- [ ] **Step 1: 写 `.h`**
- [ ] **Step 2: 写 `.c`** 按 spec §6.4 + app.js `renderSettingsPage`/`renderSettingsPanel`(363-495):
  - page-header(eyebrow "Appearance Controls" + "设置" + subtitle + search-chip `AM_ICON_SEARCH2`)
  - settings-layout:grid `[180,FR1]` gap14
    - settings-sidebar(`am_panel`):3× settings-tab(`am_card`,active 高亮;`lv_obj_add_event_cb` → `on_tab(index)`)
    - settings-main(`am_panel`):按 `active_tab` 渲染:
      - **appearance**:卡"主题模式"(3 `am_pill`:浅色 active)+ 卡"主题颜色"(theme-swatches:4 列,每个 swatch = `am_card` 内 swatch-preview(h42 r14,该主题 `am_fill_grad3(prev, hero_a, hero_b, hero_c)`)+ label + desc;active=当前主题,内描边;`lv_obj_add_event_cb`→`on_theme(index)`)+ settings-split(2 卡:圆角强度 = slider(rail h8 + 62% accent fill)+ "62%";背景氛围 = 2 pill)+ about-card(当前主题名,取 `am_theme_presets[current].label`)
      - **playback**:卡"播放占位"(3 pill)+ 卡"队列策略" + about-card
      - **about**:卡"关于样机" + 卡"设备假设" + about-card
- [ ] **Step 3: 临时验证**(内容区调 `am_page_settings_create(content,0,...,NULL)`,点 tab/swatch 控制台打印 index)
  Expected:三个 tab 内容与 mockup 一致;swatch 预览色正确。
- [ ] **Step 4: 提交**
```bash
git add main/src/v9_apple_music/am_page_settings.*
git commit -m "feat(music): implement settings page (3 tabs)"
```

---

## Task 11: 控制器接线 —— 状态机与切换(交互)

**Files:** Modify `main/src/v9_apple_music/apple_music.c`(+ 新增内部状态)

- [ ] **Step 1: 在 `apple_music.c` 顶部加状态与内容挂载点**

```c
#include "am_data.h"
#include "am_theme.h"
#include "am_shell.h"
#include "am_page_home.h"
#include "am_page_list.h"
#include "am_page_settings.h"

typedef enum { AM_PAGE_HOME, AM_PAGE_RADIO, AM_PAGE_LOCAL, AM_PAGE_PLAYLIST, AM_PAGE_SETTINGS } am_page_e;
static am_page_e  s_page = AM_PAGE_HOME;
static int        s_settings_tab = 0;
static lv_obj_t  *s_root, *s_sidebar, *s_content, *s_player;

/* 前向声明:rebuild_content/build_all 会引用这三个回调 */
static void on_nav(int nav_index, void *u);
static void on_theme_pick(int theme_index, void *u);
static void on_tab_pick(int tab_index, void *u);
static void rebuild_content(void);
static void build_all(void);
```

> 根 grid 的 `col_dsc`/`row_dsc`(Task 1 已在文件作用域定义为 `static const`)在此复用。

- [ ] **Step 2: 抽出 `build_all()` 与 `rebuild_content()`**

```c
static void rebuild_content(void)
{
    lv_obj_clean(s_content);
    switch(s_page){
        case AM_PAGE_HOME:     am_page_home_create(s_content); break;
        case AM_PAGE_RADIO:    am_page_list_create(s_content, &am_page_radio); break;
        case AM_PAGE_LOCAL:    am_page_list_create(s_content, &am_page_local); break;
        case AM_PAGE_PLAYLIST: am_page_list_create(s_content, &am_page_playlist); break;
        case AM_PAGE_SETTINGS: am_page_settings_create(s_content, s_settings_tab,
                                   on_theme_pick, on_tab_pick, NULL); break;
    }
}
/* 整壳重建:首建与换肤都走它(创建三区 + 装配 shell + 内容) */
static void build_all(void)
{
    const am_theme_t *t = am_theme_get(am_theme_current());
    s_root = lv_screen_active();
    lv_obj_clean(s_root);
    lv_obj_remove_style_all(s_root);
    am_fill_grad2(s_root, t->bg_top, t->bg_bottom);          /* 壳背景 */
    lv_obj_set_grid_dsc_array(s_root, col_dsc, row_dsc);
    lv_obj_set_style_pad_all(s_root, 0, 0);
    lv_obj_set_style_pad_gap(s_root, 0, 0);

    s_sidebar = lv_obj_create(s_root); lv_obj_remove_style_all(s_sidebar);
    lv_obj_set_grid_cell(s_sidebar, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
    s_content = lv_obj_create(s_root); lv_obj_remove_style_all(s_content);
    lv_obj_set_style_pad_left(s_content, 20, 0); lv_obj_set_style_pad_right(s_content, 20, 0);
    lv_obj_set_style_pad_top(s_content, 20, 0);
    lv_obj_set_grid_cell(s_content, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    s_player = lv_obj_create(s_root); lv_obj_remove_style_all(s_player);
    lv_obj_set_grid_cell(s_player, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    am_shell_build_sidebar(s_sidebar, (int)s_page, on_nav, NULL);
    am_shell_build_miniplayer(s_player);
    rebuild_content();
}
```

- [ ] **Step 3: 三个回调**

```c
static void on_nav(int nav_index, void *u){               /* 切页:重建内容 + 刷新侧栏 active */
    LV_UNUSED(u);
    s_page = (am_page_e)nav_index;
    lv_obj_clean(s_sidebar);                              /* 先清空再重建以更新 active */
    am_shell_build_sidebar(s_sidebar, nav_index, on_nav, NULL);
    rebuild_content();
}
static void on_theme_pick(int theme_index, void *u){      /* 换肤:整壳重建 */
    LV_UNUSED(u);
    am_theme_set((am_theme_id_t)theme_index);             /* 0..3 = cyan/blue/mint/orange */
    build_all();
}
static void on_tab_pick(int tab_index, void *u){          /* 换 tab:仅重建设置主区 */
    LV_UNUSED(u);
    s_settings_tab = tab_index;
    rebuild_content();
}
```

- [ ] **Step 4: `apple_music_create` 改为只调 `build_all()`**(取代 Task 1/7 在 `apple_music_create` 里的内联骨架与 shell 装配——那些逻辑现已收敛进 `build_all`)

```c
void apple_music_create(void){ build_all(); }
```

- [ ] **Step 5: 构建 + 交互验证**

Run: `cmake --build build -j"$(nproc)" && ./bin/main`
Expected:
- 点左栏 主页/广播/本地/歌单/设置 → 右侧内容切换,侧栏高亮跟随,侧栏与播放条不闪烁丢失。
- 设置→外观→点 4 个主题色卡 → **整界面**(侧栏 tint/高亮、hero、播放条、进度、按钮)主题色变化。
- 设置→点 外观/播放/关于 tab → 仅右侧 settings-main 变化。

- [ ] **Step 6: 提交**
```bash
git add main/src/v9_apple_music/apple_music.c
git commit -m "feat(music): wire state machine (nav switch / theme switch / settings tabs)"
```

---

## Task 12: 全量视觉验证与打磨

**Files:** 按需 Modify 各 `am_page_*` / `am_widgets` / `am_shell`

- [ ] **Step 1: 截图比对矩阵**

对 5 页(主页/广播/本地/歌单/设置-外观/设置-播放/设置-关于)× 4 主题,运行 `./bin/main` 切换并截图,与浏览器打开 `index.html`(切对应 `data-theme` 与页面)逐一对照。可用 `verify` 技能。

- [ ] **Step 2: 记录并修正差异**

重点:间距(padding/gap)、圆角、字号(spec §7.1)、accent/hero 色、阴影强度、封面波纹位置、进度条 44%/圆角强度 62%、文本超长省略(`lv_label_set_long_mode(.,LV_LABEL_LONG_DOT)`)。逐项修到位。

- [ ] **Step 3: 回归**

Run: `cd build && ctest -R apple_music --output-on-failure`(逻辑测试仍绿)+ `./bin/main` 全流程点一遍。
Expected:测试通过;各页各主题与 mockup 视觉一致。

- [ ] **Step 4: 提交**
```bash
git add -A main/src/v9_apple_music/
git commit -m "fix(music): pixel polish to match HTML mockup across pages/themes"
```

---

## 完成标准

- [ ] `./bin/main` 在 800×480 启动,默认 mint 主题主页。
- [ ] 5 个导航页全部实现,切页只刷新内容区。
- [ ] 4 套主题切换实时生效(侧栏/hero/播放条/进度/按钮联动)。
- [ ] 设置页 3 tab 切换正常。
- [ ] 中文与图标字形正常渲染(子集字体)。
- [ ] `music_player.c` 未被改动;`main.c` 旧调用以注释保留。
- [ ] `ctest -R apple_music` 全绿。
- [ ] 各页各主题与 HTML 样机视觉一致(目检/截图比对)。
