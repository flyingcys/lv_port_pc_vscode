# Apple Music 播放列表可拖动滚动条设计

## 背景

`main/src/v9_apple_music` 的播放列表弹层当前已经支持纵向滚动，但用户只能通过滚轮或拖动内容区滚动。右侧虽然可以显示标准滚动条视觉提示，但缺少一个明确、可直接拖拽的竖向拖动条，用户无法通过拖动条本身快速浏览长列表。

本次需求是在播放列表右侧增加一个符合标准滚动条视觉习惯的竖向可拖动条，用来表示下方仍有内容，并允许用户直接拖动该条滚动列表。

## 目标

- 当播放列表内容超过当前可视高度时，在右侧显示竖向滚动条。
- 滚动条需要支持鼠标直接拖拽，拖动 thumb 时同步滚动播放列表内容。
- 列表内容未超出可视区域时，滚动条整体隐藏。
- 外观保持标准滚动条风格，不引入与当前 UI 风格冲突的独立控件视觉。

## 非目标

- 不修改播放列表条目内容、排序、点击切歌逻辑。
- 不调整播放列表弹层的整体布局结构，除非为放置滚动条必须做最小间距调整。
- 不修改广播电台与本地歌曲的数据来源和数量逻辑。
- 不实现惯性拖拽、分页吸附或额外动画。

## 现状

当前播放列表弹层由 `am_shell_build_miniplayer()` 构建：

- `playlist_popup` 是浮层容器。
- `playlist_list` 是真实的可滚动内容容器。
- `playlist_list` 已启用纵向滚动。
- 当前右侧仅依赖 LVGL 标准滚动行为，缺少一个可直接拖动的专用 thumb。

当前播放列表内容由 `am_refresh_playlist_popup()` 重建，列表项数量会随播放源变化而变化，因此滚动条状态必须能在重建后重新计算。

## 方案对比

### 方案一：只使用 LVGL 原生滚动条

做法：

- 保留 `playlist_list` 的原生滚动行为。
- 仅通过 `LV_PART_SCROLLBAR` 强化右侧滚动条视觉。

优点：

- 改动最小。
- 维护成本低。

缺点：

- 只能提供视觉提示，无法可靠满足“拖动条本身可拖”的需求。
- 交互能力不足，和需求不匹配。

### 方案二：自定义标准滚动条外观与拖拽交互

做法：

- 在 `playlist_popup` 内新增滚动条轨道和 thumb。
- `playlist_list` 继续作为真实滚动容器。
- thumb 位置与大小由 `playlist_list` 的滚动状态驱动。
- thumb 的拖动事件反向驱动 `playlist_list` 滚动。

优点：

- 精确满足“竖向可拖动条”的需求。
- 保留标准滚动条视觉，同时补齐交互。
- 改动范围可控制在播放列表弹层内部。

缺点：

- 需要维护滚动映射与拖动事件。
- 需要额外测试显隐和同步逻辑。

### 结论

采用方案二。

原因是本次需求的核心不是“看得到滚动提示”，而是“用户可以直接拖右侧的条”。仅依赖 LVGL 原生滚动条无法稳定满足这一点，因此需要自定义一层轻量交互封装。

## 结构设计

### 句柄扩展

在 `am_miniplayer_handles_t` 中新增：

- `playlist_scroll_track`
- `playlist_scroll_thumb`

其中：

- `playlist_scroll_track` 挂在 `playlist_popup` 下，与 `playlist_list` 同级。
- `playlist_scroll_thumb` 挂在 `playlist_scroll_track` 下，作为轨道内部的可拖拽块。

### 布局关系

- `playlist_list` 仍然占据弹层正文主体区域。
- `playlist_scroll_track` 固定贴在 `playlist_list` 右侧。
- `playlist_scroll_thumb` 作为 `playlist_scroll_track` 的子对象，用于表示当前可视位置。

如果需要为滚动条留出点击空间，可以对 `playlist_list` 的右侧内边距做极小调整，但不改变整体布局方向和现有 header 结构。

## 交互设计

