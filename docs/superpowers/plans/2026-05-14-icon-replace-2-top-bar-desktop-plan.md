# Icon Replace 2 顶部系统状态层与页级扩展层改造 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在当前 `icon_replace_2` 上实现固定高度顶部条，其中右侧系统状态全局唯一，左侧/中间按页切换，并保持现有桌面分页、长按抖动、拖拽换位、跨页拖拽能力。

**Architecture:** 将当前全屏 `lv_tileview` 拆成 `top_bar + desktop_host` 两层；顶部条分为 `left_slot / center_slot / system_right_slot` 三个槽位，右侧只维护一份时间和 `WiFi` 对象，左侧和中间由页配置驱动。桌面区继续使用 `lv_tileview`，但所有索引、吸附和跨页判断都改为基于 `desktop_host` 的局部坐标。

**Tech Stack:** C99, LVGL 9.3, SDL simulator, CMake

---

## 文件结构

### 需要新增

- `main/icon_replace_2/icon_replace_2_layout.h`
  - 统一定义屏幕、顶部条、桌面区、图标网格相关常量和坐标辅助函数声明
- `main/icon_replace_2/icon_replace_2_page_config.h`
  - 定义页模式、顶部槽位类型、每页顶部配置结构和查询接口
- `main/icon_replace_2/icon_replace_2_page_config.c`
  - 提供各页顶部配置表，约定 `page_0` 为锁屏页
- `main/icon_replace_2/icon_replace_2_top_bar.h`
  - 顶部条模块接口，负责创建顶部条、切页应用配置、更新时间、更新 `WiFi`
- `main/icon_replace_2/icon_replace_2_top_bar.c`
  - 顶部条实现，持有 `left_slot / center_slot / system_right_slot`
- `main/icon_replace_2/icon_replace_2_desktop.h`
  - 桌面模块接口，负责桌面创建、页切换、拖拽逻辑、编辑态联动
- `main/icon_replace_2/icon_replace_2_desktop.c`
  - 从现有 `icon_replace_2.c` 中拆出桌面逻辑和局部坐标换算
- `tests/icon_replace_2/test_page_config.c`
  - 纯 C 测试，验证页配置表和锁屏页约定

### 需要修改

- `CMakeLists.txt`
  - 把新增源文件加入 `ICON_REPLACE_2_SOURCES`
  - 增加 `test_page_config` 可执行目标
- `main/icon_replace_2/icon_replace_2.h`
  - 如有需要，补充新的对外接口声明
- `main/icon_replace_2/icon_replace_2.c`
  - 从“大单文件”降级为组装入口，负责创建根容器、顶部条、桌面区和状态更新入口

## Task 1: 建立布局常量与页配置骨架

**Files:**
- Create: `main/icon_replace_2/icon_replace_2_layout.h`
- Create: `main/icon_replace_2/icon_replace_2_page_config.h`
- Create: `main/icon_replace_2/icon_replace_2_page_config.c`
- Create: `tests/icon_replace_2/test_page_config.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 写页配置测试，先锁定锁屏首页和顶部槽位协议**

```c
/* tests/icon_replace_2/test_page_config.c */
#include "icon_replace_2_page_config.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    const topbar_page_config_t * lock_cfg = icon_replace_2_get_page_config(0);
    const topbar_page_config_t * home_cfg = icon_replace_2_get_page_config(1);

    assert(lock_cfg != NULL);
    assert(home_cfg != NULL);

    assert(lock_cfg->page_mode == TOPBAR_PAGE_MODE_LOCK);
    assert(lock_cfg->show_system_right == true);
    assert(lock_cfg->center_type != TOPBAR_SLOT_NONE);

    assert(home_cfg->page_mode == TOPBAR_PAGE_MODE_HOME);
    assert(home_cfg->show_system_right == true);

    printf("test_page_config: PASS\n");
    return 0;
}
```

- [ ] **Step 2: 先把测试目标接进 CMake，但此时先故意引用未实现文件**

```cmake
# CMakeLists.txt
set(ICON_REPLACE_2_SOURCES
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2/icon_replace_2.c
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2/icon_replace_2_assets_bundle.c
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2/icon_replace_2_page_config.c
)

