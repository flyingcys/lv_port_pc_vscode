# v9_apple_music 接入真实播放服务实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 v9_apple_music UI 接入 hls_player_demo 后端，实现 mini-player 控制可用、本地文件列表点击播放、HLS 广播流切换，并通过 `am_config.json` 配置来源。

**Architecture:** 新建 `am_config`（JSON 配置解析，cJSON 单文件库）和 `am_player`（播放适配层，100ms lv_timer 轮询 music_player 状态更新 LVGL widget）。mini-player 构建时返回 widget 句柄并注册按钮回调。本地页扫描目录构建真实列表，广播页从配置构建频道列表，两者点击后调 am_player API 切换播放状态。

**Tech Stack:** C99, LVGL v9, cJSON（单文件 MIT 库），hls_player_demo/player_controller，现有 music_player.c 封装层

---

## 文件地图

| 操作 | 文件 | 职责 |
|------|------|------|
| 新建 | `third-party/cjson/cJSON.h` + `cJSON.c` | cJSON 单文件库 |
| 新建 | `main/src/v9_apple_music/am_config.h` + `am_config.c` | JSON 配置解析 |
| 新建 | `main/src/v9_apple_music/am_player.h` + `am_player.c` | 播放器适配层（核心） |
| 新建 | `main/src/v9_apple_music/am_page_local.h` + `am_page_local.c` | 本地/广播真实页面 |
| 新建 | `main/tests/am_config_test.c` | am_config 单元测试 |
| 修改 | `main/src/v9_apple_music/am_icons.h` | 新增 AM_ICON_PAUSE |
| 修改 | `main/inc/music_player.h` | 新增 `music_player_get_current_url()` |
| 修改 | `main/src/music_player.c` | 移除 `_lv_demo_music_*` 调用；实现 `music_player_get_current_url()` |
| 修改 | `main/src/v9_apple_music/am_shell.h` | mini-player 返回 `am_miniplayer_handles_t` |
| 修改 | `main/src/v9_apple_music/am_shell.c` | 实现 handles 返回 + 按钮回调 |
| 修改 | `main/src/v9_apple_music/apple_music.c` | 加载 config，init player，接通页面 |
| 修改 | `main/src/main.c` | 清理注释掉的 music_player_init |
| 修改 | `CMakeLists.txt` | 加入 cjson source + include；加入 am_config_test |

**注意：** `APPLE_MUSIC_SOURCES` 使用 `file(GLOB .../*.c)`，`am_config.c`、`am_player.c`、`am_page_local.c` 会被自动包含，无需额外 target_sources。

---

## Task 1：添加 cJSON 依赖

**Files:**
- Create: `third-party/cjson/cJSON.h`
- Create: `third-party/cjson/cJSON.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 1.1：下载 cJSON 单文件库**

```bash
mkdir -p third-party/cjson
curl -L https://raw.githubusercontent.com/DaveGamble/cJSON/v1.7.18/cJSON.h \
     -o third-party/cjson/cJSON.h
curl -L https://raw.githubusercontent.com/DaveGamble/cJSON/v1.7.18/cJSON.c \
     -o third-party/cjson/cJSON.c
```

验证文件存在且非空：

```bash
wc -l third-party/cjson/cJSON.h third-party/cjson/cJSON.c
```

Expected: cJSON.h ~300+ 行, cJSON.c ~3000+ 行

- [ ] **Step 1.2：在 CMakeLists.txt 中引入 cJSON**

在 `# Find and include SDL2 library` 行**之前**，插入：

```cmake
# cJSON single-file JSON library
add_library(cjson STATIC third-party/cjson/cJSON.c)
target_include_directories(cjson PUBLIC ${PROJECT_SOURCE_DIR}/third-party/cjson)
```

在 `target_link_libraries(main lvgl ...` 行，追加 `cjson`：

```cmake
target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} m pthread
    player_controller
    stream_player_module_linux
    ${ALSA_LIBRARIES}
    cjson
)
```

另外，在 `target_include_directories(main PRIVATE ...)` 块中追加：

```cmake
    ${PROJECT_SOURCE_DIR}/third-party/cjson
```

- [ ] **Step 1.3：验证 cJSON 能编译**

```bash
cmake -S . -B build && cmake --build build -j$(nproc) 2>&1 | tail -5
```

Expected: 无错误，`cjson` 静态库出现在 build 输出中。

- [ ] **Step 1.4：提交**

```bash
git add third-party/cjson/cJSON.h third-party/cjson/cJSON.c CMakeLists.txt
git commit -m "build: add cJSON single-file library dependency"
```

---

## Task 2：新增 AM_ICON_PAUSE

**Files:**
- Modify: `main/src/v9_apple_music/am_icons.h`

字体子集包含完整 ASCII（`0x20-0x7F`），`||` 可直接用作暂停图标，无需重新生成字体。

- [ ] **Step 2.1：向 am_icons.h 添加 PAUSE 定义**

在 `#define AM_ICON_NEXT` 行之后添加：

```c
#define AM_ICON_PAUSE     "||"  /* ASCII 竖线对，暂停图标（字体子集含全 ASCII）*/
```

- [ ] **Step 2.2：提交**

```bash
git add main/src/v9_apple_music/am_icons.h
git commit -m "feat(icons): add AM_ICON_PAUSE using ASCII vertical bars"
```

---

## Task 3：am_config — JSON 配置文件解析

**Files:**
- Create: `main/src/v9_apple_music/am_config.h`
- Create: `main/src/v9_apple_music/am_config.c`
- Create: `main/tests/am_config_test.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 3.1：写失败测试 `main/tests/am_config_test.c`**

```c
/* main/tests/am_config_test.c */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/v9_apple_music/am_config.h"

