# MasterGo 首页 LVGL 还原 Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 基于 MasterGo DSL 和设计资源，把当前 LVGL 首页改到与设计稿尽量一致，并去掉最外层页面圆角。

**Architecture:** 继续沿用现有 `home_page` 模块接入 `main.c` 的方式，在 LVGL 中保留页面结构、定位和文本绘制；对背景氛围、天气图标、冒号和必要的卡片材质层改用设计资源，减少纯样式近似导致的视觉偏差。本轮仅做静态视觉对齐，不扩展交互逻辑。

**Tech Stack:** C99、LVGL 9.3、SDL 模拟器、Tiny TTF、本地位图资源、CMake

---

## 文件结构

- Modify: `CMakeLists.txt`
- Modify: `lv_conf.h`
- Modify: `main/src/main.c`
- Modify: `main/src/home_page.c`
- Modify: `main/src/home_assets.c`
- Modify: `main/inc/home_page.h`
- Modify: `main/inc/home_assets.h`
- Create: `main/assets/home/background.jpg`
- Create: `main/assets/home/weather-icon.png`
- Create: `main/assets/home/wifi-icon.png`
- Create: `main/assets/home/clock-colon.png`
- Optional Create: `main/assets/home/switch-card-off.png`
- Optional Create: `main/assets/home/switch-card-on.png`
- Optional Create: `main/assets/home/pager-indicator.png`
- Reference: `docs/superpowers/specs/2026-03-30-mastergo-home-design.md`

## 约束说明

- 当前仓库没有现成的 LVGL 页面自动化测试基建，本次以 `编译通过 + 模拟器运行 + 视觉核对` 为主。
- 当前页面工作基于未提交改动继续演进，实施时必须在当前工作区谨慎修改，不能回滚用户已有改动。
- 除非用户明确要求，否则不要执行 git commit。

## Chunk 1: 资源和接入准备

### Task 1: 让工程具备加载首页资源和字体的前置条件

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `lv_conf.h`
- Modify: `main/src/main.c`
- Modify: `main/inc/home_assets.h`
- Modify: `main/src/home_assets.c`
- Create: `main/assets/home/background.jpg`
- Create: `main/assets/home/weather-icon.png`
- Create: `main/assets/home/wifi-icon.png`
- Create: `main/assets/home/clock-colon.png`

- [ ] **Step 1: 确认 `main/assets/home/` 目录存在并放入首批资源**

资源至少包括：

- `background.jpg`
- `weather-icon.png`
- `wifi-icon.png`
- `clock-colon.png`

- [ ] **Step 2: 在 `home_assets.h` 中补全资源路径和设计 token 宏**

至少覆盖：

- 页面尺寸 `480 x 480`
- 页面底色 `paint_161:3344`
- 卡片主色、文字色、强调色
- 资源路径宏和字体路径宏

- [ ] **Step 3: 在 `home_assets.c` 中补全资源 getter 和字体缓存**

要求：

- 背景、天气、Wi-Fi、冒号均有独立路径函数
- 时钟、温度、日期/星期、卡片标题各自有缓存字体入口
- 保留 Tiny TTF 失败时的 fallback

- [ ] **Step 4: 在 `main.c` 中确认首页入口仍然调用 `home_page_create()`**

Expected: 默认不再走 `lv_demo_widgets()`，启动后直接展示首页。

- [ ] **Step 5: 编译验证接入层**

Run: `cmake -S . -B build && cmake --build build`
Expected: 编译通过，没有资源/符号缺失导致的错误

## Chunk 2: 背景和顶部区域对齐

### Task 2: 修正背景氛围、天气模块和 Wi-Fi 图标

**Files:**
- Modify: `main/src/home_page.c`
- Modify: `main/inc/home_assets.h`
- Modify: `main/src/home_assets.c`
- Optional Create: `main/assets/home/weather-icon.png`
- Optional Create: `main/assets/home/wifi-icon.png`

- [ ] **Step 1: 去掉最外层页面容器圆角**

要求：

