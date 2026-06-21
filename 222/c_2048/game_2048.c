#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>

#define SIZE 4
#define CELL_WIDTH 8

// ANSI颜色代码
#define COLOR_RESET   "\x1B[0m"
#define COLOR_BLACK   "\x1B[30m"
#define COLOR_RED     "\x1B[31m"
#define COLOR_GREEN   "\x1B[32m"
#define COLOR_YELLOW  "\x1B[33m"
#define COLOR_BLUE    "\x1B[34m"
#define COLOR_MAGENTA "\x1B[35m"
#define COLOR_CYAN    "\x1B[36m"
#define COLOR_WHITE   "\x1B[37m"

// ANSI背景色代码
#define BG_BLACK   "\x1B[40m"
#define BG_RED     "\x1B[41m"
#define BG_GREEN   "\x1B[42m"
#define BG_YELLOW  "\x1B[43m"
#define BG_BLUE    "\x1B[44m"
#define BG_MAGENTA "\x1B[45m"
#define BG_CYAN    "\x1B[46m"
#define BG_WHITE   "\x1B[47m"

// 游戏状态
int board[SIZE][SIZE] = {0};
int score = 0;
struct termios orig_termios;

// 函数声明
void init_game(void);
void draw_board(void);
int get_key(void);
void add_random_tile(void);
int move(int direction);
int game_over(void);
void disable_raw_mode(void);
void enable_raw_mode(void);
const char* get_color(int value);
void clear_screen(void);

// 初始化终端为原始模式
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// 设置终端为无缓冲模式
void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// 清屏
void clear_screen() {
    printf("\x1B[2J\x1B[H");
}

// 获取数字对应的颜色
const char* get_color(int value) {
    switch(value) {
        case 2:    return BG_WHITE;
        case 4:    return BG_CYAN;
        case 8:    return BG_GREEN;
        case 16:   return BG_YELLOW;
        case 32:   return BG_MAGENTA;
        case 64:   return BG_RED;
        case 128:  return BG_BLUE;
        case 256:  return BG_GREEN;
        case 512:  return BG_YELLOW;
        case 1024: return BG_MAGENTA;
        case 2048: return BG_RED;
        default:   return BG_BLACK;
    }
}

// 初始化游戏
void init_game() {
    memset(board, 0, sizeof(board));
    score = 0;
    add_random_tile();
    add_random_tile();
}

// 绘制游戏板
void draw_board() {
    clear_screen();
    printf("\n\n  2048 游戏  分数: %d\n\n", score);
    
    // 打印上边框
    printf("  ╔");
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < CELL_WIDTH; j++) printf("═");
        if(i < SIZE-1) printf("╦");
    }
    printf("╗\n");
    
    // 打印每一行
    for(int i = 0; i < SIZE; i++) {
        printf("  ║");
        for(int j = 0; j < SIZE; j++) {
            if(board[i][j] == 0) {
                printf("%s%*s%s║", BG_BLACK, CELL_WIDTH, " ", COLOR_RESET);
            } else {
                printf("%s%*d%s║", get_color(board[i][j]), CELL_WIDTH, board[i][j], COLOR_RESET);
            }
        }
        printf("\n");
        
        // 打印行间分隔符
        if(i < SIZE-1) {
            printf("  ╠");
            for(int j = 0; j < SIZE; j++) {
                for(int k = 0; k < CELL_WIDTH; k++) printf("═");
                if(j < SIZE-1) printf("╬");
            }
            printf("╣\n");
        }
    }
    
    // 打印下边框
    printf("  ╚");
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < CELL_WIDTH; j++) printf("═");
        if(i < SIZE-1) printf("╩");
    }
    printf("╝\n\n");
    
    printf("  控制: ↑↓←→ 或 WASD\n");
    printf("  Q: 退出  R: 重新开始\n\n");
}

