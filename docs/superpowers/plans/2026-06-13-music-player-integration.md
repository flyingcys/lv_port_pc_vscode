# Music Player Integration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 hls_player_demo 音频引擎与 lv_demo_music UI 集成，实现能真正播放本地文件和 HLS 流的音乐播放器。

**Architecture:** 新建 `music_player.c/.h` 胶水层，持有 `player_controller_t`；LVGL timer 每 100ms 调用 `player_controller_poll()`；player 内部线程事件通过 `lv_async_call()` 投递到 LVGL 主线程更新 UI。`lv_demo_music_main.c` 的按钮回调替换为调用 `music_player_*` 接口；换曲由 `TRACK_CHANGED` 事件驱动，频谱动画改为循环不换曲。

**Tech Stack:** LVGL v9、player_controller（hls_player_demo）、stream_player（ALSA 输出、AAC/MP3/FLAC/Opus 解码）、OpenSSL（HTTPS）、pthreads

---

## 文件结构

| 操作 | 路径 | 职责 |
|------|------|------|
| 新建 | `main/inc/music_player.h` | 对外接口，供 lvgl demo 文件包含 |
| 新建 | `main/src/music_player.c` | 胶水层：URL 加载、controller 生命周期、事件→lv_async_call |
| 修改 | `CMakeLists.txt` | 引入 hls 子库、添加 music_player.c、新增依赖 |
| 修改 | `main/src/main.c` | argv → music_player_init → lv_demo_music |
| 修改 | `lvgl/demos/music/lv_demo_music_main.c` | 5 处回调 + spectrum_end_cb + album_image_create |
| 修改 | `lvgl/demos/music/lv_demo_music_list.c` | add_list_button（启用数量）+ btn_click_event_cb |

---

## Task 1: 构建系统——引入 hls_player_demo 库

**Files:**
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 在根 CMakeLists.txt 里添加新依赖和 HLS 子目录**

在 `find_package(SDL2 REQUIRED)` 之前加入（约第 65 行附近）：

```cmake
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(ALSA REQUIRED)

set(HLS_PROJECT_ROOT_DIR ${PROJECT_SOURCE_DIR}/third-party/hls_player_demo)

add_subdirectory(${HLS_PROJECT_ROOT_DIR}/src/http_client)
add_subdirectory(${HLS_PROJECT_ROOT_DIR}/src/player_controller)
add_subdirectory(${HLS_PROJECT_ROOT_DIR}/src/stream_player)
```

- [ ] **Step 2: 把 music_player.c 加入可执行文件，添加 include 路径、编译宏、链接库**

找到非 FreeRTOS 分支的 `add_executable(main ...)` 块（约第 120-123 行），修改为：

```cmake
add_executable(main
    ${PROJECT_SOURCE_DIR}/main/src/main.c
    ${PROJECT_SOURCE_DIR}/main/src/music_player.c
    ${PROJECT_SOURCE_DIR}/main/src/mouse_cursor_icon.c
)
```

在该 `add_executable` 之后（target_link_libraries 之前）添加：

```cmake
target_include_directories(main PRIVATE
    ${HLS_PROJECT_ROOT_DIR}/src
    ${HLS_PROJECT_ROOT_DIR}/src/stream_player/include
    ${HLS_PROJECT_ROOT_DIR}/src/player_controller
    ${HLS_PROJECT_ROOT_DIR}/src/http_client
    ${ALSA_INCLUDE_DIRS}
)

target_compile_definitions(main PRIVATE
    PLATFORM_LINUX
    OPUS_BUILD
    VAR_ARRAYS
)
```

找到 `target_link_libraries(main lvgl ...)` 行（约第 130 行），在末尾补充：

```cmake
target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} m pthread
    player_controller
    stream_player_module_linux
    ${ALSA_LIBRARIES}
)
```

（替换原来的 target_link_libraries 行，保留原有库，追加三个新库）

- [ ] **Step 3: 创建 main/inc 目录（CMakeLists 已有 include_directories 指向此路径）**

```bash
mkdir -p main/inc
```

- [ ] **Step 4: 创建占位文件 main/src/music_player.c（空文件，让 cmake 不报错）**

