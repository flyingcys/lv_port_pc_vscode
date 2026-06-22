# v9_apple_music 学习文档

LVGL 实现的 Apple Music 风格音乐 UI：侧栏导航 + 内容区 + 底部迷你播放条，四套主题色，三档分辨率自适应。

![](占位-home页截图)
![](占位-list页截图)
![](占位-settings页截图)

## 1. 文件分层

| 文件 | 干啥 |
|---|---|
| `apple_music.c/h` | shell 装配 |
| `am_shell.c/h` | 侧栏 + 迷你播放条 |
| `am_page_home.c/h` | 主页 |
| `am_page_list.c/h` | 广播/本地/歌单 三页共用 |
| `am_page_settings.c/h` | 设置页 |
| `am_widgets.c/h` | 通用组件 |
| `am_theme.c/h` | 四套主题色 |
| `am_metrics.c/h` | 分辨率档位 |
| `am_fonts.h` | 字体声明 |
| `am_icons.h` | 图标字形 |
| `am_data.c/h` | 静态文案 |
| `am_player.c/h` | 播放器绑定 |
| `am_config.c` | 内部配置 |
| `am_screenshot.c` | 截图工具 |
| `fonts/*.c` | 子集字体 |
| `desktop/apple_music_app.c` | 桌面入口 |

## 2. 启动流程

```
desktop 桌面点击图标
  └─> apple_music_app_launch()                    desktop/apple_music_app.c
        └─> desktop_app_launcher_open_with_close(builder, …)
              └─> apple_music_builder(overlay, w, h)
                    ├─ lv_obj_create(overlay)         // app_root，无样式，铺满
                    ├─ am_metrics_init(w, h)          // 选择 tier（800/640/480）
                    ├─ apple_music_create_in(app_root)
                    │     ├─ am_player_init()         // 100 ms lv_timer 驱动播放刷新
                    │     └─ build_all(parent)        // 装配 shell（见 §3）
                    └─ create_back_button(overlay)    // 左上角 44×44 圆形返回
```

## 3. shell 布局（apple_music.c · build_all）

根容器 `s_root` 用 **lv_obj grid** 拆三块：

```
s_root (grid: cols[sidebar_w, FR1] · rows[FR1, player_h])
├── s_sidebar    cell(0,0,1,2)    跨两行
├── s_content    cell(1,0,1,1)    flex-col，竖向滚动
└── s_player     cell(1,1,1,1)    底部迷你播放条
```

- 背景：`am_fill_grad2(s_root, t->bg_top, t->bg_bottom)` 竖向 2 段渐变。
- 切页（on_nav）：仅 `lv_obj_clean(s_content)` 后重建；切主题（on_theme_pick）：全量重建 shell。
- grid 描述符 `col_dsc / row_dsc` 必须是**文件级静态数组**（LVGL 仅存指针不拷贝）。

## 4. 各页屏幕构成

### 4.1 侧栏 `am_shell_build_sidebar`

```
sidebar  flex-col，padding/gap 由 metrics 给
├─ brand 行       lv_obj 行 flex-row
│   ├─ badge      lv_obj 32×32，r12，3 段渐变 + AM_ICON_BRAND label
│   └─ copy       两行 label（粗 / muted）
├─ am_section_title("导航")
├─ navlist        4× am_nav_item（icon + label，active 时 accent 高亮）
├─ spacer        flex-grow 1
└─ footer        am_section_title("偏好") + am_nav_item(设置) + listener-card
```

`am_nav_item`：`lv_obj` r16 容器 + 28×28 圆角图标 + 文本，整体 CLICKABLE，事件挂 `nav_click_cb`，回调通过 `lv_obj_get_user_data` 取页索引。

### 4.2 主页 `am_page_home_create`

