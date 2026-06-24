#ifndef DESIGN_FRUIT_AUDIO_H
#define DESIGN_FRUIT_AUDIO_H
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
/* 初始化音频子系统：打开音频设备、加载 3 个 WAV。
 * 若音频设备不可用（如无声卡的无头环境），必须优雅失败：返回 false，
 * 且后续所有播放函数都成为安全的 no-op，绝不崩溃。可重复调用（幂等）。*/
bool design_fruit_audio_init(void);
void design_fruit_audio_deinit(void);     /* 停止、释放、关闭设备；可重复调用 */
void design_fruit_audio_play_bgm(void);   /* 从头开始循环播放 bgm（若 bgm 当前为启用状态）*/
void design_fruit_audio_stop_bgm(void);   /* 停止 bgm 播放（不改变 enabled 状态）*/
bool design_fruit_audio_toggle_bgm(void); /* 切换 bgm 开关：开->关(停止) / 关->开(从头播放)；返回切换后的 enabled */
bool design_fruit_audio_bgm_enabled(void);/* 当前 bgm 是否启用 */
void design_fruit_audio_play_hit(void);   /* 触发一次 hit 短音效，可与 bgm/其它音效叠加，可重叠播放 */
void design_fruit_audio_play_over(void);  /* 触发一次 over 短音效 */
#ifdef __cplusplus
}
#endif
#endif
