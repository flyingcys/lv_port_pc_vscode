#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <lvgl/lvgl.h>

#define SIZE 4
// #define CELL_SIZE 80
// #define PADDING 10

static lv_obj_t * game_parent;
static lv_obj_t * score_label;
static lv_obj_t * best_label;
static lv_obj_t * grid_container;
static lv_obj_t * home, *new_game, *undo;

static lv_obj_t * grid_obj[SIZE][SIZE];
static lv_obj_t * grid_label[SIZE][SIZE];

static uint32_t grid_value[SIZE][SIZE] = {0};
static uint32_t score = 0, best = 0;

static lv_style_t title_style;
static void _new_game_cb(lv_event_t * e);

/*
todo list:
0. 优化布局
1. 支持回退
2. 增加home键，支持设置方格数量
3. 支持总分保存
4. 支持保存当前状态
5. 支持动画
*/

static int _get_value_color(uint32_t value)
{
    int color;

    switch (value) {
        case 0: color = 0xCDC1B4; break;
        case 2: color = 0xEEE4DA; break;
        case 4: color = 0xEDE0C8; break;
        case 8: color = 0xF2B179; break;
        case 16: color = 0xF59563; break;
        case 32: color = 0xF67C5F; break;
        case 64: color = 0xF65E3B; break;
        case 128: color = 0xEDCF72; break;
        case 256: color = 0xEDCC61; break;
        case 512: color = 0xEDC850; break;
        case 1024: color = 0xEDC53F; break;
        case 2048: color = 0xEDC22E; break;

        default:
            color = 0;
            break;
    }

    return color;
}

static int _is_game_over(void)
{
    // 找出所有空格子
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            if(grid_value[i][j] == 0) {
                return 0;
            }
        }
    }

    // 检查是否有相邻的相同数字
    for(int i = 0; i < SIZE; i ++) {
        for(int j = 0; j < SIZE; j ++) {
            if (j < SIZE -1 && grid_value[i][j] == grid_value[i][j + 1]) {
                return 0;
            }

            if (i < SIZE - 1 && grid_value[i][j] == grid_value[i + 1][j]) {
                return 0;
            }

        }
    }

    return 1;
}

static void _add_random_value(void)
{
    int empty_cells[SIZE*SIZE][2];
    int count = 0;
    
    // 找出所有空格子
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            if(grid_value[i][j] == 0) {
                empty_cells[count][0] = i;
                empty_cells[count][1] = j;
                count ++;
            }
        }
    }
    
    if(count > 0) {
        int index = rand() % count;
        int value = (rand() % 10 == 0) ? 4 : 2; // 10%概率生成4，90%概率生成2
        grid_value[empty_cells[index][0]][empty_cells[index][1]] = value;
    }
}

static void _game_update(void)
{
    uint32_t row, col;

    for (row = 0; row < SIZE; row ++) {
    for (col = 0; col < SIZE; col ++) {
            uint32_t value = grid_value[row][col];
            printf("%d ", value);
            if (value) {
                lv_label_set_text_fmt(grid_label[row][col], "%d", value);                                       // 设置数字文本

                lv_obj_set_style_bg_color(grid_obj[row][col], lv_color_hex(_get_value_color(value)), 0);        // 设置背景颜色（使用前面_get_value_color函数）

                // 设置文字颜色（小数字用深灰褐色，大数字用白色）
                if (value <= 4)
                    lv_obj_set_style_text_color(grid_label[row][col], lv_color_hex(0x776E65), 0);             // 深灰褐色
                else
                    lv_obj_set_style_text_color(grid_label[row][col], lv_color_white(), 0);                   // 白色
            } else {
                // 空格子处理
                lv_label_set_text(grid_label[row][col], "");
                lv_obj_set_style_bg_color(grid_obj[row][col], lv_color_hex(0xCDC1B4), 0);                   // 浅米色
            }
        }
        printf("\n");
    }

    lv_label_set_text_fmt(score_label, "Score\n%d", score);
    
    // 更新最高分
    if (score > best) {
        best = score;
        lv_label_set_text_fmt(best_label, "Best\n%d", best);
    }
}

