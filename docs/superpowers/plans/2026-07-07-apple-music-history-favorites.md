# Apple Music 最近播放与收藏持久化 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 `main/src/v9_apple_music` 增加真实的最近播放与收藏状态，让侧边栏和收藏页不再依赖写死数据，并且让这两类状态跨重启保留。

**Architecture:** 新增一个只服务 `v9_apple_music` 的轻量状态模块，使用 `apple_music_state.tsv` 以歌曲 `path` 为主键持久化 `favorite` 和 `recent_seq`。运行时先扫描本地歌曲，再把状态文件按 `path` 回灌到 `am_local_item_t`；播放本地歌曲时更新 `recent_seq` 并写盘，`正在播放` 页点击收藏按钮时切换 `favorite` 并写盘，侧边栏与收藏页都从这份合并后的内存状态生成。

**Tech Stack:** C99、LVGL v9、现有 `v9_apple_music` 组件、CMake/CTest、标准 C 文件 I/O

## Global Constraints

- 侧边栏 `最近播放` 显示真实本地播放历史，而不是静态文案。
- `最近播放` 最多显示 4 条，按最近播放时间倒序排列。
- `我的收藏` 页面显示真实收藏歌曲，不再依赖默认前 4 首。
- 在 `正在播放` 页提供收藏按钮，用于切换当前本地歌曲的收藏状态。
- 收藏状态与最近播放历史都需要写入本地状态文件，重启后可恢复。
- 不实现广播电台收藏。
- 不让广播电台进入最近播放历史。
- 不单独持久化“播放列表/当前队列”；播放列表仍来自启动时扫描到的本地歌曲列表。
- 不改动当前音频扫描目录、播放引擎或广播数据源。
- 不引入数据库、JSON 解析库或复杂配置系统。
- 状态文件固定放在应用进程当前工作目录下，文件名为 `apple_music_state.tsv`。

---

## 文件职责

- `main/src/v9_apple_music/am_data.h`
  - 扩展 `am_local_item_t` 结构，声明状态模块需要的共享类型。
- `main/src/v9_apple_music/am_data.c`
  - 删除 `am_recent_titles[4]` 静态最近播放假数据，只保留仍被 UI 使用的常量。
- `main/src/v9_apple_music/am_state.h`
  - 暴露状态文件加载、保存、最近播放收集、最近播放标记、收藏切换接口。
- `main/src/v9_apple_music/am_state.c`
  - 解析和写入 `apple_music_state.tsv`，按 `path` 合并状态，维护 `recent_seq` 逻辑。
- `main/src/v9_apple_music/am_local_scan.c`
  - 取消“前 4 首默认收藏”的写死逻辑，只生成基础扫描结果。
- `main/src/v9_apple_music/am_shell.h`
  - 扩展侧边栏构建函数签名，让其接收真实本地歌曲列表。
- `main/src/v9_apple_music/am_shell.c`
  - 根据本地歌曲状态渲染侧边栏最近播放。
- `main/src/v9_apple_music/apple_music.c`
  - 负责加载状态、在本地播放发生时记录最近播放、在 `正在播放` 页创建收藏按钮并触发 UI 刷新。
- `main/tests/apple_music_state_test.c`
  - 覆盖状态模块的纯数据读写与排序行为。
- `main/tests/apple_music_data_test.c`
  - 覆盖本地扫描默认值，防止收藏和最近播放再次回退到写死初始化。
- `main/tests/apple_music_now_view_state_test.c`
  - 覆盖侧边栏最近播放渲染和 `正在播放` 页收藏按钮显隐。
- `CMakeLists.txt`
  - 注册新增测试目标并加入 `ctest`。

### Task 1: 新增状态模块并用单测锁定文件格式与最近播放排序

**Files:**
- Modify: `main/src/v9_apple_music/am_data.h`
- Create: `main/src/v9_apple_music/am_state.h`
- Create: `main/src/v9_apple_music/am_state.c`
- Create: `main/tests/apple_music_state_test.c`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes:
  - `typedef struct { char path[1024]; char title[256]; bool favorite; uint64_t recent_seq; } am_local_item_t;`
