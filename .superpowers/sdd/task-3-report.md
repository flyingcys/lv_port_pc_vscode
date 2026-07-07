# Task 3 报告：迷你播放器“打开文件”按钮与模式按钮视觉

## 需求范围

- 新增迷你播放器 `btn_open_file` 句柄与 `on_open_file` 接线能力。
- 模式按钮提供四态文本反馈：`SEQ / ONE / LOOP / SHUF`。
- 保持按钮单行布局，顺序为：模式 / 上一首 / 播放 / 下一首 / 打开文件 / 播放列表。
- 不实现 Task 4 的文件选择行为逻辑。

## 影响面说明

- 计划要求先做 GitNexus impact，但当前 GitNexus MCP 没有索引这个仓库；尝试对 `/home/share/samba/lvgl/lv_port_pc_vscode_flyingcys` 执行 `impact` 返回 `Repository not found`。
- 因此退化为本地调用面扫描，结果与 brief 一致：
  - `am_shell_build_miniplayer()` 调用点只在 `main/src/v9_apple_music/apple_music.c`
  - `main/tests/apple_music_playlist_popup_layout_test.c`
  - `main/tests/apple_music_playlist_scrollbar_behavior_test.c`
  - `main/tests/apple_music_single_file_player_test.c`
- 本次改动风险评估：
  - `am_shell_build_miniplayer`：低到中，原因是签名变化但调用点有限且已全部补 `NULL`
  - `am_player_refresh_ui`：低，原因是只新增模式按钮标签刷新

## TDD 记录

### RED

命令：

```bash
cmake --build build --target apple_music_single_file_player_test && ctest --test-dir build -R '^apple_music_single_file_player_test$' --output-on-failure
```

关键输出：

```text
CHECK failed: handles.btn_open_file != NULL (line 273)
0% tests passed, 1 tests failed out of 1
```

为什么这个 RED 合理：

- 测试已经声明了新需求：迷你播放器必须暴露 `btn_open_file`。
- 此时仅补了编译所需的机械接口，`am_shell_build_miniplayer()` 还没有真正创建该按钮。
- 因此失败原因正是“功能未实现”，不是拼写错误、环境错误或测试本身无效。

### GREEN

最小实现：

- 在 `am_miniplayer_handles_t` 中新增 `btn_open_file`
- `am_shell_build_miniplayer()` 新增 `on_open_file` 参数，并创建 `btn_open_file`
- 模式按钮从固定图标改为文本按钮，初始文本为 `SEQ`
- `am_player_refresh_ui()` 根据 `music_player_get_play_mode()` 刷新模式按钮文案

命令：

```bash
cmake --build build --target apple_music_single_file_player_test && ctest --test-dir build -R '^apple_music_single_file_player_test$' --output-on-failure
```

关键输出：

```text
1/1 Test #18: apple_music_single_file_player_test ...   Passed
100% tests passed, 0 tests failed out of 1
```

## 回归验证

命令：

```bash
cmake --build build --target apple_music_single_file_player_test apple_music_playlist_popup_layout_test apple_music_playlist_scrollbar_behavior_test && ctest --test-dir build -R '^(apple_music_single_file_player_test|apple_music_playlist_popup_layout_test|apple_music_playlist_scrollbar_behavior_test)$' --output-on-failure
```

关键输出：

```text
1/3 Test #16: apple_music_playlist_popup_layout_test .........   Passed
2/3 Test #17: apple_music_playlist_scrollbar_behavior_test ...   Passed
3/3 Test #18: apple_music_single_file_player_test ............   Passed
100% tests passed, 0 tests failed out of 3
```

## 实际改动

- `main/src/v9_apple_music/am_icons.h`
  - 新增 `AM_ICON_OPEN_FILE`
- `main/src/v9_apple_music/am_player.h`
  - 为 `am_miniplayer_handles_t` 新增 `btn_open_file`
- `main/src/v9_apple_music/am_shell.h`
  - `am_shell_build_miniplayer()` 新增 `on_open_file` 参数
- `main/src/v9_apple_music/am_shell.c`
  - 在迷你播放器控制条中插入“打开文件”按钮
  - 保持模式按钮和控制按钮在同一行
  - 为 `on_open_file` 预留事件接线
- `main/src/v9_apple_music/am_player.c`
  - 新增播放模式到按钮文案的映射
  - 在 `am_player_refresh_ui()` 中刷新模式按钮文本
- `main/src/v9_apple_music/apple_music.c`
  - 补 `NULL` 占位，适配新签名
- `main/tests/apple_music_single_file_player_test.c`
  - 新增打开文件按钮句柄测试
  - 新增模式按钮四态文案测试
- `main/tests/apple_music_playlist_popup_layout_test.c`
  - 适配新签名
- `main/tests/apple_music_playlist_scrollbar_behavior_test.c`
  - 适配新签名

## 备注

- 本任务没有实现文件选择回调逻辑，也没有改动宿主页同步逻辑；这些仍留给 Task 4。
- 独立 subagent 代码评审尝试执行，但当前会话的 agent thread limit 已满，工具拒绝新建评审 agent，因此本次未能拿到额外的 subagent review 结果。
