# Apple Music 播放器 UI —— LVGL v9 高保真复刻设计

- 日期:2026-06-14
- 状态:设计已确认,待实现计划
- 参考样机:`third-party/apple_music_player_mockup/`(index.html / styles.css / app.js)
- 关联文档:`docs/superpowers/specs/2026-05-29-apple-music-player-design.md`(旧设计,从未落地,仅作参考)

---

## 1. 背景与目标

`third-party/apple_music_player_mockup/` 是一份 HTML 高保真样机(800×480、Apple Music 风格、玻璃拟态、4 套主题)。本项目是 LVGL v9 + SDL 工程,目标是**新建一个独立模块**,在 LVGL v9 下尽可能"一模一样"地复刻该样机的视觉与交互。

**核心目标**:静态视觉复刻 —— 视觉、布局、文案、交互(切页/换肤/设置 tab)与 HTML 完全一致,**不接真实音频**。

### 现状摸底结论
- `main/src/music_player.c`(394 行)是**纯后端音频控制器**(封装 HLS player:play/pause/seek/时长进度),**无任何 UI 代码**。
- `main.c` 当前在 **640×480** 下启动 LVGL 自带的 `lv_demo_music()`,并把后端事件回调进 demo UI。
- 旧的 `2026-05-29-apple-music-player` 设计**从未产出任何 LVGL 代码**;且 HTML 已演进(4 套主题、mint 默认)。**以当前 HTML 样机为唯一事实来源**。
- 项目**未编译任何 CJK 字体**(`LV_FONT_SOURCE_HAN_SANS_SC_*` 全为 0),中文当前无法渲染。
- 本 LVGL 版本原生渐变**仅支持 `LV_GRAD_DIR_HOR/VER`**(无斜向、无径向);`LV_USE_DRAW_SW_COMPLEX_GRADIENTS=1` 已开,`LV_GRADIENT_MAX_STOPS` 未显式定义(用默认 2)。

---

## 2. 已确认的关键决策

| # | 决策点 | 选定 | 含义 |
|---|--------|------|------|
| 1 | 目标范围 | **纯视觉复刻** | 静态假数据;不接音频;**完全不动 `music_player.c` 后端** |
| 2 | 保真策略 | **原生高保真** | 纯 LVGL v9 原生(渐变/阴影/圆角/半透明叠层);**零图片资源** |
| 3 | 中文字体 | **子集字体** | 仅子集化样机实际用到的字符,按需字号 |
| 4 | 分辨率/入口 | **改 main 启 800×480 新 UI** | `hal_init(800,480)` + `apple_music_create()`;旧调用注释保留 |
| 5 | 渐变还原 | **全原生竖向近似** | 斜向→竖向多段;径向高光→半透明白圆+柔阴影;装饰环→描边圆 |

---

## 3. 架构(方案 A:常驻 shell + 内容区切换)

- **侧栏 + 迷你播放条常驻**(只建一次)。
- **切页**:仅销毁并重建内容区(对应原设计"切页只刷新右侧")。
- **换肤**:用新主题**整壳重建**(对应"换肤重绘整壳";800×480 重建仅数毫秒)。
- **换设置 tab**:仅重建 `settings-main`。

被否方案:B 全量重渲染(每次交互整体重建,SDL 下闪烁、丢状态);C 单文件堆叠(违反"小而专"与隔离原则)。

### 3.1 模块划分 `main/src/v9_apple_music/`

```
apple_music.h/.c       入口 + 控制器:状态机、根 grid、shell 装配、事件、重建编排
am_data.h/.c           静态假数据(镜像 app.js 的 const)
am_theme.h/.c          4 套主题 token + 共享色常量 + 当前主题访问器
am_fonts.h (+生成 .c)  子集字体声明
am_icons.h             图标字形码点宏
am_widgets.h/.c        可复用工厂:玻璃面板/卡片/胶囊/nav_item/封面块/item_action/小标题
am_shell.h/.c          常驻 chrome:侧栏 + 迷你播放条
am_page_home.h/.c      主页
am_page_list.h/.c      列表页模板(广播/本地/歌单共用,数据驱动)
am_page_settings.h/.c  设置页 + 3 个 tab
```

