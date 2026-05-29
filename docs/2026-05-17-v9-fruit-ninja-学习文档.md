# `main/src/v9-fruit_ninja` 学习文档

## 1. 文档目标

这份文档只围绕 `main/src/v9-fruit_ninja` 目录里的实际实现来讲，不是泛泛介绍 Fruit Ninja 玩法，而是把这套 LVGL v9 版本的代码如何组织、如何启动、如何进入首页、如何开始游戏、如何判定切中、如何计分、如何结束，按照真实游戏流程一步一步拆开说明。

如果你准备继续维护这套代码，建议把下面 4 个文件当作主线反复对照阅读：

- `main/src/v9-fruit_ninja/fruit_ninja_scene.c`：主控文件，几乎所有流程编排都在这里。
- `main/src/v9-fruit_ninja/fruit_ninja_model.h`：所有核心运行时数据结构都在这里定义。
- `main/src/v9-fruit_ninja/fruit_ninja_input.c`：输入轨迹、刀痕、线段抽取。
- `main/src/v9-fruit_ninja/fruit_ninja_audio.c`：音频初始化与播放。

从职责上看，这个目录虽然拆成了多个文件，但真正的“游戏导演”只有一个：`fruit_ninja_scene.c`。其他文件基本都是给它提供资源、输入、碰撞和音频能力。

---

## 2. 目录结构与职责划分

`main/src/v9-fruit_ninja` 当前包含以下文件：

- `fruit_ninja.h`
  - 对外只暴露一个入口：`fruit_ninja_start()`。
- `fruit_ninja_model.h`
  - 定义游戏状态、果实定义、果实实例、碎片实例、刀痕、切割线段、全局游戏对象。
- `fruit_ninja_scene.c`
  - 真正的游戏主循环、状态切换、对象创建、动画推进、切水果判定、首页与结算逻辑。
- `fruit_ninja_input.c`
  - 管理鼠标/触摸拖拽轨迹，把连续点转换成“本帧新增的一条切割线段”。
- `fruit_ninja_collision.c`
  - 提供“线段是否命中圆形碰撞体”的数学判定。
- `fruit_ninja_assets.c`
  - 推导项目根目录，拼接图片/音频路径，校验核心资源是否存在。
- `fruit_ninja_audio.c`
  - 用 `SDL2_mixer` 初始化背景音乐和音效，提供播放接口。
- `fruit_ninja_assets.h` / `fruit_ninja_audio.h` / `fruit_ninja_collision.h`
  - 各模块头文件。

一个很关键的学习点是：

1. 这套代码的“视图对象”没有单独抽成 view 模块。
2. “生成果实”“物理更新”“状态机”“页面切换”都集中在 `fruit_ninja_scene.c`。
3. 所以阅读顺序不应该按文件名平均分配，而应该优先吃透 `fruit_ninja_scene.c`，再回头理解辅助模块。

---

## 3. 从程序入口看，Fruit Ninja 是怎么启动的

### 3.1 进程入口：`main()`

外层程序入口在 `main/src/main.c`。流程很简单：

1. `lv_init()` 初始化 LVGL。
2. `hal_init(640, 480)` 创建 SDL 窗口、鼠标、键盘等输入设备。
3. 调用 `fruit_ninja_start()`。
4. 进入 `while(1)`，不断执行 `lv_timer_handler()`。

所以 Fruit Ninja 并不是通过按钮跳转进来的，而是程序启动后直接装载这个游戏场景。

### 3.2 模块入口：`fruit_ninja_start()`

`fruit_ninja_start()` 是整个游戏的总初始化入口，可以把它理解成“开机流程”。它做了 8 件事：

1. 清空全局游戏对象 `g_game`。
2. 设置屏幕宽高为 `640x480`。
3. 首次启动时用 `time(NULL)` 做随机种子。
4. 初始化 UI 资源路径表 `g_ui_assets`。
5. 推导项目根目录并校验资源文件是否存在。
6. 初始化音频系统。
7. 创建整套静态场景对象。
8. 创建 `16ms` 定时器，驱动整个游戏循环。

最后调用：

- `lv_screen_load(g_game.screen)`：把游戏场景切到当前屏幕。
- `enter_home(&g_game)`：进入首页状态。

也就是说，`fruit_ninja_start()` 并不直接开始“游戏中”状态，而是先把所有基础设施搭好，再进入首页。

---

## 4. 先理解数据模型，否则后面流程容易乱

## 4.1 状态机：`fruit_ninja_state_t`

游戏一共只有 4 个主状态：

- `FRUIT_NINJA_STATE_HOME`
  - 首页展示状态。
- `FRUIT_NINJA_STATE_RUNNING`
  - 正常游戏状态，允许刷水果、切水果、记分、漏切。
- `FRUIT_NINJA_STATE_EXPLODING`
  - 切到炸弹后的爆炸演出状态。
- `FRUIT_NINJA_STATE_GAME_OVER`
  - 结束页状态。

这是理解全局流程最重要的主轴。几乎每个输入处理和每帧更新逻辑都会先判断当前状态。

## 4.2 果实静态定义：`fruit_ninja_fruit_def_t`

这个结构体表示“某一种水果长什么样”。它是模板，不是运行中的实例。

字段重点：

- `type_name`：水果类型名，例如 `apple`、`banana`、`boom`。
- `whole_rel_path`：完整果实图片。
- `split_left_rel_path` / `split_right_rel_path`：切开后的左右半张图。
- `width` / `height`：资源显示尺寸。
- `radius`：碰撞半径。
- `base_rotation_deg`：初始角度。
- `is_bomb`：是否炸弹。

在 `fruit_ninja_scene.c` 里，`g_fruit_defs[]` 直接把 5 种水果和 1 种炸弹都定义好了。换句话说，**这套游戏的资源配置不是外部 JSON 驱动，而是直接硬编码在 C 数组里。**

## 4.3 运行中果实实例：`fruit_ninja_fruit_t`

这个结构体表示“屏幕上当前飞着的一个具体果实”。关键字段分 5 类：

### 1）生命周期类

- `active`：这个槽位是否在使用。
- `sliced`：是否已经被切开。
- `counted_as_miss`：是否已经算过漏切。
- `has_been_visible`：是否真的进入过屏幕可见区。
- `falling`：是否已经从上抛阶段切换到下落阶段。

### 2）配置引用类

- `def`：指向它对应的 `fruit_ninja_fruit_def_t`。
- `radius`：当前实例使用的碰撞半径。

### 3）位置运动类

- `x` / `y`：当前中心点坐标。
- `vx` / `vy`：当前帧位移差，用于记录运动趋势。
- `angle` / `angular_velocity`：当前旋转角与角速度。

### 4）阶段动画类

- `shot_out_start_x/y`
- `shot_out_end_x/y`
- `fall_target_x/y`
- `phase_elapsed_ms`

这组字段决定了它不是按传统物理引擎实时积分，而是按“两个阶段的补间轨迹”运动：

1. 上抛阶段：从起点飞到最高点附近。
2. 下落阶段：从最高点飞到屏幕底部附近。

### 5）LVGL 对象指针类

- `whole_image`：完整果实图。
- `shadow_image`：阴影图。
- `slice_flash`：切中闪光图，目前实际没有深度使用。

## 4.4 碎片实例：`fruit_ninja_fragment_t`

切开普通水果后，不再继续使用原果实对象，而是额外生成两个“碎片对象”。

字段重点：

- `active`
- `x` / `y`
- `start_x` / `start_y`
- `target_x` / `target_y`
- `angle` / `start_angle` / `target_angle`
- `life_ms`
- `phase_elapsed_ms`
- `image`

你可以把它理解成“切开后左右两半的独立动画体”。它的更新逻辑与原始果实分离。

## 4.5 刀痕：`fruit_ninja_trail_t`

这部分对应玩家手势轨迹：

- `pressing`：当前是否还按着鼠标。
- `has_last_point`：是否存在上一个轨迹点。
- `count`：当前轨迹点数。
- `fade_ms`：释放后淡出计时。
- `last_x` / `last_y`：最后一个点。
- `points[]`：最多 24 个轨迹点。
- `line`：实际绘制刀痕的 `lv_line` 对象。

它和“判定切中水果”的关系是：

- 轨迹用于显示整条刀痕。
- 但碰撞并不是拿整条轨迹逐点判断，而是只拿“本次新增的那一段线段”做碰撞计算。

## 4.6 全局游戏对象：`fruit_ninja_game_t`

整个游戏的运行时几乎都收拢在一个全局结构 `g_game` 里。它包含：

- 当前状态、计时器、得分、失败次数。
- 各种 LVGL 图层。
- 首页/HUD/GameOver 的对象指针。
- 当前刀痕数据。
- 首页 3 个演示果实。
- 游戏中的果实池。
- 切开的碎片池。

这说明当前实现采用的是：

- 单场景
- 单全局游戏对象
- 定长对象池
- 16ms 定时更新

而不是 ECS 或动态容器模式。

---

## 5. 资源系统是怎么工作的

## 5.1 根目录推导

`fruit_ninja_assets.c` 不是写死绝对路径，而是尝试自动推导项目根目录。

核心思路：

1. 从当前工作目录 `getcwd()` 开始。
2. 逐级向上回退目录。
3. 检查某一级目录下是否存在 `main/assets/fruit_ninja`。
4. 找到后，把该目录保存为 `g_project_root`。

所以它的本质是一个“向上爬目录直到找到资源根”的策略。

## 5.2 为什么图片路径前面有 `S:`

图片路径由 `fruit_ninja_assets_build_image_path()` 构造，格式类似：

```text
S:/absolute/or/project/root/main/assets/fruit_ninja/images/xxx.png
```

前缀 `S:` 是 LVGL 文件系统路径前缀的用法，告诉 LVGL 这是一个文件系统资源，而不是内存数组资源。

音频路径则不加 `S:`，因为 SDL_mixer 直接吃本地文件路径。

## 5.3 资源完整性校验

`fruit_ninja_assets_validate_core_files()` 会检查：

- 背景图、logo、首页图、阴影图、爆炸图。
- 5 种水果 + 炸弹的完整图和切片图。
- 菜单音乐、爆炸音效、结束音效、切割音效、抛出音效。

如果缺资源，游戏不会直接崩，但会记录 `resources_ready = false`，并在启动时打印警告日志。

这意味着：

- 代码容忍资源不完整。
- 但视觉/音频会出现缺失。
- 这是一个“尽量可运行、便于调试”的容错策略。

---

## 6. 音频系统是怎么接进来的

`fruit_ninja_audio.c` 使用的是 `SDL2_mixer`。

## 6.1 初始化流程

`fruit_ninja_audio_init()` 会：

1. `Mix_Init(MIX_INIT_OGG | MIX_INIT_MP3)` 初始化解码器。
2. `Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048)` 打开音频设备。
3. `Mix_AllocateChannels(8)` 分配 8 个音效通道。
4. 加载以下资源：
   - 菜单背景音乐
   - 开局音效
   - 抛出水果音效
   - 切中音效
   - 炸弹音效
   - 结束音效

加载时优先 OGG，失败时回退 MP3。

## 6.2 播放接口很薄

这里没有复杂的音频管理器，只有几类简单包装：

- `fruit_ninja_audio_play_menu_music()`
- `fruit_ninja_audio_play_start()`
- `fruit_ninja_audio_play_throw()`
- `fruit_ninja_audio_play_slice()`
- `fruit_ninja_audio_play_boom()`
- `fruit_ninja_audio_play_game_over()`
- `fruit_ninja_audio_stop_music()`

