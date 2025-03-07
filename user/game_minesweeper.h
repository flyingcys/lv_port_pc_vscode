#pragma once

#include "static_include.h"

/**
 * @brief 扫雷小游戏
 * @author 启凡科创
 * @date 2024-06-27
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

LV_FONT_DECLARE(game_minesweeper_font_16);

#define BOOM_NUMBER_DEFAULT 28  // 默认雷数量
#define MINESWEEPER_LINE_NUM 12 // 列数,最大254
#define MINESWEEPER_COL_NUM 15  // 行数,最大16
#define MINESWEEPER_OFFSET_X 24 // 起始坐标x偏移
#define MINESWEEPER_OFFSET_Y 0  // 起始坐标y偏移

#define FLAG_IMG_POS_ALIGN LV_ALIGN_LEFT_MID
#define FLAG_IMG_POS_OFFSET_X 2
#define FLAG_IMG_POS_OFFSET_Y -22

#define FLAG_NUM_POS_ALIGN LV_ALIGN_LEFT_MID
#define FLAG_NUM_POS_OFFSET_X 5
#define FLAG_NUM_POS_OFFSET_Y 15
#define FLAG_NUM_SHOW_HES 0
#define FLAG_NUM_FONT game_minesweeper_font_16

#define SMILE_IMG_POS_ALIGN LV_ALIGN_RIGHT_MID
#define SMILE_IMG_POS_OFFSET_X -2
#define SMILE_IMG_POS_OFFSET_Y -22

#define TIME_NUM_POS_ALIGN LV_ALIGN_RIGHT_MID
#define TIME_NUM_POS_OFFSET_X -5
#define TIME_NUM_POS_OFFSET_Y 15
#define TIME_NUM_SHOW_HES 0
#define TIME_NUM_FONT game_minesweeper_font_16

#define MINESWEEPER_CLIP_PIX_EN 1 // 1：启用禁止布雷区域（适用于圆屏），0：禁用，禁止坐标请在game_minesweeper_clip_pix.h内填写

#define MINESWEEPER_MEMERY_MALLOC(size) heap_caps_malloc(size, MALLOC_CAP_SPIRAM) // 申请内存函数
#define MINESWEEPER_MEMERY_FREE(size) free(size)                                  // 释放内存函数
#define MINESWEEPER_GET_MS_TIC (esp_timer_get_time() / 1000)                      // 获取系统ms时间

void game_minesweeper_install();