创建 `main/src/music_player.c`，内容仅一行：
```c
/* music_player placeholder */
```

- [ ] **Step 5: 验证 cmake 配置无报错**

```bash
cd /home/share/samba/lvgl/lv_port_pc_vscode_v3-music
cmake -B build -S . 2>&1 | tail -20
```

期望：出现 `-- Configuring done` 和 `-- Build files have been written`，无 FATAL_ERROR。

- [ ] **Step 6: 验证编译成功**

```bash
cmake --build build --target main -j$(nproc) 2>&1 | tail -30
```

期望：`[100%] Linking C executable` 后成功，无编译错误。

- [ ] **Step 7: Commit**

```bash
git add CMakeLists.txt main/inc/ main/src/music_player.c
git commit -m "build: add hls_player_demo libs and music_player.c to main target"
```

---

## Task 2: 定义 music_player 接口 + stub 实现

**Files:**
- Create: `main/inc/music_player.h`
- Modify: `main/src/music_player.c`

- [ ] **Step 1: 写 music_player.h**

```c
/* main/inc/music_player.h */
#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include <stddef.h>
#include <stdbool.h>

void   music_player_init(const char **urls, size_t count);
void   music_player_deinit(void);

void   music_player_play(void);
void   music_player_pause(void);
void   music_player_resume(void);
void   music_player_next(void);
void   music_player_prev(void);
void   music_player_select(size_t index);

size_t music_player_get_count(void);
size_t music_player_get_current_index(void);
bool   music_player_is_playing(void);

#endif /* MUSIC_PLAYER_H */
```

- [ ] **Step 2: 用 stub 实现替换 music_player.c**

```c
/* main/src/music_player.c */
#include "music_player.h"

void   music_player_init(const char **urls, size_t count) { (void)urls; (void)count; }
void   music_player_deinit(void) {}
void   music_player_play(void) {}
void   music_player_pause(void) {}
void   music_player_resume(void) {}
void   music_player_next(void) {}
void   music_player_prev(void) {}
void   music_player_select(size_t index) { (void)index; }
size_t music_player_get_count(void) { return 0; }
size_t music_player_get_current_index(void) { return 0; }
bool   music_player_is_playing(void) { return false; }
```

- [ ] **Step 3: 编译确认**

```bash
cmake --build build --target main -j$(nproc) 2>&1 | tail -10
```

期望：编译成功，无警告。

- [ ] **Step 4: Commit**

```bash
git add main/inc/music_player.h main/src/music_player.c
git commit -m "feat: add music_player stub interface"
```

---

## Task 3: 修改 lv_demo_music_main.c

**Files:**
- Modify: `lvgl/demos/music/lv_demo_music_main.c`

- [ ] **Step 1: 添加 music_player.h 的 include**

在文件顶部 `#include "lv_demo_music_main.h"` 之后添加：

```c
#include "music_player.h"
```

- [ ] **Step 2: 替换 play_event_click_cb（约第 979 行）**

```c
static void play_event_click_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
        music_player_resume();
    }
    else {
        music_player_pause();
    }
}
```

- [ ] **Step 3: 替换 prev_click_event_cb（约第 990 行）**

```c
static void prev_click_event_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    music_player_prev();
}
```

- [ ] **Step 4: 替换 next_click_event_cb（约第 996 行）**

```c
static void next_click_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        music_player_next();
    }
}
```

- [ ] **Step 5: 替换 album_gesture_event_cb（约第 971 行）**

```c
static void album_gesture_event_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if(dir == LV_DIR_LEFT) music_player_next();
    if(dir == LV_DIR_RIGHT) music_player_prev();
}
```

- [ ] **Step 6: 替换 spectrum_end_cb（约第 1012 行）——改为循环不换曲**

```c
static void spectrum_end_cb(lv_anim_t * a)
{
    LV_UNUSED(a);
    if(!playing) return;
    lv_anim_t new_a;
    lv_anim_init(&new_a);
    lv_anim_set_values(&new_a, 0, (int32_t)(spectrum_len - 1));
    lv_anim_set_exec_cb(&new_a, spectrum_anim_cb);
    lv_anim_set_var(&new_a, spectrum_obj);
    lv_anim_set_duration(&new_a, (spectrum_len * 1000) / 30);
    lv_anim_set_playback_duration(&new_a, 0);
    lv_anim_set_completed_cb(&new_a, spectrum_end_cb);
    lv_anim_start(&new_a);
}
```

