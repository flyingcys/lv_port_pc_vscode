/**
 * @file game_2048.c
 * @brief 2048游戏实现 - 基于LVGL v9
 */

/*********************
 *      INCLUDES
 *********************/
#include "game_2048.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void add_random_tile(game_2048_t *game);
static bool can_move(game_2048_t *game);
static bool move_tiles(game_2048_t *game, direction_t dir);
static void check_game_status(game_2048_t *game);
static void create_tile_ui(game_2048_t *game, int row, int col);
static void update_tile_ui(game_2048_t *game, int row, int col);
static void gesture_event_handler(lv_event_t *e);
static void key_event_handler(lv_event_t *e);
static void board_event_handler(lv_event_t *e);
static void reset_button_event_handler(lv_event_t *e);

/**********************
 *  STATIC VARIABLES
 **********************/
static game_2048_t g_game; /* 全局游戏实例 */

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief 获取方块颜色
 */
uint32_t game_2048_get_tile_color(int32_t value)
{
    switch (value) {
        case 0:    return COLOR_EMPTY;
        case 2:    return 0xEEE4DA;
        case 4:    return 0xEDE0C8;
        case 8:    return 0xF2B179;
        case 16:   return 0xF59563;
        case 32:   return 0xF67C5F;
        case 64:   return 0xF65E3B;
        case 128:  return 0xEDCF72;
        case 256:  return 0xEDCC61;
        case 512:  return 0xEDC850;
        case 1024: return 0xEDC53F;
        case 2048: return 0xEDC22E;
        default:   return 0x3C3A32;
    }
}

/**
 * @brief 创建游戏实例并初始化UI
 */
