# Apple Music 打开本地文件 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 `main/src/v9_apple_music` 增加桌面系统文件选择框入口，支持选择任意单个本地音频文件并在 Apple Music 页面内以临时会话态播放，同时补齐播放模式按钮视觉状态。

**Architecture:** 方案沿用现有 Apple Music 页面分层：`am_shell.c` 负责迷你播放器 UI，`am_player.c` 负责播放控制和播放列表弹层，`apple_music.c` 负责页面宿主状态。新增一个轻量桌面文件选择适配层，通过系统命令拉起文件选择框；单文件播放不并入固定扫描资料库，而是在 `apple_music.c` 和 `am_player.c` 内维护会话态。

**Tech Stack:** C99、LVGL、SDL2 桌面运行时、现有 `music_player` / `player_controller`、CMake + CTest

## Global Constraints

- 只支持单个本地音频文件，不支持多选。
- 不引入新的重量级 GUI 依赖，优先使用 `zenity`、`qarma`、`kdialog`。
- 单文件模式不写入 `apple_music_state.tsv`。
- 单文件模式不污染 `g_app.locals` 固定扫描资料库。
- 迷你播放器新增按钮后，按钮布局仍保持“上一首 / 播放 / 下一首 / 打开文件 / 播放列表”的单行结构。
- 播放模式按钮需要有可见的四态视觉反馈：顺序、单曲、列表、随机。
- 单文件模式下，`顺序播放`、`列表循环`、`随机播放` 都不得隐式跳回固定资料库。

---

## 文件边界

- `main/src/v9_apple_music/am_desktop_file_dialog.h`
  对外暴露桌面文件选择接口与结果枚举。
- `main/src/v9_apple_music/am_desktop_file_dialog.c`
  负责命令探测、执行、返回选中文件路径。
- `main/src/v9_apple_music/am_icons.h`
  新增“打开文件”按钮图标常量，优先复用现有字符集或 ASCII，避免字体资产回归。
- `main/src/v9_apple_music/am_shell.h`
  扩展 `am_miniplayer_handles_t` 和 `am_shell_build_miniplayer(lv_obj_t *player, am_simple_event_cb_t on_mode, am_simple_event_cb_t on_prev, am_simple_event_cb_t on_play, am_simple_event_cb_t on_next, am_simple_event_cb_t on_open_file, am_simple_event_cb_t on_playlist)` 回调参数。
- `main/src/v9_apple_music/am_shell.c`
  构建“打开本地文件”按钮，保留 `btn_mode` 可被 `am_player_refresh_ui()` 动态刷新。
- `main/src/v9_apple_music/am_player.h`
  暴露单文件模式控制与查询接口。
- `main/src/v9_apple_music/am_player.c`
  维护单文件模式状态、单文件标题、单文件播放列表弹层行为、模式按钮文本刷新。
- `main/src/v9_apple_music/apple_music.c`
  接入文件选择回调，保存宿主会话态，更新“正在播放”页。
- `main/tests/apple_music_desktop_file_dialog_test.c`
  覆盖文件选择适配层的成功、取消、不可用、失败路径。
- `main/tests/apple_music_single_file_player_test.c`
  覆盖 `am_player` 单文件模式、播放列表弹层和模式按钮刷新。
- `main/tests/apple_music_now_view_single_file_test.c`
  覆盖 `apple_music.c` 中按钮点击后的宿主页面同步。
- `CMakeLists.txt`
  注册新增测试目标和对应的 `add_test(NAME apple_music_desktop_file_dialog_test COMMAND $<TARGET_FILE:apple_music_desktop_file_dialog_test>)` 等测试入口。

### Task 1: 桌面文件选择适配层

**Files:**
- Create: `main/src/v9_apple_music/am_desktop_file_dialog.h`
- Create: `main/src/v9_apple_music/am_desktop_file_dialog.c`
- Create: `main/tests/apple_music_desktop_file_dialog_test.c`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: 无
- Produces: `typedef enum { AM_FILE_PICK_OK = 0, AM_FILE_PICK_CANCEL, AM_FILE_PICK_UNAVAILABLE, AM_FILE_PICK_ERROR } am_file_pick_result_t;`
- Produces: `am_file_pick_result_t am_desktop_file_dialog_pick_audio(char *path_buf, size_t path_buf_size);`

- [ ] **Step 1: 写失败测试**

