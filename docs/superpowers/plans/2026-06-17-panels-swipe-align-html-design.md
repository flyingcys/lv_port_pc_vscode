# 面板上下滑动对齐 HTML design-ui 实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 icon_replace_2 的控制中心/通知中心上下滑动行为改为阈值触发 + 弹性动画，与 design-ui/index.html 完全一致。

**Architecture:** 去掉实时跟手的 reveal 状态机，改为在感应条和面板收起回调中积累位移，超过 50px 阈值后调用已有的 `ir2_panels_show_control/show_notify`。`slide_to` 动画改为 400ms + `lv_anim_path_overshoot`。

**Tech Stack:** LVGL v9，C，CMake，SDL2 offscreen 截图验证

---

## 文件改动索引

| 文件 | 操作 |
|------|------|
| `main/icon_replace_2/icon_replace_2_panels.c` | 修改：slide_to 曲线、panel_pressed/pressing/released_cb、删死代码 |
| `main/icon_replace_2/icon_replace_2_panels.h` | 修改：删除 drag_begin/update/end 声明 |
| `main/icon_replace_2/icon_replace_2.c` | 修改：6 个 edge 感应条回调 |

---

## Task 1：改 slide_to 动画曲线

**Files:**
- Modify: `main/icon_replace_2/icon_replace_2_panels.c:36-45`

- [ ] **Step 1：确认现有测试基线通过**

```bash
cd /home/share/samba/lvgl/lv_port_pc_vscode_v3-music && bin/test_panels_geom
```

期望输出：`test_panels_geom: PASS`

- [ ] **Step 2：修改 slide_to 动画参数**

将 `icon_replace_2_panels.c` 的 `slide_to` 函数（第 36-45 行）改为：

```c
static void slide_to(lv_obj_t * o, int32_t y, bool anim){
    if(anim){
        lv_anim_t a; lv_anim_init(&a); lv_anim_set_var(&a,o);
        lv_anim_set_exec_cb(&a,(lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_time(&a,400); lv_anim_set_values(&a, lv_obj_get_y(o), y);
        lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
        lv_anim_start(&a);
    } else {
        lv_anim_delete(o, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_obj_set_y(o, y);
    }
}
```

- [ ] **Step 3：编译验证**

```bash
cmake --build /home/share/samba/lvgl/lv_port_pc_vscode_v3-music/build --target main -j4
```

期望：无编译错误

- [ ] **Step 4：截图确认面板位置正确（静态快照，不验证动画曲线）**

```bash
cd /home/share/samba/lvgl/lv_port_pc_vscode_v3-music
scripts/shot.sh 800x480 1 control /tmp/panel_control.png
scripts/shot.sh 800x480 1 notify  /tmp/panel_notify.png
```

期望：两张图各显示对应面板全屏展开，无位置异常

- [ ] **Step 5：提交**

```bash
cd /home/share/samba/lvgl/lv_port_pc_vscode_v3-music
git add main/icon_replace_2/icon_replace_2_panels.c
git commit -m "feat(panels): slide_to 动画改为 400ms overshoot 回弹曲线"
```

---

## Task 2：边缘感应条改为阈值触发展开

**Files:**
- Modify: `main/icon_replace_2/icon_replace_2.c:668-723`

- [ ] **Step 1：替换顶部感应条三个回调**

将 `icon_replace_2.c` 中的 `edge_top_pressed_cb`、`edge_top_pressing_cb`、`edge_top_released_cb` 整体替换为：

```c
static void edge_top_pressed_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(ir2_panels_active(panels) != 0) { edge_top_dragging = 0; return; }
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    edge_top_press_y = pt.y;
    edge_top_dragging = 1;
}

static void edge_top_pressing_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(!edge_top_dragging) return;
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    if(pt.y - edge_top_press_y > 50) {
        ir2_panels_show_control(panels, true);
        edge_top_dragging = 0;
    }
}

static void edge_top_released_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    edge_top_dragging = 0;
}
```

- [ ] **Step 2：替换底部感应条三个回调**

将 `edge_bot_pressed_cb`、`edge_bot_pressing_cb`、`edge_bot_released_cb` 整体替换为：

```c
static void edge_bot_pressed_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(ir2_panels_active(panels) != 0) { edge_bot_dragging = 0; return; }
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    edge_bot_press_y = pt.y;
    edge_bot_dragging = 1;
}

static void edge_bot_pressing_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(!edge_bot_dragging) return;
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    if(edge_bot_press_y - pt.y > 50) {
        ir2_panels_show_notify(panels, true);
        edge_bot_dragging = 0;
    }
}

static void edge_bot_released_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    edge_bot_dragging = 0;
}
```

- [ ] **Step 3：编译验证**

```bash
cmake --build /home/share/samba/lvgl/lv_port_pc_vscode_v3-music/build --target main -j4
```

期望：无编译错误，无 unused variable 警告（drag API 调用已全部删除）

- [ ] **Step 4：回归测试**

```bash
cd /home/share/samba/lvgl/lv_port_pc_vscode_v3-music && bin/test_panels_geom
```

期望：`test_panels_geom: PASS`

- [ ] **Step 5：提交**

```bash
cd /home/share/samba/lvgl/lv_port_pc_vscode_v3-music
git add main/icon_replace_2/icon_replace_2.c
git commit -m "feat(panels): 边缘感应条改为阈值触发展开（>50px 一次性吸附）"
```

---

## Task 3：面板收起手势改为阈值触发

