# v9_apple_music 完整还原 + 播放稳健化设计

- 日期：2026-07-03
- 状态：设计已确认，待写实现计划
- 基线设计：`docs/superpowers/specs/2026-06-26-macos-music-lvgl-design.md`(已实现，本文档是其优化迭代)
- HTML 唯一视觉来源：`design-ui/music/music-player-macos.html` @ 800×480
- 入口：desktop `Apple Music` 图标 → `apple_music_app_launch` → `apple_music_create_in`
- 播放后端：`third-party/hls_player_demo`(经 `main/inc/music_player.h` 封装)

## 1. 背景与目标

`main/src/v9_apple_music` 已按 06-26 设计落地了整体骨架与一版真实播放接线，但"效果还不行":视觉与 mockup 有明显差距,播放接线偏重且有 bug。本次目标 = **在现有模块上做完整视觉还原 + 播放稳健化**,不重写骨架。

两条主线:
1. **视觉**:以 mockup 为唯一基准,做到 800×480 像素级还原(风格/布局/间距/配色/字号/图标形状)。
2. **播放**:复用 `music_player.h`,把接线改稳健(一次 init + select 切歌、支持拖动 seek、与 lv_demo_music 解耦)。

## 2. 关键决策(已与用户确认)

| 决策 | 选择 | 说明 |
|------|------|------|
| 数据模型 | **真实数据驱动** | 歌曲扫 `test_file`,电台读 `sources.tsv`,收藏=标记本地项。**屏幕文案反映真实文件/流,非 mockup 的"火力全开"等假名**。还原针对的是视觉结构/风格,不是文案内容。 |
| 图标 | **真实矢量图标** | Lucide/Feather 图标字体子集 + 自绘填充版 传输键 + 4 个播放模式图标。替换现有 Unicode 字形近似。 |
| 封面火焰 | **烘焙火焰 PNG** | 188×188 径向火焰 PNG 作歌曲封面;电台封面用按台配色的纯色/线性渐变块。 |
| 分辨率 | **800 主档像素级 + 640/480 自适应** | 800×480 像素级还原;640×480、480×272 走 metrics 自适应(480×272 单列堆叠),不追求逐档像素级。 |
| 歌词 | 用 mockup 的 4 句装饰歌词 | 真实文件无歌词;沿用 mockup 4 行(第 2 行高亮红/粗/15px)保持视觉还原。 |

## 3. 现状缺口(逐条,来自代码审阅)

### 3.1 视觉缺口(对照 mockup)
- **底部播放条布局错**:mockup 的传输键(模式/上一首/播放/下一首/歌单)是 `.ctrls{flex:1;justify-content:center}` 整组居中,音量 `.vol{width:110px}` 右钉,**播放条内没有标题/副标题**。现有 `am_shell.c` 在传输键左侧塞了 220px 的 meta(标题/副标)块 → 控件被挤偏。**需去掉 meta 块,让控件居中**。
- **缺进度圆点**:mockup `.sknob` 是填充末端的 12px 白色圆点(带阴影),现有无。
- **播放模式图标不循环**:mockup 4 态(随机/顺序/单曲/列表)各有图标,现有恒显 `↺`(`AM_ICON_REPLAY`)。
- **图标是 Unicode 字形近似**:`▶◉♥≡↺◔`(`am_icons.h`)与 mockup 的 SVG 图标形状差距明显。
- **封面是扁平两段渐变**:现有 `am_update_now_view` 用 VER 两段渐变,mockup 是径向火焰 + 熔岩斑点。
- **列表行副标题错**:
  - 电台行副标显示 `缓存 Nms`(应为类型/genre),且恒显 `LIVE`(应仅当前播放显示)。
  - 收藏行副标显示文件 `path`(应为歌手/来源),行尾应为 心形 + 时长。
- **歌词是开发占位文本**:`am_data.c` 的 `am_mock_lyrics` 是"把播放器壳层先搭稳…",非 mockup 的诗句。

### 3.2 播放/数据缺口
- **电台列表运行时为空**:`apple_music.c` 加载 `sources.csv`,但真实文件是 `sources.tsv` → `fopen` 失败 → 电台列表空。**改指向 `sources.tsv`**(loader 已能按 tab 分隔自动识别)。
- **切歌过重**:`am_player_play_local_index` 每次都 `music_player_deinit()+music_player_init()`,重新探测所有时长。应**一次 init 全部本地 URL,之后 `music_player_select(i)` 切歌**。
- **无拖动 seek**:进度条不可拖(06-26 明确排除)。本次接 `music_player_seek(ms)`。
- **与 lv_demo_music 潜在耦合**:`music_player` 的异步事件 handler 硬调 `_lv_demo_music_*`。apple_music 的 UI **只靠自身 120ms 轮询定时器驱动**(读 position/duration/is_playing/current_index),不依赖该异步路径,避免串到另一 demo 的 UI。
- **进程级单音频单例**:进入 app 前确保旧实例已 deinit;`apple_music_destroy` 里 deinit。