```c
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/v9_apple_music/am_desktop_file_dialog.h"

static void write_script(const char *path, const char *body)
{
    FILE *fp = fopen(path, "w");
    fputs(body, fp);
    fclose(fp);
    chmod(path, 0755);
}

static void test_pick_audio_success_via_helper(void)
{
    char path_buf[1024] = {0};
    write_script("build/am_pick_ok.sh", "#!/bin/sh\nprintf '/tmp/demo.mp3\\n'");
    setenv("AM_FILE_DIALOG_HELPER", "build/am_pick_ok.sh", 1);
    if(am_desktop_file_dialog_pick_audio(path_buf, sizeof(path_buf)) != AM_FILE_PICK_OK) abort();
    if(strcmp(path_buf, "/tmp/demo.mp3") != 0) abort();
}

static void test_pick_audio_cancel_via_helper(void)
{
    char path_buf[1024] = {0};
    write_script("build/am_pick_cancel.sh", "#!/bin/sh\nexit 1");
    setenv("AM_FILE_DIALOG_HELPER", "build/am_pick_cancel.sh", 1);
    if(am_desktop_file_dialog_pick_audio(path_buf, sizeof(path_buf)) != AM_FILE_PICK_CANCEL) abort();
}

int main(void)
{
    test_pick_audio_success_via_helper();
    test_pick_audio_cancel_via_helper();
    return 0;
}
```

```cmake
add_executable(apple_music_desktop_file_dialog_test
    ${PROJECT_SOURCE_DIR}/main/tests/apple_music_desktop_file_dialog_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_desktop_file_dialog.c
)
target_include_directories(apple_music_desktop_file_dialog_test PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/main/src
)
add_test(
    NAME apple_music_desktop_file_dialog_test
    COMMAND $<TARGET_FILE:apple_music_desktop_file_dialog_test>
)
set_tests_properties(apple_music_desktop_file_dialog_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
```

- [ ] **Step 2: 运行测试并确认失败**

Run: `cmake -S . -B build && cmake --build build --target apple_music_desktop_file_dialog_test && ctest --test-dir build -R '^apple_music_desktop_file_dialog_test$' --output-on-failure`

Expected: FAIL，报缺少 `am_desktop_file_dialog_pick_audio` 或行为不符合预期。

- [ ] **Step 3: 写最小实现**

```c
typedef struct {
    const char *cmd;
    const char *args[8];
} am_dialog_candidate_t;

static am_file_pick_result_t run_helper_override(char *path_buf, size_t path_buf_size);
static am_file_pick_result_t run_candidate(const am_dialog_candidate_t *candidate,
                                           char *path_buf, size_t path_buf_size);

am_file_pick_result_t am_desktop_file_dialog_pick_audio(char *path_buf, size_t path_buf_size)
{
    static const am_dialog_candidate_t candidates[] = {
        { "zenity", { "zenity", "--file-selection", "--title=打开本地音频",
                      "--file-filter=音频文件 | *.mp3 *.wav *.flac *.aac *.m4a *.ogg *.opus *.wma *.mp4 *.ts *.aiff *.ac3", NULL } },
        { "qarma",  { "qarma",  "--file-selection", "--title=打开本地音频",
                      "--file-filter=音频文件 | *.mp3 *.wav *.flac *.aac *.m4a *.ogg *.opus *.wma *.mp4 *.ts *.aiff *.ac3", NULL } },
        { "kdialog",{ "kdialog","--getopenfilename", ".", "*.mp3 *.wav *.flac *.aac *.m4a *.ogg *.opus *.wma *.mp4 *.ts *.aiff *.ac3", NULL } },
    };
    size_t i;
    am_file_pick_result_t rc = run_helper_override(path_buf, path_buf_size);
    if(rc != AM_FILE_PICK_UNAVAILABLE) return rc;
    for(i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
        rc = run_candidate(&candidates[i], path_buf, path_buf_size);
        if(rc == AM_FILE_PICK_OK || rc == AM_FILE_PICK_CANCEL) return rc;
    }
    return AM_FILE_PICK_UNAVAILABLE;
}
```

- [ ] **Step 4: 运行测试并确认通过**

Run: `cmake --build build --target apple_music_desktop_file_dialog_test && ctest --test-dir build -R '^apple_music_desktop_file_dialog_test$' --output-on-failure`

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt \
  main/src/v9_apple_music/am_desktop_file_dialog.h \
  main/src/v9_apple_music/am_desktop_file_dialog.c \
  main/tests/apple_music_desktop_file_dialog_test.c
