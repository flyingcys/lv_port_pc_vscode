#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// 环形缓冲区结构体
typedef struct RingBuffer RingBuffer;

/**
 * 创建环形缓冲区
 * @param capacity 缓冲区容量 (字节)
 * @return 环形缓冲区指针，失败返回NULL
 */
RingBuffer* rb_create(size_t capacity);

/**
 * 销毁环形缓冲区
 * @param rb 环形缓冲区指针
 */
void rb_destroy(RingBuffer *rb);

/**
 * 写入数据到环形缓冲区
 * @param rb 环形缓冲区指针
 * @param data 待写入数据
 * @param size 数据大小
 * @param timeout_ms 超时时间 (毫秒)，-1表示无限等待，0表示不等待
 * @return 实际写入的字节数
 */
size_t rb_write(RingBuffer *rb, const void *data, size_t size, int timeout_ms);

/**
 * 从环形缓冲区读取数据
 * @param rb 环形缓冲区指针
 * @param data 数据缓冲区
 * @param size 要读取的字节数
 * @param timeout_ms 超时时间 (毫秒)，-1表示无限等待，0表示不等待
 * @return 实际读取的字节数
 */
size_t rb_read(RingBuffer *rb, void *data, size_t size, int timeout_ms);

/**
 * 预览数据（不移除）
 * @param rb 环形缓冲区指针
 * @param data 数据缓冲区
 * @param size 要预览的字节数
 * @return 实际预览的字节数
 */
size_t rb_peek(RingBuffer *rb, void *data, size_t size);

/**
 * 跳过指定字节数的数据
 * @param rb 环形缓冲区指针
 * @param size 要跳过的字节数
 * @return 实际跳过的字节数
 */
size_t rb_skip(RingBuffer *rb, size_t size);

/**
 * 获取可读取的数据量
 * @param rb 环形缓冲区指针
 * @return 可读取的字节数
 */
size_t rb_get_used_size(RingBuffer *rb);

/**
 * 获取可写入的空间大小
 * @param rb 环形缓冲区指针
 * @return 可写入的字节数
 */
size_t rb_get_free_size(RingBuffer *rb);

/**
 * 获取缓冲区总容量
 * @param rb 环形缓冲区指针
 * @return 总容量字节数
 */
size_t rb_get_capacity(RingBuffer *rb);

/**
 * 判断缓冲区是否为空
 * @param rb 环形缓冲区指针
 * @return 为空返回true，否则返回false
 */
bool rb_is_empty(RingBuffer *rb);

/**
 * 判断缓冲区是否满
 * @param rb 环形缓冲区指针
 * @return 满返回true，否则返回false
 */
bool rb_is_full(RingBuffer *rb);

/**
 * 重置缓冲区（清空所有数据）
 * @param rb 环形缓冲区指针
 */
void rb_reset(RingBuffer *rb);

/**
 * 设置数据结束标志
 * @param rb 环形缓冲区指针
 * @param eof 是否到达数据结尾
 */
void rb_set_eof(RingBuffer *rb, bool eof);

/**
 * 获取数据结束标志
 * @param rb 环形缓冲区指针
 * @return 到达数据结尾返回true，否则返回false
 */
bool rb_is_eof(RingBuffer *rb);

/**
 * 获取缓冲区状态信息
 * @param rb 环形缓冲区指针
 * @param used_size 输出已使用大小
 * @param free_size 输出空闲大小
 * @param capacity 输出总容量
 */
void rb_get_status(RingBuffer *rb, size_t *used_size, size_t *free_size, size_t *capacity);

#ifdef __cplusplus
}
#endif

#endif // RINGBUFFER_H 