#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "audio_player.h"

static bool g_running = true;
static AudioPlayer *g_player = NULL;

// 信号处理
void signal_handler(int sig) {
    printf("\n收到退出信号\n");
    g_running = false;
    if (g_player) {
        player_stop(g_player);
    }
}

// 事件回调
void event_callback(PlayerEvent *event, void *user_data) {
    (void)user_data;
    
    switch (event->type) {
        case PLAYER_EVENT_STATE_CHANGED:
            printf("状态: %s\n", audio_player_get_state_name(event->data.state));
            break;
            
        case PLAYER_EVENT_INFO_CHANGED: {
            AudioInfo *info = &event->data.info;
            printf("音频信息: %s, %dHz, %d通道\n",
                   audio_player_get_format_name(info->format),
                   info->sample_rate, info->channels);
            break;
        }
            
        case PLAYER_EVENT_EOF:
            printf("播放结束\n");
            g_running = false;
            break;
            
        case PLAYER_EVENT_ERROR:
            printf("播放错误: %s\n", event->data.error);
            g_running = false;
            break;
            
        default:
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("用法: %s <音频文件>\n", argv[0]);
        return 1;
    }
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("简单音频播放器示例\n");
    printf("==================\n");
    
    // 初始化播放器库
    if (!audio_player_init()) {
        printf("初始化播放器失败\n");
        return 1;
    }
    
    // 创建播放器
    g_player = player_create();
    if (!g_player) {
        printf("创建播放器失败\n");
        audio_player_cleanup();
        return 1;
    }
    
    // 设置事件回调
    player_set_event_callback(g_player, event_callback, NULL);
    
    // 打开音频文件
    printf("正在打开: %s\n", argv[1]);
    if (!player_open(g_player, argv[1])) {
        printf("打开文件失败\n");
        player_destroy(g_player);
        audio_player_cleanup();
        return 1;
    }
    
    // 开始播放
    if (!player_play(g_player)) {
        printf("开始播放失败\n");
        player_destroy(g_player);
        audio_player_cleanup();
        return 1;
    }
    
    printf("正在播放...(按Ctrl+C退出)\n");
    
    // 等待播放完成或用户中断
    while (g_running) {
        sleep(1);
        
        // 显示播放进度
        int64_t pos = player_get_position(g_player);
        int64_t dur = player_get_duration(g_player);
        
        if (dur > 0) {
            char pos_str[32], dur_str[32];
            audio_player_format_time(pos, pos_str, sizeof(pos_str));
            audio_player_format_time(dur, dur_str, sizeof(dur_str));
            printf("\r进度: %s / %s", pos_str, dur_str);
            fflush(stdout);
        }
    }
    
    printf("\n正在清理...\n");
    
    // 清理资源
    player_destroy(g_player);
    audio_player_cleanup();
    
    printf("播放器已退出\n");
    return 0;
} 