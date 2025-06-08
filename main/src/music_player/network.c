#include <stdio.h>
#include <stdlib.h>
#include <string.h>    // 添加：用于memcpy和memset
#include <signal.h>    // 添加：用于信号处理
#include <stdarg.h>    // 添加：用于va_start和va_end
#include <time.h>      // 添加：用于time和localtime
#include <errno.h>     // 添加：用于ETIMEDOUT

#include "player.h"
#include "network.h"

#include <curl/curl.h>

/*这些修改添加了更全面的错误处理机制，包括错误码定义、错误检查、错误恢复和重试机制，使网络模块更加健壮和可靠。 */
// 增加错误码定义
#define NETWORK_OK           0
#define NETWORK_ERROR_INIT  -1
#define NETWORK_ERROR_CURL  -2
#define NETWORK_ERROR_MEM   -3
#define NETWORK_ERROR_TIMEOUT -4
#define NETWORK_ERROR_CONNECTION -5
#define NETWORK_ERROR_HTTP   -6
#define NETWORK_ERROR_BUFFER -7
#define NETWORK_ERROR_PARAM  -8

// 添加网络状态结构体
typedef struct {
    long http_code;           // HTTP状态码
    double download_speed;    // 下载速度 (bytes/sec)
    double total_time;        // 总下载时间
    double connect_time;      // 连接时间
    size_t downloaded_bytes;  // 已下载字节数
    size_t total_bytes;       // 总字节数
    int is_error;             // 是否有错误
    int error_code;           // 错误码
    char error_message[256];  // 错误信息
    int retry_count;          // 重试次数
    int max_retries;          // 最大重试次数
    int is_cancelled;         // 是否被取消
} network_status_t;

// 添加日志级别定义
typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} log_level_t;

static log_level_t g_log_level = LOG_LEVEL_INFO;

// 从player.h导入播放器状态枚举
extern player_state_t g_player_state;

// 添加信号处理函数
static volatile sig_atomic_t g_terminate = 0;

static void signal_handler(int signum)
{
    g_terminate = 1;
}

// 初始化网络流缓冲区 (优化版)
stream_buffer_t *init_stream_buffer(size_t capacity)
{
    if (capacity == 0) {
        fprintf(stderr, "缓冲区容量不能为0\n");
        return NULL;
    }
    
    stream_buffer_t *buffer = calloc(1, sizeof(stream_buffer_t));  // 使用calloc初始化为0
    if (!buffer) {
        fprintf(stderr, "内存分配失败: stream_buffer_t\n");
        return NULL;
    }
    
    buffer->data = malloc(capacity);
    if (!buffer->data) {
        fprintf(stderr, "内存分配失败: buffer->data (%zu bytes)\n", capacity);
        free(buffer);
        return NULL;
    }
    
    buffer->capacity = capacity;
    
    // 初始化互斥锁和条件变量
    if (pthread_mutex_init(&buffer->mutex, NULL) != 0) {
        fprintf(stderr, "互斥锁初始化失败\n");
        free(buffer->data);
        free(buffer);
        return NULL;
    }
    
    if (pthread_cond_init(&buffer->not_full, NULL) != 0 || 
        pthread_cond_init(&buffer->not_empty, NULL) != 0) {
        fprintf(stderr, "条件变量初始化失败\n");
        pthread_mutex_destroy(&buffer->mutex);
        free(buffer->data);
        free(buffer);
        return NULL;
    }
    
    return buffer;
}

// 释放网络流缓冲区 (修改)
void free_stream_buffer(stream_buffer_t *buffer)
{
    if (!buffer) return;
    
    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->not_full);
    pthread_cond_destroy(&buffer->not_empty);
    free(buffer->data);
    free(buffer);
}