static void write_tmp_config(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    assert(f);
    fputs(content, f);
    fclose(f);
}

static void test_load_valid_config(void) {
    const char *path = "/tmp/am_test_config.json";
    write_tmp_config(path,
        "{"
        "  \"local\": { \"dir\": \"/tmp/music\" },"
        "  \"radio\": ["
        "    { \"title\": \"Lo-Fi\", \"subtitle\": \"chill\", \"url\": \"https://example.com/lofi.m3u8\" },"
        "    { \"title\": \"Jazz\",  \"subtitle\": \"night\", \"url\": \"https://example.com/jazz.m3u8\" }"
        "  ]"
        "}"
    );

    am_config_t cfg = {0};
    int ret = am_config_load_from_path(path, &cfg);
    assert(ret == 0);
    assert(strcmp(cfg.local_dir, "/tmp/music") == 0);
    assert(cfg.radio_count == 2);
    assert(strcmp(cfg.radio[0].title, "Lo-Fi") == 0);
    assert(strcmp(cfg.radio[0].subtitle, "chill") == 0);
    assert(strcmp(cfg.radio[0].url, "https://example.com/lofi.m3u8") == 0);
    assert(strcmp(cfg.radio[1].title, "Jazz") == 0);
    remove(path);
}

static void test_load_missing_file(void) {
    am_config_t cfg = {0};
    int ret = am_config_load_from_path("/tmp/nonexistent_config_xyz.json", &cfg);
    assert(ret == -1);
}

static void test_load_invalid_json(void) {
    const char *path = "/tmp/am_test_bad.json";
    write_tmp_config(path, "{ not valid json }}}");
    am_config_t cfg = {0};
    int ret = am_config_load_from_path(path, &cfg);
    assert(ret == -2);
    remove(path);
}

static void test_empty_radio_array(void) {
    const char *path = "/tmp/am_test_empty.json";
    write_tmp_config(path, "{ \"local\": { \"dir\": \"/tmp\" }, \"radio\": [] }");
    am_config_t cfg = {0};
    int ret = am_config_load_from_path(path, &cfg);
    assert(ret == 0);
    assert(cfg.radio_count == 0);
    assert(strcmp(cfg.local_dir, "/tmp") == 0);
    remove(path);
}

int main(void) {
    test_load_valid_config();
    test_load_missing_file();
    test_load_invalid_json();
    test_empty_radio_array();
    printf("am_config_test: all tests passed\n");
    return 0;
}
```

- [ ] **Step 3.2：创建 `main/src/v9_apple_music/am_config.h`**

```c
/* main/src/v9_apple_music/am_config.h */
#ifndef AM_CONFIG_H
#define AM_CONFIG_H

#define AM_CONFIG_RADIO_MAX 16

typedef struct {
    char title[128];
    char subtitle[128];
    char url[512];
} am_radio_item_t;

typedef struct {
    char local_dir[512];
    am_radio_item_t radio[AM_CONFIG_RADIO_MAX];
    int  radio_count;
} am_config_t;

/* 从指定路径加载配置。返回 0=成功, -1=文件不存在, -2=解析失败 */
int  am_config_load_from_path(const char *path, am_config_t *out);

/* 按优先级查找配置文件（$AM_CONFIG → ./am_config.json → ~/.config/apple_music/config.json）*/
int  am_config_load(am_config_t *out);

#endif /* AM_CONFIG_H */
```

- [ ] **Step 3.3：创建 `main/src/v9_apple_music/am_config.c`**

```c
/* main/src/v9_apple_music/am_config.c */
#include "am_config.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void safe_copy(char *dst, size_t dsz, const char *src) {
    if(!src) { dst[0] = '\0'; return; }
    strncpy(dst, src, dsz - 1);
    dst[dsz - 1] = '\0';
}

int am_config_load_from_path(const char *path, am_config_t *out) {
    FILE *f;
    long  file_size;
    char *buf;
    size_t n;
    cJSON *root, *local, *radio, *item;
    int    i;

    if(!path || !out) return -1;
    memset(out, 0, sizeof(*out));

    f = fopen(path, "rb");
    if(!f) return -1;

    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    rewind(f);
    if(file_size <= 0) { fclose(f); return -2; }

    buf = (char *)malloc((size_t)file_size + 1u);
    if(!buf) { fclose(f); return -2; }

    n = fread(buf, 1, (size_t)file_size, f);
    fclose(f);
    buf[n] = '\0';

    root = cJSON_Parse(buf);
    free(buf);
    if(!root) return -2;

    /* local.dir */
    local = cJSON_GetObjectItemCaseSensitive(root, "local");
    if(cJSON_IsObject(local)) {
        cJSON *dir = cJSON_GetObjectItemCaseSensitive(local, "dir");
        if(cJSON_IsString(dir)) safe_copy(out->local_dir, sizeof(out->local_dir), dir->valuestring);
    }

    /* radio[] */
    radio = cJSON_GetObjectItemCaseSensitive(root, "radio");
    if(cJSON_IsArray(radio)) {
        i = 0;
        cJSON_ArrayForEach(item, radio) {
            if(i >= AM_CONFIG_RADIO_MAX) break;
            if(!cJSON_IsObject(item)) continue;
            cJSON *t  = cJSON_GetObjectItemCaseSensitive(item, "title");
            cJSON *s  = cJSON_GetObjectItemCaseSensitive(item, "subtitle");
            cJSON *u  = cJSON_GetObjectItemCaseSensitive(item, "url");
            if(!cJSON_IsString(u)) continue;
            safe_copy(out->radio[i].title,    sizeof(out->radio[i].title),    cJSON_IsString(t) ? t->valuestring : "");
            safe_copy(out->radio[i].subtitle, sizeof(out->radio[i].subtitle), cJSON_IsString(s) ? s->valuestring : "");
            safe_copy(out->radio[i].url,      sizeof(out->radio[i].url),      u->valuestring);
            i++;
        }
        out->radio_count = i;
    }

    cJSON_Delete(root);
    return 0;
}

