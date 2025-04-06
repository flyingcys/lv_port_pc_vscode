import tkinter as tk
import random
import copy
import math

class Game2048:
    def __init__(self):
        self.window = tk.Tk()
        self.window.title('2048游戏')
        self.window.resizable(False, False)
        
        # 游戏参数
        self.size = 4  # 4x4网格
        self.cell_size = 100  # 每个格子的大小
        self.padding = 10  # 格子之间的间距
        self.score = 0
        
        # 鼠标控制参数
        self.mouse_start_x = 0
        self.mouse_start_y = 0
        self.min_swipe_distance = 50  # 最小滑动距离
        
        # 创建分数显示和新游戏按钮
        self.top_frame = tk.Frame(self.window)
        self.top_frame.pack(pady=10)
        
        self.score_label = tk.Label(self.top_frame, text='分数: 0', font=('Arial', 20))
        self.score_label.pack(side=tk.LEFT, padx=20)
        
        self.new_game_btn = tk.Button(self.top_frame, text='新游戏', 
                                    command=self.new_game, font=('Arial', 15))
        self.new_game_btn.pack(side=tk.LEFT, padx=20)
        
        # 创建游戏画布
        canvas_size = self.size * (self.cell_size + self.padding) + self.padding
        self.canvas = tk.Canvas(self.window, width=canvas_size, height=canvas_size,
                              bg='#BBADA0')
        self.canvas.pack(padx=20, pady=20)
        
        # 初始化游戏网格
        self.grid = [[0] * self.size for _ in range(self.size)]
        self.cells = {}  # 存储格子的图形对象
        
        # 创建网格背景
        self.draw_grid()
        
        # 绑定键盘事件
        self.window.bind('<Key>', self.handle_keypress)
        
        # 绑定鼠标事件
        self.canvas.bind('<Button-1>', self.on_mouse_down)
        self.canvas.bind('<ButtonRelease-1>', self.on_mouse_up)
        
        # 开始新游戏
        self.new_game()
        
    def draw_grid(self):
        """绘制游戏网格背景"""
        for i in range(self.size):
            for j in range(self.size):
                x1 = j * (self.cell_size + self.padding) + self.padding
                y1 = i * (self.cell_size + self.padding) + self.padding
                x2 = x1 + self.cell_size
                y2 = y1 + self.cell_size
                self.cells[(i, j)] = self.canvas.create_rectangle(
                    x1, y1, x2, y2, fill='#CDC1B4', outline='')
                
    def get_color(self, value):
        """根据数字返回对应的颜色"""
        colors = {
            0: '#CDC1B4',
            2: '#EEE4DA',
            4: '#EDE0C8',
            8: '#F2B179',
            16: '#F59563',
            32: '#F67C5F',
            64: '#F65E3B',
            128: '#EDCF72',
            256: '#EDCC61',
            512: '#EDC850',
            1024: '#EDC53F',
            2048: '#EDC22E'
        }
        return colors.get(value, '#EDC22E')
    
    def update_grid_display(self):
        """更新网格显示"""
        for i in range(self.size):
            for j in range(self.size):
                value = self.grid[i][j]
                cell = self.cells[(i, j)]
                color = self.get_color(value)
                self.canvas.itemconfig(cell, fill=color)
                
                # 更新数字显示
                x = j * (self.cell_size + self.padding) + self.padding + self.cell_size/2
                y = i * (self.cell_size + self.padding) + self.padding + self.cell_size/2
                
                # 删除原有的数字
                for item in self.canvas.find_withtag(f'text_{i}_{j}'):
                    self.canvas.delete(item)
                
                if value != 0:
                    font_size = 40 if value < 100 else 30 if value < 1000 else 20
                    self.canvas.create_text(x, y, text=str(value),
                                         font=('Arial', font_size, 'bold'),
                                         tags=f'text_{i}_{j}',
                                         fill='#776E65' if value < 8 else '#F9F6F2')
    
    def new_game(self):
        """开始新游戏"""
        self.grid = [[0] * self.size for _ in range(self.size)]
        self.score = 0
        self.score_label.config(text=f'分数: {self.score}')
        self.add_new_tile()
        self.add_new_tile()
        self.update_grid_display()
    
    def add_new_tile(self):
        """在随机空位置添加新的数字（2或4）"""
        empty_cells = [(i, j) for i in range(self.size) for j in range(self.size)
                      if self.grid[i][j] == 0]
        if empty_cells:
            i, j = random.choice(empty_cells)
            self.grid[i][j] = 2 if random.random() < 0.9 else 4
    
    def move(self, direction):
        """移动格子
        direction: 0=上, 1=右, 2=下, 3=左
        """
        moved = False
        merged = [[False] * self.size for _ in range(self.size)]
        score_added = 0
        
        # 移动和合并
        if direction in [0, 2]:  # 上或下
            for j in range(self.size):
                for i in range(self.size) if direction == 0 else range(self.size-1, -1, -1):
                    if self.grid[i][j] != 0:
                        r = i
                        while True:
                            nr = r - 1 if direction == 0 else r + 1
                            if 0 <= nr < self.size:
                                if self.grid[nr][j] == 0:
                                    self.grid[nr][j] = self.grid[r][j]
                                    self.grid[r][j] = 0
                                    r = nr
                                    moved = True
                                elif (self.grid[nr][j] == self.grid[r][j] and 
                                      not merged[nr][j] and not merged[r][j]):
                                    self.grid[nr][j] *= 2
                                    score_added += self.grid[nr][j]
                                    self.grid[r][j] = 0
                                    merged[nr][j] = True
                                    moved = True
                                    break
                                else:
                                    break
                            else:
                                break
                                
        else:  # 左或右
            for i in range(self.size):
                for j in range(self.size) if direction == 3 else range(self.size-1, -1, -1):
                    if self.grid[i][j] != 0:
                        c = j
                        while True:
                            nc = c - 1 if direction == 3 else c + 1
                            if 0 <= nc < self.size:
                                if self.grid[i][nc] == 0:
                                    self.grid[i][nc] = self.grid[i][c]
                                    self.grid[i][c] = 0
                                    c = nc
                                    moved = True
                                elif (self.grid[i][nc] == self.grid[i][c] and 
                                      not merged[i][nc] and not merged[i][c]):
                                    self.grid[i][nc] *= 2
                                    score_added += self.grid[i][nc]
                                    self.grid[i][c] = 0
                                    merged[i][nc] = True
                                    moved = True
                                    break
                                else:
                                    break
                            else:
                                break
        
        if moved:
            self.score += score_added
            self.score_label.config(text=f'分数: {self.score}')
            self.add_new_tile()
            self.update_grid_display()
            
            # 检查游戏是否结束
            if self.is_game_over():
                tk.messagebox.showinfo('游戏结束', f'游戏结束！\n最终分数: {self.score}')
    
    def is_game_over(self):
        """检查游戏是否结束"""
        # 检查是否有空格子
        for i in range(self.size):
            for j in range(self.size):
                if self.grid[i][j] == 0:
                    return False
        
        # 检查是否有相邻的相同数字
        for i in range(self.size):
            for j in range(self.size):
                value = self.grid[i][j]
                if (j < self.size - 1 and self.grid[i][j+1] == value) or \
                   (i < self.size - 1 and self.grid[i+1][j] == value):
                    return False
        return True
    
    def handle_keypress(self, event):
        """处理键盘事件"""
        key_actions = {
            'Up': 0,
            'Right': 1,
            'Down': 2,
            'Left': 3,
            'w': 0,
            'd': 1,
            's': 2,
            'a': 3
        }
        
        if event.keysym in key_actions:
            self.move(key_actions[event.keysym])
            
    def on_mouse_down(self, event):
        """处理鼠标按下事件"""
        self.mouse_start_x = event.x
        self.mouse_start_y = event.y
        
    def on_mouse_up(self, event):
        """处理鼠标释放事件"""
        dx = event.x - self.mouse_start_x
        dy = event.y - self.mouse_start_y
        
        # 计算移动距离
        distance = math.sqrt(dx * dx + dy * dy)
        
        # 如果移动距离小于最小滑动距离，不处理
        if distance < self.min_swipe_distance:
            return
            
        # 判断移动方向
        # 如果水平移动距离大于垂直移动距离
        if abs(dx) > abs(dy):
            if dx > 0:
                self.move(1)  # 右
            else:
                self.move(3)  # 左
        else:
            if dy > 0:
                self.move(2)  # 下
            else:
                self.move(0)  # 上
    
    def run(self):
        """运行游戏"""
        self.window.mainloop()

if __name__ == '__main__':
    game = Game2048()
    game.run()