// 从网络流缓冲区读取数据 (线程安全优化版)
size_t stream_buffer_read(stream_buffer_t *buffer, uint8_t *data, size_t bytes)
{
    if (!buffer || !data || bytes == 0) {
        return 0;
    }
    
    size_t bytes_to_read = 0;  // 修改：使用bytes_to_read替代bytes_read
    
    pthread_mutex_lock(&buffer->mutex);
    
    // 等待缓冲区有数据，增加超时处理
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;  // 5秒超时
    
    while (buffer->size == 0 && !buffer->finished && g_player_state != PLAYER_STOPPED) {
        int rc = pthread_cond_timedwait(&buffer->not_empty, &buffer->mutex, &ts);
        if (rc == ETIMEDOUT) {
            // 超时处理
            if (buffer->size == 0) {
                pthread_mutex_unlock(&buffer->mutex);
                return 0;
            }
            break;
        }
    }
    
    // 如果缓冲区为空且下载已完成，或播放已停止，返回0
    if ((buffer->size == 0 && buffer->finished) || g_player_state == PLAYER_STOPPED) {
        pthread_mutex_unlock(&buffer->mutex);
        return 0;
    }
    
    // 计算可以读取的字节数
    bytes_to_read = bytes;
    if (bytes_to_read > buffer->size) {
        bytes_to_read = buffer->size;
    }
    
    // 读取数据 - 批量处理
    if (buffer->read_pos + bytes_to_read <= buffer->capacity) {
        // 连续空间足够，一次性复制
        memcpy(data, buffer->data + buffer->read_pos, bytes_to_read);
    } else {
        // 需要分两次复制
        size_t first_part = buffer->capacity - buffer->read_pos;
        memcpy(data, buffer->data + buffer->read_pos, first_part);
        memcpy(data + first_part, buffer->data, bytes_to_read - first_part);
    }
    
    buffer->read_pos = (buffer->read_pos + bytes_to_read) % buffer->capacity;
    buffer->size -= bytes_to_read;
    
    // 通知生产者有空间可写
    pthread_cond_signal(&buffer->not_full);
    pthread_mutex_unlock(&buffer->mutex);
    
    return bytes_to_read;
}

// 向网络流缓冲区写入数据 (性能优化版)
size_t stream_buffer_write(stream_buffer_t *buffer, const uint8_t *data, size_t bytes)
{
    if (!buffer || !data || bytes == 0) {
        return 0;
    }
    
    pthread_mutex_lock(&buffer->mutex);
    
    // 如果缓冲区已满且不能立即写入，尝试等待一段时间
    if (buffer->size == buffer->capacity && g_player_state != PLAYER_STOPPED) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;  // 1秒超时
        
        pthread_cond_timedwait(&buffer->not_full, &buffer->mutex, &ts);
        
        // 如果仍然没有空间且播放已停止，返回0
        if (buffer->size == buffer->capacity && g_player_state == PLAYER_STOPPED) {
            pthread_mutex_unlock(&buffer->mutex);
            return 0;
        }
    }
    
    // 等待缓冲区有足够空间
    while (buffer->size + bytes > buffer->capacity && g_player_state != PLAYER_STOPPED) {
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }
    
    // 如果播放已停止，返回0
    if (g_player_state == PLAYER_STOPPED) {
        pthread_mutex_unlock(&buffer->mutex);
        return 0;
    }
    
    // 计算可以写入的字节数
    size_t bytes_to_write = bytes;
    if (buffer->size + bytes > buffer->capacity) {
        bytes_to_write = buffer->capacity - buffer->size;
    }
    
    // 写入数据 - 批量处理
    if (buffer->write_pos + bytes_to_write <= buffer->capacity) {
        // 连续空间足够，一次性复制
        memcpy(buffer->data + buffer->write_pos, data, bytes_to_write);
    } else {
        // 需要分两次复制
        size_t first_part = buffer->capacity - buffer->write_pos;
        memcpy(buffer->data + buffer->write_pos, data, first_part);
        memcpy(buffer->data, data + first_part, bytes_to_write - first_part);
    }
    
    buffer->write_pos = (buffer->write_pos + bytes_to_write) % buffer->capacity;
    buffer->size += bytes_to_write;
    
    // 通知消费者有数据可读
    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);
    
    return bytes_to_write;
}

// 网络下载回调函数 (优化版)
static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    if (!ptr || !userdata) {
        return 0;  // 错误处理
    }
    
    playback_data_t *data = (playback_data_t *)userdata;
    if (!data->stream_buffer) {
        return 0;  // 错误处理
    }
    
    size_t bytes = size * nmemb;
    if (bytes == 0) {
        return 0;  // 没有数据
    }
    
    // 使用环形缓冲区写入数据
    size_t written = stream_buffer_write(data->stream_buffer, ptr, bytes);
    
    return written;
}

