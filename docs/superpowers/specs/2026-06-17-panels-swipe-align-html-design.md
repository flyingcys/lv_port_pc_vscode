# 面板上下滑动效果对齐 HTML design-ui

**日期**：2026-06-17  
**分支**：release/v9.3-replace2  
**目标**：使 LVGL icon_replace_2 的控制中心/通知中心上下滑动行为与 design-ui/index.html 完全一致

---

## 背景

HTML design-ui 的面板开合是**阈值触发**模式：手指滑过 50px 阈值后松手，面板以带回弹的弹性动画一次性吸附到目标位置，面板本身不跟手实时移动。

LVGL 当前实现是**实时跟手**模式：面板随手指位移实时移动，松手后再判断吸附方向。动画也是线性 250ms，无回弹。

用户要求：完全复刻 HTML 模式（方案 A）。

---

## 目标行为

| 动作 | 触发条件 | 结果 |
|------|---------|------|
| 下拉展开控制中心 | 从顶部边缘感应条按下并下滑 > 50px | 面板以弹性动画滑入（400ms overshoot） |
| 上滑收起控制中心 | 控制中心已展开，在面板上上滑 > 50px | 面板以弹性动画滑出（400ms overshoot） |
| 上拉展开通知中心 | 从底部边缘感应条按下并上滑 > 50px | 面板以弹性动画滑入（400ms overshoot） |
| 下滑收起通知中心 | 通知中心已展开，在面板上下滑 > 50px | 面板以弹性动画滑出（400ms overshoot） |

面板在手势过程中**不跟手移动**，保持原位直到阈值触发。

---

## 改动范围

### 1. `icon_replace_2.c` — 边缘感应条回调（展开手势）

**当前**：`PRESSED` 调 `drag_begin`，`PRESSING` 实时调 `drag_update`，`RELEASED` 调 `drag_end`

**新逻辑**：

```
edge_top_pressed_cb:
  记录 edge_top_press_y，置 edge_top_dragging = 1
  （删除 ir2_panels_drag_begin 调用）

edge_top_pressing_cb:
  delta = pt.y - edge_top_press_y
  if delta > 50px:
    ir2_panels_show_control(panels, true)
    edge_top_dragging = 0

edge_top_released_cb:
  edge_top_dragging = 0
  （删除 ir2_panels_drag_end 调用）

底部条 edge_bot_* 对称处理（delta = edge_bot_press_y - pt.y）
```

删除对 `ir2_panels_drag_begin/update/end` 的全部调用。

### 2. `icon_replace_2_panels.c` — 收起手势 + 动画曲线

**收起手势**（`panel_pressed_cb` / `panel_pressing_cb` / `panel_released_cb`）：

```
panel_pressed_cb:
  记录 handle_press_y，置 dragging = which
  （删除 drag_begin_internal 调用，不再设 cur_reveal）

panel_pressing_cb:
  计算 close_amount（方向判断不变：control 向上收，notify 向下收）
  if close_amount > 50px:
    ir2_panels_show_control/show_notify(p, false)
    p->dragging = 0

panel_released_cb:
  p->dragging = 0
  （删除 snap_release 调用）
```

**动画曲线**（`slide_to` 函数）：

```c
// 当前
lv_anim_set_time(&a, 250);

// 改为
lv_anim_set_time(&a, 400);
lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
```

**删除死代码**（不再被任何路径调用）：
- 函数：`drag_begin_internal`、`drag_update_internal`、`apply_reveal`、`snap_release`
- struct 字段：`dragging`、`cur_reveal`、`last_delta`

### 3. `icon_replace_2_panels.h` — 删除公开 drag API

删除以下声明（调用方 icon_replace_2.c 同步删除）：
```c
void ir2_panels_drag_begin(icon_replace_2_panels_t * p, int which);
void ir2_panels_drag_update(icon_replace_2_panels_t * p, int which, int32_t reveal);
void ir2_panels_drag_end(icon_replace_2_panels_t * p, int which);
```

---

## 不改动

- 面板高度（保持全屏 screen_h，已有意改为全屏）
- 面板圆角（保持 0，全屏铺满无需圆角）
- `ir2_panels_show_control` / `ir2_panels_show_notify` 接口不变
- `ir2_panel_drag_y`、`ir2_panel_snap_open` 几何辅助函数（在 panels_geom 模块，可后续清理）
- 边缘感应条尺寸和位置

---

## 验证标准

1. 从顶部边缘下滑 > 50px，控制中心以回弹动画展开
2. 控制中心展开后，在面板上上滑 > 50px，以回弹动画收起
3. 从底部边缘上滑 > 50px，通知中心以回弹动画展开
4. 通知中心展开后，在面板上下滑 > 50px，以回弹动画收起
5. 滑动未到阈值时面板不移动
6. 现有图标拖拽、页面切换功能不受影响