add_executable(test_page_config
    ${PROJECT_SOURCE_DIR}/tests/icon_replace_2/test_page_config.c
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2/icon_replace_2_page_config.c
)
target_include_directories(test_page_config PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2
)
```

- [ ] **Step 3: 运行测试构建，确认当前因缺少头文件/实现而失败**

Run: `rtk cmake -S . -B build && rtk cmake --build build --target test_page_config -j`

Expected:
- 首次失败
- 报错应集中在 `icon_replace_2_page_config.h` 或未定义的 `icon_replace_2_get_page_config`

- [ ] **Step 4: 写最小页配置头文件和实现**

```c
/* main/icon_replace_2/icon_replace_2_page_config.h */
#ifndef ICON_REPLACE_2_PAGE_CONFIG_H
#define ICON_REPLACE_2_PAGE_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    TOPBAR_PAGE_MODE_LOCK = 0,
    TOPBAR_PAGE_MODE_HOME,
    TOPBAR_PAGE_MODE_CUSTOM,
} topbar_page_mode_t;

typedef enum {
    TOPBAR_SLOT_NONE = 0,
    TOPBAR_SLOT_TEXT,
    TOPBAR_SLOT_CUSTOM_OBJ,
} topbar_slot_type_t;

typedef struct {
    topbar_slot_type_t left_type;
    const char * left_text;
    topbar_slot_type_t center_type;
    const char * center_text;
    topbar_page_mode_t page_mode;
    bool show_system_right;
} topbar_page_config_t;

const topbar_page_config_t * icon_replace_2_get_page_config(uint32_t page_index);

#endif
```

```c
/* main/icon_replace_2/icon_replace_2_page_config.c */
#include "icon_replace_2_page_config.h"

static const topbar_page_config_t g_page_configs[] = {
    {
        .left_type = TOPBAR_SLOT_NONE,
        .left_text = NULL,
        .center_type = TOPBAR_SLOT_TEXT,
        .center_text = "09:41",
        .page_mode = TOPBAR_PAGE_MODE_LOCK,
        .show_system_right = true,
    },
    {
        .left_type = TOPBAR_SLOT_NONE,
        .left_text = NULL,
        .center_type = TOPBAR_SLOT_NONE,
        .center_text = NULL,
        .page_mode = TOPBAR_PAGE_MODE_HOME,
        .show_system_right = true,
    },
    {
        .left_type = TOPBAR_SLOT_TEXT,
        .left_text = "自定义页",
        .center_type = TOPBAR_SLOT_TEXT,
        .center_text = "更多内容",
        .page_mode = TOPBAR_PAGE_MODE_CUSTOM,
        .show_system_right = true,
    },
};

const topbar_page_config_t * icon_replace_2_get_page_config(uint32_t page_index)
{
    if(page_index >= (sizeof(g_page_configs) / sizeof(g_page_configs[0]))) {
        return &g_page_configs[1];
    }

    return &g_page_configs[page_index];
}
```

```c
/* main/icon_replace_2/icon_replace_2_layout.h */
#ifndef ICON_REPLACE_2_LAYOUT_H
#define ICON_REPLACE_2_LAYOUT_H

#define SCREEN_W 800
#define SCREEN_H 480
#define TOP_BAR_H 40
#define DESKTOP_W SCREEN_W
#define DESKTOP_H (SCREEN_H - TOP_BAR_H)

#define ICON_START_X 90
#define ICON_START_Y 20
#define ICON_MAX_ROW 3
#define ICON_MAX_COL 5
#define ICON_X_DISTANCE 140
#define ICON_Y_DISTANCE 140
#define ICON_SIZE 60
#define PAGE_COUNT 3
#define ICON_SLOT_COUNT (ICON_MAX_ROW * ICON_MAX_COL)

#endif
```

- [ ] **Step 5: 重跑页配置测试，确认最小骨架通过**

Run: `rtk cmake --build build --target test_page_config -j && rtk ./bin/test_page_config`

Expected:
- 编译成功
- 输出 `test_page_config: PASS`

- [ ] **Step 6: 提交本任务**

```bash
rtk git add CMakeLists.txt \
  main/icon_replace_2/icon_replace_2_layout.h \
  main/icon_replace_2/icon_replace_2_page_config.h \
  main/icon_replace_2/icon_replace_2_page_config.c \
  tests/icon_replace_2/test_page_config.c
