# Task 2 报告：实现长短列表显隐与滚动同步

## 任务范围

- 需求来源：`.superpowers/sdd/task-2-brief.md`
- 本次仅实现 Task 2 要求：
  - 长短列表下滚动条显隐
  - 滚动时 thumb 同步
  - 不实现 thumb 拖拽

## TDD 过程

1. 新建行为测试 `main/tests/apple_music_playlist_scrollbar_behavior_test.c`
2. 先写短列表隐藏、长列表显示、滚动后 thumb 位移断言
3. 运行聚焦测试，初始失败：
   - 长列表下 `track should be visible`
4. 在 `main/src/v9_apple_music/am_player.c` 实现最小同步逻辑
5. 再次运行聚焦测试，发现 thumb 位移方向错误
6. 修正滚动位移方向后，聚焦测试通过

## 实现说明

### 1. 行为测试

- 新增 `apple_music_playlist_scrollbar_behavior_test`
- 用轻量 `music_player` stub 隔离播放引擎
- 覆盖三类行为：
  - 短列表时滚动条隐藏
  - 长列表时滚动条显示，且 thumb 高度有效
  - 列表滚动后 thumb 实际坐标随之移动

### 2. 滚动条同步逻辑

在 `am_player.c` 新增：

- `static void am_playlist_scrollbar_sync(void)`
- `static void am_playlist_list_scroll_cb(lv_event_t *e)`

同步逻辑要点：

- 基于 `playlist_list` 的 viewport 高度、内容高度、当前滚动量计算是否需要显示滚动条
- 内容未超出 viewport 时隐藏 `playlist_scroll_track`
- 内容超出时显示 `playlist_scroll_track`
- 根据 viewport/content 比例计算 thumb 高度，并设置最小高度 24
- 根据滚动位置计算 thumb 的 Y 偏移，实现滚动同步

### 3. 刷新链路接入

- 在 `am_refresh_playlist_popup()` 重建播放列表后同步滚动条
- 在 `am_player_refresh_ui()` 中补一次同步，覆盖“列表未重建但滚动位置变化”的场景
- 在 `am_player_bind_miniplayer()` 给 `playlist_list` 注册 `LV_EVENT_SCROLL`，驱动滚动过程中的实时同步

### 4. 构建接线

修改 `CMakeLists.txt`：

- 新增测试目标 `apple_music_playlist_scrollbar_behavior_test`
- 注册对应 `add_test(...)`

## 风险与影响

### 手工 impact 收敛

由于本地 GitNexus MCP 未识别当前仓库索引，本次改动前做了手工 blast radius 收敛：

- `am_player_refresh_ui` 的外部入口在 `main/src/v9_apple_music/apple_music.c`
- `am_player_bind_miniplayer` 的外部入口在 `main/src/v9_apple_music/apple_music.c`
- `am_refresh_playlist_popup` 为 `am_player.c` 内部静态函数

实际改动仅限：

- `main/src/v9_apple_music/am_player.c`
- `main/tests/apple_music_playlist_scrollbar_behavior_test.c`
- `CMakeLists.txt`

## 测试

执行命令：

```bash
cmake --build build --target apple_music_playlist_scrollbar_behavior_test
./bin/apple_music_playlist_scrollbar_behavior_test
```

结果：

- PASS

## 关注项

- `rtk proxy npx gitnexus analyze .` 已启动并生成部分 `.gitnexus` 缓存，但当前 CLI/MCP 仍未把该仓库识别为可用索引，因此本次未能完成 GitNexus `impact` / `detect-changes` 的正式校验。
