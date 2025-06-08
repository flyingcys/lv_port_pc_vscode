#ifdef PLATFORM_LINUX

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>

// 线程包装结构
typedef struct {
    pthread_t thread;
    void (*func)(void*);
    void *arg;
    char name[64];
} ThreadWrapper;

// 条件变量包装结构
typedef struct {
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} CondWrapper;

// 线程入口函数
static void* thread_entry(void *arg) {
    ThreadWrapper *wrapper = (ThreadWrapper*)arg;
    
    // 设置线程名称
    if (strlen(wrapper->name) > 0) {
        pthread_setname_np(wrapper->thread, wrapper->name);
    }
    
    // 调用用户函数
    wrapper->func(wrapper->arg);
    
    return NULL;
}

static void* linux_thread_create(void (*func)(void*), void *arg, const char *name) {
    if (!func) return NULL;
    
    ThreadWrapper *wrapper = malloc(sizeof(ThreadWrapper));
    if (!wrapper) return NULL;
    
    wrapper->func = func;
    wrapper->arg = arg;
    
    // 复制线程名称
    if (name) {
        strncpy(wrapper->name, name, sizeof(wrapper->name) - 1);
        wrapper->name[sizeof(wrapper->name) - 1] = '\0';
    } else {
        wrapper->name[0] = '\0';
    }
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    int ret = pthread_create(&wrapper->thread, &attr, thread_entry, wrapper);
    pthread_attr_destroy(&attr);
    
    if (ret != 0) {
        free(wrapper);
        return NULL;
    }
    
    return wrapper;
}

static void linux_thread_join(void *thread) {
    if (!thread) return;
    
    ThreadWrapper *wrapper = (ThreadWrapper*)thread;
    pthread_join(wrapper->thread, NULL);
    free(wrapper);
}

static void linux_thread_sleep(int ms) {
    if (ms <= 0) return;
    
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

static void* linux_thread_get_self(void) {
    return (void*)pthread_self();
}

static void* linux_mutex_create(void) {
    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    if (!mutex) return NULL;
    
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    
    int ret = pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    
    if (ret != 0) {
        free(mutex);
        return NULL;
    }
    
    return mutex;
}

static void linux_mutex_lock(void *mutex) {
    if (mutex) {
        pthread_mutex_lock((pthread_mutex_t*)mutex);
    }
}

static bool linux_mutex_try_lock(void *mutex) {
    if (!mutex) return false;
    return pthread_mutex_trylock((pthread_mutex_t*)mutex) == 0;
}

static void linux_mutex_unlock(void *mutex) {
    if (mutex) {
        pthread_mutex_unlock((pthread_mutex_t*)mutex);
    }
}

static void linux_mutex_destroy(void *mutex) {
    if (mutex) {
        pthread_mutex_destroy((pthread_mutex_t*)mutex);
        free(mutex);
    }
}

static void* linux_cond_create(void) {
    CondWrapper *wrapper = malloc(sizeof(CondWrapper));
    if (!wrapper) return NULL;
    
    int ret1 = pthread_cond_init(&wrapper->cond, NULL);
    int ret2 = pthread_mutex_init(&wrapper->mutex, NULL);
    
    if (ret1 != 0 || ret2 != 0) {
        if (ret1 == 0) pthread_cond_destroy(&wrapper->cond);
        if (ret2 == 0) pthread_mutex_destroy(&wrapper->mutex);
        free(wrapper);
        return NULL;
    }
    
    return wrapper;
}

static void linux_cond_wait(void *cond, void *mutex, int timeout_ms) {
    if (!cond) return;
    
    CondWrapper *wrapper = (CondWrapper*)cond;
    pthread_mutex_t *user_mutex = (pthread_mutex_t*)mutex;
    
    if (timeout_ms < 0) {
        // 无限等待
        pthread_cond_wait(&wrapper->cond, user_mutex);
    } else if (timeout_ms == 0) {
        // 不等待，直接返回
        return;
    } else {
        // 超时等待
        struct timespec ts;
        struct timeval tv;
        gettimeofday(&tv, NULL);
        
        ts.tv_sec = tv.tv_sec + timeout_ms / 1000;
        ts.tv_nsec = tv.tv_usec * 1000 + (timeout_ms % 1000) * 1000000;
        
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        
        pthread_cond_timedwait(&wrapper->cond, user_mutex, &ts);
    }
}

static void linux_cond_signal(void *cond) {
    if (cond) {
        CondWrapper *wrapper = (CondWrapper*)cond;
        pthread_cond_signal(&wrapper->cond);
    }
}

static void linux_cond_broadcast(void *cond) {
    if (cond) {
        CondWrapper *wrapper = (CondWrapper*)cond;
        pthread_cond_broadcast(&wrapper->cond);
    }
}

static void linux_cond_destroy(void *cond) {
    if (cond) {
        CondWrapper *wrapper = (CondWrapper*)cond;
        pthread_cond_destroy(&wrapper->cond);
        pthread_mutex_destroy(&wrapper->mutex);
        free(wrapper);
    }
}

// 导出函数指针
void* (*platform_thread_create_ptr)(void (*func)(void*), void *arg, const char *name) = linux_thread_create;
void (*platform_thread_join_ptr)(void *thread) = linux_thread_join;
void (*platform_thread_sleep_ptr)(int ms) = linux_thread_sleep;
void* (*platform_thread_get_self_ptr)(void) = linux_thread_get_self;

void* (*platform_mutex_create_ptr)(void) = linux_mutex_create;
void (*platform_mutex_lock_ptr)(void *mutex) = linux_mutex_lock;
bool (*platform_mutex_try_lock_ptr)(void *mutex) = linux_mutex_try_lock;
void (*platform_mutex_unlock_ptr)(void *mutex) = linux_mutex_unlock;
void (*platform_mutex_destroy_ptr)(void *mutex) = linux_mutex_destroy;

void* (*platform_cond_create_ptr)(void) = linux_cond_create;
void (*platform_cond_wait_ptr)(void *cond, void *mutex, int timeout_ms) = linux_cond_wait;
void (*platform_cond_signal_ptr)(void *cond) = linux_cond_signal;
void (*platform_cond_broadcast_ptr)(void *cond) = linux_cond_broadcast;
void (*platform_cond_destroy_ptr)(void *cond) = linux_cond_destroy;

#endif // PLATFORM_LINUX 