主场景文件只关心“什么时候播什么”，并不关心底层音频细节。

---

## 7. 静态场景是怎么搭出来的

`create_static_scene()` 是整个 UI 场景搭建函数。它做的不是“进入游戏”，而是“一次性把所有固定对象建好”。

## 7.1 图层结构

它先创建一个根屏幕，然后创建 6 层：

- `home_layer`：首页元素。
- `fruit_layer`：运行中的水果与碎片。
- `effect_layer`：刀光、切割闪光等特效。
- `hud_layer`：分数与失败图标。
- `overlay_layer`：Game Over、爆炸白闪、烟雾等覆盖层。
- `input_layer`：最顶层输入层。

这是本实现最值得学习的 UI 组织方式之一：

- 水果不直接挂在 screen 根上。
- 特效不直接跟水果混在一起。
- 输入层单独一层并移到前景，保证不会被其他对象挡住。

## 7.2 首页静态元素

首页预先创建这些对象：

- 背景图
- home mask
- logo
- 首页描述图
- ninja 图
- dojo 图
- new game 图
- new sign 图
- 提示文字 `Slice the middle fruit to start`

这里有一个关键设计：**首页元素不是进入首页时现建，而是启动时就建好，只通过显隐和位置调整来做状态切换与动画。**

## 7.3 HUD 与结算对象

同一个函数里还预建了：

- 分数图标 `score_image`
- 分数数字 `score_label`
- 3 个失败图标 `miss_icons`
- `game_over_image`
- `restart_label`

这让状态切换成本变低，因为只需切换显示，不需要频繁创建和销毁静态 UI。

## 7.4 输入绑定

`input_layer` 被打上 `LV_OBJ_FLAG_CLICKABLE`，并绑定四类事件：

- `LV_EVENT_PRESSED`
- `LV_EVENT_PRESSING`
- `LV_EVENT_RELEASED`
- `LV_EVENT_PRESS_LOST`

所有输入统一进 `input_event_cb()`，由它再根据状态分发到首页或运行中逻辑。

## 7.5 刀痕对象初始化

最后调用 `fruit_ninja_input_init(game)`，内部会在 `effect_layer` 上创建一条 `lv_line`，设置：

- 线宽 10
- 颜色浅灰蓝
- 透明度约 90%

也就是说，刀痕是一个复用对象，不是每拖一下创建一个新对象。

---

## 8. 游戏首页流程：从启动到可以开局

进入首页靠 `enter_home()`，它不是简单“show 一些对象”，而是一次完整重置。

## 8.1 `enter_home()` 做了什么

它会：

1. 状态切到 `HOME`。
2. 重置状态计时与刷水果计时。
3. 清理运行中的水果和碎片。
4. 清空旧的首页果实并重新创建 3 个首页水果。
5. 重置刀痕。
6. 清空闪光、分数脉冲、得分、miss 计数。
7. 把难度相关参数恢复为初始值：
   - `volley_num = 2`
   - `volley_multiple = 5`
8. 更新分数显示与 miss 图标。
9. 显示首页层，隐藏 HUD 和 game over。
10. 显示提示文案。
11. 清理爆炸覆盖层。
12. 停止旧音乐，并重新播放菜单音乐。

可以看到：**首页既是起点，也是“重新开局前的清场状态”。**

## 8.2 首页不是静止的，它有分阶段入场动画

`update_home_animation()` 每 16ms 会根据 `state_elapsed_ms` 刷新首页动画。

它把首页动画拆成 4 个阶段：

- `stage0`：立即显示背景遮罩和 logo。
- `stage1`：500ms 后显示 ninja。
- `stage2`：1500ms 后显示说明图。
- `stage3`：2000ms 后显示 dojo、new game、new sign，以及 3 个首页水果。

其中 logo、ninja、dojo、new sign 都会带轻微正弦浮动，让首页看起来不是死板静态图。

## 8.3 首页的 3 个水果并不只是装饰

`build_home_menu_fruits()` 创建 3 个首页水果，位置是固定的：

- 左边一个普通水果
- 中间一个普通水果
- 右边一个炸弹

这 3 个水果直接承担“菜单交互入口”。

`handle_home_menu_hits()` 的规则是：

- 切中中间水果：开始游戏。
- 切中左边水果：只做切中反馈，短暂消失后恢复。
- 切中右边炸弹：播放炸弹音效，短暂消失后恢复，但不会开局。

也就是说，这个首页并没有传统按钮，而是把“切中间水果开始游戏”做成了玩法一致的入口。

## 8.4 为什么首页开局要延迟 240ms

中间水果被切中时，并不是立刻进入 `RUNNING`，而是：

1. 先显示切中反馈和音效。
2. 创建一个 `FRUIT_NINJA_HOME_SLICE_FEEDBACK_MS = 240ms` 的定时器。
3. 240ms 到时再调用 `enter_running()`。

这样做的原因很直观：让玩家先看到“切开菜单水果”的反馈，视觉上更自然。

---

## 9. 运行态是怎么开始的

进入游戏靠 `enter_running()`。

它会做这些初始化：

1. 状态切到 `RUNNING`。
2. `state_elapsed_ms = 0`。
3. `spawn_elapsed_ms = 500`。
4. `spawn_interval_ms = 1000`。
5. 清空水果、碎片、首页果实。
6. 重置刀痕。
7. 得分归零，miss 归零。
8. `spawn_index = 0`。
9. 更新分数和 miss UI。
10. 隐藏首页层。
11. 显示 HUD。
12. 隐藏 game over 与提示文案。
13. 停止菜单音乐，播放开局音效。

这里有两个值得注意的实现点。

### 9.1 刷新计时器并没有重建

进入运行态时，并没有新建一个专门的游戏循环定时器。整个游戏从 `fruit_ninja_start()` 开始就只有一个 `update_timer`，状态切换靠 `game->state` 决定这一帧该做什么。

这是典型的“单时钟驱动多状态逻辑”的实现方式。

### 9.2 为什么 `spawn_elapsed_ms` 初值是 500

这样进入游戏后，第一次刷水果不会等满 1000ms，而是半秒后就出现第一波，减少玩家等待感。

---

## 10. 运行中的主循环：每 16ms 到底做什么

整个运行靠 `update_timer_cb()` 驱动，每 16ms 执行一次。这个函数就是全游戏心跳。

每一帧的顺序大致如下：

1. 增加全局时间：`tick_count`、`state_elapsed_ms`。
2. 如果闪光还在，增加 `flash_age_ms`。
3. 更新切中闪光的缩放与透明度。
4. 更新分数图标脉冲效果。
5. 更新刀痕淡出效果。
6. 如果是首页状态，刷新首页入场动画。
7. 如果是运行状态，推进刷水果计时，必要时生成新水果。
8. 更新所有运行中水果位置与状态。
9. 更新所有碎片位置与状态。
10. 如果是爆炸状态，推进白闪/烟雾渐隐，超时后转入 Game Over。

这说明它的结构并不是“按对象种类完全分离的 ECS”，而是：

- 一条顶层心跳
- 若干状态分支
- 若干对象池更新

非常适合中小型单场景游戏。

---

## 11. 水果生成机制：它不是随时生，而是按目标数量补齐

## 11.1 水果生成触发条件

在 `RUNNING` 状态下，每一帧都会累计 `spawn_elapsed_ms`。当它达到 `spawn_interval_ms`（初始 1000ms）时：

1. `spawn_elapsed_ms` 清零。
2. 计算当前目标水果数 `target_count`。
3. 统计当前仍活跃、且未切开的水果数 `active_running_fruits()`。
4. 用“目标数 - 当前活跃数”得到本次需要补多少个。
5. 调用 `spawn_one_fruit()` 补齐。

这非常重要：**它不是每秒固定追加 N 个，而是每秒把场上活跃水果数量补到目标值。**

## 11.2 目标水果数来自哪里

`target_fruit_count()` 返回：

- `volley_num > 0` 时，返回 `volley_num`
- 否则返回 2

初始时 `volley_num = 2`，所以开局场上目标是 2 个水果。

## 11.3 难度是如何增长的

在切开普通水果的 `slice_fruit()` 里，得分 +1 之后会检查：

- 如果 `score > volley_num * volley_multiple`
  - `volley_num += 1`
  - `volley_multiple += 50`

所以难度增长逻辑不是线性的，也不是按时间增长，而是按分数阈值增长。

它的实际效果是：

- 初期目标水果数增长很慢。
- 每次增长后，下一次增长门槛会大幅拉高。
- 这比“每得 10 分就多一个水果”更保守。

## 11.4 `spawn_one_fruit()` 具体做了什么

生成一个水果时，会：

1. 用 `choose_fruit_def()` 随机决定类型。
   - `rand() % 8 == 4` 时生成炸弹。
   - 否则在前 5 种普通水果中随机。
2. 从定长数组中找一个未激活槽位。
3. 设置水果类型与半径。
4. 随机生成起点 `x`。
5. 初始 `y = 600`，也就是屏幕底部以下。
6. 计算下列轨迹点：
   - 起点 `shot_out_start_x/y`
   - 上抛终点 `shot_out_end_x/y`
   - 下落终点 `fall_target_x/y`
7. 创建阴影图和完整水果图。
8. 设置图片 pivot，刷新视觉位置。
9. 播放“抛出”音效。

---

## 12. 这套“物理”其实是分段补间，不是真实动力学积分

很多人第一眼看到 `vx`、`vy`、`gravity` 会以为它是传统抛体运动，但实际不是。

## 12.1 上抛阶段

在 `update_fruits()` 里，如果 `falling == false`：

- 用 `phase_elapsed_ms / FRUIT_NINJA_DROP_TIME_MS` 得到进度 `progress`。
- `x` 线性插值到 `shot_out_end_x`。
- `y` 用 `ease_out_quad(progress)` 从起点补间到上抛终点。

这意味着：

- 上抛是“预先计算好终点，再按 easing 曲线移动过去”。
- 不是每帧 `x += vx`、`y += vy`、`vy += g` 的经典物理积分。

## 12.2 下落阶段

到达上抛结束后：

- `falling = true`
- `phase_elapsed_ms = 0`
- 当前点变成下落起点

接着下落阶段：

- `x` 线性插值到 `fall_target_x`
- `y` 用 `ease_in_quad(progress)` 插值到 `fall_target_y`

所以它本质上是：

- 第一段 ease-out，模拟向上冲出后逐渐减速。
- 第二段 ease-in，模拟开始下坠并逐渐加速。

从观感上像抛物线，但实现上是更可控的两段补间。

## 12.3 `vx` / `vy` 在这里更多是派生量

每帧更新完位置后，代码才做：

- `fruit->vx = fruit->x - prev_x`
- `fruit->vy = fruit->y - prev_y`

也就是说它们不是驱动位置的主变量，而是“由位置差反推出来的当前位移趋势”。

这对理解后续切开方向偏移有帮助。

---

## 13. 刀痕与输入：玩家的拖拽是如何变成一条切割线段的

## 13.1 事件入口：`input_event_cb()`

所有输入都经过 `input_event_cb()`，然后按状态解释：

### `LV_EVENT_PRESSED`

- 首页：开始一条新刀痕。
- Game Over：直接回首页。
- 运行中：开始一条新刀痕。