### 显示规则

- 当 `playlist_list` 内容高度小于等于可视高度时：
  - `playlist_scroll_track` 隐藏
  - `playlist_scroll_thumb` 隐藏
- 当内容高度大于可视高度时：
  - 显示轨道和 thumb
  - thumb 高度按可视高度占比自动计算
  - thumb 高度设置最小值，避免列表很长时过短难拖

### 同步规则

滚动条状态由以下数据决定：

- `content_height`：内容总高度
- `viewport_height`：当前可视高度
- `scroll_y`：当前列表纵向滚动偏移

映射逻辑：

- `thumb_height = max(min_thumb_height, track_height * viewport_height / content_height)`
- `thumb_y = (track_scroll_range * scroll_y / content_scroll_range)`

其中：

- `track_scroll_range = track_height - thumb_height`
- `content_scroll_range = content_height - viewport_height`

### 拖拽规则

- 用户按下 thumb 后进入拖拽状态。
- 按下时记录指针落点相对 thumb 顶部的偏移，避免拖拽开始瞬间跳变。
- 拖动过程中，用当前指针位置减去按下偏移，换算 thumb 在轨道中的目标位置。
- 再由 thumb 位置反推 `playlist_list` 应滚动到的目标偏移。
- 每次拖动都直接更新 `playlist_list` 的滚动位置。
- 拖动结束后退出拖拽状态。

### 滚动来源兼容

以下行为都应驱动 thumb 更新：

- 鼠标滚轮滚动 `playlist_list`
- 直接拖动 `playlist_list` 内容区
- 代码触发的列表滚动或内容重建

## 视觉设计

- 滚动条采用标准细竖条样式。
- 轨道使用低对比度浅灰或低透明深色，避免喧宾夺主。
- thumb 使用更实一些的颜色和圆角，保证可见但不过分抢眼。
- 当列表不可滚动时完全隐藏，而不是仅禁用。

## 实现边界

本次改动仅限以下区域：

- `main/src/v9_apple_music/am_player.h`
- `main/src/v9_apple_music/am_shell.c`
- `main/src/v9_apple_music/am_player.c`
- 与本需求直接相关的测试文件

不触碰播放条、侧边栏、歌曲数据扫描逻辑和其他页面逻辑。

## 测试设计

### 自动化测试

新增或扩展播放列表弹层测试，至少覆盖：

1. 长列表时存在右侧滚动条句柄且对象可见。
2. 短列表时滚动条隐藏。
3. 滚动条布局存在有效宽度和高度。

如测试环境允许，再补一条拖动 thumb 后列表滚动偏移变化的断言；若当前 LVGL 单测环境难以稳定模拟指针拖拽，则先保证显隐和布局映射逻辑有测试覆盖。

### 人工验证

通过无头截图或本地运行确认：

- 长列表时右侧出现竖向滚动条。
- 首屏之外仍有内容时，滚动条位置能体现当前浏览位置。
- 拖动 thumb 时列表随之滚动。

## 风险与应对

### 风险一：列表内容重建后 thumb 状态失效

原因：

- `am_refresh_playlist_popup()` 会在内容变化时重建列表项。

应对：

- 在列表重建完成后统一刷新滚动条显隐、大小和位置。

### 风险二：thumb 拖拽与内容区滚动互相干扰

原因：

- thumb 和列表都属于可交互区域。

应对：

- thumb 独立处理拖拽事件，只在 thumb 上消费拖拽输入。
- 内容区原有滚动逻辑保持不变。

### 风险三：滚动映射边界不准确

原因：

- 内容高度、可视高度和滚动偏移之间存在边界取整问题。

应对：

- 所有计算统一夹紧到合法区间。
- 空列表、单屏列表和超长列表分别覆盖测试。

## 成功标准

- 播放列表内容超出当前显示长度时，右侧出现竖向滚动条。
- 用户可通过拖动滚动条 thumb 浏览下方内容。
- 内容不足一屏时不显示滚动条。
- 不影响现有点击切歌、滚轮滚动和内容区拖动滚动行为。
