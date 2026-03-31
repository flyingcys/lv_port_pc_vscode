# MasterGo Scenes Page Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 基于 MasterGo DSL 在当前 LVGL v9 工程中实现新的 `Scenes` 页面，替换旧首页入口，并完成当前页内卡片选中态切换与截图验收。

**Architecture:** 采用 `scenes_page` + `scenes_assets` 的模块拆分方式。页面骨架、卡片状态、标题、分页和遮罩优先用 LVGL 原生对象实现；天气图标、Wi-Fi 图标和必要卡片图标本地化到资源目录；对右上角重复节点收敛为单一选中态卡片，而不是把 DSL 的静态状态稿直接堆成重叠对象。

**Tech Stack:** C99、LVGL 9.3、SDL 模拟器、Tiny TTF、本地 PNG 资源、CMake、可选最小 C 级状态测试

---

## 文件结构

- Modify: `CMakeLists.txt`
- Modify: `main/src/main.c`
- Create: `main/src/scenes_page.c`
- Create: `main/inc/scenes_page.h`
- Create: `main/src/scenes_assets.c`
- Create: `main/inc/scenes_assets.h`
- Optional Create: `main/src/scenes_state.c`
- Optional Create: `main/inc/scenes_state.h`
- Optional Create: `tests/scenes_state_test.c`
- Create: `main/assets/scenes/weather.png`
- Create: `main/assets/scenes/wifi.png`
- Optional Create: `main/assets/scenes/card-home.png`
- Optional Create: `main/assets/scenes/card-leaving-home.png`
- Optional Create: `main/assets/scenes/card-morning.png`
- Optional Create: `main/assets/scenes/card-all-lights-on.png`
- Reference: `docs/superpowers/specs/2026-03-31-mastergo-scenes-design.md`

## 约束说明

- 当前工作区已经存在未提交改动，实施时不能回滚或覆盖无关修改。
- 页面目标是固定尺寸 `480 x 480`，必须按 DSL 绝对坐标实现。
- 本轮只实现当前页内状态切换，不实现“下一个动作”或页面跳转。
- 如果测试基础设施成本过高，只为“卡片选中态状态机”补最小测试；其余以构建、运行和截图核对为主。

## Chunk 1: 新模块骨架与资源入口

### Task 1: 建立 `scenes_page` / `scenes_assets` 模块并切换首页入口

**Files:**
- Create: `main/src/scenes_page.c`
- Create: `main/inc/scenes_page.h`
- Create: `main/src/scenes_assets.c`
- Create: `main/inc/scenes_assets.h`
- Modify: `main/src/main.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 为页面入口写最小失败验证**

目标：

- `main.c` 不再依赖旧 `home_page`
- 构建阶段能够暴露缺失的 `scenes_page_create()` 声明/定义

- [ ] **Step 2: 运行构建，确认当前处于 RED 状态**

Run: `cmake -S . -B build && cmake --build build`

Expected:

- 因 `scenes_page_create` 未定义或 `CMakeLists.txt` 未接入新文件而失败

- [ ] **Step 3: 新建 `scenes_page.h/.c` 与 `scenes_assets.h/.c` 最小骨架**

要求：

- `scenes_page_create()` 先只创建一个深色 `480 x 480` 根容器
- `scenes_assets` 先只提供颜色宏、页面尺寸宏和基础资源路径接口
- 头文件遵循当前仓库 C 代码风格，带英文 Doxygen 注释

- [ ] **Step 4: 在 `main.c` 与 `CMakeLists.txt` 接入新模块**

Expected:

- 启动入口改为 `scenes_page_create()`
- 新源文件进入构建

- [ ] **Step 5: 再次构建，确认进入 GREEN 状态**

Run: `cmake -S . -B build && cmake --build build`

Expected:

- 构建通过
- 启动后至少能看到深色空白页，不再依赖旧首页

## Chunk 2: 顶部信息栏、标题区和底部骨架

### Task 2: 实现页面主骨架和非卡片区域

**Files:**
- Modify: `main/src/scenes_page.c`
- Modify: `main/src/scenes_assets.c`
- Modify: `main/inc/scenes_assets.h`
- Create: `main/assets/scenes/weather.png`
- Create: `main/assets/scenes/wifi.png`

- [ ] **Step 1: 为顶部信息栏和标题区写最小失败验证**

验证目标：

- 页面对象树包含顶部信息栏、`Scenes`、`Devices`、底部遮罩和分页
- 运行截图时这些主区块必须可见

- [ ] **Step 2: 运行程序，确认截图现状缺少这些区域**

Run: `./bin/main`

Expected:

- 当前页面仍只有基础背景，未出现完整顶栏/标题/分页

- [ ] **Step 3: 在 `scenes_assets` 中补充字体、颜色和资源 getter**

至少包含：

- `Source Han Sans CN` 标题字体
- `Urbanist` 温度/时间字体
- `SF UI Display` 或可接受替代字体用于 `AM` / 日期
- `weather.png`
- `wifi.png`
- 青色激活色、白色文字色、深色背景和底部渐变色

- [ ] **Step 4: 在 `scenes_page.c` 中实现非卡片主骨架**

必须按 DSL 绝对坐标创建：

- 顶部信息栏 `x=16, y=15, w=448, h=32`
- `Scenes` 标题 `x=16, y=70`
- `Devices` 标题 `x=144, y=70`
- 底部渐变遮罩 `y=428, h=52`
- 分页 `x=226, y=462, w=28, h=6`

- [ ] **Step 5: 构建并运行截图检查**

Run:

- `cmake --build build`
- `./bin/main`

Expected:

- 顶部信息栏和标题区可见
- 分页和底部遮罩位置正确
- 字体没有明显缺失或乱码

## Chunk 3: 卡片状态模型与最小测试

### Task 3: 抽出卡片选中态状态模型并用最小测试验证

**Files:**
- Optional Create: `main/src/scenes_state.c`
- Optional Create: `main/inc/scenes_state.h`
- Optional Create: `tests/scenes_state_test.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 为卡片状态切换写失败测试**