rtk git commit -m "feat: add top bar page config scaffold"
```

## Task 2: 落地顶部条模块和系统状态接口

**Files:**
- Create: `main/icon_replace_2/icon_replace_2_top_bar.h`
- Create: `main/icon_replace_2/icon_replace_2_top_bar.c`
- Modify: `CMakeLists.txt`
- Modify: `main/icon_replace_2/icon_replace_2.c`

- [ ] **Step 1: 先把顶部条模块头文件接口写出来**

```c
/* main/icon_replace_2/icon_replace_2_top_bar.h */
#ifndef ICON_REPLACE_2_TOP_BAR_H
#define ICON_REPLACE_2_TOP_BAR_H

#include "lvgl.h"
#include "icon_replace_2_page_config.h"

typedef enum {
    WIFI_STATE_OFF = 0,
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_WEAK,
    WIFI_STATE_NORMAL,
    WIFI_STATE_STRONG,
} wifi_state_t;

typedef struct {
    lv_obj_t * root;
    lv_obj_t * left_slot;
    lv_obj_t * center_slot;
    lv_obj_t * system_right_slot;
    lv_obj_t * wifi_label;
    lv_obj_t * time_label;
    lv_timer_t * minute_timer;
    wifi_state_t wifi_state;
} icon_replace_2_top_bar_t;

icon_replace_2_top_bar_t * icon_replace_2_top_bar_create(lv_obj_t * parent);
void icon_replace_2_top_bar_apply(icon_replace_2_top_bar_t * bar, const topbar_page_config_t * config);
void icon_replace_2_top_bar_set_time(icon_replace_2_top_bar_t * bar, const char * hhmm);
void icon_replace_2_top_bar_set_wifi_state(icon_replace_2_top_bar_t * bar, wifi_state_t state);
void icon_replace_2_top_bar_start_minute_timer(icon_replace_2_top_bar_t * bar);

#endif
```

- [ ] **Step 2: 把顶部条实现接进构建，先让主程序在链接期失败**

```cmake
set(ICON_REPLACE_2_SOURCES
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2/icon_replace_2.c
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2/icon_replace_2_assets_bundle.c
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2/icon_replace_2_page_config.c
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2/icon_replace_2_top_bar.c
)
```

```c
/* main/icon_replace_2/icon_replace_2.c */
#include "icon_replace_2_top_bar.h"

void icon_replace_demo_2(void)
{
    lv_obj_t * active = lv_screen_active();
    icon_replace_2_top_bar_t * bar = icon_replace_2_top_bar_create(active);
    icon_replace_2_top_bar_apply(bar, icon_replace_2_get_page_config(0));
}
```

- [ ] **Step 3: 运行主程序构建，确认因缺少 `icon_replace_2_top_bar.c` 实现而失败**

Run: `rtk cmake --build build --target main -j`

Expected:
- 首次失败
- 报错集中在 `icon_replace_2_top_bar_create` 等未定义符号

- [ ] **Step 4: 写顶部条最小实现，先跑通右侧系统区和页级文本切换**

```c
/* main/icon_replace_2/icon_replace_2_top_bar.c */
#include "icon_replace_2_top_bar.h"
#include "icon_replace_2_layout.h"

#include <stdlib.h>

static void minute_timer_cb(lv_timer_t * timer)
{
    icon_replace_2_top_bar_t * bar = timer->user_data;
    if(bar == NULL) {
        return;
    }

    /* 第一阶段先放静态时间，下一任务接入真实分钟刷新 */
    lv_label_set_text(bar->time_label, "09:41");
}