- Produces:
  - `#define AM_STATE_PATH "apple_music_state.tsv"`
  - `int am_state_load(const char *path, am_local_item_t *items, size_t count, uint64_t *max_recent_seq);`
  - `int am_state_save(const char *path, const am_local_item_t *items, size_t count);`
  - `size_t am_state_collect_recent(const am_local_item_t *items, size_t count, size_t *out_indices, size_t out_cap);`
  - `void am_state_mark_recent(am_local_item_t *items, size_t count, size_t index, uint64_t *next_recent_seq);`
  - `bool am_state_toggle_favorite(am_local_item_t *items, size_t count, size_t index);`

- [ ] **Step 1: 先写状态模块失败测试，锁定 round-trip、坏行跳过和最近播放排序**

```c
static void fill_item(am_local_item_t *item, const char *path, const char *title,
                      bool favorite, uint64_t recent_seq)
{
    memset(item, 0, sizeof(*item));
    snprintf(item->path, sizeof(item->path), "%s", path);
    snprintf(item->title, sizeof(item->title), "%s", title);
    item->favorite = favorite;
    item->recent_seq = recent_seq;
}

static void test_state_roundtrip(void)
{
    am_local_item_t items[3];
    am_local_item_t loaded[3];
    uint64_t max_recent = 0U;
    char state_path[] = "/tmp/am_state_roundtrip_XXXXXX";
    int fd = mkstemp(state_path);
    assert(fd >= 0);
    close(fd);
    unlink(state_path);

    fill_item(&items[0], "/music/a.mp3", "A", true, 7U);
    fill_item(&items[1], "/music/b.mp3", "B", false, 11U);
    fill_item(&items[2], "/music/c.mp3", "C", true, 0U);

    memset(loaded, 0, sizeof(loaded));
    fill_item(&loaded[0], "/music/a.mp3", "A", false, 0U);
    fill_item(&loaded[1], "/music/b.mp3", "B", false, 0U);
    fill_item(&loaded[2], "/music/c.mp3", "C", false, 0U);

    assert(am_state_save(state_path, items, 3U) == 0);
    assert(am_state_load(state_path, loaded, 3U, &max_recent) == 0);
    assert(loaded[0].favorite == true);
    assert(loaded[1].recent_seq == 11U);
    assert(loaded[2].favorite == true);
    assert(max_recent == 11U);
    unlink(state_path);
}

static void test_collect_recent_top4(void)
{
    am_local_item_t items[5];
    size_t out[4] = {0};

    fill_item(&items[0], "/music/a.mp3", "A", false, 2U);
    fill_item(&items[1], "/music/b.mp3", "B", false, 9U);
    fill_item(&items[2], "/music/c.mp3", "C", false, 0U);
    fill_item(&items[3], "/music/d.mp3", "D", false, 5U);
    fill_item(&items[4], "/music/e.mp3", "E", false, 8U);

    assert(am_state_collect_recent(items, 5U, out, 4U) == 4U);
    assert(out[0] == 1U);
    assert(out[1] == 4U);
    assert(out[2] == 3U);
    assert(out[3] == 0U);
}

static void test_load_skips_malformed_lines(void)
{
    am_local_item_t items[2];
    uint64_t max_recent = 0U;
    char state_path[] = "/tmp/am_state_badline_XXXXXX";
    FILE *fp;
    int fd = mkstemp(state_path);
    assert(fd >= 0);
    fp = fdopen(fd, "w");
    assert(fp != NULL);
    fprintf(fp, "/music/a.mp3\t1\t4\n");
    fprintf(fp, "bad line without tabs\n");
    fprintf(fp, "/music/b.mp3\t0\t12\n");
    fclose(fp);

    fill_item(&items[0], "/music/a.mp3", "A", false, 0U);
    fill_item(&items[1], "/music/b.mp3", "B", true, 1U);

    assert(am_state_load(state_path, items, 2U, &max_recent) == 0);
    assert(items[0].favorite == true);
    assert(items[1].favorite == false);
    assert(items[1].recent_seq == 12U);
    assert(max_recent == 12U);
    unlink(state_path);
}
```

