# macOS 音乐播放器 LVGL 复刻设计

- 日期：2026-06-26
- 状态：设计已确认，待实现计划
- HTML 来源：`design-ui/music/music-player-macos.html`
- 目标入口：desktop 的 `Apple Music` 图标
- 播放后端：`third-party/hls_player_demo`

## 1. 目标

把 `design-ui/music/music-player-macos.html` 复刻为 LVGL 原生 UI，并替换 desktop 下 `Apple Music` 图标当前打开的旧播放器界面。播放器接入已有音频后端：

- 本地音乐：先用 `third-party/hls_player_demo/test_file`
- 广播源：先用 `third-party/hls_player_demo/qa/production_test/config/sources.csv`

本次目标既包含视觉复刻，也包含真实播放接线。

## 2. 范围

### 2.1 包含

- 800×480 下复刻 macOS 风格音乐播放器
- 左侧栏 3 个主视图：`正在播放`、`广播电台`、`我的收藏`
- 底部播放器条：播放/暂停、上一首、下一首、播放模式、播放列表、音量、进度
- `Apple Music` 图标继续作为 desktop 入口
- 接入 `music_player` 封装层，驱动 `third-party/hls_player_demo`
- 广播列表读取 `sources.csv`
- 本地列表从 `test_file` 目录构建
- 旧 Apple Music UI 从入口处被替换
- 保留 `local_music_demo` 模块本身，不要求删除网易云音乐入口

### 2.2 不包含

- 网络封面抓取
- 歌词滚动同步
- 拖拽进度条 seek
- 广播源搜索、分页、分类筛选
- 多分辨率完整适配

## 3. 现状

### 3.1 desktop 入口

- `main/desktop/desktop_data.c` 中，`Apple Music` 已绑定 `apple_music_app_launch`
- `main/desktop/apple_music_app.c` 负责打开覆盖层，并调用 `apple_music_create_in`

### 3.2 现有 Apple Music 模块

- `main/src/v9_apple_music/` 已有一套旧的 Apple Music UI
- 当前 `apple_music.c`、`am_data.c`、`am_page_list.c`、`am_shell.c`、`am_player.c` 已经带有部分真实播放接线和静态假数据
- 当前界面结构与本次 HTML 样机不一致，需要按新样机重做

### 3.3 现有播放封装

- `main/src/local_music_demo/music_player.c` 是通用播放适配层
- 底层使用 `player_controller` 和 `stream_player`
- 已支持：
  - 本地目录展开
  - 本地文件与流 URL 播放
  - 播放/暂停/切歌
  - 播放模式
  - 音量
  - 位置/时长查询

## 4. 设计决策

### 4.1 入口策略

- 不改 desktop 图标映射关系
- 保持 `Apple Music -> apple_music_app_launch`
- 在该入口下替换 `v9_apple_music` 的界面与数据逻辑

理由：

- 写面集中
- 不需要额外改 desktop 其它应用布局
- 与现有测试入口兼容

### 4.2 替换策略

- 替换 `main/src/v9_apple_music` 的 UI 结构与数据来源
- 不删除 `main/src/local_music_demo`
- 不把 Apple Music 再指回旧 `local_music_demo`

理由：

- 用户要求“接入 desktop 的 Applemusic 图标下，去掉之前的音乐播放器”
- desktop 下 Apple Music 当前已经是独立入口，最小改动是重做该入口背后的模块

### 4.3 数据策略

- 广播页数据从 `sources.csv` 动态加载
- 本地页数据从 `test_file` 目录动态扫描
- 我的收藏先基于本地列表生成静态收藏子集

理由：

- 广播源数量大，不适合继续硬编码到 `am_data.c`
- 本地播放必须使用真实文件路径
- 收藏页先做轻量映射，满足 HTML 结构

### 4.4 播放控制策略

- 统一继续走 `am_player -> music_player -> player_controller/stream_player`
- 点击本地列表项：重建本地播放列表并从目标索引播放
- 点击广播列表项：切换到单条流播放
- 底部控制条反向驱动当前播放源

### 4.5 视觉复刻策略

- 以 HTML 为唯一视觉来源
- 使用 LVGL 原生布局与样式复刻，不做整页贴图
- 保持三视图切换、播放列表弹层、播放态样式变化

## 5. 模块划分

### 5.1 保留并重做的模块

- `main/src/v9_apple_music/apple_music.c`
- `main/src/v9_apple_music/am_shell.c`
- `main/src/v9_apple_music/am_data.c`
- `main/src/v9_apple_music/am_page_list.c`
- `main/src/v9_apple_music/am_player.c`

### 5.2 新增模块

- `main/src/v9_apple_music/am_sources_csv.c/.h`
  - 解析 `sources.csv`
  - 产出广播列表数据
