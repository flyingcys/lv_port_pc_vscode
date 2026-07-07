# Task 3 报告

## 结果摘要

- 侧边栏“最近播放”改为读取 `am_state_collect_recent()` 的真实状态，不再消费假数据 `am_recent_titles`
- “正在播放”页新增收藏按钮
- 本地音源下按钮显示并可切换收藏、落盘到 `apple_music_state.tsv`
- 广播音源下按钮隐藏
- 新增 UI 回归测试 `apple_music_now_view_state_test`
- 补强 UI 回归测试：断言 `Beta` 排在 `Alpha` 前，实际点击收藏按钮并回读 `apple_music_state.tsv` 验证落盘，广播场景按按钮对象隐藏态断言

## TDD 过程

### RED

先新增 `main/tests/apple_music_now_view_state_test.c` 与 CMake 目标，测试通过 stub 运行态数据：

- `am_local_scan_dir()` 返回 `Alpha/Beta/Gamma`
- 预写状态文件：`Beta recent_seq=9`、`Alpha recent_seq=4`、`Gamma favorite=1`
- 本地音源当前索引为 `2`

首次 RED 验证命令：

```bash
cmake --build build --target apple_music_now_view_state_test && ./bin/apple_music_now_view_state_test
```

首次有效失败结果：

- `CHECK failed: count_labels_with_text(sidebar, "Beta") >= 1U`

说明侧边栏仍在渲染假最近播放，符合预期。

### GREEN

按 brief 做最小实现：

- 扩展 `am_shell_build_sidebar()` 签名，传入 `locals/local_count`
- 侧边栏改用 `am_state_collect_recent()` 渲染最近播放
- 删除 `am_recent_titles` 定义与声明
- “正在播放”页增加 `favorite_btn/favorite_icon`
- 新增 `am_on_toggle_favorite()`，切换收藏后保存状态并刷新当前页与侧边栏
- 所有侧边栏重建入口切换到新签名
- `apple_music_parent_size_test` 链接 `am_state.c`，覆盖新增状态依赖

补测试覆盖时，继续收紧 `apple_music_now_view_state_test`：

- 不再用全页心形数量间接判断显隐，改为在主面板内定位“正在播放”收藏按钮对象并检查 `LV_OBJ_FLAG_HIDDEN`
- 侧边栏 recent 断言从“都出现”升级为“`Beta` 先于 `Alpha` 出现”
- 新增真实交互用例：发送 `LV_EVENT_CLICKED` 到收藏按钮，随后用 `am_state_load()` 重新读取 `apple_music_state.tsv`，验证 `Gamma.favorite` 从 `true` 翻到 `false`

## 验证

执行命令：

```bash
cmake --build build --target apple_music_now_view_state_test apple_music_parent_size_test apple_music_data_test
./bin/apple_music_now_view_state_test
./bin/apple_music_parent_size_test
./bin/apple_music_data_test
```

结果：

- 三个测试均通过
- `apple_music_now_view_state_test` 额外覆盖最近播放顺序、收藏按钮显隐、真实点击后的收藏落盘

## GitNexus 说明

本任务尝试按仓库要求执行 GitNexus impact：

- `mcp__gitnexus.impact` 无法命中当前仓库，因为 MCP registry 中缺少该仓库条目
- 本地执行 `npx gitnexus analyze` 时，GitNexus 原生 worker 在 Node `v22.21.1` 环境下崩溃，未能完成索引注册
- 收尾阶段再次尝试 `mcp__gitnexus.detect_changes`，MCP 仍要求已注册 repo 名称，当前仓库无法被解析到对应索引

因此本轮只能做保守手工 blast radius：

- `am_shell_build_sidebar()` 直接调用点仅 `apple_music.c` 两处
- `am_update_now_view()` / `am_build_now_view()` 仅在 `apple_music.c` 内部使用
- `am_recent_titles` 消费点仅 `am_shell.c`

保守风险评估：`MEDIUM`

## 改动文件

- `main/src/v9_apple_music/am_data.c`
- `main/src/v9_apple_music/am_data.h`
- `main/src/v9_apple_music/am_shell.h`
- `main/src/v9_apple_music/am_shell.c`
- `main/src/v9_apple_music/apple_music.c`
- `main/tests/apple_music_now_view_state_test.c`
- `CMakeLists.txt`

## 追加修复（评审后）

### 修复内容

- 为所有直接链接 `am_shell.c` 但缺失 `am_state.c` 的相关测试目标补齐最小依赖：
  - `apple_music_playlist_popup_layout_test`
  - `apple_music_playlist_scrollbar_behavior_test`