int am_config_load(am_config_t *out) {
    const char *env;
    char        path[1024];
    const char *candidates[3];
    int         k;

    if(!out) return -1;

    env = getenv("AM_CONFIG");
    candidates[0] = env;
    candidates[1] = "./am_config.json";

    /* ~/.config/apple_music/config.json */
    path[0] = '\0';
    const char *home = getenv("HOME");
    if(home) {
        snprintf(path, sizeof(path), "%s/.config/apple_music/config.json", home);
        candidates[2] = path;
    } else {
        candidates[2] = NULL;
    }

    for(k = 0; k < 3; k++) {
        if(!candidates[k]) continue;
        if(am_config_load_from_path(candidates[k], out) == 0) return 0;
    }
    memset(out, 0, sizeof(*out));
    return -1;
}
```

- [ ] **Step 3.4：在 CMakeLists.txt 中添加 am_config_test**

在 `apple_music_metrics_test` 测试定义**之后**添加：

```cmake
add_executable(am_config_test
    ${PROJECT_SOURCE_DIR}/main/tests/am_config_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_config.c
)
target_include_directories(am_config_test PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music
    ${PROJECT_SOURCE_DIR}/third-party/cjson
)
target_link_libraries(am_config_test cjson)
add_test(NAME am_config_test COMMAND $<TARGET_FILE:am_config_test>)
set_tests_properties(am_config_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
```

- [ ] **Step 3.5：编译并运行测试**

```bash
cmake -S . -B build && cmake --build build --target am_config_test -j$(nproc)
./build/am_config_test
```

Expected: `am_config_test: all tests passed`

也可通过 ctest：

```bash
cd build && ctest -R am_config_test -V
```

Expected: `1 test passed`

- [ ] **Step 3.6：提交**

```bash
git add main/src/v9_apple_music/am_config.h \
        main/src/v9_apple_music/am_config.c \
        main/tests/am_config_test.c \
        CMakeLists.txt
git commit -m "feat(config): add am_config JSON parser with cJSON"
```

---

## Task 4：music_player — 移除 demo music UI 调用，新增 get_current_url

**Files:**
- Modify: `main/inc/music_player.h`
- Modify: `main/src/music_player.c`

`_lv_demo_music_play/pause/resume` 是旧 demo music UI 的 widget 操作接口，apple_music_create() 已清除 demo 屏幕，直接调用会操作悬挂指针，必须移除。

- [ ] **Step 4.1：向 `main/inc/music_player.h` 新增声明**

在最后一行 `#endif` 之前添加：

```c
/* 返回当前播放项的 URL（本地路径或流地址）。无播放项时返回空字符串。*/
const char *music_player_get_current_url(void);
```

- [ ] **Step 4.2：修改 `main/src/music_player.c`**

**4.2a：移除 `_lv_demo_music_*` 调用。**

找到 `music_player_async_handler` 函数，将其中对 `_lv_demo_music_play`、`_lv_demo_music_pause`、`_lv_demo_music_resume` 的调用全部删除，保留 `g_current_duration_ms` 更新逻辑不变。修改后的函数体：

```c
static void music_player_async_handler(void *data) {
    music_player_async_arg_t *arg = (music_player_async_arg_t *)data;
    if(!arg) return;

    switch(arg->event) {
        case PLAYER_CONTROLLER_EVENT_TRACK_CHANGED: {
            const player_playlist_item_t *item =
                player_controller_get_playlist_item(g_controller, arg->track_index);
            g_audio_sample_rate   = 0U;
            g_audio_channels      = 0U;
            g_current_duration_ms = (item && !item->is_live)
                                    ? audio_probe_duration_ms(item->url) : 0U;
            break;
        }
        case PLAYER_CONTROLLER_EVENT_STATE_CHANGED:
        case PLAYER_CONTROLLER_EVENT_PLAYLIST_END:
        case PLAYER_CONTROLLER_EVENT_ERROR:
            break;
        default:
            break;
    }
    free(arg);
}
```

**4.2b：在文件末尾实现 `music_player_get_current_url`：**

```c
const char *music_player_get_current_url(void) {
    if(!g_controller) return "";
    const player_playlist_item_t *item =
        player_controller_get_current_item(g_controller);
    return (item && item->url[0]) ? item->url : "";
}
```

- [ ] **Step 4.3：编译验证（无测试，需保证不破坏现有构建）**

```bash
cmake --build build -j$(nproc) 2>&1 | grep -E "error:|warning:" | head -20
```

Expected: 0 个 error。

- [ ] **Step 4.4：提交**

```bash
git add main/inc/music_player.h main/src/music_player.c
git commit -m "fix(music_player): remove dead lv_demo_music UI calls; add get_current_url"
```

---

## Task 5：am_player — 播放器适配层

**Files:**
- Create: `main/src/v9_apple_music/am_player.h`
- Create: `main/src/v9_apple_music/am_player.c`

am_player 是 UI 与 music_player 之间唯一的桥梁。它通过 100ms lv_timer 轮询状态，所有 widget 操作均在 LVGL 主线程执行。

- [ ] **Step 5.1：创建 `main/src/v9_apple_music/am_player.h`**

```c
/* main/src/v9_apple_music/am_player.h */
#ifndef AM_PLAYER_H
#define AM_PLAYER_H

#include "lvgl/lvgl.h"
#include <stddef.h>

/* mini-player 关键 widget 句柄 */
typedef struct {
    lv_obj_t *title_label;
    lv_obj_t *subtitle_label;
    lv_obj_t *time_cur;
    lv_obj_t *time_total;
    lv_obj_t *progress_fill;
    lv_obj_t *play_icon;
} am_miniplayer_handles_t;

void am_player_init(void);
void am_player_deinit(void);

/* mini-player 重建后立即调用（主题切换时也需调用）*/
void am_player_bind_miniplayer(const am_miniplayer_handles_t *h);

/* 本地页：替换整个播放列表并从 start_index 播放 */
void am_player_load_local(const char **urls, size_t count, size_t start_index);

/* 广播页：直接切换 HLS 流（不影响本地列表）*/
void am_player_play_stream(const char *url, const char *title);

/* 控制按钮 LVGL 事件回调 */
void am_player_on_play_pause(lv_event_t *e);
void am_player_on_prev(lv_event_t *e);
void am_player_on_next(lv_event_t *e);

#endif /* AM_PLAYER_H */
```

- [ ] **Step 5.2：创建 `main/src/v9_apple_music/am_player.c`**

```c
/* main/src/v9_apple_music/am_player.c */
#include "am_player.h"
#include "am_icons.h"
#include "music_player.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>

#define AM_PLAYER_LOCAL_MAX 64

typedef enum {
    AM_PLAYER_MODE_IDLE = 0,
    AM_PLAYER_MODE_LOCAL,
    AM_PLAYER_MODE_STREAM,
} am_player_mode_t;

static am_miniplayer_handles_t g_h;
static am_player_mode_t        g_mode          = AM_PLAYER_MODE_IDLE;
static char                    g_stream_title[128];
static char                   *g_local_urls[AM_PLAYER_LOCAL_MAX];
static size_t                  g_local_count   = 0;
static size_t                  g_last_idx      = (size_t)-1;
static bool                    g_last_playing  = false;
static lv_timer_t             *g_timer         = NULL;

/* ── helpers ──────────────────────────────────────────────────────────── */

static const char *current_display_title(void) {
    if(g_mode == AM_PLAYER_MODE_STREAM) return g_stream_title;
    if(g_mode == AM_PLAYER_MODE_LOCAL && g_local_count > 0) {
        size_t idx = music_player_get_current_index();
        if(idx < g_local_count && g_local_urls[idx]) {
            static char name_buf[128];
            char copy[512];
            strncpy(copy, g_local_urls[idx], sizeof(copy) - 1);
            copy[sizeof(copy) - 1] = '\0';
            char *b = basename(copy);
            strncpy(name_buf, b, sizeof(name_buf) - 1);
            name_buf[sizeof(name_buf) - 1] = '\0';
            char *dot = strrchr(name_buf, '.');
            if(dot) *dot = '\0';
            return name_buf;
        }
    }
    return "";
}

static void refresh_title(void) {
    if(!g_h.title_label) return;
    lv_label_set_text(g_h.title_label, current_display_title());
}

static void refresh_play_icon(bool playing) {
    if(!g_h.play_icon) return;
    lv_label_set_text(g_h.play_icon, playing ? AM_ICON_PAUSE : AM_ICON_PLAY);
}

static void refresh_progress(void) {
    uint32_t pos = music_player_get_position_ms();
    uint32_t dur = music_player_get_duration_ms();

    /* time_cur */
    if(g_h.time_cur) {
        if(g_mode == AM_PLAYER_MODE_STREAM && dur == 0) {
            lv_label_set_text(g_h.time_cur, "LIVE");
        } else if(dur == 0) {
            lv_label_set_text(g_h.time_cur, "--:--");
        } else {
            char buf[8];
            snprintf(buf, sizeof(buf), "%u:%02u", pos / 60000u, (pos / 1000u) % 60u);
            lv_label_set_text(g_h.time_cur, buf);
        }
    }

    /* time_total */
    if(g_h.time_total) {
        if(dur == 0) {
            lv_label_set_text(g_h.time_total, "--:--");
        } else {
            char buf[8];
            snprintf(buf, sizeof(buf), "%u:%02u", dur / 60000u, (dur / 1000u) % 60u);
            lv_label_set_text(g_h.time_total, buf);
        }
    }

    /* progress fill: 仅在 dur > 0 时更新宽度 */
    if(g_h.progress_fill && dur > 0) {
        int pct = (int)((uint64_t)pos * 100ull / dur);
        if(pct > 100) pct = 100;
        lv_obj_set_width(g_h.progress_fill, LV_PCT(pct));
    }
}

/* ── timer callback (主线程，100ms) ──────────────────────────────────── */

static void am_player_timer_cb(lv_timer_t *t) {
    (void)t;
    if(g_mode == AM_PLAYER_MODE_IDLE) return;

    /* 检测曲目切换 */
    size_t idx = music_player_get_current_index();
    if(idx != g_last_idx) {
        g_last_idx = idx;
        refresh_title();
    }

    /* 检测播放状态变化 */
    bool playing = music_player_is_playing();
    if(playing != g_last_playing) {
        g_last_playing = playing;
        refresh_play_icon(playing);
    }

    refresh_progress();
}

/* ── public API ───────────────────────────────────────────────────────── */

void am_player_init(void) {
    memset(&g_h, 0, sizeof(g_h));
    g_mode         = AM_PLAYER_MODE_IDLE;
    g_stream_title[0] = '\0';
    g_last_idx     = (size_t)-1;
    g_last_playing = false;
    if(g_timer) { lv_timer_delete(g_timer); g_timer = NULL; }
    g_timer = lv_timer_create(am_player_timer_cb, 100, NULL);
}

void am_player_deinit(void) {
    size_t i;
    if(g_timer) { lv_timer_delete(g_timer); g_timer = NULL; }
    music_player_deinit();
    for(i = 0; i < g_local_count; i++) { free(g_local_urls[i]); g_local_urls[i] = NULL; }
    g_local_count = 0;
    memset(&g_h, 0, sizeof(g_h));
}

void am_player_bind_miniplayer(const am_miniplayer_handles_t *h) {
    if(!h) { memset(&g_h, 0, sizeof(g_h)); return; }
    g_h = *h;
    /* 立即刷新 */
    refresh_title();
    refresh_play_icon(music_player_is_playing());
    refresh_progress();
}

void am_player_load_local(const char **urls, size_t count, size_t start_index) {
    size_t i;
    for(i = 0; i < g_local_count; i++) { free(g_local_urls[i]); g_local_urls[i] = NULL; }
    g_local_count = 0;
    for(i = 0; i < count && i < AM_PLAYER_LOCAL_MAX; i++) {
        g_local_urls[i] = strdup(urls[i]);
        if(g_local_urls[i]) g_local_count++;
    }
    g_mode     = AM_PLAYER_MODE_LOCAL;
    g_last_idx = (size_t)-1;
    music_player_deinit();
    if(g_local_count > 0) {
        music_player_init((const char **)g_local_urls, g_local_count);
        if(start_index > 0 && start_index < g_local_count) {
            music_player_select(start_index);
        }
    }
}

void am_player_play_stream(const char *url, const char *title) {
    const char *one[1];
    strncpy(g_stream_title, title ? title : "", sizeof(g_stream_title) - 1);
    g_stream_title[sizeof(g_stream_title) - 1] = '\0';
    g_mode     = AM_PLAYER_MODE_STREAM;
    g_last_idx = (size_t)-1;
    music_player_deinit();
    one[0] = url;
    music_player_init(one, 1);
}

void am_player_on_play_pause(lv_event_t *e) {
    (void)e;
    if(music_player_is_playing()) music_player_pause();
    else                          music_player_resume();
}

void am_player_on_prev(lv_event_t *e) { (void)e; music_player_prev(); }
void am_player_on_next(lv_event_t *e) { (void)e; music_player_next(); }
```

- [ ] **Step 5.3：编译验证**

```bash
cmake --build build -j$(nproc) 2>&1 | grep -E "error:|warning:" | head -20
```

Expected: 0 个 error。

- [ ] **Step 5.4：提交**

```bash
git add main/src/v9_apple_music/am_player.h \
        main/src/v9_apple_music/am_player.c
git commit -m "feat(player): add am_player adapter layer with lv_timer polling"
```

---

## Task 6：am_shell — mini-player 返回句柄 + 按钮事件绑定

**Files:**
- Modify: `main/src/v9_apple_music/am_shell.h`
- Modify: `main/src/v9_apple_music/am_shell.c`

- [ ] **Step 6.1：修改 `main/src/v9_apple_music/am_shell.h`**

将：

```c
/* 在 player 容器内构建迷你播放条 */
void am_shell_build_miniplayer(lv_obj_t *player);
```

替换为：

```c
#include "am_player.h"

/* 在 player 容器内构建迷你播放条，返回关键 widget 句柄 */
am_miniplayer_handles_t am_shell_build_miniplayer(lv_obj_t *player);
```

- [ ] **Step 6.2：修改 `main/src/v9_apple_music/am_shell.c`**

**6.2a：新增 include：**

在文件顶部现有 `#include` 行之后添加：

```c
#include "am_player.h"
```

**6.2b：修改函数签名：**

将 `void am_shell_build_miniplayer(lv_obj_t *player)` 改为：

```c
am_miniplayer_handles_t am_shell_build_miniplayer(lv_obj_t *player)
```

**6.2c：在函数开头声明 handles 结构体：**

在函数体第一行（`const am_metrics_t *m = am_metrics();`之前）添加：

```c
am_miniplayer_handles_t h;
memset(&h, 0, sizeof(h));
```

**6.2d：保存 title_label 句柄。**

在构建 `copy_title` 的行：

```c
lv_obj_t *copy_title = am_text(copy, am_mini.title, m->f_body, AM_TEXT);
```

修改为：

```c
lv_obj_t *copy_title = am_text(copy, am_mini.title, m->f_body, AM_TEXT);
h.title_label = copy_title;
```

**6.2e：保存 subtitle_label 句柄。**

在构建 `copy_sub` 的行：

```c
lv_obj_t *copy_sub = am_text(copy, am_mini.subtitle, m->f_label, AM_MUTED);
```

修改为：

```c
lv_obj_t *copy_sub = am_text(copy, am_mini.subtitle, m->f_label, AM_MUTED);
h.subtitle_label = copy_sub;
```

**6.2f：找到构建 play button 的 `ctrl_btn` 调用（中间那个，带 `use_grad3=true` 的），保存 play 图标句柄：**

构建 play 按钮的代码：

```c
ctrl_btn(ctrls, btn_play, LV_OPA_COVER, m->f_icon, AM_ICON_PLAY, AM_WHITE,
         true, t->hero_a, t->hero_b, t->hero_c);
```

`ctrl_btn` 返回按钮对象，但当前丢弃了返回值。修改为：

```c
lv_obj_t *play_btn = ctrl_btn(ctrls, btn_play, LV_OPA_COVER, m->f_icon, AM_ICON_PLAY, AM_WHITE,
                               true, t->hero_a, t->hero_b, t->hero_c);
lv_obj_add_event_cb(play_btn, am_player_on_play_pause, LV_EVENT_CLICKED, NULL);
/* play_icon 是 ctrl_btn 内创建的 label，它是 play_btn 的第一个子 */
h.play_icon = lv_obj_get_child(play_btn, 0);
```

**6.2g：同样为 prev/next 按钮添加事件回调：**

将 prev 按钮：

```c
ctrl_btn(ctrls, btn_skip, 184, m->f_icon, AM_ICON_PREV, AM_MUTED_STRONG,
         false, t->hero_a, t->hero_b, t->hero_c);
```

修改为：

```c
lv_obj_t *prev_btn = ctrl_btn(ctrls, btn_skip, 184, m->f_icon, AM_ICON_PREV, AM_MUTED_STRONG,
                               false, t->hero_a, t->hero_b, t->hero_c);
lv_obj_add_event_cb(prev_btn, am_player_on_prev, LV_EVENT_CLICKED, NULL);
```

next 按钮同理：

```c
lv_obj_t *next_btn = ctrl_btn(ctrls, btn_skip, 184, m->f_icon, AM_ICON_NEXT, AM_MUTED_STRONG,
                               false, t->hero_a, t->hero_b, t->hero_c);
lv_obj_add_event_cb(next_btn, am_player_on_next, LV_EVENT_CLICKED, NULL);
```

**6.2h：保存进度条句柄。**

在构建 `meta` 时间文字处，保存 time_cur 和 time_total：

```c
lv_obj_t *lbl_cur   = am_text(meta, am_mini.current, m->f_label, AM_MUTED);
lv_obj_t *lbl_mid   = am_text(meta, "Now Playing",   m->f_label, AM_MUTED);
lv_obj_t *lbl_total = am_text(meta, am_mini.total,   m->f_label, AM_MUTED);
h.time_cur   = lbl_cur;
h.time_total = lbl_total;
(void)lbl_mid;
```

（原代码用 `am_text(meta, am_mini.current, ...)` 等三行，将其改为保存返回值的版本。）

**6.2i：保存 progress_fill 句柄。**

在构建 `fill` 之后：

```c
lv_obj_t *fill = lv_obj_create(rail);
/* ... 原有的 fill 样式代码 ... */
h.progress_fill = fill;
```

**6.2j：函数末尾返回 handles：**

将函数最后的隐式 return 改为：

```c
    return h;
}
```

- [ ] **Step 6.3：编译验证**

```bash
cmake --build build -j$(nproc) 2>&1 | grep -E "error:" | head -20
```

Expected: 0 个 error。（apple_music.c 中调用 `am_shell_build_miniplayer` 的地方需在下一个 task 更新，此时可能有警告但不应有 error。）

- [ ] **Step 6.4：提交**

```bash
git add main/src/v9_apple_music/am_shell.h \
        main/src/v9_apple_music/am_shell.c
git commit -m "feat(shell): miniplayer returns widget handles and wires control buttons"
```

---

## Task 7：am_page_local + am_page_radio — 真实数据页面

**Files:**
- Create: `main/src/v9_apple_music/am_page_local.h`
- Create: `main/src/v9_apple_music/am_page_local.c`

- [ ] **Step 7.1：创建 `main/src/v9_apple_music/am_page_local.h`**

```c
/* main/src/v9_apple_music/am_page_local.h */
#ifndef AM_PAGE_LOCAL_H
#define AM_PAGE_LOCAL_H

#include "lvgl/lvgl.h"
#include "am_config.h"

/* 扫描 cfg->local_dir，构建本地文件列表页（点击播放）*/
lv_obj_t *am_page_local_create(lv_obj_t *content_parent, const am_config_t *cfg);

/* 从 cfg->radio[] 构建广播频道列表页（点击切流）*/
lv_obj_t *am_page_radio_create(lv_obj_t *content_parent, const am_config_t *cfg);

#endif /* AM_PAGE_LOCAL_H */
```

- [ ] **Step 7.2：创建 `main/src/v9_apple_music/am_page_local.c`**

```c
/* main/src/v9_apple_music/am_page_local.c */
#include "am_page_local.h"
#include "am_player.h"
#include "am_widgets.h"
#include "am_theme.h"
#include "am_fonts.h"
#include "am_metrics.h"
#include "stream_player.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* lv_event_cb_t wrapper：释放 lv_event_get_user_data 指向的 lv_malloc 块 */
static void am_obj_user_data_free_cb(lv_event_t *e) {
    lv_free(lv_event_get_user_data(e));
}

/* ─── 共用 helper：构建简单列表行 ──────────────────────────────────── */

static lv_obj_t *make_list_row(lv_obj_t *parent,
                                const char *title, const char *subtitle,
                                const am_metrics_t *m, const am_theme_t *t)
{
    lv_obj_t *row = am_card(parent, 10, (lv_opa_t)(0.44f * 255));
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(row, 10, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(row, 3, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = am_text(row, title, m->f_body, AM_TEXT);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
    lv_obj_set_width(lbl, LV_PCT(100));

    if(subtitle && subtitle[0]) {
        lv_obj_t *sub = am_text(row, subtitle, m->f_label, AM_MUTED);
        lv_label_set_long_mode(sub, LV_LABEL_LONG_DOT);
        lv_obj_set_width(sub, LV_PCT(100));
        (void)sub;
    }
    (void)t;
    return row;
}

/* ─── 本地页 ────────────────────────────────────────────────────────── */

typedef struct {
    char **urls;
    size_t count;
} local_page_ctx_t;

typedef struct {
    local_page_ctx_t *page;
    size_t            index;
} local_item_ctx_t;

static void local_page_delete_cb(lv_event_t *e) {
    local_page_ctx_t *ctx = (local_page_ctx_t *)lv_event_get_user_data(e);
    size_t i;
    for(i = 0; i < ctx->count; i++) free(ctx->urls[i]); /* strdup → free */
    free(ctx->urls);                                      /* realloc → free */
    lv_free(ctx);                                         /* lv_malloc → lv_free */
}

static void local_item_click_cb(lv_event_t *e) {
    local_item_ctx_t *ctx = (local_item_ctx_t *)lv_event_get_user_data(e);
    am_player_load_local((const char **)ctx->page->urls, ctx->page->count, ctx->index);
}

static int cmp_str(const void *a, const void *b) {
    return strcmp(*(const char * const *)a, *(const char * const *)b);
}

lv_obj_t *am_page_local_create(lv_obj_t *content_parent, const am_config_t *cfg) {
    const am_metrics_t *m = am_metrics();
    const am_theme_t   *t = am_theme_get(am_theme_current());

    lv_obj_t *page = lv_obj_create(content_parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(page, m->page_gap, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    /* section title */
    am_section_title(page, "本地音乐");

    if(!cfg || !cfg->local_dir[0]) {
        am_text(page, "请在 am_config.json 中配置 local.dir", m->f_body, AM_MUTED);
        return page;
    }

    /* scan directory */
    DIR *d = opendir(cfg->local_dir);
    if(!d) {
        am_text(page, "目录不存在或无法读取", m->f_body, AM_MUTED);
        return page;
    }

    char **file_urls = NULL;
    size_t file_count = 0, file_cap = 0;
    struct dirent *entry;

    while((entry = readdir(d)) != NULL) {
        char full[1024];
        if(entry->d_name[0] == '.') continue;
        snprintf(full, sizeof(full), "%s/%s", cfg->local_dir, entry->d_name);
        if(!stream_player_is_supported_local_audio_file(full)) continue;
        if(file_count >= file_cap) {
            size_t new_cap = (file_cap == 0) ? 16 : file_cap * 2;
            char **tmp = (char **)realloc(file_urls, new_cap * sizeof(char *));
            if(!tmp) break;
            file_urls = tmp;
            file_cap  = new_cap;
        }
        file_urls[file_count] = strdup(full);
        if(file_urls[file_count]) file_count++;
    }
    closedir(d);

    if(file_count == 0) {
        free(file_urls);
        am_text(page, "暂无支持的音频文件", m->f_body, AM_MUTED);
        return page;
    }

    qsort(file_urls, file_count, sizeof(char *), cmp_str);

    /* page-level context (freed on page delete) */
    local_page_ctx_t *page_ctx = (local_page_ctx_t *)lv_malloc(sizeof(local_page_ctx_t));
    page_ctx->urls  = file_urls;
    page_ctx->count = file_count;
    lv_obj_add_event_cb(page, local_page_delete_cb, LV_EVENT_DELETE, page_ctx);

    /* build list rows */
    size_t i;
    for(i = 0; i < file_count; i++) {
        /* title = basename without extension */
        char copy[512];
        strncpy(copy, file_urls[i], sizeof(copy) - 1);
        copy[sizeof(copy) - 1] = '\0';
        char *b   = basename(copy);
        char  title[128];
        strncpy(title, b, sizeof(title) - 1);
        title[sizeof(title) - 1] = '\0';
        char *dot = strrchr(title, '.');
        if(dot) *dot = '\0';

        lv_obj_t *row = make_list_row(page, title, file_urls[i], m, t);

        /* item-level click context (lifespan = row lifespan) */
        local_item_ctx_t *ictx = (local_item_ctx_t *)lv_malloc(sizeof(local_item_ctx_t));
        ictx->page  = page_ctx;
        ictx->index = i;
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(row, local_item_click_cb, LV_EVENT_CLICKED, ictx);
        /* free ictx when row is deleted (wrapper needed: lv_free takes void*, not lv_event_t*) */
        lv_obj_add_event_cb(row, (lv_event_cb_t)am_obj_user_data_free_cb, LV_EVENT_DELETE, ictx);
    }

    return page;
}

/* ─── 广播页 ────────────────────────────────────────────────────────── */

typedef struct {
    char url[512];
    char title[128];
} radio_item_ctx_t;

static void radio_item_click_cb(lv_event_t *e) {
    radio_item_ctx_t *ctx = (radio_item_ctx_t *)lv_event_get_user_data(e);
    am_player_play_stream(ctx->url, ctx->title);
}

lv_obj_t *am_page_radio_create(lv_obj_t *content_parent, const am_config_t *cfg) {
    const am_metrics_t *m = am_metrics();
    const am_theme_t   *t = am_theme_get(am_theme_current());

    lv_obj_t *page = lv_obj_create(content_parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(page, m->page_gap, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    am_section_title(page, "广播频道");

    if(!cfg || cfg->radio_count == 0) {
        am_text(page, "请在 am_config.json 中配置 radio 列表", m->f_body, AM_MUTED);
        return page;
    }

    int i;
    for(i = 0; i < cfg->radio_count; i++) {
        lv_obj_t *row = make_list_row(page,
                                      cfg->radio[i].title,
                                      cfg->radio[i].subtitle,
                                      m, t);

        radio_item_ctx_t *ictx = (radio_item_ctx_t *)lv_malloc(sizeof(radio_item_ctx_t));
        strncpy(ictx->url,   cfg->radio[i].url,   sizeof(ictx->url)   - 1);
        strncpy(ictx->title, cfg->radio[i].title, sizeof(ictx->title) - 1);
        ictx->url[sizeof(ictx->url) - 1]     = '\0';
        ictx->title[sizeof(ictx->title) - 1] = '\0';

        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(row, radio_item_click_cb, LV_EVENT_CLICKED, ictx);
        lv_obj_add_event_cb(row, (lv_event_cb_t)am_obj_user_data_free_cb, LV_EVENT_DELETE, ictx);
    }

    return page;
}
```

- [ ] **Step 7.3：编译验证**

```bash
cmake --build build -j$(nproc) 2>&1 | grep -E "error:" | head -20
```

Expected: 0 个 error。

- [ ] **Step 7.4：提交**

```bash
git add main/src/v9_apple_music/am_page_local.h \
        main/src/v9_apple_music/am_page_local.c
git commit -m "feat(pages): add am_page_local_create and am_page_radio_create with real data"
```

---

## Task 8：apple_music.c — 接通 config + player + 真实页面

**Files:**
- Modify: `main/src/v9_apple_music/apple_music.c`

- [ ] **Step 8.1：新增 include 和静态状态变量**

在现有 `#include` 之后添加：

```c
#include "am_config.h"
#include "am_player.h"
#include "am_page_local.h"
```

在 `static am_page_e s_page` 声明之后添加：

```c
static am_config_t s_config;
```

- [ ] **Step 8.2：修改 `rebuild_content` 中本地和广播页的分支**

将：

```c
case AM_PAGE_RADIO:    am_page_list_create(s_content, &am_page_radio);                                break;
case AM_PAGE_LOCAL:    am_page_list_create(s_content, &am_page_local);                                break;
```

替换为：

```c
case AM_PAGE_RADIO:    am_page_radio_create(s_content, &s_config);  break;
case AM_PAGE_LOCAL:    am_page_local_create(s_content, &s_config);  break;
```

- [ ] **Step 8.3：修改 `build_all`：mini-player 返回句柄后 bind**

将：

```c
am_shell_build_miniplayer(s_player);
```

替换为：

```c
am_miniplayer_handles_t h = am_shell_build_miniplayer(s_player);
am_player_bind_miniplayer(&h);
```

- [ ] **Step 8.4：修改 `apple_music_create`：加载 config + 初始化 player**

将：

```c
void apple_music_create(void)
{
    apply_env_initial_state();
    build_all();
}
```

替换为：

```c
void apple_music_create(void)
{
    am_config_load(&s_config);   /* 失败时 s_config 保持全零，页面显示占位 */
    am_player_init();
    apply_env_initial_state();
    build_all();
}
```

- [ ] **Step 8.5：编译验证**

```bash
cmake --build build -j$(nproc) 2>&1 | grep -E "error:" | head -20
```

Expected: 0 个 error。

- [ ] **Step 8.6：提交**

```bash
git add main/src/v9_apple_music/apple_music.c
git commit -m "feat(apple_music): wire config load, player init, and real page data"
```

---

## Task 9：main.c 清理 + 最终构建验证

**Files:**
- Modify: `main/src/main.c`

- [ ] **Step 9.1：清理注释掉的 music_player 调用**

删除以下注释行（这些调用已由 am_player_init/deinit 接管）：

```c
  // music_player_init((const char **)(argc > 1 ? argv + 1 : NULL),
  //                   (size_t)(argc > 1 ? argc - 1 : 0));
```

和：

```c
  // music_player_deinit();
```

- [ ] **Step 9.2：完整构建**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build -j$(nproc)
```

Expected: `[100%] Linking C executable main`，无 error。

- [ ] **Step 9.3：运行所有单元测试**

```bash
cd build && ctest -V
```

Expected: 所有测试通过，包括新增的 `am_config_test`。

- [ ] **Step 9.4：手动验证 mini-player**

创建示例配置文件（按需修改路径）：

```bash
cat > am_config.json << 'EOF'
{
  "local": { "dir": "/home/user/Music" },
  "radio": [
    { "title": "Lo-Fi 电台", "subtitle": "低压氛围", "url": "https://example.com/lofi.m3u8" }
  ]
}
EOF
```

运行：

```bash
./bin/main 800 480
```

验证：
- mini-player 的 prev/play/next 按钮有点击反馈
- 本地页面显示 `/home/user/Music` 目录下的音频文件列表（若目录有文件）
- 广播页面显示 Lo-Fi 电台频道行
- 点击本地文件开始播放，mini-player 标题和进度条随之更新

- [ ] **Step 9.5：截图验证（可选，CI 用）**

```bash
AM_PAGE=local AM_CONFIG=./am_config.json AM_SHOT=/tmp/am_local.png ./bin/main 800 480
AM_PAGE=radio AM_CONFIG=./am_config.json AM_SHOT=/tmp/am_radio.png ./bin/main 800 480
```

- [ ] **Step 9.6：提交**

```bash
git add main/src/main.c
git commit -m "chore(main): remove commented-out music_player calls (now owned by am_player)"
```

---

## 验收标准

| 功能 | 预期行为 |
|------|---------|
| 本地文件播放 | 点击本地页列表项，mini-player 标题更新为文件名（无扩展名），进度条实时推进 |
| 本地文件上一首/下一首 | prev/next 按钮切换曲目，mini-player 标题随之变化 |
| 播放/暂停切换 | play 按钮图标在 ▶ 和 \|\| 之间切换，音频响应 |
| 广播流播放 | 点击广播频道，mini-player 显示频道名，进度区显示 `LIVE` / `--:--` |
| 主题切换不丢句柄 | 切换主题后 mini-player 按钮仍可用 |
| 配置文件缺失 | 两页显示占位文字，mini-player 静态展示，不崩溃 |
| VBR/未知格式 | 进度条不更新，时间显示 `--:--`，不崩溃 |