static void grid_value_calc(uint8_t dir)
{
    uint32_t col, row;
    uint8_t moved = 0;
    uint8_t merged[SIZE][SIZE] = {0};

    /*
        1. 找到有值的方块
        2. 下一个格子值为0，则移动过去
        3. 下一个格子相等，则合并，并标记该方块已经被合并
    */
    if (dir == LV_DIR_LEFT || dir == LV_DIR_RIGHT) {
        for (int i = 0; i < SIZE; i ++) {               // 遍历网格
            for (int j = (dir == LV_DIR_LEFT) ? 1 : SIZE - 2; (dir == LV_DIR_LEFT) ? (j < SIZE) : (j >= 0); (dir == LV_DIR_LEFT) ? (j ++) : (j --)) {
                if (grid_value[i][j] != 0) {
                    int col = j;
                    while (1) {
                        int next_col = col + (dir == LV_DIR_LEFT ? -1 : 1);
                        if (next_col < 0 || next_col >= SIZE)
                            break;
                        
                        if (grid_value[i][next_col] == 0) {
                            grid_value[i][next_col] = grid_value[i][col];
                            grid_value[i][col] = 0;
                            col = next_col;
                            moved = 1;
                        } else if (grid_value[i][next_col] == grid_value[i][col] && !merged[i][next_col]) {
                            grid_value[i][next_col] *= 2;
                            score += grid_value[i][next_col];
                            grid_value[i][col] = 0;
                            merged[i][next_col] = 1;
                            moved = 1;
                            break;
                        } else
                            break;
                    }
                }
            }
        }
    }

    if (dir == LV_DIR_TOP || dir == LV_DIR_BOTTOM) {
        for (int j = 0; j < SIZE; j ++) {
            for (int i = (dir == LV_DIR_TOP) ? 1 : (SIZE - 2); (dir == LV_DIR_TOP) ? (i < SIZE) : (i >= 0); (dir == LV_DIR_TOP) ? (i ++) : (i --)) {
                if (grid_value[i][j] != 0) {
                    int row = i;
                    while (1) {
                        int next_row = row + (dir == LV_DIR_TOP ? -1 : 1);
                        if (next_row < 0 || next_row >= SIZE) 
                            break;
                        
                        if (grid_value[next_row][j] == 0) {
                            grid_value[next_row][j] = grid_value[row][j];
                            grid_value[row][j] = 0;
                            row = next_row;
                            moved = 1;
                        } else if (grid_value[next_row][j] == grid_value[row][j] && !merged[next_row][j]) {
                            grid_value[next_row][j] *= 2;
                            score += grid_value[next_row][j];
                            grid_value[row][j] = 0;
                            merged[next_row][j] = 1;
                            moved = 1;
                            break;
                        } else 
                            break;
                    }
                }
            }
        }
    }


    if (moved) {
        _add_random_value();
    }

    _game_update();
}

static void event_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    LV_UNUSED(label);
    LV_LOG_USER("Button %s clicked", lv_label_get_text(label));
}

static void _show_game_over_msg(void)
{
    lv_obj_t * mbox = lv_msgbox_create(NULL);

    lv_msgbox_add_title(mbox, "Game Over");
    lv_msgbox_add_text(mbox, "No more moves!\nFinal Score:");
    
    // 添加得分显示
    lv_obj_t * score_info = lv_label_create(mbox);
    lv_label_set_text_fmt(score_info, "%d", score);
    lv_obj_set_style_text_color(score_info, lv_color_hex(0xF67C5F), 0);
    
    lv_msgbox_add_close_button(mbox);

    lv_obj_t * btn;
    btn = lv_msgbox_add_footer_button(mbox, "New Game");
    lv_obj_add_event_cb(btn, _new_game_cb, LV_EVENT_CLICKED, NULL);
    btn = lv_msgbox_add_footer_button(mbox, "Close");
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);  
}