- [ ] **Step 2: 运行测试，确认当前实现失败**

Run: `cmake --build build --target apple_music_state_test && ./bin/apple_music_state_test`

Expected: 编译失败，提示 `am_state_*` 或 `recent_seq` 未定义

- [ ] **Step 3: 先补共享结构和状态模块头文件**

```c
typedef struct {
    char path[1024];
    char title[256];
    bool favorite;
    uint64_t recent_seq;
} am_local_item_t;
```

```c
#ifndef AM_STATE_H
#define AM_STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "am_data.h"

#define AM_STATE_PATH "apple_music_state.tsv"

int am_state_load(const char *path, am_local_item_t *items, size_t count, uint64_t *max_recent_seq);
int am_state_save(const char *path, const am_local_item_t *items, size_t count);
size_t am_state_collect_recent(const am_local_item_t *items, size_t count, size_t *out_indices, size_t out_cap);
void am_state_mark_recent(am_local_item_t *items, size_t count, size_t index, uint64_t *next_recent_seq);
bool am_state_toggle_favorite(am_local_item_t *items, size_t count, size_t index);

#endif /* AM_STATE_H */
```

- [ ] **Step 4: 用最小实现补齐状态读写与最近播放工具函数**

```c
static am_local_item_t *find_item_by_path(am_local_item_t *items, size_t count, const char *path)
{
    size_t i;
    for(i = 0; i < count; i++) {
        if(strcmp(items[i].path, path) == 0) return &items[i];
    }
    return NULL;
}

int am_state_load(const char *path, am_local_item_t *items, size_t count, uint64_t *max_recent_seq)
{
    FILE *fp;
    char line[1600];

    if(max_recent_seq != NULL) *max_recent_seq = 0U;
    fp = fopen(path, "r");
    if(fp == NULL) return 0;

    while(fgets(line, sizeof(line), fp) != NULL) {
        char rec_path[1024];
        unsigned favorite = 0U;
        unsigned long long recent_seq = 0ULL;
        am_local_item_t *item;

        if(sscanf(line, "%1023[^\t]\t%u\t%llu", rec_path, &favorite, &recent_seq) != 3) continue;
        item = find_item_by_path(items, count, rec_path);
        if(item == NULL) continue;
        item->favorite = (favorite != 0U);
        item->recent_seq = (uint64_t)recent_seq;
        if(max_recent_seq != NULL && item->recent_seq > *max_recent_seq) *max_recent_seq = item->recent_seq;
    }

    fclose(fp);
    return 0;
}

int am_state_save(const char *path, const am_local_item_t *items, size_t count)
{
    FILE *fp;
    size_t i;

    fp = fopen(path, "w");
    if(fp == NULL) return -1;
    for(i = 0; i < count; i++) {
        if(items[i].path[0] == '\0') continue;
        fprintf(fp, "%s\t%u\t%llu\n",
                items[i].path,
                items[i].favorite ? 1U : 0U,
                (unsigned long long)items[i].recent_seq);
    }
    fclose(fp);
    return 0;
}
```

```c
size_t am_state_collect_recent(const am_local_item_t *items, size_t count, size_t *out_indices, size_t out_cap)
{
    size_t i, used = 0U, j;
    for(i = 0; i < count; i++) {
        if(items[i].recent_seq == 0U) continue;
        for(j = used; j > 0U; j--) {
            if(items[out_indices[j - 1U]].recent_seq >= items[i].recent_seq) break;
            if(j < out_cap) out_indices[j] = out_indices[j - 1U];
        }
        if(j < out_cap) out_indices[j] = i;
        if(used < out_cap) used++;
    }
    return used;
}

void am_state_mark_recent(am_local_item_t *items, size_t count, size_t index, uint64_t *next_recent_seq)
{
    if(items == NULL || next_recent_seq == NULL || index >= count) return;
    (*next_recent_seq)++;
    items[index].recent_seq = *next_recent_seq;
}

bool am_state_toggle_favorite(am_local_item_t *items, size_t count, size_t index)
{
    if(items == NULL || index >= count) return false;
    items[index].favorite = !items[index].favorite;
    return items[index].favorite;
}
```

