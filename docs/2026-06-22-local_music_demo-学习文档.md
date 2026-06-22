# local_music_demo 学习文档

> 一句话简介：基于 LVGL 官方 `lv_demo_music` 移植/改造的本地音乐播放器 —— UI 沿用官方专辑/频谱/列表三段式布局，底层把"假播放"的固定节拍替换为 `music_player.c` 驱动的真实音频后端（HLS / 本地文件解码）。

> 截图位：
>
> ![首屏-播放页](../assets/screenshots/local_music_main.png)
> ![歌曲列表](../assets/screenshots/local_music_list.png)

---

## 1. 文件分层

| 层 | 文件 | 是否官方原貌 | 职责 |
|---|---|---|---|
| 入口 | `main/src/main_music.c` | 项目新增 | `lv_init` → `hal_init(640,480)` → `music_player_init(argv)` → `lv_demo_music()` → `lv_timer_handler` 主循环 |
| UI 总控 | `local_music_demo/lv_demo_music.c/.h` | 官方为主，少量魔改 | 建根容器、调 list/main 创建函数；`lv_demo_music_get_title/artist/genre/track_length` 在有真音乐时回落到 `music_player_*` |
| 主播放页 | `local_music_demo/lv_demo_music_main.c/.h` | 官方为主，新增 seek / 真位置 | 标题/封面/频谱/播放控制/进度条/介绍动画；新增 `_lv_demo_music_*` 后端回调入口 |
| 列表页 | `local_music_demo/lv_demo_music_list.c/.h` | 官方原貌 | flex 纵向列表，每行 icon+title+artist+time，被选中切到 pause 图标 |
| **音频后端** | `local_music_demo/music_player.c` | **项目新增** | 真实音频后端：扫描目录 / `player_controller` / `stream_player` (HLS + 本地解码) |
| 资源 | `local_music_demo/assets/*.c` + `assets/png/` | 官方原貌 | 封面、按钮、波浪边、频谱采样数据 |

UI 文件来自官方 demo，几乎一字未改；`music_player.c` 是项目新增的**真实音频后端**，靠几个 `_lv_demo_music_*` 反向回调把后端事件灌回 UI。

---

## 2. 启动到首屏 LVGL 调用链

```
main(argc, argv)
 ├─ lv_init()
 ├─ hal_init(640, 480)            // SDL window + mouse/wheel/keyboard
 │   └─ lv_sdl_window_create / lv_sdl_mouse_create / lv_sdl_keyboard_create
 ├─ music_player_init(argv+1, argc-1)   // 创建 player_controller，开 100ms poll timer
 └─ lv_demo_music()
     └─ lv_demo_music_with_args()
         ├─ lv_obj_create(screen)              // root，bg=0x343247
         ├─ lv_demo_music_list_create(root)    // flex column list
         │   └─ for each track: add_list_button() (image+label+label+label, GRID 布局)
         └─ lv_demo_music_main_create(root)    // 播放页（main_cont + 上下 placeholder）
             ├─ create_cont                    // 可纵向滚动的外层 + 两个 snap 占位块
             ├─ create_wave_images             // 顶/底波浪贴片 (lv_image + TILE)
             ├─ create_title_box               // 3 个 lv_label
             ├─ create_icon_box                // 4 个 lv_image，第 1 个是 mute
             ├─ create_ctrl_box                // GRID 布局：rnd/prev/play/next/loop + slider + time
             ├─ create_spectrum_obj            // 自绘对象 + 内部封面 lv_image
             └─ create_handle                  // "ALL TRACKS" 提示条
```

`main_cont` 设置 `lv_obj_set_scroll_snap_y(LV_SCROLL_SNAP_CENTER)`，列表 `list` 在 root 上提前布局，整页用纵向滚动在「列表 ↔ 播放页」之间切换。

---

## 3. 主页面控件清单

