# Apple Music 播放列表可拖动滚动条 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 `main/src/v9_apple_music` 的播放列表弹层增加右侧竖向可拖动滚动条，在内容超出可视区域时显示，并允许用户通过拖动 thumb 滚动列表。

**Architecture:** 保留 `playlist_list` 作为真实滚动容器，在 `playlist_popup` 右侧新增滚动条轨道与 thumb。列表滚动事件负责同步 thumb 的显隐、尺寸和位置；thumb 的按下与拖动事件负责反向驱动 `playlist_list` 的纵向滚动。实现尽量收敛在 `am_shell.c` 和 `am_player.c`，测试分别覆盖布局存在性和长短列表显隐逻辑。

**Tech Stack:** C99、LVGL v9、现有 Apple Music 弹层组件、CMake/CTest

## Global Constraints

- 当播放列表内容超过当前可视高度时，在右侧显示竖向滚动条。
- 滚动条需要支持鼠标直接拖拽，拖动 thumb 时同步滚动播放列表内容。
- 列表内容未超出可视区域时，滚动条整体隐藏。
- 外观保持标准滚动条风格，不引入与当前 UI 风格冲突的独立控件视觉。
- 不修改播放列表条目内容、排序、点击切歌逻辑。
- 不调整播放列表弹层的整体布局结构，除非为放置滚动条必须做最小间距调整。
- 不修改广播电台与本地歌曲的数据来源和数量逻辑。
- 不实现惯性拖拽、分页吸附或额外动画。

---

## 文件职责

- `main/src/v9_apple_music/am_player.h`
  - 扩展 `am_miniplayer_handles_t`，承载滚动条轨道和 thumb 句柄。
- `main/src/v9_apple_music/am_shell.c`
  - 创建滚动条轨道和 thumb，定义其静态布局和基础样式。
- `main/src/v9_apple_music/am_player.c`
  - 维护滚动条可见性、尺寸、位置，并处理 thumb 拖拽与列表滚动映射。
- `main/tests/apple_music_playlist_popup_layout_test.c`
  - 覆盖滚动条对象创建与基础布局断言。
- `main/tests/apple_music_playlist_scrollbar_behavior_test.c`
  - 覆盖短列表隐藏、长列表显示、同步刷新等行为断言。
- `CMakeLists.txt`
  - 注册新增或扩展后的测试目标。

## Task 1: 扩展句柄并建立滚动条骨架

**Files:**
- Modify: `main/src/v9_apple_music/am_player.h`
- Modify: `main/src/v9_apple_music/am_shell.c`
- Modify: `main/tests/apple_music_playlist_popup_layout_test.c`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes:
  - `am_miniplayer_handles_t`
  - `am_shell_build_miniplayer(lv_obj_t *player, ...)`
- Produces:
  - `am_miniplayer_handles_t.playlist_scroll_track`
  - `am_miniplayer_handles_t.playlist_scroll_thumb`
  - `apple_music_playlist_popup_layout_test`

- [ ] **Step 1: 在布局测试中先写失败断言**

```c
if(handles.playlist_scroll_track == NULL) {
    return failf("playlist scroll track missing", 0);
}
if(handles.playlist_scroll_thumb == NULL) {
    return failf("playlist scroll thumb missing", 0);
}
if(lv_obj_get_width(handles.playlist_scroll_track) <= 0) {
    return failf("playlist scroll track width invalid",
                 lv_obj_get_width(handles.playlist_scroll_track));
}
if(lv_obj_get_height(handles.playlist_scroll_thumb) <= 0) {
    return failf("playlist scroll thumb height invalid",
                 lv_obj_get_height(handles.playlist_scroll_thumb));
}
```

- [ ] **Step 2: 运行测试，确认当前实现失败**

Run: `cmake --build build --target apple_music_playlist_popup_layout_test && ./bin/apple_music_playlist_popup_layout_test`

Expected: FAIL，报 `playlist scroll track missing` 或 `playlist scroll thumb missing`

- [ ] **Step 3: 扩展句柄定义**

```c
typedef struct {
    lv_obj_t *playlist_popup;
    lv_obj_t *playlist_list;
    lv_obj_t *playlist_scroll_track;
    lv_obj_t *playlist_scroll_thumb;
    lv_obj_t *btn_mode;
    lv_obj_t *btn_prev;
    lv_obj_t *btn_play;
    lv_obj_t *btn_next;
    lv_obj_t *btn_playlist;
} am_miniplayer_handles_t;
```