static void grid_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    uint8_t dir;

    if (code == LV_EVENT_GESTURE) {
        lv_indev_t * indev = lv_indev_active();
        if (indev) {
            dir = lv_indev_get_gesture_dir(indev);

            switch (lv_indev_get_gesture_dir(indev)) {
                case LV_DIR_LEFT:
                    printf("Swipe LEFT\n");
                    break;

                case LV_DIR_RIGHT:
                    printf("Swipe RIGHT\n");
                    break;

                case LV_DIR_TOP:
                    printf("Swipe UP\n");
                    break;

                case LV_DIR_BOTTOM:
                    printf("Swipe DOWN\n");
                    break;

                default:
                    return; // Invalid direction
            }

            // 执行移动
            grid_value_calc(dir);
            
            // 移动后检查游戏是否结束
            if (_is_game_over()) {
                printf("Game Over\n");
                _show_game_over_msg(); 
            }
        }
    }
}

static void game_2048_init(void)
{
    // 清空网格
    for (uint32_t i = 0; i < SIZE; i ++) {
        for (uint32_t j = 0; j < SIZE; j ++) {
            grid_value[i][j] = 0;
        }
    }

    // 初始化随机数种子
    srand(time(NULL));

    // 重置分数
    score = 0;

    // 随机添加两个初始数字（2或4）
    _add_random_value();
    _add_random_value();
}

static void _new_game_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        printf("new game\n");

        game_2048_init();

        _game_update();
    }
}


