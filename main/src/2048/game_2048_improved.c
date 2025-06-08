#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <lvgl/lvgl.h>
#include "game_2048_improved.h"

#define SIZE 4
#define CELL_SIZE 80
#define PADDING 10

// 游戏状态结构体 - 组织全局变量
typedef struct {
    lv_obj_t *parent;
    lv_obj_t *score_label;
    lv_obj_t *best_label;
    lv_obj_t *grid_container;
    lv_obj_t *main_container;
    lv_obj_t *grid_obj[SIZE][SIZE];
    lv_obj_t *grid_label[SIZE][SIZE];
    uint32_t grid_value[SIZE][SIZE];
    uint32_t score;
    uint32_t best;
    bool game_over;
} game_2048_t;

static game_2048_t game_state = {0};

// 颜色配置 - 集中管理
typedef struct {
    uint32_t value;
    uint32_t bg_color;
    uint32_t text_color;
} color_config_t;

static const color_config_t color_map[] = {
    {0,    0xCDC1B4, 0x776E65},
    {2,    0xEEE4DA, 0x776E65},
    {4,    0xEDE0C8, 0x776E65},
    {8,    0xF2B179, 0xFFFFFF},
    {16,   0xF59563, 0xFFFFFF},
    {32,   0xF67C5F, 0xFFFFFF},
    {64,   0xF65E3B, 0xFFFFFF},
    {128,  0xEDCF72, 0xFFFFFF},
    {256,  0xEDCC61, 0xFFFFFF},
    {512,  0xEDC850, 0xFFFFFF},
    {1024, 0xEDC53F, 0xFFFFFF},
    {2048, 0xEDC22E, 0xFFFFFF}
};

// 获取颜色配置
static void get_value_colors(uint32_t value, uint32_t *bg_color, uint32_t *text_color)
{
    for (size_t i = 0; i < sizeof(color_map) / sizeof(color_map[0]); i++) {
        if (color_map[i].value == value) {
            *bg_color = color_map[i].bg_color;
            *text_color = color_map[i].text_color;
            return;
        }
    }
    
    // 默认颜色（大于2048的数字）
    *bg_color = 0xEDC22E;
    *text_color = 0xFFFFFF;
}

// 检查游戏是否结束
static bool is_game_over(void)
{
    // 检查空格子
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (game_state.grid_value[i][j] == 0) {
                return false;
            }
        }
    }

    // 检查相邻相同数字
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            uint32_t current = game_state.grid_value[i][j];
            
            // 检查右邻居
            if (j < SIZE - 1 && current == game_state.grid_value[i][j + 1]) {
                return false;
            }
            
            // 检查下邻居
            if (i < SIZE - 1 && current == game_state.grid_value[i + 1][j]) {
                return false;
            }
        }
    }

    return true;
}

// 添加随机数字
static void add_random_value(void)
{
    int empty_cells[SIZE * SIZE][2];
    int count = 0;
    
    // 收集空格子位置
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (game_state.grid_value[i][j] == 0) {
                empty_cells[count][0] = i;
                empty_cells[count][1] = j;
                count++;
            }
        }
    }
    
    if (count > 0) {
        int index = rand() % count;
        int value = (rand() % 10 == 0) ? 4 : 2;
        game_state.grid_value[empty_cells[index][0]][empty_cells[index][1]] = value;
    }
}

// 更新UI显示
static void update_display(void)
{
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            uint32_t value = game_state.grid_value[i][j];
            uint32_t bg_color, text_color;
            
            get_value_colors(value, &bg_color, &text_color);
            
            if (value > 0) {
                lv_label_set_text_fmt(game_state.grid_label[i][j], "%d", value);
            } else {
                lv_label_set_text(game_state.grid_label[i][j], "");
            }
            
            lv_obj_set_style_bg_color(game_state.grid_obj[i][j], lv_color_hex(bg_color), 0);
            lv_obj_set_style_text_color(game_state.grid_label[i][j], lv_color_hex(text_color), 0);
        }
    }

    // 更新分数
    lv_label_set_text_fmt(game_state.score_label, "得分\n%d", game_state.score);
    if (game_state.best_label) {
        lv_label_set_text_fmt(game_state.best_label, "高分\n%d", game_state.best);
    }
}

