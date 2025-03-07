#pragma once

/**
 * @brief 2048小游戏
 * @author 启凡科创
 * @date 2024-06-27
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

// 方块颜色
#define GAME_2048_2_COLOR 0x71814a
#define GAME_2048_4_COLOR 0Xd7be0c
#define GAME_2048_8_COLOR 0xfaa80f
#define GAME_2048_16_COLOR 0Xe9a999
#define GAME_2048_32_COLOR 0Xb6a9d1
#define GAME_2048_64_COLOR 0Xaa8056
#define GAME_2048_128_COLOR 0X524955
#define GAME_2048_256_COLOR 0Xafa49c
#define GAME_2048_512_COLOR 0X799298
#define GAME_2048_1024_COLOR 0X5c6fc3
#define GAME_2048_2048_COLOR 0X006d98
#define GAME_2048_4096_COLOR 0X2e5b1d
#define GAME_2048_8192_COLOR 0X7abfd6
#define GAME_2048_16384_COLOR 0Xbea07d
#define GAME_2048_32768_COLOR 0Xc72c1d

#define GAME_2048_WIDTH_HEIGHT 180 // 游戏长宽
#define GAME_2048_PIX_SPACE 6      // 点间距
#define GAME_2048_OFFSET_X 30
#define GAME_2048_OFFSET_Y 30

void game_2048_install();