- [ ] **Step 7: 修改 album_image_create 的 switch，使 track_id > 2 时循环复用封面和频谱数据（约第 944 行）**

将 `switch(track_id)` 改为 `switch(track_id % 3)`：

```c
    switch(track_id % 3) {
        case 2:
            lv_image_set_src(img, &img_lv_demo_music_cover_3);
            spectrum = spectrum_3;
            spectrum_len = sizeof(spectrum_3) / sizeof(spectrum_3[0]);
            break;
        case 1:
            lv_image_set_src(img, &img_lv_demo_music_cover_2);
            spectrum = spectrum_2;
            spectrum_len = sizeof(spectrum_2) / sizeof(spectrum_2[0]);
            break;
        default:
            lv_image_set_src(img, &img_lv_demo_music_cover_1);
            spectrum = spectrum_1;
            spectrum_len = sizeof(spectrum_1) / sizeof(spectrum_1[0]);
            break;
    }
```

- [ ] **Step 8: 编译确认**

```bash
cmake --build build --target main -j$(nproc) 2>&1 | tail -10
```

期望：编译成功。

- [ ] **Step 9: 运行确认 UI 还能正常显示（按钮点击无效果是正常的，stubs 还未实现）**

```bash
./bin/main
```

期望：UI 正常显示，频谱动画循环播放，按下播放按钮无变化（等待后续实现）。

- [ ] **Step 10: Commit**

```bash
git add lvgl/demos/music/lv_demo_music_main.c
git commit -m "feat: wire lv_demo_music_main callbacks to music_player interface"
```

---

## Task 4: 修改 lv_demo_music_list.c

**Files:**
- Modify: `lvgl/demos/music/lv_demo_music_list.c`

- [ ] **Step 1: 添加 music_player.h include**

在文件顶部 `#include "lv_demo_music_main.h"` 之后添加：

```c
#include "music_player.h"
```

- [ ] **Step 2: 修改 add_list_button 中的禁用逻辑（约第 196 行）**

将原来的：
```c
    if(track_id >= 3) {
        lv_obj_add_state(btn, LV_STATE_DISABLED);
    }
```

替换为：
```c
    {
        uint32_t active_count = (uint32_t)(music_player_get_count() > 0
                                           ? music_player_get_count() : 3U);
        if(track_id >= active_count) {
            lv_obj_add_state(btn, LV_STATE_DISABLED);
        }
    }
```

- [ ] **Step 3: 修改 btn_click_event_cb（约第 229 行）——同时更新 UI 和触发真实播放**

```c
static void btn_click_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target(e);
    uint32_t idx = lv_obj_get_index(btn);
    _lv_demo_music_play(idx);
    music_player_select((size_t)idx);
}
```

（`_lv_demo_music_play` 提供即时 UI 响应；`music_player_select` 触发真实播放。
当 TRACK_CHANGED 事件到来再次调用 `_lv_demo_music_play(idx)` 时，因为 track_id 已相同，内部 `track_load` 会早返回，是幂等的。）

- [ ] **Step 4: 编译确认**

```bash
cmake --build build --target main -j$(nproc) 2>&1 | tail -10
```

- [ ] **Step 5: Commit**

```bash
git add lvgl/demos/music/lv_demo_music_list.c
git commit -m "feat: wire track list button to music_player_select"
```

---

## Task 5: 修改 main.c——传入 argv

**Files:**
- Modify: `main/src/main.c`

- [ ] **Step 1: 添加 include 并调用 music_player_init**

在 `#include "glob.h"` 之后添加：

```c
#include "music_player.h"
```

找到 `lv_demo_music();` 前面（约第 80 行），在其前插入：

```c
  music_player_init((const char **)(argv + 1), (size_t)(argc > 1 ? argc - 1 : 0));
```

完整的相关代码段如下：