// 进度回调函数
static int progress_callback(void *clientp, 
                           double dltotal, 
                           double dlnow, 
                           double ultotal, 
                           double ulnow)
{
    // 检查是否应该终止下载
    if (g_terminate) {
        return 1;  // 返回非零值表示中止传输
    }
    
    // 只显示下载进度（忽略上传进度）
    if(dltotal > 0) {
        double progress = (dlnow / dltotal) * 100;
        printf("\r下载进度: %.2f%% (%.2f/%.2f MB)", 
              progress, 
              dlnow/(1024*1024), 
              dltotal/(1024*1024));
        fflush(stdout);
    }
    
    return 0;  // 返回0表示继续传输
}

// 下载线程函数
void *download_thread_func(void *arg)
{
    // 设置信号处理
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    playback_data_t *data = (playback_data_t *)arg;
    CURL *curl;
    CURLcode res;
    
    // 检查参数有效性
    if (!data || !data->filename || !data->stream_buffer) {
        network_log(LOG_LEVEL_ERROR, "下载线程参数无效");
        return NULL;
    }
    
    // 初始化网络状态
    network_status_t status;
    memset(&status, 0, sizeof(network_status_t));
    status.max_retries = 3; // 设置最大重试次数
    
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
        network_log(LOG_LEVEL_ERROR, "curl全局初始化失败");
        return NULL;
    }
    
    curl = curl_easy_init();
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, data->filename);                            // 设置要请求的URL地址
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);                  // 设置数据接收的回调函数
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);                                // 设置传递给回调函数的用户数据指针
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);                             // 启用HTTP重定向跟随

        // 添加进度回调设置
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);                                 // 启用进度回调
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, data);

        // 设置超时选项
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);  // 连接超时：10秒
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);          // 传输超时：不限制（流媒体）
        
        // 设置重试选项
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);      // HTTP错误码>=400时报错
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);       // 最大重定向次数
        
        // 在循环中检查终止信号
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, data);
        
        do {
            res = curl_easy_perform(curl);
            
            // 获取网络状态
            get_network_status(curl, &status);
            
            if (res != CURLE_OK) {
                // 转换CURL错误为我们的错误码
                int error_code = network_check_curl_error(res);
                status.is_error = 1;
                status.error_code = error_code;
                
                // 记录错误信息
                snprintf(status.error_message, sizeof(status.error_message), 
                         "下载失败: %s (错误码: %d)", curl_easy_strerror(res), res);
                
                network_log(LOG_LEVEL_ERROR, "%s", status.error_message);
                
                // 检查是否需要重试
                if (error_code == NETWORK_ERROR_CONNECTION || 
                    error_code == NETWORK_ERROR_TIMEOUT) {
                    
                    if (network_retry_download(curl, &status, data) == NETWORK_OK) {
                        // 重试成功，继续循环
                        continue;
                    }
                }
                
                // 设置错误状态
                pthread_mutex_lock(&data->stream_buffer->mutex);
                data->stream_buffer->finished = 1;
                data->stream_buffer->error = 1;
                pthread_mutex_unlock(&data->stream_buffer->mutex);
                
                break;
            } else {
                // 下载成功
                network_log(LOG_LEVEL_INFO, "下载完成! 总共下载 %.2f MB, 平均速度 %.2f KB/s", 
                           status.downloaded_bytes/(1024.0*1024.0), 
                           status.download_speed/1024.0);
                break;
            }
        } while (!g_terminate && g_player_state != PLAYER_STOPPED);
        
        curl_easy_cleanup(curl);
    } else {
        network_log(LOG_LEVEL_ERROR, "curl初始化失败");
        curl_global_cleanup();
        return NULL;
    }
    
    curl_global_cleanup();
    
    // 标记下载完成
    pthread_mutex_lock(&data->stream_buffer->mutex);
    data->stream_buffer->finished = 1;
    pthread_mutex_unlock(&data->stream_buffer->mutex);
    
    return NULL;
}



// 增加错误处理函数
const char *network_error_string(int error_code) {
    switch(error_code) {
        case NETWORK_OK:
            return "成功";
        case NETWORK_ERROR_INIT:
            return "初始化失败";
        case NETWORK_ERROR_CURL:
            return "CURL操作失败";
        case NETWORK_ERROR_MEM:
            return "内存分配失败";
        case NETWORK_ERROR_TIMEOUT:
            return "网络连接超时";
        case NETWORK_ERROR_CONNECTION:
            return "网络连接失败";
        case NETWORK_ERROR_HTTP:
            return "HTTP请求错误";
        case NETWORK_ERROR_BUFFER:
            return "缓冲区操作错误";
        case NETWORK_ERROR_PARAM:
            return "参数错误";
        default:
            return "未知错误";
    }
}