- [ ] **Step 4: 在 `am_shell_build_miniplayer()` 中创建轨道和 thumb**

```c
    h.playlist_scroll_track = lv_obj_create(h.playlist_popup);
    lv_obj_remove_style_all(h.playlist_scroll_track);
    lv_obj_set_size(h.playlist_scroll_track, 8, LV_PCT(100));
    lv_obj_align(h.playlist_scroll_track, LV_ALIGN_RIGHT_MID, -6, 0);
    lv_obj_set_style_bg_color(h.playlist_scroll_track, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(h.playlist_scroll_track, 20, 0);
    lv_obj_set_style_radius(h.playlist_scroll_track, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_flag(h.playlist_scroll_track, LV_OBJ_FLAG_HIDDEN);

    h.playlist_scroll_thumb = lv_obj_create(h.playlist_scroll_track);
    lv_obj_remove_style_all(h.playlist_scroll_thumb);
    lv_obj_set_size(h.playlist_scroll_thumb, LV_PCT(100), 32);
    lv_obj_align(h.playlist_scroll_thumb, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(h.playlist_scroll_thumb, lv_color_hex(0x7d7d7d), 0);
    lv_obj_set_style_bg_opa(h.playlist_scroll_thumb, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(h.playlist_scroll_thumb, LV_RADIUS_CIRCLE, 0);
```

- [ ] **Step 5: 让 `playlist_list` 为滚动条留出最小右边距**

```c
    lv_obj_set_style_pad_right(h.playlist_list, 16, 0);
```

- [ ] **Step 6: 重新运行布局测试，确认通过**

Run: `./bin/apple_music_playlist_popup_layout_test`

Expected: PASS

- [ ] **Step 7: 提交骨架改动**

```bash
git add main/src/v9_apple_music/am_player.h \
        main/src/v9_apple_music/am_shell.c \
        main/tests/apple_music_playlist_popup_layout_test.c \
        CMakeLists.txt
git commit -m "feat: add playlist scrollbar shell objects"
```

### Task 2: 实现长短列表显隐与滚动同步

**Files:**
- Create: `main/tests/apple_music_playlist_scrollbar_behavior_test.c`
- Modify: `main/src/v9_apple_music/am_player.c`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes:
  - `am_player_bind_miniplayer(const am_miniplayer_handles_t *h)`
  - `am_player_set_sources(const am_local_item_t *locals, size_t local_count, const am_radio_item_t *radios, size_t radio_count)`
  - `am_player_play_local_index(size_t index)`
  - `am_player_refresh_ui(void)`
- Produces:
  - `static void am_playlist_scrollbar_sync(void)`
  - `apple_music_playlist_scrollbar_behavior_test`

- [ ] **Step 1: 新建行为测试并先写短列表隐藏、长列表显示两个失败断言**

```c
static int expect_hidden(lv_obj_t *obj, const char *name)
{
    if(!lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) {
        fprintf(stderr, "%s should be hidden\n", name);
        return 1;
    }
    return 0;
}

static int expect_visible(lv_obj_t *obj, const char *name)
{
    if(lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) {
        fprintf(stderr, "%s should be visible\n", name);
        return 1;
    }
    return 0;
}
```

```c
if(expect_hidden(handles.playlist_scroll_track, "track")) return 1;
...
if(expect_visible(handles.playlist_scroll_track, "track")) return 1;
if(lv_obj_get_height(handles.playlist_scroll_thumb) < 10) {
    fprintf(stderr, "thumb height too small: %d\n",
            (int)lv_obj_get_height(handles.playlist_scroll_thumb));
    return 1;
}
```

- [ ] **Step 2: 运行行为测试，确认当前实现失败**

Run: `cmake --build build --target apple_music_playlist_scrollbar_behavior_test && ./bin/apple_music_playlist_scrollbar_behavior_test`

Expected: FAIL，短列表/长列表显隐断言至少一条失败

- [ ] **Step 3: 在 `am_player.c` 增加滚动条同步辅助函数**

