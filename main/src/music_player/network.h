#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <pthread.h>
#include "player.h"  // 包含播放器相关的结构体定义

// 网络流缓冲区操作函数
stream_buffer_t *init_stream_buffer(size_t capacity);
void free_stream_buffer(stream_buffer_t *buffer);
size_t stream_buffer_write(stream_buffer_t *buffer, const uint8_t *data, size_t bytes);
size_t stream_buffer_read(stream_buffer_t *buffer, uint8_t *data, size_t bytes);

// 网络下载相关函数
int download_audio(const char *url, const char *save_path);
void *download_thread_func(void *arg);

// 判断是否为网络URL
int is_url(const char *path);

#endif // NETWORK_H