```c
  /*Initialize LVGL*/
  lv_init();

  /*Initialize the HAL (display, input devices, tick) for LVGL*/
  hal_init(640, 480);

  #if LV_USE_OS == LV_OS_NONE

  music_player_init((const char **)(argv + 1), (size_t)(argc > 1 ? argc - 1 : 0));

  lv_demo_music();
```

- [ ] **Step 2: 编译确认**

```bash
cmake --build build --target main -j$(nproc) 2>&1 | tail -10
```

- [ ] **Step 3: Commit**

```bash
git add main/src/main.c
git commit -m "feat: pass argv URLs to music_player_init"
```

---

## Task 6: 实现 music_player.c——URL 加载 + controller 生命周期 + polling

**Files:**
- Modify: `main/src/music_player.c`

- [ ] **Step 1: 用完整实现替换 music_player.c**

```c
/* main/src/music_player.c */
#define _DEFAULT_SOURCE
#include "music_player.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "lvgl/lvgl.h"
#include "lvgl/demos/music/lv_demo_music_main.h"

#include "player_controller.h"
#include "stream_player.h"

#define MUSIC_PLAYER_MAX_TRACKS 64U

static char                *g_urls[MUSIC_PLAYER_MAX_TRACKS];
static size_t               g_url_count = 0;
static player_controller_t *g_controller = NULL;
static lv_timer_t          *g_poll_timer = NULL;

/* ---- forward declarations ---- */
static void on_player_event(player_controller_t *ctrl,
                            player_controller_event_t event,
                            const void *event_data,
                            void *user_data);
static void poll_timer_cb(lv_timer_t *t);

/* ---- helpers ---- */
static bool is_supported_file(const char *path) {
    return stream_player_is_supported_local_audio_file(path);
}

static bool is_directory(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0) && S_ISDIR(st.st_mode);
}

static int cmp_str(const void *a, const void *b) {
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

static void add_url(const char *url) {
    if (g_url_count >= MUSIC_PLAYER_MAX_TRACKS) return;
    g_urls[g_url_count] = strdup(url);
    if (g_urls[g_url_count]) g_url_count++;
}

static void expand_directory(const char *dir_path) {
    DIR *d = opendir(dir_path);
    if (!d) return;

    char *entries[MUSIC_PLAYER_MAX_TRACKS];
    size_t entry_count = 0;
    struct dirent *ent;

    while ((ent = readdir(d)) != NULL && entry_count < MUSIC_PLAYER_MAX_TRACKS) {
        if (ent->d_name[0] == '.') continue;
        size_t dir_len = strlen(dir_path);
        size_t name_len = strlen(ent->d_name);
        char *full = (char *)malloc(dir_len + 1 + name_len + 1);
        if (!full) continue;
        memcpy(full, dir_path, dir_len);
        full[dir_len] = '/';
        memcpy(full + dir_len + 1, ent->d_name, name_len + 1);
        if (!is_supported_file(full)) {
            free(full);
            continue;
        }
        entries[entry_count++] = full;
    }
    closedir(d);

    qsort(entries, entry_count, sizeof(entries[0]), cmp_str);
    for (size_t i = 0; i < entry_count; i++) {
        add_url(entries[i]);
        free(entries[i]);
    }
}

/* ---- public API ---- */

void music_player_init(const char **urls, size_t count) {
    stream_player_config_t config;
    const char *url_array[MUSIC_PLAYER_MAX_TRACKS];

    g_url_count = 0;

    for (size_t i = 0; i < count && g_url_count < MUSIC_PLAYER_MAX_TRACKS; i++) {
        if (!urls[i]) continue;
        if (is_directory(urls[i])) {
            expand_directory(urls[i]);
        } else {
            add_url(urls[i]);
        }
    }

    if (g_url_count == 0) return;  /* no URLs: fallback to fake UI */

    stream_player_reset_interrupt_state();

    stream_player_get_default_config(&config);
    stream_player_apply_profile(&config, STREAM_PROFILE_BALANCED);
    config.log_level = STREAM_LOG_WARN;

    g_controller = player_controller_create(on_player_event, NULL);
    if (!g_controller) { g_url_count = 0; return; }

    if (player_controller_set_stream_config(g_controller, &config) != 0) {
        player_controller_destroy(g_controller);
        g_controller = NULL;
        g_url_count = 0;
        return;
    }

    for (size_t i = 0; i < g_url_count; i++) url_array[i] = g_urls[i];
    if (player_controller_load_urls(g_controller, url_array, g_url_count) != 0) {
        player_controller_destroy(g_controller);
        g_controller = NULL;
        g_url_count = 0;
        return;
    }

    g_poll_timer = lv_timer_create(poll_timer_cb, 100, NULL);

    player_controller_play(g_controller);
}

void music_player_deinit(void) {
    if (g_poll_timer) {
        lv_timer_delete(g_poll_timer);
        g_poll_timer = NULL;
    }
    if (g_controller) {
        player_controller_stop(g_controller);
        player_controller_destroy(g_controller);
        g_controller = NULL;
    }
    for (size_t i = 0; i < g_url_count; i++) {
        free(g_urls[i]);
        g_urls[i] = NULL;
    }
    g_url_count = 0;
}

void music_player_play(void) {
    if (!g_controller) return;
    player_controller_play(g_controller);
}

void music_player_pause(void) {
    if (!g_controller) return;
    player_controller_pause(g_controller);
}

void music_player_resume(void) {
    if (!g_controller) return;
    player_controller_state_t s = player_controller_get_state(g_controller);
    if (s == PLAYER_CONTROLLER_STATE_PAUSED) {
        player_controller_resume(g_controller);
    } else {
        player_controller_play(g_controller);
    }
}

void music_player_next(void) {
    if (!g_controller) return;
    player_controller_next(g_controller);
}

void music_player_prev(void) {
    if (!g_controller) return;
    player_controller_prev(g_controller);
}

void music_player_select(size_t index) {
    if (!g_controller) return;
    player_controller_select(g_controller, index);
}

size_t music_player_get_count(void) {
    return g_url_count;
}

size_t music_player_get_current_index(void) {
    if (!g_controller) return 0;
    return player_controller_get_current_index(g_controller);
}

bool music_player_is_playing(void) {
    if (!g_controller) return false;
    return player_controller_get_state(g_controller) == PLAYER_CONTROLLER_STATE_PLAYING;
}

/* ---- internal ---- */

static void poll_timer_cb(lv_timer_t *t) {
    (void)t;
    if (g_controller) player_controller_poll(g_controller);
}
```

