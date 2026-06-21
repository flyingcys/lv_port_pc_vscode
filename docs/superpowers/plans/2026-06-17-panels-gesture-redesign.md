# 控制中心 / 通知中心面板交互重做 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把控制中心/通知中心改为「跟手拖拽 + 松手吸附 + 内容自适应高度 + 把手反向拖拽收起」，结构上消除「卡在中间」，并消除空暗区。

**Architecture:** 把可单测的几何/吸附数学抽到 lvgl-free 的纯函数模块（`icon_replace_2_panels_geom`，TDD + ctest）；面板的内容自适应高度、把手、跟手/吸附动画内聚在 `icon_replace_2_panels.c`；`icon_replace_2.c` 的边缘感应条改为转发 reveal 给 panels，并删除旧的 `LV_EVENT_GESTURE` 收起逻辑。

**Tech Stack:** C99 / LVGL v9 / SDL 模拟器 / CMake + ctest / 无头快照（`scripts/shot.sh`）。

**Spec:** `docs/superpowers/specs/2026-06-17-panels-gesture-redesign-design.md`

---

## 文件结构

| 文件 | 动作 | 职责 |
|---|---|---|
| `main/icon_replace_2/icon_replace_2_panels_geom.h` | 新建 | 纯几何/吸附数学声明（lvgl-free，仅 stdint/stdbool） |
| `main/icon_replace_2/icon_replace_2_panels_geom.c` | 新建 | `ir2_panel_drag_y` / `ir2_panel_snap_open` 实现 |
| `tests/icon_replace_2/test_panels_geom.c` | 新建 | 几何/吸附单测（standalone main，返回 0/1） |
| `CMakeLists.txt` | 改 | 注册 `test_panels_geom` 测试目标（geom.c 自动被 main 的 GLOB 收录） |
| `main/icon_replace_2/icon_replace_2_widgets.h/.c` | 改 | 新增 `ir2_widget_panel_handle` 把手控件工厂 |
| `main/icon_replace_2/icon_replace_2_panels.h/.c` | 改 | 内容自适应高度 + 实测 H + 把手收起拖拽 + 展开拖拽 API + 吸附动画 |
| `main/icon_replace_2/icon_replace_2.c` | 改 | 边缘条改为转发 reveal；删除 `panel_gesture_cb` 及 GESTURE 注册 |

> 注：`main/icon_replace_2/*.c` 已被 `CMakeLists.txt` 的 `file(GLOB CONFIGURE_DEPENDS ...)` 收录进 `main`，新增 `geom.c` 重新构建即自动纳入。仅新增**测试**目标需要改 CMake。

---

## Task 1: 纯几何/吸附数学模块 + 单测（TDD）

**Files:**
- Create: `main/icon_replace_2/icon_replace_2_panels_geom.h`
- Create: `main/icon_replace_2/icon_replace_2_panels_geom.c`
- Test: `tests/icon_replace_2/test_panels_geom.c`
- Modify: `CMakeLists.txt`（在 `add_test(NAME test_page_config ...)` 行后插入新测试目标）

- [ ] **Step 1: 写头文件（声明 + 常量）**

`main/icon_replace_2/icon_replace_2_panels_geom.h`:

```c
#ifndef ICON_REPLACE_2_PANELS_GEOM_H
#define ICON_REPLACE_2_PANELS_GEOM_H
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#define IR2_PANEL_CONTROL 1
#define IR2_PANEL_NOTIFY  2

/* 末段移动 >= 此值视为甩动，按方向兜底吸附（露出增量像素） */
#define IR2_PANEL_FLICK_PX 12

/* 由「从全关起算的露出像素 reveal」算出面板 y（已钳制到 [全关, 全开]）。
 *   control: 全关 y=-panel_h, 全开 y=0                  （reveal 下滑为正）
 *   notify : 全关 y=screen_h, 全开 y=screen_h-panel_h   （reveal 上滑为正）
 */
int32_t ir2_panel_drag_y(int which, int32_t reveal, int32_t panel_h, int32_t screen_h);

/* 松手吸附：返回 true=吸附到全开。
 *   末段甩动 |last_delta|>=IR2_PANEL_FLICK_PX 按方向兜底（last_delta 朝开为正）；
 *   否则按露出比例：reveal >= 50% panel_h 则开。panel_h<=0 视为关。
 */
bool ir2_panel_snap_open(int32_t reveal, int32_t panel_h, int32_t last_delta);

#ifdef __cplusplus
}
#endif
#endif
```

- [ ] **Step 2: 写实现桩（先返回定值，让测试能编译并 FAIL）**

`main/icon_replace_2/icon_replace_2_panels_geom.c`:

```c
#include "icon_replace_2_panels_geom.h"

int32_t ir2_panel_drag_y(int which, int32_t reveal, int32_t panel_h, int32_t screen_h)
{
    (void)which; (void)reveal; (void)panel_h; (void)screen_h;
    return 0;   /* 桩：故意错误 */
}

bool ir2_panel_snap_open(int32_t reveal, int32_t panel_h, int32_t last_delta)
{
    (void)reveal; (void)panel_h; (void)last_delta;
    return false;   /* 桩：故意错误 */
}
```

- [ ] **Step 3: 写测试**

