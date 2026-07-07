# Task 4 实现报告

## 变更概述

- 宿主页 mini player 改为通过 `am_shell_build_miniplayer_ex(...)` 接入 `am_on_open_file(...)`
- `apple_music.c` 增加单文件宿主会话态：`has_picked_file`、`picked_file`、`toast_text`
- `am_update_now_view()` 新增单文件模式分支：标题显示去后缀文件名，副标题显示 `本地文件`，收藏按钮隐藏
- 增加污染保护：`am_record_current_local_playback()` 和 `am_on_toggle_favorite()` 在单文件模式下直接返回
- 新增 `apple_music_now_view_single_file_test`，覆盖“点击 `+` 后 now view 同步且不污染状态文件”

## RED

### 命令

```bash
cmake -S . -B build && cmake --build build --target apple_music_now_view_single_file_test && ctest --test-dir build -R '^apple_music_now_view_single_file_test$' --output-on-failure
```

### 关键输出

```text
Test #16: apple_music_now_view_single_file_test ...Subprocess aborted
CHECK failed: g_is_single_file_mode (main/tests/apple_music_now_view_single_file_test.c:288)
0% tests passed, 1 tests failed out of 1
```

### 为什么这个 RED 合理

- 测试点击的是宿主页 mini player 的打开文件按钮
- 失败点是 `g_is_single_file_mode` 仍为 `false`
- 这说明点击 `+` 后宿主还没有调用 `am_player_play_single_file(...)`
- 失败原因正是 Task 4 要补的“按钮回调接线与 now view 同步”缺失，而不是测试夹具或构建配置错误

## GREEN

### 命令 1

```bash
cmake --build build --target apple_music_now_view_single_file_test && ctest --test-dir build -R '^apple_music_now_view_single_file_test$' --output-on-failure
```

### 关键输出 1

```text
Test #16: apple_music_now_view_single_file_test ...   Passed
100% tests passed, 0 tests failed out of 1
```

### 命令 2

```bash
ctest --test-dir build -R '^(apple_music_now_view_single_file_test|apple_music_single_file_player_test|apple_music_desktop_file_dialog_test)$' --output-on-failure
```

### 关键输出 2

```text
Test #10: apple_music_desktop_file_dialog_test ....   Passed
Test #16: apple_music_now_view_single_file_test ...   Passed
Test #19: apple_music_single_file_player_test .....   Passed
100% tests passed, 0 tests failed out of 3
```

## 结果说明

- 成功选择单文件后，宿主页会记录单文件会话态，但不会写入 `g_app.locals` 或状态文件
- now view 会显示 `My Song` 和 `本地文件`
- 收藏按钮会隐藏
- 单文件模式下不会写 recent，也不会切 favorite
- dialog 返回 cancel 时保持现状；返回 unavailable/error 时仅写 `toast_text`

## Reviewer Fix

### 修复内容

- 把 `apple_music_now_view_single_file_test.c` 里的 player stub 从“空桩”补成可执行测试桩
- 新增 `g_play_mode`
- `am_player_cycle_mode()` 现在按 `SEQ -> REPEAT_ONE -> REPEAT_ALL -> SHUFFLE -> SEQ` 循环
- `am_player_prev()` / `am_player_next()` 在固定资料库模式下会移动本地索引，在单文件模式下保持单文件会话不变
- `am_player_on_mode()` / `am_player_on_prev()` / `am_player_on_next()` 也改为真实转调对应 player stub
- 新增任务级用例：进入单文件模式后，在 `SEQ / REPEAT_ALL / SHUFFLE` 下通过宿主页按钮路径触发 `mode / prev / next`，验证 now view、收藏按钮、状态文件、单文件会话标记都保持正确

### Reviewer-Fix RED

#### 命令

```bash
cmake --build build --target apple_music_now_view_single_file_test && ctest --test-dir build -R '^apple_music_now_view_single_file_test$' --output-on-failure
```

#### 关键输出

```text
Test #16: apple_music_now_view_single_file_test ...Subprocess aborted
CHECK failed: g_play_mode == target_mode (main/tests/apple_music_now_view_single_file_test.c:359)
0% tests passed, 1 tests failed out of 1
```

#### 为什么这个 RED 合理

- 新用例已经通过宿主页 `mode` 按钮路径驱动模式切换
- 故障点是 `g_play_mode` 没有切到目标模式
- 这正好证明评审指出的问题存在：旧测试桩无法表达 `mode` 路径，也就无法证明单文件模式在不同播放模式下不会隐式跳回固定资料库

### Reviewer-Fix GREEN

#### 命令 1

```bash
cmake --build build --target apple_music_now_view_single_file_test && ctest --test-dir build -R '^apple_music_now_view_single_file_test$' --output-on-failure
```

#### 关键输出 1

```text
Test #16: apple_music_now_view_single_file_test ...   Passed
100% tests passed, 0 tests failed out of 1
```

#### 命令 2

```bash
ctest --test-dir build -R '^(apple_music_now_view_single_file_test|apple_music_single_file_player_test|apple_music_desktop_file_dialog_test)$' --output-on-failure
```

#### 关键输出 2

```text
Test #10: apple_music_desktop_file_dialog_test ....   Passed
Test #16: apple_music_now_view_single_file_test ...   Passed
Test #19: apple_music_single_file_player_test .....   Passed
100% tests passed, 0 tests failed out of 3
```

### 为什么现在已经形成可执行证据

- 测试桩现在同时具备两种行为：
  - 固定资料库模式下，`prev / next` 会真实移动本地索引
  - 单文件模式下，`prev / next` 不会改动固定资料库索引，也不会清掉单文件会话