（事件处理在 Task 7 补充）

在文件末尾（`poll_timer_cb` 之后）暂时加入空的 on_player_event stub：

```c
static void on_player_event(player_controller_t *ctrl,
                            player_controller_event_t event,
                            const void *event_data,
                            void *user_data) {
    (void)ctrl; (void)event; (void)event_data; (void)user_data;
}
```

- [ ] **Step 2: 编译确认**

```bash
cmake --build build --target main -j$(nproc) 2>&1 | tail -20
```

期望：编译成功。

- [ ] **Step 3: 用本地音频文件测试启动播放（如有 mp3/flac 文件）**

```bash
./bin/main /path/to/audio_dir/
# 或
./bin/main /path/to/a.mp3 /path/to/b.flac
```

期望：听到音频开始播放，UI 正常显示（按钮状态暂时不同步，Task 7 修复）。

无音频文件时测试无 URL 降级：

```bash
./bin/main
```

期望：UI 正常，假动画照常运行。

- [ ] **Step 4: Commit**

```bash
git add main/src/music_player.c
git commit -m "feat: implement music_player URL loading and player_controller lifecycle"
```

---

## Task 7: 实现 music_player.c 事件处理——lv_async_call 同步 UI

**Files:**
- Modify: `main/src/music_player.c`

- [ ] **Step 1: 在文件顶部的 forward declarations 区域添加 async 结构体**

在 `/* ---- forward declarations ---- */` 区域之前添加：

```c
typedef struct {
    player_controller_event_t  event;
    size_t                     track_index;
    player_controller_state_t  state;
} music_player_async_arg_t;

static void music_player_async_handler(void *arg);
```

- [ ] **Step 2: 用完整实现替换 on_player_event stub**

找到文件末尾的空 `on_player_event` stub，替换为：