`tests/icon_replace_2/test_panels_geom.c`:

```c
#include "icon_replace_2_panels_geom.h"
#include <stdio.h>
#include <stdbool.h>

static int report_check(bool cond, const char * msg)
{
    if(cond) return 0;
    fprintf(stderr, "test_panels_geom: %s\n", msg);
    return 1;
}

int main(void)
{
    int failures = 0;
    const int32_t H = 300, SH = 480;

    /* drag_y: control 端点与中段 */
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_CONTROL, 0,    H, SH) == -H,        "control closed y=-H");
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_CONTROL, H,    H, SH) == 0,         "control open y=0");
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_CONTROL, H/2,  H, SH) == -H + H/2,  "control mid y");
    /* drag_y: control 越界钳制 */
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_CONTROL, -50,  H, SH) == -H,        "control reveal<0 clamps closed");
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_CONTROL, H+50, H, SH) == 0,         "control reveal>H clamps open");

    /* drag_y: notify 端点与越界 */
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_NOTIFY, 0,    H, SH) == SH,         "notify closed y=SH");
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_NOTIFY, H,    H, SH) == SH - H,     "notify open y=SH-H");
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_NOTIFY, H+50, H, SH) == SH - H,     "notify reveal>H clamps open");

    /* snap: 位置阈值 50% */
    failures += report_check(ir2_panel_snap_open(H/2,     H, 0) == true,  "reveal=50% -> open");
    failures += report_check(ir2_panel_snap_open(H/2 - 1, H, 0) == false, "reveal<50% -> close");
    failures += report_check(ir2_panel_snap_open(0,       H, 0) == false, "reveal=0 -> close");

    /* snap: 甩动方向兜底（忽略位置） */
    failures += report_check(ir2_panel_snap_open(10,    H,  IR2_PANEL_FLICK_PX) == true,  "flick-open overrides position");
    failures += report_check(ir2_panel_snap_open(H-10,  H, -IR2_PANEL_FLICK_PX) == false, "flick-close overrides position");

    /* 退化保护 */
    failures += report_check(ir2_panel_snap_open(0, 0, 0) == false, "panel_h=0 -> close");

    if(failures != 0) {
        fprintf(stderr, "test_panels_geom: FAIL (%d checks failed)\n", failures);
        return 1;
    }
    printf("test_panels_geom: PASS\n");
    return 0;
}
```

- [ ] **Step 4: CMake 注册测试目标**

在 `CMakeLists.txt` 中 `add_test(NAME test_page_config COMMAND $<TARGET_FILE:test_page_config>)` 这一行**之后**插入：

```cmake
add_executable(test_panels_geom
    ${PROJECT_SOURCE_DIR}/tests/icon_replace_2/test_panels_geom.c
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2/icon_replace_2_panels_geom.c
)
target_include_directories(test_panels_geom PRIVATE
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2
)
add_test(NAME test_panels_geom COMMAND $<TARGET_FILE:test_panels_geom>)
```

- [ ] **Step 5: 配置 + 构建测试 + 运行，确认 FAIL**

Run:
```bash
cmake -S . -B build
cmake --build build --target test_panels_geom
ctest --test-dir build -R test_panels_geom --output-on-failure
```
Expected: 构建成功；ctest **FAIL**（桩返回定值，断言不通过，stderr 出现 `test_panels_geom: FAIL`）。

- [ ] **Step 6: 写真实实现**

把 `main/icon_replace_2/icon_replace_2_panels_geom.c` 整体替换为：

```c
#include "icon_replace_2_panels_geom.h"

int32_t ir2_panel_drag_y(int which, int32_t reveal, int32_t panel_h, int32_t screen_h)
{
    if(reveal < 0)        reveal = 0;
    if(reveal > panel_h)  reveal = panel_h;
    if(which == IR2_PANEL_CONTROL) {
        return -panel_h + reveal;       /* [-panel_h, 0] */
    }
    return screen_h - reveal;           /* [screen_h - panel_h, screen_h]，notify */
}

bool ir2_panel_snap_open(int32_t reveal, int32_t panel_h, int32_t last_delta)
{
    if(last_delta >= IR2_PANEL_FLICK_PX)  return true;
    if(last_delta <= -IR2_PANEL_FLICK_PX) return false;
    if(panel_h <= 0)                      return false;
    return reveal * 2 >= panel_h;        /* reveal/panel_h >= 0.5 */
}
```

- [ ] **Step 7: 构建 + 运行测试，确认 PASS**

Run:
```bash
cmake --build build --target test_panels_geom
ctest --test-dir build -R test_panels_geom --output-on-failure
```
Expected: **PASS**（`test_panels_geom: PASS`，ctest 1/1 通过）。

- [ ] **Step 8: 提交**

```bash
git add main/icon_replace_2/icon_replace_2_panels_geom.h main/icon_replace_2/icon_replace_2_panels_geom.c tests/icon_replace_2/test_panels_geom.c CMakeLists.txt
git commit -m "feat(panels): 面板跟手/吸附纯几何模块 + 单测(lvgl-free)"
```

---

## Task 2: 把手控件工厂

**Files:**
- Modify: `main/icon_replace_2/icon_replace_2_widgets.h`
- Modify: `main/icon_replace_2/icon_replace_2_widgets.c`

