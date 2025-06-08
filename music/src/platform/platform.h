#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ==== 线程相关接口 ====
typedef void* (*PlatformThreadCreate)(void (*func)(void*), void *arg, const char *name);
typedef void (*PlatformThreadJoin)(void *thread);
typedef void (*PlatformThreadSleep)(int ms);
typedef void* (*PlatformThreadGetSelf)(void);

// ==== 互斥锁相关接口 ====
typedef void* (*PlatformMutexCreate)(void);
typedef void (*PlatformMutexLock)(void *mutex);
typedef bool (*PlatformMutexTryLock)(void *mutex);
typedef void (*PlatformMutexUnlock)(void *mutex);
typedef void (*PlatformMutexDestroy)(void *mutex);

// ==== 条件变量相关接口 ====
typedef void* (*PlatformCondCreate)(void);
typedef void (*PlatformCondWait)(void *cond, void *mutex, int timeout_ms);
typedef void (*PlatformCondSignal)(void *cond);
typedef void (*PlatformCondBroadcast)(void *cond);
typedef void (*PlatformCondDestroy)(void *cond);

// ==== 文件操作相关接口 ====
typedef void* (*PlatformFileOpen)(const char *path, const char *mode);
typedef int (*PlatformFileRead)(void *file, void *buffer, int size);
typedef int (*PlatformFileWrite)(void *file, const void *buffer, int size);
typedef int64_t (*PlatformFileSeek)(void *file, int64_t offset, int whence);
typedef int64_t (*PlatformFileTell)(void *file);
typedef int64_t (*PlatformFileSize)(void *file);
typedef bool (*PlatformFileEof)(void *file);
typedef void (*PlatformFileClose)(void *file);

// ==== 网络操作相关接口 ====
typedef void* (*PlatformNetOpen)(const char *url);
typedef int (*PlatformNetRead)(void *net, void *buffer, int size);
typedef int (*PlatformNetWrite)(void *net, const void *buffer, int size);
typedef bool (*PlatformNetIsConnected)(void *net);
typedef void (*PlatformNetClose)(void *net);

// ==== 时间相关接口 ====
typedef int64_t (*PlatformTimeGetMs)(void);
typedef void (*PlatformTimeDelay)(int ms);

// ==== 内存相关接口 ====
typedef void* (*PlatformMemAlloc)(size_t size);
typedef void* (*PlatformMemRealloc)(void *ptr, size_t size);
typedef void (*PlatformMemFree)(void *ptr);
typedef void* (*PlatformMemSet)(void *ptr, int value, size_t size);
typedef void* (*PlatformMemCopy)(void *dest, const void *src, size_t size);

// ==== 字符串相关接口 ====
typedef int (*PlatformStrLen)(const char *str);
typedef int (*PlatformStrCopy)(char *dest, const char *src, int dest_size);
typedef int (*PlatformStrCompare)(const char *str1, const char *str2);
typedef int (*PlatformStrCompareN)(const char *str1, const char *str2, int n);
typedef char* (*PlatformStrFind)(const char *str, const char *substr);

// ==== 日志相关接口 ====
typedef enum {
    PLATFORM_LOG_ERROR,
    PLATFORM_LOG_WARN,
    PLATFORM_LOG_INFO,
    PLATFORM_LOG_DEBUG
} PlatformLogLevel;

typedef void (*PlatformLogPrint)(PlatformLogLevel level, const char *tag, const char *format, ...);

// 平台API结构体
typedef struct {
    // 线程相关
    PlatformThreadCreate thread_create;
    PlatformThreadJoin thread_join;
    PlatformThreadSleep thread_sleep;
    PlatformThreadGetSelf thread_get_self;
    
    // 互斥锁相关
    PlatformMutexCreate mutex_create;
    PlatformMutexLock mutex_lock;
    PlatformMutexTryLock mutex_try_lock;
    PlatformMutexUnlock mutex_unlock;
    PlatformMutexDestroy mutex_destroy;
    
    // 条件变量相关
    PlatformCondCreate cond_create;
    PlatformCondWait cond_wait;
    PlatformCondSignal cond_signal;
    PlatformCondBroadcast cond_broadcast;
    PlatformCondDestroy cond_destroy;
    
    // 文件操作相关
    PlatformFileOpen file_open;
    PlatformFileRead file_read;
    PlatformFileWrite file_write;
    PlatformFileSeek file_seek;
    PlatformFileTell file_tell;
    PlatformFileSize file_size;
    PlatformFileEof file_eof;
    PlatformFileClose file_close;
    
    // 网络操作相关
    PlatformNetOpen net_open;
    PlatformNetRead net_read;
    PlatformNetWrite net_write;
    PlatformNetIsConnected net_is_connected;
    PlatformNetClose net_close;
    
    // 时间相关
    PlatformTimeGetMs time_get_ms;
    PlatformTimeDelay time_delay;
    
    // 内存相关
    PlatformMemAlloc mem_alloc;
    PlatformMemRealloc mem_realloc;
    PlatformMemFree mem_free;
    PlatformMemSet mem_set;
    PlatformMemCopy mem_copy;
    
    // 字符串相关
    PlatformStrLen str_len;
    PlatformStrCopy str_copy;
    PlatformStrCompare str_compare;
    PlatformStrCompareN str_compare_n;
    PlatformStrFind str_find;
    
    // 日志相关
    PlatformLogPrint log_print;
} PlatformAPI;

// ==== 平台接口函数 ====

/**
 * 初始化平台
 * @return 成功返回true，失败返回false
 */
bool platform_init(void);

/**
 * 获取平台API
 * @return 平台API结构体指针
 */
const PlatformAPI* platform_get_api(void);

/**
 * 平台清理
 */
void platform_cleanup(void);

/**
 * 获取平台名称
 * @return 平台名称字符串
 */
const char* platform_get_name(void);

// ==== 常用宏定义 ====

// 文件操作常量
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

// 日志宏
#define PLATFORM_LOG_E(tag, format, ...) \
    do { \
        const PlatformAPI *api = platform_get_api(); \
        if (api && api->log_print) { \
            api->log_print(PLATFORM_LOG_ERROR, tag, format, ##__VA_ARGS__); \
        } \
    } while(0)

#define PLATFORM_LOG_W(tag, format, ...) \
    do { \
        const PlatformAPI *api = platform_get_api(); \
        if (api && api->log_print) { \
            api->log_print(PLATFORM_LOG_WARN, tag, format, ##__VA_ARGS__); \
        } \
    } while(0)

#define PLATFORM_LOG_I(tag, format, ...) \
    do { \
        const PlatformAPI *api = platform_get_api(); \
        if (api && api->log_print) { \
            api->log_print(PLATFORM_LOG_INFO, tag, format, ##__VA_ARGS__); \
        } \
    } while(0)

#define PLATFORM_LOG_D(tag, format, ...) \
    do { \
        const PlatformAPI *api = platform_get_api(); \
        if (api && api->log_print) { \
            api->log_print(PLATFORM_LOG_DEBUG, tag, format, ##__VA_ARGS__); \
        } \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_H 