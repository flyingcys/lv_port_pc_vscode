# Fruit Ninja LVGL v9 — 1:1 复刻设计方案

> 状态:设计确认中
> 分支:`feat/fruit-ninja-1to1`(从 `feat/apple-music-lvgl-ui` 切出)
> 日期:2026-06-14
> 标准答案来源:`third-party/FruitNinja/scripts/all.js`(JS 原版)
> 目标代码:`main/src/v9-fruit_ninja/`(LVGL v9.1)

## 背景

`main/src/v9-fruit_ninja/` 是早先照 `third-party/FruitNinja` 这个 JS 版复刻的 LVGL v9 实现,但效果不到位。经代码核对,资源(图片/音频)与加载链路(libpng / tjpgd / `S:` 文件系统)都是健全的,**问题集中在特效实现简陋**:

1. **刀光**——JS 是"每段轨迹独立存活 200ms、线宽从 10px 线性衰减到 0",逐段堆叠成流动拖尾;现状是单条 `lv_line` 整体渐隐,缺拖尾感。
2. **汁液飞溅**——JS 每切一刀爆 10 个果汁粒子(果色 / 径向飞散 / 重力 / 1.5s 缩放消失);现状**完全没有**。
3. **炸弹效果**——JS 是"10 道爆炸光线 + 背景抖动 4s + 持续火焰 + 白屏渐隐";现状只有烟雾 + 白闪。
4. **分裂物理**——JS 两半带重力 + 随机旋转飞散;现状是线性插值到目标点,略生硬。

此外 `fruit_ninja_scene.c` 已 1110 行、职责混杂,这次大改顺势拆分。

## 目标与原则

- **严格对齐 JS 版行为,不增不减**。所有效果、数值、时序以 `all.js` 为唯一标准。**不引入** JS 原版没有的特性(连击倍数、Zen/多模式、特殊道具、冰冻等)——这些 JS 源码里都没有。
- **位图走 `lv_image`,矢量特效走 `lv_canvas`**。水果本体 / 分裂两半 / 切割闪光 / UI 用图片;刀光 / 汁液 / 炸弹光线 / 火焰 / 白闪自绘到 canvas。
- **逻辑坐标系固定 640×480**(JS 画布尺寸)。物理与所有 JS 数值在该虚拟坐标系内完全复用,只在显示层做一次缩放映射。
- **多分辨率自适应**:800×480 / 640×480 / 480×272 三档,等比缩放居中(letterbox),保证游戏比例与手感跨分辨率一致。

---

## 一、多分辨率自适应(letterbox + viewport)

新增模块 `fruit_ninja_viewport`,把"逻辑坐标(640×480)"映射到"物理屏(W×H)"。游戏逻辑层永远只认 640×480,显示层负责缩放居中。

### 缩放与偏移

```
scale    = min(W / 640.0, H / 480.0)      // 等比,保证不裁切
offset_x = (W - 640 * scale) / 2          // 水平居中
offset_y = (H - 480 * scale) / 2          // 垂直居中
```

三档实测:

| 物理屏 | scale | 游戏区 (逻辑×scale) | 留边 |
|--------|-------|---------------------|------|
| 800×480 | 1.000 | 640×480 居中 | 左右各 80px |
| 640×480 | 1.000 | 640×480 铺满 | 无 |
| 480×272 | 0.567 | 363×272 居中 | 左右各 ~58px |

### 映射接口(`fruit_ninja_viewport.h`)

```c
void  fn_vp_init(int phys_w, int phys_h);     // 算 scale / offset
float fn_vp_x(float logic_x);                 // 逻辑 X → 物理 X
float fn_vp_y(float logic_y);                 // 逻辑 Y → 物理 Y
float fn_vp_len(float logic_len);             // 标量缩放(线宽 / 半径)
float fn_vp_scale(void);                      // 取 scale(给 lv_image zoom 用)
float fn_vp_to_logic_x(float phys_x);         // 物理 X → 逻辑 X(输入命中)
float fn_vp_to_logic_y(float phys_y);
```

### 各元素如何应用