- [ ] **Step 1: 在 widgets.h 声明把手工厂**

在 `icon_replace_2_widgets.h` 里，已有控件声明附近（如 `ir2_widget_dots` 声明之后）加入：

```c
/* 面板把手：满宽透明命中条（承载拖拽收起）+ 居中可见小药丸。返回命中条对象。 */
lv_obj_t * ir2_widget_panel_handle(lv_obj_t * parent);
```

- [ ] **Step 2: 在 widgets.c 实现把手工厂**

在 `icon_replace_2_widgets.c` 末尾（最后一个函数之后）加入：

```c
lv_obj_t * ir2_widget_panel_handle(lv_obj_t * parent){
    const ir2_metrics_t * m = ir2_metrics();
    const ir2_theme_t * th = ir2_theme();
    int32_t strip_h = (m->screen_h <= 272) ? 20 : 30;
    int32_t pill_w  = (m->screen_h <= 272) ? 40 : 60;
    int32_t pill_h  = (m->screen_h <= 272) ? 4  : 6;

    /* 命中条：满宽固定高，可点击不可滚动（拖拽收起回调挂这里） */
    lv_obj_t * strip = lv_obj_create(parent);
    lv_obj_remove_style_all(strip);
    lv_obj_set_size(strip, LV_PCT(100), strip_h);
    lv_obj_set_style_bg_opa(strip, LV_OPA_TRANSP, 0);
    lv_obj_add_flag(strip, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(strip, LV_OBJ_FLAG_SCROLLABLE);

    /* 可见小药丸：居中 */
    lv_obj_t * pill = lv_obj_create(strip);
    lv_obj_remove_style_all(pill);
    lv_obj_set_size(pill, pill_w, pill_h);
    lv_obj_set_style_radius(pill, pill_h / 2, 0);
    lv_obj_set_style_bg_color(pill, th->text_primary, 0);
    lv_obj_set_style_bg_opa(pill, 76, 0);   /* ~rgba(255,255,255,0.3) */
    lv_obj_center(pill);
    ir2_make_decorative(pill);
    return strip;
}
```

> 说明：`ir2_widget_glass_panel` 已经把高度参数透传给 `lv_obj_set_size`，传 `LV_SIZE_CONTENT` 即可内容自适应，无需改它。

- [ ] **Step 3: 构建，确认通过**

Run: `cmake --build build --target main`
Expected: 编译链接成功，无新增报错。

- [ ] **Step 4: 提交**

```bash
git add main/icon_replace_2/icon_replace_2_widgets.h main/icon_replace_2/icon_replace_2_widgets.c
git commit -m "feat(panels): 新增面板把手控件工厂(命中条+小药丸)"
```

---

## Task 3: 面板内容自适应 + 实测 H + 把手收起 + 展开拖拽 API + 吸附

**Files:**
- Modify: `main/icon_replace_2/icon_replace_2_panels.h`（整体替换）
- Modify: `main/icon_replace_2/icon_replace_2_panels.c`（整体替换）

- [ ] **Step 1: 整体替换 panels.h**

把 `main/icon_replace_2/icon_replace_2_panels.h` 整体替换为：

```c
#ifndef ICON_REPLACE_2_PANELS_H
#define ICON_REPLACE_2_PANELS_H
#include "lvgl.h"
#include "icon_replace_2_panels_geom.h"  /* IR2_PANEL_CONTROL / IR2_PANEL_NOTIFY 供调用方使用 */
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct icon_replace_2_panels icon_replace_2_panels_t;
icon_replace_2_panels_t * ir2_panels_create(lv_obj_t * parent);
void ir2_panels_destroy(icon_replace_2_panels_t * p);

/* 直接吸附到全开/全关（带动画）。截图钩子/程序化调用用。 */
void ir2_panels_show_control(icon_replace_2_panels_t * p, bool show);
void ir2_panels_show_notify(icon_replace_2_panels_t * p, bool show);

/* 截图钩子按 AM_PANEL 设初始全开态（无动画）："control" / "notify" / 其它=none。 */
void ir2_panels_apply_initial(icon_replace_2_panels_t * p, const char * which);

/* 跟手拖拽（由屏幕边缘感应条驱动）。
 *   which: IR2_PANEL_CONTROL / IR2_PANEL_NOTIFY
 *   reveal: 从全关起算的露出像素（control 下滑为正、notify 上滑为正）
 *   begin 从全关起拖；update 实时跟手；end 松手按位置/甩动吸附。
 */
void ir2_panels_drag_begin(icon_replace_2_panels_t * p, int which);
void ir2_panels_drag_update(icon_replace_2_panels_t * p, int which, int32_t reveal);
void ir2_panels_drag_end(icon_replace_2_panels_t * p, int which);

/* 当前展开态：0=无 1=控制中心 2=通知中心。供边缘条门控（展开时忽略边缘按下）。 */
int  ir2_panels_active(icon_replace_2_panels_t * p);

#ifdef __cplusplus
}
#endif
#endif
```

- [ ] **Step 2: 整体替换 panels.c**

把 `main/icon_replace_2/icon_replace_2_panels.c` 整体替换为：