- [ ] **Step 5: 注册测试目标**

```cmake
add_executable(apple_music_state_test
    ${PROJECT_SOURCE_DIR}/main/tests/apple_music_state_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_state.c
)
target_include_directories(apple_music_state_test PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/main/src
)
target_link_libraries(apple_music_state_test lvgl ${PNG_LIBRARIES})

add_test(
    NAME apple_music_state_test
    COMMAND $<TARGET_FILE:apple_music_state_test>
)
set_tests_properties(apple_music_state_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
```

- [ ] **Step 6: 重新运行状态测试，确认通过**

Run: `cmake --build build --target apple_music_state_test && ./bin/apple_music_state_test`

Expected: PASS，无 stderr 输出

- [ ] **Step 7: 提交状态模块**

```bash
git add main/src/v9_apple_music/am_data.h \
        main/src/v9_apple_music/am_state.h \
        main/src/v9_apple_music/am_state.c \
        main/tests/apple_music_state_test.c \
        CMakeLists.txt
git commit -m "feat: add apple music state persistence helpers"
```

### Task 2: 接入运行态加载与真实最近播放更新，去掉扫描阶段的假收藏

**Files:**
- Modify: `main/src/v9_apple_music/am_local_scan.c`
- Modify: `main/src/v9_apple_music/apple_music.c`
- Modify: `main/tests/apple_music_data_test.c`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes:
  - `int am_state_load(const char *path, am_local_item_t *items, size_t count, uint64_t *max_recent_seq);`
  - `int am_state_save(const char *path, const am_local_item_t *items, size_t count);`
  - `void am_state_mark_recent(am_local_item_t *items, size_t count, size_t index, uint64_t *next_recent_seq);`
  - `am_source_kind_t am_player_source_kind(void);`
  - `size_t am_player_current_local_index(void);`
- Produces:
  - `static uint64_t g_recent_seq_next;`
  - `static void am_record_current_local_playback(void);`
  - `static void am_save_local_state(void);`

- [ ] **Step 1: 先扩展数据测试，锁定扫描默认值不再隐式收藏**

```c
static void test_local_scan(void)
{
    am_local_item_t *local = NULL;
    size_t local_count = 0;
    int rc = am_local_scan_dir(
        "third-party/hls_player_demo/test_file",
        &local,
        &local_count
    );

    assert(rc == 0);
    assert(local != NULL);
    assert(local_count > 3);
    assert(local[0].favorite == false);
    assert(local[0].recent_seq == 0U);
    assert(local[1].favorite == false);

    am_local_scan_free(local);
}
```

- [ ] **Step 2: 运行数据测试，确认当前实现失败**

Run: `cmake --build build --target apple_music_data_test && ./bin/apple_music_data_test`

Expected: FAIL，`favorite == false` 断言失败

- [ ] **Step 3: 删掉扫描阶段的假收藏初始化**

```c
        memset(&buffer[used], 0, sizeof(buffer[used]));
        am_copy_text(buffer[used].path, sizeof(buffer[used].path), full_path);
        am_title_from_name(buffer[used].title, sizeof(buffer[used].title), entry->d_name);
        buffer[used].favorite = false;
        buffer[used].recent_seq = 0U;
        used++;
```

- [ ] **Step 4: 在 `apple_music.c` 中集中维护状态加载、保存和最近播放更新**