- **背景**:`background.jpg` 铺满整个物理屏(可非等比拉伸——背景是道场纹理,轻微拉伸无感),覆盖 letterbox 留边,不出现黑边。
- **位图(水果 / 分裂两半 / 闪光 / UI,`lv_image`)**:位置用 `fn_vp_x/y` 映射;尺寸用 `lv_image_set_scale(obj, (uint16_t)(fn_vp_scale()*256))` 缩放(LVGL 256 = 100%,`LV_DRAW_SW_COMPLEX` 提供抗锯齿)。
- **Canvas 特效层**:canvas 覆盖整个物理屏(W×H,ARGB8888)。绘制刀光 / 汁液 / 光线时,坐标与线宽都过 `fn_vp_*` 映射后再画——矢量特效按**物理分辨率原生重绘**,缩放后依然清晰,不会像位图采样那样模糊。
- **输入**:SDL 鼠标给的是物理坐标,先 `fn_vp_to_logic_*` 转回 640×480 逻辑坐标,再做切割判定(判定全程在逻辑坐标系内,与 JS 完全一致)。

### 为什么不用 LVGL 整体 transform

对一个含 canvas 的容器做 `transform_scale` 会对 canvas 位图采样导致模糊,且输入命中要反算。手工 viewport 映射让矢量特效按物理分辨率重绘(最清晰),位图水果用 image zoom 缩放(位图缩放可接受),最可控。

---

## 二、模块拆分

借这次大改把 `scene.c`(1110 行)按职责拆开,**写面互不重叠**,便于后续多 subagent 并行实现:

| 文件 | 职责 | 主要内容 / 迁入函数 |
|------|------|---------------------|
| `fruit_ninja_scene.c`(瘦身 ~250 行) | 入口、静态场景 / 图层创建、主循环编排 | `fruit_ninja_start`、`create_static_scene`、`create_layer`、`update_timer_cb` |
| `fruit_ninja_viewport.c/.h`(新) | 逻辑↔物理坐标映射、三分辨率 letterbox | 见第一节接口 |
| `fruit_ninja_physics.c/.h`(新) | 水果抛物线、旋转、生成、碎片物理;缓动函数 | `ease_*`、`frand_range`、`spawn_one_fruit`、`choose_fruit_def`、`update_fruits`、`update_single_fruit_visual`、`alloc_fruit`、`alloc_fragment`、`spawn_fragment`、`update_fragments` |
| `fruit_ninja_effects.c/.h`(新,**核心**) | Canvas 特效层:刀光拖尾、汁液、炸弹光线 / 火焰、白闪、切割闪光、分数脉冲 | `spawn_flash`、`clear_flash_if_needed`、`update_score_pulse`、`clear_explosion_overlays` + 全新粒子系统 |
| `fruit_ninja_state.c/.h`(新) | 状态机、HUD、首页菜单动画 | `enter_*`、`update_score_label`、`update_miss_icons`、`build_home_menu_fruits`、`update_home_animation`、各 `clear_*` |
| `fruit_ninja_input.c`(已有,扩展) | 轨迹采集 + 切割判定派发 | 增 `slice_fruit`、`handle_segment_hits`、`handle_home_menu_hits`、`input_event_cb` |
| `fruit_ninja_collision.c`(已有) | 线段-圆碰撞(与 JS 一致,保留) | — |
| `fruit_ninja_assets.c`(已有,小改) | 资源路径 | 补登记缺失的 UI 图(ninja / dojo / quit / score 等) |
| `fruit_ninja_audio.c`(已有,小改) | 音效 | 对齐触发时机 |
| `fruit_ninja_model.h`(扩展) | 数据结构 | 加特效粒子结构体(刀光段 / 汁液 / 光线 / 火焰) |

---

## 三、Canvas 特效系统(灵魂)

`effect_layer` 改为覆盖物理屏的 `lv_canvas`(ARGB8888)。每帧:透明清空 → `lv_canvas_init_layer` + `lv_draw_*` 绘制所有活跃特效 → `lv_canvas_finish_layer`。所有坐标 / 尺寸过 `fn_vp_*` 映射。

1. **刀光拖尾**:刀光段数组,每段 `{sx,sy,ex,ey,age_ms}`,生命 **200ms**,`line_width = 10 × (1 − age/200)`(再过 `fn_vp_len`),颜色 **#cbd3db**。逐段衰减堆叠 → JS 那种流动刀痕。`lv_draw_line` 画到 canvas。
2. **汁液飞溅**:每切一刀爆 **10** 个粒子,果色专属(见附录),半径 10(过 `fn_vp_len`),生命 **1500ms**;径向角 `random(360°)`、距离 100–300px(`ease_out_expo` 插值)+ 重力 +200px(`ease_out_quad`)、`scale` 1→0。`lv_draw_arc` / 填充圆画到 canvas。banana 无汁液。
3. **切割闪光**:`flash.png` 在切点旋转缩放 `scale` 1e-5→1→1e-5,**~100ms**(保留 `lv_image`,过 viewport 缩放)。
4. **炸弹三件套**:
   - **10 道放射光线**:白色细三角从炸弹中心放射,**100ms** 间隔逐道出现。
   - **背景抖动**:`background` 位置 ±6px,**50ms** 一跳,持续 ~4s。
   - **持续火焰**:黄色火苗(`#fafad9`→`#f0ef9c`),**40ms** 生成,单簇 life 200–700ms,90% 概率,canvas 画。
   - **白屏渐隐**:全屏白覆盖 `opa` 1→0,**4000ms**。
