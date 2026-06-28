# macOS 音乐播放器 LVGL 复刻 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 用 LVGL 复刻 `design-ui/music/music-player-macos.html`，替换 desktop 下 `Apple Music` 图标当前播放器，并接通本地文件与广播真实播放。

**Architecture:** 保留 `Apple Music -> apple_music_app_launch -> apple_music_create_in` 入口，重做 `main/src/v9_apple_music` 的 UI 与运行态，新增 CSV/本地扫描模块提供真实列表数据，底部控制条继续走 `am_player -> music_player -> player_controller/stream_player`。主线程负责 GitNexus impact、集成、验证；实现使用单个 worker 保持写面集中。

**Tech Stack:** C99, LVGL v9, SDL, CMake, GitNexus, `third-party/hls_player_demo`

## Global Constraints

- 所有 markdown 文档使用中文。
- shell 命令统一走 `rtk`。
- 改任一函数、类、方法前必须先跑 `gitnexus impact` 并报告 blast radius。
- 若 impact 风险为 `HIGH` 或 `CRITICAL`，必须先提示风险再改。
- desktop 入口保持 `Apple Music` 图标不变。
- 本地音乐源固定使用 `third-party/hls_player_demo/test_file`。
- 广播源固定使用 `third-party/hls_player_demo/qa/production_test/config/sources.csv`。
- 不回退或覆盖用户已有改动。

---

### Task 1: 建立真实数据输入层

**Files:**
- Create: `main/src/v9_apple_music/am_sources_csv.c`
- Create: `main/src/v9_apple_music/am_sources_csv.h`
- Create: `main/src/v9_apple_music/am_local_scan.c`
- Create: `main/src/v9_apple_music/am_local_scan.h`
- Modify: `CMakeLists.txt`
- Test: `main/tests/apple_music_data_test.c`

**Interfaces:**
- Produces: `int am_sources_csv_load(const char *path, am_radio_item_t **items, size_t *count);`
- Produces: `void am_sources_csv_free(am_radio_item_t *items);`
- Produces: `int am_local_scan_dir(const char *dir, am_local_item_t **items, size_t *count);`
- Produces: `void am_local_scan_free(am_local_item_t *items);`

- [ ] **Step 1: 写失败测试，覆盖 CSV 解析与本地目录扫描**

```c
/* main/tests/apple_music_data_test.c */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "../src/v9_apple_music/am_sources_csv.h"
#include "../src/v9_apple_music/am_local_scan.h"

int main(void)
{
    am_radio_item_t *radio = NULL;
    am_local_item_t *local = NULL;
    size_t radio_count = 0;
    size_t local_count = 0;

    int rc_radio = am_sources_csv_load(
        "third-party/hls_player_demo/qa/production_test/config/sources.csv",
        &radio,
        &radio_count
    );
    assert(rc_radio == 0);
    assert(radio != NULL);
    assert(radio_count > 0);
    assert(radio[0].title[0] != '\0');
    assert(radio[0].url[0] != '\0');

    int rc_local = am_local_scan_dir(
        "third-party/hls_player_demo/test_file",
        &local,
        &local_count
    );
    assert(rc_local == 0);
    assert(local != NULL);
    assert(local_count > 0);
    assert(local[0].path[0] != '\0');
    assert(local[0].title[0] != '\0');

    am_sources_csv_free(radio);
    am_local_scan_free(local);
    return 0;
}
```

- [ ] **Step 2: 运行测试，确认当前失败**

Run: `rtk ctest --test-dir build -R apple_music_data_test --output-on-failure`

Expected: FAIL，提示找不到测试目标或缺少实现。

- [ ] **Step 3: 实现 CSV 与本地扫描模块**

```c
/* am_sources_csv.h */
typedef struct {
    char title[128];
    char url[1024];
    uint32_t duration_ms;
    uint32_t network_cache_ms;
} am_radio_item_t;

int am_sources_csv_load(const char *path, am_radio_item_t **items, size_t *count);
void am_sources_csv_free(am_radio_item_t *items);
```

```c
/* am_local_scan.h */
typedef struct {
    char path[1024];
    char title[256];
    bool favorite;
} am_local_item_t;

int am_local_scan_dir(const char *dir, am_local_item_t **items, size_t *count);
void am_local_scan_free(am_local_item_t *items);
```