```
page  flex-col
├─ page-header   左：胶囊 eyebrow + lv_label 大标题(f_title) + subtitle；右：search-chip
└─ 800/640: grid [FR125, FR92]            480: flex-col 单列堆叠
    ├─ col0: hero-card                    渐变大卡 + vinyl 装饰
    │   ├─ am_fill_grad3(hero_a/b/c)
    │   ├─ topline / h2 title / desc      lv_label
    │   ├─ actions    LV_ALIGN_BOTTOM_LEFT 绝对定位，2 个 r999 按钮
    │   ├─ floating-pill  LV_ALIGN_TOP_RIGHT 角标
    │   └─ visual 144×144  ring(r圆+16px白边) + core(白圆)
    └─ col1: side-stack
        ├─ recommend-panel  am_panel + 3× build_media_row
        └─ recent-panel     am_panel + 3× media-row + grid 3 列 metrics 卡
```

`build_media_row`：`am_card` 行 + `am_cover`（size×size，r16，2 段渐变 + 2 个白色装饰圆）+ flex-col 标题副标 + `am_item_action`（圆形按钮）。

### 4.3 列表页 `am_page_list_create`（radio / local / playlist 共用）

```
page  flex-col gap=page_gap
├─ page-header                         同 home，右侧 search-chip
├─ feature-banner                      accent 渐变大卡
│    └─ 800/640: grid [FR120, FR80]    480: flex-col
│         ├─ 左：banner_title + desc + 3× feature-badge（白@18% 胶囊）
│         └─ 右：2× feature-cardlet（白@16% 圆角卡）
└─ content-grid
     ├─ catalog-panel(col0)            am_panel
     │    ├─ panel-header  (标题 + "查看全部")
     │    └─ 4× build_media_item       封面 + 标题/副标 + meta + aux-badge
     │           └─ data->play_mode 决定是否挂 media_item_click_cb
     │                LOCAL → am_player_load_local()
     │                STREAM → am_player_play_stream()
     └─ queue-panel(col1)              am_panel + queue-badge + 3× queue-item
```

封面色由 `item->kind` (album/radio/playlist/song) 决定，硬编码渐变对。

### 4.4 设置页 `am_page_settings_create`

```
page
├─ page-header
└─ 800/640: grid [settings_nav_w, FR1]    480: tab-row + 全宽 main
    ├─ sidebar (am_panel)                3 个 tab 项（tab_click_cb）
    └─ main (am_panel)
         switch(active_tab):
           0 外观 → swatches grid 4 列（or 2 列）+ toggle-row pills + about-card
           1 播放 → toggle-row pills + about-card
           2 关于 → settings-card + about-card
```

theme-swatch：`am_card` r18 + 42px 预览块（**该 swatch 自己主题的 hero_a/b/c 三段渐变**）+ label + desc，active 时加 2px accent 边框。

### 4.5 底部迷你播放条 `am_shell_build_miniplayer`

```
player  grid cols[FR23, CONTENT, FR20]  rows[FR1]
├─ col0 now-playing       flex-row
│   ├─ art (lv_obj r=size/3 渐变3) + 装饰白圆
│   └─ copy: title(LONG_DOT) + subtitle(LONG_DOT)   ← h.title_label/subtitle_label
├─ col1 controls          3 个圆形 ctrl_btn
│   ├─ prev (white@72%)
│   ├─ play (hero 3段渐变，AM_WHITE 图标)            ← h.play_icon
│   └─ next
└─ col2 progress-cluster  flex-col
    ├─ meta row: time_cur(LIVE/--:--/m:ss) + "Now Playing" + time_total
    └─ rail (h=6, r=999, 灰@16%) + fill (LV_PCT(pct), accent 2段渐变)
```

`am_player_bind_miniplayer` 把上面 5 个 label/对象指针存进 `g_h`，100 ms 定时器 `am_player_timer_cb` 检测曲目/播放态变化并刷新。

> ⚠️ 注意 `LV_GRID_FR(100) == LV_GRID_TEMPLATE_LAST`，会被当成数组终止符——播放条 grid 用 `FR(23):FR(20)` 而非 100:100。

## 5. 关键控件实现要点

**a. 玻璃面板 `am_panel`** — `lv_obj_create` + r22 + bg=AM_WHITE/opa=158(62%) + 1px 白边(opa=173) + 阴影(w=30, offset_y=12, color=0x363446, opa=26)。所有内容卡都基于它。