```c
static void am_playlist_scrollbar_sync(void)
{
    lv_obj_t *list = g_h.playlist_list;
    lv_obj_t *track = g_h.playlist_scroll_track;
    lv_obj_t *thumb = g_h.playlist_scroll_thumb;
    int32_t viewport_h;
    int32_t content_h;
    int32_t scroll_y;
    int32_t content_range;
    int32_t track_h;
    int32_t thumb_h;
    int32_t track_range;
    int32_t thumb_y;

    if(list == NULL || track == NULL || thumb == NULL) return;

    lv_obj_update_layout(list);
    viewport_h = lv_obj_get_content_height(list);
    content_h = lv_obj_get_scroll_bottom(list) - lv_obj_get_scroll_top(list) + viewport_h;
    scroll_y = -lv_obj_get_scroll_y(list);
    content_range = content_h - viewport_h;

    if(content_range <= 0) {
        lv_obj_add_flag(track, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(track, LV_OBJ_FLAG_HIDDEN);
    track_h = lv_obj_get_content_height(track);
    thumb_h = (track_h * viewport_h) / content_h;
    if(thumb_h < 24) thumb_h = 24;
    if(thumb_h > track_h) thumb_h = track_h;

    track_range = track_h - thumb_h;
    thumb_y = (content_range > 0 && track_range > 0)
        ? (track_range * scroll_y) / content_range
        : 0;

    lv_obj_set_height(thumb, thumb_h);
    lv_obj_align(thumb, LV_ALIGN_TOP_MID, 0, thumb_y);
}
```

- [ ] **Step 4: 在列表刷新链路中调用同步函数**

```c
    am_refresh_playlist_popup();
    am_playlist_scrollbar_sync();
```

```c
    lv_obj_clean(g_h.playlist_list);
    ...
    am_playlist_scrollbar_sync();
```

- [ ] **Step 5: 给 `playlist_list` 注册滚动事件以驱动同步**

```c
static void am_playlist_list_scroll_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    am_playlist_scrollbar_sync();
}
```

```c
    lv_obj_add_event_cb(g_h.playlist_list, am_playlist_list_scroll_cb, LV_EVENT_SCROLL, NULL);
```

- [ ] **Step 6: 重新运行行为测试，确认长短列表显隐通过**

Run: `./bin/apple_music_playlist_scrollbar_behavior_test`

Expected: PASS

- [ ] **Step 7: 提交同步逻辑**

```bash
git add main/src/v9_apple_music/am_player.c \
        main/tests/apple_music_playlist_scrollbar_behavior_test.c \
        CMakeLists.txt
git commit -m "feat: sync playlist scrollbar state"
```

### Task 3: 实现 thumb 拖拽驱动列表滚动并完成验证

**Files:**
- Modify: `main/src/v9_apple_music/am_player.c`
- Modify: `main/tests/apple_music_playlist_scrollbar_behavior_test.c`

**Interfaces:**
- Consumes:
  - `am_playlist_scrollbar_sync(void)`
  - `g_h.playlist_scroll_thumb`
  - `g_h.playlist_list`
- Produces:
  - `static void am_playlist_thumb_event_cb(lv_event_t *e)`
  - 可拖动的播放列表 thumb

- [ ] **Step 1: 在行为测试中先加一个“拖动后 scroll_y 变化”的失败断言**

```c
int32_t before = lv_obj_get_scroll_y(handles.playlist_list);
simulate_thumb_drag(handles.playlist_scroll_thumb, 24);
int32_t after = lv_obj_get_scroll_y(handles.playlist_list);
if(after == before) {
    fprintf(stderr, "scroll y should change after dragging thumb\n");
    return 1;
}
```

- [ ] **Step 2: 运行测试，确认拖拽断言失败**

Run: `./bin/apple_music_playlist_scrollbar_behavior_test`

Expected: FAIL，报 `scroll y should change after dragging thumb`

- [ ] **Step 3: 在 `am_player.c` 增加拖拽状态和事件处理**

```c
static bool g_playlist_thumb_dragging = false;
static int32_t g_playlist_thumb_press_ofs_y = 0;
```