### 3.3 工程缺口
- **无 headless 启动分支**:`main.c` 的 `AM_APP` 没有 `apple_music` 分支,截图验证跑不起来。**新增 `AM_APP=apple_music` 分支**(照 calc/2048 等写法)。
- **遗留死文件**:`am_page_home.c`、`am_page_settings.c`(不在 CMake 里,未编译),`am_config.c/.h`(radio 结构与 am_data 重复)。**确认无引用后删除**。

## 4. 目标视觉规格(mockup token,逐块)

配色 token(浅色 macOS Apple Music):
- 背景 `#ffffff`;侧栏 `#f0f0f3`;正文 `#1d1d1f`;次要 `#86868b`;强调 `#fa2d48`;
- 分隔 `rgba(0,0,0,.09)`;轨道 `#d6d6db`;传输图标 `#2a2a2e`;交通灯 `#ff5f57/#febc2e/#28c840`。
- 字号档:标题 23、侧栏/分区 14、歌手/列表 13、时间/caption 11;列表标题 24;歌词高亮 15。

分块:
1. **侧栏(196px)**:交通灯(3×12px 圆点,gap8,高 44) → "音乐"(18px 700) → nav(3 项,图标 17px 强调色 + 文案 14,选中 `rgba(0,0,0,.08)` 圆角7 加粗) → "最近播放"(11px 次要,letter-spacing .5) → recent(13px 次要,省略号)。
2. **header(46px)**:crumb(14px 600),底部 1px 分隔。
3. **正在播放**:padding 18/26,gap26,行布局居中。封面 188×188 圆角14 火焰 + 阴影(0 12 30 rgba .22);右侧标题(23/700)+ 歌手(13 次要,margin 5/14)+ 歌词 4 行(13 次要,第2行强调15/700)。暂停 → 封面 scale .9 + 阴影变小。电台 → 纯色封面 + 副标"● 正在直播 · 类型"。
4. **电台列表**:list-title 24/700;行 padding8 圆角8 gap12,42×42 圆角8 渐变块 + 声波图标,名(13)+ 类型(11 次要),当前行背景 hover + 名强调加粗 + 行尾 `正在播放`(11 强调 700)。
5. **收藏列表**:行同上,42×42 火焰块,名(13)+ 歌手(11 次要),行尾 心形(15 强调)+ 时长(11 次要)。
6. **播放条**:padding 10/24/14。scrub 行:时间(11 次要,min 32)+ 轨道(4px 圆角2 `#d6d6db`)含填充(强调)+ 白点 knob(12px)+ 右时间。pbar 行:`.ctrls` flex:1 居中 gap26(模式 20 强调 / 上一首 22 / 播放 30 填充圆 / 下一首 22 / 歌单 21)+ `.vol` 110px 右钉(喇叭 17 次要 + 70px 轨道 + 填充 65% 次要色)。
7. **播放列表弹层**:300px 宽,右下(right18 bottom96),圆角12 阴影(0 16 44 .22),标题"播放列表 N 首"(13/700 + 计数 11 次要),列表行(序号18 + 名13/副11 + 时长11),当前行 hover + 序号/名强调。遮罩点击关闭。

## 5. 图标资产方案

- **来源**:Lucide(或 Feather)开源 SVG/字体。抽取所需字形码点,用 `lv_font_conv` 生成子集图标字体(每个用到的字号一份,或用一个基准字号 + `lv_image_set_scale`)。
- **需要的图标**:nav(play-circle、radio-waves、heart-fill);传输(prev-fill、play-fill、pause-fill、next-fill);模式(shuffle、repeat/order、repeat-one、list-loop);歌单(list-music);音量(speaker)。
- **填充传输键**:mockup 的 play/pause/prev/next 是自定义填充多边形。优先用图标字体里的 fill 版;缺失的用自绘(LVGL canvas/多边形)或烘焙小 PNG。
- 落地到 `am_icons.h` 的宏(UTF-8 字面量)与 `am_fonts.h` 的字体角色。生成后 grep 字体 .c 的 cmap 确认码点已嵌入(避免豆腐块)。

## 6. 播放接线(稳健化)