- [ ] **Step 4: 接入 CMake 测试目标**

```cmake
add_executable(apple_music_data_test
    ${PROJECT_SOURCE_DIR}/main/tests/apple_music_data_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_sources_csv.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_local_scan.c
)
target_include_directories(apple_music_data_test PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/main/src
    ${HLS_PROJECT_ROOT_DIR}/src/stream_player/include
)
target_link_libraries(apple_music_data_test lvgl ${PNG_LIBRARIES})
```

- [ ] **Step 5: 运行测试，确认通过**

Run: `rtk ctest --test-dir build -R apple_music_data_test --output-on-failure`

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add main/src/v9_apple_music/am_sources_csv.c main/src/v9_apple_music/am_sources_csv.h main/src/v9_apple_music/am_local_scan.c main/src/v9_apple_music/am_local_scan.h main/tests/apple_music_data_test.c CMakeLists.txt
git commit -m "feat: add apple music data loaders"
```

### Task 2: 重做 macOS 风格 UI 壳与运行态

**Files:**
- Modify: `main/src/v9_apple_music/apple_music.c`
- Modify: `main/src/v9_apple_music/am_shell.c`
- Modify: `main/src/v9_apple_music/am_shell.h`
- Modify: `main/src/v9_apple_music/am_data.c`
- Modify: `main/src/v9_apple_music/am_data.h`
- Modify: `main/src/v9_apple_music/am_page_list.c`
- Modify: `main/src/v9_apple_music/am_widgets.c`
- Test: `main/tests/apple_music_parent_size_test.c`

**Interfaces:**
- Consumes: `am_sources_csv_load(...)`
- Consumes: `am_local_scan_dir(...)`
- Produces: `void apple_music_create_in(lv_obj_t *parent);`
- Produces: `am_miniplayer_handles_t am_shell_build_miniplayer(lv_obj_t *player);`

- [ ] **Step 1: 写失败测试，确保父容器尺寸和子结构仍成立**

```c
/* main/tests/apple_music_parent_size_test.c */
assert(lv_obj_get_width(parent) == 800);
assert(lv_obj_get_height(parent) == 480);
assert(lv_obj_get_child_count(parent) >= 3);
```

- [ ] **Step 2: 运行测试，确认重构前基线**

Run: `rtk ctest --test-dir build -R apple_music_parent_size_test --output-on-failure`

Expected: PASS

- [ ] **Step 3: 重做界面布局与 view 切换**

```c
/* apple_music.c 关键结构 */
typedef enum {
    AM_VIEW_NOW = 0,
    AM_VIEW_RADIO,
    AM_VIEW_FAVORITES,
} am_view_t;

typedef struct {
    am_view_t view;
    bool playlist_open;
    size_t current_radio;
    size_t current_local;
} am_app_state_t;
```

- [ ] **Step 4: 在 `am_shell.c` 构建 sidebar、header、player bar、playlist popup**

```c
/* am_shell.h */
typedef struct {
    lv_obj_t *title_label;
    lv_obj_t *subtitle_label;
    lv_obj_t *time_cur;
    lv_obj_t *time_total;
    lv_obj_t *progress_fill;
    lv_obj_t *play_icon;
    lv_obj_t *playlist_popup;
    lv_obj_t *playlist_list;
} am_miniplayer_handles_t;
```

- [ ] **Step 5: 更新 `am_data`，保留仅视觉常量，不再承载真实广播/本地列表**

```c
/* am_data.h */
extern const char *am_sidebar_recent_titles[4];
extern const char *am_mock_lyrics[4];
```

- [ ] **Step 6: 更新 `am_page_list.c`，支持广播/收藏两种动态列表渲染**

```c
void am_page_list_build_radio(lv_obj_t *parent,
                              const am_radio_item_t *items,
                              size_t count,
                              size_t current_index);

void am_page_list_build_favorites(lv_obj_t *parent,
                                  const am_local_item_t *items,
                                  size_t count,
                                  size_t current_index);
