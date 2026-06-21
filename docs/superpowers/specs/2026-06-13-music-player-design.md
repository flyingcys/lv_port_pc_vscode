# lv_demo_music 真实播放器集成设计

**日期**：2026-06-13  
**状态**：已确认，待实现

---

## 背景

`lv_demo_music` 是 LVGL 自带的音乐播放器 UI demo，具备完整的视觉效果（频谱动画、专辑封面、播放控件），但没有真实音频播放能力——时间计数是假的，频谱动画由预录数据驱动，按钮只触发 UI 动画。

`third-party/hls_player_demo/src` 已实现一个完整的纯 C 音频播放引擎，支持 HLS/HTTP 流、本地文件（MP3/FLAC/WAV/AAC/Opus 等），对外通过 `player_controller_t` 提供播放列表管理 API。

本次目标：将两者集成，实现一个 UI 完整、能真正播放音频的播放器。

---

## 设计决策

| 决策 | 选择 | 理由 |
|------|------|------|
| 音频来源 | 任意 URL（本地文件 + HTTP/HLS 流混合） | 与 CLI demo 一致，最灵活 |
| 频谱动画 | 保留预录数据，通过 PLAYBACK_COMPLETE 事件同步换曲 | 保留视觉效果，避免复杂 FFT 集成 |
| 播放列表配置 | 命令行参数 / 配置读取，启动时载入 | 灵活，无需重新编译换歌单 |
| 跨线程 UI 更新 | `lv_async_call()` | LVGL 官方机制，零侵入，内部有锁保护 |

---

## 模块结构

```
main/src/
  music_player.h    ← 对外接口（供 lv_demo_music_main.c 调用）
  music_player.c    ← 胶水层：持有 player_controller_t，管理 LVGL timer，处理事件

lvgl/demos/music/
  lv_demo_music_main.c  ← 修改：5 处按钮/手势回调 + spectrum_end_cb
  lv_demo_music.c       ← 可选修改：轨道数量与实际 URL 数量对齐

third-party/hls_player_demo/src/
  player_controller.*   ← 初版保持不变（API 已够用）
  stream_player/        ← 纯音频引擎，不修改
  http_client/          ← HTTP/TLS 客户端，不修改
```

---

## music_player.h 接口

```c
void music_player_init(const char **urls, size_t count);
void music_player_deinit(void);

void music_player_play(void);
void music_player_pause(void);
void music_player_resume(void);
void music_player_next(void);
void music_player_prev(void);
void music_player_select(size_t index);

size_t music_player_get_count(void);
size_t music_player_get_current_index(void);
bool   music_player_is_playing(void);
```

`music_player_init(NULL, 0)` 为降级模式：跳过 controller 创建，所有接口变为 no-op，lv_demo_music 照常跑假动画。

---

## music_player.c 内部设计

### 生命周期

```
music_player_init()
  → player_controller_create(on_player_event, NULL)
  → player_controller_set_stream_config()
  → player_controller_load_urls()
  → lv_timer_create(poll_timer_cb, 100, NULL)
  → player_controller_play()
```

### LVGL Timer 轮询（100ms）

```c
static void poll_timer_cb(lv_timer_t *t) {
    player_controller_poll(g_controller);
}
```

### 跨线程事件 → lv_async_call

player 内部线程触发回调 → 分配 `music_player_async_arg_t`（含事件类型和 track_index）→ `lv_async_call()` → LVGL 主线程执行：

| 事件 | LVGL 主线程动作 |
|------|----------------|
| `TRACK_CHANGED` | `_lv_demo_music_play(new_index)`（重置频谱 + 更新 UI 元数据） |
| `STATE_CHANGED → PAUSED` | `_lv_demo_music_pause()` |
| `STATE_CHANGED → PLAYING` | `_lv_demo_music_resume()`（仅 UI 未处于 playing 时） |
| `PLAYLIST_END` | `_lv_demo_music_pause()` |
| `ERROR` | `_lv_demo_music_pause()` |

```c
typedef struct {
    player_controller_event_t event;
    size_t track_index;
} music_player_async_arg_t;
// malloc 在 player 回调里，async handler 里 free
```

---

## lv_demo_music_main.c 改动

### 5 处回调替换

