# v9-fruit_ninja LVGL 实现速记

切水果小游戏：HOME 菜单 → 抛水果 → 滑动切开 → 计分/三次失误结束。逻辑坐标恒定 640×480，运行时 letterbox 映射到任意物理分辨率。

> 截图位：`![demo](../images/fruit_ninja_demo.png)`

---

## 1. 文件分层

| 文件 | 职责（一行） |
|---|---|
| `fruit_ninja.h` | 对外仅 3 个 API：`create / start / stop` |
| `fruit_ninja_scene.c` | 启动入口、对象树搭建、`lv_timer` 主循环回调 |
| `fruit_ninja_state.c` | 状态机（HOME/RUNNING/EXPLODING/GAME_OVER）+ HUD 更新 |
| `fruit_ninja_physics.c` | 水果/碎片抛体运动，每帧改 `lv_image` 的 pos/rotation/scale |
| `fruit_ninja_collision.c` | 线段-圆相交判定（纯算法，无 LVGL） |
| `fruit_ninja_input.c` | 监听 `input_layer` 的 PRESSED/PRESSING/RELEASED，采集刀痕点 |
| `fruit_ninja_effects.c` | 闪光/汁液/爆炸光线/火焰——全部画在一块 `lv_canvas` 上 |
| `fruit_ninja_viewport.c` | 逻辑→物理坐标线性映射（scale + offset） |
| `fruit_ninja_easing.c` | 缓动函数（quad/expo） |
| `fruit_ninja_audio.c` | SDL_mixer 音效封装 |
| `fruit_ninja_assets.c` | 资源路径拼接 + 文件存在性校验 |
| `desktop/fruit_ninja_app.c` | 桌面启动器入口，建背景图 + 返回按钮 |

---

## 2. 启动到首帧

`fruit_ninja_app_launch()` → `desktop_app_launcher_open_with_close(builder, ...)` →
1. `fruit_ninja_create(parent, w, h)` 进入 `fruit_ninja_create_internal`
2. `fruit_ninja_viewport_init(640, 480)`：算出 scale / offset
3. `fruit_ninja_assets_init / audio_init`：加载贴图、SDL_mixer
4. `create_static_scene()`：建 `screen` 容器 + 6 个 layer + HUD + 背景图 + 输入监听 + canvas 特效层
5. `lv_timer_create(update_timer_cb, 16ms, &g_game)` 启动主循环；进入 HOME 状态

---

## 3. 场景对象树

`screen (lv_obj)` ─ 居中放置，固定 640×480 容器
├── `background` `lv_image` 整屏背景
├── `home_layer` `lv_obj` HOME 菜单（logo / ninja / new-game / quit / dojo / 三颗演示水果）
├── `fruit_layer` `lv_obj` 水果实体 + 碎片
├── `effect_layer` `lv_obj` 内含 `effect_canvas` `lv_canvas`（ARGB8888，800×480 静态 buf）
├── `hud_layer` `lv_obj` 计分图标 `lv_image` + 计分文本 `lv_label` + 3 个 miss 图标 `lv_image`
├── `overlay_layer` `lv_obj` game-over 图
└── `input_layer` `lv_obj`（透明 + CLICKABLE，置顶接收触控）

控件选型要点：
- **水果整体 / 碎片 / 闪光 / HUD 图标**：全部用 `lv_image` 加载 PNG，靠 `lv_image_set_scale / set_rotation / set_pivot` 做缩放旋转。
- **刀痕、汁液、爆炸光线、火焰**：全部画到同一块 `lv_canvas` 上（不每帧 create 控件）。
- **分数**：`lv_label` + `lv_label_set_text_fmt`，配合 `lv_image_set_scale` 让分数图标做心跳脉冲。
- **layer 容器**：`lv_obj_create` + `lv_obj_remove_style_all` + 透明背景，仅做 z-order 分组。

---

## 4. 帧驱动

- **统一节拍**：`lv_timer_create(cb, 16ms, &game)`，约 62.5 FPS。**没有用 `lv_anim`**——动画全部手工算。
- 每个 tick `update_timer_cb` 干的事：
  ```
  poll audio
  tick_count += 16
  effects_update_flash       // 闪光缩放/淡出
  effects_update_score_pulse // 分数图标心跳
  state_update_miss_pop      // miss 图标弹出
  input_tick                 // 刀痕活跃期
  if HOME:      state_update_home_animation
  if RUNNING:   按 spawn_interval 生成水果
  physics_update_fruits / update_fragments  // 改每个 lv_image 的 pos/angle
  if EXPLODING: 抖背景 + 渐隐白闪/烟雾 overlay
  effects_render(16)         // 重画 canvas
  ```