```

- [ ] **Step 7: 运行父容器测试**

Run: `rtk ctest --test-dir build -R apple_music_parent_size_test --output-on-failure`

Expected: PASS

- [ ] **Step 8: Commit**

```bash
git add main/src/v9_apple_music/apple_music.c main/src/v9_apple_music/am_shell.c main/src/v9_apple_music/am_shell.h main/src/v9_apple_music/am_data.c main/src/v9_apple_music/am_data.h main/src/v9_apple_music/am_page_list.c main/src/v9_apple_music/am_widgets.c main/tests/apple_music_parent_size_test.c
git commit -m "feat: rebuild apple music macos ui"
```

### Task 3: 接通真实播放与广播切台逻辑

**Files:**
- Modify: `main/src/v9_apple_music/am_player.c`
- Modify: `main/src/v9_apple_music/am_player.h`
- Modify: `main/src/v9_apple_music/apple_music.c`
- Modify: `main/inc/music_player.h`
- Modify: `main/src/local_music_demo/music_player.c`
- Test: `main/tests/desktop_apple_music_launcher_test.c`

**Interfaces:**
- Consumes: `music_player_init(...)`
- Consumes: `music_player_play() / pause() / next() / prev() / select()`
- Produces: `void am_player_play_radio_index(size_t index);`
- Produces: `void am_player_play_local_index(size_t index);`
- Produces: `void am_player_bind_playlist(...)`

- [ ] **Step 1: 写失败测试，确保 Apple Music launcher 仍能打开关闭**

```c
/* main/tests/desktop_apple_music_launcher_test.c */
assert(desktop_app_launcher_is_open());
assert(g_last_parent != NULL);
assert(lv_obj_get_width(g_last_parent) == 800);
assert(lv_obj_get_height(g_last_parent) == 480);
```

- [ ] **Step 2: 运行 launcher 测试，确认当前通过**

Run: `rtk ctest --test-dir build -R desktop_apple_music_launcher_test --output-on-failure`

Expected: PASS

- [ ] **Step 3: 扩展 `am_player`，让广播上一首/下一首走 UI 层索引**

```c
typedef enum {
    AM_SOURCE_NONE = 0,
    AM_SOURCE_LOCAL,
    AM_SOURCE_RADIO,
} am_source_kind_t;
```

- [ ] **Step 4: 如现有 `music_player` 接口不足，补最小查询接口**

```c
const char *music_player_get_title(size_t index);
bool music_player_is_live(size_t index);
```

- [ ] **Step 5: 在 `apple_music.c` 里绑定行点击、底部按钮、播放列表弹层**

```c
static void on_radio_click(size_t index);
static void on_local_click(size_t index);
static void on_playlist_toggle(lv_event_t *e);
```

- [ ] **Step 6: 运行 launcher 测试**

Run: `rtk ctest --test-dir build -R desktop_apple_music_launcher_test --output-on-failure`

Expected: PASS

- [ ] **Step 7: Commit**

```bash
git add main/src/v9_apple_music/am_player.c main/src/v9_apple_music/am_player.h main/src/v9_apple_music/apple_music.c main/inc/music_player.h main/src/local_music_demo/music_player.c main/tests/desktop_apple_music_launcher_test.c
git commit -m "feat: wire apple music playback"
```

### Task 4: 集成构建、运行验证、截图验证

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `main/src/main.c`
- Test: `main/tests/desktop_runtime_page_count_test.c`

**Interfaces:**
- Consumes: `apple_music_app_launch()`
- Consumes: `apple_music_create_in()`

- [ ] **Step 1: 确认新源文件进入主目标与测试目标**

```cmake
set(V9_APPLE_MUSIC_SOURCES
    ...
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_sources_csv.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_local_scan.c
)
```

- [ ] **Step 2: 配置并构建**

Run: `rtk cmake -S . -B build`

Expected: configure 成功

- [ ] **Step 3: 运行定向测试**

Run: `rtk ctest --test-dir build -R 'apple_music|desktop_apple_music|desktop_runtime_page_count' --output-on-failure`

Expected: PASS

- [ ] **Step 4: 运行应用，验证 Apple Music 入口**

Run: `rtk env AM_APP=apple_music ./bin/main`

Expected: 打开新 macOS 风格播放器

- [ ] **Step 5: 做无头截图**

Run: `rtk env SDL_VIDEODRIVER=offscreen AM_APP=apple_music AM_SHOT=/tmp/apple_music.ppm ./bin/main`

Expected: 生成 `/tmp/apple_music.ppm`

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt main/src/main.c main/tests/desktop_runtime_page_count_test.c
git commit -m "test: verify apple music desktop integration"
```