game_2048_t *game_2048_create(void)
{
    game_2048_t *game = &g_game;
    memset(game, 0, sizeof(game_2048_t));

    /* 初始化随机数种子 */
    srand((unsigned)time(NULL));

    /* 创建主容器 */
    lv_obj_t *container = lv_obj_create(lv_screen_active());
    lv_obj_set_size(container, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(container, lv_color_hex(0xFAF8EF), 0);
    lv_obj_center(container);
    lv_obj_remove_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(container, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_remove_flag(container, LV_OBJ_FLAG_SCROLL_MOMENTUM);

    /* 创建顶部信息栏 */
    lv_obj_t *top_panel = lv_obj_create(container);
    lv_obj_set_size(top_panel, LV_HOR_RES - 40, 60);
    lv_obj_set_pos(top_panel, 20, 10);
    lv_obj_set_style_bg_color(top_panel, lv_color_hex(0xBBADA0), 0);
    lv_obj_remove_flag(top_panel, LV_OBJ_FLAG_SCROLLABLE);

    /* 得分标签 */
    game->score_label = lv_label_create(top_panel);
    lv_label_set_text(game->score_label, "Score: 0");
    lv_obj_set_style_text_color(game->score_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(game->score_label, &lv_font_montserrat_20, 0);
    lv_obj_align(game->score_label, LV_ALIGN_LEFT_MID, 10, 0);

    /* 重置按钮 */
    game->reset_btn = lv_button_create(top_panel);
    lv_obj_set_size(game->reset_btn, 80, 40);
    lv_obj_align(game->reset_btn, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_add_event_cb(game->reset_btn, reset_button_event_handler,
                        LV_EVENT_CLICKED, game);

    lv_obj_t *btn_label = lv_label_create(game->reset_btn);
    lv_label_set_text(btn_label, "Reset");
    lv_obj_center(btn_label);

    /* 创建棋盘容器 */
    int board_size = GRID_SIZE * TILE_SIZE + (GRID_SIZE + 1) * TILE_PADDING;
    game->board_container = lv_obj_create(container);
    lv_obj_set_size(game->board_container, board_size, board_size);
    lv_obj_align(game->board_container, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_color(game->board_container, lv_color_hex(COLOR_BG), 0);
    lv_obj_set_style_border_width(game->board_container, 0, 0);
    lv_obj_set_style_radius(game->board_container, 6, 0);

    /* 完全禁用滚动功能 */
    lv_obj_remove_flag(game->board_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(game->board_container, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_remove_flag(game->board_container, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_add_flag(game->board_container, LV_OBJ_FLAG_CLICKABLE);

    /* 添加事件处理 */
    lv_obj_add_event_cb(game->board_container, board_event_handler,
                        LV_EVENT_PRESSED, game);
    lv_obj_add_event_cb(game->board_container, board_event_handler,
                        LV_EVENT_PRESSING, game);
    lv_obj_add_event_cb(game->board_container, board_event_handler,
                        LV_EVENT_RELEASED, game);
    lv_obj_add_event_cb(container, key_event_handler,
                        LV_EVENT_KEY, game);

    /* 使容器可以获得焦点以接收键盘事件 */
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);
    lv_group_add_obj(lv_group_get_default(), container);

    /* 创建背景方块 */
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            lv_obj_t *bg_tile = lv_obj_create(game->board_container);
            lv_obj_set_size(bg_tile, TILE_SIZE, TILE_SIZE);
            lv_obj_set_pos(bg_tile,
                          TILE_PADDING + j * (TILE_SIZE + TILE_PADDING),
                          TILE_PADDING + i * (TILE_SIZE + TILE_PADDING));
            lv_obj_set_style_bg_color(bg_tile, lv_color_hex(COLOR_EMPTY), 0);
            lv_obj_set_style_border_width(bg_tile, 0, 0);
            lv_obj_set_style_radius(bg_tile, 3, 0);
            lv_obj_remove_flag(bg_tile, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_remove_flag(bg_tile, LV_OBJ_FLAG_CLICKABLE);
        }
    }

    /* 状态标签 */
    game->status_label = lv_label_create(container);
    lv_label_set_text(game->status_label, "");
    lv_obj_set_style_text_font(game->status_label, &lv_font_montserrat_24, 0);
    lv_obj_align(game->status_label, LV_ALIGN_BOTTOM_MID, 0, -20);

    /* 初始化游戏 */
    game_2048_reset(game);

    return game;
}

/**
 * @brief 重置游戏
 */
void game_2048_reset(game_2048_t *game)
{
    /* 清空网格 */
    memset(game->grid, 0, sizeof(game->grid));
    game->score = 0;
    game->state = GAME_STATE_RUNNING;

    /* 删除所有方块UI */
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game->tiles[i][j].tile_obj != NULL) {
                lv_obj_delete(game->tiles[i][j].tile_obj);
                game->tiles[i][j].tile_obj = NULL;
                game->tiles[i][j].label_obj = NULL;
            }
            game->tiles[i][j].value = 0;
        }
    }

    /* 添加两个初始方块 */
    add_random_tile(game);
    add_random_tile(game);

    /* 更新UI */
    game_2048_update_ui(game);
}

/**
 * @brief 移动方块
 */
bool game_2048_move(game_2048_t *game, direction_t dir)
{
    if (game->state != GAME_STATE_RUNNING) {
        return false;
    }

    bool moved = move_tiles(game, dir);

    if (moved) {
        add_random_tile(game);
        check_game_status(game);
        game_2048_update_ui(game);
    }

    return moved;
}

/**
 * @brief 更新UI显示
 */
void game_2048_update_ui(game_2048_t *game)
{
    /* 更新得分 */
    char score_text[32];
    snprintf(score_text, sizeof(score_text), "Score: %u", game->score);
    lv_label_set_text(game->score_label, score_text);

    /* 删除所有旧的方块UI */
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game->tiles[i][j].tile_obj != NULL) {
                lv_obj_delete(game->tiles[i][j].tile_obj);
                game->tiles[i][j].tile_obj = NULL;
                game->tiles[i][j].label_obj = NULL;
                game->tiles[i][j].value = 0;
            }
        }
    }

    /* 根据当前grid重新创建所有方块UI */
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game->grid[i][j] != 0) {
                create_tile_ui(game, i, j);
            }
        }
    }

    /* 更新状态信息 */
    if (game->state == GAME_STATE_WIN) {
        lv_label_set_text(game->status_label, "You Win!");
        lv_obj_set_style_text_color(game->status_label,
                                    lv_color_hex(0x00AA00), 0);
    } else if (game->state == GAME_STATE_LOSE) {
        lv_label_set_text(game->status_label, "Game Over!");
        lv_obj_set_style_text_color(game->status_label,
                                    lv_color_hex(0xFF0000), 0);
    } else {
        lv_label_set_text(game->status_label, "");
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * @brief 添加随机方块
 */
static void add_random_tile(game_2048_t *game)
{
    /* 统计空位 */
    int empty_cells[GRID_SIZE * GRID_SIZE][2];
    int count = 0;

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game->grid[i][j] == 0) {
                empty_cells[count][0] = i;
                empty_cells[count][1] = j;
                count++;
            }
        }
    }

    if (count > 0) {
        /* 随机选择一个空位 */
        int idx = rand() % count;
        int row = empty_cells[idx][0];
        int col = empty_cells[idx][1];

        /* 90%概率生成2，10%概率生成4 */
        game->grid[row][col] = (rand() % 10 == 0) ? 4 : 2;
    }
}