```c
#include "icon_replace_2_panels.h"
#include "icon_replace_2_panels_geom.h"
#include "icon_replace_2_widgets.h"
#include "icon_replace_2_metrics.h"
#include "icon_replace_2_theme.h"
#include "icon_replace_2_glyphs.h"
#include "icon_replace_2_data.h"
#include <stdlib.h>
#include <string.h>

struct icon_replace_2_panels {
    lv_obj_t * control;
    lv_obj_t * notify;
    int32_t    h_control;     /* 实测内容高 */
    int32_t    h_notify;
    int        active;        /* 0=无 1=控制中心 2=通知中心 */
    int        dragging;      /* 当前拖拽的 which；0=无 */
    int32_t    cur_reveal;    /* 跟手过程当前 reveal */
    int32_t    last_delta;    /* 末段露出增量（朝开为正），供甩动判定 */
    int32_t    handle_press_y;/* 把手收起拖拽起点(屏幕Y) */
};

/* ---- 几何辅助 ---- */
static int32_t panel_h_of(icon_replace_2_panels_t * p, int which){
    return (which == IR2_PANEL_CONTROL) ? p->h_control : p->h_notify;
}
static lv_obj_t * obj_of(icon_replace_2_panels_t * p, int which){
    return (which == IR2_PANEL_CONTROL) ? p->control : p->notify;
}
static int32_t closed_y_of(int which, int32_t H, int32_t screen_h){
    return (which == IR2_PANEL_CONTROL) ? -H : screen_h;
}
static int32_t open_y_of(int which, int32_t H, int32_t screen_h){
    return (which == IR2_PANEL_CONTROL) ? 0 : (screen_h - H);
}

static void slide_to(lv_obj_t * o, int32_t y, bool anim){
    if(anim){
        lv_anim_t a; lv_anim_init(&a); lv_anim_set_var(&a,o);
        lv_anim_set_exec_cb(&a,(lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_time(&a,250); lv_anim_set_values(&a, lv_obj_get_y(o), y); lv_anim_start(&a);
    } else {
        lv_anim_del(o, (lv_anim_exec_xcb_t)lv_obj_set_y);   /* 跟手时清掉残留动画，防打架 */
        lv_obj_set_y(o, y);
    }
}

/* 跟手：按 reveal 直接定位（无动画） */
static void apply_reveal(icon_replace_2_panels_t * p, int which, int32_t reveal){
    const ir2_metrics_t * m = ir2_metrics();
    int32_t H = panel_h_of(p, which);
    slide_to(obj_of(p, which), ir2_panel_drag_y(which, reveal, H, m->screen_h), false);
}

/* 松手吸附 */
static void snap_release(icon_replace_2_panels_t * p, int which){
    const ir2_metrics_t * m = ir2_metrics();
    int32_t H = panel_h_of(p, which);
    bool open = ir2_panel_snap_open(p->cur_reveal, H, p->last_delta);
    slide_to(obj_of(p, which),
             open ? open_y_of(which, H, m->screen_h) : closed_y_of(which, H, m->screen_h),
             true);
    p->active = open ? which : 0;
    p->dragging = 0;
}

/* ---- 内部拖拽状态机（展开/收起共用） ---- */
static void drag_begin_internal(icon_replace_2_panels_t * p, int which, int32_t reveal0){
    p->dragging   = which;
    p->cur_reveal = reveal0;
    p->last_delta = 0;
}
static void drag_update_internal(icon_replace_2_panels_t * p, int which, int32_t reveal){
    p->last_delta = reveal - p->cur_reveal;   /* 朝开为正 */
    p->cur_reveal = reveal;
    apply_reveal(p, which, reveal);
}

/* ---- 把手收起拖拽回调（内聚于 panels） ---- */
static int which_of_handle(icon_replace_2_panels_t * p, lv_obj_t * handle){
    lv_obj_t * parent = lv_obj_get_parent(handle);
    if(parent == p->control) return IR2_PANEL_CONTROL;
    if(parent == p->notify)  return IR2_PANEL_NOTIFY;
    return 0;
}
static void handle_pressed_cb(lv_event_t * e){
    icon_replace_2_panels_t * p = lv_event_get_user_data(e);
    lv_obj_t * handle = lv_event_get_target(e);
    int which = which_of_handle(p, handle);
    if(which == 0) return;
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    p->handle_press_y = pt.y;
    drag_begin_internal(p, which, panel_h_of(p, which));   /* 起点=全开 */
}
static void handle_pressing_cb(lv_event_t * e){
    icon_replace_2_panels_t * p = lv_event_get_user_data(e);
    lv_obj_t * handle = lv_event_get_target(e);
    int which = which_of_handle(p, handle);
    if(which == 0 || p->dragging != which) return;
    int32_t H = panel_h_of(p, which);
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    int32_t dy = pt.y - p->handle_press_y;
    /* 收起方向：control 向上(dy<0)收起；notify 向下(dy>0)收起 */
    int32_t close_amount = (which == IR2_PANEL_CONTROL) ? -dy : dy;
    if(close_amount < 0) close_amount = 0;
    int32_t reveal = H - close_amount;
    if(reveal < 0) reveal = 0;
    drag_update_internal(p, which, reveal);
}
static void handle_released_cb(lv_event_t * e){
    icon_replace_2_panels_t * p = lv_event_get_user_data(e);
    lv_obj_t * handle = lv_event_get_target(e);
    int which = which_of_handle(p, handle);
    if(which == 0 || p->dragging != which) return;
    snap_release(p, which);
}

static lv_obj_t * make_title(lv_obj_t * parent, const char * txt){
    const ir2_metrics_t * m = ir2_metrics();
    lv_obj_t * t = lv_label_create(parent);
    lv_obj_set_style_text_font(t, m->font_label, 0);
    lv_obj_set_style_text_color(t, ir2_theme()->text_primary, 0);
    lv_label_set_text(t, txt); ir2_make_decorative(t);
    return t;
}

/* 给把手挂收起拖拽回调 */
static void attach_handle(icon_replace_2_panels_t * p, lv_obj_t * handle){
    lv_obj_add_event_cb(handle, handle_pressed_cb,  LV_EVENT_PRESSED,  p);
    lv_obj_add_event_cb(handle, handle_pressing_cb, LV_EVENT_PRESSING, p);
    lv_obj_add_event_cb(handle, handle_released_cb, LV_EVENT_RELEASED, p);
}

icon_replace_2_panels_t * ir2_panels_create(lv_obj_t * parent){
    const ir2_metrics_t * m = ir2_metrics();
    icon_replace_2_panels_t * p = lv_malloc_zeroed(sizeof(*p));
    if(!p) return NULL;

    /* ---------- 控制中心（顶部下滑，内容自适应高度） ---------- */
    p->control = ir2_widget_glass_panel(parent, m->screen_w, LV_SIZE_CONTENT);
    lv_obj_set_style_max_height(p->control, m->screen_h, 0);
    lv_obj_set_flex_flow(p->control, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(p->control, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(p->control, 16, 0);
    lv_obj_set_style_pad_row(p->control, 12, 0);

    make_title(p->control, "\xe6\x8e\xa7\xe5\x88\xb6\xe4\xb8\xad\xe5\xbf\x83");  /* 控制中心 */

    lv_obj_t * row = lv_obj_create(p->control); lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 16, 0); ir2_make_decorative(row);
    ir2_widget_toggle(row, IR2_GLYPH_WIFI, true);
    ir2_widget_toggle(row, IR2_GLYPH_BLUETOOTH, false);
    ir2_widget_toggle(row, IR2_GLYPH_AIRPLANE, true);
    ir2_widget_toggle(row, IR2_GLYPH_MOON, false);

    ir2_widget_slider(p->control, IR2_GLYPH_SUN, 70);
    ir2_widget_slider(p->control, IR2_GLYPH_SPEAKER, 45);

    /* 控制中心把手在底部（内容之后） */
    lv_obj_t * ctrl_handle = ir2_widget_panel_handle(p->control);
    attach_handle(p, ctrl_handle);

    /* ---------- 通知中心（底部上滑，内容自适应高度） ---------- */
    p->notify = ir2_widget_glass_panel(parent, m->screen_w, LV_SIZE_CONTENT);
    lv_obj_set_style_max_height(p->notify, m->screen_h, 0);
    lv_obj_set_flex_flow(p->notify, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(p->notify, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(p->notify, 16, 0);
    lv_obj_set_style_pad_row(p->notify, 10, 0);

    /* 通知中心把手在顶部（内容之前） */
    lv_obj_t * ntfy_handle = ir2_widget_panel_handle(p->notify);
    attach_handle(p, ntfy_handle);

    make_title(p->notify, "\xe9\x80\x9a\xe7\x9f\xa5\xe4\xb8\xad\xe5\xbf\x83");  /* 通知中心 */

    for(uint32_t i = 0; i < ir2_notify_count; i++) {
        const ir2_notify_t * n = &ir2_notifies[i];
        const ir2_metrics_t * mm = ir2_metrics();
        const ir2_theme_t * th = ir2_theme();
        lv_obj_t * card = lv_obj_create(p->notify);
        lv_obj_remove_style_all(card);
        lv_obj_set_size(card, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_color(card, th->glass_hi, 0);
        lv_obj_set_style_bg_opa(card, th->glass_hi_opa, 0);
        lv_obj_set_style_radius(card, 14, 0);
        lv_obj_set_style_pad_all(card, 10, 0);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(card, 4, 0);
        ir2_make_decorative(card);

        lv_obj_t * title = lv_label_create(card);
        lv_obj_set_style_text_font(title, mm->font_label, 0);
        lv_obj_set_style_text_color(title, th->text_primary, 0);
        lv_label_set_text(title, n->title);
        ir2_make_decorative(title);

        lv_obj_t * body = lv_label_create(card);
        lv_obj_set_style_text_font(body, mm->font_label, 0);
        lv_obj_set_style_text_color(body, th->text_primary, 0);
        lv_obj_set_style_text_opa(body, 204, 0);
        lv_obj_set_width(body, LV_PCT(100));
        lv_label_set_long_mode(body, LV_LABEL_LONG_MODE_WRAP);
        lv_label_set_text(body, n->body);
        ir2_make_decorative(body);

        lv_obj_t * tm = lv_label_create(card);
        lv_obj_set_style_text_font(tm, mm->font_label, 0);
        lv_obj_set_style_text_color(tm, th->text_primary, 0);
        lv_obj_set_style_text_opa(tm, 128, 0);
        lv_label_set_text(tm, n->time);
        ir2_make_decorative(tm);
    }

    /* ---------- 实测内容高，置初始全关位 ---------- */
    lv_obj_update_layout(p->control);
    lv_obj_update_layout(p->notify);
    p->h_control = lv_obj_get_height(p->control);
    p->h_notify  = lv_obj_get_height(p->notify);
    lv_obj_set_y(p->control, closed_y_of(IR2_PANEL_CONTROL, p->h_control, m->screen_h));
    lv_obj_set_y(p->notify,  closed_y_of(IR2_PANEL_NOTIFY,  p->h_notify,  m->screen_h));
    lv_obj_set_x(p->control, 0);
    lv_obj_set_x(p->notify, 0);

    p->active = 0;
    p->dragging = 0;
    return p;
}

void ir2_panels_destroy(icon_replace_2_panels_t * p){
    if(!p) return;
    if(p->control) lv_obj_delete(p->control);
    if(p->notify) lv_obj_delete(p->notify);
    lv_free(p);
}

void ir2_panels_show_control(icon_replace_2_panels_t * p, bool show){
    const ir2_metrics_t * m = ir2_metrics();
    if(!p || !p->control) return;
    slide_to(p->control,
             show ? open_y_of(IR2_PANEL_CONTROL, p->h_control, m->screen_h)
                  : closed_y_of(IR2_PANEL_CONTROL, p->h_control, m->screen_h), true);
    p->active = show ? IR2_PANEL_CONTROL : 0;
}
void ir2_panels_show_notify(icon_replace_2_panels_t * p, bool show){
    const ir2_metrics_t * m = ir2_metrics();
    if(!p || !p->notify) return;
    slide_to(p->notify,
             show ? open_y_of(IR2_PANEL_NOTIFY, p->h_notify, m->screen_h)
                  : closed_y_of(IR2_PANEL_NOTIFY, p->h_notify, m->screen_h), true);
    p->active = show ? IR2_PANEL_NOTIFY : 0;
}

void ir2_panels_apply_initial(icon_replace_2_panels_t * p, const char * which){
    const ir2_metrics_t * m = ir2_metrics();
    if(!p || !which) return;
    if(strcmp(which, "control") == 0 && p->control) {
        slide_to(p->control, open_y_of(IR2_PANEL_CONTROL, p->h_control, m->screen_h), false);
        p->active = IR2_PANEL_CONTROL;
    } else if(strcmp(which, "notify") == 0 && p->notify) {
        slide_to(p->notify, open_y_of(IR2_PANEL_NOTIFY, p->h_notify, m->screen_h), false);
        p->active = IR2_PANEL_NOTIFY;
    }
}

void ir2_panels_drag_begin(icon_replace_2_panels_t * p, int which){
    if(!p) return;
    drag_begin_internal(p, which, 0);   /* 从全关起拖 */
}
void ir2_panels_drag_update(icon_replace_2_panels_t * p, int which, int32_t reveal){
    if(!p || p->dragging != which) return;
    drag_update_internal(p, which, reveal);
}
void ir2_panels_drag_end(icon_replace_2_panels_t * p, int which){
    if(!p || p->dragging != which) return;
    snap_release(p, which);
}

int ir2_panels_active(icon_replace_2_panels_t * p){
    return p ? p->active : 0;
}
```