`am_player.c` 改造(复用 `music_player.h`,不改 wrapper):
- **本地**:进入 app / 首次播放时 `music_player_init(all_local_urls, n)` 一次;列表点击 → `music_player_select(i)`;上一首/下一首 → `music_player_prev/next()`;当前索引以 `music_player_get_current_index()` 为准。
- **电台**:切电台时 deinit 后 `music_player_init(&url, 1)`(流单条);台间上一首/下一首由 UI 维护台索引后 re-init。
- **进度**:120ms `lv_timer` 轮询 `get_position_ms/get_duration_ms/is_playing/get_current_index` 刷 UI;电台(duration 0)显示 LIVE、进度不推进。
- **seek**:进度轨道加可点击/拖动手势,`RELEASED` → 按点击位置比例算 ms → `music_player_seek(ms)`(电台不 seek)。
- **音量**:音量轨道可点/拖 → `music_player_set_volume(0..100)`;喇叭图标点击 → `music_player_mute_toggle()`。
- **模式**:模式键 → `music_player_cycle_play_mode()`,图标随 `music_player_get_play_mode()`(4 态)切换。
- **解耦**:不依赖 wrapper 的 `_lv_demo_music_*` 异步回调路径。

## 7. 验证回路(skill 核心)

1. **建 Linux build**:`cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && make -C build main -j`。
2. **headless 启动**:`main.c` 加 `AM_APP=apple_music` → `apple_music_create()`;`AM_SHOT=/path.ppm` 出图(现有 `maybe_take_snapshot` 已就绪)。批量态用 env(`AM_PAGE=now/radio/fav`、`AM_PANEL=playlist`、`AM_PAUSED=1`)驱动初始态。
3. **mockup 基线**:headless Chrome 渲 `render.html?view=...` 为 800×480 PNG(去 device-frame 装饰)。
4. **比对**:每做完一块,主线程 `Read` 两图并排比对,差异立即修。
5. **SDL 回退**:若本机 SDL 仍无法出图(见 memory `build-and-headless-verify`),用"软件显示 + lv_snapshot"独立 harness 出图。

## 8. 模块与写面

改动集中在 `main/src/v9_apple_music/`:
- `apple_music.c`:正在播放页(火焰封面/歌词/暂停态)、sources.tsv 路径修复、init-once 接线。
- `am_shell.c`:侧栏图标、播放条布局(去 meta、控件居中、knob、音量交互)、弹层。
- `am_player.c`:稳健化接线(select 切歌、seek、模式图标、音量、mute)。
- `am_page_list.c`:电台/收藏行副标题与行尾修正、渐变块 + 图标。
- `am_data.c`:歌词换 mockup 诗句;recent 用真实曲目(或保留)。
- `am_icons.h` / `am_fonts.h` / `fonts/`:真实矢量图标字体 + 火焰封面 PNG(`assets/` 或 `fonts/`)。
- `am_metrics.c`:补齐 640/480 结构尺寸自适应(容器/坐标走 metrics)。
- `main/src/main.c`:`AM_APP=apple_music` 分支。
- 删:`am_page_home.c`、`am_page_settings.c`、`am_config.c/.h`(确认无引用后)。

不动:`music_player.c`、`music_player.h`、`local_music_demo` 其余、desktop 入口映射。

## 9. 成功标准

- desktop `Apple Music` 打开的界面在 800×480 与 mockup **视觉结构/风格/图标/封面/播放条布局一致**(文案为真实数据)。
- 本地文件真播放:播放/暂停/上一首/下一首/选曲/进度/拖动 seek/音量/静音/4 态模式 全可用。
- 电台列表非空(sources.tsv),可切流播放,显示 LIVE。
- 640×480、480×272 布局不错位、可用(480 单列)。
- 每个关键页(正在播放/电台/收藏/弹层/暂停态)都有截图与 mockup 比对记录。
- 无遗留死文件;`music_player` 未被侵入式修改。

## 10. 风险

- 本机 SDL 可能无法出图 → 用 lv_snapshot 独立 harness 回退(memory 有记录)。
- 图标字体子集若源字体缺某字形会静默豆腐块 → 生成后校验 cmap。
- 电台 URL 可用性不稳定,不影响 UI 结构;截图验证以结构为准。
- `music_player` 异步 handler 调 `_lv_demo_music_*`:需确认 apple_music 场景下不触发或安全(实现时验证,必要时只轮询驱动)。
- 火焰 PNG + 图标字体增加 flash/RAM,注意 `LV_MEM_SIZE` 足够放快照缓冲。