static void game_2048_create_grid(lv_obj_t *parent)
{
    static int32_t col_dsc[SIZE + 1] = {0};
    static int32_t row_dsc[SIZE + 1] = {0};

    // 获取当前屏幕参数
    int32_t screen_w = lv_obj_get_width(parent);
    int32_t screen_h = lv_obj_get_height(parent);

    printf("screen_w:%d, screen_h:%d\n", screen_w, screen_h);

    // 创建主窗口
    lv_obj_t * main_windows = lv_obj_create(parent);
    lv_obj_set_size(main_windows, screen_w, screen_h);
    lv_obj_set_style_border_width(main_windows, 0, LV_PART_MAIN);               // 设置边框宽度为0
    lv_obj_set_style_bg_color(main_windows, lv_color_hex(0xFAF8EF), 0);         // 设置浅米色背景

    lv_obj_set_scrollbar_mode(main_windows, LV_SCROLLBAR_MODE_OFF);             // 完全禁用滚动条
    lv_obj_remove_flag(main_windows, LV_OBJ_FLAG_SCROLLABLE);                   // 完全禁止滚动/拖动

    // 样式设置
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_color(&style, lv_color_hex(0xCDC1B4));                  // 设置背景色
    lv_style_set_border_width(&style, 0);                                   // 设置样式对象的边框宽度为 0（无边框）
    lv_style_set_pad_all(&style, 10);                                       // 添加内边距
    lv_style_set_radius(&style, 5);                                        // 设置圆角半径为 5px

    // 极简直接布局 - 让grid严格居中，顶部为控件预留空间
    
    // 固定参数
    int32_t top_space = 120;    // 顶部预留控件空间
    int32_t min_margin = 20;    // 最小边距
    int32_t padding = 10;       // 固定单元格间距
    
    // 计算grid最大可能大小 - 确保方形
    int32_t max_size = LV_MIN(screen_w, screen_h - top_space);
    max_size -= 2 * min_margin; // 减去最小边距
    
    // 计算单元格大小
    int32_t cell_size = (max_size - padding * (SIZE + 1)) / SIZE;
    cell_size = LV_MAX(35, cell_size); // 确保至少35px
    
    // 计算实际grid尺寸
    int32_t actual_grid_size = cell_size * SIZE + padding * (SIZE + 1);
    
    // 水平居中
    int32_t grid_x = (screen_w - actual_grid_size) / 2;
    
    // 垂直位置 - 精确计算
    int32_t remaining_space = screen_h - top_space - actual_grid_size;
    int32_t grid_y;
    
    // 检查：是否还有足够的垂直空间？
    if (remaining_space >= 2 * min_margin) {
        // 有足够空间，在剩余空间中完美居中
        grid_y = top_space + remaining_space / 2;
    } else {
        // 空间不够，仅保留最小边距
        grid_y = top_space + min_margin;
        
        // 如果空间真的不够，可能需要进一步减小grid尺寸
        if (screen_h - grid_y - actual_grid_size < min_margin) {
            actual_grid_size = screen_h - grid_y - min_margin;
            cell_size = (actual_grid_size - padding * (SIZE + 1)) / SIZE;
            actual_grid_size = cell_size * SIZE + padding * (SIZE + 1);
        }
    }
    
    // 最终验证：确保上下边距真的相等
    int32_t top_margin = grid_y - top_space;  // 上边距（不包括顶部控件区域）
    int32_t bottom_margin = screen_h - grid_y - actual_grid_size;  // 下边距

    // 更新grid描述符（使用最终的cell_size）
    for (int i = 0; i < SIZE; i ++) {
        col_dsc[i] = cell_size;                 // 每列宽度
        row_dsc[i] = cell_size;                 // 每行高度
    }
    col_dsc[SIZE] = LV_GRID_TEMPLATE_LAST;      // 结束标记
    row_dsc[SIZE] = LV_GRID_TEMPLATE_LAST;

    // 创建grid容器 - 使用精确定位
    grid_container = lv_obj_create(main_windows);
    
    // 重要：先设置样式再设置尺寸和位置
    lv_obj_set_style_grid_column_dsc_array(grid_container, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(grid_container, row_dsc, 0);
    lv_obj_set_style_pad_all(grid_container, padding, 0);  // 内边距
    lv_obj_set_style_pad_gap(grid_container, padding, 0);  // 单元格间距
    
    // 禁用所有可能影响布局的标志
    lv_obj_add_flag(grid_container, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_remove_flag(grid_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(grid_container, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scrollbar_mode(grid_container, LV_SCROLLBAR_MODE_OFF);
    
    // 设置最终尺寸和位置
    lv_obj_set_size(grid_container, actual_grid_size, actual_grid_size);
    lv_obj_set_pos(grid_container, grid_x, grid_y);

    // 布局顶部控件 - 使用绝对坐标确保精确定位
    int32_t grid_left = grid_x;
    int32_t grid_right = grid_x + actual_grid_size;
    
    // 标准按钮尺寸
    int32_t std_btn_width = 70;
    int32_t std_btn_height = 50;
    int32_t small_btn_height = 40;
    int32_t btn_spacing = 10;
    
    // 2048 Logo - 与grid左边沿精确对齐
    lv_style_init(&title_style);
    lv_style_set_text_color(&title_style, lv_color_hex(0x5F5A51));
    lv_style_set_text_font(&title_style, &lv_font_montserrat_48);
    
    lv_obj_t * title = lv_label_create(main_windows);
    lv_obj_add_flag(title, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_label_set_text(title, "2048");
    lv_obj_add_style(title, &title_style, 0);
    lv_obj_set_pos(title, grid_left, 10);  // 精确与grid左对齐

    // home 键 - 在logo下方左对齐
    home = lv_button_create(main_windows);
    lv_obj_add_flag(home, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_size(home, std_btn_height, std_btn_height);  // 正方形按钮
    lv_obj_set_pos(home, grid_left, 70);  // 直接使用绝对位置
    lv_obj_add_event_cb(home, _new_game_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_style(home, &style, 0);

    lv_obj_t *home_label = lv_label_create(home);
    lv_label_set_text(home_label, LV_SYMBOL_HOME);
    lv_obj_set_style_text_font(home_label, &lv_font_montserrat_24, 0);
    lv_obj_set_align(home_label, LV_ALIGN_CENTER);
    
    // 右上角控件精确布局
    
    // 最高分数（最右边）- 与grid右边沿完全对齐
    lv_obj_t * best_score = lv_button_create(main_windows);
    lv_obj_set_size(best_score, std_btn_width, std_btn_height);
    lv_obj_add_flag(best_score, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_style(best_score, &style, 0);
    lv_obj_set_pos(best_score, grid_right - std_btn_width, 10);  // 精确与grid右对齐
   
    best_label = lv_label_create(best_score);
    lv_label_set_text_fmt(best_label, "Best\n%d", best);
    lv_label_set_long_mode(best_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_align(best_label, LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(best_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(best_label, lv_color_white(), 0);

    // 当前分数 - 在best左侧
    lv_obj_t * current_score = lv_button_create(main_windows);
    lv_obj_set_size(current_score, std_btn_width, std_btn_height);
    lv_obj_add_flag(current_score, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_style(current_score, &style, 0);
    lv_obj_set_pos(current_score, grid_right - std_btn_width * 2 - btn_spacing, 10);

    score_label = lv_label_create(current_score);
    lv_label_set_text_fmt(score_label, "Score\n%d", score);
    lv_label_set_long_mode(score_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_align(score_label, LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(score_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(score_label, lv_color_white(), 0);

    // 撤销按钮 - 精确位置
    undo = lv_button_create(main_windows);
    lv_obj_set_size(undo, std_btn_width, small_btn_height);
    lv_obj_add_flag(undo, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_style(undo, &style, 0);
    lv_obj_set_pos(undo, grid_right - std_btn_width * 2 - btn_spacing, 10 + std_btn_height + 5);
   
    lv_obj_t * undo_label = lv_label_create(undo);
    lv_label_set_text(undo_label, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_font(undo_label, &lv_font_montserrat_20, 0);
    lv_obj_set_align(undo_label, LV_ALIGN_CENTER);

    // 刷新按钮 - 精确位置
    new_game = lv_button_create(main_windows);
    lv_obj_set_size(new_game, std_btn_width, small_btn_height);
    lv_obj_add_flag(new_game, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_event_cb(new_game, _new_game_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_style(new_game, &style, 0);
    lv_obj_set_pos(new_game, grid_right - std_btn_width, 10 + std_btn_height + 5);
   
    lv_obj_t * refresh_label = lv_label_create(new_game);
    lv_label_set_text(refresh_label, LV_SYMBOL_REFRESH);
    lv_obj_set_style_text_font(refresh_label, &lv_font_montserrat_20, 0);
    lv_obj_set_align(refresh_label, LV_ALIGN_CENTER);

    lv_obj_set_layout(grid_container, LV_LAYOUT_GRID);                          // 将布局模式设为LV_LAYOUT_GRID以支持网格排列。
    lv_obj_set_style_pad_all(grid_container, padding, 0);                       // 统一内边距
    lv_obj_set_style_bg_color(grid_container, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_radius(grid_container, 6, LV_PART_MAIN);                    // 设置圆角半径为 6px  

    lv_obj_set_scrollbar_mode(grid_container, LV_SCROLLBAR_MODE_OFF);           // 完全禁用滚动条
    lv_obj_remove_flag(grid_container, LV_OBJ_FLAG_SCROLLABLE);                 // 完全禁止滚动/拖动

    // 网格样式
    static lv_style_t cell_style;
    lv_style_init(&cell_style);
    lv_style_set_radius(&cell_style, 3);
    lv_style_set_border_width(&cell_style, 0);

    for (uint32_t col = 0; col < SIZE; col ++) {
        for (uint32_t row = 0; row < SIZE; row ++) {
            grid_obj[col][row] = lv_button_create(grid_container);
            lv_obj_remove_flag(grid_obj[col][row], LV_OBJ_FLAG_CLICKABLE);                          // 禁用点击
            lv_obj_add_style(grid_obj[col][row], &cell_style, 0);                                   // 应用样式

            lv_obj_set_style_bg_color(grid_obj[col][row], lv_color_hex(0xCDC1B4), 0);               // 设置按钮背景色
            
            // 设置网格位置
            lv_obj_set_grid_cell(grid_obj[col][row], LV_GRID_ALIGN_STRETCH, col, 1,
                                LV_GRID_ALIGN_STRETCH, row, 1);

            grid_label[col][row] = lv_label_create(grid_obj[col][row]);
            lv_label_set_text(grid_label[col][row], "");                                            // 默认为空

            lv_obj_center(grid_label[col][row]);
            
            // 根据格子大小选择合适的字体
            if (cell_size >= 100) {
                lv_obj_set_style_text_font(grid_label[col][row], &lv_font_montserrat_48, 0);
            } else if (cell_size >= 80) {
                lv_obj_set_style_text_font(grid_label[col][row], &lv_font_montserrat_32, 0);
            } else {
                lv_obj_set_style_text_font(grid_label[col][row], &lv_font_montserrat_24, 0);
            }
        }
    }
}

void game_2048(lv_obj_t *parent)
{
    game_parent = parent;
    game_2048_init();
    
    game_2048_create_grid(parent);
 
    _game_update();

    lv_obj_add_event_cb(parent, grid_event_cb, LV_EVENT_ALL, NULL);                // 添加事件回调
}