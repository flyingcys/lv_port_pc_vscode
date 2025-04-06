#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <lvgl/lvgl.h>

#define SIZE 4
#define CELL_SIZE 80
#define PADDING 10

static lv_obj_t * game_parent;
static lv_obj_t * score_label;
static lv_obj_t * grid_container;

static lv_obj_t * grid_obj[SIZE][SIZE];
static lv_obj_t * grid_label[SIZE][SIZE];

static uint32_t grid_value[SIZE][SIZE] = {0};
static uint32_t score = 0;


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
    uint32_t col, row;

    for (col = 0; col < SIZE; col ++) {
        for (row = 0; row < SIZE; row ++) {
            uint32_t value = grid_value[col][row];
            printf("%d ", value);
            if (value) {
                lv_label_set_text_fmt(grid_label[row][col], "%d", value);
                lv_obj_set_style_bg_color(grid_obj[row][col], lv_color_hex(_get_value_color(value)), 0);

                if (value <= 4)
                    lv_obj_set_style_text_color(grid_obj[row][col], lv_color_black(), 0);
                else
                    lv_obj_set_style_text_color(grid_obj[row][col], lv_color_white(), 0);
            } else {
                lv_label_set_text(grid_label[row][col], "");
                lv_obj_set_style_bg_color(grid_obj[row][col], lv_color_hex(0xCDC1B4), 0);
            }
        }
        printf("\n");
    }

    lv_label_set_text_fmt(score_label, "Score:%d", score);
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
        for (int i = 0; i < SIZE; i ++) {
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
    // lv_obj_t * msg_box = lv_msgbox_create(game_parent);
    // lv_msgbox_add_text(msg_box, "Game Over");  
    lv_obj_t * mbox1 = lv_msgbox_create(NULL);

    lv_msgbox_add_title(mbox1, "Hello");

    lv_msgbox_add_text(mbox1, "This is a message box with two buttons.");
    lv_msgbox_add_close_button(mbox1);

    lv_obj_t * btn;
    btn = lv_msgbox_add_footer_button(mbox1, "Apply");
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);
    btn = lv_msgbox_add_footer_button(mbox1, "Cancel");
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
                    printf("左滑\n");
                    break;

                case LV_DIR_RIGHT:
                    printf("右滑\n");
                    break;

                case LV_DIR_TOP:
                    printf("上滑\n");
                    break;

                case LV_DIR_BOTTOM:
                    printf("下滑\n");
                    break;

                default:
                    break;
            }

            if (_is_game_over()) {
                printf("游戏结束\n");
                _show_game_over_msg(); 
                return;
            }

            grid_value_calc(dir);


        }

    }
}

static void game_2048_init(void)
{
    for (uint32_t i = 0; i < SIZE; i ++) {
        for (uint32_t j = 0; j < SIZE; j ++) {
            grid_value[i][j] = 0;
        }
    }

    // 初始化随机数种子
    srand(time(NULL));

    // 添加两个随机数
    _add_random_value();
    _add_random_value();

    score = 0;
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

    for (int i = 0; i < SIZE; i ++) {
        col_dsc[i] = CELL_SIZE;                 // 每列宽度
        row_dsc[i] = CELL_SIZE;                 // 每行高度
    }
    col_dsc[SIZE] = LV_GRID_TEMPLATE_LAST;      // 结束标记
    row_dsc[SIZE] = LV_GRID_TEMPLATE_LAST;

    /*Create a container with grid*/
    grid_container = lv_obj_create(parent);
    lv_obj_set_style_grid_column_dsc_array(grid_container, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(grid_container, row_dsc, 0);

    lv_obj_set_size(grid_container, CELL_SIZE * SIZE + PADDING * (SIZE + 1), CELL_SIZE * SIZE + PADDING * (SIZE + 1));
    lv_obj_center(grid_container);

    lv_obj_set_layout(grid_container, LV_LAYOUT_GRID);                          // 将布局模式设为LV_LAYOUT_GRID以支持网格排列。
    lv_obj_set_style_pad_all(grid_container, PADDING, 0);                     // 统一内边距
    lv_obj_set_style_bg_color(grid_container, lv_color_hex(0xBBADA0), 0);

    lv_obj_set_scrollbar_mode(grid_container, LV_SCROLLBAR_MODE_OFF);       // 完全禁用滚动条
    lv_obj_remove_flag(grid_container, LV_OBJ_FLAG_SCROLLABLE);             // 完全禁止滚动/拖动


    for (uint32_t col = 0; col < SIZE; col ++) {
        for (uint32_t row = 0; row < SIZE; row ++) {
            grid_obj[col][row] = lv_button_create(grid_container);
            lv_obj_remove_flag(grid_obj[col][row], LV_OBJ_FLAG_CLICKABLE);                          // 禁用点击

            lv_obj_set_style_bg_color(grid_obj[col][row], lv_color_hex(0xCDC1B4), 0);               // 设置按钮背景色
            /*Stretch the cell horizontally and vertically too
            *Set span to 1 to make the cell 1 column/row sized*/
            lv_obj_set_grid_cell(grid_obj[col][row], LV_GRID_ALIGN_STRETCH, col, 1,
                                LV_GRID_ALIGN_STRETCH, row, 1);

            grid_label[col][row] = lv_label_create(grid_obj[col][row]);
            lv_label_set_text(grid_label[col][row], "");
            // lv_label_set_text_fmt(grid_label[col][row], "c%d:r%d", col, row);
            lv_obj_center(grid_label[col][row]);
            lv_obj_set_style_text_font(grid_label[col][row], &lv_font_montserrat_48, 0);
        }
    }

    lv_obj_t * title = lv_label_create(parent);
    lv_obj_add_flag(title, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_label_set_text(title, "2048");
    lv_obj_set_align(title, LV_ALIGN_OUT_TOP_LEFT);

    lv_obj_t * new_game = lv_button_create(parent);
    lv_obj_add_event_cb(new_game, _new_game_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_align(new_game, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t * btn_label = lv_label_create(new_game);
    lv_label_set_text(btn_label, "New Game");
    lv_obj_center(btn_label);
    
    score_label = lv_label_create(parent);
    lv_label_set_text_fmt(score_label, "Score:%d", score);
    lv_obj_align(score_label, LV_ALIGN_TOP_RIGHT, -10, 10);
}


void game_2048(lv_obj_t *parent)
{
    game_parent = parent;
    game_2048_init();

    game_2048_create_grid(parent);
 
    _game_update();

    lv_obj_add_event_cb(parent, grid_event_cb, LV_EVENT_ALL, NULL);                // 添加事件回调
}