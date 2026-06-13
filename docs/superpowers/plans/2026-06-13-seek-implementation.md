# Seek 功能实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 实现进度条拖动 seek 功能，同时修复播放时间与进度条不对应的问题。

**Architecture:** 三层：(1) 真实位置/时长显示（零侵入，覆盖弱符号）；(2) hls_player_demo 内部增加 PCM skip seek 机制；(3) 全链路 seek 接口 + UI 拖动交互。

**Tech Stack:** LVGL v9 / hls_player_demo (C99) / ALSA / POSIX

**规范文档：** `docs/superpowers/specs/2026-06-13-seek-design.md`

---

### Task 1：真实位置与时长——music_player + lv_demo_music_main（层次 1+2）

**Files:**
- Modify: `main/inc/music_player.h`
- Modify: `main/src/music_player.c`
- Modify: `lvgl/demos/music/lv_demo_music_main.c`

---

#### Step 1.1：music_player.h 新增三个函数声明

在 `music_player.h` 的现有声明之后追加：

```c
void     music_player_seek(uint32_t position_ms);
uint32_t music_player_get_duration_ms(void);
uint32_t music_player_get_position_ms(void);
```

---

#### Step 1.2：music_player.c —— 覆盖 hls_observer 弱符号，捕获真实音频参数

在 `music_player.c` 的现有 `#include` 区块之后、全局变量之前，添加（`observer_hooks.h` 路径已在 include 目录中）：

```c
#include "stream_player/src/core/observer_hooks.h"
```

添加全局变量（与其他 `static` 全局变量放在一起）：

```c
static uint32_t g_audio_sample_rate   = 0;
static uint8_t  g_audio_channels      = 0;
static uint8_t  g_audio_bps           = 16;
static uint32_t g_current_duration_ms = 0;
```

覆盖弱符号（放在文件末尾）：

```c
void hls_observer_audio_output_started(uint32_t sample_rate,
                                       uint32_t channels,
                                       uint32_t bits_per_sample) {
    g_audio_sample_rate = sample_rate;
    g_audio_channels    = (uint8_t)channels;
    g_audio_bps         = (uint8_t)bits_per_sample;
}
```

---

#### Step 1.3：music_player.c —— 实现 music_player_get_position_ms()

```c
uint32_t music_player_get_position_ms(void) {
    stream_player_stats_t stats;
    uint32_t denom;

    if(!g_controller) return 0;
    if(g_audio_sample_rate == 0 || g_audio_channels == 0) return 0;
    if(player_controller_get_stats(g_controller, &stats) != 0) return 0;

    denom = g_audio_sample_rate * g_audio_channels * (g_audio_bps / 8U);
    if(denom == 0) return 0;

    return (uint32_t)(stats.pcm_bytes_written * 1000ULL / denom);
}
```

---

#### Step 1.4：music_player.c —— 实现 audio_probe_duration_ms() + music_player_get_duration_ms()

在 `#include <sys/stat.h>` 之后确认已有，如无则添加 `#include <stdio.h>`。

**文件时长探测函数（放在文件私有部分，不暴露）：**