icon_replace_2_top_bar_t * icon_replace_2_top_bar_create(lv_obj_t * parent)
{
    icon_replace_2_top_bar_t * bar = lv_malloc(sizeof(*bar));
    lv_memzero(bar, sizeof(*bar));

    bar->root = lv_obj_create(parent);
    lv_obj_set_size(bar->root, SCREEN_W, TOP_BAR_H);
    lv_obj_set_pos(bar->root, 0, 0);
    lv_obj_clear_flag(bar->root, LV_OBJ_FLAG_SCROLLABLE);

    bar->left_slot = lv_obj_create(bar->root);
    bar->center_slot = lv_obj_create(bar->root);
    bar->system_right_slot = lv_obj_create(bar->root);

    lv_obj_set_size(bar->left_slot, 200, TOP_BAR_H);
    lv_obj_set_size(bar->center_slot, 300, TOP_BAR_H);
    lv_obj_set_size(bar->system_right_slot, 180, TOP_BAR_H);
    lv_obj_align(bar->left_slot, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_align(bar->center_slot, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(bar->system_right_slot, LV_ALIGN_RIGHT_MID, 0, 0);

    bar->wifi_label = lv_label_create(bar->system_right_slot);
    bar->time_label = lv_label_create(bar->system_right_slot);
    lv_label_set_text(bar->wifi_label, "WiFi");
    lv_label_set_text(bar->time_label, "09:41");
    lv_obj_align(bar->wifi_label, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_align(bar->time_label, LV_ALIGN_RIGHT_MID, 0, 0);

    return bar;
}

void icon_replace_2_top_bar_apply(icon_replace_2_top_bar_t * bar, const topbar_page_config_t * config)
{
    if(bar == NULL || config == NULL) {
        return;
    }

    lv_obj_clean(bar->left_slot);
    lv_obj_clean(bar->center_slot);

    if(config->left_type == TOPBAR_SLOT_TEXT && config->left_text != NULL) {
        lv_obj_t * label = lv_label_create(bar->left_slot);
        lv_label_set_text(label, config->left_text);
        lv_obj_center(label);
    }

    if(config->center_type == TOPBAR_SLOT_TEXT && config->center_text != NULL) {
        lv_obj_t * label = lv_label_create(bar->center_slot);
        lv_label_set_text(label, config->center_text);
        lv_obj_center(label);
    }

    if(config->show_system_right) {
        lv_obj_clear_flag(bar->system_right_slot, LV_OBJ_FLAG_HIDDEN);
    }
    else {
        lv_obj_add_flag(bar->system_right_slot, LV_OBJ_FLAG_HIDDEN);
    }
}

void icon_replace_2_top_bar_set_time(icon_replace_2_top_bar_t * bar, const char * hhmm)
{
    if(bar != NULL && hhmm != NULL) {
        lv_label_set_text(bar->time_label, hhmm);
    }
}

void icon_replace_2_top_bar_set_wifi_state(icon_replace_2_top_bar_t * bar, wifi_state_t state)
{
    static const char * labels[] = { "WiFiX", "WiFi?", "WiFi1", "WiFi2", "WiFi3" };
    if(bar == NULL) {
        return;
    }

    bar->wifi_state = state;
    lv_label_set_text(bar->wifi_label, labels[state]);
}

void icon_replace_2_top_bar_start_minute_timer(icon_replace_2_top_bar_t * bar)
{
    if(bar == NULL || bar->minute_timer != NULL) {
        return;
    }

    bar->minute_timer = lv_timer_create(minute_timer_cb, 60000, bar);
}
```

- [ ] **Step 5: 重新构建主程序，确认顶部条骨架已能链接**

Run: `rtk cmake --build build --target main -j`

Expected:
- 编译通过
- `main/icon_replace_2/icon_replace_2.c` 中可正常调用顶部条接口

- [ ] **Step 6: 提交本任务**

```bash
rtk git add \
  CMakeLists.txt \
  main/icon_replace_2/icon_replace_2.c \
  main/icon_replace_2/icon_replace_2_top_bar.h \
  main/icon_replace_2/icon_replace_2_top_bar.c
rtk git commit -m "feat: add top bar module skeleton"
```

## Task 3: 把桌面分页逻辑下沉到 `desktop_host`

**Files:**
- Create: `main/icon_replace_2/icon_replace_2_desktop.h`
- Create: `main/icon_replace_2/icon_replace_2_desktop.c`
- Modify: `main/icon_replace_2/icon_replace_2.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 先声明桌面模块接口和上下文**

```c
/* main/icon_replace_2/icon_replace_2_desktop.h */
#ifndef ICON_REPLACE_2_DESKTOP_H
#define ICON_REPLACE_2_DESKTOP_H

#include "lvgl.h"

typedef struct icon_replace_2_desktop icon_replace_2_desktop_t;

typedef void (* icon_replace_2_page_changed_cb_t)(uint32_t page_index, void * user_data);

icon_replace_2_desktop_t * icon_replace_2_desktop_create(
    lv_obj_t * parent,
    icon_replace_2_page_changed_cb_t page_changed_cb,
    void * user_data);

uint32_t icon_replace_2_desktop_get_current_page(const icon_replace_2_desktop_t * desktop);

#endif
```

- [ ] **Step 2: 先把旧 `icon_replace_2.c` 里的整屏 `tileview` 创建迁到桌面模块中**

```c
/* main/icon_replace_2/icon_replace_2_desktop.c */
#include "icon_replace_2_desktop.h"
#include "icon_replace_2_layout.h"
#include "icon_replace_2_assets.h"

struct icon_replace_2_desktop {
    lv_obj_t * host;
    lv_obj_t * tileview;
    lv_obj_t * page[PAGE_COUNT];
    icon_replace_2_page_changed_cb_t page_changed_cb;
    void * user_data;
    uint32_t current_page;
};

icon_replace_2_desktop_t * icon_replace_2_desktop_create(
    lv_obj_t * parent,
    icon_replace_2_page_changed_cb_t page_changed_cb,
    void * user_data)
{
    icon_replace_2_desktop_t * desktop = lv_malloc(sizeof(*desktop));
    lv_memzero(desktop, sizeof(*desktop));

    desktop->host = lv_obj_create(parent);
    lv_obj_set_size(desktop->host, DESKTOP_W, DESKTOP_H);
    lv_obj_set_pos(desktop->host, 0, TOP_BAR_H);
    lv_obj_clear_flag(desktop->host, LV_OBJ_FLAG_SCROLLABLE);

    desktop->tileview = lv_tileview_create(desktop->host);
    lv_obj_set_size(desktop->tileview, DESKTOP_W, DESKTOP_H);

    for(uint32_t j = 0; j < PAGE_COUNT; j++) {
        desktop->page[j] = lv_tileview_add_tile(desktop->tileview, j, 0, LV_DIR_ALL);
        lv_obj_set_size(desktop->page[j], DESKTOP_W, DESKTOP_H);
        lv_obj_clear_flag(desktop->page[j], LV_OBJ_FLAG_SCROLLABLE);
    }

    desktop->page_changed_cb = page_changed_cb;
    desktop->user_data = user_data;
    desktop->current_page = 0;
    return desktop;
}

uint32_t icon_replace_2_desktop_get_current_page(const icon_replace_2_desktop_t * desktop)
{
    return desktop != NULL ? desktop->current_page : 0;
}
```

- [ ] **Step 3: 修改主组装入口，只负责建根容器、顶部条和桌面宿主**

```c
/* main/icon_replace_2/icon_replace_2.c */
#include "icon_replace_2_desktop.h"
#include "icon_replace_2_layout.h"
#include "icon_replace_2_page_config.h"
#include "icon_replace_2_top_bar.h"

typedef struct {
    lv_obj_t * root;
    icon_replace_2_top_bar_t * top_bar;
    icon_replace_2_desktop_t * desktop;
} icon_replace_2_app_t;

static void on_page_changed(uint32_t page_index, void * user_data)
{
    icon_replace_2_app_t * app = user_data;
    icon_replace_2_top_bar_apply(app->top_bar, icon_replace_2_get_page_config(page_index));
}

void icon_replace_demo_2(void)
{
    static icon_replace_2_app_t app;

    lv_obj_clean(lv_screen_active());
    app.root = lv_obj_create(lv_screen_active());
    lv_obj_set_size(app.root, SCREEN_W, SCREEN_H);
    lv_obj_set_style_pad_all(app.root, 0, 0);

    app.top_bar = icon_replace_2_top_bar_create(app.root);
    app.desktop = icon_replace_2_desktop_create(app.root, on_page_changed, &app);

    icon_replace_2_top_bar_apply(app.top_bar, icon_replace_2_get_page_config(0));
    icon_replace_2_top_bar_start_minute_timer(app.top_bar);
}
```

- [ ] **Step 4: 把桌面模块加入构建并跑整包编译**

```cmake
set(ICON_REPLACE_2_SOURCES
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2/icon_replace_2.c
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2/icon_replace_2_assets_bundle.c
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2/icon_replace_2_page_config.c
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2/icon_replace_2_top_bar.c
    ${PROJECT_SOURCE_DIR}/main/icon_replace_2/icon_replace_2_desktop.c
)
```

Run: `rtk cmake --build build --target main -j`

Expected:
- 编译通过
- 运行后顶部条固定在顶部，下方出现桌面分页宿主

- [ ] **Step 5: 手工运行模拟器，确认容器分层正确**

Run: `rtk ./bin/main`

Expected:
- SDL 窗口打开
- 顶部有固定高度条
- 下方桌面区不覆盖顶部条

- [ ] **Step 6: 提交本任务**

```bash
rtk git add \
  CMakeLists.txt \
  main/icon_replace_2/icon_replace_2.c \
  main/icon_replace_2/icon_replace_2_desktop.h \
  main/icon_replace_2/icon_replace_2_desktop.c
rtk git commit -m "refactor: split desktop host from top bar"
```

## Task 4: 迁移拖拽逻辑到桌面局部坐标并接入页切换联动

**Files:**
- Modify: `main/icon_replace_2/icon_replace_2_desktop.c`
- Modify: `main/icon_replace_2/icon_replace_2_layout.h`
- Modify: `main/icon_replace_2/icon_replace_2_top_bar.c`

- [ ] **Step 1: 在桌面模块内补充局部坐标换算辅助函数**

```c
static lv_point_t desktop_local_point(icon_replace_2_desktop_t * desktop)
{
    lv_point_t p;
    lv_area_t a;

    lv_indev_get_point(lv_indev_get_act(), &p);
    lv_obj_get_coords(desktop->host, &a);
    p.x -= a.x1;
    p.y -= a.y1;
    return p;
}

static int clamp_index(int index)
{
    if(index < 0) return 0;
    if(index >= ICON_SLOT_COUNT) return ICON_SLOT_COUNT - 1;
    return index;
}

static int index_by_local_xy(const lv_point_t * p)
{
    int index = (p->x - ICON_START_X) / ICON_X_DISTANCE +
                ((p->y - ICON_START_Y) / ICON_Y_DISTANCE) * ICON_MAX_COL;
    return clamp_index(index);
}
```

- [ ] **Step 2: 把旧的触摸、松手、跨页边界判断改成使用 `desktop_local_point()`**

```c
/* 核心替换思路，放进 icon_replace_2_desktop.c 的拖拽回调中 */
lv_point_t local = desktop_local_point(desktop);
lv_obj_set_pos(target, local.x - offsetx, local.y - offsety);

if(local.x < ICON_SIZE / 2) {
    border_lefttest_count++;
}
else if(local.x > DESKTOP_W - ICON_SIZE / 2) {
    border_righttest_count++;
}
else {
    border_lefttest_count = 0;
    border_righttest_count = 0;
}

new_index = index_by_local_xy(&local);
```

- [ ] **Step 3: 在页切换回调里同步应用顶部配置**

```c
static void desktop_tile_changed_cb(lv_event_t * e)
{
    icon_replace_2_desktop_t * desktop = lv_event_get_user_data(e);
    lv_obj_t * tile = lv_tileview_get_tile_active(desktop->tileview);

    for(uint32_t i = 0; i < PAGE_COUNT; i++) {
        if(tile == desktop->page[i]) {
            desktop->current_page = i;
            if(desktop->page_changed_cb != NULL) {
                desktop->page_changed_cb(i, desktop->user_data);
            }
            break;
        }
    }
}
```

- [ ] **Step 4: 构建并手工验证拖拽、换位、跨页**

Run:
- `rtk cmake --build build --target main -j`
- `rtk ./bin/main`

Expected:
- 普通翻页时顶部右侧状态常驻
- 切到 `page_0` 时顶部按锁屏配置变化
- 长按进入编辑态仍能拖拽
- 拖到左右边缘可跨页
- 顶部内容变化不影响落点索引

- [ ] **Step 5: 提交本任务**

```bash
rtk git add \
  main/icon_replace_2/icon_replace_2_desktop.c \
  main/icon_replace_2/icon_replace_2_layout.h \
  main/icon_replace_2/icon_replace_2_top_bar.c
rtk git commit -m "feat: localize desktop drag coordinates"
```

## Task 5: 接入分钟级时间刷新与 `WiFi` 状态更新入口

**Files:**
- Modify: `main/icon_replace_2/icon_replace_2_top_bar.h`
- Modify: `main/icon_replace_2/icon_replace_2_top_bar.c`
- Modify: `main/icon_replace_2/icon_replace_2.c`

- [ ] **Step 1: 先写最小时间格式化函数和分钟刷新逻辑**

```c
#include <time.h>

static void top_bar_refresh_time(icon_replace_2_top_bar_t * bar)
{
    time_t now = time(NULL);
    struct tm tm_now;
    char hhmm[6];

    if(bar == NULL) {
        return;
    }

    localtime_r(&now, &tm_now);
    strftime(hhmm, sizeof(hhmm), "%H:%M", &tm_now);
    lv_label_set_text(bar->time_label, hhmm);
}

static void minute_timer_cb(lv_timer_t * timer)
{
    icon_replace_2_top_bar_t * bar = timer->user_data;
    top_bar_refresh_time(bar);
}

void icon_replace_2_top_bar_start_minute_timer(icon_replace_2_top_bar_t * bar)
{
    if(bar == NULL) {
        return;
    }

    top_bar_refresh_time(bar);

    if(bar->minute_timer == NULL) {
        bar->minute_timer = lv_timer_create(minute_timer_cb, 60000, bar);
    }
}
```

- [ ] **Step 2: 把 `WiFi` 状态映射收敛成单点更新接口**

```c
void icon_replace_2_top_bar_set_wifi_state(icon_replace_2_top_bar_t * bar, wifi_state_t state)
{
    static const char * labels[] = {
        LV_SYMBOL_CLOSE,
        LV_SYMBOL_WARNING,
        LV_SYMBOL_WIFI,
        LV_SYMBOL_WIFI,
        LV_SYMBOL_WIFI,
    };

    if(bar == NULL) {
        return;
    }

    bar->wifi_state = state;
    lv_label_set_text(bar->wifi_label, labels[state]);
}
```

- [ ] **Step 3: 在主入口增加演示级系统状态初始化**

```c
void icon_replace_demo_2(void)
{
    static icon_replace_2_app_t app;

    /* ...已有 root/top_bar/desktop 创建逻辑... */
    icon_replace_2_top_bar_apply(app.top_bar, icon_replace_2_get_page_config(0));
    icon_replace_2_top_bar_set_wifi_state(app.top_bar, WIFI_STATE_NORMAL);
    icon_replace_2_top_bar_start_minute_timer(app.top_bar);
}
```

- [ ] **Step 4: 构建并运行，确认时间和 `WiFi` 状态入口工作正常**

Run:
- `rtk cmake --build build --target main -j`
- `rtk ./bin/main`

Expected:
- 启动时右上角立即显示当前 `HH:MM`
- 手动切页时右侧系统状态不重建
- 修改演示初始化的 `WiFi_STATE_*` 后，图标文本能变化

- [ ] **Step 5: 提交本任务**

```bash
rtk git add \
  main/icon_replace_2/icon_replace_2.c \
  main/icon_replace_2/icon_replace_2_top_bar.h \
  main/icon_replace_2/icon_replace_2_top_bar.c
rtk git commit -m "feat: wire minute timer and wifi state"
```

## Task 6: 收尾验证与文档回写

**Files:**
- Modify: `docs/2026-05-14-icon-replace-2-statusbar-desktop-design.md`

- [ ] **Step 1: 运行最终验证**

Run:
- `rtk ./bin/test_page_config`
- `rtk cmake --build build --target main -j`
- `rtk ./bin/main`

Expected:
- `test_page_config: PASS`
- 主程序编译通过
- 模拟器窗口中顶部条、锁屏页、普通页、编辑态和跨页拖拽均符合预期

- [ ] **Step 2: 回写设计文档的实现状态**

```md
## 实现状态

- 已完成 `top_bar + desktop_host` 容器拆分
- 已完成 `page_0` 锁屏首页模式
- 已完成右侧系统状态唯一实例
- 已完成分钟级时间刷新
- 已完成 `WiFi` 状态更新入口
- 已完成桌面局部坐标拖拽
```

- [ ] **Step 3: 提交本任务**

```bash
rtk git add docs/2026-05-14-icon-replace-2-statusbar-desktop-design.md
rtk git commit -m "docs: mark icon replace 2 implementation status"
```

## 自检

### Spec 覆盖

- 顶部固定高度：Task 1 `layout.h`，Task 3 `top_bar + desktop_host`
- 右侧时间和 `WiFi`：Task 2 顶部条骨架，Task 5 系统状态接入
- 锁屏作为第一页：Task 1 页配置，Task 4 页切换联动
- 每页顶部内容可配置：Task 1 页配置，Task 2 顶部条应用配置
- 编辑态和跨页联动：Task 4 拖拽与页切换联动

### Placeholder 扫描

- 无 `TODO/TBD/implement later`
- 每个需要改代码的步骤都给了文件路径、最小代码和验证命令

### 类型一致性

- 页配置统一使用 `topbar_page_config_t`
- `WiFi` 状态统一使用 `wifi_state_t`
- 顶部条统一使用 `icon_replace_2_top_bar_t`
- 桌面模块统一使用 `icon_replace_2_desktop_t`