```c
static uint64_t g_recent_seq_next = 0U;

static void am_save_local_state(void)
{
    if(g_app.locals == NULL || g_app.local_count == 0U) return;
    if(am_state_save(AM_STATE_PATH, g_app.locals, g_app.local_count) != 0) {
        LV_LOG_WARN("apple music state save failed: %s", AM_STATE_PATH);
    }
}

static void am_record_current_local_playback(void)
{
    size_t index;

    if(am_player_source_kind() != AM_SOURCE_LOCAL) return;
    if(g_app.locals == NULL || g_app.local_count == 0U) return;

    index = am_player_current_local_index();
    if(index >= g_app.local_count) return;

    am_state_mark_recent(g_app.locals, g_app.local_count, index, &g_recent_seq_next);
    am_save_local_state();
}
```

```c
static void am_load_runtime_data(void)
{
    if(g_app.radios == NULL) {
        am_sources_csv_load("third-party/hls_player_demo/qa/production_test/config/sources.tsv",
                            &g_app.radios, &g_app.radio_count);
    }
    if(g_app.locals == NULL) {
        am_local_scan_dir("third-party/hls_player_demo/test_file", &g_app.locals, &g_app.local_count);
        g_recent_seq_next = 0U;
        if(g_app.locals != NULL && g_app.local_count > 0U) {
            am_state_load(AM_STATE_PATH, g_app.locals, g_app.local_count, &g_recent_seq_next);
        }
    }
    am_player_set_sources(g_app.locals, g_app.local_count, g_app.radios, g_app.radio_count);
}
```

- [ ] **Step 5: 在所有本地曲目切入路径后记录最近播放**

```c
static void am_on_local_selected(size_t index, void *user)
{
    LV_UNUSED(user);
    am_player_play_local_index(index);
    am_record_current_local_playback();
    am_sync_current_view();
}
```

```c
static void am_on_play_pause(lv_event_t *e)
{
    LV_UNUSED(e);
    am_player_toggle_playback();
    am_record_current_local_playback();
    if(g_app.current_view == AM_VIEW_NOW) am_update_now_view();
}
```

```c
static void am_on_prev(lv_event_t *e)
{
    LV_UNUSED(e);
    am_player_prev();
    am_record_current_local_playback();
    am_sync_current_view();
}

static void am_on_next(lv_event_t *e)
{
    LV_UNUSED(e);
    am_player_next();
    am_record_current_local_playback();
    am_sync_current_view();
}

static void am_on_playlist_pick(void)
{
    am_record_current_local_playback();
    am_show_view(AM_VIEW_NOW);
}
```

- [ ] **Step 6: 重新运行数据测试，确认扫描默认值已固定**

Run: `./bin/apple_music_data_test`

Expected: PASS

- [ ] **Step 7: 提交运行态接线**

```bash
git add main/src/v9_apple_music/am_local_scan.c \
        main/src/v9_apple_music/apple_music.c \
        main/tests/apple_music_data_test.c
git commit -m "feat: wire apple music runtime state updates"
```

### Task 3: 用真实状态驱动侧边栏最近播放，并在正在播放页加收藏按钮

**Files:**
- Modify: `main/src/v9_apple_music/am_data.c`
- Modify: `main/src/v9_apple_music/am_data.h`
- Modify: `main/src/v9_apple_music/am_shell.h`
- Modify: `main/src/v9_apple_music/am_shell.c`
- Modify: `main/src/v9_apple_music/apple_music.c`
- Create: `main/tests/apple_music_now_view_state_test.c`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes:
  - `size_t am_state_collect_recent(const am_local_item_t *items, size_t count, size_t *out_indices, size_t out_cap);`
  - `bool am_state_toggle_favorite(am_local_item_t *items, size_t count, size_t index);`
  - `int am_state_save(const char *path, const am_local_item_t *items, size_t count);`
- Produces:
  - `void am_shell_build_sidebar(lv_obj_t *sidebar, am_view_t active_view, const am_local_item_t *locals, size_t local_count, am_nav_cb_t cb, void *user);`
  - `static void am_on_toggle_favorite(lv_event_t *e);`
  - `lv_obj_t *favorite_btn;`
  - `lv_obj_t *favorite_icon;`

