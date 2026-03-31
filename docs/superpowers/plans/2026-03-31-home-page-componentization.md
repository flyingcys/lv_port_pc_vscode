# Home Page Weather/Clock Componentization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 `home_page.c` 中的天气区和时钟区从整块位图改为独立 `lv_image/lv_label` 对象，同时尽量保持当前视觉效果不变。

**Architecture:** 保留首页现有的固定坐标页面结构，不引入 `flex`。天气区改为容器 + 图标/温度/日期文本对象，时钟区改为容器 + 小时标签/冒号图片/分钟标签；测试只验证对象结构和关键文本/资源，不做截图对比。

**Tech Stack:** C99、LVGL 9.3、CMake、自定义可执行测试、Tiny TTF、本地位图资源

---

## 文件结构

- Modify: `CMakeLists.txt`
- Modify: `main/inc/home_page.h`
- Modify: `main/src/home_page.c`
- Optional Modify: `main/inc/home_assets.h`
- Optional Modify: `main/src/home_assets.c`
- Create: `tests/home_page_test.c`

## Task 1: 建立失败测试并接入构建

**Files:**
- Modify: `CMakeLists.txt`
- Create: `tests/home_page_test.c`

- [ ] **Step 1: 写结构性失败测试**

测试目标：

- 创建一个最小 `LVGL` display 环境
- 调用 `home_page_create()`
- 递归查找对象树，断言存在：
  - 使用 `home_assets_get_path_weather_png()` 的天气图标 `lv_image`
  - 使用 `home_assets_get_path_clock_colon_png()` 的冒号 `lv_image`
  - 文本为 `26` 的标签
  - 文本为 `6/24` 的标签
  - 文本为 `09` 的标签
  - 文本为 `26` 的分钟标签

- [ ] **Step 2: 把测试目标接入 `CMakeLists.txt`**

新增 `home_page_test` 可执行文件，至少包含：

- `tests/home_page_test.c`
- `main/src/home_page.c`
- `main/src/home_assets.c`

- [ ] **Step 3: 运行测试并确认失败**

Run: `cmake -S . -B build && cmake --build build --target home_page_test && ./build/home_page_test`
Expected: 失败，原因是当前 `home_page_create()` 仍使用 `weather-block.png` 和 `clock-group.png`，找不到独立天气图标、冒号和文本标签。

## Task 2: 最小实现天气区和时钟区组件化

**Files:**
- Modify: `main/inc/home_page.h`
- Modify: `main/src/home_page.c`
- Optional Modify: `main/inc/home_assets.h`
- Optional Modify: `main/src/home_assets.c`

- [ ] **Step 1: 为天气区增加独立对象句柄**

在 `HOME_PAGE_CTX_T` 中增加天气区和时钟区关键对象句柄，至少包括：

- `weather_container`
- `weather_icon`
- `weather_temp_label`
- `weather_date_label`
- `clock_container`
- `clock_hour_label`
- `clock_colon_image`
- `clock_minute_label`

- [ ] **Step 2: 用独立对象重写天气区**

实现原则：

- 保持外框位置仍为 `x=28, y=28, w=128, h=85`
- 使用 `home_assets_get_path_weather_png()`
- 使用文本标签表达温度和日期
- 不引入 `flex`
- 内部对象使用显式坐标微调，优先贴近当前视觉

- [ ] **Step 3: 用独立对象重写时钟区**

实现原则：

- 保持外框位置仍为 `x=28, y=113, w=278, h=132`
- 使用两个数字标签和一个 `clock-colon.png`
- 继续复用 `home_assets_font_cached_clock()`
- 内部坐标手工校准，尽量贴近当前 `09:26` 视觉效果

- [ ] **Step 4: 运行测试并确认通过**

Run: `cmake --build build --target home_page_test && ./build/home_page_test`
Expected: 通过，说明天气区和时钟区已经是独立对象树。

## Task 3: 回归构建验证

**Files:**
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 确保 `main` 目标也能编译 `home_page` 模块**

将 `main/src/home_page.c` 与 `main/src/home_assets.c` 加入主工程源码，避免首页模块只在测试里编译。

- [ ] **Step 2: 做最终构建验证**

Run: `cmake -S . -B build && cmake --build build`
Expected: `main` 和 `home_page_test` 都编译通过。