- [ ] **Step 3: 构建，确认通过**

Run: `cmake --build build --target main`
Expected: 编译链接成功。若有 `lv_event_get_user_data` / `lv_obj_set_style_max_height` 等符号问题，确认 LVGL v9 头已包含（本仓库 v9，均可用）。

- [ ] **Step 4: 提交**

```bash
git add main/icon_replace_2/icon_replace_2_panels.h main/icon_replace_2/icon_replace_2_panels.c
git commit -m "feat(panels): 内容自适应高度+实测H+把手收起拖拽+展开拖拽API+松手吸附"
```

---

## Task 4: 边缘感应条改为转发 reveal；删除旧 GESTURE 收起

**Files:**
- Modify: `main/icon_replace_2/icon_replace_2.c`

- [ ] **Step 1: 删除旧 `edge_panel_open` 静态、新增拖拽标志**

把：
```c
/* 避免重复触发：0=未展开, 1=控制中心, 2=通知中心 */
static int        edge_panel_open;
```
替换为：
```c
/* 边缘条跟手拖拽进行中标志（避免无 begin 的 pressing/released 误触发） */
static int        edge_top_dragging;
static int        edge_bot_dragging;
```

- [ ] **Step 2: 更新前置声明**

把这两行：
```c
static void edge_bot_pressed_cb(lv_event_t * e);
static void edge_bot_pressing_cb(lv_event_t * e);
/* 面板收起手势 */
static void panel_gesture_cb(lv_event_t * e);
```
替换为：
```c
static void edge_bot_pressed_cb(lv_event_t * e);
static void edge_bot_pressing_cb(lv_event_t * e);
static void edge_top_released_cb(lv_event_t * e);
static void edge_bot_released_cb(lv_event_t * e);
```
（删除 `panel_gesture_cb` 声明；新增两个 released 声明。`edge_top_pressed_cb`/`edge_top_pressing_cb` 声明保持不变。）