| 原函数 | 改动 |
|--------|------|
| `play_event_click_cb` | checked → `music_player_resume()`；unchecked → `music_player_pause()` |
| `next_click_event_cb` | `music_player_next()` |
| `prev_click_event_cb` | `music_player_prev()` |
| `album_gesture_event_cb` | LEFT → `music_player_next()`；RIGHT → `music_player_prev()` |
| `spectrum_end_cb` | 不再换曲，改为循环重启频谱动画 |

### spectrum_end_cb 新逻辑

```c
static void spectrum_end_cb(lv_anim_t *a) {
    if (!playing) return;
    lv_anim_t new_a;
    lv_anim_init(&new_a);
    lv_anim_set_values(&new_a, 0, spectrum_len - 1);
    lv_anim_set_exec_cb(&new_a, spectrum_anim_cb);
    lv_anim_set_var(&new_a, spectrum_obj);
    lv_anim_set_duration(&new_a, (spectrum_len * 1000) / 30);
    lv_anim_set_completed_cb(&new_a, spectrum_end_cb);
    lv_anim_start(&new_a);
}
```

真实换曲由 `TRACK_CHANGED` 事件通过 `lv_async_call` → `_lv_demo_music_play(new_id)` 驱动。

---

## main.c 启动流程

```c
int main(int argc, char **argv) {
    lv_init();
    hal_init(640, 480);

    // argv[1..] 全部作为 URL/路径，目录自动展开
    music_player_init((const char **)(argv + 1), argc - 1);

    lv_demo_music();

    while (1) {
        lv_timer_handler();
        usleep(5000);
    }

    music_player_deinit();
    return 0;
}
```

目录展开逻辑复用 CLI demo 的扫描代码，提取为 `music_player.c` 内部函数。

### 示例用法

```bash
./MusicPlayer /home/user/music/           # 本地目录，自动扫描
./MusicPlayer /home/user/a.mp3 b.flac    # 本地文件列表
./MusicPlayer http://example.com/live.m3u8  # HLS 流
./MusicPlayer a.mp3 http://example.com/b.m3u8  # 混合
```

---

## 构建系统（CMakeLists.txt 根文件）

### 新增依赖

```cmake
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(ALSA REQUIRED)

set(HLS_PROJECT_ROOT_DIR ${PROJECT_SOURCE_DIR}/third-party/hls_player_demo)

add_subdirectory(${HLS_PROJECT_ROOT_DIR}/src/http_client)
add_subdirectory(${HLS_PROJECT_ROOT_DIR}/src/player_controller)
add_subdirectory(${HLS_PROJECT_ROOT_DIR}/src/stream_player)
```

### 主可执行文件修改

```cmake
add_executable(main
    ${PROJECT_SOURCE_DIR}/main/src/main.c
    ${PROJECT_SOURCE_DIR}/main/src/music_player.c    # 新增
    ${PROJECT_SOURCE_DIR}/main/src/mouse_cursor_icon.c
)

target_include_directories(main PRIVATE
    ${HLS_PROJECT_ROOT_DIR}/src
    ${HLS_PROJECT_ROOT_DIR}/src/stream_player/include
    ${HLS_PROJECT_ROOT_DIR}/src/player_controller
    ${HLS_PROJECT_ROOT_DIR}/src/http_client
    ${ALSA_INCLUDE_DIRS}
)

target_compile_definitions(main PRIVATE
    PLATFORM_LINUX OPUS_BUILD VAR_ARRAYS
)

target_link_libraries(main
    ... （原有）...
    player_controller
    stream_player_module_linux
    ${ALSA_LIBRARIES}
)
```

---

## 约束与已知限制

- **时间进度条**：仍使用假计时器（每秒 +1），不反映真实播放进度。后续可通过 `player_controller_get_stats()` 计算 PCM 字节数对应时间来改进。
- **轨道元数据**：标题/艺术家/流派继续使用 `lv_demo_music.c` 里的硬编码数组，按 track index 对应。URL 数量超过数组长度时截断；不足时显示 "Unknown"。
- **频谱动画**：预录数据与实际音频内容无关，仅作视觉效果。
- **hls_player_demo 代码**：初版保持不动，后续可按需扩展（如暴露播放位置查询接口）。
