#include "ringbuffer.h"
#include "platform.h"
#include <string.h>

#define TAG "RingBuffer"

// 环形缓冲区内部结构
struct RingBuffer {
    uint8_t *buffer;             // 缓冲区内存
    size_t capacity;             // 缓冲区总容量
    size_t size;                 // 当前存储数据量
    size_t read_pos;             // 读取位置
    size_t write_pos;            // 写入位置
    void *mutex;                 // 互斥锁
    void *read_cond;             // 读取条件变量
    void *write_cond;            // 写入条件变量
    bool is_eof;                 // 数据结束标志
};

RingBuffer* rb_create(size_t capacity) {
    const PlatformAPI *api = platform_get_api();
    if (!api) {
        return NULL;
    }
    
    RingBuffer *rb = api->mem_alloc(sizeof(RingBuffer));
    if (!rb) {
        return NULL;
    }
    
    // 初始化结构
    api->mem_set(rb, 0, sizeof(RingBuffer));
    rb->capacity = capacity;
    
    // 分配缓冲区内存
    rb->buffer = api->mem_alloc(capacity);
    if (!rb->buffer) {
        api->mem_free(rb);
        return NULL;
    }
    
    // 创建同步对象
    rb->mutex = api->mutex_create();
    rb->read_cond = api->cond_create();
    rb->write_cond = api->cond_create();
    
    if (!rb->mutex || !rb->read_cond || !rb->write_cond) {
        rb_destroy(rb);
        return NULL;
    }
    
    PLATFORM_LOG_D(TAG, "Created ring buffer with capacity: %zu", capacity);
    return rb;
}

void rb_destroy(RingBuffer *rb) {
    if (!rb) return;
    
    const PlatformAPI *api = platform_get_api();
    if (!api) return;
    
    // 唤醒所有等待的线程
    if (rb->read_cond) {
        api->cond_broadcast(rb->read_cond);
        api->cond_destroy(rb->read_cond);
    }
    
    if (rb->write_cond) {
        api->cond_broadcast(rb->write_cond);
        api->cond_destroy(rb->write_cond);
    }
    
    if (rb->mutex) {
        api->mutex_destroy(rb->mutex);
    }
    
    if (rb->buffer) {
        api->mem_free(rb->buffer);
    }
    
    api->mem_free(rb);
    PLATFORM_LOG_D(TAG, "Ring buffer destroyed");
}

size_t rb_write(RingBuffer *rb, const void *data, size_t size, int timeout_ms) {
    if (!rb || !data || size == 0) {
        return 0;
    }
    
    const PlatformAPI *api = platform_get_api();
    if (!api) return 0;
    
    api->mutex_lock(rb->mutex);
    
    size_t written = 0;
    const uint8_t *src = (const uint8_t *)data;
    int64_t start_time = api->time_get_ms();
    
    while (written < size) {
        // 检查可用空间
        size_t free_space = rb->capacity - rb->size;
        
        if (free_space == 0) {
            // 缓冲区已满，检查超时
            if (timeout_ms == 0) {
                break; // 非阻塞模式，直接退出
            }
            
            if (timeout_ms > 0) {
                int64_t elapsed = api->time_get_ms() - start_time;
                if (elapsed >= timeout_ms) {
                    break; // 超时
                }
                
                // 等待空间可用
                api->cond_wait(rb->write_cond, rb->mutex, timeout_ms - (int)elapsed);
            } else {
                // 无限等待
                api->cond_wait(rb->write_cond, rb->mutex, -1);
            }
            continue;
        }
        
        // 计算本次写入大小
        size_t to_write = size - written;
        if (to_write > free_space) {
            to_write = free_space;
        }
        
        // 分段写入
        size_t end_space = rb->capacity - rb->write_pos;
        if (to_write <= end_space) {
            // 一次性写入
            api->mem_copy(rb->buffer + rb->write_pos, src + written, to_write);
            rb->write_pos = (rb->write_pos + to_write) % rb->capacity;
        } else {
            // 分两段写入
            api->mem_copy(rb->buffer + rb->write_pos, src + written, end_space);
            api->mem_copy(rb->buffer, src + written + end_space, to_write - end_space);
            rb->write_pos = to_write - end_space;
        }
        
        rb->size += to_write;
        written += to_write;
        
        // 通知读取线程
        api->cond_signal(rb->read_cond);
    }
    
    api->mutex_unlock(rb->mutex);
    return written;
}

size_t rb_read(RingBuffer *rb, void *data, size_t size, int timeout_ms) {
    if (!rb || !data || size == 0) {
        return 0;
    }
    
    const PlatformAPI *api = platform_get_api();
    if (!api) return 0;
    
    api->mutex_lock(rb->mutex);
    
    size_t read_count = 0;
    uint8_t *dest = (uint8_t *)data;
    int64_t start_time = api->time_get_ms();
    
    while (read_count < size) {
        // 检查可读数据
        if (rb->size == 0) {
            // 没有数据可读
            if (rb->is_eof) {
                break; // 数据流结束
            }
            
            if (timeout_ms == 0) {
                break; // 非阻塞模式，直接退出
            }
            
            if (timeout_ms > 0) {
                int64_t elapsed = api->time_get_ms() - start_time;
                if (elapsed >= timeout_ms) {
                    break; // 超时
                }
                
                // 等待数据可用
                api->cond_wait(rb->read_cond, rb->mutex, timeout_ms - (int)elapsed);
            } else {
                // 无限等待
                api->cond_wait(rb->read_cond, rb->mutex, -1);
            }
            continue;
        }
        
        // 计算本次读取大小
        size_t to_read = size - read_count;
        if (to_read > rb->size) {
            to_read = rb->size;
        }
        
        // 分段读取
        size_t end_space = rb->capacity - rb->read_pos;
        if (to_read <= end_space) {
            // 一次性读取
            api->mem_copy(dest + read_count, rb->buffer + rb->read_pos, to_read);
            rb->read_pos = (rb->read_pos + to_read) % rb->capacity;
        } else {
            // 分两段读取
            api->mem_copy(dest + read_count, rb->buffer + rb->read_pos, end_space);
            api->mem_copy(dest + read_count + end_space, rb->buffer, to_read - end_space);
            rb->read_pos = to_read - end_space;
        }
        
        rb->size -= to_read;
        read_count += to_read;
        
        // 通知写入线程
        api->cond_signal(rb->write_cond);
    }
    
    api->mutex_unlock(rb->mutex);
    return read_count;
}