// 获取按键输入
int get_key() {
    char c;
    read(STDIN_FILENO, &c, 1);
    
    if(c == '\x1B') {
        char seq[2];
        if(read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1B';
        if(read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1B';
        
        if(seq[0] == '[') {
            switch(seq[1]) {
                case 'A': return 'w'; // 上
                case 'B': return 's'; // 下
                case 'C': return 'd'; // 右
                case 'D': return 'a'; // 左
            }
        }
    }
    return c;
}

// 添加随机数字
void add_random_tile() {
    int empty_cells[SIZE*SIZE][2];
    int count = 0;
    
    // 找出所有空格子
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            if(board[i][j] == 0) {
                empty_cells[count][0] = i;
                empty_cells[count][1] = j;
                count++;
            }
        }
    }
    
    if(count > 0) {
        int index = rand() % count;
        int value = (rand() % 10 == 0) ? 4 : 2; // 10%概率生成4，90%概率生成2
        board[empty_cells[index][0]][empty_cells[index][1]] = value;
    }
}

// 移动和合并
int move(int direction) {
    int moved = 0;
    int merged[SIZE][SIZE] = {0};
    
    // 0:上, 1:右, 2:下, 3:左
    if(direction == 0 || direction == 2) { // 上或下
        for(int j = 0; j < SIZE; j++) {
            for(int i = (direction == 0) ? 1 : SIZE-2; 
                (direction == 0) ? (i < SIZE) : (i >= 0); 
                (direction == 0) ? i++ : i--) {
                if(board[i][j] != 0) {
                    int row = i;
                    while(1) {
                        int next_row = row + (direction == 0 ? -1 : 1);
                        if(next_row < 0 || next_row >= SIZE) break;
                        
                        if(board[next_row][j] == 0) {
                            board[next_row][j] = board[row][j];
                            board[row][j] = 0;
                            row = next_row;
                            moved = 1;
                        }
                        else if(board[next_row][j] == board[row][j] && !merged[next_row][j]) {
                            board[next_row][j] *= 2;
                            score += board[next_row][j];
                            board[row][j] = 0;
                            merged[next_row][j] = 1;
                            moved = 1;
                            break;
                        }
                        else break;
                    }
                }
            }
        }
    }
    else { // 左或右
        for(int i = 0; i < SIZE; i++) {
            for(int j = (direction == 3) ? 1 : SIZE-2;
                (direction == 3) ? (j < SIZE) : (j >= 0);
                (direction == 3) ? j++ : j--) {
                if(board[i][j] != 0) {
                    int col = j;
                    while(1) {
                        int next_col = col + (direction == 3 ? -1 : 1);
                        if(next_col < 0 || next_col >= SIZE) break;
                        
                        if(board[i][next_col] == 0) {
                            board[i][next_col] = board[i][col];
                            board[i][col] = 0;
                            col = next_col;
                            moved = 1;
                        }
                        else if(board[i][next_col] == board[i][col] && !merged[i][next_col]) {
                            board[i][next_col] *= 2;
                            score += board[i][next_col];
                            board[i][col] = 0;
                            merged[i][next_col] = 1;
                            moved = 1;
                            break;
                        }
                        else break;
                    }
                }
            }
        }
    }
    return moved;
}

// 检查游戏是否结束
int game_over() {
    // 检查是否有空格子
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            if(board[i][j] == 0) return 0;
        }
    }
    
    // 检查是否有相邻的相同数字
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            if((j < SIZE-1 && board[i][j] == board[i][j+1]) ||
               (i < SIZE-1 && board[i][j] == board[i+1][j])) {
                return 0;
            }
        }
    }
    return 1;
}

int main() {
    // 初始化随机数生成器
    srand(time(NULL));
    
    // 设置终端
    enable_raw_mode();
    
    // 初始化游戏
    init_game();
    
    // 游戏主循环
    while(1) {
        draw_board();
        
        if(game_over()) {
            printf("\n  游戏结束！最终分数: %d\n\n", score);
            printf("  按'R'重新开始，按'Q'退出\n");
        }
        
        char c = get_key();
        int direction = -1;
        
        switch(c) {
            case 'w': case 'W': direction = 0; break; // 上
            case 'd': case 'D': direction = 1; break; // 右
            case 's': case 'S': direction = 2; break; // 下
            case 'a': case 'A': direction = 3; break; // 左
            case 'q': case 'Q': goto cleanup; // 退出
            case 'r': case 'R': // 重新开始
                init_game();
                continue;
        }
        
        if(direction != -1) {
            if(move(direction)) {
                add_random_tile();
            }
        }
    }
    
cleanup:
    // 恢复终端设置
    disable_raw_mode();
    clear_screen();
    return 0;
}