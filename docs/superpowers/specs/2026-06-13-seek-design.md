# 进度条拖动 Seek 功能设计

**日期**：2026-06-13  
**状态**：待实现

---

## 背景与问题

当前音乐播放器存在两个问题：

1. **进度条与播放时间不对应**：`lv_demo_music_main.c` 使用假计时器（每秒 `time_act++`），与真实音频播放进度完全独立。暂停、换曲后累积误差越来越大。

2. **进度条不可交互**：slider 没有注册拖动回调，用户拖动后没有任何效果。

---

## 架构总览

```
┌─────────────────────────────────────────────────────┐
│  LVGL 主线程                                         │
│                                                     │
│  lv_demo_music_main.c                               │
│  ┌────────────────────────────────────────────────┐ │
│  │  timer_cb (1s) → music_player_get_position_ms()│ │
│  │  track_load   → music_player_get_duration_ms() │ │
│  │  slider RELEASED → music_player_seek()         │ │
│  └────────────────────────────────────────────────┘ │
│                                                     │
│  music_player.c                                     │
│  ┌────────────────────────────────────────────────┐ │
│  │  hls_observer_audio_output_started()  捕获参数  │ │
│  │  get_position_ms()  pcm_bytes_written÷格式      │ │
│  │  get_duration_ms()  探测文件元数据               │ │
│  │  seek()  → player_controller_seek()             │ │
│  └────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────┘
         ↕ player_controller API
┌─────────────────────────────────────────────────────┐
│  player_controller.c（hls_player_demo，可修改）      │
│  player_controller_seek() → stream_player_seek()    │
└─────────────────────────────────────────────────────┘
         ↕ stream_player API
┌─────────────────────────────────────────────────────┐
│  stream_player（hls_player_demo，可修改）             │
│                                                     │
│  stream_player_playback.c                           │
│  ┌────────────────────────────────────────────────┐ │
│  │  ... decode loop ...                            │ │
│  │  [NEW] skip_pcm_bytes check                    │ │
│  │  stream_player_platform_audio_write()          │ │
│  │  stream_player_add_pcm_bytes_written()         │ │
│  └────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────┘
```

---

## 实现分三个层次

### 层次一：真实位置显示（修复 time_act 不对应问题）

**原理**：  
`hls_observer_audio_output_started` 是弱符号，在 `music_player.c` 中覆盖可拿到真实的 `sample_rate / channels / bits_per_sample`。  
`player_controller_get_stats()` 可拿到 `pcm_bytes_written`（单曲从 0 开始累计的 PCM 字节数）。  
因此：

```
position_ms = pcm_bytes_written × 1000 / (sample_rate × channels × bits_per_sample÷8)
```

**不修改 hls_player_demo，零风险。**

**涉及文件**：
- `main/src/music_player.c`：覆盖弱符号，实现 `music_player_get_position_ms()`
- `main/inc/music_player.h`：声明新 API
- `lvgl/demos/music/lv_demo_music_main.c`：`timer_cb` 改为读真实位置

---

### 层次二：曲目总时长（duration）

**原理**：  
对本地文件，从文件头元数据中读取总时长，不依赖 stream_player：

| 格式 | 读取方式 |
|------|---------|
| WAV  | 44 字节固定头：`data_chunk_size / byte_rate × 1000` |
| FLAC | STREAMINFO block：`total_samples / sample_rate × 1000` |
| MP3  | 检查 Xing/VBRI VBR 头；CBR 则 `(file_size−id3_size)×8 / bitrate` |
| AAC/M4A | 暂返回 0（需解 box，复杂度高） |
| HLS 直播 | 返回 0（无总时长） |

**不修改 hls_player_demo，零风险。**

**涉及文件**：
- `main/src/music_player.c`：新增 `audio_probe_duration_ms(url)` 内部函数；新增 `g_current_duration_ms` 全局变量，在 TRACK_CHANGED async handler 里更新
- `main/inc/music_player.h`：声明 `music_player_get_duration_ms()`
- `lvgl/demos/music/lv_demo_music_main.c`：`track_load()` 中用真实 duration 设置 slider range；duration == 0 时 fallback 到原硬编码值

---

### 层次三：Seek（跳转到指定位置）

**原理（PCM skip）**：  
修改 `stream_player_playback.c` 中的音频输出路径，增加 skip 计数器：

```
seek(position_ms) 调用流程：

main thread → player_controller_seek(position_ms)
  → stream_player_seek(player, position_ms)
      → 计算 target_bytes = position_ms × sr × ch × bps/8 / 1000
      → 若 target_bytes > current pcm_bytes：
          forward seek：skip_pcm_bytes += delta（不需要重启）
      → 若 target_bytes ≤ current pcm_bytes：
          backward seek：
            1. 设置 player->skip_pcm_bytes = target_bytes
            2. 调用 stream_player_change_url(player, same_url)
               （内部 stop + reset_session + restart，pcm_bytes_written 归零）

playback thread（stream_player_playback.c 约 line 790）：
  if (player->skip_pcm_bytes > 0):
      skip_now = min(output_bytes, skip_pcm_bytes)
      player->skip_pcm_bytes -= skip_now   // atomic
      output_buffer += skip_now
      output_bytes  -= skip_now
      // 对跳过的字节也计入 pcm_bytes_written（维持位置语义正确）
  stream_player_platform_audio_write(output_buffer, output_bytes)
  stream_player_add_pcm_bytes_written(player, total_bytes)  // 含 skipped
```

