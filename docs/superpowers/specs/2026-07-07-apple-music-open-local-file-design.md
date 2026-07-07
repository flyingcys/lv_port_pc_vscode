# Apple Music 打开本地文件设计

日期：2026-07-07

## 背景

当前 `main/src/v9_apple_music` 已具备以下能力：

- 底部迷你播放器已支持播放、暂停、上一首、下一首、播放模式切换、播放列表弹层。
- 本地音频来源仅支持启动时扫描固定目录 `third-party/hls_player_demo/test_file`。
- 底层 `music_player_init(const char **urls, size_t count)` 已支持播放单个本地文件。

当前缺口：

- 没有“打开本地文件”按钮。
- 没有桌面系统文件选择框能力。
- 没有“运行时选中任意单个本地音频文件并立即播放”的 Apple Music 上层接入。

本设计目标是在桌面环境中新增系统文件选择框入口，允许用户选择任意单个本地音频文件，并在 Apple Music 页面内以会话态临时播放该文件。

## 目标

- 在底部迷你播放器增加“打开本地文件”按钮。
- 点击后弹出桌面系统文件选择框。
- 用户选择单个音频文件后，立即切换到该文件播放。
- “正在播放”页展示所选文件名，副标题展示“本地文件”。
- 单文件模式下继续支持播放、暂停、播放模式切换和播放列表弹层。
- 单文件模式不污染固定扫描资料库，不写入收藏和最近播放持久化状态。

## 非目标

- 不支持一次选择多个文件。
- 不支持把所选文件永久导入固定资料库。
- 不支持在“我的收藏”或侧边栏“最近播放”中展示该临时文件。
- 不新增复杂桌面 UI 文件浏览器，仅使用系统文件选择框。

## 现状约束

### 数据与页面结构

- `apple_music.c` 负责页面宿主状态、视图切换、标题副标题刷新、收藏和最近播放同步。
- `am_shell.c` 负责迷你播放器 UI 构建。
- `am_player.c` 负责播放器来源、播放列表弹层、播放控制和 UI 刷新。
- `am_local_scan.c` 只负责固定目录扫描，不适合承载运行时单文件选择。

### 播放器能力

- 固定本地库通过 `am_player_play_local_index()` 进入播放链。
- 电台通过 `am_player_play_radio_index()` 进入播放链。
- 底层 `music_player` 能接受任意本地 `file://` URL 或本地文件路径列表。

### 工程环境

- 当前工程已接入 `SDL2`，但仓库中没有现成的原生文件选择封装。
- 当前仓库未建立可用 GitNexus 索引，本轮设计基于本地源码关系判断，不依赖 GitNexus impact 输出。

## 方案比较

### 方案 A：新增独立来源类型 `AM_SOURCE_PICKED_FILE`

做法：

- 在 `am_source_kind_t` 中新增单独来源类型。
- 为标题、副标题、播放列表、上一首下一首、收藏和最近播放分别增加分支。

优点：

- 语义最直观。

缺点：

- 分支面最大，页面与播放器状态代码都要扩散。
- 与现有“本地音频”逻辑复用较少。

### 方案 B：把选中文件并入 `g_app.locals`

做法：

- 选择文件后临时插入本地列表，再走现有索引播放链。

优点：

- 复用现有索引播放逻辑。

缺点：

- 会污染固定扫描资料库语义。
- 上一首、下一首、随机、列表循环可能跳回原资料库。
- 持久化状态边界变复杂。

### 方案 C：临时单文件会话态

做法：

- `apple_music.c` 维护一个临时单文件元数据。
- `am_player.c` 提供单文件播放接口，并在内部维护“当前是否处于临时单文件模式”。
- UI 仍按“本地音频”风格展示，但不把该文件并入固定扫描资料库。

优点：

- 改动面可控。
- 保持固定资料库与临时文件会话态解耦。
- 可以复用大部分本地音频展示与播放链路。

缺点：

- 需要在播放器和页面同步路径中补少量“单文件模式”判断。

结论：采用方案 C。

## 总体设计

### 1. 新增桌面文件选择适配层

新增文件：