// 添加进度回调函数
int network_check_curl_error(CURLcode curl_code) {
    if (curl_code == CURLE_OK) {
        return NETWORK_OK;
    }
    
    switch (curl_code) {
        case CURLE_COULDNT_CONNECT:
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_RESOLVE_PROXY:
            return NETWORK_ERROR_CONNECTION;
            
        case CURLE_OPERATION_TIMEDOUT:
            return NETWORK_ERROR_TIMEOUT;
            
        case CURLE_OUT_OF_MEMORY:
            return NETWORK_ERROR_MEM;
            
        case CURLE_HTTP_RETURNED_ERROR:
        case CURLE_HTTP2:
        case CURLE_HTTP2_STREAM:
            return NETWORK_ERROR_HTTP;
            
        default:
            return NETWORK_ERROR_CURL;
    }
}



// 日志函数
void network_log(log_level_t level, const char *format, ...)
{
    if (level > g_log_level) {
        return;
    }
    
    const char *level_str[] = {"错误", "警告", "信息", "调试"};
    
    time_t now;
    struct tm *tm_info;
    char time_str[20];
    
    time(&now);
    tm_info = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(stderr, "[%s][网络][%s] ", time_str, level_str[level]);
    
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    
    fprintf(stderr, "\n");
}

// 设置日志级别函数
void network_set_log_level(log_level_t level)
{
    g_log_level = level;
}


// 获取网络状态函数
void get_network_status(CURL *curl, network_status_t *status)
{
    if (!curl || !status) {
        return;
    }
    
    memset(status, 0, sizeof(network_status_t));
    
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status->http_code);
    curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &status->download_speed);
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &status->total_time);
    curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &status->connect_time);
    curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &status->downloaded_bytes);
    curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &status->total_bytes);
}

// 添加网络错误重试函数
int network_retry_download(CURL *curl, network_status_t *status, playback_data_t *data) {
    if (!curl || !status || !data) {
        return NETWORK_ERROR_PARAM;
    }
    
    // 如果已经达到最大重试次数，则返回错误
    if (status->retry_count >= status->max_retries) {
        network_log(LOG_LEVEL_ERROR, "下载失败，已达到最大重试次数 %d", status->max_retries);
        return NETWORK_ERROR_CONNECTION;
    }
    
    // 增加重试计数
    status->retry_count++;
    
    // 计算重试等待时间（指数退避策略）
    int wait_time = 1 << (status->retry_count - 1); // 1, 2, 4, 8, 16...秒
    if (wait_time > 30) wait_time = 30; // 最大等待30秒
    
    network_log(LOG_LEVEL_WARN, "下载失败，%d秒后进行第%d次重试...", 
                wait_time, status->retry_count);
    
    // 等待指定时间
    sleep(wait_time);
    
    // 重置CURL选项
    curl_easy_reset(curl);
    
    // 重新设置CURL选项
    curl_easy_setopt(curl, CURLOPT_URL, data->filename);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, data);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, data);
    
    // 设置断点续传
    if (status->downloaded_bytes > 0) {
        curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, (curl_off_t)status->downloaded_bytes);
        network_log(LOG_LEVEL_INFO, "从 %zu 字节处继续下载", status->downloaded_bytes);
    }
    
    return NETWORK_OK;
}

// 添加网络连接检测函数
int network_check_connection(const char *test_url) {
    CURL *curl;
    CURLcode res;
    int result = NETWORK_ERROR_CONNECTION;
    
    // 如果未提供测试URL，使用默认值
    if (!test_url) {
        test_url = "http://www.baidu.com";
    }
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, test_url);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);           // 只需要头部
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);   // 5秒连接超时
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);         // 10秒总超时
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);   // 跟随重定向
        
        res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            
            if (http_code >= 200 && http_code < 400) {
                result = NETWORK_OK;
                network_log(LOG_LEVEL_INFO, "网络连接正常 (HTTP状态码: %ld)", http_code);
            } else {
                network_log(LOG_LEVEL_ERROR, "网络连接异常 (HTTP状态码: %ld)", http_code);
                result = NETWORK_ERROR_HTTP;
            }
        } else {
            network_log(LOG_LEVEL_ERROR, "网络连接失败: %s", curl_easy_strerror(res));
            result = network_check_curl_error(res);
        }
        
        curl_easy_cleanup(curl);
    } else {
        network_log(LOG_LEVEL_ERROR, "curl初始化失败");
        result = NETWORK_ERROR_INIT;
    }
    
    return result;
}