- [ ] **Step 1: 先写 UI 测试，锁定最近播放渲染和收藏按钮显隐**

```c
static uint32_t count_labels_with_text(lv_obj_t *root, const char *text)
{
    uint32_t count = 0U;
    uint32_t child_count = lv_obj_get_child_count(root);
    uint32_t i;

    if(lv_obj_check_type(root, &lv_label_class)) {
        const char *value = lv_label_get_text(root);
        if(value != NULL && strcmp(value, text) == 0) count++;
    }
    for(i = 0; i < child_count; i++) {
        count += count_labels_with_text(lv_obj_get_child(root, (int32_t)i), text);
    }
    return count;
}

static void test_recent_sidebar_and_local_favorite_button(void)
{
    /* 自定义 am_local_scan_dir stub 返回 Alpha/Beta/Gamma 三首歌；
       预写入 state 文件：Beta recent_seq=9, Alpha recent_seq=4, Gamma favorite=1；
       am_player_source_kind() stub 返回 AM_SOURCE_LOCAL，当前索引返回 2。 */
    apple_music_create_in(parent);
    lv_obj_update_layout(parent);

    assert(count_labels_with_text(parent, "Beta") >= 1U);
    assert(count_labels_with_text(parent, "Alpha") >= 1U);
    assert(count_labels_with_text(parent, AM_ICON_HEART) >= 2U);
}

static void test_radio_hides_now_view_favorite_button(void)
{
    /* 同一套 stub，仅把 am_player_source_kind() 改为 AM_SOURCE_RADIO。 */
    apple_music_create_in(parent);
    lv_obj_update_layout(parent);

    assert(count_labels_with_text(parent, AM_ICON_HEART) == 1U);
}
```

- [ ] **Step 2: 运行 UI 测试，确认当前实现失败**

Run: `cmake --build build --target apple_music_now_view_state_test && ./bin/apple_music_now_view_state_test`

Expected: FAIL，`Beta`/`Alpha` 不出现，或心形数量与断言不符

- [ ] **Step 3: 删除静态最近播放假数据，并改造侧边栏签名**

```c
extern const char *am_mock_lyrics[4];
```

```c
void am_shell_build_sidebar(lv_obj_t *sidebar,
                            am_view_t active_view,
                            const am_local_item_t *locals,
                            size_t local_count,
                            am_nav_cb_t cb,
                            void *user);
```

```c
void am_shell_build_sidebar(lv_obj_t *sidebar,
                            am_view_t active_view,
                            const am_local_item_t *locals,
                            size_t local_count,
                            am_nav_cb_t cb,
                            void *user)
{
    size_t recent[4] = {0};
    size_t recent_count = am_state_collect_recent(locals, local_count, recent, 4U);
    size_t i;
    ...
    am_section_title(sidebar, "最近播放");
    for(i = 0; i < recent_count; i++) {
        lv_obj_t *label = am_text(sidebar, locals[recent[i]].title, m->f_body, AM_MUTED);
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_set_width(label, LV_PCT(100));
        lv_obj_set_style_pad_left(label, 8, 0);
        lv_obj_set_style_pad_top(label, 5, 0);
    }
}
```

- [ ] **Step 4: 在 `正在播放` 页添加收藏按钮和收藏切换回调**

```c
static void am_on_toggle_favorite(lv_event_t *e)
{
    size_t index;
    LV_UNUSED(e);

    if(am_player_source_kind() != AM_SOURCE_LOCAL) return;
    index = am_player_current_local_index();
    if(g_app.locals == NULL || index >= g_app.local_count) return;

    am_state_toggle_favorite(g_app.locals, g_app.local_count, index);
    am_save_local_state();
    if(g_app.current_view == AM_VIEW_FAVORITES) am_request_view(AM_VIEW_FAVORITES);
    am_update_now_view();
    am_shell_build_sidebar(g_app.sidebar, g_app.current_view, g_app.locals, g_app.local_count, (am_nav_cb_t)am_show_view, NULL);
}
```

