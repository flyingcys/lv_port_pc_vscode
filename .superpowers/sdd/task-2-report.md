# Task 2 实现报告

## 目标

按 `task-2-brief.md` 实现播放器内部的单文件会话态，范围仅限：

- 单文件播放入口与状态查询/清理接口
- 播放列表弹层在单文件模式下只渲染一行
- `prev` / `next` 在单文件模式下不离开当前文件
- 对播放器标题/副标题做最小必要刷新

未实现：

- Task 3/4 的按钮接线
- 宿主页同步
- 超出 brief 的播放模式扩展

## 实现摘要

- 在 `am_player` 内新增单文件模式状态、路径和标题缓存。
- 新增 `am_player_play_single_file()`、`am_player_is_single_file_mode()`、`am_player_clear_single_file_mode()`。
- 单文件模式下：
  - 标题显示传入文件标题
  - 副标题显示 `单个文件`
  - 播放列表弹层固定渲染 1 行
  - `am_player_prev()` / `am_player_next()` 只刷新 UI，不切出当前文件
- 当切回本地库或电台播放时，清除单文件模式标记。

## TDD 记录

### RED

命令：

```bash
cmake -S . -B build && cmake --build build --target apple_music_single_file_player_test && ctest --test-dir build -R '^apple_music_single_file_player_test$' --output-on-failure
```

关键输出：

