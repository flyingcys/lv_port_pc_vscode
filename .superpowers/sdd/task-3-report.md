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