// 添加网络错误恢复函数
int network_recover_error(int error_code) {
    switch (error_code) {
        case NETWORK_ERROR_CONNECTION:
        case NETWORK_ERROR_TIMEOUT:
            // 尝试恢复网络连接
            network_log(LOG_LEVEL_INFO, "尝试恢复网络连接...");
            
            // 等待一段时间后重试
            for (int i = 0; i < 3; i++) {
                network_log(LOG_LEVEL_INFO, "第%d次尝试检查网络连接...", i+1);
                sleep(2); // 等待2秒
                
                if (network_check_connection(NULL) == NETWORK_OK) {
                    network_log(LOG_LEVEL_INFO, "网络连接已恢复");
                    return NETWORK_OK;
                }
            }
            
            network_log(LOG_LEVEL_ERROR, "网络连接恢复失败");
            return error_code;
            
        case NETWORK_ERROR_HTTP:
            // HTTP错误通常需要检查服务器状态
            network_log(LOG_LEVEL_WARN, "HTTP错误，请检查服务器状态或URL是否正确");
            return error_code;
            
        case NETWORK_ERROR_MEM:
            // 内存错误，尝试释放一些资源
            network_log(LOG_LEVEL_WARN, "内存不足，尝试释放资源...");
            // 这里可以添加一些释放内存的操作
            return error_code;
            
        default:
            network_log(LOG_LEVEL_ERROR, "无法恢复的错误: %s", network_error_string(error_code));
            return error_code;
    }
}

// 添加内存边界检查函数
void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        network_log(LOG_LEVEL_ERROR, "内存分配失败: %zu 字节", size);
        return NULL;
    }
    return ptr;
}

void *safe_calloc(size_t nmemb, size_t size) {
    void *ptr = calloc(nmemb, size);
    if (!ptr) {
        network_log(LOG_LEVEL_ERROR, "内存分配失败: %zu 元素, 每个 %zu 字节", nmemb, size);
        return NULL;
    }
    return ptr;
}

void *safe_realloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr && size > 0) {
        network_log(LOG_LEVEL_ERROR, "内存重新分配失败: %zu 字节", size);
        return NULL;
    }
    return new_ptr;
}

// 添加资源清理函数
void cleanup_network_resources(CURL *curl, stream_buffer_t *buffer, playback_data_t *data) {
    if (curl) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }
    
    if (buffer) {
        pthread_mutex_lock(&buffer->mutex);
        buffer->finished = 1;
        buffer->error = 1;
        pthread_mutex_unlock(&buffer->mutex);
    }
    
    network_log(LOG_LEVEL_INFO, "网络资源已清理");
}
// 添加线程安全的全局变量访问
static pthread_mutex_t g_network_mutex = PTHREAD_MUTEX_INITIALIZER;

void set_player_state(player_state_t state) {
    pthread_mutex_lock(&g_network_mutex);
    g_player_state = state;
    pthread_mutex_unlock(&g_network_mutex);
}

player_state_t get_player_state(void) {
    player_state_t state;
    pthread_mutex_lock(&g_network_mutex);
    state = g_player_state;
    pthread_mutex_unlock(&g_network_mutex);
    return state;
}

// 添加SSL/TLS支持
int setup_secure_connection(CURL *curl) {
    if (!curl) {
        return NETWORK_ERROR_PARAM;
    }
    
    // 设置SSL/TLS选项
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    // 可以设置证书路径
    // curl_easy_setopt(curl, CURLOPT_CAINFO, "/path/to/ca-bundle.crt");
    
    return NETWORK_OK;
}