```text
undefined reference to `am_player_play_single_file'
undefined reference to `am_player_is_single_file_mode'
undefined reference to `am_player_clear_single_file_mode'
```

为什么这个 RED 合理：

- 新测试先声明了 brief 要求的新接口和行为。
- 当时生产代码里还没有这些接口定义，链接失败正好证明测试确实覆盖到了“尚未实现的能力”，不是误测已有行为。

### GREEN

命令：

```bash
cmake --build build --target apple_music_single_file_player_test && ctest --test-dir build -R '^apple_music_single_file_player_test$' --output-on-failure
```

关键输出：

```text
1/1 Test #18: apple_music_single_file_player_test ...   Passed
100% tests passed, 0 tests failed out of 1
```

## 最终验证

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

## 写面确认

本次只修改了 brief 允许的文件：

- `main/src/v9_apple_music/am_player.h`
- `main/src/v9_apple_music/am_player.c`
- `main/tests/apple_music_single_file_player_test.c`
- `CMakeLists.txt`
- `.superpowers/sdd/task-2-report.md`

## 关注点

- 当前副标题文案固定为 `单个文件`，这是按“最小必要刷新”实现的内部态展示；宿主页文案同步仍留给后续任务。
- 单文件模式下播放列表条目目前不提供切歌交互，符合本任务只做内部会话态的边界。

## 后续修复记录

### 问题

- 播放列表缓存指纹 `g_pl_*` 之前只看 `来源 / 当前索引 / 条目数`。
- 单文件模式沿用 `AM_SOURCE_LOCAL`，如果先构建过本地库列表，再切到单文件模式，就可能命中旧指纹直接返回，导致弹层保留旧的多行本地列表。

### 本次修复

- 在播放列表缓存指纹中加入 `g_single_file_mode`。
- 这样切入或切出单文件模式时，播放列表重建判定一定失效并重建。

### 本次 TDD

#### RED

命令：

```bash
cmake --build build --target apple_music_single_file_player_test && ctest --test-dir build -R '^apple_music_single_file_player_test$' --output-on-failure
```

关键输出：

```text
CHECK failed: lv_obj_get_child_count(handles.playlist_list) == 1U
```

为什么这个 RED 合理：

- 新增回归用例先构建 2 行本地库播放列表，再切到 `am_player_play_single_file()`。
- 失败点正是“切到单文件后仍然保留旧的 2 行列表”，直接锁定了缓存指纹漏掉单文件态这个根因。

#### GREEN

命令：

```bash
cmake --build build --target apple_music_single_file_player_test && ctest --test-dir build -R '^apple_music_single_file_player_test$' --output-on-failure
```

关键输出：

```text
1/1 Test #18: apple_music_single_file_player_test ...   Passed
100% tests passed, 0 tests failed out of 1
```

### 本次覆盖验证

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

## Reviewer Fix 记录 2

### 问题

- reviewer 这轮不再质疑 `clear` 语义，而是要求补齐全局约束的证据闭环：
  - 顺序播放
  - 列表循环
  - 随机播放
- 需要证明单文件会话下，这些模式变化不会隐式切回固定资料库。
- 旧测试桩里 `music_player_get_play_mode()` / `music_player_cycle_play_mode()` 固定返回 `MP_MODE_SEQ`，即使生产代码正确，也无法把 `REPEAT_ALL / SHUFFLE` 的约束变成可执行证据。

### 本次修复

- 把 `apple_music_single_file_player_test.c` 中的播放模式桩改成真实持有状态并按顺序切换：
  - `SEQ -> REPEAT_ONE -> REPEAT_ALL -> SHUFFLE -> SEQ`
- 新增单测覆盖：
  - 先注入一个固定本地库上下文
  - 再进入单文件会话
  - 在 `SEQ / REPEAT_ALL / SHUFFLE` 下分别执行 `next / prev`
  - 每一步都验证单文件会话不变量
- 在 `am_player_prev()` / `am_player_next()` 的单文件短路分支补注释，明确该路径与播放模式无关，不能切回固定资料库。

### 本次 TDD

#### RED

这一轮的 RED 落在“证据能力不足”本身：

- 旧桩始终返回 `MP_MODE_SEQ`，无法表达 `REPEAT_ALL / SHUFFLE` 的真实模式迁移。
- 也就是说，在补桩之前，reviewer 要求的断言不可验证，证据链天然是红的。

#### GREEN

命令：

```bash
cmake --build build --target apple_music_single_file_player_test && ctest --test-dir build -R '^apple_music_single_file_player_test$' --output-on-failure
```

关键输出：

```text
1/1 Test #18: apple_music_single_file_player_test ...   Passed
100% tests passed, 0 tests failed out of 1
```

### 为什么这个新测试足以证明三种模式不会跳回固定资料库

- 测试先显式注入了一个 2 条目的固定本地库，这样“跳回资料库”有真实目标，不是空场景自证。
- 单文件会话建立后，测试在 `SEQ / REPEAT_ALL / SHUFFLE` 下都执行了 `next / prev`。
- 每个模式后都验证以下不变量：
  - `am_player_is_single_file_mode()` 仍为 `true`
  - 播放引擎 `music_player_get_count()` 仍为 `1`
  - UI 标题/副标题仍显示单文件内容，而不是固定资料库条目
  - 播放列表弹层仍只有 `1` 行
- 这组断言合起来证明：模式变化只改变 play mode，不会把来源、队列或 UI 恢复成固定资料库。

### 本次覆盖验证

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

## Reviewer Fix 记录

### 问题

- `am_player_clear_single_file_mode()` 之前只清除了单文件 flag 和文本缓存。
- 在单文件播放后调用它，`g_source_kind` 仍保留为 `AM_SOURCE_LOCAL`，播放器会继续带着“当前固定本地库条目”的上下文。
- 这不满足“单文件模式不污染固定资料库上下文”的闭环要求。

### 本次修复

- `am_player_clear_single_file_mode()` 在当前确实处于单文件模式时：
  - 停止当前单文件播放会话
  - 将来源重置为 `AM_SOURCE_NONE`
  - 重置当前本地/电台索引
  - 清空单文件缓存
  - 刷新 UI
- `am_refresh_playlist_popup()` 在 `AM_SOURCE_NONE` 时不再继续渲染本地库列表，避免 neutral state 下仍伪装成固定本地库当前项。

### 本次 TDD

#### RED

命令：

```bash
cmake --build build --target apple_music_single_file_player_test && ctest --test-dir build -R '^apple_music_single_file_player_test$' --output-on-failure
```

关键输出：

```text
CHECK failed: am_player_source_kind() == AM_SOURCE_NONE
```

为什么这个 RED 合理：

- 扩展后的 clear 用例先注入本地库 sources，再进入单文件会话并调用 `am_player_clear_single_file_mode()`。
- 失败点直接证明 clear 后仍保留了 `AM_SOURCE_LOCAL` 上下文，正是 reviewer 指出的语义缺口。

#### GREEN

命令：

```bash
cmake --build build --target apple_music_single_file_player_test && ctest --test-dir build -R '^apple_music_single_file_player_test$' --output-on-failure
```

关键输出：

```text
1/1 Test #18: apple_music_single_file_player_test ...   Passed
100% tests passed, 0 tests failed out of 1
```

### 本次覆盖验证

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