```c
static uint32_t audio_probe_duration_ms(const char *url) {
    FILE   *f;
    uint8_t buf[48];
    size_t  n;
    long    file_size;

    if(!url) return 0;

    /* 只处理本地文件（无 ://，或以 file:// 开头） */
    if(strstr(url, "://") && strncmp(url, "file://", 7) != 0) return 0;
    const char *path = (strncmp(url, "file://", 7) == 0) ? url + 7 : url;

    f = fopen(path, "rb");
    if(!f) return 0;

    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    rewind(f);

    n = fread(buf, 1, sizeof(buf), f);
    fclose(f);
    if(n < 12) return 0;

    /* WAV: "RIFF....WAVE" */
    if(n >= 44 &&
       buf[0]=='R' && buf[1]=='I' && buf[2]=='F' && buf[3]=='F' &&
       buf[8]=='W' && buf[9]=='A' && buf[10]=='V' && buf[11]=='E') {
        /* fmt chunk: offset 12+ */
        /* byte_rate at offset 28 (4 bytes, LE) */
        uint32_t byte_rate = (uint32_t)buf[28] | ((uint32_t)buf[29]<<8)
                           | ((uint32_t)buf[30]<<16) | ((uint32_t)buf[31]<<24);
        if(byte_rate == 0) return 0;
        /* data chunk size at offset 40 (4 bytes) — assumes standard 44-byte header */
        uint32_t data_size = (uint32_t)buf[40] | ((uint32_t)buf[41]<<8)
                           | ((uint32_t)buf[42]<<16) | ((uint32_t)buf[43]<<24);
        return data_size / byte_rate * 1000U
             + (data_size % byte_rate) * 1000U / byte_rate;
    }

    /* FLAC: "fLaC" */
    if(n >= 42 &&
       buf[0]=='f' && buf[1]=='L' && buf[2]=='a' && buf[3]=='C') {
        /* STREAMINFO block starts at offset 4 */
        /* sample_rate: bits [80..99] of STREAMINFO = bytes [14..16] partially */
        /* STREAMINFO layout (after 4-byte block header):
           bytes 0-1: min blocksize
           bytes 2-3: max blocksize
           bytes 4-6: min framesize (24 bits)
           bytes 7-9: max framesize (24 bits)
           bytes 10-12+: 20-bit sample_rate | 3-bit channels-1 | 5-bit bps-1 | 36-bit total_samples */
        /* offset in buf = 4 (fLaC) + 4 (block header) + 10 = 18 */
        if(n < 42) return 0;
        uint32_t sr = ((uint32_t)buf[18] << 12) | ((uint32_t)buf[19] << 4)
                    | ((uint32_t)buf[20] >> 4);
        /* total_samples: 36 bits at bits [108..143] = 4.5 bytes from offset 21 */
        uint64_t total = ((uint64_t)(buf[21] & 0x0F) << 32)
                       | ((uint64_t)buf[22] << 24)
                       | ((uint64_t)buf[23] << 16)
                       | ((uint64_t)buf[24] << 8)
                       | (uint64_t)buf[25];
        if(sr == 0) return 0;
        return (uint32_t)(total * 1000ULL / sr);
    }

    /* MP3: find first sync word 0xFF 0xE0..0xFF — simplified CBR estimate */
    {
        uint32_t i;
        for(i = 0; i + 3 < (uint32_t)n; i++) {
            if(buf[i] == 0xFF && (buf[i+1] & 0xE0) == 0xE0) {
                /* MPEG header at buf[i..i+3] */
                uint8_t  version  = (buf[i+1] >> 3) & 0x03;
                uint8_t  layer    = (buf[i+1] >> 1) & 0x03;
                uint8_t  br_idx   = (buf[i+2] >> 4) & 0x0F;
                /* MPEG1 Layer3 bitrate table (kbps) */
                static const uint32_t kbps_table[16] = {
                    0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0
                };
                if(version == 3 && layer == 1 && br_idx > 0 && br_idx < 15) {
                    uint32_t bitrate_bps = kbps_table[br_idx] * 1000U;
                    if(bitrate_bps == 0) break;
                    /* skip ID3 tag size (assume first 4 bytes of file, rough) */
                    uint32_t audio_bytes = (uint32_t)(file_size > 128 ? file_size - 128 : file_size);
                    return audio_bytes * 8U / (bitrate_bps / 1000U);
                }
                break;
            }
        }
    }

    return 0;
}
```

**music_player_get_duration_ms() + 在 TRACK_CHANGED 事件时更新：**

```c
uint32_t music_player_get_duration_ms(void) {
    return g_current_duration_ms;
}
```

在 `music_player_async_handler` 中，TRACK_CHANGED 分支里添加（`_lv_demo_music_play()` 调用之后）：