- [ ] **Step 3: 重写边缘条创建块（含事件注册），删除面板 GESTURE 收起块**

把 `icon_replace_2.c` 中从 `/* 顶部感应条 */` 注释开始、到该 `{ ... }` 作用域结束（即包含 `edge_top`、`edge_bot` 创建与事件注册、以及 `if(panels != NULL){ ... ctrl/ntfy 的 GESTURE 注册 ... }` 整段）替换为下面内容。原块对应当前文件约 248–288 行（以 `edge_top = lv_obj_create(...)` 到 `}` 收尾的两段 `lv_obj_add_event_cb(... LV_EVENT_GESTURE ...)` 注册全部移除）：

```c
        /* 顶部感应条 */
        edge_top = lv_obj_create(lv_screen_active());
        lv_obj_remove_style_all(edge_top);
        lv_obj_set_size(edge_top, em->screen_w, EDGE_SENSOR_H);
        lv_obj_set_pos(edge_top, 0, 0);
        lv_obj_set_style_bg_opa(edge_top, LV_OPA_TRANSP, 0);
        lv_obj_add_flag(edge_top, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_flag(edge_top, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(edge_top, LV_OBJ_FLAG_GESTURE_BUBBLE);
        lv_obj_add_event_cb(edge_top, edge_top_pressed_cb,  LV_EVENT_PRESSED,  NULL);
        lv_obj_add_event_cb(edge_top, edge_top_pressing_cb, LV_EVENT_PRESSING, NULL);
        lv_obj_add_event_cb(edge_top, edge_top_released_cb, LV_EVENT_RELEASED, NULL);

        /* 底部感应条 */
        edge_bot = lv_obj_create(lv_screen_active());
        lv_obj_remove_style_all(edge_bot);
        lv_obj_set_size(edge_bot, em->screen_w, EDGE_SENSOR_H);
        lv_obj_set_pos(edge_bot, 0, em->screen_h - EDGE_SENSOR_H);
        lv_obj_set_style_bg_opa(edge_bot, LV_OPA_TRANSP, 0);
        lv_obj_add_flag(edge_bot, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_flag(edge_bot, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(edge_bot, LV_OBJ_FLAG_GESTURE_BUBBLE);
        lv_obj_add_event_cb(edge_bot, edge_bot_pressed_cb,  LV_EVENT_PRESSED,  NULL);
        lv_obj_add_event_cb(edge_bot, edge_bot_pressing_cb, LV_EVENT_PRESSING, NULL);
        lv_obj_add_event_cb(edge_bot, edge_bot_released_cb, LV_EVENT_RELEASED, NULL);
```

