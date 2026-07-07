# Task 2 报告

## 状态

DONE

## 执行记录

1. 已读取唯一需求源 `.superpowers/sdd/task-2-brief.md`，按 brief 约束尝试执行 TDD。
2. 已确认当前工作树基线为 `39ea04f`，且未触碰无关脏文件 `third-party/hls_player_demo` 与 `21.png`。
3. 已尝试使用 GitNexus 做预编辑分析，但当前环境中该仓库未出现在 MCP 索引列表；随后执行 `npx gitnexus analyze` 也因 native worker/binding 异常中止，无法得到可用的 impact / detect_changes 结果。
4. 已先把 `main/tests/apple_music_data_test.c` 改成显式失败风格，验证 RED：
   - `cmake --build build --target apple_music_data_test && ./bin/apple_music_data_test`
   - 失败点为 `local[i].favorite == false`，证明“扫描阶段伪收藏”确实存在。
5. 已在允许写面内尝试最小实现：
   - `am_local_scan.c` 去掉扫描默认收藏并清零 `recent_seq`
   - `apple_music.c` 接入 `am_state_load/save/mark_recent`
6. GREEN/编译验证时触发硬阻塞：
   - `cmake --build build --target apple_music_data_test main`
   - `apple_music_data_test` 可通过
   - `main` 链接失败，缺少 `am_state_load` / `am_state_save` / `am_state_mark_recent`

## 阻塞原因

当前 `main` 目标的源集合没有把 `main/src/v9_apple_music/am_state.c` 链进去。要完成 Task 2，必须修改 `CMakeLists.txt`（或等价构建配置）把 `am_state.c` 纳入 `V9_APPLE_MUSIC_SOURCES` / `main` 目标。

但你当前明确限制了写面只允许：

- `main/src/v9_apple_music/am_local_scan.c`
- `main/src/v9_apple_music/apple_music.c`
- `main/tests/apple_music_data_test.c`

并要求“如果发现必须改出这三个文件，先停下并报 NEEDS_CONTEXT”。因此我没有继续改 `CMakeLists.txt`，并且已经把三个允许写面的临时改动全部回滚，避免把仓库留在半成品状态。

## 需要的上下文/放权

至少需要允许修改以下文件之一：

- `CMakeLists.txt`

建议变更：

- 将 `main/src/v9_apple_music/am_state.c` 加入 `V9_APPLE_MUSIC_SOURCES`

拿到该写面后，可以按同一 TDD 路径快速完成：

1. 重做 `apple_music_data_test.c` 的 RED
2. 实现 `am_local_scan.c` 与 `apple_music.c` 的最小改动
3. 验证 `apple_music_data_test` GREEN
4. 验证 `main` 重新链接成功
5. 再做一次变更范围检查并提交

## 继续执行结果

1. 已获得额外写面 `CMakeLists.txt`，并按原 RED 路径继续执行。
2. 重新验证 RED：
   - `cmake --build build --target apple_music_data_test && ./bin/apple_music_data_test`
   - 失败点仍为 `local[i].favorite == false`
3. 已完成最小实现：
   - `main/tests/apple_music_data_test.c`
     - 把数据测试切换为显式 `CHECK`，避免 Release 构建下 `assert` 被 `-DNDEBUG` 吃掉
     - 锁定“扫描结果中全部 `favorite == false` 且 `recent_seq == 0U`”
   - `main/src/v9_apple_music/am_local_scan.c`
     - 去掉扫描阶段的假收藏初始化
     - 显式清零 `recent_seq`
   - `main/src/v9_apple_music/apple_music.c`
     - 新增 `am_state.h` include
     - 新增 `g_recent_seq_next`
     - 新增 `am_save_local_state()` / `am_record_current_local_playback()`
     - 在 `am_load_runtime_data()` 中加载持久化状态
     - 在本地切歌入口 `am_on_local_selected()` / `am_on_play_pause()` / `am_on_prev()` / `am_on_next()` / `am_on_playlist_pick()` 后记录最近播放
   - `CMakeLists.txt`
     - 仅把 `main/src/v9_apple_music/am_state.c` 纳入 `V9_APPLE_MUSIC_SOURCES`
4. GREEN 与链接验证：
   - `cmake --build build --target apple_music_data_test apple_music_state_test main`
   - `./bin/apple_music_data_test`
   - `./bin/apple_music_state_test`
   - 以上均通过，且 `main` 成功重新链接。

## 提交

- 已创建 commit：`feat: wire apple music runtime state updates`
