# Task 1 报告

## 结果

- 已新增 `am_state` 状态模块，包含状态加载、状态保存、最近播放收集、最近播放打点、收藏切换接口。
- 已为 `am_local_item_t` 增加 `recent_seq` 字段，用于最近播放排序。
- 已新增 `apple_music_state_test`，锁定以下行为：
  - 状态文件 round-trip 读写
  - 坏行跳过
  - 最近播放按 `recent_seq` 倒序取前 4 项

## TDD 记录

1. 先新增测试文件和测试目标。
2. 执行 `cmake -S . -B build && cmake --build build --target apple_music_state_test`，确认 RED：
   - 失败原因为 `am_state.c` 尚不存在，状态模块未实现。
3. 再补 `am_data.h`、`am_state.h`、`am_state.c` 的最小实现。
4. 执行 `cmake -S . -B build && cmake --build build --target apple_music_state_test && ./bin/apple_music_state_test`，确认 GREEN。

## 验证

- 通过命令：

```bash
cmake -S . -B build && cmake --build build --target apple_music_state_test && ./bin/apple_music_state_test
```

- 结果：命令退出码为 0，测试通过，无新增 stderr 输出。

## 影响与边界

- 仅修改以下允许写面：
  - `main/src/v9_apple_music/am_data.h`
  - `main/src/v9_apple_music/am_state.h`
  - `main/src/v9_apple_music/am_state.c`
  - `main/tests/apple_music_state_test.c`
  - `CMakeLists.txt`
- 未改运行态接线、侧边栏、正在播放页 UI。
- 未触碰无关脏文件 `third-party/hls_player_demo`、`21.png` 和其他用户改动。

## Concerns

- 仓库内要求的 GitNexus impact / detect_changes 在本地不可用：`npx gitnexus analyze` 在当前环境触发 native worker abort，随后 `npx gitnexus status` 仍显示 `Repository not indexed`。本次只能以本地引用扫描和 diff 自检作为替代，无法提供正式 GitNexus 报告。

## 评审修复追加记录

### 修复说明

- 修正 `main/src/v9_apple_music/am_state.c` 的 `am_state_save()`：仅当歌曲 `favorite == true` 或 `recent_seq > 0` 时才写入状态文件，避免把整份本地列表持久化为播放状态。
- 在 `main/tests/apple_music_state_test.c` 新增测试，锁定 `AM_STATE_PATH` 必须等于 `apple_music_state.tsv`。
- 同时把该测试文件内的断言改为自定义 `CHECK()`，原因是当前构建配置下 `assert` 被编译掉，无法可靠地产生 RED/GREEN 信号。

### 本次 TDD 记录

1. 先在 `main/tests/apple_music_state_test.c` 增加两条测试：
   - `test_state_path_constant`
   - `test_state_save_skips_plain_local_items`
2. 首次执行定向测试目标后，发现 `assert` 在当前构建配置下无效，新增用例以空指针崩溃形式失败；随后将该测试文件中的断言替换为自定义 `CHECK()`，确保失败能明确暴露。
3. 再次执行定向测试，确认 RED：
   - 失败点为 `test_state_save_skips_plain_local_items`
   - 失败信息为 `CHECK failed: strstr(line, "/music/a.mp3") == NULL`
4. 最后仅修改 `am_state_save()` 的过滤逻辑，重新执行同一测试目标，确认 GREEN。

### 本次验证命令与结果

RED：

```bash
cmake --build build --target apple_music_state_test && ./bin/apple_music_state_test
```

- 结果：退出码 `134`
- 摘要：命中 `test_state_save_skips_plain_local_items`，暴露当前实现仍会写出 `/music/a.mp3`

GREEN：

```bash
cmake --build build --target apple_music_state_test && ./bin/apple_music_state_test
```

- 结果：退出码 `0`
- 摘要：`apple_music_state_test` 定向目标通过