/**
 * @brief 移动并合并方块
 */
static bool move_tiles(game_2048_t *game, direction_t dir)
{
    int32_t old_grid[GRID_SIZE][GRID_SIZE];
    memcpy(old_grid, game->grid, sizeof(old_grid));
    bool moved = false;

    if (dir == DIR_LEFT) {
        /* 向左移动 */
        for (int i = 0; i < GRID_SIZE; i++) {
            int pos = 0;
            int last_merge = -1;
            for (int j = 0; j < GRID_SIZE; j++) {
                if (game->grid[i][j] != 0) {
                    int value = game->grid[i][j];
                    game->grid[i][j] = 0;

                    if (pos > 0 && game->grid[i][pos-1] == value &&
                        last_merge != pos-1) {
                        /* 合并 */
                        game->grid[i][pos-1] *= 2;
                        game->score += game->grid[i][pos-1];
                        last_merge = pos - 1;
                    } else {
                        game->grid[i][pos] = value;
                        pos++;
                    }
                }
            }
        }
    } else if (dir == DIR_RIGHT) {
        /* 向右移动 */
        for (int i = 0; i < GRID_SIZE; i++) {
            int pos = GRID_SIZE - 1;
            int last_merge = -1;
            for (int j = GRID_SIZE - 1; j >= 0; j--) {
                if (game->grid[i][j] != 0) {
                    int value = game->grid[i][j];
                    game->grid[i][j] = 0;

                    if (pos < GRID_SIZE - 1 && game->grid[i][pos+1] == value &&
                        last_merge != pos+1) {
                        game->grid[i][pos+1] *= 2;
                        game->score += game->grid[i][pos+1];
                        last_merge = pos + 1;
                    } else {
                        game->grid[i][pos] = value;
                        pos--;
                    }
                }
            }
        }
    } else if (dir == DIR_UP) {
        /* 向上移动 */
        for (int j = 0; j < GRID_SIZE; j++) {
            int pos = 0;
            int last_merge = -1;
            for (int i = 0; i < GRID_SIZE; i++) {
                if (game->grid[i][j] != 0) {
                    int value = game->grid[i][j];
                    game->grid[i][j] = 0;

                    if (pos > 0 && game->grid[pos-1][j] == value &&
                        last_merge != pos-1) {
                        game->grid[pos-1][j] *= 2;
                        game->score += game->grid[pos-1][j];
                        last_merge = pos - 1;
                    } else {
                        game->grid[pos][j] = value;
                        pos++;
                    }
                }
            }
        }
    } else if (dir == DIR_DOWN) {
        /* 向下移动 */
        for (int j = 0; j < GRID_SIZE; j++) {
            int pos = GRID_SIZE - 1;
            int last_merge = -1;
            for (int i = GRID_SIZE - 1; i >= 0; i--) {
                if (game->grid[i][j] != 0) {
                    int value = game->grid[i][j];
                    game->grid[i][j] = 0;

                    if (pos < GRID_SIZE - 1 && game->grid[pos+1][j] == value &&
                        last_merge != pos+1) {
                        game->grid[pos+1][j] *= 2;
                        game->score += game->grid[pos+1][j];
                        last_merge = pos + 1;
                    } else {
                        game->grid[pos][j] = value;
                        pos--;
                    }
                }
            }
        }
    }

    /* 检查是否有移动 */
    for (int i = 0; i < GRID_SIZE && !moved; i++) {
        for (int j = 0; j < GRID_SIZE && !moved; j++) {
            if (old_grid[i][j] != game->grid[i][j]) {
                moved = true;
            }
        }
    }

    return moved;
}