### `LV_EVENT_PRESSING`

- 首页：追加轨迹点，并检测是否切到首页菜单水果。
- 运行中：追加轨迹点，并检测是否切到活跃水果。

### `LV_EVENT_RELEASED` / `LV_EVENT_PRESS_LOST`

- 首页或运行中：结束本次按压，进入刀痕淡出阶段。

## 13.2 为什么是“线段”而不是“点”做判定

`fruit_ninja_input_push_point()` 每次收到新点时，不只记录轨迹，还会返回一个 `fruit_ninja_segment_t`：

- 上一个点 `(x1, y1)`
- 当前点 `(x2, y2)`
- `valid`

如果新点距离上一个点太近，小于 `FRUIT_NINJA_SEGMENT_MIN_DIST = 12`，就不生成有效线段。

这样做有 3 个好处：

1. 降低无意义高频抖动。
2. 避免一次轻微移动触发过多碰撞检测。
3. 保持刀痕视觉点数可控，最多 24 个。

## 13.3 刀痕是如何淡出的

当玩家还按着时：

- 线条透明度固定在 `LV_OPA_90`。

当玩家松开后：

- `fruit_ninja_input_tick()` 每帧累积 `fade_ms`。
- 180ms 内逐步降低透明度。
- 超过 180ms 直接调用 `fruit_ninja_input_reset()` 隐藏整条线。

所以刀痕实现非常轻量：

- 一条 `lv_line`
- 点数组复用
- 松手后做一次透明度衰减

---

## 14. 命中判定：为什么用“线段打圆”

真正的碰撞函数在 `fruit_ninja_collision.c`：`fruit_ninja_segment_hits_circle()`。

它的算法是经典的：

1. 取线段方向向量 `(dx, dy)`。
2. 计算圆心在这条线段上的投影参数 `t`。
3. 把 `t` 限制在 `[0, 1]`，确保只看线段，不看无限直线。
4. 求得离圆心最近的线段点 `(px, py)`。
5. 判断该点到圆心的距离平方是否小于半径平方。

这是一种非常适合切水果的近似：

- 刀痕看作一段快速划过的线。
- 水果看作圆形碰撞区。
- 不用做复杂像素级碰撞。

## 14.1 首页命中逻辑

`handle_home_menu_hits()` 遍历 3 个首页水果，命中后按索引做不同处理：

- `i == 1`：中间水果，触发开局。
- `i == 2`：右边炸弹，播放 boom 反馈后恢复。
- 其他：普通切中反馈后恢复。

## 14.2 运行中命中逻辑

`handle_segment_hits()` 遍历活跃水果池：

- 跳过未激活或已切开的水果。
- 命中即调用 `slice_fruit()`。
- 如果 `slice_fruit()` 导致状态切成 `EXPLODING`，立刻结束本次遍历。

这种写法保证：

- 炸弹一旦命中，爆炸流程立即抢占当前逻辑。
- 不会在同一段线里继续处理后面的水果。

---

## 15. 切中普通水果后，到底发生了什么

核心逻辑在 `slice_fruit()`。

## 15.1 先看炸弹分支

如果命中的是炸弹：

1. 隐藏炸弹本体和阴影。
2. 调用 `enter_exploding(game, fruit->x, fruit->y)`。
3. 直接返回。

炸弹不会走“切成两半”的分支。

## 15.2 普通水果分支

普通水果被切中后：

1. `fruit->sliced = true`
2. 隐藏完整果实图和阴影图。
3. 随机生成左右半片的目标位置与目标角度。
4. 调用 `spawn_fragment()` 两次，创建左右碎片对象。
5. 给碎片设置 1200ms 的下落寿命与目标状态。
6. 在切中位置附近生成刀光闪白 `spawn_flash()`。
7. `score += 1`
8. 触发分数图标脉冲动画。
9. 视分数调整难度阈值。
10. 刷新分数文本。
11. 播放切中音效。

## 15.3 切开后的左右两半怎么飞

注意这里的碎片并不是继承原水果当前速度继续飞，而是重新设定：

- 起点就是切中时完整水果左上角附近。
- 目标 x 左右分开。
- 目标 y 统一落到屏幕底部附近。
- 目标角度左右分别朝不同方向旋转。

这意味着当前实现更重视“观感稳定”，而不是完全物理一致。

---

## 16. 切到炸弹后，为什么不会立刻黑屏结束

`enter_exploding()` 专门负责爆炸演出。

它做了这些事：

1. 状态切到 `EXPLODING`。
2. `state_elapsed_ms = 0`。
3. 在炸弹位置生成一条高亮 flash。
4. 创建或复用烟雾图 `smoke_overlay`。
5. 创建或复用全屏白闪层 `white_flash_overlay`。
6. 把场上所有活跃水果的速度和角速度都乘以 `0.2`，造成“爆炸冲击后世界变慢”的观感。
7. 播放炸弹音效。

之后不立刻 Game Over，而是交给 `update_timer_cb()` 在后续 4 秒里慢慢推进：

- 白闪逐渐变透明。
- 烟雾在前 1200ms 内逐渐淡出。
- 到 `FRUIT_NINJA_EXPLODING_MS = 4000ms` 后，才清理覆盖层并进入 `GAME_OVER`。

这让炸弹结算更有戏剧性，而不是生硬中断。

---

## 17. 漏切判定是怎么做的

漏切逻辑是很多人读代码时容易忽略的地方，实际上它做得挺严谨。

## 17.1 什么时候算“真正进入过屏幕”

果实从 `y = 600` 出生，最开始在屏幕外。如果直接拿它是否落出屏幕判断 miss，会误判。

所以代码专门有：

- `fruit_ninja_fruit_is_visible_on_screen()`
- `has_been_visible`

一旦某颗水果真正进入可见区域，才把 `has_been_visible = true`。

## 17.2 什么时候算 miss

`fruit_ninja_fruit_should_count_miss()` 要同时满足：

- 果实存在。
- 不是已切开的。
- 不是炸弹。
- 还没有算过 miss。
- 已经至少进过可见区。
- 并且它的顶部已经离开屏幕底部。

这套条件的含义是：

- 炸弹掉下去不算漏切。
- 屏幕外出生的水果不误判。
- 同一颗水果只记一次 miss。

## 17.3 记 miss 后会发生什么

在 `update_fruits()` 里，如果某颗水果触发 miss：

1. `counted_as_miss = true`
2. `active = false`
3. 删除完整果实和阴影对象。
4. `game->misses += 1`
5. 更新顶部 miss 图标。
6. 如果 miss 达到 3 次，立刻 `enter_game_over()`。

所以“漏掉 3 个普通水果结束”就是在这里实现的。

---

## 18. 碎片更新与回收是怎么做的

`update_fragments()` 负责切开后左右半片的运动。

它与水果更新非常像，也是按进度补间：

1. `phase_elapsed_ms += 16`
2. `progress = elapsed / FRUIT_NINJA_DROP_TIME_MS`
3. `x` 线性插值到目标 x
4. `y` 用 `ease_in_quad` 掉到目标 y
5. `angle` 插值到目标角度
6. `life_ms` 递减
7. 更新 LVGL 图片位置和旋转
8. 超时或飞出屏幕就销毁并清空槽位

这意味着碎片系统是：

- 固定容量对象池
- 生命周期驱动回收
- 无额外链表或堆分配

对小型游戏来说，这样做稳定且简单。

---

## 19. Game Over 流程

进入结束态有两条路：

### 路线 1：漏掉 3 个普通水果

`update_fruits()` 里 miss 达到 3 次后，直接 `enter_game_over()`。

### 路线 2：切到炸弹

先 `enter_exploding()`，4 秒演出结束后，再 `enter_game_over()`。

## 19.1 `enter_game_over()` 做了什么

它会：

1. 状态切到 `GAME_OVER`。
2. 重置状态计时。
3. 隐藏提示文案。
4. 显示 `game_over_image`。
5. 显示 `restart_label`。
6. 停止背景音乐。
7. 播放 game over 音效。
8. 清理首页水果。

注意：

- 它没有销毁整个场景。
- 没有重建 screen。
- 只是把状态切到结算页。

## 19.2 结束后怎么重新开始

在 `input_event_cb()` 中，如果当前状态是 `GAME_OVER`，用户再次按下屏幕会直接调用 `enter_home(game)`。

也就是说，结算页点击任意位置即可返回首页。

---

## 20. 视觉特效是如何实现的

## 20.1 切中闪光 `spawn_flash()`

切中水果或切首页菜单水果时，会创建一张 flash 图片：

- 放在 `effect_layer`
- 围绕切中点摆放
- 通过 `clear_flash_if_needed()` 每帧调整缩放和透明度
- 200ms 后自动删除

这是一个“短寿命对象 + 定时器统一回收”的特效。

## 20.2 分数跳动 `update_score_pulse()`

每次得分后：

- `score_pulse_ms = 90`

接着每帧按剩余时间把 `score_image` 在 `256` 到 `307` 之间放大/缩回，形成轻微脉冲。

## 20.3 爆炸白闪与烟雾

切中炸弹后会复用两个覆盖对象：

- `white_flash_overlay`
- `smoke_overlay`

它们不重复创建过多次，只在需要时创建、之后复用，减少对象抖动。

---

## 21. 这套实现里几个非常值得注意的设计选择

## 21.1 不是“真物理”，而是“视觉补间 + 状态机”

代码里虽然有 `vx/vy/gravity` 字段，但主运动完全靠两段 easing 插值。它追求的是：

- 视觉节奏可控
- 调参简单
- 不需要复杂物理系统

这对 UI 框架里的小游戏非常适合。

## 21.2 所有动态对象都走定长对象池

- 水果最多 `16`
- 碎片最多 `32`
- 刀痕点最多 `24`

优点：

- 内存稳定
- 无频繁堆分配
- 回收策略简单

代价：

- 上限写死
- 极端情况下对象不足会直接不生成

## 21.3 一个定时器驱动全局

全局只维护一个 `update_timer`，每 16ms 刷一次状态。这样：

- 不容易出现多个逻辑定时器不同步
- 所有动画节奏统一
- 调试时容易定位主循环

## 21.4 首页交互和核心玩法一致

开始游戏不是点按钮，而是“切中间水果”。这让菜单本身就成为玩法教学的一部分，设计上很聪明。

---

## 22. 阅读源码的推荐顺序

如果你想真正吃透，而不是看完文档就忘，建议按下面顺序读：

### 第 1 轮：先抓全局

1. 读 `main/src/main.c`
2. 读 `main/src/v9-fruit_ninja/fruit_ninja.h`
3. 读 `main/src/v9-fruit_ninja/fruit_ninja_scene.c` 里的 `fruit_ninja_start()`
4. 读 `create_static_scene()`
5. 读 `enter_home()`、`enter_running()`、`enter_exploding()`、`enter_game_over()`

目标：先搞清楚状态机与场景搭建。

### 第 2 轮：看主循环

1. 读 `update_timer_cb()`
2. 读 `update_home_animation()`
3. 读 `update_fruits()`
4. 读 `update_fragments()`

目标：理解每帧在推进什么。

### 第 3 轮：看输入与判定

1. 读 `input_event_cb()`
2. 读 `fruit_ninja_input_begin()` / `fruit_ninja_input_push_point()` / `fruit_ninja_input_tick()`
3. 读 `fruit_ninja_segment_hits_circle()`
4. 读 `handle_home_menu_hits()` / `handle_segment_hits()` / `slice_fruit()`