**b. 3 段渐变 `am_fill_grad3`** — `lv_grad_dsc_t` 是按指针被样式引用的，**必须持久存活**。这里 `lv_malloc` 一份 + 挂 `LV_EVENT_DELETE` 回调 `am_free_grad_cb` 自动释放。2 段渐变 `am_fill_grad2` 是 by-value 设置 `bg_color`/`bg_grad_color`，无生命周期问题。

**c. 列表行单行省略** — 中文不会因为换行而 `LV_LABEL_LONG_DOT` 触发，因此要**显式钉住一行高度**：`lv_obj_set_size(label, LV_PCT(100), lv_font_get_line_height(font))`，再 `lv_label_set_long_mode(label, LV_LABEL_LONG_DOT)`。home/list 的标题副标都用这招。

**d. 设置项胶囊 `am_pill`** — `lv_obj` + `lv_obj_set_size(LV_SIZE_CONTENT, LV_SIZE_CONTENT)` + `LV_RADIUS_CIRCLE`，否则会被画成大圆。active 用 `t->accent + opa=31(12%)` 软高亮。设置页"柔光/冷雾"这类 toggle 就是 active/!active 两个 pill 排列。

**e. 进度条** — 不用 `lv_slider`，自己做：rail 是 r999 的 `lv_obj` + 内嵌 fill (`lv_obj_set_width(fill, LV_PCT(pct))`)，省去 slider 的拖柄样式，配色用 `am_fill_grad2(accent, accent_soft)`。

**f. 回调上下文释放** — 任何 `lv_malloc` 出来给事件 user_data 的结构（`nav_ctx_t`、`am_cb_ctx_t`、`media_click_ctx_t`、grid 描述符），都挂一个 `LV_EVENT_DELETE` 回调 `lv_free` 它，避免内存泄漏。

## 6. 主题 / 字体 / 图标

- **主题**：4 套预设（cyan / blue / mint / orange），结构 `am_theme_t { bg_top/bottom, accent/accent_soft, hero_a/b/c, tile_a/b/c }`。`am_theme_set()` 切换后由 `apple_music.c::on_theme_pick` 触发 `build_all` 全量重建。共享色 `AM_TEXT/AM_MUTED/AM_WHITE/AM_OPA_*` 由 `am_theme.h` 宏定义。
- **字体**：8 个 800/640 档（11/12/13/14/16/18/24/34）+ 5 个 480 档（10/12/15/18/22），子集编入 `fonts/`，码点见 `fonts/charset.txt`。`am_metrics_t` 里 `f_title/f_h2/f_metric/f_strong/f_body/f_label/f_icon` 7 个角色按 tier 选择。
- **图标**：纯 Unicode 字形（U+2302 主页、U+25C9 广播、U+266B 本地、U+2263 歌单、U+2699 设置、U+25B6/⏮/⏭ 播控…），用 `lv_label` 显示，**字符已编入字体子集**——不引入额外图标库。

## 7. 三档分辨率自适应

`am_metrics_init(w,h)` 在 builder 里按 `w>=800 / >=640 / 其他` 选定 tier，全局 `cur` 指针被切换，所有页面之后 `am_metrics()` 拿到匹配档：

| 维度 | 800×480 | 640×480 | 480×272 |
|---|---|---|---|
| sidebar_w | 164 | 140 | 108 |
| player_h | 78 | 78 | 56 |
| content_pad / panel_pad | 20/18 | 18/16 | 12/12 |
| cover / card_radius | 44/18 | 40/16 | 34/14 |
| queue_w | 244 | 200 | 0 |
| stack_content | false | false | **true** |
| 字体集 | am_font_* | am_font_* | am_font_480_* |

`stack_content=true` 是 480 档的开关：home/list/settings 三页都会 `if(m->stack_content)` 走单列堆叠分支（取代双列 grid），把 queue/侧栏/swatches 等次要内容降级为全宽顺排，迷你播放条的 art/btn 尺寸也由 `player_h - 24/-36` 派生跟着缩。

DONE /Users/cys/embedded/lv_port_pc_vscode/docs/2026-06-22-v9_apple_music-学习文档.md