### 3.2 依赖图(单向)

```
am_data ─┐
am_theme ─┼─→ am_widgets ─→ am_shell ─┐
am_fonts ─┤                  am_page_* ┼─→ apple_music(控制器)
am_icons ─┘                            ┘
```

- 约 10 个模块(.c/.h 共约 18 文件 + 生成字体 .c),粒度适合**并行实现**(各页/壳/widgets/theme/data 写面互不重叠)。
- 公共入口:`void apple_music_create(void);`(在 `lv_screen_active()` 上构建整个 UI)。

---

## 4. 数据模型(`am_data`)

全部 `static const`,文案与样机**逐字一致**(子集字体据此提取字符集)。镜像 `app.js`:

- `am_nav_item_t nav_items[5]` —— `{id, label, icon}`:主页⌂ / 广播◉ / 本地♫ / 歌单≣ / 设置⚙
- `am_theme_preset_t theme_presets[4]` —— `{id, label, desc}`:Cyan/Blue/Mint/Orange
- `am_settings_tab_t settings_tabs[3]` —— `{id, title, desc}`:外观/播放/关于
- `home_data`:
  - `hero` `{topline, title, desc, badge}`
  - `recommends[3]` `{title, subtitle}`
  - `recent[3]` `{kind, title, subtitle}`
  - `metrics[3]` `{label, value}`
- `page_data[radio|local|playlist]`:
  - `{eyebrow, title, subtitle, banner_title, banner_desc, badges[3], queue_label, list[4], queue[3]}`
  - `list[]` item:`{kind, title, subtitle, meta, aux}`;`kind` 决定封面渐变色(album=蓝、radio=粉、playlist=青、song=tile)
  - `queue[]` item:`{title, subtitle}`
- `mini_player` `{title, subtitle, current, total, progress_pct=44}`

> 列表页三页共用同一渲染函数 `am_page_list_create(parent, &page_data[i])`。

---

## 5. 主题模型(`am_theme`)

```c
typedef struct {
    const char *id;
    lv_color_t bg_top, bg_bottom;       /* 壳背景竖向渐变 */
    lv_color_t accent, accent_soft;     /* 强调色;rgba(accent,α) 用 bg_color+bg_opa 表达 */
    lv_color_t hero_a, hero_b, hero_c;  /* hero/徽标/播放头像/主按钮 三段 */
    lv_color_t tile_a, tile_b, tile_c;  /* 封面块 */
} am_theme_t;
```

### 5.1 四套主题色值(取自 styles.css,默认 **mint**)

| token | cyan | blue | mint(默认) | orange |
|-------|------|------|------|--------|
| bg-top | #eef8f5 | #eef4ff | #edf8f4 | #fff3e7 |
| bg-bottom | #e3edf1 | #e5ebfb | #e5eff2 | #f2ebea |
| accent | #4bbcae | #4c78ff | #23b497 | #f08d3c |
| accent-soft | #d7f5ef | #dbe5ff | #d4f7ee | #ffe6cf |
| hero-a | #a8ebe0 | #91b3ff | #7ae1cc | #ffc66f |
| hero-b | #59c7ba | #567cff | #28b89c | #f18b47 |
| hero-c | #2b7373 | #2b3b81 | #1a6662 | #8f4a32 |
| tile-a | #daf6f0 | #dce8ff | #d9f8ef | #ffe5cc |
| tile-b | #f1fffc | #eff3ff | #f0fffb | #fff6ef |
| tile-c | #dff4fb | #dff6ff | #ddf4ff | #ffeedc |

### 5.2 共享色常量(不随主题变)

