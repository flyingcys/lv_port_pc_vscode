#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "audio_player.h"
#include "platform.h"

#define TAG "AudioPlayer"

static bool g_running = true;
static AudioPlayer *g_player = NULL;

// 信号处理函数
static void signal_handler(int sig) {
    printf("\n收到信号 %d，正在退出...\n", sig);
    g_running = false;
    
    if (g_player) {
        player_stop(g_player);
    }
}

// 播放器事件回调
static void player_event_callback(PlayerEvent *event, void *user_data) {
    (void)user_data; // 避免未使用参数警告
    
    switch (event->type) {
        case PLAYER_EVENT_STATE_CHANGED:
            printf("状态改变: %s\n", audio_player_get_state_name(event->data.state));
            break;
            
        case PLAYER_EVENT_POSITION_CHANGED: {
            char pos_str[32], dur_str[32];
            AudioInfo info;
            
            if (player_get_info(g_player, &info)) {
                audio_player_format_time(event->data.position, pos_str, sizeof(pos_str));
                audio_player_format_time(info.duration, dur_str, sizeof(dur_str));
                printf("\r播放进度: %s / %s", pos_str, dur_str);
                fflush(stdout);
            }
            break;
        }
            
        case PLAYER_EVENT_INFO_CHANGED: {
            AudioInfo *info = &event->data.info;
            printf("\n音频信息:\n");
            printf("  格式: %s\n", audio_player_get_format_name(info->format));
            printf("  采样率: %d Hz\n", info->sample_rate);
            printf("  通道数: %d\n", info->channels);
            printf("  比特率: %d bps\n", info->bit_rate);
            
            char dur_str[32];
            audio_player_format_time(info->duration, dur_str, sizeof(dur_str));
            printf("  时长: %s\n", dur_str);
            
            if (strlen(info->title) > 0) {
                printf("  标题: %s\n", info->title);
            }
            if (strlen(info->artist) > 0) {
                printf("  艺术家: %s\n", info->artist);
            }
            if (strlen(info->album) > 0) {
                printf("  专辑: %s\n", info->album);
            }
            break;
        }
            
        case PLAYER_EVENT_EOF:
            printf("\n播放结束\n");
            break;
            
        case PLAYER_EVENT_ERROR:
            printf("\n播放错误: %s\n", event->data.error);
            break;
            
        default:
            break;
    }
}

// 显示帮助信息
static void show_help(void) {
    printf("跨平台音乐播放器\n");
    printf("用法: audio_player <音频文件或URL>\n\n");
    printf("支持的格式:\n");
    
    AudioFormat formats[10];
    int count = audio_player_get_supported_formats(formats, 10);
    
    for (int i = 0; i < count; i++) {
        printf("  %s\n", audio_player_get_format_name(formats[i]));
    }
    
    printf("\n播放控制:\n");
    printf("  空格键    - 播放/暂停\n");
    printf("  s         - 停止\n");
    printf("  +/-       - 增加/减少音量\n");
    printf("  q/Ctrl+C  - 退出\n");
    printf("  h/?       - 显示帮助\n");
    printf("  i         - 显示音频信息\n");
}

// 显示音频信息
static void show_info(AudioPlayer *player) {
    AudioInfo info;
    if (!player_get_info(player, &info)) {
        printf("无法获取音频信息\n");
        return;
    }
    
    printf("\n=== 音频信息 ===\n");
    printf("格式: %s\n", audio_player_get_format_name(info.format));
    printf("采样率: %d Hz\n", info.sample_rate);
    printf("通道数: %d\n", info.channels);
    printf("比特率: %d bps\n", info.bit_rate);
    
    char dur_str[32];
    audio_player_format_time(info.duration, dur_str, sizeof(dur_str));
    printf("时长: %s\n", dur_str);
    
    if (strlen(info.title) > 0) {
        printf("标题: %s\n", info.title);
    }
    if (strlen(info.artist) > 0) {
        printf("艺术家: %s\n", info.artist);
    }
    if (strlen(info.album) > 0) {
        printf("专辑: %s\n", info.album);
    }
    
    printf("状态: %s\n", audio_player_get_state_name(player_get_state(player)));
    printf("音量: %d%%\n", player_get_volume(player));
    
    char pos_str[32];
    audio_player_format_time(player_get_position(player), pos_str, sizeof(pos_str));
    printf("位置: %s\n", pos_str);
}

// 处理用户输入
static void handle_input(AudioPlayer *player) {
    char input;
    
    printf("\n按任意键控制播放 (h显示帮助): ");
    fflush(stdout);
    
    while (g_running && (input = getchar()) != EOF) {
        switch (input) {
            case ' ':  // 空格键 - 播放/暂停
                if (player_get_state(player) == PLAYER_STATE_PLAYING) {
                    player_pause(player);
                    printf("已暂停\n");
                } else if (player_get_state(player) == PLAYER_STATE_PAUSED) {
                    player_play(player);
                    printf("继续播放\n");
                }
                break;
                
            case 's':  // 停止
                player_stop(player);
                printf("已停止\n");
                break;
                
            case '+':  // 增加音量
            case '=': {
                int volume = player_get_volume(player);
                volume = (volume < 100) ? volume + 5 : 100;
                player_set_volume(player, volume);
                printf("音量: %d%%\n", volume);
                break;
            }
                
            case '-':  // 减少音量
            case '_': {
                int volume = player_get_volume(player);
                volume = (volume > 0) ? volume - 5 : 0;
                player_set_volume(player, volume);
                printf("音量: %d%%\n", volume);
                break;
            }
                
            case 'q':  // 退出
                g_running = false;
                break;
                
            case 'h':  // 帮助
            case '?':
                show_help();
                break;
                
            case 'i':  // 信息
                show_info(player);
                break;
                
            case '\n':  // 忽略换行符
                break;
                
            default:
                printf("未知命令: %c (按h显示帮助)\n", input);
                break;
        }
        
        if (g_running) {
            printf("按任意键控制播放: ");
            fflush(stdout);
        }
    }
}

int main(int argc, char *argv[]) {
    // 检查参数
    if (argc != 2) {
        printf("用法: %s <音频文件或URL>\n", argv[0]);
        printf("使用 %s -h 显示详细帮助\n", argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        show_help();
        return 0;
    }
    
    // 初始化平台
    if (!platform_init()) {
        printf("平台初始化失败\n");
        return 1;
    }
    
    // 初始化音频播放器库
    if (!audio_player_init()) {
        printf("音频播放器初始化失败\n");
        platform_cleanup();
        return 1;
    }
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 创建播放器
    g_player = player_create();
    if (!g_player) {
        printf("创建播放器失败\n");
        audio_player_cleanup();
        platform_cleanup();
        return 1;
    }
    
    // 设置事件回调
    player_set_event_callback(g_player, player_event_callback, NULL);
    
    // 打开音频文件
    printf("正在打开: %s\n", argv[1]);
    if (!player_open(g_player, argv[1])) {
        printf("打开音频文件失败: %s\n", argv[1]);
        player_destroy(g_player);
        audio_player_cleanup();
        platform_cleanup();
        return 1;
    }
    
    // 开始播放
    if (!player_play(g_player)) {
        printf("开始播放失败\n");
        player_destroy(g_player);
        audio_player_cleanup();
        platform_cleanup();
        return 1;
    }
    
    printf("开始播放...\n");
    
    // 处理用户输入
    handle_input(g_player);
    
    // 清理资源
    printf("\n正在清理资源...\n");
    player_destroy(g_player);
    audio_player_cleanup();
    platform_cleanup();
    
    printf("再见！\n");
    return 0;
} 