# 跨平台音乐播放器

一个高性能、跨平台的命令行音乐播放器，支持多种音频格式和网络流媒体。

## 特性

- 🎵 **多格式支持**: MP3, AAC, Opus, WAV, M3U8
- 🔄 **多线程架构**: 采用生产者-消费者模式，音频源、解码、输出分离
- 💾 **环形缓冲区**: 高效的音频数据缓冲管理
- 🌐 **网络流支持**: HTTP/HTTPS流媒体播放
- 🖥️ **跨平台**: Linux, ESP32 (RTOS)
- 🎛️ **实时控制**: 播放/暂停/音量调节/定位
- 📊 **事件系统**: 播放状态、进度、错误事件通知
- 🔧 **模块化设计**: 解码器、音频源、输出设备可插拔

## 架构设计

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   音频源线程    │───▶│   解码线程      │───▶│   输出线程      │
│  (文件/网络)    │    │  (MP3/AAC等)    │    │ (ALSA/I2S等)    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  源数据缓冲区   │    │  PCM数据缓冲区  │    │   音频输出      │
│  (RingBuffer)   │    │  (RingBuffer)   │    │    设备         │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## 目录结构

```
music/
├── src/
│   ├── core/                    # 核心功能模块
│   │   ├── ringbuffer.c/h       # 环形缓冲区实现
│   │   ├── player.c/h           # 播放器核心控制
│   │   ├── decoder.c/h          # 解码器抽象接口
│   │   ├── source.c/h           # 音频源抽象接口
│   │   └── output.c/h           # 音频输出抽象接口
│   ├── decoders/                # 各种解码器实现
│   │   ├── mp3_decoder.c/h      # MP3解码器
│   │   ├── aac_decoder.c/h      # AAC解码器
│   │   ├── opus_decoder.c/h     # Opus解码器
│   │   └── wav_decoder.c/h      # WAV解码器
│   ├── sources/                 # 音频源实现
│   │   ├── file_source.c/h      # 本地文件源
│   │   ├── http_source.c/h      # HTTP流媒体源
│   │   └── m3u8_source.c/h      # M3U8播放列表源
│   ├── outputs/                 # 音频输出实现
│   │   ├── alsa_output.c/h      # Linux ALSA输出
│   │   ├── pulse_output.c/h     # PulseAudio输出
│   │   └── i2s_output.c/h       # ESP32 I2S输出
│   ├── platform/                # 平台抽象层
│   │   ├── platform.h           # 平台通用接口
│   │   ├── linux/               # Linux实现
│   │   └── esp32/               # ESP32实现
│   ├── ui/                      # 用户界面
│   │   ├── cli.c/h              # 命令行界面
│   │   └── lvgl_ui.c/h          # LVGL图形界面
│   └── main.c                   # 程序入口
├── include/                     # 公共头文件
│   └── audio_player.h           # 对外接口定义
├── libs/                        # 第三方库
│   ├── minimp3/                 # MP3解码库
│   └── dr_libs/                 # 音频处理库集合
├── examples/                    # 示例代码
└── CMakeLists.txt               # CMake构建脚本
```

## 构建方式

### Linux平台

#### 依赖安装

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake libasound2-dev libpulse-dev libcurl4-openssl-dev
```

**CentOS/RHEL:**
```bash
sudo yum install gcc gcc-c++ cmake alsa-lib-devel pulseaudio-libs-devel libcurl-devel
```

#### 编译

```bash
cd music
mkdir build
cd build
cmake ..
make -j$(nproc)
```

#### 调试版本

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### ESP32平台

#### 环境准备
1. 安装ESP-IDF开发环境
2. 配置ESP-IDF环境变量

#### 编译
```bash
cd music
# 配置为ESP32目标
cmake -DCMAKE_SYSTEM_NAME=Generic -DCMAKE_TOOLCHAIN_FILE=$IDF_PATH/tools/cmake/toolchain-esp32.cmake ..
make -j$(nproc)
```

## 使用方法

### 基本用法

```bash
# 播放本地音频文件
./audio_player /path/to/music.mp3

