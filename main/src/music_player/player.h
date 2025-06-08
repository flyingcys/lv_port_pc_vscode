#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <pthread.h>

// 定义播放器状态枚举
typedef enum {
    PLAYER_STOPPED = 0,
    PLAYER_PLAYING,
    PLAYER_PAUSED
} player_state_t;

// 网络流缓冲区结构 (修改为环形缓冲区)
typedef struct {
    uint8_t *data;
    size_t capacity;
    size_t size;
    size_t read_pos;
    size_t write_pos;
    int finished;
    int error;  // 添加错误标志
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} stream_buffer_t;

// PCM缓冲区结构
typedef struct {
    int16_t *data;          // PCM数据
    size_t capacity;        // 缓冲区容量（帧数）
    size_t size;            // 当前缓冲区中的帧数
    size_t read_pos;        // 读取位置
    size_t write_pos;       // 写入位置
    pthread_mutex_t mutex;  // 互斥锁
    pthread_cond_t not_full;    // 缓冲区不满条件变量
    pthread_cond_t not_empty;   // 缓冲区不空条件变量
    int finished;           // 解码是否完成
    int channels;           // 声道数
} pcm_buffer_t;

// 播放数据结构
typedef struct {
    const char *filename;
    uint8_t *buffer;
    size_t buffer_size;
    int is_stream; // 1为网络流，0为本地文件
    stream_buffer_t *stream_buffer; // 网络流缓冲区
    pcm_buffer_t *pcm_buffer; // PCM数据缓冲区
} playback_data_t;


// 音频播放函数
int play_audio(const char *filename);

// 暂停播放
void pause_playback(void);

// 继续播放
void resume_playback(void);

// 停止播放
void stop_playback(void);

// 设置音量 (0-100)
void set_volume(int volume);

// 获取暂停状态
int is_paused(void);

// 获取播放状态
int is_playing(void);

#endif // PLAYER_H