> 注意：原来在此块尾部对 `ir2_panels_get_control/get_notify` 返回对象注册 `LV_EVENT_GESTURE`（`panel_gesture_cb`）的整段 `if(panels != NULL){...}` **整体删除**（收起改由 panels 内部把手处理）。

- [ ] **Step 4: 重写边缘条回调函数体**

把现有的 `edge_top_pressed_cb` / `edge_top_pressing_cb` / `edge_bot_pressed_cb` / `edge_bot_pressing_cb` / `panel_gesture_cb` 五个函数（从 `/* 顶部条：PRESSED — 记录起始 Y */` 到文件末尾 `panel_gesture_cb` 结束）整体替换为：

```c
/* 顶部条：PRESSED — 记录起点并开始跟手（仅当无面板展开） */
static void edge_top_pressed_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(ir2_panels_active(panels) != 0) { edge_top_dragging = 0; return; }
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    edge_top_press_y = pt.y;
    edge_top_dragging = 1;
    ir2_panels_drag_begin(panels, IR2_PANEL_CONTROL);
}

/* 顶部条：PRESSING — 转发 reveal（下滑为正） */
static void edge_top_pressing_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(!edge_top_dragging) return;
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    ir2_panels_drag_update(panels, IR2_PANEL_CONTROL, pt.y - edge_top_press_y);
}

/* 顶部条：RELEASED — 松手吸附 */
static void edge_top_released_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(!edge_top_dragging) return;
    edge_top_dragging = 0;
    ir2_panels_drag_end(panels, IR2_PANEL_CONTROL);
}

/* 底部条：PRESSED — 记录起点并开始跟手（仅当无面板展开） */
static void edge_bot_pressed_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(ir2_panels_active(panels) != 0) { edge_bot_dragging = 0; return; }
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    edge_bot_press_y = pt.y;
    edge_bot_dragging = 1;
    ir2_panels_drag_begin(panels, IR2_PANEL_NOTIFY);
}

/* 底部条：PRESSING — 转发 reveal（上滑为正） */
static void edge_bot_pressing_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(!edge_bot_dragging) return;
    lv_point_t pt; lv_indev_get_point(lv_indev_get_act(), &pt);
    ir2_panels_drag_update(panels, IR2_PANEL_NOTIFY, edge_bot_press_y - pt.y);
}

/* 底部条：RELEASED — 松手吸附 */
static void edge_bot_released_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(!edge_bot_dragging) return;
    edge_bot_dragging = 0;
    ir2_panels_drag_end(panels, IR2_PANEL_NOTIFY);
}
```

- [ ] **Step 5: 修正 `clear_runtime_object_refs` 里的状态复位**