/**
 * @brief 检查是否还能移动
 */
static bool can_move(game_2048_t *game)
{
    /* 检查是否有空位 */
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game->grid[i][j] == 0) {
                return true;
            }
        }
    }

    /* 检查是否有相邻相同数字 */
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            int value = game->grid[i][j];
            if ((i < GRID_SIZE - 1 && game->grid[i+1][j] == value) ||
                (j < GRID_SIZE - 1 && game->grid[i][j+1] == value)) {
                return true;
            }
        }
    }

    return false;
}

/**
 * @brief 检查游戏状态
 */
static void check_game_status(game_2048_t *game)
{
    /* 检查胜利条件 */
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game->grid[i][j] == WIN_VALUE) {
                game->state = GAME_STATE_WIN;
                return;
            }
        }
    }

    /* 检查失败条件 */
    if (!can_move(game)) {
        game->state = GAME_STATE_LOSE;
    }
}

/**
 * @brief 创建方块UI
 */
static void create_tile_ui(game_2048_t *game, int row, int col)
{
    int32_t value = game->grid[row][col];
    if (value == 0) return;

    /* 创建方块对象 */
    lv_obj_t *tile = lv_obj_create(game->board_container);
    lv_obj_set_size(tile, TILE_SIZE, TILE_SIZE);
    lv_obj_set_pos(tile,
                  TILE_PADDING + col * (TILE_SIZE + TILE_PADDING),
                  TILE_PADDING + row * (TILE_SIZE + TILE_PADDING));
    lv_obj_set_style_bg_color(tile, lv_color_hex(game_2048_get_tile_color(value)), 0);
    lv_obj_set_style_border_width(tile, 0, 0);
    lv_obj_set_style_radius(tile, 3, 0);
    lv_obj_remove_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

    /* 创建数字标签 */
    lv_obj_t *label = lv_label_create(tile);
    char text[16];
    snprintf(text, sizeof(text), "%d", value);
    lv_label_set_text(label, text);
    lv_obj_center(label);

    /* 设置文字颜色和大小 */
    uint32_t text_color = (value <= 4) ? COLOR_TEXT_DARK : COLOR_TEXT_LIGHT;
    lv_obj_set_style_text_color(label, lv_color_hex(text_color), 0);

    if (value < 100) {
        lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
    } else if (value < 1000) {
        lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
    } else {
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    }

    game->tiles[row][col].tile_obj = tile;
    game->tiles[row][col].label_obj = label;
    game->tiles[row][col].value = value;
    game->tiles[row][col].row = row;
    game->tiles[row][col].col = col;
}

/**
 * @brief 更新方块UI
 */
static void update_tile_ui(game_2048_t *game, int row, int col)
{
    int32_t value = game->grid[row][col];
    lv_obj_t *tile = game->tiles[row][col].tile_obj;
    lv_obj_t *label = game->tiles[row][col].label_obj;

    if (tile == NULL || value == 0) return;

    /* 更新位置 */
    lv_obj_set_pos(tile,
                  TILE_PADDING + col * (TILE_SIZE + TILE_PADDING),
                  TILE_PADDING + row * (TILE_SIZE + TILE_PADDING));

    /* 更新颜色 */
    lv_obj_set_style_bg_color(tile, lv_color_hex(game_2048_get_tile_color(value)), 0);

    /* 更新数字 */
    char text[16];
    snprintf(text, sizeof(text), "%d", value);
    lv_label_set_text(label, text);

    /* 更新文字颜色和大小 */
    uint32_t text_color = (value <= 4) ? COLOR_TEXT_DARK : COLOR_TEXT_LIGHT;
    lv_obj_set_style_text_color(label, lv_color_hex(text_color), 0);

    if (value < 100) {
        lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
    } else if (value < 1000) {
        lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
    } else {
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    }

    game->tiles[row][col].value = value;
    game->tiles[row][col].row = row;
    game->tiles[row][col].col = col;
}