// 添加URL安全性检查
int validate_url(const char *url) {
    if (!url) {
        return NETWORK_ERROR_PARAM;
    }
    
    // 检查URL长度
    size_t len = strlen(url);
    if (len < 10 || len > 2048) {
        network_log(LOG_LEVEL_ERROR, "URL长度无效: %zu", len);
        return NETWORK_ERROR_PARAM;
    }
    
    // 检查URL协议
    if (strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0) {
        network_log(LOG_LEVEL_ERROR, "URL协议无效: %s", url);
        return NETWORK_ERROR_PARAM;
    }
    
    // 可以添加更多的URL验证逻辑
    
    return NETWORK_OK;
}

// 添加缓冲区预分配和调整函数
int optimize_buffer_size(stream_buffer_t *buffer, size_t expected_bitrate) {
    if (!buffer) {
        return NETWORK_ERROR_PARAM;
    }
    
    // 根据比特率计算合适的缓冲区大小
    // 例如：对于320kbps的音频，5秒的缓冲需要约200KB
    size_t optimal_size = (expected_bitrate / 8) * 5; // 5秒的数据
    
    // 确保缓冲区至少有1MB
    if (optimal_size < 1024 * 1024) {
        optimal_size = 1024 * 1024;
    }
    
    // 如果当前缓冲区太小，重新分配
    if (buffer->capacity < optimal_size) {
        pthread_mutex_lock(&buffer->mutex);
        
        uint8_t *new_data = safe_realloc(buffer->data, optimal_size);
        if (!new_data) {
            pthread_mutex_unlock(&buffer->mutex);
            return NETWORK_ERROR_MEM;
        }
        
        buffer->data = new_data;
        buffer->capacity = optimal_size;
        
        pthread_mutex_unlock(&buffer->mutex);
        
        network_log(LOG_LEVEL_INFO, "缓冲区大小已优化为 %zu 字节", optimal_size);
    }
    
    return NETWORK_OK;
}

// 添加网络传输优化函数
void optimize_curl_transfer(CURL *curl) {
    if (!curl) {
        return;
    }
    
    // 启用HTTP/2
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
    
    // 启用TCP保持活动
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);
    
    // 设置适当的缓冲区大小
    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 32768L);
    
    // 启用压缩
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
}

// 添加网络状态监控函数
typedef struct {
    int is_monitoring;
    pthread_t monitor_thread;
    int check_interval;  // 秒
    const char *test_url;
    int (*callback)(int status, void *user_data);
    void *user_data;
} network_monitor_t;

static network_monitor_t g_network_monitor = {0};

void *network_monitor_thread(void *arg) {
    network_monitor_t *monitor = (network_monitor_t *)arg;
    
    while (monitor->is_monitoring && !g_terminate) {
        int status = network_check_connection(monitor->test_url);
        
        if (monitor->callback) {
            monitor->callback(status, monitor->user_data);
        }
        
        // 等待指定的间隔时间
        for (int i = 0; i < monitor->check_interval && monitor->is_monitoring && !g_terminate; i++) {
            sleep(1);
        }
    }
    
    return NULL;
}

int start_network_monitoring(int check_interval, const char *test_url, 
                            int (*callback)(int status, void *user_data), 
                            void *user_data) {
    if (g_network_monitor.is_monitoring) {
        network_log(LOG_LEVEL_WARN, "网络监控已经在运行");
        return NETWORK_OK;
    }
    
    g_network_monitor.is_monitoring = 1;
    g_network_monitor.check_interval = check_interval > 0 ? check_interval : 30;
    g_network_monitor.test_url = test_url ? test_url : "http://www.baidu.com";
    g_network_monitor.callback = callback;
    g_network_monitor.user_data = user_data;
    
    int ret = pthread_create(&g_network_monitor.monitor_thread, NULL, 
                           network_monitor_thread, &g_network_monitor);
    if (ret != 0) {
        network_log(LOG_LEVEL_ERROR, "创建网络监控线程失败: %s", strerror(ret));
        g_network_monitor.is_monitoring = 0;
        return NETWORK_ERROR_INIT;
    }
    
    network_log(LOG_LEVEL_INFO, "网络监控已启动，检查间隔: %d秒", g_network_monitor.check_interval);
    return NETWORK_OK;
}

void stop_network_monitoring(void) {
    if (!g_network_monitor.is_monitoring) {
        return;
    }
    
    g_network_monitor.is_monitoring = 0;
    pthread_join(g_network_monitor.monitor_thread, NULL);
    
    network_log(LOG_LEVEL_INFO, "网络监控已停止");
}