size_t rb_peek(RingBuffer *rb, void *data, size_t size) {
    if (!rb || !data || size == 0) {
        return 0;
    }
    
    const PlatformAPI *api = platform_get_api();
    if (!api) return 0;
    
    api->mutex_lock(rb->mutex);
    
    size_t to_peek = (size < rb->size) ? size : rb->size;
    uint8_t *dest = (uint8_t *)data;
    
    if (to_peek > 0) {
        size_t end_space = rb->capacity - rb->read_pos;
        if (to_peek <= end_space) {
            // 一次性读取
            api->mem_copy(dest, rb->buffer + rb->read_pos, to_peek);
        } else {
            // 分两段读取
            api->mem_copy(dest, rb->buffer + rb->read_pos, end_space);
            api->mem_copy(dest + end_space, rb->buffer, to_peek - end_space);
        }
    }
    
    api->mutex_unlock(rb->mutex);
    return to_peek;
}

size_t rb_skip(RingBuffer *rb, size_t size) {
    if (!rb || size == 0) {
        return 0;
    }
    
    const PlatformAPI *api = platform_get_api();
    if (!api) return 0;
    
    api->mutex_lock(rb->mutex);
    
    size_t to_skip = (size < rb->size) ? size : rb->size;
    
    if (to_skip > 0) {
        rb->read_pos = (rb->read_pos + to_skip) % rb->capacity;
        rb->size -= to_skip;
        
        // 通知写入线程
        api->cond_signal(rb->write_cond);
    }
    
    api->mutex_unlock(rb->mutex);
    return to_skip;
}

size_t rb_get_used_size(RingBuffer *rb) {
    if (!rb) return 0;
    
    const PlatformAPI *api = platform_get_api();
    if (!api) return 0;
    
    api->mutex_lock(rb->mutex);
    size_t used = rb->size;
    api->mutex_unlock(rb->mutex);
    
    return used;
}

size_t rb_get_free_size(RingBuffer *rb) {
    if (!rb) return 0;
    
    const PlatformAPI *api = platform_get_api();
    if (!api) return 0;
    
    api->mutex_lock(rb->mutex);
    size_t free_size = rb->capacity - rb->size;
    api->mutex_unlock(rb->mutex);
    
    return free_size;
}

size_t rb_get_capacity(RingBuffer *rb) {
    return rb ? rb->capacity : 0;
}

bool rb_is_empty(RingBuffer *rb) {
    if (!rb) return true;
    
    const PlatformAPI *api = platform_get_api();
    if (!api) return true;
    
    api->mutex_lock(rb->mutex);
    bool empty = (rb->size == 0);
    api->mutex_unlock(rb->mutex);
    
    return empty;
}

bool rb_is_full(RingBuffer *rb) {
    if (!rb) return false;
    
    const PlatformAPI *api = platform_get_api();
    if (!api) return false;
    
    api->mutex_lock(rb->mutex);
    bool full = (rb->size == rb->capacity);
    api->mutex_unlock(rb->mutex);
    
    return full;
}

void rb_reset(RingBuffer *rb) {
    if (!rb) return;
    
    const PlatformAPI *api = platform_get_api();
    if (!api) return;
    
    api->mutex_lock(rb->mutex);
    
    rb->size = 0;
    rb->read_pos = 0;
    rb->write_pos = 0;
    rb->is_eof = false;
    
    // 通知所有等待的线程
    api->cond_broadcast(rb->read_cond);
    api->cond_broadcast(rb->write_cond);
    
    api->mutex_unlock(rb->mutex);
    
    PLATFORM_LOG_D(TAG, "Ring buffer reset");
}

void rb_set_eof(RingBuffer *rb, bool eof) {
    if (!rb) return;
    
    const PlatformAPI *api = platform_get_api();
    if (!api) return;
    
    api->mutex_lock(rb->mutex);
    rb->is_eof = eof;
    
    if (eof) {
        // 通知所有等待读取的线程
        api->cond_broadcast(rb->read_cond);
        PLATFORM_LOG_D(TAG, "Ring buffer EOF set");
    }
    
    api->mutex_unlock(rb->mutex);
}

bool rb_is_eof(RingBuffer *rb) {
    if (!rb) return true;
    
    const PlatformAPI *api = platform_get_api();
    if (!api) return true;
    
    api->mutex_lock(rb->mutex);
    bool eof = rb->is_eof;
    api->mutex_unlock(rb->mutex);
    
    return eof;
}

void rb_get_status(RingBuffer *rb, size_t *used_size, size_t *free_size, size_t *capacity) {
    if (!rb) {
        if (used_size) *used_size = 0;
        if (free_size) *free_size = 0;
        if (capacity) *capacity = 0;
        return;
    }
    
    const PlatformAPI *api = platform_get_api();
    if (!api) return;
    
    api->mutex_lock(rb->mutex);
    
    if (used_size) *used_size = rb->size;
    if (free_size) *free_size = rb->capacity - rb->size;
    if (capacity) *capacity = rb->capacity;
    
    api->mutex_unlock(rb->mutex);
} 