5. **分数脉冲**:分数变化时 `score_label` `scale` 1→1.2→1,~60ms。

> 缓动函数:现状已有 `ease_out_quad`(=JS `quadratic.co`)、`ease_in_quad`(=`quadratic.ci`)。需补 `ease_out_expo`(=`exponential.co`,汁液距离)、`ease_out_back`(=`back.co`,生命图标弹出)。

### 性能

每帧重绘 canvas:最大 800×480×4 ≈ 1.5MB 填充 + 绘制,`LV_MEM_SIZE` 4MB 充足。62.5fps(`lv_timer` 16ms)下 PC 模拟器无压力。验证阶段实测帧率,若吃紧则只重绘脏区或缩小 canvas 到 letterbox 游戏区。

---

## 四、物理与游戏逻辑(对齐 JS 数值)

所有时序以 `delta_ms` 驱动(与帧率解耦):JS 是 `setInterval` 10ms(~100fps),LVGL 是 `lv_timer` 16ms(62.5fps),用 delta 累加保证**时长一致**而非帧数一致。

| 项 | JS 标准 | 现状 | 动作 |
|----|---------|------|------|
| 抛物线时长 | 1200ms,升 `quad.co` / 落 `quad.ci` | 已两段缓动 | 对齐缓动曲线与时长 |
| 旋转速度 | ±[60,50,40]°/s 随机 | ±90–270°/s | 改为 JS 值 |
| 生成 | volleyNum 起始 2,间隔 1000ms | 类似 | 对齐数值 |
| 难度递增 | `score > volleyNum × mult` 时 `volleyNum++, mult+=50`(mult 起始 5) | 类似 | 对齐数值 |
| 炸弹概率 | `random(8)==4` → 1/8 (12.5%) | 20% | 改 12.5% |
| 分裂两半 | targetX 左 `-(random275)` / 右 `random275+75`、targetY 600、转 `apartAngle ± (random150+50)`;X 线性 / Y `quad.ci` | 线性插值 | 改 JS 缓动与散射 |
| 计分 | 每果 +1,**无连击** | +1 | 保持 |
| 生命 | 3 命(x/xx/xxx),漏果扣(炸弹漏**不**扣);lose 图标 `back.co` 弹出 scale 1e-5→1 (500ms) → 停 1500ms → 缩回 (500ms) | 有 | 对齐弹出动画 |
| Game Over | `game-over.png` scale 1e-5→1 (500ms),点击回菜单 | 有 | 对齐 |
| 音效 | throw(抛)/ splatter(切)/ boom(炸)/ menu(首页)/ start(开始) | 有 | 对齐触发时机 |

碰撞:JS 用线段-椭圆(单一 radius,实为圆),现状线段-圆与之等价,**保留**。

---

## 五、资源、音频与集成

- **资源已全部就位,无需转换**:`main/assets/fruit_ninja/` 下水果整图 + 分裂两半、阴影、闪光、烟雾、生命图标、UI、音频齐全;`apple.png 66×66`、`sandia 98×85`、`background.jpg 640×480` 等尺寸与 JS 完全一致。
- **音频**:SDL_mixer,5 个 SFX + 背景音乐,对齐 JS 触发时机。
- **集成**:`main.c:86` 取消注释 `fruit_ninja_start()`(临时注释 `apple_music_create()`),或加环境变量切换。
- **运行**:须在项目根目录(`S:` 路径相对工作目录):`./build/bin/stream_player 800 480` / `640 480` / `480 272` 三档分别验证。

---

## 六、验证方式("一模一样"如何达标)

- **并排对比**:浏览器开 `third-party/FruitNinja/index.html` vs `stream_player`,逐项核对刀光 / 汁液 / 炸弹 / 分裂 / 生成节奏。
- **三分辨率回归**:800×480 / 640×480 / 480×272 各跑一遍,确认 letterbox 居中、比例不变、无裁切 / 拉伸。
- **截图对比**:复用 `AM_SHOT` 式机制抓关键帧。
- **数值对照清单**:用附录 A 常量表逐条验收。
- **帧率监测**:确认 canvas 每帧重绘 ≥60fps,三档都不掉帧。