**`stream_player_change_url()` 已有实现**：stop + reset + reinit runtime + restart，PCM 字节计数自动归零。  
Seek 延迟：backward seek = 解码跳过 N 秒的延迟（本地文件解码速度远超实时，约 50–200ms）。

**涉及文件（hls_player_demo）**：
- `stream_player/src/stream_player_internal.h`：struct 增加 `skip_pcm_bytes`（uint64_t）、`seek_sample_rate`/`channels`/`bps`
- `stream_player/src/pipeline/stream_player_playback.c`：音频写出路径加 skip 逻辑；audio_output_init 后存储 audio params
- `stream_player/include/stream_player.h`：声明 `stream_player_seek()`
- `stream_player/src/core/stream_player.c`：实现 `stream_player_seek()`
- `player_controller/player_controller.h`：声明 `player_controller_seek()`
- `player_controller/player_controller.c`：实现（含 is_live 判断，直播禁止 seek）

**涉及文件（主项目）**：
- `main/inc/music_player.h`：声明 `music_player_seek(position_ms)`
- `main/src/music_player.c`：实现，is_live 判断
- `lvgl/demos/music/lv_demo_music_main.c`：
  - slider 注册 `LV_EVENT_PRESSED`（冻结位置更新，`g_seeking=true`）
  - slider 注册 `LV_EVENT_RELEASED`（读取 slider value → `music_player_seek()`，`g_seeking=false`）
  - `timer_cb` 中：`if (!g_seeking)` 才更新 slider

---

## API 变更汇总

### 新增 stream_player API

```c
// stream_player.h
stream_error_t stream_player_seek(stream_player_t *player, uint32_t position_ms);
```

### 新增 player_controller API

```c
// player_controller.h
int player_controller_seek(player_controller_t *controller, uint32_t position_ms);
```

### 新增 music_player API

```c
// music_player.h（新增三个函数）
void     music_player_seek(uint32_t position_ms);
uint32_t music_player_get_duration_ms(void);   // 0 表示未知（直播/不支持格式）
uint32_t music_player_get_position_ms(void);   // 当前曲目内的播放位置
```

### 弱符号覆盖（music_player.c 内部）

```c
// 覆盖 hls_player_demo 弱符号，捕获真实 audio 参数
void hls_observer_audio_output_started(uint32_t sample_rate,
                                       uint32_t channels,
                                       uint32_t bits_per_sample);
```

---

## 内部数据结构修改

### stream_player_internal.h

```c
struct stream_player {
    // ... 原有字段 ...

    // [NEW] Seek support
    _Atomic uint64_t skip_pcm_bytes;  // playback thread atomically reads/decrements
    uint32_t seek_sample_rate;        // detected audio sample rate (for seek byte calc)
    uint8_t  seek_channels;           // detected channel count
    uint8_t  seek_bits_per_sample;    // detected bits per sample
};
```

---

## 边界条件与约束

| 情形 | 处理 |
|------|------|
| `is_live == true`（HLS 直播） | `music_player_seek()` no-op；slider 禁止拖动（设为不可点击） |
| `duration_ms == 0`（未知时长） | slider range 保持硬编码原值；position 更新照常 |
| seek 到超出 duration 的位置 | `stream_player_seek()` 返回错误；UI 忽略 |
| 音频 params 未知（seek 在 audio_output_started 之前） | `seek_sample_rate == 0`，使用 config 的 `sample_rate` fallback（可能不准） |
| backward seek（PCM skip + restart） | ALSA 输出有约 200–500ms 无声间隙（重新 buffer），属正常 |
| 用户连续快速拖动 | 每次 RELEASED 只 seek 一次；PRESSED 时冻结 timer 更新避免 UI 抖动 |

---

## 文件影响矩阵

| 文件 | 层次 | 修改类型 |
|------|------|---------|
| `main/inc/music_player.h` | 1+2+3 | 新增 3 个函数声明 |
| `main/src/music_player.c` | 1+2+3 | 覆盖弱符号、新增 probe/seek |
| `lvgl/demos/music/lv_demo_music_main.c` | 1+2+3 | timer_cb、track_load、slider 事件 |
| `hls_player_demo/stream_player_internal.h` | 3 | struct 新增字段 |
| `hls_player_demo/stream_player_playback.c` | 3 | 音频写出路径 skip 逻辑 |
| `hls_player_demo/stream_player.h` | 3 | 新增 seek 声明 |
| `hls_player_demo/stream_player.c` | 3 | 实现 seek |
| `hls_player_demo/player_controller.h` | 3 | 新增 seek 声明 |
| `hls_player_demo/player_controller.c` | 3 | 实现 seek |