- `main/src/v9_apple_music/am_desktop_file_dialog.h`
- `main/src/v9_apple_music/am_desktop_file_dialog.c`

职责：

- 提供 `am_desktop_file_dialog_pick_audio(...)` 接口。
- 在 Linux 桌面环境中按顺序尝试系统命令：
  - `zenity`
  - `qarma`
  - `kdialog`
- 成功时返回用户选中的单个绝对路径。
- 用户取消时返回“取消”状态。
- 系统不支持或命令执行失败时返回错误状态。

接口建议：

```c
typedef enum {
    AM_FILE_PICK_OK = 0,
    AM_FILE_PICK_CANCEL,
    AM_FILE_PICK_UNAVAILABLE,
    AM_FILE_PICK_ERROR,
} am_file_pick_result_t;

am_file_pick_result_t am_desktop_file_dialog_pick_audio(char *path_buf, size_t path_buf_size);
```

说明：

- 只返回一个文件路径。
- 只接受音频相关后缀过滤。
- 不在该层直接操作播放器或 UI。

### 2. 新增临时单文件会话态

在 `apple_music.c` 宿主状态中新增临时单文件字段，例如：

- `bool has_picked_file`
- `am_local_item_t picked_file`
- `char toast_text[...]`

设计原则：

- 该结构仅代表当前会话里的单文件播放项。
- 不写入 `g_app.locals`。
- 不参与 `am_state_save()`。

状态转换：

- 用户成功选择文件：写入 `picked_file`，置 `has_picked_file=true`。
- 用户重新选择固定本地曲目或电台：清除该状态。

### 3. 扩展播放器为“单文件模式”

修改：

- `main/src/v9_apple_music/am_player.h`
- `main/src/v9_apple_music/am_player.c`

新增能力：

- `am_player_play_single_file(const char *path, const char *title)`
- `am_player_is_single_file_mode(void)`

内部行为：

- 单文件模式下，播放器使用单元素 URL 列表重新初始化 `music_player`。
- 来源语义仍视为本地音频，以便复用进度条、seek、时长、非直播 UI。
- 播放列表弹层只展示当前单文件 1 行。
- `上一首/下一首` 在单文件模式下不切换到固定本地库，也不切电台。
- `顺序播放`、`列表循环`、`随机播放` 在单文件模式下都保持当前文件不跳源。
- `单曲循环` 仍交给底层播放模式处理。

### 4. 扩展迷你播放器 UI

修改：

- `main/src/v9_apple_music/am_shell.h`
- `main/src/v9_apple_music/am_shell.c`
- `main/src/v9_apple_music/am_player.h`

改动内容：

- 在 `am_miniplayer_handles_t` 中新增 `btn_open_file`。
- `am_shell_build_miniplayer()` 在 `btn_next` 和 `btn_playlist` 之间插入“打开本地文件”按钮。
- 新按钮使用新的图标常量，例如 `AM_ICON_OPEN_FILE`。
- 迷你播放器构建函数新增 `on_open_file` 回调参数。

播放模式按钮视觉也同步修正：

- 现有模式按钮图标固定为 `AM_ICON_REPLAY`。
- 需要改成由 `am_player_refresh_ui()` 根据当前 `music_player_get_play_mode()` 动态刷新。
- 视觉目标与 HTML 版一致：顺序、单曲、列表、随机四态可区分。

### 5. 宿主页面接入“打开本地文件”

修改：

- `main/src/v9_apple_music/apple_music.c`

新增回调：

- `static void am_on_open_file(lv_event_t *e);`

行为：

1. 调用 `am_desktop_file_dialog_pick_audio()`。
2. 若返回取消：直接返回。
3. 若返回不可用或失败：写提示文本并保持当前播放状态。
4. 若返回成功：
   - 从文件名生成标题。
   - 填充 `g_app.picked_file.path/title`。
   - 清空当前电台态。
   - 调用 `am_player_play_single_file(path, title)`。
   - 切换到 `AM_VIEW_NOW`。
   - 刷新标题、副标题、封面、歌词区域和迷你播放器。

`am_update_now_view()` 的单文件显示规则：