- text `#21242e`、muted `#7a8194`、muted-strong `#5f6678`
- surface `白 62%`、surface-strong `白 86%`、surface-soft `白 44%`、border `白 68%`
- shadow `0 24 64 rgba(39,30,60,0.14)`、shadow-soft `0 12 30 rgba(54,52,70,0.1)`
- progress-rail `rgba(109,117,140,0.16)`

### 5.3 透明度映射

CSS `rgba(accent-rgb, 0.12)` → LVGL `bg_color=accent; bg_opa=round(0.12*255)≈31`。所有 `rgba(白, α)` 同理用 `bg_color=white; bg_opa=α*255`。

### 5.4 应用策略

- **换肤** = 设新主题 → `apple_music` 整壳重建(侧栏+内容+迷你条)。因渐变/强调色多为逐对象设置,整壳重建是最干净的方式。
- 渐变实现:`lv_grad_dsc_t` 竖向(`LV_GRAD_DIR_VER`)多段;需 `LV_GRADIENT_MAX_STOPS≥3`。

---

## 6. 布局映射(800×480,px 1:1)

根用 **LVGL grid** 精确对应 CSS `grid-template`:

- 列:`[164px, LV_GRID_FR(1)]`;行:`[LV_GRID_FR(1), 78px]`;**侧栏跨两行**。

| 区域 | 位置/尺寸 | 内部布局 |
|------|-----------|----------|
| 侧栏 | (0,0) 164×480 | flex 列 gap14,padding 18/14/18/16 |
| 内容区 | (164,0) 636×402 | padding 20/20/0/20,装载当前页 |
| 迷你条 | (164,402) 636×78 | grid `[1.15FR, auto, 1FR]` gap16,padding 12/20/14/14 |

### 6.1 侧栏
品牌区(badge 36×36 r14 + 文案)→"导航"小标题(10px/800 muted)→ 4 个 nav-item → **flex-grow 空白撑开** → footer(顶边线 + "偏好" + 设置 nav-item + listener 卡)
- nav-item:h42 r16,padding 0 12,gap10;icon 28×28 r12(白 60%);label 14px/700;**active**=accent 文字 + `accent@12%` 底 + 内描边
- listener 卡:padding12 r18,白 54%,shadow-soft

### 6.2 主页(`am_page_home`,flex 列 gap16,h100%)
- page-header(flex space-between):左 = eyebrow 胶囊(dot+"广播优先首页")+ page-title "主页"(34px/700)+ subtitle(13px muted);右 = search-chip(⌕ + 占位文案)
- home-grid(grid `[1.25FR, 0.92FR]` gap16):
  - **hero-card**:r28 min-h218 padding22/24/24;hero-a→b→c **竖向三段渐变**;白字;shadow
    - hero-copy:topline(12/700 大写)+ title(24/800)+ desc(12)
    - hero-actions-docked(绝对 左下):button-primary "继续收听" + button-secondary "探索更多"
    - floating-pill(绝对 右上):"今日主打"
    - hero-visual(绝对 右下 144×144):vinyl-ring(描边圆)+ vinyl-core(白圆);径向白高光用半透明白圆+柔阴影近似
  - side-stack(flex 列 gap14):
    - recommend-panel:header("推荐广播"+"查看全部")+ 3× recommend-item(grid `[44, 1FR, auto]`:cover-wave 44×44 r16 渐变 + 标题/副标 + action ▶ 圆 30×30)
    - recent-panel:header("最近播放"+"继续")+ 3× recent-item(同上,action ↺)+ recent-metrics(3 列:广播收藏12 / 连续收听48m / 今日推荐06)

### 6.3 列表页(`am_page_list`,广播/本地/歌单共用)
- page-header(同主页结构,文案来自 page_data)
- feature-banner:grid `[1.2FR, 0.8FR]` padding18 r26;`accent` 竖向渐变;白字
  - 左:h3(banner_title 24px)+ p(banner_desc)+ feature-badges(3 胶囊)
  - 右:feature-visual(2× feature-cardlet w160 padding12 r18:queue_label / "视觉重点")
