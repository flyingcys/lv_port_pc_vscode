#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "player.h"
#include "network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 16384
#define SAMPLE_RATE 44100
#define CHANNELS    2

// 全局变量
player_state_t g_player_state = PLAYER_STOPPED;
static float current_volume = 1.0f;
static pthread_mutex_t audio_mutex = PTHREAD_MUTEX_INITIALIZER;
static snd_pcm_t *audio_handle = NULL;
static pthread_t playback_thread;
static mp3dec_t mp3d;



// 初始化PCM缓冲区
static pcm_buffer_t *init_pcm_buffer(size_t capacity, int channels)
{
    pcm_buffer_t *buffer = malloc(sizeof(pcm_buffer_t));

    if (!buffer)
        return NULL;
    
    buffer->data = malloc(capacity * channels * sizeof(int16_t));
    if (!buffer->data) {
        free(buffer);
        return NULL;
    }
    
    buffer->capacity = capacity;
    buffer->size = 0;
    buffer->read_pos = 0;
    buffer->write_pos = 0;
    buffer->finished = 0;
    buffer->channels = channels;
    
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->not_full, NULL);
    pthread_cond_init(&buffer->not_empty, NULL);
    
    return buffer;
}

// 释放PCM缓冲区
static void free_pcm_buffer(pcm_buffer_t *buffer)
{
    if (!buffer)
        return;
    
    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->not_full);
    pthread_cond_destroy(&buffer->not_empty);
    free(buffer->data);
    free(buffer);
}

// 向PCM缓冲区写入数据
static int pcm_buffer_write(pcm_buffer_t *buffer, int16_t *pcm, size_t frames)
{
    pthread_mutex_lock(&buffer->mutex);
    
    // 等待缓冲区有足够空间
    while (buffer->size + frames > buffer->capacity && g_player_state != PLAYER_STOPPED) {
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }
    
    // 如果播放已停止，返回错误
    if (g_player_state == PLAYER_STOPPED) {
        pthread_mutex_unlock(&buffer->mutex);
        return -1;
    }
    
    // 计算可以写入的帧数
    size_t frames_to_write = frames;
    if (buffer->size + frames > buffer->capacity) {
        frames_to_write = buffer->capacity - buffer->size;
    }
    
    // 写入数据
    for (size_t i = 0; i < frames_to_write; i++) {
        for (int c = 0; c < buffer->channels; c++) {
            buffer->data[(buffer->write_pos * buffer->channels) + c] = 
                pcm[(i * buffer->channels) + c];
        }
        buffer->write_pos = (buffer->write_pos + 1) % buffer->capacity;
    }
    
    buffer->size += frames_to_write;
    
    // 通知消费者有数据可读
    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);
    
    return frames_to_write;
}

// 从PCM缓冲区读取数据
static int pcm_buffer_read(pcm_buffer_t *buffer, int16_t *pcm, size_t frames)
{
    pthread_mutex_lock(&buffer->mutex);
    
    // 等待缓冲区有数据
    while (buffer->size == 0 && !buffer->finished && g_player_state != PLAYER_STOPPED) {
        pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
    }
    
    // 如果缓冲区为空且解码已完成，或播放已停止，返回0
    if ((buffer->size == 0 && buffer->finished) || g_player_state == PLAYER_STOPPED) {
        pthread_mutex_unlock(&buffer->mutex);
        return 0;
    }
    
    // 计算可以读取的帧数
    size_t frames_to_read = frames;
    if (frames_to_read > buffer->size) {
        frames_to_read = buffer->size;
    }
    
    // 读取数据
    for (size_t i = 0; i < frames_to_read; i++) {
        for (int c = 0; c < buffer->channels; c++) {
            pcm[(i * buffer->channels) + c] = 
                buffer->data[(buffer->read_pos * buffer->channels) + c];
        }
        buffer->read_pos = (buffer->read_pos + 1) % buffer->capacity;
    }
    
    buffer->size -= frames_to_read;
    
    // 通知生产者有空间可写
    pthread_cond_signal(&buffer->not_full);
    pthread_mutex_unlock(&buffer->mutex);
    
    return frames_to_read;
}