```c
case PLAYER_CONTROLLER_EVENT_TRACK_CHANGED: {
    const player_playlist_item_t *item =
        player_controller_get_playlist_item(g_controller, arg->track_index);
    g_audio_sample_rate   = 0;   /* reset — new track has new format */
    g_audio_channels      = 0;
    g_current_duration_ms = (item && !item->is_live)
                            ? audio_probe_duration_ms(item->url) : 0U;
    _lv_demo_music_play((uint32_t)arg->track_index);
    break;
}
```

在 `music_player_init()` 中，`player_controller_play()` 之前也探测第一首：

```c
{
    const player_playlist_item_t *item =
        player_controller_get_playlist_item(g_controller, 0);
    g_current_duration_ms = (item && !item->is_live)
                            ? audio_probe_duration_ms(item->url) : 0U;
}
```

---

#### Step 1.5：music_player_seek() stub（层次3会替换，先加空实现）

```c
void music_player_seek(uint32_t position_ms) {
    (void)position_ms;
    /* implemented in seek layer */
}
```

---

#### Step 1.6：lv_demo_music_main.c —— 修改 timer_cb 使用真实位置

先读取文件，找到 `timer_cb` 函数（约第 1006 行）：

原始 `timer_cb`：
```c
static void timer_cb(lv_timer_t * t) {
    time_act++;
    lv_label_set_text_fmt(time_obj, "%"LV_PRIu32":%02"LV_PRIu32,
                          time_act / 60,
                          time_act % 60);
    lv_slider_set_value(slider_obj, time_act, LV_ANIM_ON);
}
```

替换为：
```c
static void timer_cb(lv_timer_t * t) {
    LV_UNUSED(t);
    uint32_t pos_ms  = music_player_get_position_ms();
    uint32_t pos_sec = pos_ms / 1000U;
    if(pos_sec != time_act) {
        time_act = pos_sec;
        lv_label_set_text_fmt(time_obj, "%"LV_PRIu32":%02"LV_PRIu32,
                              time_act / 60U,
                              time_act % 60U);
    }
    if(!g_seeking) {
        lv_slider_set_value(slider_obj, (int32_t)time_act, LV_ANIM_ON);
    }
}
```

在文件顶部（其他 `static` 变量附近）添加：
```c
static bool g_seeking = false;
```

---

#### Step 1.7：lv_demo_music_main.c —— track_load 使用真实 duration

找到 `lv_slider_set_range(slider_obj, 0, _lv_demo_music_get_track_length(track_id));`（约第 364 行）。

替换为：
```c
{
    uint32_t dur_sec = music_player_get_duration_ms() / 1000U;
    if(dur_sec == 0U) dur_sec = _lv_demo_music_get_track_length(track_id);
    lv_slider_set_range(slider_obj, 0, (int32_t)dur_sec);
}
```

---

#### Step 1.8：确认 music_player.h 已在 lv_demo_music_main.c 中 include

检查文件顶部是否已有 `#include "music_player.h"`（Task 3 已加入）。若无则添加。

---

#### Step 1.9：编译验证

```bash
cmake --build /home/share/samba/lvgl/lv_port_pc_vscode_v3-music/build --target main -j$(nproc) 2>&1 | tail -20
```

期望：无错误。

---

#### Step 1.10：Commit

```bash
cd /home/share/samba/lvgl/lv_port_pc_vscode_v3-music
git add main/inc/music_player.h main/src/music_player.c lvgl/demos/music/lv_demo_music_main.c
git commit -m "feat: real position/duration tracking in music player"
```

---

### Task 2：hls_player_demo —— PCM skip seek 机制（层次 3，内部）

> 可与 Task 1 并行执行（修改文件不重叠）。

**Files:**
- Modify: `third-party/hls_player_demo/src/stream_player/src/stream_player_internal.h`
- Modify: `third-party/hls_player_demo/src/stream_player/src/pipeline/stream_player_playback.c`
- Modify: `third-party/hls_player_demo/src/stream_player/include/stream_player.h`
- Modify: `third-party/hls_player_demo/src/stream_player/src/core/stream_player.c`
- Modify: `third-party/hls_player_demo/src/player_controller/player_controller.h`
- Modify: `third-party/hls_player_demo/src/player_controller/player_controller.c`

---