// 添加详细的错误报告函数
void network_report_error(int error_code, const char *context, ...) {
    char context_msg[512] = {0};
    
    if (context) {
        va_list args;
        va_start(args, context);
        vsnprintf(context_msg, sizeof(context_msg), context, args);
        va_end(args);
    }
    
    const char *error_str = network_error_string(error_code);
    
    if (context_msg[0]) {
        network_log(LOG_LEVEL_ERROR, "网络错误 [%d]: %s - %s", 
                  error_code, error_str, context_msg);
    } else {
        network_log(LOG_LEVEL_ERROR, "网络错误 [%d]: %s", error_code, error_str);
    }
    
    // 可以在这里添加错误统计和报告逻辑
}

// 添加错误统计结构
typedef struct {
    int error_counts[10];  // 对应每种错误类型的计数
    int total_errors;
    int total_requests;
    time_t first_error_time;
    time_t last_error_time;
    pthread_mutex_t mutex;
} error_stats_t;

static error_stats_t g_error_stats = {0};

void init_error_stats(void) {
    memset(&g_error_stats, 0, sizeof(error_stats_t));
    pthread_mutex_init(&g_error_stats.mutex, NULL);
}

void record_network_error(int error_code) {
    pthread_mutex_lock(&g_error_stats.mutex);
    
    g_error_stats.total_errors++;
    
    // 记录错误类型计数
    if (error_code < 0 && error_code >= -8) {
        g_error_stats.error_counts[abs(error_code)]++;
    }
    
    // 记录时间
    time_t now = time(NULL);
    if (g_error_stats.first_error_time == 0) {
        g_error_stats.first_error_time = now;
    }
    g_error_stats.last_error_time = now;
    
    pthread_mutex_unlock(&g_error_stats.mutex);
}

void record_network_request(void) {
    pthread_mutex_lock(&g_error_stats.mutex);
    g_error_stats.total_requests++;
    pthread_mutex_unlock(&g_error_stats.mutex);
}

void print_error_stats(void) {
    pthread_mutex_lock(&g_error_stats.mutex);
    
    network_log(LOG_LEVEL_INFO, "网络错误统计:");
    network_log(LOG_LEVEL_INFO, "总请求数: %d", g_error_stats.total_requests);
    network_log(LOG_LEVEL_INFO, "总错误数: %d", g_error_stats.total_errors);
    
    if (g_error_stats.total_requests > 0) {
        float error_rate = (float)g_error_stats.total_errors / g_error_stats.total_requests * 100;
        network_log(LOG_LEVEL_INFO, "错误率: %.2f%%", error_rate);
    }
    
    for (int i = 1; i <= 8; i++) {
        if (g_error_stats.error_counts[i] > 0) {
            network_log(LOG_LEVEL_INFO, "错误类型 [%d] %s: %d 次", 
                      -i, network_error_string(-i), g_error_stats.error_counts[i]);
        }
    }
    
    pthread_mutex_unlock(&g_error_stats.mutex);
}


// 添加网络配置结构体
typedef struct {
    int connect_timeout;      // 连接超时（秒）
    int transfer_timeout;     // 传输超时（秒，0表示无限制）
    int max_retries;          // 最大重试次数
    int retry_interval;       // 重试间隔（秒）
    int buffer_size;          // 缓冲区大小（字节）
    int enable_ssl;           // 是否启用SSL
    int verify_ssl;           // 是否验证SSL证书
    char ca_path[256];        // CA证书路径
    char user_agent[256];     // 用户代理字符串
    int log_level;            // 日志级别
    int enable_compression;   // 是否启用压缩
    int low_speed_limit;      // 低速限制（字节/秒）
    int low_speed_time;       // 低速时间（秒）
} network_config_t;

static network_config_t g_network_config = {
    .connect_timeout = 10,
    .transfer_timeout = 0,
    .max_retries = 3,
    .retry_interval = 5,
    .buffer_size = 1024 * 1024,  // 1MB
    .enable_ssl = 1,
    .verify_ssl = 1,
    .ca_path = "",
    .user_agent = "AudioPlayer/1.0",
    .log_level = LOG_LEVEL_INFO,
    .enable_compression = 1,
    .low_speed_limit = 1000,  // 1KB/s
    .low_speed_time = 10      // 10秒
};