| 区块 | LVGL 控件 | 关键 API |
|---|---|---|
| 顶/底波浪、四角圆边 | `lv_image` (`LV_IMAGE_ALIGN_TILE`) + `LV_OBJ_FLAG_IGNORE_LAYOUT` | `lv_image_set_inner_align` |
| 专辑封面 | `lv_image` | 在 spectrum_obj 内 `LV_ALIGN_CENTER`，支持 `lv_image_set_scale` 跟随低频脉动 |
| 频谱 | 自绘 `lv_obj_t` + `LV_EVENT_DRAW_MAIN_BEGIN` 回调里 `lv_draw_triangle` 画 20 根扇形条 | `lv_anim` 驱动 `spectrum_i` 在 `spectrum_X` 数组里推进 |
| 歌名 / 艺术家 / 流派 | `lv_label` ×3，flex column | `lv_label_set_text` |
| 播放/暂停 | `lv_imagebutton`，`CHECKED_RELEASED` 状态切到 pause 图 | `lv_obj_add_state(.., LV_STATE_CHECKED)` |
| 上一首/下一首/loop/rnd/mute | 普通 `lv_image` + `LV_OBJ_FLAG_CLICKABLE` + click 事件回调 | `lv_obj_add_event_cb` |
| 进度条 | `lv_slider`，knob 用图片，indicator 水平渐变 (蓝→紫) | `lv_slider_set_range/value`，`LV_EVENT_VALUE_CHANGED / RELEASED` |
| 时间标签 | `lv_label`，每秒更新 | `lv_label_set_text_fmt("%d:%02d", ...)` |
| 介绍动画 | logo + title 用 `lv_obj_fade_in/out` + 多个 `lv_anim`（bounce 路径） | `INTRO_TIME = 2000ms` |

歌曲列表页：`lv_obj` + `LV_FLEX_FLOW_COLUMN`，每行是一个 `lv_obj`（启用 GRID layout），三列两行装 icon / title+artist / time。

---

## 4. 列表 ↔ 播放页 怎么切换

**不是多 screen，也不是 lv_obj_set_pos+anim**，而是**单页纵向 snap 滚动**：

- `lv_demo_music()` 在同一个 root 下先 `lv_demo_music_list_create(root)` 再 `lv_demo_music_main_create(root)`。
- list 的 y 偏移 = `LV_DEMO_MUSIC_HANDLE_SIZE`，高度占满 root；main 的 `main_cont` 也是 100%。
- `main_cont` 启用 `LV_SCROLL_SNAP_CENTER` + 两个透明 placeholder，使滚动只在「播放页」和「上方列表区」两个 snap 点间停留。
- 底部的 "ALL TRACKS" + 横条就是滚动手柄的视觉暗示。
- `LV_DEMO_MUSIC_AUTO_PLAY` 演示流程里也是直接 `lv_obj_scroll_by(ctrl, 0, -music_height, LV_ANIM_ON)` 来切换，没有 screen load。

列表里点歌：`btn_click_event_cb` → 有真音乐时 `_lv_demo_music_begin_track_switch()` + `music_player_select(idx)`，由后端事件回灌 UI；无音乐时退化为 `lv_demo_music_play(idx)` 直接走"假播放"。

---

## 5. 进度 / 频谱 怎么刷新

### 频谱

- `start_spectrum_anim()` 创建一条 `lv_anim`，把 `spectrum_i` 从当前推到 `spectrum_len-1`，duration = `(剩余采样数 * 1000)/30`，相当于固定 30fps 扫频。
- `spectrum_anim_cb` 每帧 `spectrum_i = v` + `lv_obj_invalidate(spectrum_obj)`，触发 `spectrum_draw_event_cb` 重画。
- 重画里读 `spectrum[spectrum_i][0..3]` 四个 band 幅值，用三角形画 20 根条；低频还驱动 `lv_image_set_scale(album_image_obj, ...)` 让封面跟着脉动。
- 动画跑完 `spectrum_end_cb`：真音乐则循环 `start_spectrum_anim`；假播放则 `lv_demo_music_album_next(true)` 自动下一首。

> 注意：频谱不是 FFT 出来的，仍然是官方预录在 `spectrum_1/2/3.h` 里的数组。真实音频后端没有接 PCM 频谱采样。

### 进度条 / 时间

- `create_ctrl_box` 末尾 `sec_counter_timer = lv_timer_create(timer_cb, 1000, NULL)` + `lv_timer_pause(...)`。
- `lv_demo_music_resume()` 调 `lv_timer_resume`；`lv_demo_music_pause()` 调 `lv_timer_pause`。
- `timer_cb` 每秒：
  - 有真音乐：`position_s = music_player_get_position_ms()/1000`、`duration_s = music_player_get_duration_ms()/1000`，刷 slider 和 label。
  - 无音乐：用 `time_act++` 假计时。