#### Step 2.1：stream_player_internal.h —— 添加 seek 字段

在 `struct stream_player` 末尾（`};` 之前）添加：

```c
    /* seek support */
    volatile uint64_t skip_pcm_bytes; /* bytes to discard from audio output; playback thread reads/decrements */
    uint32_t seek_sample_rate;        /* detected output sample rate */
    uint8_t  seek_channels;           /* detected output channel count */
    uint8_t  seek_bits_per_sample;    /* detected output bits per sample */
```

---

#### Step 2.2：stream_player_playback.c —— 存储 audio 参数 + skip 逻辑

**Step 2.2a**: 在 `HLS_OBSERVER_AUDIO_OUTPUT_STARTED_HOOK` 调用之后（约第 776 行），存储检测到的音频参数：

```c
                    HLS_OBSERVER_AUDIO_OUTPUT_STARTED_HOOK(...);
                    /* [NEW] store detected audio params for seek calculation */
                    player->seek_sample_rate     = (uint32_t)audio_info.sample_rate;
                    player->seek_channels        = (uint8_t)audio_info.channels;
                    player->seek_bits_per_sample = (uint8_t)player->config.bits_per_sample;
                    stream_player_state_notify(...);
```

**Step 2.2b**: 在 `stream_player_platform_audio_write(output_buffer, output_bytes)` 调用处（约第 789 行），替换该调用块：

原始：
```c
                {
                    int written = stream_player_platform_audio_write(output_buffer, output_bytes);
                    ...
                    stream_player_add_pcm_bytes_written(player, (uint64_t)written);
                }
```

替换为：
```c
                {
                    /* seek: skip PCM bytes without writing to audio output */
                    int      written      = 0;
                    size_t   skip_bytes   = 0;
                    size_t   write_size   = (size_t)output_bytes;
                    uint8_t *write_buf    = output_buffer;

                    if(player->skip_pcm_bytes > 0) {
                        uint64_t can_skip = (uint64_t)write_size;
                        if(can_skip > player->skip_pcm_bytes)
                            can_skip = player->skip_pcm_bytes;
                        /* atomic-like: single writer (this thread), single reader (seek sets it once) */
                        player->skip_pcm_bytes -= can_skip;
                        skip_bytes  = (size_t)can_skip;
                        write_buf  += skip_bytes;
                        write_size -= skip_bytes;
                    }

                    if(write_size > 0) {
                        written = stream_player_platform_audio_write(write_buf, (int)write_size);
                        if(stream_player_playback_should_report_audio_write_error(
                               stream_player_should_stop(player), written)) {
                            stream_player_playback_log(player, STREAM_LOG_ERROR,
                                                       "Audio output error: %d\n", written);
                            stream_player_playback_record_error(player, STREAM_ERROR_AUDIO,
                                                                STREAM_ERROR_SOURCE_AUDIO, 0);
                            stream_player_increment_stat_counter(player, STREAM_PLAYER_STAT_AUDIO_ERROR);
                            stream_player_playback_emit_event(player, STREAM_PLAYER_EVENT_AUDIO_ERROR, NULL);
                            stream_player_state_notify(player, STREAM_PLAYER_STATE_ERROR);
                            break;
                        } else if(written < 0) {
                            break;
                        }
                    }

                    stream_player_increment_stat_counter(player, STREAM_PLAYER_STAT_DECODE_FRAME);
                    stream_player_mark_playable_media_progress(player, (uint64_t)output_bytes);
                    /* pcm_bytes_written tracks logical position (skipped + written) */
                    stream_player_add_pcm_bytes_written(player,
                        (uint64_t)skip_bytes + (uint64_t)(written > 0 ? written : 0));
                }
```

**注意**：`output_buffer` 的类型需要确认是 `uint8_t*`。如果是 `void*` 或 `int16_t*`，需要先转换为 `uint8_t*` 再做指针算术。读取文件后按实际类型调整。

---

#### Step 2.3：stream_player.h —— 声明 stream_player_seek

在文件末尾（`#endif` 之前）添加：