```c
    lv_obj_t *actions = lv_obj_create(info);
    lv_obj_remove_style_all(actions);
    lv_obj_set_size(actions, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(actions, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_top(actions, 8, 0);
    lv_obj_clear_flag(actions, LV_OBJ_FLAG_SCROLLABLE);

    g_app.favorite_btn = lv_obj_create(actions);
    lv_obj_remove_style_all(g_app.favorite_btn);
    lv_obj_set_size(g_app.favorite_btn, 28, 28);
    lv_obj_add_flag(g_app.favorite_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(g_app.favorite_btn, am_on_toggle_favorite, LV_EVENT_CLICKED, NULL);
    g_app.favorite_icon = am_text(g_app.favorite_btn, AM_ICON_HEART, m->f_icon, AM_MUTED);
    lv_obj_center(g_app.favorite_icon);
```

```c
    if(g_app.favorite_btn != NULL) {
        bool is_local = (am_player_source_kind() == AM_SOURCE_LOCAL);
        bool is_favorite = false;
        if(is_local && g_app.locals != NULL && am_player_current_local_index() < g_app.local_count) {
            is_favorite = g_app.locals[am_player_current_local_index()].favorite;
        }
        if(is_local) lv_obj_clear_flag(g_app.favorite_btn, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_app.favorite_btn, LV_OBJ_FLAG_HIDDEN);
        if(g_app.favorite_icon != NULL) {
            lv_obj_set_style_text_color(g_app.favorite_icon,
                                        is_favorite ? lv_color_hex(0xfa2d48) : AM_MUTED,
                                        0);
        }
    }
```

- [ ] **Step 5: 把所有侧边栏重建入口切换到真实数据签名**

```c
    am_shell_build_sidebar(g_app.sidebar, AM_VIEW_NOW, g_app.locals, g_app.local_count, am_on_nav, NULL);
```

```c
    am_shell_build_sidebar(g_app.sidebar, view, g_app.locals, g_app.local_count,
                           (am_nav_cb_t)am_show_view, NULL);
```

- [ ] **Step 6: 运行 UI 测试与回归测试，确认全部通过**

Run: `cmake --build build --target apple_music_now_view_state_test apple_music_parent_size_test apple_music_data_test && ./bin/apple_music_now_view_state_test && ./bin/apple_music_parent_size_test && ./bin/apple_music_data_test`

Expected: 三个测试都 PASS

- [ ] **Step 7: 提交 UI 接线**

```bash
git add main/src/v9_apple_music/am_data.c \
        main/src/v9_apple_music/am_data.h \
        main/src/v9_apple_music/am_shell.h \
        main/src/v9_apple_music/am_shell.c \
        main/src/v9_apple_music/apple_music.c \
        main/tests/apple_music_now_view_state_test.c \
        CMakeLists.txt
git commit -m "feat: drive apple music ui from persisted state"
```

## Self-Review

- Spec coverage
  - 状态文件位置、格式、跨重启保留：Task 1。
  - 删除假收藏、真实最近播放更新：Task 2。
  - 侧边栏最近播放、正在播放页收藏按钮、广播隐藏：Task 3。
  - 自动化测试覆盖状态读写、默认值、UI 显示：Task 1-3。
- Placeholder scan
  - 计划中没有 `TODO`、`TBD`、`类似 Task N`、`适当处理` 这类占位词。
- Type consistency
  - 所有任务统一使用 `am_state_*` 接口。
  - 最近播放序号统一为 `uint64_t recent_seq`。
  - 侧边栏新签名统一为 `am_shell_build_sidebar(..., const am_local_item_t *locals, size_t local_count, ...)`。

## Verification Commands

- `cmake --build build --target apple_music_state_test apple_music_data_test apple_music_now_view_state_test`
- `./bin/apple_music_state_test`
- `./bin/apple_music_data_test`
- `./bin/apple_music_now_view_state_test`
- `ctest --test-dir build --output-on-failure -R '^(apple_music_state_test|apple_music_data_test|apple_music_now_view_state_test)$'`