- content-grid(grid `[1FR, 244px]` gap14):
  - catalog-panel:header(title+" 列表" + "4 项假数据")+ 4× media-item(grid `[44,1FR,auto]`:封面(kind 决定渐变)+ meta + extra(meta span + aux 标签))
  - queue-panel(244):headline "下一步结构观察" + queue-badge(`accent@12%`)+ 3× queue-item(strong+span)

### 6.4 设置页(`am_page_settings`)
- page-header(eyebrow "Appearance Controls" + title "设置" + subtitle + search-chip ◌)
- settings-layout(grid `[180px, 1FR]` gap14):
  - settings-sidebar:3× settings-tab(外观/播放/关于),active 态
  - settings-main(按 tab):
    - **外观**:卡"主题模式"(mode-row:浅色active/深色/跟随系统 胶囊)+ 卡"主题颜色"(theme-swatches 4 列,active=当前主题)+ settings-split(2 卡:圆角强度 slider 62% / 背景氛围 胶囊)+ about-card(当前主题名)
    - **播放**:卡"播放占位"(胶囊)+ 卡"队列策略" + about-card
    - **关于**:卡"关于样机" + 卡"设备假设" + about-card
- swatch:padding10 r18;swatch-preview h42 r14(该主题 hero 竖向渐变预览);slider h8 r999 fill 62%(`accent` 渐变)

### 6.5 迷你播放条(`am_shell` 内)
grid `[1.15FR, auto, 1FR]`:
- now-playing(grid `[52, 1FR]`):player-art 52×52 r18(hero 渐变)+ copy(strong title + span subtitle)
- player-controls(flex gap10):⏮ control-button 34×34 圆 + ▶ primary 42×42(hero 渐变)+ ⏭ 34×34
- progress-cluster:progress-meta(current "01:42" | "Now Playing" | total "03:58")+ progress-bar h6 r999(rail 底 + `accent` 渐变 fill 44%)

> 仅装饰元素(vinyl 环、cover-wave 波纹圆、floating-pill、径向高光圆)用绝对定位 —— 符合"仅装饰可绝对"原则;其余一律 flex/grid。

---

## 7. 字体与图标

### 7.1 子集字体
- **工具**:`lv_font_conv`(`npx lv_font_conv`),合并多源:
  - 拉丁:Plus Jakarta Sans(贴近样机,OFL)
  - 中文:Noto Sans SC(OFL)
  - 符号:覆盖图标字形的符号字体(如 DejaVu Sans / Noto Sans Symbols)
- **子集**:仅提取 `am_data` 中实际出现的字符(中英文 + 符号),`--symbols` 或 `--range` 精确控制。
- **字号**(取自 CSS,9 档):**10/11/12/13/14/16/18/24/34**(按实现可适度合并相近档以减少字体体积)。
- **字重**:先单档(semibold 偏向,保证小字清晰);标题档(24/34)若不够"重"再补 bold 变体。
- 注册:`am_fonts.h` 用 `LV_FONT_DECLARE`;`LV_FONT_CUSTOM_DECLARE` 挂上;`LV_FONT_DEFAULT` 仍可留 Montserrat 作 fallback。

### 7.2 图标字形(`am_icons.h`,参考码点)
⌂ U+2302 / ◉ U+25C9 / ♫ U+266B / ≣ U+2263 / ⚙ U+2699 / ♪ U+266A / ⌕ U+2315 / ◌ U+25CC / ▶ U+25B6 / ⏮ U+23EE / ⏭ U+23ED / ↺ U+21BA

> 字形由数据中的字面字符驱动子集化;若所选符号源缺某字形,回退到最近的 LVGL 内置 symbol(如 `LV_SYMBOL_PLAY/PREV/NEXT`)。

---

## 8. 交互与状态