```c
/**
 * @brief Seek to a position in the current track (local files only).
 *        For HLS live streams, returns STREAM_ERROR_NOT_SUPPORTED.
 * @param position_ms Target position in milliseconds from track start.
 */
stream_error_t stream_player_seek(stream_player_t *player, uint32_t position_ms);
```

---

#### Step 2.4：stream_player.c —— 实现 stream_player_seek

在文件末尾添加（在其他函数之后）：

```c
stream_error_t stream_player_seek(stream_player_t *player, uint32_t position_ms) {
    uint64_t target_bytes;
    uint64_t current_bytes;
    uint32_t sr, ch, bps;

    if(!player) return STREAM_ERROR_INVALID_ARGUMENT;

    sr  = (player->seek_sample_rate > 0) ? player->seek_sample_rate
                                          : (uint32_t)player->config.sample_rate;
    ch  = (player->seek_channels  > 0)   ? player->seek_channels
                                          : (uint32_t)player->config.channels;
    bps = (player->seek_bits_per_sample > 0) ? player->seek_bits_per_sample
                                              : (uint32_t)player->config.bits_per_sample;

    if(sr == 0 || ch == 0 || bps == 0) return STREAM_ERROR_INVALID_ARGUMENT;

    target_bytes  = (uint64_t)position_ms * sr * ch * (bps / 8U) / 1000ULL;

    {
        stream_player_stats_t stats;
        if(stream_player_get_stats(player, &stats) != STREAM_OK)
            current_bytes = 0;
        else
            current_bytes = stats.pcm_bytes_written;
    }

    if(target_bytes >= current_bytes) {
        /* forward seek: add to skip counter */
        player->skip_pcm_bytes += (target_bytes - current_bytes);
        return STREAM_OK;
    }

    /* backward seek: restart stream from beginning, then skip to target */
    player->skip_pcm_bytes = target_bytes;
    /* stream_player_change_url with same URL: stops + resets pcm_bytes_written + restarts */
    return stream_player_change_url(player, player->config.url);
}
```

---

#### Step 2.5：player_controller.h —— 声明 player_controller_seek

在 `player_controller_get_stats` 声明之后添加：

```c
/**
 * @brief Seek to a position in the current track.
 *        No-op for live streams (is_live == true) or if controller has no active player.
 * @return 0 on success, -1 on error or unsupported.
 */
int player_controller_seek(player_controller_t *controller, uint32_t position_ms);
```

---

#### Step 2.6：player_controller.c —— 实现 player_controller_seek

在文件末尾（`player_controller_get_stats` 实现之后）添加：

```c
int player_controller_seek(player_controller_t *controller, uint32_t position_ms) {
    const player_playlist_item_t *item;

    if(!controller || !controller->player) return -1;

    item = player_controller_get_current_item(controller);
    if(!item || item->is_live) return -1;   /* live streams cannot be seeked */

    return (stream_player_seek(controller->player, position_ms) == STREAM_OK) ? 0 : -1;
}
```

---

#### Step 2.7：编译验证（hls_player_demo 子库）

```bash
cmake --build /home/share/samba/lvgl/lv_port_pc_vscode_v3-music/build --target player_controller stream_player_module_linux -j$(nproc) 2>&1 | tail -20
```

期望：无错误。如有类型错误，修正后重编。

---

#### Step 2.8：Commit

```bash
cd /home/share/samba/lvgl/lv_port_pc_vscode_v3-music
git add third-party/hls_player_demo/
git commit -m "feat: add PCM-skip seek to stream_player and player_controller"
```

---

### Task 3：Seek 全链路 wiring（层次 3，主项目）

> 依赖 Task 1 和 Task 2 全部完成。

**Files:**
- Modify: `main/inc/music_player.h`（已有声明，无需再改）
- Modify: `main/src/music_player.c`
- Modify: `lvgl/demos/music/lv_demo_music_main.c`

---

#### Step 3.1：music_player.c —— 实现真正的 music_player_seek

找到 Task 1 中添加的 stub：
```c
void music_player_seek(uint32_t position_ms) {
    (void)position_ms;
}
```