git commit -m "feat: add desktop audio file picker for apple music"
```

### Task 2: 单文件模式播放器状态与播放列表弹层

**Files:**
- Modify: `main/src/v9_apple_music/am_player.h`
- Modify: `main/src/v9_apple_music/am_player.c`
- Create: `main/tests/apple_music_single_file_player_test.c`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: `am_file_pick_result_t am_desktop_file_dialog_pick_audio(char *path_buf, size_t path_buf_size);`
- Produces: `void am_player_play_single_file(const char *path, const char *title);`
- Produces: `bool am_player_is_single_file_mode(void);`
- Produces: `void am_player_clear_single_file_mode(void);`

- [ ] **Step 1: 写失败测试**

```c
static void test_single_file_playlist_renders_one_row(void)
{
    am_miniplayer_handles_t h = am_shell_build_miniplayer(root, NULL, NULL, NULL, NULL, NULL, NULL);
    am_player_init();
    am_player_bind_miniplayer(&h);
    am_player_play_single_file("/tmp/picked.mp3", "picked");
    am_player_set_playlist_open(true);
    am_player_refresh_ui();
    if(lv_obj_get_child_count(h.playlist_list) != 1) abort();
    if(!am_player_is_single_file_mode()) abort();
}

static void test_single_file_next_prev_do_not_leave_current_file(void)
{
    am_player_play_single_file("/tmp/picked.mp3", "picked");
    am_player_next();
    am_player_prev();
    if(!am_player_is_single_file_mode()) abort();
}
```

```cmake
add_executable(apple_music_single_file_player_test
    ${PROJECT_SOURCE_DIR}/main/tests/apple_music_single_file_player_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_data.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_metrics.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_player.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_shell.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_theme.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_widgets.c
    ${V9_APPLE_MUSIC_FONT_SOURCES}
)
```

- [ ] **Step 2: 运行测试并确认失败**

Run: `cmake --build build --target apple_music_single_file_player_test && ctest --test-dir build -R '^apple_music_single_file_player_test$' --output-on-failure`

Expected: FAIL，报缺少 `am_player_play_single_file` / `am_player_is_single_file_mode` 或播放列表仍按固定本地库渲染。

- [ ] **Step 3: 写最小实现**

```c
static bool g_single_file_mode = false;
static char g_single_file_path[1024];
static char g_single_file_title[256];

void am_player_play_single_file(const char *path, const char *title)
{
    const char *urls[1] = { path };
    g_single_file_mode = true;
    am_copy_text(g_single_file_path, sizeof(g_single_file_path), path);
    am_copy_text(g_single_file_title, sizeof(g_single_file_title), title);
    g_source_kind = AM_SOURCE_LOCAL;
    g_current_local = 0U;
    music_player_deinit();
    g_local_engine_ready = false;
    music_player_init(urls, 1U);
    music_player_set_volume(g_volume);
    am_player_refresh_ui();
}

bool am_player_is_single_file_mode(void)
{
    return g_single_file_mode;
}

void am_player_clear_single_file_mode(void)
{
    g_single_file_mode = false;
    g_single_file_path[0] = '\0';
    g_single_file_title[0] = '\0';
}
```

- [ ] **Step 4: 运行测试并确认通过**

Run: `cmake --build build --target apple_music_single_file_player_test && ctest --test-dir build -R '^apple_music_single_file_player_test$' --output-on-failure`

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt \
  main/src/v9_apple_music/am_player.h \
  main/src/v9_apple_music/am_player.c \
  main/tests/apple_music_single_file_player_test.c
git commit -m "feat: add apple music single file player mode"
```

### Task 3: 迷你播放器“打开文件”按钮与模式按钮视觉

**Files:**
- Modify: `main/src/v9_apple_music/am_icons.h`
- Modify: `main/src/v9_apple_music/am_shell.h`
- Modify: `main/src/v9_apple_music/am_shell.c`
- Modify: `main/src/v9_apple_music/am_player.c`
- Modify: `main/tests/apple_music_single_file_player_test.c`

**Interfaces:**
- Consumes: `void am_player_play_single_file(const char *path, const char *title);`
- Consumes: `bool am_player_is_single_file_mode(void);`
- Produces: `am_miniplayer_handles_t am_shell_build_miniplayer(lv_obj_t *player, am_simple_event_cb_t on_mode, am_simple_event_cb_t on_prev, am_simple_event_cb_t on_play, am_simple_event_cb_t on_next, am_simple_event_cb_t on_open_file, am_simple_event_cb_t on_playlist);`

- [ ] **Step 1: 写失败测试**

