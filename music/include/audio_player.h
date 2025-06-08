#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// 音频格式枚举
typedef enum {
    AUDIO_FORMAT_UNKNOWN,
    AUDIO_FORMAT_MP3,
    AUDIO_FORMAT_AAC,
    AUDIO_FORMAT_OPUS,
    AUDIO_FORMAT_WAV,
    AUDIO_FORMAT_M3U8
} AudioFormat;

// 音频信息结构
typedef struct {
    int sample_rate;            // 采样率 (Hz)
    int channels;               // 通道数
    int bits_per_sample;        // 每样本位数
    AudioFormat format;         // 音频格式
    int bit_rate;               // 比特率 (bps)
    int64_t duration;           // 总时长 (毫秒)
    char title[256];            // 歌曲标题
    char artist[256];           // 艺术家
    char album[256];            // 专辑名
} AudioInfo;

// 播放器状态枚举
typedef enum {
    PLAYER_STATE_STOPPED,
    PLAYER_STATE_PLAYING,
    PLAYER_STATE_PAUSED,
    PLAYER_STATE_BUFFERING,
    PLAYER_STATE_ERROR
} PlayerState;

// 播放器事件类型
typedef enum {
    PLAYER_EVENT_STATE_CHANGED,
    PLAYER_EVENT_POSITION_CHANGED,
    PLAYER_EVENT_DURATION_CHANGED,
    PLAYER_EVENT_INFO_CHANGED,
    PLAYER_EVENT_EOF,
    PLAYER_EVENT_ERROR
} PlayerEventType;

// 播放器事件结构
typedef struct {
    PlayerEventType type;
    union {
        PlayerState state;
        int64_t position;
        int64_t duration;
        AudioInfo info;
        const char *error;
    } data;
} PlayerEvent;

// 播放器事件回调函数类型
typedef void (*PlayerEventCallback)(PlayerEvent *event, void *user_data);

// 播放器句柄 (不透明指针)
typedef struct AudioPlayer AudioPlayer;

// ==== 播放器核心接口 ====

/**
 * 创建音频播放器实例
 * @return 播放器实例指针，失败返回NULL
 */
AudioPlayer* player_create(void);

/**
 * 销毁音频播放器实例
 * @param player 播放器实例
 */
void player_destroy(AudioPlayer *player);

/**
 * 打开音频源
 * @param player 播放器实例
 * @param uri 音频源URI (本地文件路径或网络URL)
 * @return 成功返回true，失败返回false
 */
bool player_open(AudioPlayer *player, const char *uri);

/**
 * 开始播放
 * @param player 播放器实例
 * @return 成功返回true，失败返回false
 */
bool player_play(AudioPlayer *player);

/**
 * 暂停播放
 * @param player 播放器实例
 * @return 成功返回true，失败返回false
 */
bool player_pause(AudioPlayer *player);

/**
 * 停止播放
 * @param player 播放器实例
 * @return 成功返回true，失败返回false
 */
bool player_stop(AudioPlayer *player);

/**
 * 跳转到指定位置
 * @param player 播放器实例
 * @param position 目标位置 (毫秒)
 * @return 成功返回true，失败返回false
 */
bool player_seek(AudioPlayer *player, int64_t position);

/**
 * 获取当前播放位置
 * @param player 播放器实例
 * @return 当前位置 (毫秒)
 */
int64_t player_get_position(AudioPlayer *player);

/**
 * 获取音频总时长
 * @param player 播放器实例
 * @return 总时长 (毫秒)
 */
int64_t player_get_duration(AudioPlayer *player);

/**
 * 设置音量
 * @param player 播放器实例
 * @param volume 音量值 (0-100)
 * @return 成功返回true，失败返回false
 */
bool player_set_volume(AudioPlayer *player, int volume);

/**
 * 获取当前音量
 * @param player 播放器实例
 * @return 音量值 (0-100)
 */
int player_get_volume(AudioPlayer *player);

/**
 * 获取当前播放状态
 * @param player 播放器实例
 * @return 播放状态
 */
PlayerState player_get_state(AudioPlayer *player);

/**
 * 获取音频信息
 * @param player 播放器实例
 * @param info 输出音频信息结构
 * @return 成功返回true，失败返回false
 */
bool player_get_info(AudioPlayer *player, AudioInfo *info);

/**
 * 设置事件回调
 * @param player 播放器实例
 * @param callback 回调函数
 * @param user_data 用户数据
 */
void player_set_event_callback(AudioPlayer *player, 
                               PlayerEventCallback callback, 
                               void *user_data);

// ==== 辅助接口 ====

/**
 * 初始化音频播放器库
 * @return 成功返回true，失败返回false
 */
bool audio_player_init(void);

/**
 * 清理音频播放器库
 */
void audio_player_cleanup(void);

/**
 * 获取支持的音频格式列表
 * @param formats 输出格式数组
 * @param max_count 最大格式数量
 * @return 实际格式数量
 */
int audio_player_get_supported_formats(AudioFormat *formats, int max_count);

/**
 * 检查是否支持指定格式
 * @param format 音频格式
 * @return 支持返回true，不支持返回false
 */
bool audio_player_is_format_supported(AudioFormat format);

/**
 * 根据文件扩展名检测音频格式
 * @param filename 文件名
 * @return 检测到的音频格式
 */
AudioFormat audio_player_detect_format(const char *filename);

/**
 * 获取格式名称字符串
 * @param format 音频格式
 * @return 格式名称字符串
 */
const char* audio_player_get_format_name(AudioFormat format);

/**
 * 获取播放状态名称字符串
 * @param state 播放状态
 * @return 状态名称字符串
 */
const char* audio_player_get_state_name(PlayerState state);

/**
 * 格式化时间为字符串 (MM:SS 或 HH:MM:SS)
 * @param time_ms 时间 (毫秒)
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 格式化后的字符串长度
 */
int audio_player_format_time(int64_t time_ms, char *buffer, int buffer_size);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_PLAYER_H 