目标：理解玩家拖拽是如何转成碰撞与得分的。

### 第 4 轮：看资源和音频

1. 读 `fruit_ninja_assets.c`
2. 读 `fruit_ninja_audio.c`

目标：理解为什么图片能直接读文件、声音如何播放。

---

## 23. 按真实游戏流程复盘一次

这里把整个流程再串成一条时间线，帮助你把零散函数连起来。

### 阶段 A：程序启动

1. `main()` 初始化 LVGL 和 SDL 输入/显示。
2. 调用 `fruit_ninja_start()`。
3. 初始化资源、音频、随机数、静态场景。
4. 创建全局 16ms 定时器。
5. 进入首页 `enter_home()`。

### 阶段 B：首页展示

1. 首页元素按阶段渐进显示。
2. 菜单音乐循环播放。
3. 3 个首页水果开始漂浮/旋转。
4. 玩家拖拽时也会显示刀痕。

### 阶段 C：首页交互

1. 玩家拖拽产生新的切割线段。
2. 用线段撞首页水果的圆形碰撞体。
3. 如果切中中间水果，240ms 后进入 `RUNNING`。
4. 如果切中左右水果，只做反馈后恢复。

### 阶段 D：进入游戏

1. 清空首页对象状态。
2. 显示 HUD。
3. 播放开始音效。
4. 半秒后开始刷第一波水果。

### 阶段 E：运行中循环

1. 每秒检查是否需要补足目标水果数。
2. 新水果从底部以下飞出。
3. 上抛完成后进入下落阶段。
4. 玩家拖拽时实时产生切割线段。
5. 线段命中普通水果则切开、得分、生成碎片。
6. 命中炸弹则进入爆炸演出。
7. 普通水果掉出屏幕则计为 miss。

### 阶段 F：结束路径

- 漏掉 3 个普通水果：直接 Game Over。
- 切中炸弹：先爆炸演出 4 秒，再 Game Over。

### 阶段 G：重新开始

1. 结算页点击屏幕。
2. 重新回到首页。
3. 所有得分、miss、动态对象重新清零。

---

## 24. 几个实现层面的细节观察

这些不是主流程，但对继续维护很有价值。

## 24.1 `gravity` 字段当前基本没有主导运动

水果和碎片结构里都有 `gravity`，但当前主动画主要靠插值函数推进，`gravity` 更像预留字段或早期思路残留，而不是核心动力学参数。

## 24.2 `slice_flash` 指针也没有形成完整子系统

`fruit_ninja_fruit_t` 里保留了 `slice_flash`，但当前切中视觉主要依赖 `flash_overlay` 全局短寿命对象，不是每个果实自己持有专属闪光对象。

## 24.3 `update_trail_points()` 里的坐标赋值是空操作

`fruit_ninja_input.c` 里有：

```c
trail->points[i].x = trail->points[i].x;
trail->points[i].y = trail->points[i].y;
```

这在逻辑上是 no-op，对功能没有影响，更多像是保留写法或调试遗留。

## 24.4 `spawn_index` 当前没有真正参与刷怪逻辑

`fruit_ninja_game_t` 里有 `spawn_index`，进入运行态时也会清零，但当前刷水果逻辑并没有真正使用它来控制出场序列。

说明这套实现还保留了一些可继续演化的空间。

---

## 25. 如果你准备继续扩展，这些点最适合下手

基于当前代码结构，后续最自然的扩展方向有：

1. 把 `fruit_ninja_scene.c` 按职责继续拆分
   - 例如拆成 `scene_state`、`scene_spawn`、`scene_effects`、`scene_view`。
2. 把当前两段 easing 轨迹升级成更真实的速度/重力积分模型。
3. 用更丰富的首页交互或按钮系统替换固定 3 果实菜单。
4. 给炸弹、切片、刀痕增加更强的视觉层次。
5. 把难度曲线从简单阈值增长改成更可配置的曲线表。
6. 对资源缺失给出屏幕级错误提示，而不只是日志告警。

---

## 26. 一句话总结

`main/src/v9-fruit_ninja` 这套实现，本质上是一个“以 `fruit_ninja_scene.c` 为中心、由单个 16ms 定时器驱动、使用对象池与状态机组织的 LVGL 小游戏框架”。

它没有追求重型引擎式抽象，而是用：

- 固定图层
- 固定对象池
- 状态切换
- easing 补间
- 线段打圆碰撞

把 Fruit Ninja 最核心的体验——首页切入、拖拽切割、分数增长、炸弹惩罚、结算重开——完整地实现了出来。

如果你是第一次读这套代码，最应该优先掌握的不是每个 API，而是下面这条主线：

**`fruit_ninja_start()` 初始化一切 -> `enter_home()` 进入首页 -> `input_event_cb()` 接收拖拽 -> `update_timer_cb()` 作为全局心跳推进动画与状态 -> `handle_segment_hits()` / `slice_fruit()` 完成玩法核心 -> `enter_exploding()` / `enter_game_over()` 完成失败闭环。**

抓住这条线，整套代码就不会再散。

---

## 27. 关键函数阅读顺序图与调用链图

这一节的目的不是重复前面的说明，而是把真正值得盯住的函数，用“入口 -> 场景搭建 -> 输入 -> 主循环 -> 状态切换 -> 玩法结算”的视角串成一张阅读图。

### 27.1 最短主线阅读顺序

如果你时间有限，直接按下面顺序读，效率最高：

1. `main/src/main.c` 里的 `main()`
2. `main/src/v9-fruit_ninja/fruit_ninja_scene.c` 里的 `fruit_ninja_start()`
3. `main/src/v9-fruit_ninja/fruit_ninja_scene.c` 里的 `create_static_scene()`
4. `main/src/v9-fruit_ninja/fruit_ninja_scene.c` 里的 `enter_home()`
5. `main/src/v9-fruit_ninja/fruit_ninja_scene.c` 里的 `input_event_cb()`
6. `main/src/v9-fruit_ninja/fruit_ninja_scene.c` 里的 `update_timer_cb()`
7. `main/src/v9-fruit_ninja/fruit_ninja_scene.c` 里的 `spawn_one_fruit()`
8. `main/src/v9-fruit_ninja/fruit_ninja_scene.c` 里的 `update_fruits()`
9. `main/src/v9-fruit_ninja/fruit_ninja_scene.c` 里的 `handle_segment_hits()`
10. `main/src/v9-fruit_ninja/fruit_ninja_scene.c` 里的 `slice_fruit()`
11. `main/src/v9-fruit_ninja/fruit_ninja_scene.c` 里的 `enter_exploding()` / `enter_game_over()`

这 11 个点基本就构成了整套玩法骨架。

### 27.2 启动链路图

```text
main()
  -> lv_init()
  -> hal_init(640, 480)
  -> fruit_ninja_start()
       -> init_ui_asset_paths()
       -> fruit_ninja_assets_init()
       -> fruit_ninja_assets_validate_core_files()
       -> fruit_ninja_audio_init()
       -> create_static_scene()
            -> create_layer()
            -> create_file_image()
            -> build_home_menu_fruits()
            -> fruit_ninja_input_init()
       -> lv_timer_create(update_timer_cb, 16ms, &g_game)
       -> lv_screen_load(g_game.screen)
       -> enter_home(&g_game)
```

这个链路最重要的结论是：

- 所有静态 UI 都在启动时一次性创建。
- 后续玩法不是切换 screen 重建，而是在同一 screen 内切状态。
- `update_timer_cb()` 是游戏真正开始“活起来”的心跳源。

### 27.3 输入链路图

```text
LV_EVENT_PRESSED / PRESSING / RELEASED / PRESS_LOST
  -> input_event_cb()
       -> fruit_ninja_input_begin()
       -> fruit_ninja_input_push_point()
            -> push_internal()
                 -> update_trail_points()
       -> handle_home_menu_hits()    [HOME]
       -> handle_segment_hits()      [RUNNING]
       -> fruit_ninja_input_end()
```

这一段最关键的设计是：

- 输入系统只负责生成轨迹和最新线段。
- 切中谁，不在 input 模块里决定，而在 scene 模块里决定。
- 所以 `fruit_ninja_input.c` 是一个纯输入/视觉辅助模块，不懂游戏规则。

### 27.4 运行态心跳链路图

```text
update_timer_cb()
  -> clear_flash_if_needed()
  -> update_score_pulse()
  -> fruit_ninja_input_tick()
  -> update_home_animation()         [HOME]
  -> spawn_one_fruit()               [RUNNING and need refill]
  -> update_fruits()
  -> update_fragments()
  -> enter_game_over()               [EXPLODING timeout or miss >= 3]
```

这条链说明：

- 所有按帧更新都在一个函数里编排。
- “是否刷水果”“是否做首页动画”“是否做爆炸演出”都靠状态决定。
- `update_fruits()` 是运行态最重的逻辑函数，既负责运动，又负责 miss 判定，又负责对象回收。

### 27.5 切水果核心链路图

```text
玩家拖拽
  -> input_event_cb()
  -> fruit_ninja_input_push_point()
  -> 得到最新线段 segment
  -> handle_segment_hits(segment)
       -> fruit_ninja_segment_hits_circle()
       -> slice_fruit()
            -> 普通水果: spawn_fragment() x2 + score++ + flash + 音效
            -> 炸弹: enter_exploding()
```

这里的阅读重点有两个：

1. 线段命中判定只发生在 `handle_segment_hits()`，不是在水果更新里被动检测。
2. `slice_fruit()` 是玩法结果分叉点：普通水果和炸弹从这里开始完全走不同结局。

### 27.6 状态切换总图

```text
fruit_ninja_start()
  -> HOME

HOME
  -> 切中中间首页水果
  -> start_running_timer_cb()
  -> RUNNING

RUNNING
  -> 切中普通水果         -> 继续 RUNNING
  -> 漏掉 3 个普通水果    -> GAME_OVER
  -> 切中炸弹             -> EXPLODING

EXPLODING
  -> 4 秒演出结束         -> GAME_OVER

GAME_OVER
  -> 点击屏幕             -> HOME
```

这张图能帮助你快速定位“一个 bug 属于哪段状态逻辑”。

### 27.7 调试时最值得打断点的位置

如果后面你要查 bug，我最建议优先在这些函数打断点：

- `fruit_ninja_start()`：确认资源、音频、scene 是否真的初始化成功。
- `create_static_scene()`：确认对象有没有建出来、层级是否正确。
- `input_event_cb()`：确认鼠标事件是否正常进入。
- `fruit_ninja_input_push_point()`：确认是否真的产生了有效线段。
- `handle_segment_hits()`：确认命中判定是否发生。
- `slice_fruit()`：确认普通水果 / 炸弹分支是否走对。
- `update_fruits()`：确认飞行、miss、回收逻辑。
- `enter_exploding()`：确认炸弹演出是否启动。
- `enter_game_over()`：确认结束页为什么被触发。

---

## 28. 对照 `third-party/FruitNinja/scripts/all.js` 的实现差异分析

这一节不是泛泛说“参考了网页版”，而是对照 `third-party/FruitNinja/scripts/all.js` 里的关键逻辑，看当前 `v9-fruit_ninja` 到底继承了什么、简化了什么、改写了什么。

先给结论：