- `main/src/v9_apple_music/am_local_scan.c/.h`
  - 扫描 `test_file`
  - 产出本地列表数据
- `main/src/v9_apple_music/am_macos_ui.c/.h`
  - 聚合 macOS 风格界面构建
  - 降低 `apple_music.c` 复杂度

### 5.3 尽量不动模块

- `main/src/local_music_demo/music_player.c`
- `main/inc/music_player.h`

只有在现有接口不足以支撑新 UI 时，才做小范围扩展。

## 6. 页面结构

### 6.1 总体布局

- 左侧固定 sidebar
- 右侧 main 区
- main 顶部 breadcrumb/header
- main 中部切换 3 个 view
- 底部固定 player bar
- 右下播放列表 popup

对应 HTML：

- `.sidebar`
- `.main`
- `.main-bar`
- `.content`
- `.player`
- `.pl-pop`

### 6.2 正在播放页

- 左：封面
- 右：标题、副标题、歌词占位
- 当播放广播时：
  - 标题切为广播名
  - 副标题显示直播状态
  - 封面切为纯渐变台标样式

### 6.3 广播电台页

- 列表滚动显示 `sources.csv`
- 每项展示：
  - 名称
  - 简化副标题
  - 直播/当前播放标识
- 点击某项后：
  - 切换到该流
  - 当前播放页同步更新

### 6.4 我的收藏页

- 先用本地列表前若干项或固定命中项构造
- 点击后切到当前播放
- 当前播放项高亮

### 6.5 播放列表弹层

- 先展示当前本地播放队列
- 广播模式下可以为空或只显示当前流
- 保持 HTML 的浮层结构与开关行为

## 7. 数据模型

### 7.1 广播项

```c
typedef struct {
    char title[128];
    char url[1024];
    uint32_t duration_ms;
    uint32_t network_cache_ms;
} am_radio_item_t;
```

### 7.2 本地项

```c
typedef struct {
    char path[1024];
    char title[256];
    bool favorite;
} am_local_item_t;
```

### 7.3 运行态

```c
typedef enum {
    AM_VIEW_NOW = 0,
    AM_VIEW_RADIO,
    AM_VIEW_FAVORITES,
} am_view_t;

typedef enum {
    AM_SOURCE_NONE = 0,
    AM_SOURCE_LOCAL,
    AM_SOURCE_RADIO,
} am_source_kind_t;
```

运行态至少需要记录：

- 当前 view
- 当前播放源类型
- 当前本地索引
- 当前广播索引
- 播放列表弹层开关

## 8. 交互规则

### 8.1 sidebar

- 点击 `正在播放`：显示当前播放页
- 点击 `广播电台`：显示广播页
- 点击 `我的收藏`：显示收藏页

### 8.2 本地播放

- 本地列表点击项：
  - 生成整份本地队列
  - 调 `am_player_load_local(urls, count, index)`
- 上一首/下一首按本地队列工作

### 8.3 广播播放

- 广播列表点击项：
  - 调 `am_player_play_stream(url, title)`
- 上一首/下一首：
  - 若当前源为广播，则切前后电台

### 8.4 播放模式

- 本地模式：真实切换 `music_player` 的播放模式
- 广播模式：UI 可显示，但不要求影响流播放行为

### 8.5 进度显示

- 本地文件：
  - 显示当前位置与总时长
  - 更新进度条
- 广播流：
  - 显示 `LIVE`
  - 进度条固定或不推进

## 9. 测试与验证

### 9.1 单元测试

新增或更新测试覆盖：

- `sources.csv` 解析
- 本地目录扫描
- Apple Music launcher 仍能正常打开/关闭
- 父容器尺寸不被破坏

### 9.2 运行验证

- `AM_APP=apple_music` 可直接打开新播放器
- 本地文件可播放
- 广播流可切换
- 播放/暂停/上一首/下一首可用

### 9.3 视觉验证

- 用 HTML 截图作为基线
- 用 `AM_SHOT` 生成 LVGL 截图
- 至少比对：
  - 正在播放页
  - 广播页
  - 收藏页
  - 播放列表弹层展开态

## 10. 风险

- `sources.csv` 量大，直接全量渲染可能带来列表性能压力
- 广播 URL 可用性不稳定，但这不影响 UI 结构实现
- 现有 `am_player.c` 已有一部分旧逻辑，重做时要避免残留状态污染
- `music_player` 对广播上一首/下一首没有现成“台列表语义”，需要 UI 层自己维护索引

## 11. 成功标准

- desktop `Apple Music` 图标打开的是新 macOS 风格播放器
- 旧 Apple Music 页面不再出现
- 本地页使用 `test_file` 真文件播放
- 广播页使用 `sources.csv` 真源播放
- 底部控制条能控制当前播放
- 关键页面视觉结构与 HTML 明显一致