- 坐标更新模式：`lv_obj_set_pos(img, viewport_x(lx) - w/2, viewport_y(ly) - h/2)` + `lv_image_set_rotation(img, angle*10)`。

---

## 5. 刀痕（关键）

不用 `lv_line`、不用每帧 create 对象。

```c
// 输入回调里
segment = input_push_point(game, lx, ly);   // 返回上一点→当前点的线段
effects_push_blade(game, x1, y1, x2, y2);   // 写入一个 ring buffer（MAX_BLADE_SEGMENTS 段）
```

每帧 `effects_render()`：
```c
lv_canvas_fill_bg(canvas, black, LV_OPA_TRANSP); // 整块清零
lv_canvas_init_layer(canvas, &layer);
for each blade segment:
    age += 16
    width = 10 * (1 - age/200)          // 线宽随年龄收缩
    lv_draw_line_dsc_t ld = {color=0xcbd3db, opa=90%, round_start=1, round_end=1, p1, p2, width}
    lv_draw_line(&layer, &ld)
lv_canvas_finish_layer(canvas, &layer);
```
汁液用 `lv_draw_arc`（实心圆 width=radius），爆炸光线用 `lv_draw_triangle`，火焰也是实心 arc。**碰撞用的逻辑点单独存在 `trail->points[]`**，绘制和碰撞解耦。

Canvas buffer 是文件级静态：`static uint8_t g_canvas_buf[800*480*4]`，1.5 MB，不动态分配。

---

## 6. 分辨率自适应（`viewport.c`）

22 行干完。逻辑系恒定 640×480：

```c
scale = min(phys_w/640, phys_h/480);   // 等比
off_x = (phys_w - 640*scale) / 2;       // letterbox 居中
off_y = (phys_h - 480*scale) / 2;

viewport_x(lx) = off_x + lx*scale
viewport_y(ly) = off_y + ly*scale
viewport_len(l)= l*scale
viewport_scale()= scale
```

所有定位经 `fruit_ninja_set_image_geometry` 或 `fruit_ninja_place_logic` 走一遍换算；`lv_image_set_scale(img, viewport_scale()*256)` 让位图本身也跟着缩。触控反向：`viewport_to_logic_x/y` 把 indev 的物理坐标拉回逻辑系再喂碰撞。支持 800×480 / 640×480 / 480×272 自动适配。

---

## 7. 性能要点

- **复用对象**：HOME / HUD / 背景图在 `create_static_scene` 里一次性建好；每帧只改属性（pos / rotation / scale / opa / hidden），不调 `lv_obj_create`。
- **水果池**：`g_game.fruits[]`、`fragments[]` 数组+`active` 标记，sliced 后 `lv_obj_delete` 并复用槽位（physics.c）。
- **特效不创建控件**：刀痕、汁液、火焰、爆炸光线统统画在一块 `lv_canvas` 上，靠 ring buffer + 年龄字段管理生命周期，避免大量 `lv_obj_create / delete`。
- **canvas 静态 buf**：上限 800×480 ARGB8888，超界分辨率退化为左上角区域，零运行时分配。
- **层 z-order 固定**：`fruit < effect < hud < overlay < input`，用 `lv_obj_move_foreground` 一次决定，不每帧重排。
- **Label 不缩放位图**：分数 `lv_label` 用 `place_logic(..., scale_image=false)`，避免对文本做无意义 scale。
- **抖动只动 background**：EXPLODING 阶段 `lv_obj_set_pos(background, ±6, ±6)`，不动其它子树，减少重绘范围。
- **stop 时彻底回收**：`lv_timer_delete` 主循环 + 所有 pending 定时器，`lv_obj_delete(screen)` 整树释放，`g_game` 清零。

---

## 8. 常用 LVGL API 速查

```
lv_obj_create / lv_obj_remove_style_all / lv_obj_set_size / lv_obj_set_pos
lv_obj_set_style_bg_opa / set_style_opa / add_flag(HIDDEN/CLICKABLE/SCROLLABLE)
lv_obj_move_foreground / move_background / lv_obj_delete

lv_image_create / lv_image_set_src / set_scale / set_rotation / set_pivot / set_inner_align
lv_label_create / lv_label_set_text_fmt / lv_obj_set_style_text_color

lv_canvas_create / lv_canvas_set_buffer(ARGB8888) / fill_bg / init_layer / finish_layer
lv_draw_line / lv_draw_arc / lv_draw_triangle  (走 lv_layer_t)

lv_timer_create / lv_timer_get_user_data / lv_timer_delete
lv_obj_add_event_cb (PRESSED/PRESSING/RELEASED/PRESS_LOST)
lv_indev_active / lv_indev_get_point / lv_obj_get_coords
```