/**
 * @brief 手势事件处理
 */
static void gesture_event_handler(lv_event_t *e)
{
    game_2048_t *game = (game_2048_t *)lv_event_get_user_data(e);
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

    direction_t game_dir;
    switch (dir) {
        case LV_DIR_LEFT:
            game_dir = DIR_LEFT;
            break;
        case LV_DIR_RIGHT:
            game_dir = DIR_RIGHT;
            break;
        case LV_DIR_TOP:
            game_dir = DIR_UP;
            break;
        case LV_DIR_BOTTOM:
            game_dir = DIR_DOWN;
            break;
        default:
            return;
    }

    game_2048_move(game, game_dir);
}

/**
 * @brief 棋盘拖拽事件处理（用于鼠标滑动）
 */
static void board_event_handler(lv_event_t *e)
{
    static int32_t start_x = 0, start_y = 0;
    static bool drag_started = false;

    game_2048_t *game = (game_2048_t *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED) {
        /* 记录起始位置 */
        lv_indev_t *indev = lv_indev_active();
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        start_x = point.x;
        start_y = point.y;
        drag_started = false;
    } else if (code == LV_EVENT_PRESSING) {
        /* 检测拖拽方向 */
        if (!drag_started) {
            lv_indev_t *indev = lv_indev_active();
            lv_point_t point;
            lv_indev_get_point(indev, &point);

            int32_t dx = point.x - start_x;
            int32_t dy = point.y - start_y;

            /* 拖拽距离阈值 */
            const int32_t threshold = 30;

            if (abs(dx) > threshold || abs(dy) > threshold) {
                drag_started = true;
                direction_t dir;

                /* 判断主要移动方向 */
                if (abs(dx) > abs(dy)) {
                    dir = (dx > 0) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    dir = (dy > 0) ? DIR_DOWN : DIR_UP;
                }

                if (game_2048_move(game, dir)) {
                    /* 移动成功，打印调试信息 */
                    printf("Move success: direction=%d\n", dir);
                }
            }
        }
    } else if (code == LV_EVENT_RELEASED) {
        /* 松开鼠标，重置状态 */
        drag_started = false;
    }
}

/**
 * @brief 键盘事件处理
 */
static void key_event_handler(lv_event_t *e)
{
    game_2048_t *game = (game_2048_t *)lv_event_get_user_data(e);
    uint32_t key = lv_event_get_key(e);

    direction_t dir;
    bool valid_key = true;

    printf("Key pressed: %u\n", key);

    switch (key) {
        case LV_KEY_LEFT:
            dir = DIR_LEFT;
            printf("Direction: LEFT\n");
            break;
        case LV_KEY_RIGHT:
            dir = DIR_RIGHT;
            printf("Direction: RIGHT\n");
            break;
        case LV_KEY_UP:
            dir = DIR_UP;
            printf("Direction: UP\n");
            break;
        case LV_KEY_DOWN:
            dir = DIR_DOWN;
            printf("Direction: DOWN\n");
            break;
        default:
            valid_key = false;
            break;
    }

    if (valid_key) {
        if (game_2048_move(game, dir)) {
            printf("Key move success!\n");
        } else {
            printf("Key move failed (no valid move)\n");
        }
    }
}

/**
 * @brief 重置按钮事件处理
 */
static void reset_button_event_handler(lv_event_t *e)
{
    game_2048_t *game = (game_2048_t *)lv_event_get_user_data(e);
    game_2048_reset(game);
}