- 在 `apple_music.c` 收敛出 `am_refresh_sidebar()`，统一侧边栏重建逻辑
- `am_record_current_local_playback()` 在 recent 序号更新并落盘后，立即刷新侧边栏，避免 UI 比状态晚一拍
- 扩展 `apple_music_now_view_state_test`：
  - 真实点击“下一首”按钮触发播放状态变化
  - 断言侧边栏 recent 立即改成 `Gamma -> Beta -> Alpha`
  - 回读 `apple_music_state.tsv`，确认 `Gamma.recent_seq` 变为 `10`

### 本轮验证命令

```bash
cmake --build build --target apple_music_playlist_popup_layout_test
cmake --build build --target apple_music_now_view_state_test && ./bin/apple_music_now_view_state_test
cmake --build build --target apple_music_playlist_scrollbar_behavior_test
```

### 本轮验证结果

- `apple_music_playlist_popup_layout_test` 现已可成功链接构建，不再缺失 `am_state_collect_recent`
- `apple_music_now_view_state_test` 通过，新增覆盖证明：
  - 收藏点击会落盘
  - recent 顺序正确
  - 播放切换后 recent 会立刻刷新到 sidebar，而不是只更新持久化状态
- `apple_music_playlist_scrollbar_behavior_test` 同样完成最小依赖补齐并可成功构建

### 主线程最终复验

执行命令：

```bash
cmake --build build --target apple_music_playlist_popup_layout_test apple_music_playlist_scrollbar_behavior_test apple_music_now_view_state_test
./bin/apple_music_playlist_popup_layout_test
./bin/apple_music_playlist_scrollbar_behavior_test
./bin/apple_music_now_view_state_test
cmake --build build --target apple_music_parent_size_test apple_music_data_test
./bin/apple_music_parent_size_test
./bin/apple_music_data_test
```

结果：

- 上述目标均构建成功
- 五个可执行测试均退出码 `0`
- 输出仅包含 LVGL 断言/完整性检查已开启的告警，无失败

## 追加修复（最终总评）

### 修复内容

- 去掉 `apple_music.c` 中把 `am_show_view(am_view_t)` 强转为 `am_nav_cb_t(am_view_t, void *)` 的做法
  - `am_refresh_sidebar()` 和 `am_show_view()` 现在统一经 `am_on_nav()` 走签名匹配的回调路径
- 收紧 `am_state_save()` 的错误处理
  - `fprintf()` 失败立即记错并返回非 `0`
  - `fclose()` 失败同样返回非 `0`
- 收紧 `am_on_play_pause()` 语义
  - 仅在 `am_player_is_playing()` 为真时记录 recent，暂停不再打点
- 删除 `am_shell.c` 在无 recent 时补 4 个空 label 的占位逻辑
  - 现在没有历史时只显示“最近播放”标题，不渲染空占位

### 本轮 RED

先补测试：

- `apple_music_state_test`
  - 新增 `test_state_save_reports_write_failure()`，用 `/dev/full` 锁定写入失败返回非 `0`
- `apple_music_now_view_state_test`
  - 新增“无 recent 不渲染空占位”断言
  - 新增“暂停不刷新 recent/sidebar”断言

RED 验证命令：

```bash
cmake --build build --target apple_music_state_test apple_music_now_view_state_test
./bin/apple_music_state_test
./bin/apple_music_now_view_state_test
```

RED 结果：

- `apple_music_state_test` 失败：`am_state_save("/dev/full", items, 1U) != 0`
- `apple_music_now_view_state_test` 失败：`count_empty_labels(sidebar) == 0U`

### 本轮 GREEN / 回归

执行命令：

```bash
cmake --build build --target apple_music_state_test apple_music_now_view_state_test
./bin/apple_music_state_test
./bin/apple_music_now_view_state_test
cmake --build build --target apple_music_parent_size_test apple_music_playlist_popup_layout_test apple_music_playlist_scrollbar_behavior_test apple_music_data_test
./bin/apple_music_parent_size_test
./bin/apple_music_playlist_popup_layout_test
./bin/apple_music_playlist_scrollbar_behavior_test
./bin/apple_music_data_test
```

结果：

- `apple_music_state_test` 通过，覆盖 `am_state_save()` 写失败返回值
- `apple_music_now_view_state_test` 通过，新增覆盖：
  - 无 recent 时 sidebar 不渲染空占位
  - 暂停不会刷新 recent
  - 既有收藏落盘、recent 顺序、切歌即时刷新仍保持通过
- `apple_music_parent_size_test`、`apple_music_playlist_popup_layout_test`、`apple_music_playlist_scrollbar_behavior_test`、`apple_music_data_test` 全部通过
- 输出仅包含 LVGL 断言/完整性检查告警，无失败
