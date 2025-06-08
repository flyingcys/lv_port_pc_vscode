#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <lvgl/lvgl.h>

#define SIZE 4

static lv_obj_t * game_parent;
static lv_obj_t * score_label;
static lv_obj_t * best_label;
static lv_obj_t * grid_container;
static lv_obj_t * home_btn, *refresh_btn;

static lv_obj_t * grid_obj[SIZE][SIZE];
static lv_obj_t * grid_label[SIZE][SIZE];

static uint32_t grid_value[SIZE][SIZE] = {0};
static uint32_t score = 5888, best = 37784;

static lv_style_t title_style, button_style, grid_style;

// 根据数值获取对应颜色
static int _get_value_color(uint32_t value)
{
    int color;
    switch (value) {
        case 0: color = 0xCDC1B4; break;        // 空格子 - 浅米色
        case 2: color = 0xEEE4DA; break;        // 2 - 浅黄色
        case 4: color = 0xEDE0C8; break;        // 4 - 浅橙色  
        case 8: color = 0xF2B179; break;        // 8 - 橙色
        case 16: color = 0xF59563; break;       // 16 - 深橙色
        case 32: color = 0xF67C5F; break;       // 32 - 红橙色
        case 64: color = 0xF65E3B; break;       // 64 - 红色
        case 128: color = 0xEDCF72; break;      // 128 - 金黄色
        case 256: color = 0xEDCC61; break;      // 256 - 深金黄色
        case 512: color = 0xEDC850; break;      // 512 - 更深金黄色
        case 1024: color = 0xEDC53F; break;     // 1024 - 深黄色
        case 2048: color = 0xEDC22E; break;     // 2048 - 最深黄色
        default: color = 0xEDC22E; break;       // 更大数值使用相同颜色
    }
    return color;
}

// 检查游戏是否结束
static int _is_game_over(void)
{
    // 检查是否有空格子
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            if(grid_value[i][j] == 0) {
                return 0;
            }
        }
    }

    // 检查是否有相邻的相同数字
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            if (j < SIZE - 1 && grid_value[i][j] == grid_value[i][j + 1]) {
                return 0;
            }
            if (i < SIZE - 1 && grid_value[i][j] == grid_value[i + 1][j]) {
                return 0;
            }
        }
    }
    return 1;
}

// 在空格子中随机添加2或4
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
                count++;
            }
        }
    }
    
    if(count > 0) {
        int index = rand() % count;
        int value = (rand() % 10 == 0) ? 4 : 2; // 10%概率生成4，90%概率生成2
        grid_value[empty_cells[index][0]][empty_cells[index][1]] = value;
    }
}

// 更新游戏界面
static void _game_update(void)
{
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {
            uint32_t value = grid_value[row][col];
            
            if (value > 0) {
                lv_label_set_text_fmt(grid_label[row][col], "%d", value);
                lv_obj_set_style_bg_color(grid_obj[row][col], lv_color_hex(_get_value_color(value)), 0);
                
                // 设置文字颜色（小数字用深灰褐色，大数字用白色）
                if (value <= 4)
                    lv_obj_set_style_text_color(grid_label[row][col], lv_color_hex(0x776E65), 0);
                else
                    lv_obj_set_style_text_color(grid_label[row][col], lv_color_white(), 0);
            } else {
                // 空格子处理
                lv_label_set_text(grid_label[row][col], "");
                lv_obj_set_style_bg_color(grid_obj[row][col], lv_color_hex(0xCDC1B4), 0);
            }
        }
    }

    // 更新分数显示
    lv_label_set_text_fmt(score_label, "得分\n%d", score);
    lv_label_set_text_fmt(best_label, "高分\n%d", best);
}