替换为：
```c
void music_player_seek(uint32_t position_ms) {
    if(!g_controller) return;
    player_controller_seek(g_controller, position_ms);
    /* reset local position tracking for immediate UI response */
    if(g_audio_sample_rate > 0 && g_audio_channels > 0) {
        /* update g_audio stats reflect new target — stats will converge via poll */
    }
}
```

---

#### Step 3.2：lv_demo_music_main.c —— slider 拖动 seek

在 slider 创建区块（约第 628 行，`lv_obj_add_event_cb(slider_obj, del_counter_timer_cb, LV_EVENT_DELETE, NULL)` 之后）添加两个事件回调注册：

```c
lv_obj_add_event_cb(slider_obj, slider_pressed_cb,  LV_EVENT_PRESSED,  NULL);
lv_obj_add_event_cb(slider_obj, slider_released_cb, LV_EVENT_RELEASED, NULL);
```

在文件顶部（其他 `static void` 回调之后）添加两个回调函数实现：

```c
static void slider_pressed_cb(lv_event_t *e) {
    LV_UNUSED(e);
    g_seeking = true;   /* freeze timer_cb position update while dragging */
}

static void slider_released_cb(lv_event_t *e) {
    lv_obj_t *slider  = lv_event_get_target(e);
    int32_t   val_sec = lv_slider_get_value(slider);
    g_seeking = false;
    music_player_seek((uint32_t)val_sec * 1000U);
    /* Update time label immediately */
    time_act = (uint32_t)val_sec;
    lv_label_set_text_fmt(time_obj, "%"LV_PRIu32":%02"LV_PRIu32,
                          time_act / 60U, time_act % 60U);
}
```

---

#### Step 3.3：lv_demo_music_main.c —— 直播流禁用 slider

在 `track_load()` 中，设置 slider range 处，根据是否 live stream 控制可交互性：

```c
{
    uint32_t dur_sec = music_player_get_duration_ms() / 1000U;
    if(dur_sec == 0U) dur_sec = _lv_demo_music_get_track_length(track_id);
    lv_slider_set_range(slider_obj, 0, (int32_t)dur_sec);
    /* disable drag for live streams (duration unknown) */
    if(music_player_get_duration_ms() == 0U && music_player_get_count() > 0U) {
        lv_obj_remove_flag(slider_obj, LV_OBJ_FLAG_CLICKABLE);
    } else {
        lv_obj_add_flag(slider_obj, LV_OBJ_FLAG_CLICKABLE);
    }
}
```

---

#### Step 3.4：全量编译验证

```bash
cmake --build /home/share/samba/lvgl/lv_port_pc_vscode_v3-music/build --target main -j$(nproc) 2>&1 | tail -20
```

期望：无错误，无新警告。

---

#### Step 3.5：Commit

```bash
cd /home/share/samba/lvgl/lv_port_pc_vscode_v3-music
git add main/src/music_player.c lvgl/demos/music/lv_demo_music_main.c
git commit -m "feat: wire seek through music_player to slider UI"
```

---

### Task 4：端到端验证

**Files:** 只读

- [ ] **Step 4.1：编译干净**
  ```bash
  cmake --build .../build --target main -j$(nproc) 2>&1
  ```
  期望：`Built target main`，无 Error。

- [ ] **Step 4.2：运行无参数冒烟测试（降级模式）**
  ```bash
  timeout 3 .../build/bin/main 2>&1 || true
  ```
  期望：不 segfault。

- [ ] **Step 4.3：运行本地文件，验证进度条同步**
  ```bash
  timeout 8 .../build/bin/main /path/to/test.mp3 2>&1 || true
  ```
  期望：进度条随播放推进（不再是从 0 开始每秒+1 的假计时）。

- [ ] **Step 4.4：git log 确认所有 commit 到位**
  ```bash
  cd .../lv_port_pc_vscode_v3-music && git log --oneline -8
  ```

- [ ] **Step 4.5：Commit（如有残余未提交改动）**
  若 `git status` 有未提交内容，一并提交。