```c
static void music_player_async_handler(void *arg) {
    music_player_async_arg_t *data = (music_player_async_arg_t *)arg;

    switch (data->event) {
        case PLAYER_CONTROLLER_EVENT_TRACK_CHANGED:
            _lv_demo_music_play((uint32_t)data->track_index);
            break;
        case PLAYER_CONTROLLER_EVENT_STATE_CHANGED:
            if (data->state == PLAYER_CONTROLLER_STATE_PAUSED ||
                data->state == PLAYER_CONTROLLER_STATE_STOPPED) {
                _lv_demo_music_pause();
            } else if (data->state == PLAYER_CONTROLLER_STATE_PLAYING) {
                _lv_demo_music_resume();
            }
            break;
        case PLAYER_CONTROLLER_EVENT_PLAYLIST_END:
        case PLAYER_CONTROLLER_EVENT_ERROR:
            _lv_demo_music_pause();
            break;
        default:
            break;
    }

    free(data);
}

static void on_player_event(player_controller_t *ctrl,
                            player_controller_event_t event,
                            const void *event_data,
                            void *user_data) {
    (void)user_data;

    music_player_async_arg_t *arg =
        (music_player_async_arg_t *)malloc(sizeof(*arg));
    if (!arg) return;

    arg->event       = event;
    arg->track_index = player_controller_get_current_index(ctrl);
    arg->state       = PLAYER_CONTROLLER_STATE_IDLE;

    if (event == PLAYER_CONTROLLER_EVENT_STATE_CHANGED && event_data) {
        arg->state = *(const player_controller_state_t *)event_data;
    }

    lv_async_call(music_player_async_handler, arg);
}
```

- [ ] **Step 3: 编译确认**

```bash
cmake --build build --target main -j$(nproc) 2>&1 | tail -10
```

- [ ] **Step 4: 端到端测试——验证 UI 与播放状态同步**

```bash
./bin/main /path/to/audio_dir/
```

验证清单：
- [ ] 启动后自动开始播放，play 按钮显示为 pause 状态（已 checked）
- [ ] 点击 pause → 音频暂停，频谱动画停止
- [ ] 点击 play → 音频恢复，频谱动画继续
- [ ] 点击 next → 切换到下一首，封面滑动动画播放
- [ ] 点击 prev → 切换到上一首
- [ ] 一首播完后自动切换到下一首（TRACK_CHANGED 触发 UI 更新）
- [ ] 播放列表末尾：UI 停止（PLAYLIST_END 触发 pause）

- [ ] **Step 5: Commit**

```bash
git add main/src/music_player.c
git commit -m "feat: implement music_player event handling via lv_async_call"
```

---

## Task 8: 端到端回归测试

**Files:** 无修改，仅测试

- [ ] **Step 1: 测试无参数启动（降级模式）**

```bash
./bin/main
```

期望：
- UI 正常显示，假动画播放
- 播放按钮点击无效果（无音频）
- 无崩溃，无报错

- [ ] **Step 2: 测试单个本地文件**

```bash
./bin/main /path/to/test.mp3
```

期望：
- 播放列表仅 1 首，启动自动播放
- 曲目播完后 UI 停止（PLAYLIST_END），无崩溃

- [ ] **Step 3: 测试本地目录**

```bash
./bin/main /path/to/music_dir/
```

期望：
- 目录里所有支持格式的文件按文件名排序加入播放列表
- 列表视图中对应数量的曲目可点击（其余灰色禁用）
- next/prev 按钮循环切换

- [ ] **Step 4: 测试 HLS 流（如有网络）**

```bash
./bin/main http://ngcdn001.cnr.cn/live/zgzs/index.m3u8
```

期望：
- 网络流加载后开始播放
- UI 进入播放状态

- [ ] **Step 5: 最终 Commit（如有遗留改动）**

```bash
git status
# 如有未提交改动：
git add -p
git commit -m "fix: address integration test findings"
```

---

## 已知限制（不在本计划范围内）

- 进度条时间基于假计时器（每秒 +1），不反映真实播放位置
- 轨道标题/艺术家/流派使用 lv_demo_music.c 硬编码数据，不从文件 metadata 读取
- 频谱动画使用预录数据，与音频内容无关
- 无信号处理（Ctrl+C 直接终止进程，音频线程由 OS 清理）