// 游戏移动逻辑
static void grid_value_calc(uint8_t dir)
{
    uint8_t moved = 0;
    uint8_t merged[SIZE][SIZE] = {0};

    if (dir == LV_DIR_LEFT || dir == LV_DIR_RIGHT) {
        for (int i = 0; i < SIZE; i++) {
            for (int j = (dir == LV_DIR_LEFT) ? 1 : SIZE - 2; 
                 (dir == LV_DIR_LEFT) ? (j < SIZE) : (j >= 0); 
                 (dir == LV_DIR_LEFT) ? (j++) : (j--)) {
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
        for (int j = 0; j < SIZE; j++) {
            for (int i = (dir == LV_DIR_TOP) ? 1 : (SIZE - 2); 
                 (dir == LV_DIR_TOP) ? (i < SIZE) : (i >= 0); 
                 (dir == LV_DIR_TOP) ? (i++) : (i--)) {
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

    // 更新最高分
    if (score > best) {
        best = score;
    }

    _game_update();
}

// 游戏结束消息框
static void _show_game_over_msg(void)
{
    lv_obj_t * mbox = lv_msgbox_create(NULL);
    lv_msgbox_add_title(mbox, "游戏结束");
    lv_msgbox_add_text(mbox, "无法继续移动!");
    lv_msgbox_add_close_button(mbox);
}

// 手势事件处理
static void grid_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_GESTURE) {
        lv_indev_t * indev = lv_indev_active();
        if (indev) {
            uint8_t dir = lv_indev_get_gesture_dir(indev);

            if (_is_game_over()) {
                _show_game_over_msg(); 
                return;
            }

            grid_value_calc(dir);
        }
    }
}

// 初始化游戏
static void game_2048_init(void)
{
    // 初始化游戏数据，设置与图片一致的初始状态
    for (uint32_t i = 0; i < SIZE; i++) {
        for (uint32_t j = 0; j < SIZE; j++) {
            grid_value[i][j] = 0;
        }
    }

    // 根据图片设置初始状态
    grid_value[0][0] = 512;    // 第一行第一列: 512
    grid_value[0][1] = 2;      // 第一行第二列: 2  
    grid_value[0][2] = 4;      // 第一行第三列: 4
    grid_value[0][3] = 2;      // 第一行第四列: 2
    
    grid_value[1][0] = 256;    // 第二行第一列: 256
    grid_value[1][1] = 16;     // 第二行第二列: 16
    grid_value[1][2] = 8;      // 第二行第三列: 8
    grid_value[1][3] = 2;      // 第二行第四列: 2
    
    grid_value[2][0] = 16;     // 第三行第一列: 16
    grid_value[2][1] = 8;      // 第三行第二列: 8
    // 第三行其他位置为空
    
    grid_value[3][0] = 2;      // 第四行第一列: 2
    // 第四行其他位置为空

    // 初始化随机数种子
    srand(time(NULL));
}

// 新游戏按钮回调
static void _new_game_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        score = 0;
        game_2048_init();
        _add_random_value();
        _add_random_value();
        _game_update();
    }
}

// 创建游戏界面
static void game_2048_create_ui(lv_obj_t *parent)
{
    // 获取屏幕尺寸
    int32_t screen_w = lv_obj_get_width(parent);
    int32_t screen_h = lv_obj_get_height(parent);

    // 创建主容器
    lv_obj_t * main_container = lv_obj_create(parent);
    lv_obj_set_size(main_container, screen_w, screen_h);
    lv_obj_set_style_border_width(main_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(main_container, lv_color_hex(0xFAF8EF), 0);
    lv_obj_set_scrollbar_mode(main_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(main_container, LV_OBJ_FLAG_SCROLLABLE);

    // 初始化样式
    lv_style_init(&title_style);
    lv_style_set_text_color(&title_style, lv_color_hex(0x776E65));
    lv_style_set_text_font(&title_style, &lv_font_montserrat_48);

    lv_style_init(&button_style);
    lv_style_set_bg_color(&button_style, lv_color_hex(0xBBADA0));
    lv_style_set_border_width(&button_style, 0);
    lv_style_set_radius(&button_style, 3);
    lv_style_set_pad_all(&button_style, 8);

    // 创建顶部容器来固定布局
    lv_obj_t * top_container = lv_obj_create(main_container);
    lv_obj_set_size(top_container, screen_w - 40, 120);  // 固定高度120px
    lv_obj_set_pos(top_container, 20, 20);
    lv_obj_set_style_border_width(top_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(top_container, LV_OPA_TRANSP, 0);  // 透明背景
    lv_obj_set_style_pad_all(top_container, 0, 0);

    // 创建2048标题
    lv_obj_t * title = lv_label_create(top_container);
    lv_label_set_text(title, "2048");
    lv_obj_add_style(title, &title_style, 0);
    lv_obj_set_pos(title, 0, 0);

    // 创建右上角容器，用flex布局
    lv_obj_t * right_top_container = lv_obj_create(top_container);
    lv_obj_set_size(right_top_container, 200, 80);
    lv_obj_set_pos(right_top_container, screen_w - 260, 0);
    lv_obj_set_style_border_width(right_top_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(right_top_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(right_top_container, 0, 0);
    lv_obj_set_layout(right_top_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(right_top_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(right_top_container, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(right_top_container, 10, 0);

    // 创建得分按钮
    lv_obj_t * score_btn = lv_obj_create(right_top_container);
    lv_obj_add_style(score_btn, &button_style, 0);
    lv_obj_set_size(score_btn, 80, 60);

    score_label = lv_label_create(score_btn);
    lv_label_set_text_fmt(score_label, "得分\n%d", score);
    lv_obj_set_style_text_align(score_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(score_label, lv_color_white(), 0);
    lv_obj_center(score_label);

    // 创建高分按钮
    lv_obj_t * best_btn = lv_obj_create(right_top_container);
    lv_obj_add_style(best_btn, &button_style, 0);
    lv_obj_set_size(best_btn, 80, 60);

    best_label = lv_label_create(best_btn);
    lv_label_set_text_fmt(best_label, "高分\n%d", best);
    lv_obj_set_style_text_align(best_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(best_label, lv_color_white(), 0);
    lv_obj_center(best_label);

    // 创建右下角按钮容器
    lv_obj_t * right_bottom_container = lv_obj_create(top_container);
    lv_obj_set_size(right_bottom_container, 120, 50);
    lv_obj_set_pos(right_bottom_container, screen_w - 140, 70);
    lv_obj_set_style_border_width(right_bottom_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(right_bottom_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(right_bottom_container, 0, 0);
    lv_obj_set_layout(right_bottom_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(right_bottom_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(right_bottom_container, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(right_bottom_container, 10, 0);

    // 创建刷新按钮
    refresh_btn = lv_btn_create(right_bottom_container);
    lv_obj_add_style(refresh_btn, &button_style, 0);
    lv_obj_set_size(refresh_btn, 50, 50);
    lv_obj_add_event_cb(refresh_btn, _new_game_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * refresh_icon = lv_label_create(refresh_btn);
    lv_label_set_text(refresh_icon, LV_SYMBOL_REFRESH);
    lv_obj_set_style_text_color(refresh_icon, lv_color_white(), 0);
    lv_obj_center(refresh_icon);

    // 创建返回按钮
    home_btn = lv_btn_create(right_bottom_container);
    lv_obj_add_style(home_btn, &button_style, 0);
    lv_obj_set_size(home_btn, 50, 50);

    lv_obj_t * home_icon = lv_label_create(home_btn);
    lv_label_set_text(home_icon, LV_SYMBOL_HOME);
    lv_obj_set_style_text_color(home_icon, lv_color_white(), 0);
    lv_obj_center(home_icon);

    // 创建游戏网格
    int32_t grid_size = LV_MIN(screen_w - 40, screen_h - 180);
    int32_t cell_size = (grid_size - 25) / 4;  // 减去间距
    
    grid_container = lv_obj_create(main_container);
    lv_obj_set_size(grid_container, grid_size, grid_size);
    lv_obj_set_pos(grid_container, 20, 160);  // 调整位置适应新的顶部布局
    lv_obj_set_style_bg_color(grid_container, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_border_width(grid_container, 0, 0);
    lv_obj_set_style_radius(grid_container, 6, 0);
    lv_obj_set_style_pad_all(grid_container, 5, 0);

    lv_style_init(&grid_style);
    lv_style_set_radius(&grid_style, 3);
    lv_style_set_border_width(&grid_style, 0);

    // 创建4x4网格
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {
            grid_obj[row][col] = lv_obj_create(grid_container);
            lv_obj_add_style(grid_obj[row][col], &grid_style, 0);
            lv_obj_set_size(grid_obj[row][col], cell_size, cell_size);
            lv_obj_set_pos(grid_obj[row][col], 
                           col * (cell_size + 5), 
                           row * (cell_size + 5));

            grid_label[row][col] = lv_label_create(grid_obj[row][col]);
            lv_obj_set_style_text_font(grid_label[row][col], &lv_font_montserrat_32, 0);
            lv_obj_center(grid_label[row][col]);
        }
    }

    // 添加手势事件
    lv_obj_add_event_cb(main_container, grid_event_cb, LV_EVENT_GESTURE, NULL);
}

// 主函数
void new_2048_game(lv_obj_t *parent)
{
    game_parent = parent;
    game_2048_init();
    game_2048_create_ui(parent);
    _game_update();
}