```c
typedef struct {
    am_page_t      page;          /* home/radio/local/playlist/settings */
    am_theme_id_t  theme;         /* cyan/blue/mint/orange(默认 mint) */
    am_settings_tab_t settings_tab; /* appearance/playback/about */
} am_state_t;
```

**仅 3 类元素可交互**(与样机 `bindEvents` 完全一致):
1. 侧栏导航 → 改 `page` → **重建内容区** + 更新 nav active 态
2. 设置-外观的 4 个主题色卡 → 改 `theme` → **整壳重建**
3. 设置的 3 个 tab → 改 `settings_tab` → **重建 settings-main**

其余元素(hero 按钮、search-chip、item-action ▶/↺、迷你条 ⏮▶⏭、"查看全部/继续"、各 pill)**纯静态装饰,不挂事件** —— 忠于样机。

事件实现:`lv_obj_add_event_cb(obj, cb, LV_EVENT_CLICKED, ...)`,用 `lv_obj_set_user_data` 或事件 user_data 携带目标索引。

---

## 9. 构建与集成

### 9.1 顶层 `CMakeLists.txt`
```cmake
file(GLOB APPLE_MUSIC_SOURCES CONFIGURE_DEPENDS
     "${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/*.c")
file(GLOB APPLE_MUSIC_FONTS CONFIGURE_DEPENDS
     "${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/fonts/*.c")
```
将 `${APPLE_MUSIC_SOURCES} ${APPLE_MUSIC_FONTS}` 追加进 main 可执行目标(非 FreeRTOS 分支,约 131-133 行)。

### 9.2 `lv_conf.h`
- 新增/设置 `#define LV_GRADIENT_MAX_STOPS 4`(支持三段色;现用默认 2)。
- `LV_USE_DRAW_SW_COMPLEX_GRADIENTS` 已为 1,无需改。

### 9.3 `main.c`
- `hal_init(640, 480)` → `hal_init(800, 480)`
- `lv_demo_music();` → `apple_music_create();`
- **注释保留(不删)**:`lv_demo_music`、fruit_ninja 相关调用。
- **注释 `music_player_init/deinit` 调用**:纯视觉无音频,避免后端回调进已不存在的 demo UI。`music_player.c` 整文件不动,随时可恢复。

---

## 10. 边界与验证

### 10.1 边界
- 文案定长;`item-title` 等加 `LV_LABEL_LONG_DOT` 兜底防溢出。
- 窗口固定 800×480,不做响应式。
- 4 主题 × 各页都需目检。
- CJK 在 10–11px 的可读性需实测(必要时该档微调)。

### 10.2 验证
- 构建通过 → SDL 800×480 运行 → **逐页(主页/广播/本地/歌单/设置×3 tab)× 4 主题截图**,与浏览器打开的 `index.html` 对照。
- 可借 `design-to-lvgl` / `verify` 技能做像素级对照。
- 可选纯逻辑单测(沿用 `main/tests` 模式):`am_theme_get(id)` 正确性、`am_data` 计数一致性。

---

## 11. 明确不做(YAGNI)

- 真实音频 / 本地扫描 / 搜索功能。
- 深色模式真正生效(只做"浅色"选中态占位,与样机一致)。
- `device-frame` / `stage-glow` / `device-noise`(浏览器舞台装饰,不在 `.app-shell` 内)。
- 响应式缩放、hover/transition 动画(嵌入式无指针 hover)。

---

## 12. 风险与待实现期验证

1. **符号字形覆盖**:确认所选符号源含 ⌂◉♫≣⚙ 等;缺失则回退 LVGL symbol。
2. **竖向渐变近似的 hero 观感**:首次渲染后目检;若明显失真,按既定退路单独为 hero 加 1 张图(需届时再确认)。
3. **字重单/双档**:标题档若不够重,补 bold 变体。
4. **`LV_GRADIENT_MAX_STOPS` 全局影响**:仅放宽上限,无副作用。
5. **CJK 小字号清晰度**:10–11px 实测。