**Files:**
- Modify: `main/icon_replace_2/icon_replace_2_panels.c:88-115`

- [ ] **Step 1：替换三个面板手势回调**

将 `icon_replace_2_panels.c` 中的 `panel_pressed_cb`、`panel_pressing_cb`、`panel_released_cb` 整体替换为：

```c
static void panel_pressed_cb(lv_event_t * e){
    icon_replace_2_panels_t * p = lv_event_get_user_data(e);
    int which = which_of_panel(p, lv_event_get_target(e));
    if(which == 0 || p->active != which) return;
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    p->handle_press_y = pt.y;
}
static void panel_pressing_cb(lv_event_t * e){
    icon_replace_2_panels_t * p = lv_event_get_user_data(e);
    int which = which_of_panel(p, lv_event_get_target(e));
    if(which == 0 || p->active != which) return;
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    int32_t dy = pt.y - p->handle_press_y;
    int32_t close_amount = (which == IR2_PANEL_CONTROL) ? -dy : dy;
    if(close_amount > 50) {
        const ir2_metrics_t * m = ir2_metrics();
        int32_t H = panel_h_of(p, which);
        slide_to(obj_of(p, which), closed_y_of(which, H, m->screen_h), true);
        p->active = 0;
    }
}
static void panel_released_cb(lv_event_t * e){
    LV_UNUSED(e);
}
```

注意：`pressed_cb` 改用 `p->active != which` 作门控（取代原来的 `dragging` 字段），确保只有已展开的面板才记录起点。

- [ ] **Step 2：编译验证**

```bash
cmake --build /home/share/samba/lvgl/lv_port_pc_vscode_v3-music/build --target main -j4
```

期望：无编译错误。此时 `dragging`、`cur_reveal`、`last_delta` 字段虽仍在 struct 中，但已无路径读写（编译器可能产生 unused field 提示，属正常，Task 4 清理）

- [ ] **Step 3：回归测试**

```bash
cd /home/share/samba/lvgl/lv_port_pc_vscode_v3-music && bin/test_panels_geom
```

期望：`test_panels_geom: PASS`

- [ ] **Step 4：提交**

```bash
cd /home/share/samba/lvgl/lv_port_pc_vscode_v3-music
git add main/icon_replace_2/icon_replace_2_panels.c
git commit -m "feat(panels): 面板收起手势改为阈值触发（>50px 一次性吸附）"
```

---

## Task 4：删除死代码，简化 struct 和公开 API

**Files:**
- Modify: `main/icon_replace_2/icon_replace_2_panels.c`
- Modify: `main/icon_replace_2/icon_replace_2_panels.h`

- [ ] **Step 1：删除 panels.c 中的死代码函数**

删除以下函数（Task 1-3 完成后已无任何调用路径）：
- `apply_reveal`（第 48-52 行）
- `snap_release`（第 55-64 行）
- `drag_begin_internal`（第 67-71 行）
- `drag_update_internal`（第 72-76 行）
- 公开函数：`ir2_panels_drag_begin`、`ir2_panels_drag_update`、`ir2_panels_drag_end`（文件末尾）

- [ ] **Step 2：简化 struct 定义**

将 `icon_replace_2_panels.c` 顶部的 struct 从：

```c
struct icon_replace_2_panels {
    lv_obj_t * control;
    lv_obj_t * notify;
    int32_t    h_control;
    int32_t    h_notify;
    int        active;
    int        dragging;
    int32_t    cur_reveal;
    int32_t    last_delta;
    int32_t    handle_press_y;
};
```

改为：

```c
struct icon_replace_2_panels {
    lv_obj_t * control;
    lv_obj_t * notify;
    int32_t    h_control;
    int32_t    h_notify;
    int        active;
    int32_t    handle_press_y;
};
```

- [ ] **Step 3：删除 panels.h 中的 drag API 声明**

在 `icon_replace_2_panels.h` 中，删除以下三行及其注释块：

```c
void ir2_panels_drag_begin(icon_replace_2_panels_t * p, int which);
void ir2_panels_drag_update(icon_replace_2_panels_t * p, int which, int32_t reveal);
void ir2_panels_drag_end(icon_replace_2_panels_t * p, int which);
```

（注释块「跟手拖拽（由屏幕边缘感应条驱动）」一并删除）

- [ ] **Step 4：编译验证**

```bash
cmake --build /home/share/samba/lvgl/lv_port_pc_vscode_v3-music/build --target main -j4
```

期望：零错误，零警告

- [ ] **Step 5：全量回归测试**

```bash
cd /home/share/samba/lvgl/lv_port_pc_vscode_v3-music
bin/test_panels_geom && bin/test_page_config
```

期望：两个测试均输出 PASS

- [ ] **Step 6：截图验证面板展开/收起静态状态**

```bash
cd /home/share/samba/lvgl/lv_port_pc_vscode_v3-music
scripts/shot.sh 800x480 1 control /tmp/final_control.png
scripts/shot.sh 800x480 1 notify  /tmp/final_notify.png
scripts/shot.sh 800x480 1 none    /tmp/final_desktop.png
```

期望：control/notify 各显示全屏面板，none 显示桌面无面板

- [ ] **Step 7：提交**

```bash
cd /home/share/samba/lvgl/lv_port_pc_vscode_v3-music
git add main/icon_replace_2/icon_replace_2_panels.c \
        main/icon_replace_2/icon_replace_2_panels.h
git commit -m "refactor(panels): 删除实时跟手状态机，简化 struct 和公开 API"
```