```c
static void am_playlist_thumb_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *thumb = lv_event_get_target(e);
    lv_obj_t *track = g_h.playlist_scroll_track;
    lv_obj_t *list = g_h.playlist_list;
    lv_indev_t *indev = lv_event_get_indev(e);
    lv_point_t p;
    lv_area_t a;
    int32_t track_h;
    int32_t thumb_h;
    int32_t track_range;
    int32_t target_y;
    int32_t viewport_h;
    int32_t content_h;
    int32_t content_range;
    int32_t scroll_target;

    if(indev == NULL || thumb == NULL || track == NULL || list == NULL) return;
    lv_indev_get_point(indev, &p);

    if(code == LV_EVENT_PRESSED) {
        lv_obj_get_coords(thumb, &a);
        g_playlist_thumb_dragging = true;
        g_playlist_thumb_press_ofs_y = p.y - a.y1;
        return;
    }

    if(code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        g_playlist_thumb_dragging = false;
        return;
    }

    if(code != LV_EVENT_PRESSING || !g_playlist_thumb_dragging) return;

    lv_obj_update_layout(track);
    lv_obj_update_layout(list);
    lv_obj_get_coords(track, &a);
    track_h = lv_obj_get_content_height(track);
    thumb_h = lv_obj_get_height(thumb);
    track_range = track_h - thumb_h;
    viewport_h = lv_obj_get_content_height(list);
    content_h = lv_obj_get_scroll_bottom(list) - lv_obj_get_scroll_top(list) + viewport_h;
    content_range = content_h - viewport_h;

    if(track_range <= 0 || content_range <= 0) return;

    target_y = p.y - a.y1 - g_playlist_thumb_press_ofs_y;
    if(target_y < 0) target_y = 0;
    if(target_y > track_range) target_y = track_range;

    scroll_target = (target_y * content_range) / track_range;
    lv_obj_scroll_to_y(list, scroll_target, LV_ANIM_OFF);
    am_playlist_scrollbar_sync();
}
```

- [ ] **Step 4: 给 thumb 绑定拖拽事件并开启点击能力**

```c
    lv_obj_add_flag(g_h.playlist_scroll_thumb, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(g_h.playlist_scroll_thumb, am_playlist_thumb_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(g_h.playlist_scroll_thumb, am_playlist_thumb_event_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(g_h.playlist_scroll_thumb, am_playlist_thumb_event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(g_h.playlist_scroll_thumb, am_playlist_thumb_event_cb, LV_EVENT_PRESS_LOST, NULL);
```

- [ ] **Step 5: 跑行为测试与布局测试**

Run: `./bin/apple_music_playlist_scrollbar_behavior_test && ./bin/apple_music_playlist_popup_layout_test`

Expected: PASS

- [ ] **Step 6: 做一次无头截图人工核对**

Run: `AM_APP=apple_music AM_SHOT=/tmp/am_playlist_scrollbar.ppm AM_PANEL=playlist ./bin/main`

Expected: 成功输出截图文件，右侧可看到标准细竖条风格滚动条

- [ ] **Step 7: 提交拖拽实现**

```bash
git add main/src/v9_apple_music/am_player.c \
        main/tests/apple_music_playlist_scrollbar_behavior_test.c
git commit -m "feat: make playlist scrollbar draggable"
```

## Self-Review

### Spec coverage

- “内容超出时显示滚动条”：
  - Task 2 实现并测试显隐逻辑。
- “拖动 thumb 时同步滚动列表”：
  - Task 3 实现事件映射并补测试。
- “内容不足一屏时隐藏”：
  - Task 2 的短列表断言覆盖。
- “保持标准滚动条风格”：
  - Task 1 在 `am_shell.c` 中建立细竖条轨道和 thumb 样式。

### Placeholder scan

- 未使用 `TODO` / `TBD` / “后续补充”等占位描述。
- 每个测试步骤都给出具体命令与期望。
- 每个代码步骤都给出明确文件和示例代码。

### Type consistency

- 句柄字段统一为：
  - `playlist_scroll_track`
  - `playlist_scroll_thumb`
- 同步函数统一命名为：
  - `am_playlist_scrollbar_sync`
- 拖拽回调统一命名为：
  - `am_playlist_thumb_event_cb`

## Execution Handoff

Plan complete and saved to `docs/superpowers/plans/2026-07-06-apple-music-playlist-scrollbar.md`. Two execution options:

**1. Subagent-Driven (recommended)** - 我分任务派发独立 subagent 实现，每个任务之间做收敛和复核

**2. Inline Execution** - 在当前会话里按计划逐步实现和验证

Which approach?