```c
static void test_miniplayer_has_open_file_button(void)
{
    am_miniplayer_handles_t h = am_shell_build_miniplayer(root, NULL, NULL, NULL, NULL, NULL, NULL);
    if(h.btn_open_file == NULL) abort();
}

static void test_mode_button_text_follows_play_mode(void)
{
    g_stub_mode = MP_MODE_SHUFFLE;
    am_player_refresh_ui();
    if(strcmp(lv_label_get_text(lv_obj_get_child(h.btn_mode, 0)), "SHUF") != 0) abort();
}
```

- [ ] **Step 2: 运行测试并确认失败**

Run: `cmake --build build --target apple_music_single_file_player_test && ctest --test-dir build -R '^apple_music_single_file_player_test$' --output-on-failure`

Expected: FAIL，`btn_open_file` 为空或模式按钮文本不变。

- [ ] **Step 3: 写最小实现**

```c
#define AM_ICON_OPEN_FILE "+"

typedef struct {
    lv_obj_t *btn_mode;
    lv_obj_t *btn_prev;
    lv_obj_t *btn_play;
    lv_obj_t *btn_next;
    lv_obj_t *btn_open_file;
    lv_obj_t *btn_playlist;
} am_miniplayer_handles_t;

static const char *am_mode_button_text(music_play_mode_t mode)
{
    switch(mode) {
        case MP_MODE_SEQ: return "SEQ";
        case MP_MODE_REPEAT_ONE: return "ONE";
        case MP_MODE_REPEAT_ALL: return "LOOP";
        case MP_MODE_SHUFFLE: return "SHUF";
        default: return "SEQ";
    }
}
```

- [ ] **Step 4: 运行测试并确认通过**

Run: `cmake --build build --target apple_music_single_file_player_test && ctest --test-dir build -R '^apple_music_single_file_player_test$' --output-on-failure`

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add main/src/v9_apple_music/am_icons.h \
  main/src/v9_apple_music/am_shell.h \
  main/src/v9_apple_music/am_shell.c \
  main/src/v9_apple_music/am_player.c \
  main/tests/apple_music_single_file_player_test.c
git commit -m "feat: add apple music open-file button and mode labels"
```

### Task 4: 宿主页面接入按钮回调与“正在播放”页同步

**Files:**
- Modify: `main/src/v9_apple_music/apple_music.c`
- Create: `main/tests/apple_music_now_view_single_file_test.c`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: `am_file_pick_result_t am_desktop_file_dialog_pick_audio(char *path_buf, size_t path_buf_size);`
- Consumes: `void am_player_play_single_file(const char *path, const char *title);`
- Consumes: `bool am_player_is_single_file_mode(void);`
- Produces: `static void am_on_open_file(lv_event_t *e);`

- [ ] **Step 1: 写失败测试**

```c
static void test_open_file_click_updates_now_view(void)
{
    write_script("build/am_pick_ok.sh", "#!/bin/sh\nprintf '/tmp/My Song.mp3\\n'");
    setenv("AM_FILE_DIALOG_HELPER", "build/am_pick_ok.sh", 1);
    apple_music_create_in(root);
    simulate_click(find_obj_by_text(root, "+"));
    if(strcmp(lv_label_get_text(find_obj_by_text(root, "My Song")), "My Song") != 0) abort();
    if(strcmp(lv_label_get_text(find_obj_by_text(root, "本地文件")), "本地文件") != 0) abort();
}
```

```cmake
add_executable(apple_music_now_view_single_file_test
    ${PROJECT_SOURCE_DIR}/main/tests/apple_music_now_view_single_file_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_desktop_file_dialog.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_data.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_metrics.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_page_list.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_player.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_shell.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_state.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_theme.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_widgets.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/apple_music.c
    ${V9_APPLE_MUSIC_FONT_SOURCES}
    ${V9_APPLE_MUSIC_ASSET_SOURCES}
)
```

- [ ] **Step 2: 运行测试并确认失败**

Run: `cmake --build build --target apple_music_now_view_single_file_test && ctest --test-dir build -R '^apple_music_now_view_single_file_test$' --output-on-failure`

Expected: FAIL，按钮点击后标题、副标题或收藏按钮状态不符合预期。

- [ ] **Step 3: 写最小实现**

```c
typedef struct {
    bool has_picked_file;
    am_local_item_t picked_file;
    char toast_text[128];
    lv_obj_t *title;
    lv_obj_t *subtitle;
    lv_obj_t *favorite_btn;
    lv_obj_t *lyrics;
} am_app_t;

