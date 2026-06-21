/**
 * @file game_2048.h
 * @brief 2048游戏头文件 - 基于LVGL v9实现
 */

#ifndef GAME_2048_H
#define GAME_2048_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"
#include <stdint.h>
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/
#define GRID_SIZE           4       /* 游戏网格大小 4x4 */
#define TILE_SIZE           80      /* 每个方块的大小（像素）*/
#define TILE_PADDING        10      /* 方块间距 */
#define BOARD_PADDING       20      /* 棋盘边距 */
#define ANIM_DURATION       200     /* 动画持续时间（毫秒）*/
#define WIN_VALUE           2048    /* 胜利目标值 */

/* 颜色定义 */
#define COLOR_BG            0xBBADA0    /* 棋盘背景色 */
#define COLOR_EMPTY         0xCDC1B4    /* 空方块颜色 */
#define COLOR_TEXT_DARK     0x776E65    /* 深色文字（2和4）*/
#define COLOR_TEXT_LIGHT    0xF9F6F3    /* 浅色文字 */

/**********************
 *      TYPEDEFS
 **********************/

/**
 * @brief 游戏状态枚举
 */
typedef enum {
    GAME_STATE_RUNNING = 0,     /* 游戏进行中 */
    GAME_STATE_WIN,              /* 游戏胜利 */
    GAME_STATE_LOSE              /* 游戏失败 */
} game_state_t;

/**
 * @brief 移动方向枚举
 */
typedef enum {
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} direction_t;

/**
 * @brief 方块数据结构
 */
typedef struct {
    int32_t value;              /* 方块数值（0表示空）*/
    lv_obj_t *tile_obj;         /* 方块UI对象 */
    lv_obj_t *label_obj;        /* 数字标签对象 */
    int8_t row;                 /* 当前行位置 */
    int8_t col;                 /* 当前列位置 */
} tile_t;

/**
 * @brief 游戏主结构体
 */
typedef struct {
    int32_t grid[GRID_SIZE][GRID_SIZE];     /* 游戏网格数据 */
    tile_t tiles[GRID_SIZE][GRID_SIZE];     /* 方块对象数组 */
    uint32_t score;                          /* 当前得分 */
    game_state_t state;                      /* 游戏状态 */

    /* UI对象 */
    lv_obj_t *board_container;               /* 棋盘容器 */
    lv_obj_t *score_label;                   /* 得分标签 */
    lv_obj_t *status_label;                  /* 状态标签 */
    lv_obj_t *reset_btn;                     /* 重置按钮 */
} game_2048_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief 初始化2048游戏
 * @return 指向游戏实例的指针
 */
game_2048_t *game_2048_create(void);

/**
 * @brief 重置游戏到初始状态
 * @param game 游戏实例指针
 */
void game_2048_reset(game_2048_t *game);

/**
 * @brief 执行移动操作
 * @param game 游戏实例指针
 * @param dir 移动方向
 * @return true表示移动成功，false表示无效移动
 */
bool game_2048_move(game_2048_t *game, direction_t dir);

/**
 * @brief 更新UI显示
 * @param game 游戏实例指针
 */
void game_2048_update_ui(game_2048_t *game);

/**
 * @brief 获取方块颜色
 * @param value 方块数值
 * @return 颜色值
 */
uint32_t game_2048_get_tile_color(int32_t value);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* GAME_2048_H */