**当前 `main/src/v9-fruit_ninja` 不是逐行移植网页版，而是保留了它的核心玩法骨架、资源真值和节奏框架，同时把原本基于 Raphael/timeline/message 的浏览器实现，重写成了基于 LVGL 对象层 + 16ms 定时器 + SDL_mixer 的本地版本。**

### 28.1 一致点 1：水果配置几乎直接沿用了网页版真值

网页版 `all.js` 里有一个 `infos` 配置表，定义了：

- `boom`
- `peach`
- `sandia`
- `apple`
- `banana`
- `basaha`

并且给出了：

- 图片路径
- 宽高
- 半径
- 初始修正角度
- 是否反转
- 果汁颜色

当前 `fruit_ninja_scene.c` 中的 `g_fruit_defs[]`，水果种类、资源路径、尺寸、半径、基础角度，整体上就是按这张表重新写成 C 结构体。

也就是说：

- 当前版本在“资源真值”和“碰撞半径真值”上，明显是尊重原版网页逻辑的。
- 不是随便另起一套尺寸。

### 28.2 一致点 2：首页 3 个菜单水果的位置和语义几乎照搬

网页版首页会创建：

- `peach = fruit.create("peach", 137, 333, true)`
- `sandia = fruit.create("sandia", 330, 322, true)`
- `boom = fruit.create("boom", 552, 367, true, 2500)`

当前 LVGL 版的：

- `g_home_menu_positions[0] = {137, 333}`
- `g_home_menu_positions[1] = {330, 322}`
- `g_home_menu_positions[2] = {552, 367}`

完全可以看出是沿用同一份菜单布局真值。

但有一个实现语义差异：

- 网页版里这 3 个菜单水果分别代表 `dojo / new game / quit`。
- 当前 LVGL 版只保留了“中间水果开始游戏”的有效主路径。
- 左右两侧水果现在更像演示交互反馈，而不是完整菜单入口。

所以这部分属于：**视觉布局高度一致，功能范围有意裁剪。**

### 28.3 一致点 3：开局节奏保持了“500ms 后开刷”

网页版 `game.start()` 的逻辑是：

- 先创建音效
- `timeline.setTimeout(..., 500)`
- 500ms 后把状态切成 `playing`
- 再开启每秒刷水果

当前 LVGL 版 `enter_running()` 虽然没有直接写一个 500ms 延迟切状态，但它把：

- `spawn_elapsed_ms = 500`
- `spawn_interval_ms = 1000`

组合起来，达到的是同样的效果：进入运行态半秒后，第一波水果出现。

这说明当前实现虽然改了机制形式，但保留了原版“开场先给玩家一个呼吸间隔，再开始抛水果”的节奏感。

### 28.4 一致点 4：难度增长公式基本一致

网页版 `applyScore()`：

- 当 `score > volleyNum * volleyMultipleNumber`
- `volleyNum++`
- `volleyMultipleNumber += 50`

当前 LVGL 版 `slice_fruit()` 里：

- `if(game->score > game->volley_num * game->volley_multiple)`
- `game->volley_num += 1U`
- `game->volley_multiple += 50U`

这基本就是等价迁移。

因此当前版本的“场上目标水果数增长节奏”，从设计意图上与网页版是一致的。

### 28.5 一致点 5：炸弹权重规则也保留了

网页版 `getType()` 的逻辑是：

- `if(random(8) == 4) return "boom"`
- 否则从 5 种普通水果里随机

当前 LVGL 版 `choose_fruit_def()`：

- `(rand() % 8) == 4` 时返回炸弹
- 否则从前 5 个普通水果里随机

这说明刷怪概率也基本沿用了原版配置。

### 28.6 差异点 1：网页版是真正的模块消息驱动，LVGL 版改成单文件集中调度

网页版大量依赖这些机制：

- `message.postMessage(...)`
- `message.addEventListener(...)`
- `timeline.createTask(...)`
- `sence.switchSence(...)`

也就是说它的风格更偏：

- 消息总线
- 动画任务系统
- 模块之间解耦广播

而当前 LVGL 版完全换了一种结构：

- 统一由 `fruit_ninja_scene.c` 驱动
- 输入事件直接调用 scene 内函数
- 主循环定时器直接推进所有动画
- 状态切换是直接函数调用，不再走消息总线

这么改的结果是：

优点：

- 本地 C 代码更直白，调试路径更短。
- 少了很多异步消息转发，定位 bug 更简单。
- 更适合 LVGL 这种对象式 UI 框架。

代价：

- `fruit_ninja_scene.c` 变得很大，承担了原网页多个模块的职责。
- 灵活性和抽象层次不如原来的 timeline/message 体系。

### 28.7 差异点 2：网页版水果运动更“任务化”，LVGL 版更“补间化”

网页版 `shotOut()` / `fallOff()` / `apart()` 主要依赖 timeline 任务：

- `onShotOuting`
- `onShotOutEnd`
- `onBrokenDropUpdate`
- `onBrokenDropEnd`

当前 LVGL 版把这些任务思想合并成了两个更新函数：

- `update_fruits()`
- `update_fragments()`

并且用：

- `phase_elapsed_ms`
- `ease_out_quad()`
- `ease_in_quad()`

统一做阶段推进。

所以当前版本的特点是：

- 没有原版那么多分散的动画任务对象。
- 而是把“任务系统”压平为一个全局逐帧状态推进器。

### 28.8 差异点 3：网页版有更多演出细节，LVGL 版目前做了核心保留与适度简化

原版网页在普通水果切开时，除了 flash，还会：

- 生成 `juice` 果汁喷溅
- 使用更完整的 rotate / apart / brokenDrop 动效链

当前 LVGL 版保留了：

- 刀光闪白
- 左右半片掉落
- 旋转
- 爆炸白闪 + 烟雾

但省略或弱化了：

- 果汁喷溅系统
- 更细分的时间线动画组合
- 更复杂的菜单模块切换
- 完整 dojo / quit / developing 流程

这说明当前版本的取舍很明确：

**优先交付“可玩且核心观感对齐”的版本，而不是 1:1 完整复刻网页项目所有模式与支线。**

### 28.9 差异点 4：Game Over 回流路径更直接

网页版结束后是通过：

- 事件消息
- 场景切换模块
- `sence.switchSence("home-menu")`

来回到首页。

当前 LVGL 版简单很多：

- `GAME_OVER` 状态下点击屏幕
- 直接 `enter_home(game)`

这样更朴素，但对本地单场景 demo 来说足够稳定。

### 28.10 差异点 5：HUD 结构做了本地化简化

网页版 `score.js` 除了主分数，还有：

- `BEST 999` 文本
- 更明显的横向滑入动画

网页版 `lose.js` 还有：

- 屏幕底部的 `lose.png` 提示图放大反馈
- 顶部 3 个 X 图标分别切换资源

当前 LVGL 版则采用更简化的 HUD：

- 左上角 `score.png` + 当前分数数字
- 右上角 3 个 miss 图标切换空/满资源
- 分数图标通过轻微 scale pulse 做反馈

即：核心信息保留，但部分高级 UI 动画被压缩成更适合 LVGL 的简单实现。

### 28.11 差异点 6：首页功能范围被收窄成“教学式开始菜单”

原版首页实际上是多入口菜单：

- dojo
- new game
- quit

当前 LVGL 版把首页主要做成：

- 展示品牌风格
- 用“切中中间水果”告诉玩家怎么玩
- 进入单一主玩法

所以当前首页更像：

- 一个有动效的开场页
- 一个玩法教学页
- 而不是完整模式选择菜单

这个取舍对于 PC simulator 演示是合理的，但如果后面你要做更高保真复刻，这里会是第一批要补的地方。

### 28.12 差异点 7：原版的 `fruitCache` / timeline 回收，LVGL 版变成定长对象池

网页版用的是更动态的数组：

- `fruits = []`
- `fruitCache = []`
- 命中/离屏后 splice/remove

当前 LVGL 版则变成固定上限：

- `fruits[16]`
- `fragments[32]`
- `home_menu_fruits[3]`

这种改法很符合本地 C/LVGL 场景：

- 更稳定
- 更省心
- 不依赖频繁分配释放

但也意味着它更像“嵌入式风格小游戏实现”，而不是浏览器里的动态对象系统。

### 28.13 总结：当前版本与原版的关系

如果用一句话概括两者关系：

**当前 `v9-fruit_ninja` 保留了原版 Fruit Ninja 的资源真值、首页布局真值、刷怪概率、开场节奏、难度增长公式和核心玩法路径，但把原本网页里的消息总线 + 时间线动画 + 多菜单模块结构，收拢成了一个更适合 LVGL/本地运行时的集中式 scene 驱动实现。**

换个更工程化的说法：

- 它是“行为与观感尽量对齐”的重写版。
- 不是“架构完全镜像”的逐行移植版。

所以后面你如果继续学习或改造，应该这样看待它：

1. 想理解玩法真值：继续对照 `third-party/FruitNinja/scripts/all.js`
2. 想理解本地 LVGL 落地方式：重点看 `main/src/v9-fruit_ninja/fruit_ninja_scene.c`
3. 想做更高保真复刻：优先补首页多入口、juice 特效、timeline 级细粒度演出
4. 想做稳定维护：优先考虑把当前大一统 `scene.c` 再拆小

---

## 29. 按函数逐段源码解读

这一节专门给“准备直接对着 C 源码读”的场景用。前面章节偏流程和设计，这里偏“这个函数为什么要存在，它在整套系统里承担什么职责，它的前置条件和后置结果是什么”。

### 29.1 `fruit_ninja_start()`：总装入口

这个函数可以理解成整套游戏的 `bootstrap`。

它的输入几乎没有，所有工作都围绕全局 `g_game` 展开，核心责任有 4 类：

1. 建立运行环境
   - 清零 `g_game`
   - 设置屏幕尺寸
   - 初始化随机种子

2. 建立资源能力
   - 初始化图片/音频路径
   - 校验资源完整性
   - 初始化 SDL_mixer

3. 建立 UI 与逻辑承载体
   - 创建静态 screen
   - 创建全局 update timer

4. 让游戏进入第一个可见状态
   - `lv_screen_load(...)`
   - `enter_home(...)`

你后面如果碰到“为什么一启动就是黑屏”“为什么图片/声音没出来”，第一个就该看这里。

### 29.2 `create_static_scene()`：把整个舞台先搭好

这个函数是“搭舞台”，不是“开始表演”。

它最重要的工程思想有两个：

- 一次性把静态对象建完，后面状态切换只做 show/hide 和位置更新。
- 用图层隔离不同职责，避免水果、HUD、特效、输入互相打架。

你可以把它拆成 6 个子步骤去看：

1. 创建根 screen
2. 创建背景图
3. 创建 6 个功能图层
4. 创建首页静态资源
5. 创建 HUD / Game Over 静态资源
6. 绑定 input event + 初始化刀痕线

这个函数本身不复杂，但它决定了：

- 后面对象是挂在哪层上的
- 某个特效会不会挡住输入
- Game Over 为什么不用重建页面

所以它是非常典型的“架构先手函数”。

### 29.3 `enter_home()`：不是简单切页面，而是完整“重置到首页基线”

这个函数非常像一个“reset to menu baseline”的操作。

如果只把它理解成“进入首页”，会低估它的作用。它实际同时完成：

- 清理上一局残留水果
- 清理碎片
- 清理首页临时切中状态
- 重置分数 / miss / 难度
- 重置刀痕
- 重置爆炸特效覆盖层
- 恢复首页静态 UI 可见性
- 恢复首页背景音乐