把：
```c
    edge_top_press_y = 0;
    edge_bot_press_y = 0;
    edge_panel_open = 0;
```
替换为：
```c
    edge_top_press_y = 0;
    edge_bot_press_y = 0;
    edge_top_dragging = 0;
    edge_bot_dragging = 0;
```

- [ ] **Step 6: 构建，确认通过且无 `panel_gesture_cb` 残留**

Run:
```bash
cmake --build build --target main
grep -n "panel_gesture_cb\|edge_panel_open" main/icon_replace_2/icon_replace_2.c || echo "clean"
```
Expected: 构建成功；grep 输出 `clean`（无残留引用）。

- [ ] **Step 7: 提交**

```bash
git add main/icon_replace_2/icon_replace_2.c
git commit -m "feat(panels): 边缘条改跟手转发reveal+松手吸附；删除旧GESTURE收起"
```

---

## Task 5: 整体验证（单测 + 三档快照 + 交互回归）

**Files:** 无代码改动（仅验证；若发现问题回到对应 Task 修复）。

- [ ] **Step 1: 全量构建 + 全部单测**

Run:
```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
Expected: 全部目标构建成功；`ctest` 全绿（含 `test_page_config`、`test_panels_geom` 及既有 apple_music/fruit_ninja 测试）。

- [ ] **Step 2: 三档 × 面板全开快照**

Run:
```bash
for R in 800x480 640x480 480x272; do
  scripts/shot.sh $R 0 control /tmp/ir2_${R}_control.png
  scripts/shot.sh $R 0 notify  /tmp/ir2_${R}_notify.png
done
ls -la /tmp/ir2_*_control.png /tmp/ir2_*_notify.png
```
Expected: 6 张 PNG 全部生成（`shot.sh` 打印 `wrote ...`）。

- [ ] **Step 3: 肉眼健全性检查（Read 每张 PNG）**

逐张 `Read` 上述 6 张图，确认：
- 面板**内容自适应**：控制中心=标题+一排开关+两条滑条+底部把手；通知中心=顶部把手+标题+2 张卡片；**无大片空暗区**（面板高度贴合内容）。
- 面板处于**全开**态（control 贴顶、notify 贴底），**非中间态**。
- **把手小药丸可见**且居中（control 在面板底缘、notify 在面板顶缘）。
- 三档均不溢出、文字不竖排、卡片贴合。
Expected: 全部满足；否则回 Task 2/3 调整尺寸或 flex 顺序后重测。

- [ ] **Step 4: 页面回归快照（确认桌面/锁屏未受影响）**

Run:
```bash
for P in 0 1 2; do scripts/shot.sh 800x480 $P none /tmp/ir2_p${P}.png; done
```
逐张 `Read`，确认锁屏大时钟、两页图标网格、顶栏、分页圆点均正常（与改动前一致）。
Expected: 桌面三页无回归。

- [ ] **Step 5: 交互手测（人工，SDL 窗口）**

> 无头环境无法验证拖拽手感，此步由人工在带显示环境执行；自动化置信度由 Step1 单测（吸附数学）+ Step3 快照（全开形态）+ 代码审查共同保证。

Run: `./bin/main`（需图形环境）
手测清单：
- 顶部边缘下滑：控制中心**跟手**下拉；松手过半→全开、不过半→回弹全关；**不会停在中间**。
- 底部边缘上滑：通知中心跟手上拉，松手吸附；**不会停在中间**（复现原 bug 场景）。
- 面板把手反向拖拽：control 上拖收起、notify 下拖收起；中途回拖可重新展开。
- 一个面板展开时，另一边缘按下被忽略（不会两个同时开）。
- 桌面左右翻页、图标长按摇晃/拖拽换位/跨页、锁屏，均不受影响。

- [ ] **Step 6: 完成判据复核 + 收尾提交（如有微调）**

对照 spec §7 验证判据逐条确认。若 Step3/Step5 触发了尺寸/圆角/把手微调，合并到对应文件并提交：
```bash
git add -A
git commit -m "fix(panels): 截图回路微调(把手/圆角/高度)"
```
Expected: 所有判据满足；工作区干净。

---

## 自检记录（writing-plans self-review）

- **Spec 覆盖**：跟手展开=Task3/4；松手吸附=Task1(数学)+Task3(snap_release)；内容自适应高度=Task3(LV_SIZE_CONTENT+实测H)；把手+反向拖拽收起=Task2+Task3；删除旧 GESTURE=Task4；截图钩子按实测H=Task3(apply_initial)；三档/回归=Task5。spec 各节均有对应任务。
- **占位符扫描**：无 TBD/TODO；所有代码步骤给出完整代码与命令。
- **类型一致性**：`IR2_PANEL_CONTROL/NOTIFY`、`ir2_panel_drag_y`、`ir2_panel_snap_open`、`ir2_panels_drag_begin/update/end`、`ir2_panels_active`、`ir2_widget_panel_handle` 在声明与调用处签名一致；`which_of_handle` 依赖把手父对象=对应 panel（创建时已保证）。
- **已知近似**：贴屏一侧圆角（统一 radius 24）按 spec §4.3 留作截图回路微调，不阻塞；面板内容超 `max_height` 时不滚动（当前内容不会触顶，YAGNI）。