---

## 七、测试策略

- 图形游戏以运行 + 目测 + 截图对比为主。
- 纯逻辑写轻量断言 / 单测:缓动函数(`ease_out_quad/in_quad/out_expo/out_back` 边界值)、线段-圆碰撞、viewport 映射(三档 scale/offset 与往返映射)、难度递增。

---

## 八、实施顺序建议(供 writing-plans 细化)

1. **地基**:`fruit_ninja_viewport`(含单测)+ 模块拆分骨架(把 scene.c 现有函数迁到 physics/state/effects/input,先保持行为不变,编译通过)。
2. **接入 viewport**:所有位图位置 / 缩放、输入命中走映射;三档能正常起跑(效果仍是旧的)。
3. **Canvas 特效层**:刀光拖尾 → 汁液飞溅 → 切割闪光(核心手感)。
4. **炸弹三件套**:光线 + 背景抖动 + 火焰 + 白闪。
5. **物理与逻辑数值对齐**:抛物线 / 旋转 / 生成 / 难度 / 分裂散射 / 生命弹出。
6. **集成与三分辨率回归 + 对照验收**。

---

## 附录 A:JS 数值常量对照表(验收清单)

### 画布与主循环
- 画布:640×480;主循环 JS `setInterval` 10ms(~100fps),LVGL `lv_timer` 16ms,均以 delta 时长驱动。

### 水果定义(尺寸 / 半径 / 基础旋转 / 汁液色)
| 类型 | 图 | 宽×高 | radius | base_rot | 汁液色 |
|------|----|-------|--------|----------|--------|
| peach | peach.png | 62×59 | 37 | −50° | #e6c731(黄) |
| sandia(西瓜) | sandia.png | 98×85 | 38 | −100° | #c00(红) |
| apple | apple.png | 66×66 | 31 | −54° | #c8e925(绿) |
| banana | banana.png | 126×50 | 43 | 90° | 无 |
| basaha | basaha.png | 68×72 | 32 | −135° | #c00(红) |
| boom(炸弹) | boom.png | 66×68 | 26 | 0° | 无 |

### 关键数值
| 常量 | 值 |
|------|----|
| dropTime(抛物线时长) | 1200ms |
| 旋转速度池 | ±[60,50,40]°/s |
| volleyNum 起始 / 间隔 | 2 / 1000ms |
| 难度递增 | `score > volleyNum × mult` → `volleyNum++, mult+=50`(mult 起始 5) |
| 炸弹概率 | 1/8(12.5%) |
| 刀光 | 色 #cbd3db,宽 10px,life 200ms,`width=10×(1−t/200)` |
| 汁液 | 10 粒 / 刀,r 10,life 1500ms,距离 100–300px,角 random(360°),重力 +200px,scale 1→0 |
| 切割闪光 | scale 1e-5→1→1e-5,~100ms |
| 炸弹光线 | 10 道,100ms 间隔 |
| 背景抖动 | ±6px,50ms 一跳,~4s |
| 火焰 | 40ms 生成,life 200–700ms,90% 概率,色 #fafad9→#f0ef9c |
| 白屏渐隐 | opa 1→0,4000ms |
| 分数显示 | 色 90-#fc7f0c-#ffec53,30px,脉冲 scale 1→1.2→1 ~60ms |
| 分裂两半 | targetX 左 −random(275) / 右 random(275)+75,targetY 600,转 apartAngle±(random150+50) |
| 生命图标弹出 | `back.co` scale 1e-5→1 (500ms) → 停 1500ms → 缩回 (500ms) |

### 缓动对应
| JS | LVGL | 用途 |
|----|------|------|
| quadratic.co | ease_out_quad(已有) | 抛物线上升 |
| quadratic.ci | ease_in_quad(已有) | 抛物线下落 / 分裂 Y |
| exponential.co | ease_out_expo(需补) | 汁液距离 |
| back.co | ease_out_back(需补) | 生命图标弹出 |

## 附录 B:viewport 映射示例

```
物理屏 800×480 → scale=1.0, offset=(80,0)
  逻辑点 (320,240) [画布中心] → 物理 (400,240) [屏幕中心] ✓
  逻辑刀光宽 10 → 物理 10

物理屏 480×272 → scale=0.567, offset=(58,0)
  逻辑点 (320,240) → 物理 (58+181, 136) = (239,136) [屏幕中心] ✓
  逻辑刀光宽 10 → 物理 5.67
```