所以：

- 它既是冷启动首页入口
- 也是从 Game Over 回到初始态的恢复入口

如果未来你要支持“暂停后回首页”“结束后自动回首页”，也应继续复用这条函数，而不是新写一套 reset 逻辑。

### 29.4 `enter_running()`：把首页场景切成真正可玩的游戏态

这个函数的关键词是：

- 清空首页痕迹
- 打开 HUD
- 准备刷怪
- 切断菜单音乐
- 播放开局音效

它本身没有生成水果，但它把所有“生成水果所需上下文”都布好了：

- `state = RUNNING`
- `spawn_elapsed_ms = 500`
- `spawn_interval_ms = 1000`
- `score = 0`
- `misses = 0`

所以它更像“把战斗态上下文切到位”。真正第一波水果，是后面 `update_timer_cb()` 自动补出来的。

### 29.5 `input_event_cb()`：输入解释器

这个函数的重点不是拿坐标，而是“解释输入在当前状态下代表什么”。

同样一次鼠标按下，在不同状态里的语义完全不同：

- `HOME`：这是菜单切水果输入
- `RUNNING`：这是实际玩法输入
- `GAME_OVER`：这是返回首页的确认输入

因此这个函数最重要的思想是：

**输入事件本身很通用，但游戏语义由状态机决定。**

这也是为什么它不把逻辑直接写进 LVGL 事件绑定处，而是集中到一个 callback 里统一解释。

### 29.6 `fruit_ninja_input_push_point()`：把连续拖拽变成“新增的一刀”

很多人读这套代码时，会把注意力放在 `points[]` 数组上，但真正玩法关键不在“轨迹数组”，而在这个函数返回的 `segment`。

它做的事是：

1. 如果还没按下，就自动当作 begin
2. 如果点太密，忽略这次更新
3. 如果点足够远，生成一条有效线段
4. 把新点写入刀痕数组
5. 刷新 `lv_line`

也就是说，它一边服务视觉，一边服务玩法判定。属于“输入可视化 + 命中检测桥梁函数”。

### 29.7 `handle_home_menu_hits()`：把首页水果当成菜单按钮

这个函数很适合拿来学习“玩法式菜单”的写法。

它不是做 `if click rect then start game`，而是做：

- 把 3 个水果都看作真实可切对象
- 命中后执行各自的菜单语义

当前版本里三者语义分别是：

- 左：反馈后恢复
- 中：反馈后开局
- 右：炸弹反馈后恢复

如果你后面想把首页扩回更接近原版的 `dojo/new game/quit` 菜单，最自然的改点就是这里。

### 29.8 `update_timer_cb()`：整个游戏的大脑心跳

这个函数是最像“大脑”的函数，因为它本身不拥有资源对象，却指挥全局每一帧该做什么。

阅读它时最好的方式不是逐句看，而是按下面 5 段分层理解：

1. 通用时基推进
   - `tick_count`
   - `state_elapsed_ms`
   - `flash_age_ms`

2. 与状态无关的公共视觉维护
   - flash 渐隐
   - score pulse
   - trail fade

3. `HOME` 专属逻辑
   - 首页入场动画

4. `RUNNING` 专属逻辑
   - 刷怪节奏
   - 水果更新
   - 碎片更新

5. `EXPLODING` 专属逻辑
   - 白闪淡出
   - 烟雾淡出
   - 到时进入 game over

这就是一个很标准的“单 tick 驱动多状态子系统”的结构。

### 29.9 `spawn_one_fruit()`：把“配置模板”实例化成屏幕里的一个活对象

这个函数不是简单创建图片，它同时完成三件事：

1. 选择一个水果模板
2. 计算它的初始轨迹参数
3. 创建对应的 LVGL 可视对象

尤其要注意中间那组轨迹参数：

- 起点
- 上抛终点
- 下落终点

这说明它在“出生时”就把主要轨迹骨架算好了。后面的 `update_fruits()` 只是按时间推进。

也因此，如果你想调“飞得高一点”“出场更偏左一点”“更像原版 JS”，首选改这个函数。

### 29.10 `update_fruits()`：单颗水果生命周期的总管

这是当前实现里语义最密集的函数之一，因为它同时负责：

- 运动推进
- 旋转推进
- 可见性标记
- miss 判定
- 离屏回收
- Game Over 触发

换句话说，一颗水果从出生到消失，绝大多数关键节点都经过这里。

学习它时建议按每颗水果生命周期看：

1. `active == false`：跳过
2. `RUNNING`：推进位置/角度
3. 刷新图片位置
4. 首次进入屏幕则 `has_been_visible = true`
5. 满足 miss 条件则计 miss
6. 如果是 sliced 或 bomb 且足够离屏，则回收

后续如果你要加“combo”“特殊水果”“冻结水果”，这里几乎一定是必改点。

### 29.11 `handle_segment_hits()`：真正执行玩法碰撞的入口

这个函数非常纯粹：

- 输入是一条新增线段
- 输出是“可能改变某颗水果状态”

它不管刀痕怎么画，也不管碎片怎么飞，只管：

1. 当前是否在 `RUNNING`
2. 某颗水果是否 active 且未切开
3. 这条线是否命中它的圆碰撞体
4. 命中后调用 `slice_fruit()`

它是整个玩法里“判定层”和“结果层”的连接点。

### 29.12 `slice_fruit()`：玩法结果分发中心

这个函数的职责非常明确：

- 命中了水果，接下来怎么办？

它其实是在做“结果分流”：

#### 命中炸弹

- 隐藏本体
- 进入 `EXPLODING`
- 交给爆炸系统接管

#### 命中普通水果

- 标记 `sliced`
- 隐藏完整图
- 生成左右碎片
- 生成 flash
- 分数 +1
- 更新难度
- 播放切中音效

从系统设计角度看，它相当于一个小型 reducer：

- 输入：一次命中
- 输出：游戏状态和对象世界如何变化

### 29.13 `enter_exploding()`：炸弹命中后的临时接管状态

这个函数很值得学习，因为它展示了“失败不是立即结束，而是先切到演出态”。

它做的不是销毁一切，而是：

- 切状态
- 生成白闪
- 生成烟雾
- 让现有水果整体慢下来
- 播放爆炸音效

所以 `EXPLODING` 在架构上不是“效果函数”，而是一个完整的中间状态。

这让失败结算更平滑，也更方便以后继续往里加震屏、粒子、音效层次。

### 29.14 `enter_game_over()`：结束画面的显式收口点

这个函数的重要性在于：它是几条失败路径的统一收口点。

无论你是：

- miss 三次
- 切到炸弹演出结束

最终都收口到这里。

它负责把“玩法层失败”翻译成“UI 层结算态”——也就是：

- 显示 `game_over`
- 显示 restart 提示
- 切断音乐
- 播放结束音效

所以如果以后你要加“结算分数排名”“best score”“分享按钮”，自然也都应该挂在这个状态入口周围。

---

## 30. `third-party/FruitNinja/scripts/all.js` 与当前 C 实现的模块映射表

上一节讲的是差异，这一节更强调“如果我正在对照原版 JS 学习，应该把哪个 JS 概念映射到当前哪个 C 实现”。

### 30.1 总体映射原则

原版网页项目是“按模块拆功能”的：

- game
- scene
- fruit factory
- score object
- lose object
- knife object
- light effect
- message bus
- timeline

当前 C 版本则把这些能力大幅收拢到了几个核心文件：

- `fruit_ninja_scene.c`
- `fruit_ninja_input.c`
- `fruit_ninja_audio.c`
- `fruit_ninja_assets.c`
- `fruit_ninja_collision.c`
- `fruit_ninja_model.h`

所以映射时不要追求“一对一文件对应”，而要接受“一个 JS 模块群 -> 一个 C 主控文件”的关系。

### 30.2 映射表：入口与场景

| 原版 JS 概念 | 当前 C 对应 | 说明 |
|---|---|---|
| `scripts/sence` | `fruit_ninja_scene.c` | JS 里负责场景切换的职责，当前基本都并入 scene 主文件 |
| `showMenu()` / `hideMenu()` | `enter_home()` + `update_home_animation()` + `clear_home_menu_fruits()` | 当前没有独立 scene manager，而是直接状态切换 |
| `showNewGame()` | `enter_running()` | 进入玩法态的主入口 |
| `gameOver.show()` + click back | `enter_game_over()` + `input_event_cb()` | 结束态展示与点击返回首页 |

### 30.3 映射表：玩法主逻辑

| 原版 JS 概念 | 当前 C 对应 | 说明 |
|---|---|---|
| `game.start()` | `enter_running()` + `update_timer_cb()` | 开局准备与后续刷怪心跳合并 |
| `barbette()` | `spawn_one_fruit()` + `active_running_fruits()` + `target_fruit_count()` | JS 的“补齐一波水果”逻辑在 C 里拆成几个辅助函数 |
| `applyScore()` | `slice_fruit()` 内的阈值判断 | 当前没有单独抽函数，直接放在切中逻辑里 |
| `sliceAt()` | `handle_segment_hits()` + `slice_fruit()` | 线段命中检测与命中后果拆开实现 |
| `pauseAllFruit()` | `enter_exploding()` 的减速 + 状态切换 | 当前没有完全等价 pause，而是用状态与减速接管 |

### 30.4 映射表：水果系统

| 原版 JS 概念 | 当前 C 对应 | 说明 |
|---|---|---|
| `infos` | `g_fruit_defs[]` | 水果静态模板表 |
| `fruit.create(...)` | `alloc_fruit()` + `spawn_one_fruit()` | 当前先分配槽位，再写入模板与轨迹 |
| `shotOut()` | `spawn_one_fruit()` + `update_fruits()` 的上抛阶段 | 当前不再用 timeline task |
| `fallOff()` | `update_fruits()` 的下落阶段 | 下落也是统一在帧循环里推进 |
| `broken()` | `slice_fruit()` | 普通水果 / 炸弹分支都从这里出发 |
| `apart()` | `spawn_fragment()` + `update_fragments()` | 左右半片系统 |
| `fruitCache` | `fruits[]` + `home_menu_fruits[]` + `fragments[]` | 从动态缓存改为定长对象池 |

### 30.5 映射表：输入与刀痕

| 原版 JS 概念 | 当前 C 对应 | 说明 |
|---|---|---|
| `knife` 模块 | `fruit_ninja_input.c` + `input_event_cb()` | 当前没有独立复杂 knife 对象，而是输入轨迹 + scene 分发 |
| `message.postMessage("slice", knife)` 一类切割广播 | `fruit_ninja_input_push_point()` 返回 `segment` 后直接本地调用 `handle_segment_hits()` | 从消息广播改为同步函数调用 |
| `knife.endAll()` / `knife.pause()` | `fruit_ninja_input_end()` / `fruit_ninja_input_reset()` / `fruit_ninja_input_tick()` | 当前刀痕系统更轻量，职责也更窄 |

### 30.6 映射表：特效与结算