- 用户拖 slider：`slider_event_cb` 在 `VALUE_CHANGED` 时只改 label + 标记 `seeking`，在 `RELEASED` 时调 `music_player_seek(seek_target_ms)`。`timer_cb` 在 seeking 状态下不让真实位置覆盖目标值，直到回放追上 (`real_ms + 1500 >= seek_target`) 才解除。

---

## 6. music_player.c 干啥（不展开内部）

项目新增的**真实音频后端**，对外（被 UI 调用）：

```
music_player_init(urls, count)      // 入口注入 URL 数组（命令行 argv 或 desktop launcher 传入的目录）
music_player_deinit()
music_player_play/pause/resume/next/prev/select(i)/seek(ms)
music_player_get_count/current_index/title/is_live
music_player_is_playing/is_muted/get_volume/get_play_mode
music_player_set_volume/mute_toggle/set_play_mode/cycle_play_mode
music_player_get_position_ms/get_duration_ms/get_track_duration_ms
local_music_demo_launch/close       // 桌面图标入口（builder → desktop_app_launcher）
```

UI ↔ 后端对接：

1. UI 主动调上面的 `music_player_*` 控制播放。
2. 内部用 `player_controller`（封装 `stream_player`，支持 HLS / 本地文件 / 实时流）实际解码，并通过 100ms 的 `lv_timer` `poll_timer_cb` 推进。
3. 后端线程产生事件（`TRACK_CHANGED / STATE_CHANGED / PLAYLIST_END / ERROR`）→ `on_player_event` → `lv_async_call(music_player_async_handler, ...)` 切回 LVGL 线程 → 调 `_lv_demo_music_play / _resume / _pause` 反向驱动 UI。
4. URL 支持目录：`mp_expand_directory` 递归列目录，过滤 `stream_player_is_supported_local_audio_file` 通过的文件，按字典序排序。
5. 桌面入口默认路径 `s_local_music_demo_paths = { "third-party/hls_player_demo/test_file" }`。

UI 端的"回落"逻辑：`lv_demo_music_get_title/artist/genre/track_length` 都先看 `music_player_get_count()`，>0 走真实元数据，否则用官方 demo 内置的 `title_list / artist_list / time_list`。这就是文件分层表里说的"少量魔改"。

---

## 7. 资源组织

- 全部 PNG 经 LVGL converter 转成 `.c` 数组，文件在 `local_music_demo/assets/`，源 PNG 留在 `assets/png/`。
- 命名约定 `img_lv_demo_music_*`，含 `_large` 后缀的是大屏版本（受 `LV_DEMO_MUSIC_LARGE` 宏控制）。
- 封面只有 3 张（`cover_1..3`），按 `track_id % 3` 切；频谱采样数据 `spectrum_1/2/3.h` 与封面一一对应。
- 真实音乐的元数据（标题、时长）来自 `player_controller` 探测，不依赖打包资源。

---

## 8. 分辨率自适应

- 入口固定 `hal_init(640, 480)`，但 UI 全部用 `lv_pct(100)` + GRID 的 `LV_GRID_FR(n)` / `LV_GRID_CONTENT`，所以缩到 480×272 也能展开。
- `lv_demo_music_main_create` 一进来 `music_width = lv_obj_get_content_width(parent)`、`music_height = ...`，所有内部布局/动画位移都基于这两个变量，没有任何硬编码屏幕尺寸。
- `LV_DEMO_MUSIC_LARGE / SQUARE / LANDSCAPE / ROUND` 四个宏切换 GRID 模板、字体大小、频谱半径、按钮间距，覆盖 800×480 / 640×480 / 480×272 / 圆屏几种形态。当前默认全 0。
- 桌面入口走 `desktop_metrics()` 取实际屏幕宽高传给 `desktop_app_launcher_open_with_close`，再透传到 `lv_demo_music_with_args(args.parent = overlay)`，因此从桌面拉起时尺寸跟随桌面 metrics。

---

## 9. 一句话回顾

**UI 完全是 LVGL 官方 `lv_demo_music`**（频谱、封面动画、波浪边框、滚动 snap 切换、列表 GRID 行布局都来自官方）；**真音频是项目新增的 `music_player.c` + `player_controller` + `stream_player`**，通过 `_lv_demo_music_*` 反向回调与 LVGL 线程对接，并接管 `get_title/get_track_length/get_position_ms` 等以替换官方的"假数据"。