// 统一的移动逻辑
static bool move_line(uint32_t line[SIZE], bool reverse)
{
    uint32_t temp[SIZE] = {0};
    int pos = 0;
    bool moved = false;
    
    // 收集非零元素
    if (reverse) {
        for (int i = SIZE - 1; i >= 0; i--) {
            if (line[i] != 0) {
                temp[pos++] = line[i];
            }
        }
    } else {
        for (int i = 0; i < SIZE; i++) {
            if (line[i] != 0) {
                temp[pos++] = line[i];
            }
        }
    }
    
    // 合并相同数字
    for (int i = 0; i < pos - 1; i++) {
        if (temp[i] == temp[i + 1]) {
            temp[i] *= 2;
            game_state.score += temp[i];
            
            // 移除已合并的元素
            for (int j = i + 1; j < pos - 1; j++) {
                temp[j] = temp[j + 1];
            }
            pos--;
        }
    }
    
    // 重新填充line
    uint32_t new_line[SIZE] = {0};
    
    if (reverse) {
        for (int i = 0; i < pos; i++) {
            new_line[SIZE - 1 - i] = temp[i];
        }
    } else {
        for (int i = 0; i < pos; i++) {
            new_line[i] = temp[i];
        }
    }
    
    // 检查是否有变化
    for (int i = 0; i < SIZE; i++) {
        if (line[i] != new_line[i]) {
            moved = true;
        }
        line[i] = new_line[i];
    }
    
    return moved;
}

// 统一的移动处理
static bool process_move(lv_dir_t direction)
{
    bool moved = false;
    
    switch (direction) {
        case LV_DIR_LEFT:
            for (int i = 0; i < SIZE; i++) {
                if (move_line(game_state.grid_value[i], false)) {
                    moved = true;
                }
            }
            break;
            
        case LV_DIR_RIGHT:
            for (int i = 0; i < SIZE; i++) {
                if (move_line(game_state.grid_value[i], true)) {
                    moved = true;
                }
            }
            break;
            
        case LV_DIR_TOP:
        case LV_DIR_BOTTOM: {
            for (int j = 0; j < SIZE; j++) {
                uint32_t column[SIZE];
                
                // 提取列
                for (int i = 0; i < SIZE; i++) {
                    column[i] = game_state.grid_value[i][j];
                }
                
                // 移动列
                if (move_line(column, direction == LV_DIR_BOTTOM)) {
                    moved = true;
                }
                
                // 写回列
                for (int i = 0; i < SIZE; i++) {
                    game_state.grid_value[i][j] = column[i];
                }
            }
            break;
        }
        
        default:
            break;
    }
    
    return moved;
}

// 手势事件处理
static void gesture_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_GESTURE) {
        lv_indev_t *indev = lv_indev_active();
        if (!indev) return;
        
        lv_dir_t dir = lv_indev_get_gesture_dir(indev);
        
        if (game_state.game_over) {
            return;
        }
        
        if (process_move(dir)) {
            add_random_value();
            
            // 更新最高分
            if (game_state.score > game_state.best) {
                game_state.best = game_state.score;
            }
            
            update_display();
            
            // 检查游戏结束
            if (is_game_over()) {
                game_state.game_over = true;
                
                lv_obj_t *mbox = lv_msgbox_create(NULL);
                lv_msgbox_add_title(mbox, "游戏结束");
                lv_msgbox_add_text(mbox, "无法继续移动!");
                lv_msgbox_add_close_button(mbox);
            }
        }
    }
}

// 新游戏按钮回调
static void new_game_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // 重置游戏状态
        memset(game_state.grid_value, 0, sizeof(game_state.grid_value));
        game_state.score = 0;
        game_state.game_over = false;
        
        // 添加初始数字
        add_random_value();
        add_random_value();
        
        update_display();
    }
}