| 原版 JS 概念 | 当前 C 对应 | 说明 |
|---|---|---|
| `flash.showAt(...)` | `spawn_flash()` + `clear_flash_if_needed()` | 刀光闪白的短寿命特效 |
| `light.start(boom)` | `enter_exploding()` + `update_timer_cb()` 的 exploding 分支 | 当前用白闪层和烟雾层完成爆炸演出 |
| `overWhiteLight.show` -> `game.over` | `EXPLODING` 持续 4 秒后 `enter_game_over()` | 当前不再依赖消息事件串联 |
| `lose.showLoseAt(...)` | `update_fruits()` 中 miss 计数 + `update_miss_icons()` | 当前保留顶部 miss UI，弱化底部 lose 演出 |
| `score.number(...)` | `update_score_label()` + `update_score_pulse()` | 当前分数反馈由文本更新加图标脉冲组成 |

### 30.7 映射表：资源与平台能力

| 原版 JS 概念 | 当前 C 对应 | 说明 |
|---|---|---|
| 浏览器里直接加载 `images/...` / `sound/...` | `fruit_ninja_assets.c` | 负责把仓库内相对资源路径拼成 LVGL / SDL 可用路径 |
| 浏览器音频对象 `sound.create(...)` | `fruit_ninja_audio.c` | 当前统一由 `SDL2_mixer` 管理 |
| Raphael/DOM 图层 | `create_layer()` 建出的 LVGL layer | 平台对象模型完全不同，但分层思想是一致的 |

### 30.8 如果你要继续“对着 JS 学当前 C”，推荐对照顺序

最省力的对照方法不是整份 `all.js` 顺着读，而是按下面顺序做模块映射：

1. 先看 `all.js` 里首页与 game start 段落
   - 对照 `enter_home()` / `enter_running()`
2. 再看 `infos` / `getType()` / `shotOut()`
   - 对照 `g_fruit_defs[]` / `choose_fruit_def()` / `spawn_one_fruit()` / `update_fruits()`
3. 再看 `sliceAt()` / `broken()` / `apart()`
   - 对照 `handle_segment_hits()` / `slice_fruit()` / `spawn_fragment()` / `update_fragments()`
4. 再看 `lose` / `score` / `light`
   - 对照 `update_miss_icons()` / `update_score_label()` / `update_score_pulse()` / `enter_exploding()`
5. 最后再回看整个 `update_timer_cb()`
   - 你会更容易理解它相当于把原版多个 timeline / message 模块集中收拢到了同一帧循环里

### 30.9 这一份映射表的真正用途

这张映射表最适合 3 种场景：

1. 你在读原版 JS，不知道当前 C 里该看哪里
2. 你想验证某个玩法是不是已经对齐原版真值
3. 你准备继续把当前 LVGL 版往“更高保真复刻”推进

如果是第 3 种情况，优先检查这些差距：

- 首页多入口功能是否要补回来
- `juice` 类特效是否要单独实现
- miss 的底部演出是否要补回
- score / lose / flash 动画是否要进一步贴近原版节奏
- 当前 `scene.c` 是否要重新拆分，避免继续膨胀

---

## 31. `fruit_ninja_scene.c` 按源码行区块阅读指南

如果你现在准备直接打开 `main/src/v9-fruit_ninja/fruit_ninja_scene.c` 学，我最建议的方式不是从第 1 行机械读到最后，而是按“行区块职责”来读。这样你更容易在脑子里形成结构地图。

### 31.1 第 1-70 行：常量、全局状态、静态水果真值

这一段主要回答 3 个问题：

1. 这个场景的全局节奏参数是什么
2. 游戏里有哪些水果类型
3. 首页菜单水果用的是哪 3 个对象

重点看这些定义：

- `FRUIT_NINJA_UPDATE_MS`：全局 tick 间隔，16ms
- `FRUIT_NINJA_EXPLODING_MS`：爆炸演出持续 4 秒
- `FRUIT_NINJA_DROP_TIME_MS`：上抛/下落主阶段用时 1200ms
- `FRUIT_NINJA_JS_START_Y`：与 JS 真值对齐的出生高度 `600`
- `g_fruit_defs[]`：6 种水果/炸弹的资源与几何真值
- `g_home_menu_defs[]` 和 `g_home_menu_positions[]`：首页菜单水果

学习建议：

- 先把这里当作“玩法静态配置表”看
- 暂时不要陷入后面函数细节
- 先记住：当前实现很多行为参数不是分散的，而是在文件头部就定死了

### 31.2 第 72-147 行：基础工具函数与 UI 资源路径表

这一段是 scene 的“辅助底座”，主要内容有：

- 随机浮点工具 `frand_range()`
- 两个 easing 函数
- 图层创建 `create_layer()`
- 文件图片创建 `create_file_image()`
- 图片位置/尺寸设置 `set_image_geometry()`
- UI 资源路径注册 `init_ui_asset_paths()`

这一段阅读重点：

- `create_layer()` 体现了“所有层都去样式化、透明化、不可滚动”的统一基线
- `init_ui_asset_paths()` 实际上是 scene 这一层对资源键名的集中注册点

如果后面你改资源名字、换首页图、换 game over 图，这一段通常就是第一修改入口。

### 31.3 第 149-193 行：刷怪与 HUD 的最小辅助逻辑

这里主要有 4 个小函数：

- `choose_fruit_def()`：水果/炸弹概率分配
- `active_running_fruits()`：统计当前活跃未切水果
- `target_fruit_count()`：当前目标水果数量
- `update_score_label()` / `update_miss_icons()`：HUD 更新

这一段很短，但很关键，因为它把“难度/刷怪”和“显示层更新”两个系统的基本接口先立住了。

阅读时记住两点：

- `choose_fruit_def()` 里已经把 `1/8` 炸弹概率定死了
- `active_running_fruits()` 只数 `active && !sliced`，这决定了“补齐波次”的统计口径

### 31.4 第 195-272 行：通用对象显隐/销毁/清场函数

这段是典型的“场景 housekeeping 区域”，包括：

- `hide_obj()` / `show_obj()`
- `destroy_if_present()`
- `clear_fragments()`
- `clear_home_menu_fruits()`
- `clear_fruits()`
- `clear_explosion_overlays()`

这类函数虽然看起来普通，但它们决定了状态切换是否干净。

你以后如果遇到这类问题：

- 回首页后残留旧水果
- 炸弹白闪没有消失
- 结束后碎片还挂在屏幕上

高概率就是这一段清场链路没被正确触发。

### 31.5 第 253-341 行：瞬时特效系统

这一段包括：

- `spawn_flash()`
- `clear_flash_if_needed()`
- `update_score_pulse()`

建议把它理解成两个轻量特效子系统：

1. 刀光 flash 子系统
   - 创建一张图片
   - 记录年龄
   - 后续按年龄缩放 + 淡出 + 销毁

2. score 图标 pulse 子系统
   - 不创建新对象
   - 只是对既有 `score_image` 做短时放大再恢复

这是很典型的 LVGL 小游戏做法：

- 重要反馈保留
- 特效对象数量极少
- 生命周期逻辑全部挂到主心跳里统一推进

### 31.6 第 343-387 行：碎片池分配与碎片对象创建

这里有两个关键函数：

- `alloc_fragment()`：从定长数组里找空槽
- `spawn_fragment()`：为某个切片图片创建一个运行中碎片实例

阅读这一段时特别要注意：

- 碎片不是切开瞬间靠现成对象变形得到的
- 而是切开后重新生成的新对象
- 它们的 `start_*` / `target_*` / `life_ms` 会在这里初始化为一套新的动画上下文

也就是说，切开后的左右半片系统，在实现上与“原水果继续飞”是两套对象模型。

### 31.7 第 389-497 行：四个主状态入口函数

这是整个文件最值得先吃透的一个区块：

- `enter_home()`
- `enter_running()`
- `enter_game_over()`
- `enter_exploding()`

建议把这 4 个函数放一起对读，因为它们共同定义了：

- 每个状态的初始化动作
- 哪些对象可见/不可见
- 哪些计数被清零
- 哪些音效会播放

学习技巧：

- 不要单独看某一个函数
- 把 4 个函数的“显示层变化”“计时器变量变化”“音频变化”横向比较

你会很快明白：状态机真正的控制权就在这一组函数手里。

### 31.8 第 499-570 行：水果实例的可视更新与出生逻辑

这段含两个核心能力：

- `update_single_fruit_visual()`：把逻辑坐标映射成图片位置/旋转
- `spawn_one_fruit()`：创建一个新的运行中水果实例

建议阅读顺序：

1. 先看 `update_single_fruit_visual()`
2. 再看 `alloc_fruit()`
3. 最后看 `spawn_one_fruit()`

原因是：

- 你先理解“一个水果怎样在屏幕上摆出来”
- 再看“一个水果怎样被分配槽位并灌入轨迹参数”
- 比直接啃 `spawn_one_fruit()` 容易很多

这段也是后面调手感时最常改的区域之一。

### 31.9 第 573-673 行：首页菜单构建与首页动画

这段主要包括：

- `build_home_menu_fruits()`
- `start_running_timer_cb()`
- `restore_home_fruit_timer_cb()`
- `update_home_animation()`

可以把它理解成“首页系统”自己的局部子模块。

其中最值得注意的是：

- 首页 3 个水果本身也是 `fruit_ninja_fruit_t`
- 只是挂在 `home_layer`，并且运动/交互语义不同
- 中间水果被切中后，并不是立即开始游戏，而是先经过 `240ms` 的反馈延迟

所以首页不是一堆静态图，而是一个拥有自己对象系统和输入命中逻辑的轻量小游戏场景。

### 31.10 第 675-788 行：切中结果与命中处理

这一段是玩法最核心的一组函数：

- `slice_fruit()`
- `handle_home_menu_hits()`
- `handle_segment_hits()`

建议按这个顺序读：

1. `handle_home_menu_hits()`：先理解首页为什么能“切水果开局”
2. `handle_segment_hits()`：再理解运行态怎么命中活跃水果
3. `slice_fruit()`：最后看命中后果到底怎么分叉

因为如果你先读 `slice_fruit()`，会知道结果，却不知道谁触发它；
先把命中入口理解了，再看结果，会更顺。

### 31.11 第 790-832 行：统一输入分发器

`input_event_cb()` 这一段很适合和第 675-788 行一起读。

你可以把它理解成：

- 上面那段决定“命中后怎么处理”
- 这一段决定“什么时候会发生命中检测”

关键观察点：

- `LV_EVENT_PRESSED` 只负责开始一次输入或做状态级跳转
- 真正的切割判定只发生在 `LV_EVENT_PRESSING`
- `GAME_OVER` 下点击屏幕会走 `enter_home()`，而不是重新构建 screen

### 31.12 第 834-935 行：运行时对象更新区

这里包括两个大函数：

- `update_fruits()`
- `update_fragments()`

这两个函数共同承担“运行中对象生命周期推进”的职责。

建议阅读时分成 4 个问题：

1. 一个水果怎么从上抛切换到下落
2. 一个水果什么时候开始算已经出现过
3. 一个水果什么时候算 miss
4. 一个碎片什么时候该回收

如果你带着这 4 个问题去读，这一大段代码会比直接逐句看清晰很多。

### 31.13 第 937-990 行：全局 tick 调度器

这是 `update_timer_cb()`，虽然前面已经讲过，但从源码行块角度，你要特别注意它在文件中的位置：

- 它出现在所有子系统函数都定义完之后
- 再往后就是 `create_static_scene()` 和 `fruit_ninja_start()`

这其实是一种很常见的 C 文件布局：

- 先把所有局部工具和子行为定义好
- 然后用一个总调度函数把它们串起来
- 最后给出总入口

所以从文件组织上看，`update_timer_cb()` 就是“scene 主行为收口点”。