// 初始化ALSA
static int init_alsa(void)
{
    int err;
    
    // 打开PCM设备
    if ((err = snd_pcm_open(&audio_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "无法打开PCM设备: %s\n", snd_strerror(err));
        return -1;
    }
    
    // 设置PCM参数
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(audio_handle, hw_params);
    snd_pcm_hw_params_set_access(audio_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(audio_handle, hw_params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(audio_handle, hw_params, CHANNELS);
    unsigned int rate = SAMPLE_RATE;
    snd_pcm_hw_params_set_rate_near(audio_handle, hw_params, &rate, 0);
    snd_pcm_uframes_t buffer_size = 4096;
    snd_pcm_hw_params_set_buffer_size_near(audio_handle, hw_params, &buffer_size);
    
    if ((err = snd_pcm_hw_params(audio_handle, hw_params)) < 0) {
        fprintf(stderr, "无法设置PCM硬件参数: %s\n", snd_strerror(err));
        snd_pcm_close(audio_handle);
        return -1;
    }
    
    return 0;
}


// 解码线程函数
static void *decode_thread_func(void *arg)
{
    playback_data_t *data = (playback_data_t *)arg;
    FILE *file = NULL;
    uint8_t *input_buffer = NULL;
    pthread_t download_thread;
    int is_network_stream = data->is_stream;
    long file_size = 0;
    pcm_buffer_t *pcm_buffer = data->pcm_buffer;
    
    if (is_network_stream) {
        // 网络流播放
        printf("开始下载网络流: %s\n", data->filename);
        
        // 初始化网络流缓冲区
        data->stream_buffer = init_stream_buffer(BUFFER_SIZE * 4);
        if (!data->stream_buffer) {
            fprintf(stderr, "错误: 无法分配网络流缓冲区\n");
            return NULL;
        }
        
        // 启动下载线程
        if (pthread_create(&download_thread, NULL, download_thread_func, data) != 0) {
            fprintf(stderr, "错误: 创建下载线程失败\n");
            free_stream_buffer(data->stream_buffer);
            return NULL;
        }
        
        input_buffer = malloc(BUFFER_SIZE);
        if (!input_buffer) {
            fprintf(stderr, "错误: 无法分配解码缓冲区\n");
            free_stream_buffer(data->stream_buffer);
            return NULL;
        }
    } else {
        // 本地文件播放
        file = fopen(data->filename, "rb");
        if (!file) {
            fprintf(stderr, "错误: 无法打开文件 '%s': %s\n", data->filename, strerror(errno));
            return NULL;
        }
        
        // 获取文件大小
        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        printf("开始解码文件: %s (大小: %.2f MB)\n", data->filename, file_size / 1024.0 / 1024.0);
        
        input_buffer = malloc(BUFFER_SIZE);
        if (!input_buffer) {
            fprintf(stderr, "错误: 无法分配解码缓冲区\n");
            fclose(file);
            return NULL;
        }
    }
    
    mp3dec_frame_info_t info;
    int16_t pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
    int total_frames = 0;
    int decode_errors = 0;
    size_t buffer_offset = 0;
    size_t bytes_remaining = 0;
    
    while (g_player_state != PLAYER_STOPPED) {
        // 解码线程函数中的网络流处理部分
        if (bytes_remaining < BUFFER_SIZE / 2) {
            if (buffer_offset > 0 && bytes_remaining > 0) {
                memmove(input_buffer, input_buffer + buffer_offset, bytes_remaining);
            }
            buffer_offset = 0;
            
            size_t bytes_read = 0;
            if (is_network_stream) {
                // 从网络流缓冲区读取数据
                bytes_read = stream_buffer_read(data->stream_buffer, 
                                              input_buffer + bytes_remaining, 
                                              BUFFER_SIZE - bytes_remaining);
                
                if (bytes_read == 0 && data->stream_buffer->finished) {
                    // 下载完成且缓冲区为空，解码结束
                    printf("网络流下载完成\n");
                    break;
                }
                
                if (bytes_read == 0) {
                    // 等待更多数据
                    usleep(10000);
                    continue;
                }
            } else {
                // 从文件读取数据
                bytes_read = fread(input_buffer + bytes_remaining, 1, 
                                  BUFFER_SIZE - bytes_remaining, file);
                if (bytes_read == 0) {
                    if (feof(file)) {
                        printf("文件读取完成\n");
                        break;
                    }
                    fprintf(stderr, "文件读取错误: %s\n", strerror(errno));
                    break;
                }
            }
            bytes_remaining += bytes_read;
        }
        
        int samples = mp3dec_decode_frame(&mp3d, input_buffer + buffer_offset, 
                                        bytes_remaining, pcm, &info);
        
        if (info.frame_bytes > 0) {
            buffer_offset += info.frame_bytes;
            bytes_remaining -= info.frame_bytes;
        } else {
            // 如果没有找到有效帧，跳过一个字节继续查找
            buffer_offset++;
            bytes_remaining--;
            continue;
        }
        
        if (samples > 0) {
            if (total_frames == 0) {
                printf("MP3信息: 比特率: %d kbps, 采样率: %d Hz, 声道数: %d\n", 
                       info.bitrate_kbps, info.hz, info.channels);
                
                // 更新PCM缓冲区的声道数
                pcm_buffer->channels = info.channels;
            }
            total_frames++;
            
            pthread_mutex_lock(&audio_mutex);
            // 应用音量
            for (int i = 0; i < samples * info.channels; i++) {
                pcm[i] = (int16_t)(pcm[i] * current_volume);
            }
            pthread_mutex_unlock(&audio_mutex);
            
            // 将解码后的PCM数据写入缓冲区
            if (pcm_buffer_write(pcm_buffer, pcm, samples) < 0) {
                // 写入失败，可能是播放已停止
                break;
            }
            
            // 每处理1000帧打印一次进度
            if (total_frames % 1000 == 0) {
                if (!is_network_stream && file) {
                    long current_pos = ftell(file);
                    float progress = (float)current_pos / file_size * 100;
                    printf("解码进度: %.1f%% (已解码 %d 帧)\n", progress, total_frames);
                } else {
                    printf("已解码 %d 帧\n", total_frames);
                }
            }
        }
    }
    
    printf("\n解码统计:\n");
    printf("总解码帧数: %d\n", total_frames);
    printf("解码错误数: %d\n", decode_errors);
    if (total_frames > 0) {
        printf("平均比特率: %d kbps\n", info.bitrate_kbps);
    }
    
    // 标记解码完成
    pthread_mutex_lock(&pcm_buffer->mutex);
    pcm_buffer->finished = 1;
    pthread_cond_signal(&pcm_buffer->not_empty); // 通知播放线程解码已完成
    pthread_mutex_unlock(&pcm_buffer->mutex);
    
    free(input_buffer);
    
    if (file) {
        fclose(file);
    }
    
    if (is_network_stream) {
        pthread_join(download_thread, NULL);
        free_stream_buffer(data->stream_buffer);
    }
    
    return NULL;
}

// 播放线程函数
static void *playback_thread_func(void *arg)
{
    playback_data_t *data = (playback_data_t *)arg;
    pcm_buffer_t *pcm_buffer = data->pcm_buffer;
    pthread_t decode_thread;
    int16_t pcm[BUFFER_SIZE];
    
    // 初始化ALSA
    if (init_alsa() < 0) {
        fprintf(stderr, "错误: ALSA音频系统初始化失败\n");
        return NULL;
    }
    printf("音频系统初始化成功 (采样率: %d Hz, 声道数: %d)\n", SAMPLE_RATE, CHANNELS);
    
    // 启动解码线程
    if (pthread_create(&decode_thread, NULL, decode_thread_func, data) != 0) {
        fprintf(stderr, "错误: 创建解码线程失败\n");
        return NULL;
    }
    
    while (g_player_state != PLAYER_STOPPED) {
        pthread_mutex_lock(&audio_mutex);
        if (g_player_state == PLAYER_PAUSED) {
            static int pause_message_shown = 0;
            if (!pause_message_shown) {
                printf("播放已暂停\n");
                pause_message_shown = 1;
            }
            pthread_mutex_unlock(&audio_mutex);
            usleep(10000); // 缩短等待时间为10毫秒
            continue; // 跳过当前帧处理
        }
        pthread_mutex_unlock(&audio_mutex);
        
        // 从PCM缓冲区读取数据
        int frames = pcm_buffer_read(pcm_buffer, pcm, BUFFER_SIZE / (pcm_buffer->channels * 2));
        
        if (frames == 0) {
            // 缓冲区为空且解码已完成，或播放已停止
            if (g_player_state == PLAYER_STOPPED) {
                printf("播放已停止\n");
            } else {
                printf("播放完成\n");
            }
            break;
        }
        
        // 播放音频
        snd_pcm_sframes_t written = snd_pcm_writei(audio_handle, pcm, frames);
        if (written < 0) {
            fprintf(stderr, "警告: PCM写入错误 (%s), 尝试恢复...\n", snd_strerror(written));
            written = snd_pcm_recover(audio_handle, written, 0);
        }
        
        if (written < 0) {
            fprintf(stderr, "错误: PCM写入失败: %s\n", snd_strerror(written));
            break;
        }
    }
    
    snd_pcm_close(audio_handle);
    pthread_join(decode_thread, NULL);
    
    free(data->buffer);
    free_pcm_buffer(pcm_buffer);
    free(data);
    return NULL;
}

int play_audio(const char *filename)
{
    if (!filename) {
        fprintf(stderr, "错误: 无效的文件名\n");
        return -1;
    }
    
    if (g_player_state != PLAYER_STOPPED) {
        printf("停止当前播放...\n");
        stop_playback();
    }
    
    playback_data_t *data = malloc(sizeof(playback_data_t));
    if (!data) {
        fprintf(stderr, "错误: 内存分配失败\n");
        return -1;
    }
    
    data->filename = filename;
    data->buffer = malloc(BUFFER_SIZE);
    if (!data->buffer) {
        fprintf(stderr, "错误: 缓冲区内存分配失败\n");
        free(data);
        return -1;
    }
    data->buffer_size = BUFFER_SIZE;
    
    // 判断是否为网络流
    if (strstr(filename, "http://") == filename || strstr(filename, "https://") == filename) {
        data->is_stream = 1;
    } else {
        data->is_stream = 0;
    }
    
    // 初始化PCM缓冲区
    data->pcm_buffer = init_pcm_buffer(BUFFER_SIZE, CHANNELS);
    if (!data->pcm_buffer) {
        fprintf(stderr, "错误: 无法分配PCM缓冲区\n");
        free(data->buffer);
        free(data);
        return -1;
    }
    
    g_player_state = PLAYER_PLAYING;
    
    mp3dec_init(&mp3d);
    printf("开始播放: %s\n", filename);
    
    if (pthread_create(&playback_thread, NULL, playback_thread_func, data) != 0) {
        fprintf(stderr, "错误: 创建播放线程失败\n");
        free_pcm_buffer(data->pcm_buffer);
        free(data->buffer);
        free(data);
        return -1;
    }
    
    return 0;
}

void pause_playback(void)
{
    pthread_mutex_lock(&audio_mutex);
    if (g_player_state == PLAYER_PLAYING) {
        g_player_state = PLAYER_PAUSED;
        if (audio_handle) {
            snd_pcm_pause(audio_handle, 1);
        }
    }
    pthread_mutex_unlock(&audio_mutex);
}

void resume_playback(void)
{
    pthread_mutex_lock(&audio_mutex);
    if (g_player_state == PLAYER_PAUSED) {
        g_player_state = PLAYER_PLAYING;
        if (audio_handle) {
            snd_pcm_pause(audio_handle, 0);
        }
    }
    pthread_mutex_unlock(&audio_mutex);
}

void stop_playback(void)
{
    pthread_mutex_lock(&audio_mutex);
    g_player_state = PLAYER_STOPPED;
    pthread_mutex_unlock(&audio_mutex);
    pthread_join(playback_thread, NULL);
}

void set_volume(int volume)
{
    pthread_mutex_lock(&audio_mutex);
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    current_volume = volume / 100.0f;
    printf("音量已设置为: %d%%\n", volume);
    pthread_mutex_unlock(&audio_mutex);
}

int is_paused(void)
{
    int paused;
    pthread_mutex_lock(&audio_mutex);
    paused = (g_player_state == PLAYER_PAUSED);
    pthread_mutex_unlock(&audio_mutex);
    return paused;
}

int is_playing(void)
{
    int playing;
    pthread_mutex_lock(&audio_mutex);
    playing = (g_player_state != PLAYER_STOPPED);
    pthread_mutex_unlock(&audio_mutex);
    return playing;
}