建议测试行为：

- 初始激活卡片为 `Morning`
- 点击任一卡片后，激活索引切到对应卡片
- 再次点击同一卡片时状态保持稳定
- 非法索引不会破坏当前状态

- [ ] **Step 2: 运行测试，确认当前失败**

Run: `cmake -S . -B build && cmake --build build --target scenes_state_test && ./build/scenes_state_test`

Expected:

- 因状态模块尚未实现而失败，或目标不存在而失败

- [ ] **Step 3: 以最小实现补齐状态模块**

建议：

- 用纯 C 状态结构保存当前选中索引
- 提供初始化、点击更新和只读查询接口
- 让状态逻辑不依赖 LVGL，便于测试

- [ ] **Step 4: 再次运行测试，确认通过**

Run: `cmake --build build --target scenes_state_test && ./build/scenes_state_test`

Expected:

- 最小状态测试通过

- [ ] **Step 5: 清理命名并保持测试继续通过**

Expected:

- 状态模块保持小而纯粹
- 不把 UI 逻辑和状态测试混在一起

## Chunk 4: 四张卡片与点击反馈

### Task 4: 用统一卡片组件实现四张场景卡片

**Files:**
- Modify: `main/src/scenes_page.c`
- Modify: `main/src/scenes_assets.c`
- Modify: `main/inc/scenes_assets.h`
- Modify: `main/src/scenes_state.c`
- Modify: `main/inc/scenes_state.h`
- Optional Create: `main/assets/scenes/card-home.png`
- Optional Create: `main/assets/scenes/card-leaving-home.png`
- Optional Create: `main/assets/scenes/card-morning.png`
- Optional Create: `main/assets/scenes/card-all-lights-on.png`

- [ ] **Step 1: 运行现状，确认卡片尚未按设计出现**

Run: `./bin/main`

Expected:

- 页面缺少完整四卡片布局或没有点击反馈

- [ ] **Step 2: 实现统一卡片配置结构**

要求：

- 卡片标题、图标、坐标、选中态颜色通过配置传入
- 四张卡片共享一套创建函数

- [ ] **Step 3: 按 DSL 坐标实现四张卡片**

必须覆盖：

- 左上 `x=16, y=130, w=219, h=145`
- 左下 `x=16, y=285, w=219, h=145`
- 右上 `x=245, y=130, w=219, h=145`
- 右下 `x=245, y=285, w=219, h=145`

并落实：

- 默认玻璃态卡片使用 `rgba(255,255,255,0.15)` 风格
- 选中态 `Morning` 使用蓝色渐变
- 标题文字按 `Source Han Sans CN 18 Medium`

- [ ] **Step 4: 接入点击事件与状态刷新**

要求：

- 点击任一卡片仅更新当前页选中态
- 初始默认选中 `Morning`
- 不实现跳转或“下一个动作”

- [ ] **Step 5: 构建并运行截图检查**

Run:

- `cmake --build build`
- `./bin/main`

Expected:

- 四张卡片位置正确
- 右上卡片初始高亮
- 点击其他卡片可见选中态切换

## Chunk 5: 视觉校准与最终验收

### Task 5: 完成截图比对、细节微调和收尾验证

**Files:**
- Modify: `main/src/scenes_page.c`
- Modify: `main/src/scenes_assets.c`
- Modify: `main/inc/scenes_assets.h`
- Reference: `docs/superpowers/specs/2026-03-31-mastergo-scenes-design.md`

- [ ] **Step 1: 对照 spec 逐项复核关键设计值**

复核项：

- 顶部信息栏
- 标题区
- 四张卡片
- 底部遮罩
- 分页

- [ ] **Step 2: 微调渐变、透明度、间距和文字位置**

Expected:

- 不改主结构，只做精确对齐微调

- [ ] **Step 3: 执行最终全量构建**

Run: `cmake -S . -B build && cmake --build build -j$(nproc)`

Expected:

- 构建通过

- [ ] **Step 4: 运行模拟器做最终截图验收**

Run: `./bin/main`

Expected:

- 页面整体与 MasterGo 设计稿高度一致
- 卡片点击只切换当前页选中态
- 没有白块遮挡、资源丢失、字体缺失或明显错位

- [ ] **Step 5: 汇总剩余偏差并记录原因**

输出至少包括：

- 最终修改文件
- 本地化资源列表
- 字体匹配情况
- 构建结果
- 运行结果
- 仍存在的偏差点与原因
