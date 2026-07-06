# Task 3 报告：Apple Music 播放列表 scrollbar thumb 拖拽滚动

## 任务范围

- 需求来源：`.superpowers/sdd/task-3-brief.md`
- 本次仅修改：
  - `main/src/v9_apple_music/am_player.c`
  - `main/tests/apple_music_playlist_scrollbar_behavior_test.c`

## TDD 过程

1. 先在 `main/tests/apple_music_playlist_scrollbar_behavior_test.c` 增加拖拽后的 `scroll_y` 变化断言。
2. 运行聚焦测试，先得到失败结果：
   - `scroll y should change after dragging thumb`
3. 在 `main/src/v9_apple_music/am_player.c` 增加 thumb 拖拽状态和事件回调，驱动 `playlist_list` 纵向滚动。
4. 再次运行聚焦测试，断言通过。

## 实现说明

### 1. 拖拽滚动

在 `am_player.c` 中新增：

- `static bool g_playlist_thumb_dragging`
- `static int32_t g_playlist_thumb_press_ofs_y`
- `static void am_playlist_thumb_event_cb(lv_event_t *e)`

行为要点：

- `LV_EVENT_PRESSED` 时记录按下点相对 thumb 顶部的偏移
- `LV_EVENT_PRESSING` 时把 thumb 的纵向位移映射为 `playlist_list` 的 `scroll_y`
- `LV_EVENT_RELEASED` / `LV_EVENT_PRESS_LOST` 时结束拖拽状态

### 2. 绑定与可拖动能力

- 在 `am_player_bind_miniplayer()` 中给 `playlist_scroll_thumb` 绑定 `PRESSED / PRESSING / RELEASED / PRESS_LOST`
- 同时给 thumb 增加 `CLICKABLE` 和 `PRESS_LOCK`
- 保持 `playlist_list` 原有滚动同步逻辑不变

### 3. 视觉收敛

- 在 `am_playlist_scrollbar_sync()` 内把 thumb 宽度收窄为 `4`
- 保持纵向同步逻辑与最小高度策略不变
- 仅在 `am_player.c` 内做最小改动，没有扩展到其他文件

### 4. 行为测试

`main/tests/apple_music_playlist_scrollbar_behavior_test.c` 增加了一个最小拖拽模拟器：

- 通过自定义 `lv_indev` 记录指针坐标
- 用 `lv_obj_send_event(..., LV_EVENT_PRESSED/PRESSING/RELEASED, indev)` 直接驱动 thumb 事件
- 验证拖拽前后 `lv_obj_get_scroll_y(handles.playlist_list)` 发生变化

## 测试与截图

### 1. 聚焦行为测试

命令：

```bash
cmake --build build --target apple_music_playlist_scrollbar_behavior_test -j4
./bin/apple_music_playlist_scrollbar_behavior_test
```

结果：

- 先失败，报错：`scroll y should change after dragging thumb`
- 实现后通过

### 2. 布局测试

命令：

```bash
cmake --build build --target apple_music_playlist_scrollbar_behavior_test apple_music_playlist_popup_layout_test -j4
./bin/apple_music_playlist_scrollbar_behavior_test
./bin/apple_music_playlist_popup_layout_test
```

结果：

- 两个测试均通过

### 3. 截图验证

命令：

```bash
AM_APP=apple_music AM_SHOT=/tmp/am_playlist_scrollbar.ppm AM_PANEL=playlist ./bin/main
```

结果：

- 成功生成截图文件：`/tmp/am_playlist_scrollbar.ppm`
- 额外转成 PNG 后人工查看：`/tmp/am_playlist_scrollbar.png`
- 右侧边缘裁剪图：`/tmp/am_playlist_scrollbar_right.png`

## 改动文件

- `main/src/v9_apple_music/am_player.c`
- `main/tests/apple_music_playlist_scrollbar_behavior_test.c`

## 备注

- 本次没有修改任何其他源码文件。
- 截图命令已执行并产出文件；主应用初始态下滚动条在截图里不算醒目，但行为测试已覆盖拖拽滚动的核心逻辑。

## Review 修复追加

### 修复项

- 修复 `am_playlist_scrollbar_sync()` 与 `am_playlist_thumb_event_cb()` 使用不同 track 高度基准的问题。
- 现在两处统一使用 `lv_obj_get_content_height(track)`，避免后续给 track 加 padding 等样式后，thumb 同步位置和拖拽比例发生漂移。

### 测试增强

- 保留原有“拖动后 `scroll_y` 变化”的断言。
- 新增“向下拖动后 `scroll_y` 应增大”的方向断言。
- 新增一组带 `track` 上下 padding 的断言，覆盖：
  - 大位移拖拽后 `scroll_y` 能钳到接近底部
  - `thumb` 仍保持在 `track` 内容区内
- 同时把测试里的底部拖拽位移改为基于实际 `track` 坐标计算，避免越出显示区域的输入告警。

### 本轮命令与结果

命令：

```bash
cmake --build build --target apple_music_playlist_scrollbar_behavior_test -j4
./bin/apple_music_playlist_scrollbar_behavior_test
```

结果：

- PASS
- 增强后的方向与底部钳制断言均通过
