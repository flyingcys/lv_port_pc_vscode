#ifndef PLAYER_H
#define PLAYER_H

#include "audio_player.h"
#include "ringbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

// 前向声明
typedef struct AudioSource AudioSource;
typedef struct AudioDecoder AudioDecoder;  
typedef struct AudioOutput AudioOutput;

// 播放器内部结构
typedef struct AudioPlayer {
    // 组件
    AudioSource *source;         // 音频源
    AudioDecoder *decoder;       // 解码器
    AudioOutput *output;         // 音频输出
    
    // 缓冲区
    RingBuffer *source_buffer;   // 音频源到解码器的缓冲区
    RingBuffer *output_buffer;   // 解码器到输出的缓冲区
    
    // 线程
    void *source_thread;         // 音频源线程
    void *decode_thread;         // 解码线程
    void *output_thread;         // 输出线程
    
    // 同步
    void *control_mutex;         // 控制互斥锁
    
    // 状态
    PlayerState state;           // 播放器状态
    int64_t position;            // 当前播放位置 (毫秒)
    int64_t duration;            // 总时长 (毫秒)
    int volume;                  // 当前音量 (0-100)
    
    // 控制标志
    bool request_stop;           // 停止请求标志
    bool request_pause;          // 暂停请求标志
    bool request_seek;           // 定位请求标志
    int64_t seek_position;       // 定位目标位置
    
    // 事件回调
    PlayerEventCallback event_callback;
    void *callback_data;
    
    // 配置
    struct {
        size_t source_buffer_size;   // 源缓冲区大小
        size_t output_buffer_size;   // 输出缓冲区大小
        int decode_thread_priority;  // 解码线程优先级
        int output_thread_priority;  // 输出线程优先级
        bool enable_gapless;         // 启用无缝播放
        int prebuffer_ms;            // 预缓冲时间(毫秒)
    } config;
    
    // 统计信息
    struct {
        int64_t bytes_read;          // 已读取字节数
        int64_t bytes_decoded;       // 已解码字节数
        int64_t bytes_played;        // 已播放字节数
        int decode_errors;           // 解码错误数
        int buffer_underruns;        // 缓冲区下溢次数
    } stats;
} AudioPlayer;

// 播放器内部函数
void player_fire_event(AudioPlayer *player, PlayerEventType type, ...);
void player_set_state(AudioPlayer *player, PlayerState state);
bool player_wait_for_state(AudioPlayer *player, PlayerState state, int timeout_ms);

// 线程函数声明
void source_thread_func(void *arg);
void decode_thread_func(void *arg);
void output_thread_func(void *arg);

#ifdef __cplusplus
}
#endif

#endif // PLAYER_H 