// 创建分数区域
static void create_score_area(lv_obj_t *parent)
{
    // 分数容器样式
    static lv_style_t score_style;
    lv_style_init(&score_style);
    lv_style_set_bg_color(&score_style, lv_color_hex(0xBBADA0));
    lv_style_set_border_width(&score_style, 0);
    lv_style_set_radius(&score_style, 3);
    lv_style_set_pad_all(&score_style, 8);

    // 得分显示
    lv_obj_t *score_box = lv_obj_create(parent);
    lv_obj_add_style(score_box, &score_style, 0);
    lv_obj_set_size(score_box, 80, 60);
    lv_obj_set_pos(score_box, lv_obj_get_width(parent) - 180, 20);

    game_state.score_label = lv_label_create(score_box);
    lv_label_set_text_fmt(game_state.score_label, "得分\n%d", game_state.score);
    lv_obj_set_style_text_align(game_state.score_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(game_state.score_label, lv_color_white(), 0);
    lv_obj_center(game_state.score_label);

    // 最高分显示
    lv_obj_t *best_box = lv_obj_create(parent);
    lv_obj_add_style(best_box, &score_style, 0);
    lv_obj_set_size(best_box, 80, 60);
    lv_obj_set_pos(best_box, lv_obj_get_width(parent) - 90, 20);

    game_state.best_label = lv_label_create(best_box);
    lv_label_set_text_fmt(game_state.best_label, "高分\n%d", game_state.best);
    lv_obj_set_style_text_align(game_state.best_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(game_state.best_label, lv_color_white(), 0);
    lv_obj_center(game_state.best_label);
}

// 创建游戏网格
static void create_game_grid(lv_obj_t *parent)
{
    int32_t screen_w = lv_obj_get_width(parent);
    int32_t screen_h = lv_obj_get_height(parent);
    
    // 计算网格尺寸
    int32_t grid_size = LV_MIN(screen_w - 40, screen_h - 200);
    int32_t cell_size = (grid_size - PADDING * (SIZE + 1)) / SIZE;
    
    // 创建网格容器
    game_state.grid_container = lv_obj_create(parent);
    lv_obj_set_size(game_state.grid_container, grid_size, grid_size);
    lv_obj_set_pos(game_state.grid_container, 20, 160);
    lv_obj_set_style_bg_color(game_state.grid_container, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_border_width(game_state.grid_container, 0, 0);
    lv_obj_set_style_radius(game_state.grid_container, 6, 0);
    lv_obj_set_style_pad_all(game_state.grid_container, PADDING, 0);

    // 网格样式
    static lv_style_t cell_style;
    lv_style_init(&cell_style);
    lv_style_set_radius(&cell_style, 3);
    lv_style_set_border_width(&cell_style, 0);

    // 创建4x4网格
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            game_state.grid_obj[i][j] = lv_obj_create(game_state.grid_container);
            lv_obj_add_style(game_state.grid_obj[i][j], &cell_style, 0);
            lv_obj_set_size(game_state.grid_obj[i][j], cell_size, cell_size);
            lv_obj_set_pos(game_state.grid_obj[i][j], 
                           j * (cell_size + PADDING), 
                           i * (cell_size + PADDING));

            game_state.grid_label[i][j] = lv_label_create(game_state.grid_obj[i][j]);
            lv_obj_set_style_text_font(game_state.grid_label[i][j], &lv_font_montserrat_32, 0);
            lv_obj_center(game_state.grid_label[i][j]);
        }
    }
}

// 创建控制按钮
static void create_control_buttons(lv_obj_t *parent)
{
    static lv_style_t btn_style;
    lv_style_init(&btn_style);
    lv_style_set_bg_color(&btn_style, lv_color_hex(0xBBADA0));
    lv_style_set_border_width(&btn_style, 0);
    lv_style_set_radius(&btn_style, 3);

    // 新游戏按钮
    lv_obj_t *new_game_btn = lv_button_create(parent);
    lv_obj_add_style(new_game_btn, &btn_style, 0);
    lv_obj_set_size(new_game_btn, 50, 50);
    lv_obj_set_pos(new_game_btn, lv_obj_get_width(parent) - 125, 100);
    lv_obj_add_event_cb(new_game_btn, new_game_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *refresh_icon = lv_label_create(new_game_btn);
    lv_label_set_text(refresh_icon, LV_SYMBOL_REFRESH);
    lv_obj_set_style_text_color(refresh_icon, lv_color_white(), 0);
    lv_obj_center(refresh_icon);
}

// 初始化游戏
static void game_init(void)
{
    memset(&game_state, 0, sizeof(game_state));
    srand(time(NULL));
    
    add_random_value();
    add_random_value();
}

// 主创建函数
void game_2048_improved(lv_obj_t *parent)
{
    game_state.parent = parent;
    
    // 创建主容器
    game_state.main_container = lv_obj_create(parent);
    lv_obj_set_size(game_state.main_container, lv_obj_get_width(parent), lv_obj_get_height(parent));
    lv_obj_set_style_border_width(game_state.main_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(game_state.main_container, lv_color_hex(0xFAF8EF), 0);
    lv_obj_set_scrollbar_mode(game_state.main_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(game_state.main_container, LV_OBJ_FLAG_SCROLLABLE);

    // 创建标题
    static lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_color(&title_style, lv_color_hex(0x776E65));
    lv_style_set_text_font(&title_style, &lv_font_montserrat_48);
    
    lv_obj_t *title = lv_label_create(game_state.main_container);
    lv_label_set_text(title, "2048");
    lv_obj_add_style(title, &title_style, 0);
    lv_obj_set_pos(title, 20, 20);

    // 创建UI组件
    create_score_area(game_state.main_container);
    create_control_buttons(game_state.main_container);
    create_game_grid(game_state.main_container);
    
    // 初始化游戏
    game_init();
    update_display();
    
    // 添加手势事件
    lv_obj_add_event_cb(game_state.main_container, gesture_event_cb, LV_EVENT_GESTURE, NULL);
    
    printf("改进版2048游戏创建完成\n");
}

// 清理资源
void game_2048_cleanup(void)
{
    if (game_state.main_container) {
        lv_obj_del(game_state.main_container);
        memset(&game_state, 0, sizeof(game_state));
    }
} 