# 播放网络流媒体
./audio_player http://example.com/stream.mp3

# 播放M3U8播放列表
./audio_player http://example.com/playlist.m3u8
```

### 控制命令

播放过程中支持以下控制命令：

- **空格键**: 播放/暂停
- **s**: 停止播放
- **+/-**: 增加/减少音量
- **i**: 显示音频信息
- **h/?**: 显示帮助
- **q/Ctrl+C**: 退出程序

## API使用示例

### 简单播放示例

```c
#include "audio_player.h"

int main() {
    // 初始化
    audio_player_init();
    
    // 创建播放器
    AudioPlayer *player = player_create();
    
    // 打开音频文件
    player_open(player, "music.mp3");
    
    // 开始播放
    player_play(player);
    
    // 等待播放完成...
    
    // 清理
    player_destroy(player);
    audio_player_cleanup();
    
    return 0;
}
```

### 带事件回调的示例

```c
#include "audio_player.h"

void event_callback(PlayerEvent *event, void *user_data) {
    switch (event->type) {
        case PLAYER_EVENT_STATE_CHANGED:
            printf("状态改变: %s\n", 
                   audio_player_get_state_name(event->data.state));
            break;
            
        case PLAYER_EVENT_POSITION_CHANGED:
            printf("播放位置: %lld ms\n", event->data.position);
            break;
            
        case PLAYER_EVENT_ERROR:
            printf("播放错误: %s\n", event->data.error);
            break;
    }
}

int main() {
    audio_player_init();
    
    AudioPlayer *player = player_create();
    player_set_event_callback(player, event_callback, NULL);
    
    player_open(player, "music.mp3");
    player_play(player);
    
    // 控制播放
    sleep(5);
    player_pause(player);
    
    sleep(2);
    player_play(player);
    
    sleep(10);
    player_stop(player);
    
    player_destroy(player);
    audio_player_cleanup();
    
    return 0;
}
```

## 配置选项

播放器支持多种配置选项，可在创建播放器后进行设置：

```c
AudioPlayer *player = player_create();

// 设置缓冲区大小
player->config.source_buffer_size = 256 * 1024;  // 256KB
player->config.output_buffer_size = 512 * 1024;  // 512KB

// 设置预缓冲时间
player->config.prebuffer_ms = 1000;  // 1秒

// 启用无缝播放
player->config.enable_gapless = true;
```

## 性能优化

### 内存使用优化
- 根据设备内存调整缓冲区大小
- ESP32等资源受限设备建议使用较小缓冲区

### 网络播放优化
- 增加网络缓冲区大小
- 设置合适的预缓冲时间
- 使用HTTP Keep-Alive

### 多线程优化
- 调整线程优先级
- 合理设置线程CPU亲和性

## 故障排除

### 常见问题

1. **编译错误: 找不到头文件**
   - 确保安装了所有依赖库
   - 检查CMake配置

2. **播放无声音**
   - 检查ALSA/PulseAudio配置
   - 确认音频设备权限

3. **网络流播放失败**
   - 检查网络连接
   - 确认URL有效性
   - 查看防火墙设置

4. **ESP32编译失败**
   - 确认ESP-IDF版本兼容性
   - 检查交叉编译工具链

### 调试

启用调试模式：
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

查看详细日志：
```bash
export AUDIO_PLAYER_LOG_LEVEL=DEBUG
./audio_player music.mp3
```

## 扩展开发

### 添加新的解码器

1. 在`src/decoders/`创建新的解码器文件
2. 实现`AudioDecoderOperations`接口
3. 在解码器工厂中注册新格式

### 添加新的音频源

1. 在`src/sources/`创建新的音频源文件
2. 实现`AudioSourceOperations`接口
3. 在音频源工厂中注册新协议

### 添加新的输出设备

1. 在`src/outputs/`创建新的输出设备文件
2. 实现`AudioOutputOperations`接口
3. 在输出设备工厂中注册新设备

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 贡献

欢迎提交Issue和Pull Request！

## 联系方式

- 项目主页: [GitHub链接]
- 问题反馈: [Issues页面]
- 技术讨论: [讨论区链接] 