- 根容器不设置圆角
- 不启用根容器裁切圆角
- 内部卡片圆角保持不变

- [ ] **Step 2: 用设计资源和覆盖层重做背景区域**

要求：

- 背景图使用设计稿资源
- 仍保留深色底色
- 通过 wash/vignette 控制前景对比度
- 避免当前截图里的“顶区发白”和“背景干扰文字”

- [ ] **Step 3: 按 DSL 坐标重做天气模块**

目标数据：

- 容器位置：`x=28, y=28`
- 温度：`26℃`
- 日期：`6/24`
- 星期：`周一`

要求：

- 文字字重和字号贴近 DSL
- 日期和星期必要时改为图片或更接近设计的等价实现
- 不再使用明显偏灰、偏淡的近似效果

- [ ] **Step 4: 按 DSL 位置重做右上 Wi-Fi**

目标数据：

- `x=432, y=20, size=28`

- [ ] **Step 5: 编译并运行顶部区域验证**

Run: `cmake --build build && ./bin/main`
Expected: 模拟器启动后，顶部区域无遮挡、无明显发白、Wi-Fi 位置正确

## Chunk 3: 时钟、中下部卡片和分页对齐

### Task 3: 修正时钟、双卡片和分页视觉

**Files:**
- Modify: `main/src/home_page.c`
- Modify: `main/inc/home_page.h`
- Optional Create: `main/assets/home/switch-card-off.png`
- Optional Create: `main/assets/home/switch-card-on.png`
- Optional Create: `main/assets/home/pager-indicator.png`

- [ ] **Step 1: 按 DSL 坐标重做时钟区**

目标数据：

- 容器：`x=28, y=113, w=278, h=132`
- 内容：`09:26`

要求：

- 数字字体为 Urbanist 110
- 双点冒号改为红色资源或等价精确图形
- 数字间距按设计稿重新校准

- [ ] **Step 2: 按 DSL 坐标重做左卡片**

目标数据：

- `x=28, y=298, w=204, h=140`
- 标题：`开关一`

要求：

- 背景使用深色半透明材质
- 底部白色细条位于 `x=88, y=120`
- 卡片圆角保持 `20px`

- [ ] **Step 3: 按 DSL 坐标重做右卡片**

目标数据：

- `x=248, y=298, w=204, h=140`
- 标题：`开关二`

要求：

- 背景使用浅色渐变材质
- 底部青色条位于 `x=88, y=120`
- 文字颜色为近黑色

- [ ] **Step 4: 按 DSL 坐标重做底部分页**

目标数据：

- `x=226, y=462, w=28, h=6`

要求：

- 左侧青色长条，右侧半透明白点
- 水平间距与设计稿一致

- [ ] **Step 5: 编译并运行中下部区域验证**

Run: `cmake --build build && ./bin/main`
Expected: 时钟、卡片、分页位置和材质接近设计稿，没有明显错位

## Chunk 4: 收尾校准和回归验证

### Task 4: 做最终视觉微调和工程验证

**Files:**
- Modify: `main/src/home_page.c`
- Modify: `main/src/home_assets.c`
- Modify: `main/inc/home_assets.h`
- Modify: `main/src/main.c`
- Reference: `docs/superpowers/specs/2026-03-30-mastergo-home-design.md`

- [ ] **Step 1: 对照设计稿逐项复核坐标、颜色和字体**

复核项：

- 顶部天气区
- 时钟数字与冒号
- 两张卡片材质与文字
- Wi-Fi 与分页
- 根容器无圆角

- [ ] **Step 2: 清理 `home_page.c` 中为调试残留的冗余样式或占位对象**

Expected: 最终代码只保留交付所需对象和样式。

- [ ] **Step 3: 执行最终编译验证**

Run: `cmake -S . -B build && cmake --build build`
Expected: 全量编译通过

- [ ] **Step 4: 运行模拟器做最终人工验收**

Run: `./bin/main`
Expected: 页面整体与 MasterGo `首页` 设计稿高度一致，且没有截图中原有的明显问题