- 任务级用例不是直接调用内部函数，而是走宿主页按钮路径：
  - 点击 `+` 进入单文件模式
  - 点击 `mode` 把模式切到 `SEQ / REPEAT_ALL / SHUFFLE`
  - 再点击 `prev / next`
- 每轮都验证：
  - now view 仍显示 `My Song / 本地文件`
  - 收藏按钮仍隐藏
  - 状态文件 recent/favorite 未变化
  - `g_is_single_file_mode` 仍为真
  - 固定资料库索引仍停在原值
- 因此，“不会因模式切换或传输键隐式跳回固定资料库”已经从口头推断变成了可执行、可回归的任务级证据

## Fresh Verification Fix

### 现象

- 单独跑 `apple_music_now_view_single_file_test` 可以通过
- 组合跑
  `ctest --test-dir build -R '^(apple_music_now_view_single_file_test|apple_music_single_file_player_test|apple_music_desktop_file_dialog_test)$' --output-on-failure`
  时，曾出现：

```text
Test #16: apple_music_now_view_single_file_test ...Subprocess aborted
CHECK failed: loaded[0].recent_seq == 4U (main/tests/apple_music_now_view_single_file_test.c:368)
```

- 失败后仓库根目录的受控文件 `apple_music_state.tsv` 会落成工作树删除状态

### 根因

- `apple_music_now_view_single_file_test` 之前直接在仓库根目录运行
- 该测试使用 `AM_STATE_PATH`，也就是相对路径 `apple_music_state.tsv`
- 因此测试实际是在直接改写并删除仓库受控文件，而不是在隔离夹具中操作临时状态文件
- 一旦前一次运行中断或失败，工作树就会留下删除/脏内容；后续组合回归再进入同一测试时，就可能读到遗留状态，从而表现出“不单独跑、组合跑才炸”的不稳定性

### 修复

- 给 `apple_music_now_view_single_file_test` 增加独立临时工作目录夹具
- 测试启动时 `mkdtemp(...)` 并 `chdir(...)` 到临时目录
- 测试结束时仅删除临时目录里的 `apple_music_state.tsv`，再回到原工作目录
- 这样 `AM_STATE_PATH` 仍保持不变，但它指向的是隔离的临时测试文件，而不是仓库根下的受控文件

### 验证

#### 命令 1

```bash
ctest --test-dir build -R '^apple_music_now_view_single_file_test$' --output-on-failure
```

#### 关键输出 1

```text
Test #16: apple_music_now_view_single_file_test ...   Passed
100% tests passed, 0 tests failed out of 1
```

#### 命令 2

```bash
ctest --test-dir build -R '^(apple_music_now_view_single_file_test|apple_music_single_file_player_test|apple_music_desktop_file_dialog_test)$' --output-on-failure
```

#### 关键输出 2

```text
Test #10: apple_music_desktop_file_dialog_test ....   Passed
Test #16: apple_music_now_view_single_file_test ...   Passed
Test #19: apple_music_single_file_player_test .....   Passed
100% tests passed, 0 tests failed out of 3
```

### 最终稳定性说明

- `apple_music_now_view_single_file_test` 现在不再依赖仓库根目录的 `apple_music_state.tsv`
- 单跑与组合回归都稳定通过
- 测试执行后，`apple_music_state.tsv` 不再被这个测试留成工作树删除状态
- 本次 fresh-verification 修复没有扩大到生产代码，问题已确认是测试夹具隔离不足，而不是 Task 4 宿主逻辑再次回归

## Task 5 Build Fix

### 现象

- `cmake --build build --target main` 链接失败：

```text
apple_music.c:(.text+0xcd9): undefined reference to `am_desktop_file_dialog_pick_audio'
```

- `cmake --build build --target apple_music_now_view_state_test` 链接失败：

```text
undefined reference to `am_player_is_single_file_mode'
undefined reference to `am_desktop_file_dialog_pick_audio'
undefined reference to `am_player_play_single_file'
```

### 根因

- `main` 目标通过 `V9_APPLE_MUSIC_SOURCES` 组装 Apple Music 源文件，但 Task 4 新引入的 `am_desktop_file_dialog.c` 只被单独测试目标显式引用，没有进入 `main`
- 旧的 `apple_music_now_view_state_test.c` stub 仍停留在 Task 4 之前，缺少 `apple_music.c` 新增依赖的最小符号实现

### 修复

- 在 `V9_APPLE_MUSIC_SOURCES` 中加入 `main/src/v9_apple_music/am_desktop_file_dialog.c`
- 在 `main/tests/apple_music_now_view_state_test.c` 增加最小兼容 stub：
  - `am_player_is_single_file_mode()` 返回 `false`
  - `am_player_play_single_file(...)` 仅做空实现
  - `am_player_clear_single_file_mode()` 仅做空实现
  - `am_desktop_file_dialog_pick_audio(...)` 返回 `AM_FILE_PICK_CANCEL`
- 这些 stub 只为满足旧测试的链接与既有语义，不把单文件模式逻辑扩进这套旧测试

### 验证

#### 命令 1

```bash
cmake --build build --target apple_music_now_view_state_test
```

#### 结果 1

```text
Built target apple_music_now_view_state_test
```

#### 命令 2

```bash
cmake --build build --target main
```

#### 结果 2

```text
Built target main
```

#### 命令 3

```bash
ctest --test-dir build -R '^apple_music_' --output-on-failure
```

#### 结果 3

```text
100% tests passed, 0 tests failed out of 9
```
