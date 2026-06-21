# v9_apple_music 接入真实播放服务设计

**日期：** 2026-06-19  
**分支：** feat/apple-music-lvgl-ui  
**范围：** 迷你播放条 + 本地页 + 广播页全部接通真实音频后端

---

## 背景

`v9_apple_music` UI 当前完全使用 `am_data.c` 的静态假数据；控制按钮无回调；`music_player.c`（已封装 `player_controller`）在 `main.c` 中被注释掉。本设计将三者全部接通，保持其他页面（主页/歌单/设置）不变。

---

## 一、整体架构

### 新增文件

```
main/src/v9_apple_music/
  am_player.h / am_player.c   ← 播放器适配层（核心新增）
  am_config.h / am_config.c   ← JSON 配置解析

third-party/cjson/
  cJSON.h / cJSON.c           ← 单文件 cJSON 库（新增依赖）
```

### 数据流

```
config.json
    │ am_config_load()
    ▼
am_config_t { local_dir, radio_items[] }
    │
    ├─→ am_page_local_create()              扫描目录 → 真实列表
    ├─→ am_page_list_create(动态构造数据)   读取 radio[] → 真实广播列表
    │
    └─→ am_player_init()
            │ player_controller_create()
            │ music_player_init()
            │
            ├─ lv_timer 100ms → poll position → 更新 progress fill
            └─ lv_async_call  → TRACK_CHANGED/STATE_CHANGED → 更新 title/icon
```

### 线程边界

`player_controller` 内部维护自己的线程。所有 LVGL widget 操作只在 `lv_async_call` 回调（主线程）里执行，沿用 `music_player.c` 现有模式，不新增锁。

---

## 二、am_player 适配层

### 公开 API

```c
/* am_player.h */

typedef struct {
    lv_obj_t *title_label;
    lv_obj_t *subtitle_label;
    lv_obj_t *time_cur;
    lv_obj_t *time_total;
    lv_obj_t *progress_fill;
    lv_obj_t *play_icon;
} am_miniplayer_handles_t;

void am_player_init(void);
void am_player_deinit(void);

/* mini-player 重建后调用（主题切换也需调用）*/
void am_player_bind_miniplayer(const am_miniplayer_handles_t *h);

/* 本地页：替换整个播放列表，从 start_index 开始播放 */
void am_player_load_local(const char **urls, size_t count, size_t start_index);

/* 广播页：直接切换 HLS 流，不改本地列表 */
void am_player_play_stream(const char *url, const char *title);

/* 挂到控制按钮的 LVGL 事件回调 */
void am_player_on_play_pause(lv_event_t *e);
void am_player_on_prev(lv_event_t *e);
void am_player_on_next(lv_event_t *e);
```

### mini-player 改动

- `am_shell_build_miniplayer` 签名改为返回 `am_miniplayer_handles_t`
- 三个控制按钮增加 `lv_obj_add_event_cb`，分别挂 `am_player_on_play_pause/prev/next`
- `am_shell.h` 同步更新

### 重建安全

主题切换时 `build_all()` 重建整个 UI，之后立刻调 `am_player_bind_miniplayer(&new_handles)`，旧句柄自动替换，无悬挂指针风险。

---

## 三、进度条精度处理

| 情况 | `duration_ms` | 处理方式 |
|------|--------------|---------|
| WAV / FLAC / CBR MP3 | > 0 | 按 `position_ms / duration_ms` 更新 fill 宽度和时间文字 |
| VBR MP3 或探测失败 | = 0 | fill 不更新，时间显示 `--:--` |
| HLS 直播流 | = 0，is_live=true | fill 固定宽度，时间区域显示 `LIVE` 标签 |

位置计算依赖 `hls_observer_audio_output_started` 设置音频参数，播放开始前短暂返回 0——不影响显示（fill 停在初始值）。

---

## 四、配置文件

### 查找顺序

1. `$AM_CONFIG` 环境变量指定路径
2. `./am_config.json`（工作目录）
3. `~/.config/apple_music/config.json`

### 格式

```json
{
  "local": {
    "dir": "/home/user/Music"
  },
  "radio": [
    { "title": "Lo-Fi 早班电台", "subtitle": "低压氛围", "url": "https://example.com/lofi.m3u8" },
    { "title": "City Pop Avenue", "subtitle": "东京夜风", "url": "https://example.com/citypop.m3u8" }
  ]
}
```

### 数据结构

```c
/* am_config.h */
#define AM_CONFIG_RADIO_MAX 16

typedef struct {
    char title[128];
    char subtitle[128];
    char url[512];
} am_radio_item_t;

typedef struct {
    char local_dir[512];
    am_radio_item_t radio[AM_CONFIG_RADIO_MAX];
    int radio_count;
} am_config_t;

int  am_config_load(am_config_t *out);   /* 0=ok, -1=not found, -2=parse error */
void am_config_free(am_config_t *cfg);   /* 目前为 no-op，结构体全是数组 */
```

### 降级行为

- `local.dir` 缺失或目录不存在 → 本地页显示"暂无本地音乐"占位
- `radio` 为空 → 广播页显示"暂无广播源"占位
- 整个文件不存在 → 两页均占位，mini-player 静态展示，不崩溃

cJSON 以单文件放入 `third-party/cjson/`，CMakeLists 加一条 `target_sources`。

---

## 五、页面改动

### 本地页

新增 `am_page_local_create(lv_obj_t *parent, const am_config_t *cfg)`：

- 扫描 `cfg->local_dir`，用 `stream_player_is_supported_local_audio_file()` 过滤
- 每个文件渲染为列表项（文件名为标题，路径存入 `lv_obj_set_user_data`）
- 点击 → `am_player_load_local(urls, count, clicked_index)`
- 目录为空 → 显示占位提示

### 广播页

广播页复用 `am_page_list_create`，数据来源从静态 `am_page_radio` 切换为运行时从 `am_config_t.radio[]` 构造的 `am_list_page_t`：

- 点击列表项 → `am_player_play_stream(url, title)` 直接切流
- mini-player 标题立即更新为广播名，进度条切为 LIVE 模式

### 不动页面

主页、歌单、设置继续使用 `am_data.c` 静态数据。

---

## 六、构建变更

| 变更 | 说明 |
|------|------|
| `third-party/cjson/` 新增 | cJSON 单文件库 |
| `main/src/v9_apple_music/am_player.c` | 新增 |
| `main/src/v9_apple_music/am_config.c` | 新增 |
| `main/src/v9_apple_music/am_shell.c` | mini-player 返回句柄，按钮加回调 |
| `main/src/v9_apple_music/am_page_list.c` | 广播页数据改为运行时构造 |
| `main/src/music_player.c` | 新增 `music_player_reload(urls, count)`（供 `am_player_load_local` 调用，内部做 deinit+reinit）；暴露 `music_player_get_current_title()` 等查询接口 |
| `main/src/main.c` | 取消注释 `music_player_init`，改由 `am_player_init` 统一调用 |
| `CMakeLists.txt` | 加入 cjson、am_player、am_config 的 source |

---

## 七、不在本次范围内

- 歌词显示
- 封面图片（网络获取）
- 播放队列 UI 联动
- seek（进度条拖拽）
- 歌单页真实数据