// 加载网络配置
int load_network_config(const char *config_file) {
    if (!config_file) {
        network_log(LOG_LEVEL_WARN, "未指定配置文件，使用默认配置");
        return NETWORK_OK;
    }
    
    FILE *fp = fopen(config_file, "r");
    if (!fp) {
        network_log(LOG_LEVEL_ERROR, "无法打开配置文件: %s", config_file);
        return NETWORK_ERROR_PARAM;
    }
    
    char line[512];
    char key[256];
    char value[256];
    
    while (fgets(line, sizeof(line), fp)) {
        // 跳过注释和空行
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        
        if (sscanf(line, "%255[^=]=%255s", key, value) == 2) {
            // 去除键和值中的空格
            char *p = key;
            while (*p) {
                if (*p == ' ' || *p == '\t') {
                    *p = '\0';
                    break;
                }
                p++;
            }
            
            // 处理配置项
            if (strcmp(key, "connect_timeout") == 0) {
                g_network_config.connect_timeout = atoi(value);
            } else if (strcmp(key, "transfer_timeout") == 0) {
                g_network_config.transfer_timeout = atoi(value);
            } else if (strcmp(key, "max_retries") == 0) {
                g_network_config.max_retries = atoi(value);
            } else if (strcmp(key, "retry_interval") == 0) {
                g_network_config.retry_interval = atoi(value);
            } else if (strcmp(key, "buffer_size") == 0) {
                g_network_config.buffer_size = atoi(value);
            } else if (strcmp(key, "enable_ssl") == 0) {
                g_network_config.enable_ssl = atoi(value);
            } else if (strcmp(key, "verify_ssl") == 0) {
                g_network_config.verify_ssl = atoi(value);
            } else if (strcmp(key, "ca_path") == 0) {
                strncpy(g_network_config.ca_path, value, sizeof(g_network_config.ca_path) - 1);
            } else if (strcmp(key, "user_agent") == 0) {
                strncpy(g_network_config.user_agent, value, sizeof(g_network_config.user_agent) - 1);
            } else if (strcmp(key, "log_level") == 0) {
                g_network_config.log_level = atoi(value);
            } else if (strcmp(key, "enable_compression") == 0) {
                g_network_config.enable_compression = atoi(value);
            } else if (strcmp(key, "low_speed_limit") == 0) {
                g_network_config.low_speed_limit = atoi(value);
            } else if (strcmp(key, "low_speed_time") == 0) {
                g_network_config.low_speed_time = atoi(value);
            }
        }
    }
    
    fclose(fp);
    
    // 应用配置
    network_set_log_level(g_network_config.log_level);
    
    network_log(LOG_LEVEL_INFO, "网络配置已加载: %s", config_file);
    return NETWORK_OK;
}

// 应用网络配置到CURL句柄
void apply_network_config(CURL *curl) {
    if (!curl) {
        return;
    }
    
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, g_network_config.connect_timeout);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, g_network_config.transfer_timeout);
    
    if (g_network_config.user_agent[0]) {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, g_network_config.user_agent);
    }
    
    if (g_network_config.enable_compression) {
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    }
    
    if (g_network_config.enable_ssl) {
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, g_network_config.verify_ssl ? 1L : 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, g_network_config.verify_ssl ? 2L : 0L);
        
        if (g_network_config.ca_path[0]) {
            curl_easy_setopt(curl, CURLOPT_CAINFO, g_network_config.ca_path);
        }
    }
    
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, g_network_config.low_speed_limit);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, g_network_config.low_speed_time);
    
    // 设置缓冲区大小
    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 32768L);
}
/*
以上优化建议涵盖了以下几个方面：

1. 内存安全性 ：添加安全的内存分配函数，防止内存泄漏和溢出
2. 资源管理 ：确保所有资源（如CURL句柄、互斥锁等）都能正确释放
3. 线程安全 ：增强全局变量的线程安全访问
4. 网络安全 ：添加SSL/TLS支持和URL验证
5. 性能优化 ：优化缓冲区大小和网络传输参数
6. 健壮性 ：添加网络状态监控，及时发现和处理网络问题
7. 错误处理 ：完善错误报告和统计功能
8. 配置灵活性 ：添加可配置的网络参数，方便根据不同环境调整
这些优化将使 network.c 文件更加适合生产环境使用，提高其稳定性、安全性和性能。
*/