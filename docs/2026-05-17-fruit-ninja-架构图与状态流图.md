# Fruit Ninja 架构图与状态流图

## 1. 文档目标

这份文档不再按代码细节展开，而是把当前 Fruit Ninja 的整体结构抽象成几张“读图用”的材料，方便你在看源码前先建立全局心智模型。

文档覆盖 3 个层面：

- 架构图：整个 `main/src/v9-fruit_ninja` 目录里各模块如何协作
- 状态流图：从首页到开局、到运行、到爆炸、到结算的完整状态流转
- 模块关系图：scene、input、audio、assets、collision、model 之间谁依赖谁、谁提供能力、谁负责调度

如果前面的几份学习文档更像“教材正文”，这份文档更像“总览地图”。

---

## 2. 总体架构图

先看一句话总纲：

**当前 Fruit Ninja 是一个以 `fruit_ninja_scene.c` 为核心调度器、以 `fruit_ninja_model.h` 为数据骨架、以 `input/assets/audio/collision` 为能力模块、由单个 `16ms` 定时器驱动的 LVGL 小游戏系统。**

### 2.1 总体架构图

```text
+-------------------------------------------------------------+
|                          main.c                             |
|  lv_init() -> hal_init() -> fruit_ninja_start()             |
+-----------------------------+-------------------------------+
                              |
                              v
+-------------------------------------------------------------+
|                 fruit_ninja_scene.c                         |
|                                                             |
|  1. 初始化资源/音频/静态场景                                 |
|  2. 维护 HOME/RUNNING/EXPLODING/GAME_OVER 状态机            |
|  3. 接收输入事件并分发命中检测                               |
|  4. 通过 16ms timer 推进水果/碎片/特效/首页动画             |
+--------+----------------+----------------+------------------+
         |                |                |                  |
         v                v                v                  v
+----------------+ +----------------+ +----------------+ +----------------+
| fruit_ninja_   | | fruit_ninja_   | | fruit_ninja_   | | fruit_ninja_   |
| input.c        | | collision.c    | | audio.c        | | assets.c       |
|                | |                | |                | |                |
| 轨迹采样       | | 线段打圆碰撞   | | SDL_mixer      | | 资源根定位     |
| 刀痕显示       | | 命中几何判定   | | 音效/音乐播放  | | 图片/音频路径  |
| 生成切割线段   | |                | |                | | 核心资源校验   |
+--------+-------+ +--------+-------+ +--------+-------+ +--------+-------+
         \                |                |                  /
          \               |                |                 /
           \              |                |                /
            \             |                |               /
             v            v                v              v
              +------------------------------------------+
              |          fruit_ninja_model.h             |
              |                                          |
              |  游戏状态、果实模板、果实实例、碎片、     |
              |  刀痕、线段、全局 game 结构定义           |
              +------------------------------------------+
```

### 2.2 这张图怎么理解

最重要的结论有 4 个：

1. `main.c` 只负责把游戏启动起来，不负责玩法。
2. `fruit_ninja_scene.c` 是“导演”，其他 `.c` 文件更像“能力提供者”。
3. `fruit_ninja_model.h` 不是可有可无的头文件，而是整个系统的数据中枢。
4. 游戏运行时没有多个独立逻辑主循环，只有 scene 的一个 16ms timer 在统一推进。

---

## 3. 运行时分层图

从运行时对象层次看，当前实现不是传统游戏引擎 ECS，而是 LVGL 场景图分层。

### 3.1 图层关系图

```text
screen
├── background
├── home_layer
│   ├── home_mask_image
│   ├── logo_image
│   ├── home_desc_image
│   ├── ninja_image
│   ├── dojo_image
│   ├── new_game_image
│   ├── new_sign_image
│   ├── hint_label
│   └── home_menu_fruits[3]
├── fruit_layer
│   ├── running fruits[*].shadow_image
│   ├── running fruits[*].whole_image
│   └── fragments[*].image
├── effect_layer
│   ├── trail.line
│   └── flash_overlay
├── hud_layer
│   ├── score_image
│   ├── score_label
│   └── miss_icons[3]
├── overlay_layer
│   ├── game_over_image
│   ├── restart_label
│   ├── smoke_overlay
│   └── white_flash_overlay
└── input_layer
    └── LV_EVENT_PRESSED / PRESSING / RELEASED / PRESS_LOST
```

### 3.2 这层设计的意义

这套分层不是为了“看起来工整”，而是为了解决 4 个实际问题：

- 首页元素和运行态水果不能混在一层，否则切状态难清理
- 刀痕与切中 flash 应该盖在水果上面，但不应和 Game Over 覆盖层抢层级
- HUD 应该稳定显示，不受水果/特效位置影响
- 输入层必须在最前面，避免被别的对象挡住鼠标事件

所以这是一套非常典型的 UI 框架小游戏分层方案。

---

## 4. 核心数据结构关系图