- 标题：文件名去后缀。
- 副标题：`本地文件`
- 封面：沿用本地歌曲封面样式。
- 歌词：隐藏，不显示静态歌词。
- 收藏按钮：隐藏。

### 6. 持久化与列表行为

明确约束：

- 单文件模式不写入 `apple_music_state.tsv`。
- `am_record_current_local_playback()` 在单文件模式下直接返回。
- `am_on_toggle_favorite()` 在单文件模式下直接返回。
- 侧边栏“最近播放”只展示固定扫描资料库里的记录。
- “我的收藏”只展示固定资料库中 `favorite=true` 的记录。

### 7. 播放列表弹层行为

修改：

- `am_refresh_playlist_popup()`

规则：

- 单文件模式下只渲染一行。
- 该行标题显示当前文件名。
- 副标题显示 `本地文件`。
- 行高亮始终为当前项。
- 点击该行不切源，仅关闭弹层或保持当前状态均可。

推荐行为：

- 点击后关闭弹层，不触发额外切歌。

## 错误处理

### 文件选择器不可用

条件：

- 未找到任一支持的系统文件选择命令。

行为：

- 不改变当前播放状态。
- 页面显示轻量提示：`当前桌面环境不支持文件选择`。

### 用户取消选择

行为：

- 静默返回。
- 不提示，不刷新当前页。

### 选择成功但播放初始化失败

行为：

- 不覆盖当前已在播放的来源状态。
- 显示提示：`打开文件失败`。

## 测试与验证

### 代码级验证

- 为 `am_desktop_file_dialog` 增加最小可替换命令测试或注入式验证，覆盖：
  - 成功返回路径
  - 用户取消
  - 命令不可用
  - 命令执行失败

### 播放控制回归

- 验证单文件模式下：
  - 播放 / 暂停
  - 顺序 / 单曲 / 列表 / 随机 模式切换
  - 播放列表弹层
  - 从单文件切回固定本地库
  - 从单文件切到电台

### 手工验证

- 桌面启动 Apple Music 页面。
- 点击“打开本地文件”。
- 选择一个本地 `mp3` 或 `wav`。
- 确认：
  - 切到“正在播放”
  - 标题为文件名
  - 副标题为“本地文件”
  - 可播放、暂停
  - 模式按钮视觉可切换
  - 播放列表弹层仅显示该文件
  - 收藏与最近播放未被污染

## 风险与控制

### 风险 1：单文件模式与固定本地库索引语义冲突

控制：

- 单文件状态不进入 `g_app.locals`。
- 在 `am_player.c` 内部显式维护单文件模式。

### 风险 2：播放模式按钮有行为但没有正确视觉反馈

控制：

- 本轮把按钮图标刷新一并纳入改动范围。

### 风险 3：回调期间同步重建列表导致对象生命周期问题

控制：

- 继续沿用当前 `lv_async_call` 的推迟切换策略。
- 新按钮回调不直接销毁正在处理事件的列表对象。

### 风险 4：系统文件选择命令依赖宿主环境

控制：

- 做多命令回退。
- 明确“不可用”提示。
- 不引入更重的 GUI 库依赖。

## 预计改动文件

- 新增 `main/src/v9_apple_music/am_desktop_file_dialog.h`
- 新增 `main/src/v9_apple_music/am_desktop_file_dialog.c`
- 修改 `main/src/v9_apple_music/am_icons.h`
- 修改 `main/src/v9_apple_music/am_shell.h`
- 修改 `main/src/v9_apple_music/am_shell.c`
- 修改 `main/src/v9_apple_music/am_player.h`
- 修改 `main/src/v9_apple_music/am_player.c`
- 修改 `main/src/v9_apple_music/apple_music.c`

## 成功标准

- 用户可以在桌面环境中通过系统文件选择框选择任意单个本地音频文件。
- 选中文件后能够立即播放，并在 Apple Music 页面正确展示。
- 单文件模式下的播放控制、模式切换和播放列表行为符合设计。
- 固定扫描资料库、收藏、最近播放和持久化状态不被临时单文件污染。
