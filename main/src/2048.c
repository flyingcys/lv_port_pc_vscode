#include <string.h>
#include "lvgl/lvgl.h"

#define SIZE 4
#define CELL_SIZE 80
#define PADDING 10

static lv_obj_t *labels[SIZE][SIZE];
static int grid[SIZE][SIZE] = {{0}};
static int score = 0;
static lv_obj_t *score_label;

// 全局输入设备指针
static lv_indev_t *keyboard_indev = NULL;

// 颜色配置
static const lv_color_t colors[] = {
    LV_COLOR_MAKE(0xCC, 0xC0, 0xB3), // 0
    LV_COLOR_MAKE(0xEE, 0xE4, 0xDA), // 2
    LV_COLOR_MAKE(0xED, 0xE0, 0xC8), // 4
    LV_COLOR_MAKE(0xF2, 0xB1, 0x79), // 8
    LV_COLOR_MAKE(0xF5, 0x95, 0x63), // 16
    LV_COLOR_MAKE(0xF6, 0x7C, 0x5F), // 32
    LV_COLOR_MAKE(0xF6, 0x5E, 0x3B), // 64
    LV_COLOR_MAKE(0xED, 0xCF, 0x72), // 128
    LV_COLOR_MAKE(0xED, 0xCC, 0x61), // 256
    LV_COLOR_MAKE(0xED, 0xC8, 0x50), // 512
    LV_COLOR_MAKE(0xED, 0xC5, 0x3F), // 1024
    LV_COLOR_MAKE(0xED, 0xC2, 0x2E)  // 2048
};

// 游戏逻辑函数
static void add_new_tile();
static bool move_grid(int dx, int dy);
static void update_grid();
static bool game_over_check();

// 界面更新函数
static void update_ui();
static void create_grid_ui(lv_obj_t *parent);
static void create_score_label(lv_obj_t *parent);

// 事件处理
static void handle_key(lv_event_t *e);

// 初始化游戏
void game_init() {
    score = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = 0;
        }
    }
    add_new_tile();
    add_new_tile();
    update_ui();
}

// 创建新方块
static void add_new_tile() {
    int empty[SIZE * SIZE][2];
    int count = 0;

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (grid[i][j] == 0) {
                empty[count][0] = i;
                empty[count][1] = j;
                count++;
            }
        }
    }

    if (count > 0) {
        int index = rand() % count;
        grid[empty[index][0]][empty[index][1]] = (rand() % 10) < 9 ? 2 : 4;
    }
}

// 主界面创建
void game_create(lv_obj_t *parent) {
    create_score_label(parent);
    create_grid_ui(parent);
    game_init();

    // 添加键盘事件
    lv_obj_add_event_cb(parent, handle_key, LV_EVENT_KEY, NULL);
    lv_group_t *g = lv_group_create();
    lv_group_add_obj(g, parent);

    // 使用全局输入设备指针
    if (keyboard_indev) {
        lv_indev_set_group(keyboard_indev, g);
    }
}

static void create_score_label(lv_obj_t *parent) {
    score_label = lv_label_create(parent);
    lv_label_set_text(score_label, "Score: 0");
    lv_obj_align(score_label, LV_ALIGN_TOP_RIGHT, -10, 10);
}

static void create_grid_ui(lv_obj_t *parent) {
    lv_obj_t *grid_container = lv_obj_create(parent);
    lv_obj_set_size(grid_container,
                    CELL_SIZE * SIZE + PADDING * (SIZE + 1),
                    CELL_SIZE * SIZE + PADDING * (SIZE + 1));
    lv_obj_center(grid_container);
    lv_obj_set_layout(grid_container, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_all(grid_container, PADDING, 0);
    lv_obj_set_style_bg_color(grid_container, lv_color_hex(0xBBADA0), 0);

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            lv_obj_t *cell = lv_obj_create(grid_container);
            lv_obj_set_size(cell, CELL_SIZE, CELL_SIZE);
            lv_obj_set_style_bg_color(cell, colors[0], 0);
            lv_obj_set_style_radius(cell, 5, 0);

            labels[i][j] = lv_label_create(cell);
            lv_label_set_text(labels[i][j], "");
            lv_obj_center(labels[i][j]);
        }
    }
}

static void update_ui() {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            int value = grid[i][j];
            lv_label_set_text_fmt(labels[i][j], "%d", value);
            lv_obj_set_style_bg_color(lv_obj_get_parent(labels[i][j]),
                                     value ? colors[__builtin_ctz(value)] : colors[0], 0);
            lv_obj_set_style_text_color(labels[i][j],
                                       value > 4 ? lv_color_white() : lv_color_hex(0x776E65), 0);
        }
    }
    lv_label_set_text_fmt(score_label, "Score: %d", score);
}

// 移动处理函数
static bool move_grid(int dx, int dy) {
    bool moved = false;
    int new_grid[SIZE][SIZE] = {{0}};

    for (int i = 0; i < SIZE; i++) {
        int pos = 0;
        int prev = -1;
        for (int j = 0; j < SIZE; j++) {
            int x = dx ? (dx > 0 ? SIZE - 1 - j : j) : i;
            int y = dy ? (dy > 0 ? SIZE - 1 - j : j) : i;
            int value = grid[x][y];

            if (value == 0) continue;

            if (prev == value) {
                new_grid[dx ? x : pos - 1][dy ? y : pos - 1] = value * 2;
                score += value * 2;
                prev = -1;
                moved = true;
            } else {
                new_grid[dx ? x : pos][dy ? y : pos] = value;
                prev = value;
                pos++;
                if ((dx && x != (dx > 0 ? SIZE - 1 - pos + 1 : pos - 1)) ||
                    (dy && y != (dy > 0 ? SIZE - 1 - pos + 1 : pos - 1))) {
                    moved = true;
                }
            }
        }
    }

    if (moved) {
        memcpy(grid, new_grid, sizeof(grid));
        add_new_tile();
    }
    return moved;
}

// 键盘事件处理
static void handle_key(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        bool moved = false;

        switch (key) {
            case LV_KEY_UP:    moved = move_grid(0, -1); break;
            case LV_KEY_DOWN:  moved = move_grid(0, 1);  break;
            case LV_KEY_LEFT:  moved = move_grid(-1, 0); break;
            case LV_KEY_RIGHT: moved = move_grid(1, 0);  break;
        }

        if (moved) {
            update_ui();
            if (game_over_check()) {
                // 使用新的 LVGL 9 API 创建消息框
                lv_obj_t *mbox = lv_msgbox_create(NULL); // 创建消息框
                lv_msgbox_add_title(mbox, "Game Over");  // 添加标题
                lv_msgbox_add_text(mbox, "No more moves!"); // 添加内容
                lv_obj_center(mbox); // 居中显示
            }
        }
    }
}

// 游戏结束检查
static bool game_over_check() {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (grid[i][j] == 0) return false;
            if (i < SIZE - 1 && grid[i][j] == grid[i + 1][j]) return false;
            if (j < SIZE - 1 && grid[i][j] == grid[i][j + 1]) return false;
        }
    }
    return true;
}

// 主函数
// int main(void) {
//     lv_init();
//     // 初始化显示和输入设备...
//     input_init(); // 初始化输入设备

//     lv_obj_t *scr = lv_scr_act();
//     game_create(scr);

//     while (1) {
//         lv_timer_handler();
//         usleep(5000);
//     }
//     return 0;
// }