### 4.1 数据结构关系

```text
fruit_ninja_game_t
├── state / score / misses / timers / spawn params
├── screen + layers + static UI object refs
├── trail : fruit_ninja_trail_t
│   ├── pressing / active / fade_ms
│   ├── points[24]
│   └── line
├── home_menu_fruits[3] : fruit_ninja_fruit_t
├── fruits[16] : fruit_ninja_fruit_t
└── fragments[32] : fruit_ninja_fragment_t

fruit_ninja_fruit_t
├── lifecycle flags: active / sliced / counted_as_miss / has_been_visible / falling
├── def : fruit_ninja_fruit_def_t *
├── motion: x/y/vx/vy/angle/angular_velocity
├── trajectory: shot_out_start/end, fall_target, phase_elapsed_ms
└── visuals: whole_image / shadow_image / slice_flash

fruit_ninja_fragment_t
├── active / life_ms / phase_elapsed_ms
├── start_x/y -> target_x/y
├── start_angle -> target_angle
└── image
```

### 4.2 关键理解点

- `fruit_ninja_game_t` 是“整局游戏世界”的容器。
- `fruit_ninja_fruit_t` 既被首页水果复用，也被运行态水果复用。
- 碎片不是水果状态的一部分继续演化，而是独立对象池。
- 刀痕本身也是 game 世界状态的一部分，而不是临时局部变量。

---

## 5. 启动时序图

### 5.1 程序启动到首页可见

```text
main()
  -> lv_init()
  -> hal_init(640, 480)
  -> fruit_ninja_start()
       -> memset(g_game)
       -> init_ui_asset_paths()
       -> fruit_ninja_assets_init()
       -> fruit_ninja_assets_validate_core_files()
       -> fruit_ninja_audio_init()
       -> create_static_scene()
       -> lv_timer_create(update_timer_cb, 16ms)
       -> lv_screen_load(g_game.screen)
       -> enter_home(&g_game)
  -> while(1) { lv_timer_handler(); }
```

### 5.2 时序理解

- 资源和音频能力先准备好
- 然后一次性建完整个 screen
- 再启动全局 timer
- 最后才进入 `HOME`

这意味着：

- 首页不是“启动前就有的默认屏幕”
- 而是程序启动后 scene 自己切进去的第一个状态

---

## 6. 输入 -> 切割 -> 命中的时序图

### 6.1 运行态切水果时序

```text
用户拖拽鼠标
  -> LV_EVENT_PRESSING
  -> input_event_cb()
  -> fruit_ninja_input_push_point(game, x, y)
       -> push_internal()
       -> 返回 segment
  -> handle_segment_hits(game, segment)
       -> fruit_ninja_segment_hits_circle(...)
       -> slice_fruit(game, fruit, dx, dy)
            -> 普通水果: spawn_fragment() + score++ + flash + slice sound
            -> 炸弹: enter_exploding()
```

### 6.2 首页开局时序

```text
用户切中首页中间水果
  -> input_event_cb()
  -> fruit_ninja_input_push_point()
  -> handle_home_menu_hits()
  -> lv_timer_create(start_running_timer_cb, 240ms, game)
  -> start_running_timer_cb()
  -> enter_running(game)
```

### 6.3 这里体现出的设计思想

- 输入模块只负责提供“最新一刀”
- 首页命中与运行态命中分成两条逻辑链
- 开局不是按钮切换，而是玩法式切换

---

## 7. 主循环状态流图

### 7.1 总状态流图

```text
+--------+
|  HOME  |
+--------+
    |
    | 切中首页中间水果
    v
+-----------+
|  RUNNING  |
+-----------+
   |      |
   |      | 切中炸弹
   |      v
   |   +-------------+
   |   | EXPLODING   |
   |   +-------------+
   |          |
   |          | 4 秒演出结束
   |          v
   |     +-------------+
   +---> |  GAME_OVER  |
 miss=3  +-------------+
                |
                | 点击屏幕
                v
             +--------+
             |  HOME  |
             +--------+
```

### 7.2 每个状态负责什么

#### `HOME`

- 首页 UI 展示
- 首页元素入场动画
- 菜单音乐播放
- 首页 3 水果交互

#### `RUNNING`

- 刷水果
- 更新水果飞行与旋转
- 处理切中与碎片
- 统计 score / miss

#### `EXPLODING`

- 白闪、烟雾、世界减速感
- 暂停正常玩法进程
- 倒计时后进入结算

#### `GAME_OVER`

- 展示 game over UI
- 播放结束音效
- 等待玩家点击回首页

---

## 8. 单帧调度图

### 8.1 `update_timer_cb()` 内部调度

