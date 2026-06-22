# Design Fruit LVGL Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 `design-ui/fruit` HTML 样机复刻为 LVGL v9 桌面应用，并接入 desktop。

**Architecture:** 新增独立 `main/src/design_fruit/` 模块承载资源路径、消除棋盘模型和 LVGL 视图；新增 `main/desktop/design_fruit_app.*` 只负责 desktop overlay 包装。desktop 注册表和 `AM_APP=design_fruit` 提供桌面点击与无头截图入口。

**Tech Stack:** C99、LVGL v9、SDL desktop、现有 `desktop_app_launcher_open_with_close`、现有 `AM_SHOT` PPM 快照链路。

## Global Constraints

- 使用中文交流和回复；所有 markdown 文档使用中文。
- HTML 样机是唯一视觉事实来源：`design-ui/fruit/index.html`、`css/all.css`、`js/all.js`、`png/`、`svg/`、`mp3/`。
- 主档为 `800x480` desktop overlay；HTML 本体 `680x480` 原尺寸居中，左右各留 `60px`，不滚动不拉伸。
- MVP 范围包含核心玩法：8x8 棋盘、7 种水果、选择相邻块、交换、三连检测、消除、下落、补块、分数刷新。
- 非核心按钮保留视觉：新游戏可重置；模式、提示、排行榜、历史数据、背景音乐、帮助可显示占位状态，不接后端持久化。
- 资源复制到 `main/assets/design_fruit/`，不直接运行 `design-ui/fruit` 下资源。
- 编辑函数/类/方法前必须先跑 GitNexus upstream impact；HIGH/CRITICAL 需先告知用户。
- 验证至少包含 build、相关 ctest、`AM_APP=design_fruit AM_SHOT=...` 无头截图。

---

### Task 1: RED 测试

**Files:**
- Create: `main/tests/design_fruit_model_test.c`
- Modify: `main/tests/desktop_apple_music_test.c`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces expected API: `design_fruit_model_init`, `design_fruit_model_set_board`, `design_fruit_model_swap`, `design_fruit_model_score`, `design_fruit_app_launch`

- [ ] 新增 model 测试，断言初始化棋盘没有三连，固定棋盘交换后可消除并加分。
- [ ] 更新 desktop 注册测试，断言 `"水果对对碰"` 入口存在并指向 `design_fruit_app_launch`。
- [ ] 将两个测试接入 CMake。
- [ ] 运行 `rtk cmake -S . -B build && rtk cmake --build build --target design_fruit_model_test desktop_apple_music_test -j4`，预期因为生产 API 未实现失败。

### Task 2: 模型和资源

**Files:**
- Create: `main/src/design_fruit/design_fruit_model.h`
- Create: `main/src/design_fruit/design_fruit_model.c`
- Create: `main/src/design_fruit/design_fruit_assets.h`
- Create: `main/src/design_fruit/design_fruit_assets.c`
- Copy: `design-ui/fruit/png/*` to `main/assets/design_fruit/png/`
- Copy/convert: `design-ui/fruit/svg/*` to `main/assets/design_fruit/png/fruit/`
- Copy: `design-ui/fruit/mp3/*` to `main/assets/design_fruit/mp3/`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces model API consumed by view.
- Produces `design_fruit_assets_build_image_path(char *out, size_t out_size, const char *relative_path)`.

- [ ] 复制 PNG/MP3，SVG 转 60x60 PNG。
- [ ] 实现 deterministic seed 的模型初始化，保证无初始三连。
- [ ] 实现相邻交换、三连收集、消除、重力下落、补块、分数规则。
- [ ] 接入 CMake，跑 `design_fruit_model_test` 变绿。

### Task 3: LVGL 视图和 desktop 接线

**Files:**
- Create: `main/src/design_fruit/design_fruit.h`
- Create: `main/src/design_fruit/design_fruit.c`
- Create: `main/desktop/design_fruit_app.h`
- Create: `main/desktop/design_fruit_app.c`
- Modify: `main/desktop/desktop_data.c`
- Modify: `main/src/main.c`

**Interfaces:**
- Consumes model/assets API。
- Produces `design_fruit_create(parent, screen_w, screen_h)`、`design_fruit_stop()`、`design_fruit_app_launch()`。

- [ ] 构建 800x480 overlay 内居中 680x480 本体。
- [ ] 左栏复刻背景 logo、计分圆、按钮。
- [ ] 右侧棋盘用 `bg.png` 平铺感背景和 8x8 水果图片。
- [ ] 鼠标点击/拖动映射到模型交换；刷新分数和棋盘。
- [ ] 接入 desktop 注册表，添加 `AM_APP=design_fruit`。
- [ ] 跑 desktop 注册测试和主程序构建。

### Task 4: 验证和截图

**Files:**
- No required code changes unless verification exposes defects.

- [ ] 运行 `rtk cmake --build build --target design_fruit_model_test desktop_apple_music_test main -j4`。
- [ ] 运行 `rtk ctest --test-dir build -R 'design_fruit|desktop_apple_music' --output-on-failure`。
- [ ] 运行 `SDL_VIDEODRIVER=offscreen AM_RES=800x480 AM_APP=design_fruit AM_SHOT=/tmp/design_fruit.ppm rtk ./bin/main`。
- [ ] 如可用，运行 Chrome HTML baseline 截图，人工对比 `/tmp/design_fruit.ppm` 和 HTML 视觉结构。