static void am_on_open_file(lv_event_t *e)
{
    char path_buf[1024] = {0};
    char title_buf[256] = {0};
    am_file_pick_result_t rc = am_desktop_file_dialog_pick_audio(path_buf, sizeof(path_buf));
    if(rc == AM_FILE_PICK_CANCEL) return;
    if(rc != AM_FILE_PICK_OK) {
        snprintf(g_app.toast_text, sizeof(g_app.toast_text), "当前桌面环境不支持文件选择");
        return;
    }
    am_title_from_path(title_buf, sizeof(title_buf), path_buf);
    memset(&g_app.picked_file, 0, sizeof(g_app.picked_file));
    strcpy(g_app.picked_file.path, path_buf);
    strcpy(g_app.picked_file.title, title_buf);
    g_app.has_picked_file = true;
    am_player_play_single_file(g_app.picked_file.path, g_app.picked_file.title);
    am_show_view(AM_VIEW_NOW);
}
```

- [ ] **Step 4: 运行测试并确认通过**

Run: `cmake --build build --target apple_music_now_view_single_file_test && ctest --test-dir build -R '^(apple_music_now_view_single_file_test|apple_music_single_file_player_test|apple_music_desktop_file_dialog_test)$' --output-on-failure`

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt \
  main/src/v9_apple_music/apple_music.c \
  main/tests/apple_music_now_view_single_file_test.c
git commit -m "feat: wire apple music open local file flow"
```

### Task 5: 全量回归与手工验证

**Files:**
- Modify: 无
- Test: `main/tests/apple_music_desktop_file_dialog_test.c`
- Test: `main/tests/apple_music_single_file_player_test.c`
- Test: `main/tests/apple_music_now_view_single_file_test.c`
- Test: 现有 `main/tests/apple_music_data_test.c`
- Test: 现有 `main/tests/apple_music_now_view_state_test.c`
- Test: 现有 `main/tests/apple_music_playlist_popup_layout_test.c`
- Test: 现有 `main/tests/apple_music_playlist_scrollbar_behavior_test.c`

**Interfaces:**
- Consumes: 前四个任务产出的所有接口
- Produces: 已验证的功能分支

- [ ] **Step 1: 运行 Apple Music 相关自动化测试**

Run: `cmake --build build --target apple_music_desktop_file_dialog_test apple_music_single_file_player_test apple_music_now_view_single_file_test apple_music_data_test apple_music_now_view_state_test apple_music_playlist_popup_layout_test apple_music_playlist_scrollbar_behavior_test && ctest --test-dir build -R '^apple_music_' --output-on-failure`

Expected: PASS

- [ ] **Step 2: 构建主程序**

Run: `cmake --build build --target main`

Expected: PASS

- [ ] **Step 3: 进行桌面手工验证**

Run: `./bin/main`

Expected:

- 可以点击迷你播放器里的“打开文件”按钮。
- 可以弹出系统文件选择框。
- 选择一个本地 `mp3` 或 `wav` 后切到“正在播放”。
- 标题显示文件名去后缀，副标题显示“本地文件”。
- 收藏按钮隐藏。
- 播放列表弹层只显示当前文件 1 行。
- 模式按钮文本可在 `SEQ / ONE / LOOP / SHUF` 间变化。

- [ ] **Step 4: 记录验证结果并整理变更**

```bash
git status --short
git log --oneline -n 5
```

- [ ] **Step 5: Commit**

```bash
git add .
git commit -m "test: verify apple music open local file flow"
```

## Self-Review

### Spec coverage

- 系统文件选择框：Task 1
- 单文件会话态：Task 2、Task 4
- 迷你播放器按钮：Task 3
- 模式按钮视觉：Task 3
- 不污染收藏/最近播放/持久化：Task 2、Task 4
- 播放列表弹层单文件行为：Task 2
- 桌面手工验证：Task 5

### Placeholder scan

- 本计划未使用 `TODO`、`TBD`、`实现细节后补` 一类占位词。
- 每个代码步骤都给出具体文件、函数名、命令和预期结果。

### Type consistency

- 文件选择返回统一使用 `am_file_pick_result_t`。
- 单文件播放统一使用 `am_player_play_single_file(const char *path, const char *title)`。
- 单文件状态查询统一使用 `am_player_is_single_file_mode(void)`。

## Execution Handoff

Plan complete and saved to `docs/superpowers/plans/2026-07-07-apple-music-open-local-file.md`. Two execution options:

1. Subagent-Driven (recommended) - 我按任务逐个派发独立 subagent，实现后逐段审查与收敛
2. Inline Execution - 我在当前会话里按任务顺序直接实现并在检查点汇报

Which approach?