```text
update_timer_cb()
├── tick_count += 16
├── state_elapsed_ms += 16
├── flash_age_ms += 16               [if flash exists]
├── clear_flash_if_needed()
├── update_score_pulse()
├── fruit_ninja_input_tick()
├── update_home_animation()          [HOME]
├── spawn logic                      [RUNNING]
│   ├── spawn_elapsed_ms += 16
│   ├── if elapsed >= interval
│   ├── target = target_fruit_count()
│   ├── active = active_running_fruits()
│   └── while need spawn -> spawn_one_fruit()
├── update_fruits()
├── update_fragments()
└── exploding overlay fade / enter_game_over()   [EXPLODING]
```

### 8.2 为什么这很重要

这张图能直接解释两个最常见的问题：

- 为什么当前实现不需要多个“水果 update timer / 特效 timer / 首页动画 timer”
- 为什么状态 bug 一般都能在 `update_timer_cb()` 附近找到主线原因

因为这里就是所有子系统的统一调度入口。

---

## 9. 水果生命周期图

### 9.1 普通水果生命周期

```text
alloc_fruit()
  -> spawn_one_fruit()
  -> active = true
  -> 出生在 y = 600
  -> 上抛阶段
  -> 下落阶段
     ├── 被切中 -> sliced = true -> spawn_fragment() x2 -> 最终回收
     └── 未切中 -> 离屏 -> miss++ -> 回收
```

### 9.2 炸弹生命周期

```text
alloc_fruit()
  -> spawn_one_fruit()
  -> active = true
  -> 上抛 / 下落
     ├── 被切中 -> enter_exploding()
     └── 未切中 -> 掉出屏幕 -> 直接回收，不计 miss
```

### 9.3 这里的重点差异

- 普通水果会参与 miss 系统
- 炸弹不会记 miss
- 普通水果切开后变成两个碎片继续演出
- 炸弹切中后不产生左右半片，而是把整局切到 `EXPLODING`

---

## 10. 碎片生命周期图

```text
slice_fruit()
  -> spawn_fragment(left)
  -> spawn_fragment(right)
  -> fragment.active = true
  -> update_fragments()
       -> 按 start -> target 做补间
       -> life_ms 递减
       -> 超时或离屏
       -> destroy image + memset(fragment)
```

它的本质是：

- “命中后额外生成的短寿命动画实体”
- 不会回到水果对象池
- 也不参与 miss 判定

---

## 11. 模块依赖关系图

### 11.1 依赖方向图

```text
main.c
  -> fruit_ninja.h
       -> fruit_ninja_scene.c
            -> fruit_ninja_model.h
            -> fruit_ninja_assets.h
            -> fruit_ninja_audio.h
            -> fruit_ninja_collision.h

fruit_ninja_input.c
  -> fruit_ninja_model.h

fruit_ninja_audio.c
  -> fruit_ninja_assets.h

fruit_ninja_collision.c
  -> fruit_ninja_collision.h

fruit_ninja_assets.c
  -> fruit_ninja_assets.h
```

### 11.2 依赖结构的含义

- `scene.c` 是唯一几乎依赖所有其他模块的地方
- `input/audio/collision/assets` 之间彼此基本不强耦合
- 这种结构天然说明：scene 是 orchestration 层，其余是 capability 层

这也是当前目录整体可读性不错的原因之一。

---

## 12. 原版 JS 与当前 C 的架构对照图

### 12.1 对照图

```text
原版 JS                                   当前 C
---------------------------------------------------------------
message bus                               同步函数调用
timeline tasks                            16ms 全局 timer
scene module                              scene.c 状态入口函数群
ClassFruit 对象方法                       fruit struct + update_fruits()
knife 模块广播 slice                      input.c 返回 segment + scene 分发
light / lose / score 各自模块             scene.c 统一编排 + 少量辅助函数
动态数组 / cache                          定长对象池
```

### 12.2 核心结论

原版更像：

- 模块事件驱动的网页小游戏框架

当前 C 版更像：

- 集中式状态机 + 统一心跳驱动的本地 LVGL 重写版

但两者玩法骨架高度一致，所以很适合“一边对照、一边学习”。

---

## 13. 推荐使用方式

这份图解文档最适合和前面 3 份文档配合：

1. 先看 `docs/2026-05-17-fruit-ninja-架构图与状态流图.md`
   - 建立整体地图
2. 再看 `docs/2026-05-17-v9-fruit-ninja-学习文档.md`
   - 进入代码与实现细节
3. 再看 `docs/2026-05-17-third-party-fruit-ninja-all-js-学习文档.md`
   - 理解原版行为真值
4. 最后看 `docs/2026-05-17-fruit-ninja-js-c-逐函数对照表.md`
   - 做快速查表和对照定位

---

## 14. 一句话总结

如果把当前 Fruit Ninja 压缩成一句话：

**它是一个由 `scene.c` 统筹、以 `model.h` 为数据骨架、借助 `input/assets/audio/collision` 提供能力、用单个 16ms timer 推动状态机和对象池演化的 LVGL 小游戏系统。**

理解了这句话，再回头看所有源码细节，就不容易迷路。