### 31.14 第 992-1110 行：静态场景搭建 + 总入口收尾

最后一段包括：

- `create_static_scene()`
- `fruit_ninja_start()`

也就是说，文件结尾不是玩法逻辑，而是“把前面所有能力组装起来”。

这非常适合作为第二轮复习时从后往前看：

- 先看入口怎么拼装
- 再倒回去理解每个拼装零件

这种读法对建立全局结构感很有效。

### 31.15 最推荐的实战阅读方法

如果你下一步真要对着源码精读，我建议这样操作：

1. 打开 `main/src/v9-fruit_ninja/fruit_ninja_scene.c`
2. 先只看每个区块的首尾行号和函数名，不看细节
3. 再按下面顺序精读：
   - 第 992-1110 行
   - 第 389-497 行
   - 第 790-832 行
   - 第 937-990 行
   - 第 834-935 行
   - 第 675-788 行
   - 第 499-673 行
4. 每读完一块，就回到文档这一节对照“这一块在全局里负责什么”

这样你会比直接从第 1 行读到第 1110 行更快建立整体感。

---

## 32. 辅助模块按源码行块精读

前面的第 31 节只覆盖了 `fruit_ninja_scene.c`。这一节把另外 4 个关键辅助模块也按源码区块拆开，方便你把整个 `main/src/v9-fruit_ninja` 目录真正读透。

## 32.1 `fruit_ninja_input.c`：输入轨迹与刀痕模块

这个文件很小，但它在整套系统里非常关键，因为它负责把“连续鼠标拖拽”转成两种东西：

- 玩家看得到的刀痕线
- 游戏逻辑能拿来判定的一段新切割线段

### 第 1-13 行：空线段构造器

`empty_segment()` 的作用就是返回一个全零、`valid = false` 的空线段。

它的存在价值不在算法，而在接口语义：

- 任何时候如果本次拖动没有形成有效切割，就返回一个标准“空结果”
- 后续 scene 层只需要判断 `segment.valid`

这是一种很干净的输入接口风格。

### 第 15-32 行：刀痕线对象刷新

`update_trail_points()` 只负责一件事：

- 如果点数大于等于 2，就把 `points[]` 提交给 `lv_line_set_points()` 并显示线
- 否则隐藏线对象

这里最值得注意的不是那两行 no-op 坐标赋值，而是它体现出的设计：

- 刀痕是“一个可复用的 LVGL line 对象”
- 轨迹刷新只是更新点数组，而不是不断新建线段对象

### 第 34-75 行：真正的核心——`push_internal()`

这个函数是整个输入模块最重要的函数。

它做了 5 件事：

1. 根据 `reset_chain` 决定是不是开启一条新轨迹
2. 如果轨迹点满了，就左移数组，保留最近的点
3. 如果有上一个点，就计算本次移动距离
4. 若距离超过阈值，生成有效 `segment`
5. 把当前点写进轨迹并刷新刀痕线

其中最关键的逻辑是：

- `FRUIT_NINJA_SEGMENT_MIN_DIST`
- `FRUIT_NINJA_MAX_TRAIL_POINTS`

前者负责降噪，后者负责控内存和控视觉复杂度。

所以这个函数本质上是：

**拖拽采样 + 轨迹裁剪 + 新刀线段抽取**

### 第 77-88 行：`fruit_ninja_input_init()`

这是输入模块的 UI 初始化入口。

它会：

- 清空 `trail` 结构
- 在 `effect_layer` 上创建 `lv_line`
- 设定线宽、颜色、透明度
- 默认隐藏

学习重点：

- 输入模块并不知道 screen 或 home/running/game over
- 它只依赖调用方给它一个 `effect_layer`
- 所以它是一个依附于 scene、但相对独立的小 UI 子系统

### 第 90-100 行：`fruit_ninja_input_reset()`

这是完整重置刀痕的入口。

它不是“结束一次输入”，而是更强的 reset：

- active 清掉
- pressing 清掉
- last point 清掉
- 点数清零
- fade 计数清零
- 线隐藏

所以 scene 切状态时用的是这个函数，而不是 `fruit_ninja_input_end()`。

### 第 102-121 行：begin / push / end 三件套

这三个函数共同组成输入状态机最外层接口：

- `fruit_ninja_input_begin()`：标记开始按压，并清空旧链后加入第一个点
- `fruit_ninja_input_push_point()`：正常追加点；若调用时并未 pressing，则自动转 begin
- `fruit_ninja_input_end()`：结束按压，但保留刀痕进入 fade 阶段

设计上非常清楚：

- `begin/end` 是输入状态控制
- `push_point` 是轨迹推进

### 第 123-145 行：`fruit_ninja_input_tick()`

这个函数负责“玩家松手后的尾迹衰减”。

规则很简单：

- 按着时：线保持 `LV_OPA_90`
- 松开后：180ms 内逐步变淡
- 超过 180ms：直接 reset

这说明当前刀痕系统设计非常克制：

- 没有复杂粒子
- 没有多段尾焰
- 但足够清楚地传达“切割动作仍有残影”

## 32.2 `fruit_ninja_assets.c`：资源根定位与路径拼接模块

这个文件的职责只有一个：

**把仓库中的相对资源，变成 LVGL 和 SDL 能直接读取的实际文件路径。**

### 第 1-19 行：基础常量与存在性检查

最重要的是：

- `FRUIT_NINJA_ASSET_ROOT = "main/assets/fruit_ninja"`

这表示当前项目已经把游戏运行资源完全收口进仓库自己的 `main/assets/fruit_ninja` 目录，而不是继续依赖 `third-party/FruitNinja` 直接运行。

### 第 21-41 行：路径复制与尾斜杠修正

`fruit_ninja_assets_copy_root()` 的关键不是 memcpy，而是它会去掉路径末尾多余的 `/`。

这样后面拼接时可以避免：

- `//main/assets/...`
- 或根路径为空字符串的异常情况

这是一个很典型的“路径清洗辅助函数”。

### 第 43-90 行：自动推导项目根目录

这里是整个文件最关键的一段。

逻辑是：

1. 从 `getcwd()` 拿当前工作目录
2. 逐层向父目录回退
3. 每一层检查是否存在 `main/assets/fruit_ninja`
4. 找到就把这一层认定为项目根

这个设计很实用，因为它让你：

- 不需要写死绝对路径
- 在 repo 根目录或子目录里运行程序时都还有机会自动找到资源

### 第 92-145 行：路径拼接核心

`fruit_ninja_assets_build_path()` 是底层通用拼接器。

两个公开接口只是它的不同前缀包装：

- `fruit_ninja_assets_build_image_path()` -> 前缀 `S:`
- `fruit_ninja_assets_build_audio_path()` -> 无前缀

这里一定要记住：

- `S:` 是给 LVGL 文件图片加载链看的
- SDL_mixer 不需要 `S:`

这就是为什么图片和音频虽然都来自同一个资源根，但构造出来的路径字符串不同。

### 第 147-192 行：核心资源完整性校验

这一段把游戏运行最重要的图片和音频资源全部列成白名单：

- 背景
- 首页元素
- 阴影 / flash / smoke / game over
- 全部水果及其左右切片
- 全部关键音效

学习重点：

- 它校验的是“运行闭环必须资源”，不是所有仓库资源
- 一旦某项缺失，函数直接返回 false
- scene 层拿这个结果决定是否打 warning

所以这个文件不是“资源系统 + 自动修复器”，而是“资源定位器 + 启动前守门员”。

## 32.3 `fruit_ninja_audio.c`：SDL_mixer 音频桥接层

这个文件的特点是：

- 很薄
- 很直接
- 不做复杂音频状态机

它的职责就是把 scene 层的“我要播什么声音”翻译成 SDL_mixer 调用。

### 第 12-24 行：音频状态结构

`fruit_ninja_audio_state_t` 里只保存：

- 是否已初始化
- 一首 menu music
- 5 个关键短音效

它说明当前音频模型非常简单：

- 背景音乐只有菜单主音乐
- 玩法中的声音都走 chunk
- 没有复杂混音分组或 bus 系统

### 第 26-52 行：音乐/音效加载回退策略

这里有两个内部函数：

- `load_music()`
- `load_chunk()`

它们都遵循同一个策略：

1. 先尝试 `.ogg`
2. 如果失败，再尝试 `.mp3`

这非常符合当前项目目标：

- 优先使用更稳定、常见的 ogg
- 同时兼容 mp3 资源存在的场景

### 第 54-91 行：`fruit_ninja_audio_init()`

这是音频模块的真正初始化入口。

它做的事按顺序是：

1. 防止重复初始化
2. 初始化 mixer codec
3. 打开音频设备
4. 分配 8 个通道
5. 清空并标记全局状态
6. 按顺序加载 menu/start/throw/slice/boom/game over 资源

注意它的失败策略非常温和：

- `Mix_OpenAudio` 失败才真正返回 false
- 单个资源加载失败只是 `LV_LOG_WARN`

这说明它允许：

- 游戏逻辑照常跑
- 只是缺少部分音效

### 第 93-108 行：`fruit_ninja_audio_shutdown()`

这是一套非常标准的 SDL_mixer 收尾：

- 停音乐
- 停所有通道
- 释放 music/chunk
- 清空状态
- 关闭音频设备
- `Mix_Quit()`

当前 Fruit Ninja 场景启动逻辑里没有主动调用它，但如果未来支持：

- 退出程序时收尾
- 在多个游戏/应用场景之间切换

这个函数就是必要的生命周期出口。

### 第 110-157 行：各类播放包装函数

后半段每个函数都很薄：

- 判断是否已初始化
- 判断对应资源是否非空
- 调 `Mix_PlayMusic` 或 `Mix_PlayChannel`

这样 scene 层就不必关心：

- 资源有没有加载好
- 应该播在哪个 channel
- 当前 mixer 是否已初始化

所以这个文件就是一个典型“平台依赖隔离层”。

## 32.4 `fruit_ninja_collision.c`：几何碰撞核心

这个文件只有一个函数，但它承接了整个玩法最核心的数学判定。

### 第 3-40 行：`fruit_ninja_segment_hits_circle()`

算法本质是：

1. 求线段方向向量 `(dx, dy)`
2. 计算圆心在这条线上的投影比例 `t`
3. 把 `t` 限制在 `[0, 1]`
4. 算出线段上离圆心最近的点 `(px, py)`
5. 比较该点到圆心的距离平方和半径平方

它的优点是：

- 算法非常稳定
- 不依赖图像内容
- 对拖拽切割这种高速动作特别合适

在当前工程里，这个文件虽然最小，但它从玩法价值上看是“高杠杆模块”：

- 一旦这里错了，整套切水果就不可信
- 一旦这里稳定，scene 层只需要关心命中后果

## 32.5 读完整个目录后的结构心智模型

如果你把 `scene.c + input.c + assets.c + audio.c + collision.c + model.h` 全读完，应该在脑中形成这样一张图：

- `model.h`
  - 定义游戏世界里有哪些对象和状态
- `assets.c`
  - 负责把资源找出来
- `audio.c`
  - 负责把音效播出来
- `input.c`
  - 负责把拖拽变成轨迹和线段
- `collision.c`
  - 负责判断线段有没有切中水果
- `scene.c`
  - 负责把前面这些能力编排成一整局游戏

换句话说：

**前 5 个文件提供能力，`scene.c` 提供流程。**
