# Apple Music 风格音乐播放器 UI Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 先交付一个可审阅的 `HTML` 高保真样机，再在 `macOS + SDL` 下实现一个使用 `LVGL v9 flex/grid` 的 Apple Music 风格音乐播放器 UI。

**Architecture:** `HTML` 样机与 `LVGL` 实现共用同一套信息架构、主题色命名和页面边界：左栏常驻、右侧内容区切页、底部迷你播放条常驻。`LVGL` 侧将代码拆成主题状态、假数据、全局 shell 和各页面 view，页面主结构统一通过 `flex/grid` 组织，避免整屏绝对定位。

**Tech Stack:** HTML/CSS/JavaScript, C99, LVGL v9, SDL2, CMake, ctest

---

## 文件结构

### 新建文件

- `third-party/apple_music_player_mockup/index.html`：HTML 样机入口。
- `third-party/apple_music_player_mockup/styles.css`：样机视觉样式与 800x480 mockup 外壳。
- `third-party/apple_music_player_mockup/app.js`：样机状态、页面切换、设置页主题切换。
- `main/src/v9_music_player/music_player.h`：对外暴露 `music_player_start()`。
- `main/src/v9_music_player/music_player_theme.h`
- `main/src/v9_music_player/music_player_theme.c`：主题模式、主题色、样式 token。
- `main/src/v9_music_player/music_player_mock_data.h`
- `main/src/v9_music_player/music_player_mock_data.c`：广播、本地、歌单、播放条的静态假数据。
- `main/src/v9_music_player/music_player_shell.h`
- `main/src/v9_music_player/music_player_shell.c`：全局根容器、左栏、右侧内容区、底部播放条、页面切换。
- `main/src/v9_music_player/music_player_home_view.c`：主页。
- `main/src/v9_music_player/music_player_radio_view.c`：广播页。
- `main/src/v9_music_player/music_player_local_view.c`：本地页。
- `main/src/v9_music_player/music_player_playlist_view.c`：歌单页。
- `main/src/v9_music_player/music_player_settings_view.c`：设置页。
- `main/tests/music_player_html_mockup_test.c`：HTML 样机契约测试，覆盖默认主题、`auto-1` 色槽和 800x480 外壳。
- `main/tests/music_player_theme_test.c`：主题状态回归测试。
- `main/tests/music_player_mock_data_test.c`：假数据边界与数量回归测试。

### 修改文件

- `main/src/main.c`：将 SDL 启动分辨率改为 `800x480`，入口从 `fruit_ninja_start()` 切到 `music_player_start()`。
- `CMakeLists.txt`：加入 `main/src/v9_music_player/*.c` 与新增测试目标。

### 不修改文件

- `main/src/freertos_main.cpp`：明确不参与本次实现。
- `main/src/v9-fruit_ninja/*`：作为现有参考实现保留，不在本线修改。

---

### Task 1: 搭建 HTML 样机骨架并跑通主页/设置页静态结构

**Files:**
- Create: `third-party/apple_music_player_mockup/index.html`
- Create: `third-party/apple_music_player_mockup/styles.css`
- Create: `third-party/apple_music_player_mockup/app.js`

- [x] **Step 1: 创建 HTML 入口骨架**

```html
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Apple Music Player Mockup</title>
  <link rel="stylesheet" href="./styles.css">
</head>
<body data-theme="cyan" data-page="home">
  <div class="app-shell">
    <aside class="sidebar"></aside>
    <main class="content-shell"></main>
    <footer class="mini-player"></footer>
  </div>
  <script src="./app.js"></script>
</body>
</html>
```

- [x] **Step 2: 写入 800x480 样机外壳和基础视觉 token**

```css
:root {
  --bg: #f4f5f8;
  --panel: rgba(255,255,255,0.72);
  --text: #1f232d;
  --muted: #8f95a3;
  --accent: #ff2d55;
}

body {
  margin: 0;
  min-height: 100vh;
  display: grid;
  place-items: center;
  background: linear-gradient(180deg, #eef1f6 0%, #e6e9f1 100%);
}

.app-shell {
  width: 800px;
  height: 480px;
  display: grid;
  grid-template-columns: 160px 1fr;
  grid-template-rows: 1fr 72px;
}
```

- [x] **Step 3: 用静态模板先填出主页和设置页占位**

```js
const state = { page: "home", theme: "cyan" };

function render() {
  renderSidebar();
  renderContent();
  renderMiniPlayer();
}

render();
```

- [x] **Step 4: 在浏览器中打开样机确认结构能加载**

Run: `rtk python3 -m http.server 8029 --directory third-party/apple_music_player_mockup`
Expected: 终端显示 `Serving HTTP on ... 8029`，浏览器打开 `http://localhost:8029` 可见 800x480 mockup，至少能看到左栏、主页标题、设置页占位。

Current evidence: 2026-05-30 已启动 `rtk python3 -m http.server 8029 --directory third-party/apple_music_player_mockup`，`curl -fsS http://localhost:8029/` 可加载入口 HTML；Google Chrome headless 已生成 `/tmp/music_player_html_mockup.png`，截图可见 `800x480` 样机外壳、左栏、默认 `主页`、Cyan 主题 hero、右侧推荐广播与底部 mini-player。`music_player_html_mockup_test` 覆盖入口、`800x480` CSS 外壳和默认 `data-theme="cyan"`。

- [ ] **Step 5: Commit**

```bash
rtk git add third-party/apple_music_player_mockup/index.html third-party/apple_music_player_mockup/styles.css third-party/apple_music_player_mockup/app.js
rtk git commit -m "feat: add apple music html mockup shell"
```

### Task 2: 完成 HTML 样机交互并把主题切换做进设置页

**Files:**
- Modify: `third-party/apple_music_player_mockup/index.html`
- Modify: `third-party/apple_music_player_mockup/styles.css`
- Modify: `third-party/apple_music_player_mockup/app.js`

- [x] **Step 1: 先补全 JS 状态与导航切页逻辑**

```js
const pages = ["home", "radio", "local", "playlist", "settings"];
const themes = ["cyan", "blue", "mint", "auto-1"];

function setPage(page) {
  state.page = page;
  render();
}

function setTheme(theme) {
  state.theme = theme;
  document.body.dataset.theme = theme;
  render();
}
```

- [x] **Step 2: 把首页内容与设置页结构做成真实可切换模板**

```js
function renderContent() {
  const root = document.querySelector(".content-shell");
  if (state.page === "home") root.innerHTML = homeTemplate();
  else if (state.page === "settings") root.innerHTML = settingsTemplate();
  else root.innerHTML = listPageTemplate(state.page);
}
```

- [x] **Step 3: 在设置页中加入主题色卡片和选中态**

```html
<section class="settings-card">
  <h3>主题颜色</h3>
  <div class="theme-swatches">
    <button data-theme="cyan">Cyan</button>
    <button data-theme="blue">Blue</button>
    <button data-theme="mint">Mint</button>
    <button data-theme="auto-1">自动一</button>
  </div>
</section>
```

- [x] **Step 4: 写出三套内置主题变量和一套 `自动一` 自定义色槽联动**

```css
body[data-theme="cyan"] { --accent: #4bbcae; --hero-a: #a8ebe0; --hero-b: #59c7ba; }
body[data-theme="blue"] { --accent: #4c7dff; --hero-a: #7ba2ff; --hero-b: #405fdb; }
body[data-theme="mint"] { --accent: #27b29b; --hero-a: #79dec9; --hero-b: #2e9985; }
body[data-theme="auto-1"] { --accent: var(--user-accent); --hero-a: var(--user-hero-a); --hero-b: var(--user-hero-b); }
```

补充要求：

- 默认主题颜色为 `Cyan`（淡青色）
- `Cyan / Blue / Mint` 是软件内置的三套默认颜色
- `自动一` 是用户自定义颜色槽
- HTML 样机阶段至少体现 `自动一` 的独立入口、预览色和选中态；真实可编辑逻辑可在后续继续补

Current evidence: `third-party/apple_music_player_mockup/app.js` 默认 `state.page = "home"`、`state.theme = "cyan"`，左栏包含 `主页 / 广播 / 本地 / 歌单 / 设置`，主题色卡为 `Cyan / Blue / Mint / 自动一`；`styles.css` 包含 `body[data-theme="cyan|blue|mint|auto-1"]`，其中 `auto-1` 通过 `--user-accent`、`--user-hero-a`、`--user-hero-b`、`--user-hero-c` 作为用户自定义色槽。`music_player_html_mockup_test` 已断言不再存在旧的 `orange` 色槽。

- [ ] **Step 5: 手工验证 HTML 样机**

Run: `open http://localhost:8029`
Expected: 左栏五项可切换；`设置` 页中的四个色卡可切换；默认色为 `Cyan`；前三个色卡是软件预置色；`自动一` 为用户可自定义色槽；主页 hero、选中菜单、底部进度高亮跟着变化。

Current evidence: 自动化证据覆盖源码契约、HTTP 加载和首页截图；尚未进行人工点击验证，因此本项不标记完成。

- [ ] **Step 6: 停在用户确认 gate，不进入 LVGL**

Run: `rtk git status --short`
Expected: 只有 `third-party/apple_music_player_mockup/*` 三个文件变更。把浏览器效果给用户确认，明确等待“样机认可”后再继续 Task 3。

Current evidence: 本轮目标已由用户明确推进到“开始 LVGL v9 开发”，因此此 gate 被上层目标覆盖；不能表述为已完成的用户确认。

- [ ] **Step 7: Commit**

```bash
rtk git add third-party/apple_music_player_mockup/index.html third-party/apple_music_player_mockup/styles.css third-party/apple_music_player_mockup/app.js
rtk git commit -m "feat: add html mockup interactions"
```

### Task 3: 用 TDD 建立 LVGL 主题状态与假数据边界

**Files:**
- Create: `main/src/v9_music_player/music_player_theme.h`
- Create: `main/src/v9_music_player/music_player_theme.c`
- Create: `main/src/v9_music_player/music_player_mock_data.h`
- Create: `main/src/v9_music_player/music_player_mock_data.c`
- Create: `main/tests/music_player_theme_test.c`
- Create: `main/tests/music_player_mock_data_test.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 先写主题失败测试**

```c
#include <assert.h>
#include "../src/v9_music_player/music_player_theme.h"

int main(void)
{
    music_player_theme_state_t state = music_player_theme_default_state();
    assert(state.mode == MUSIC_PLAYER_THEME_LIGHT);
    assert(state.accent == MUSIC_PLAYER_ACCENT_CYAN);

    music_player_theme_set_accent(&state, MUSIC_PLAYER_ACCENT_MINT);
    assert(music_player_theme_accent_hex(&state) == 0x27B29B);
    return 0;
}
```

- [ ] **Step 2: 运行测试确认缺文件失败**

Run: `rtk cc -I. main/tests/music_player_theme_test.c main/src/v9_music_player/music_player_theme.c -o /tmp/music_player_theme_test`
Expected: FAIL，提示头文件或实现文件不存在。

- [ ] **Step 3: 写最小主题实现**

```c
typedef enum {
    MUSIC_PLAYER_THEME_LIGHT,
    MUSIC_PLAYER_THEME_DARK,
} music_player_theme_mode_t;

typedef enum {
    MUSIC_PLAYER_ACCENT_CYAN,
    MUSIC_PLAYER_ACCENT_BLUE,
    MUSIC_PLAYER_ACCENT_MINT,
    MUSIC_PLAYER_ACCENT_AUTO_1,
} music_player_theme_accent_t;
```

```c
music_player_theme_state_t music_player_theme_default_state(void)
{
    music_player_theme_state_t state = { MUSIC_PLAYER_THEME_LIGHT, MUSIC_PLAYER_ACCENT_CYAN };
    return state;
}
```

- [ ] **Step 4: 再写假数据失败测试**

```c
#include <assert.h>
#include "../src/v9_music_player/music_player_mock_data.h"

int main(void)
{
    assert(music_player_radio_count() == 3);
    assert(music_player_recent_count() == 3);
    assert(music_player_playlist_count() >= 2);
    return 0;
}
```

- [ ] **Step 5: 跑假数据测试并确认失败**

Run: `rtk cc -I. main/tests/music_player_mock_data_test.c main/src/v9_music_player/music_player_mock_data.c -o /tmp/music_player_mock_data_test`
Expected: FAIL，提示实现不存在。

- [ ] **Step 6: 写最小假数据实现并转绿**

```c
static const music_player_station_t g_radio_items[] = {
    { "Apple Pop Radio", "流行热歌" },
    { "Morning Acoustic", "轻松清晨" },
    { "City Drive FM", "通勤节奏" },
};
```

- [ ] **Step 7: 把两个测试注册到 `CMakeLists.txt`**

```cmake
file(GLOB MUSIC_PLAYER_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/main/src/v9_music_player/*.c"
)

add_executable(music_player_theme_test
    ${PROJECT_SOURCE_DIR}/main/tests/music_player_theme_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_music_player/music_player_theme.c
)
```

- [x] **Step 8: 跑测试确认全部通过**

Run: `rtk cmake -S . -B build && rtk cmake --build build --target music_player_theme_test music_player_mock_data_test && rtk ctest --test-dir build -R "music_player_(theme|mock_data)_test" --output-on-failure`
Expected: 两个测试均 PASS。

Current evidence: 2026-05-30 已运行 `rtk cmake -S . -B build && rtk cmake --build build --target music_player_theme_test music_player_mock_data_test music_player_html_mockup_test music_player_shell_contract_test music_player_shell_lvgl_test main && rtk ctest --test-dir build --output-on-failure`，全量测试为 8/8 PASS。主题测试覆盖 `Mint`、`自动一` 默认色和自定义色的 accent/hero token，并覆盖 `Cyan / Blue / Mint / Auto 1` 四个主题色卡 descriptor；LVGL 实机显示文案暂用 ASCII，以避免默认字体缺 CJK glyph。

- [ ] **Step 9: Commit**

```bash
rtk git add CMakeLists.txt main/src/v9_music_player/music_player_theme.h main/src/v9_music_player/music_player_theme.c main/src/v9_music_player/music_player_mock_data.h main/src/v9_music_player/music_player_mock_data.c main/tests/music_player_theme_test.c main/tests/music_player_mock_data_test.c
rtk git commit -m "test: add music player theme and data guards"
```

### Task 4: 搭建 LVGL 全局 shell，并用 flex/grid 固化页面骨架

**Files:**
- Create: `main/src/v9_music_player/music_player.h`
- Create: `main/src/v9_music_player/music_player_shell.h`
- Create: `main/src/v9_music_player/music_player_shell.c`

- [x] **Step 1: 先对将要修改的入口做 impact 分析**

Run: `gitnexus impact --repo lv_port_pc_vscode --target main --file_path main/src/main.c --kind Function --direction upstream`
Expected: 风险应集中在 SDL 启动入口；若出现 `HIGH` 或 `CRITICAL`，先停下汇报，再继续。

Current evidence: `main` impact 为 LOW；`npx gitnexus analyze` 后 `music_player_start` impact 为 LOW，0 direct callers，0 affected processes。

- [x] **Step 2: 写 shell 对外接口与页面枚举**

```c
typedef enum {
    MUSIC_PLAYER_PAGE_HOME,
    MUSIC_PLAYER_PAGE_RADIO,
    MUSIC_PLAYER_PAGE_LOCAL,
    MUSIC_PLAYER_PAGE_PLAYLIST,
    MUSIC_PLAYER_PAGE_SETTINGS,
} music_player_page_t;

void music_player_start(void);
```

- [x] **Step 3: 写全局根容器，确保主结构走 flex**

```c
lv_obj_t * root = lv_obj_create(NULL);
lv_obj_set_size(root, 800, 480);
lv_obj_set_flex_flow(root, LV_FLEX_FLOW_ROW);

lv_obj_t * sidebar = lv_obj_create(root);
lv_obj_set_width(sidebar, 160);

lv_obj_t * main_column = lv_obj_create(root);
lv_obj_set_flex_grow(main_column, 1);
lv_obj_set_flex_flow(main_column, LV_FLEX_FLOW_COLUMN);
```

- [x] **Step 4: 为右侧内容区和底部播放条建立二级 flex 容器**

```c
lv_obj_t * content_host = lv_obj_create(main_column);
lv_obj_set_flex_grow(content_host, 1);

lv_obj_t * mini_player = lv_obj_create(main_column);
lv_obj_set_height(mini_player, 72);
```

- [ ] **Step 5: 运行最小 SDL smoke，确认空壳能进屏**

Run: `rtk cmake --build build --target main && rtk ./bin/main`
Expected: SDL 窗口打开，尺寸仍可见；即使页面内容还是占位，也必须出现左栏、右侧空白区、底部条三段布局。

Current evidence: `rtk ./bin/main` 可运行并保持 SDL 进程；`pgrep/ps` 显示 `/Users/cys/embedded/lv_port_pc_vscode/bin/main` 存活，`lsappinfo visibleProcessList` 可见 `main`，CoreGraphics 窗口枚举显示 owner=`main`、name=`LVGL Simulator`、bounds=`1328,453,784x502`，ScreenCaptureKit 的 `SCShareableContent` 也能枚举到 owner=`main`、title=`LVGL Simulator`、frame=`1328,453,784,502`，证明真实 SDL 窗口已创建并处于可见窗口列表。中文文案曾触发 LVGL 默认字体缺 glyph 警告，已将 LVGL 实机显示文案改为 ASCII，`/tmp/music_player_sdl_smoke.log` 当前为空。`music_player_shell_lvgl_test` 在无窗口环境中创建 `800x480` LVGL display 和 shell，加载 screen 后验证 root/sidebar/content/mini-player 对象存在，并用真实 `lv_obj_send_event(..., LV_EVENT_CLICKED, ...)` 覆盖左栏 `Radio` 与底部 `Settings` 点击切页，证明点击事件会更新当前页面且 shell 结构仍稳定。测试同时通过离屏 framebuffer 非单色断言证明 LVGL 渲染产生了可见像素变化，并导出 `/tmp/music_player_lvgl_offscreen.ppm`，已转换查看 `/tmp/music_player_lvgl_offscreen.png`，画面可见左栏、Home 首屏内容和底部 mini-player。但 `screencapture -x` 得到全黑图，`screencapture -l4988` / `-R1328,453,784,502` 无法创建图像，Computer Use 与 `System Events` 均被 macOS Apple Events 权限拒绝，ScreenCaptureKit 在 `SCContentFilter(desktopIndependentWindow:)` 创建阶段触发 SkyLight `CGS_REQUIRE_INIT` 断言崩溃，因此当前证据仍不能替代真实 SDL 屏幕截图和人工验收。

- [ ] **Step 6: Commit**

```bash
rtk git add main/src/v9_music_player/music_player.h main/src/v9_music_player/music_player_shell.h main/src/v9_music_player/music_player_shell.c
rtk git commit -m "feat: add lvgl music player shell"
```

### Task 5: 实现主页/广播/本地/歌单/设置五个页面，并确保页面主结构用 grid/flex

**Files:**
- Create: `main/src/v9_music_player/music_player_home_view.c`
- Create: `main/src/v9_music_player/music_player_radio_view.c`
- Create: `main/src/v9_music_player/music_player_local_view.c`
- Create: `main/src/v9_music_player/music_player_playlist_view.c`
- Create: `main/src/v9_music_player/music_player_settings_view.c`
- Modify: `main/src/v9_music_player/music_player_shell.c`

- [x] **Step 1: 主页先用 grid 切出 hero + 双列区**

```c
static int32_t cols[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
static int32_t rows[] = { 56, 154, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };

lv_obj_t * page = lv_obj_create(parent);
lv_obj_set_layout(page, LV_LAYOUT_GRID);
lv_obj_set_grid_dsc_array(page, cols, rows);
```

- [x] **Step 2: 广播、本地、歌单页统一复用列表/卡片节奏**

```c
lv_obj_t * list = lv_obj_create(parent);
lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
lv_obj_set_style_pad_row(list, 10, 0);
```

- [x] **Step 3: 设置页用两列 grid，颜色切换只放在“外观”分组**

```c
static int32_t settings_cols[] = { 150, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
lv_obj_set_layout(page, LV_LAYOUT_GRID);
lv_obj_set_grid_dsc_array(page, settings_cols, settings_rows);
```

- [x] **Step 4: 让主题切换刷新页面内容与播放条样式**

```c
static void on_theme_accent_clicked(lv_event_t * e)
{
    app->theme.accent = (music_player_theme_accent_t)(uintptr_t)lv_event_get_user_data(e);
    music_player_shell_refresh(app);
}
```

- [ ] **Step 5: 手工验 layout 约束**

Run: `rtk ./bin/main`
Expected: 主页 hero、推荐广播、最近播放不靠绝对坐标拼版；设置页左右分栏稳定；切主题时至少主页 hero、左栏选中态、底部进度条变色。

Current evidence: 代码静态检查显示页面主结构使用 `LV_LAYOUT_GRID` / `LV_LAYOUT_FLEX`，未发现 `lv_obj_set_pos`；独立 subagent review 指出 flex/grid 主结构基本符合。`music_player_shell_contract_test` 覆盖五个左栏入口、底部独立 Settings 入口、设置页 Appearance/Playback/About 三分组，并断言这些 LVGL 显示标签是 ASCII；`music_player_theme_test` 覆盖四个主题色卡 descriptor，并断言 Auto 1 标签是 ASCII；`music_player_shell_lvgl_test` 覆盖 shell 对象树创建、五页逐一切换、常驻 mini-player、`Blue` 与 `Auto 1` 主题刷新后的结构稳定性，并新增真实 LVGL click event 断言：点击左栏 `Radio` 切到广播页，点击底部 `Settings` 切到设置页，点击设置分组 `Playback`/`Appearance` 会更新设置分组，点击 `Auto 1` 色卡会把主题切到 `MUSIC_PLAYER_ACCENT_AUTO_1` 且自定义色为 `0x8B9CF7`。缺口：当前环境未取得真实 SDL 屏幕截图或人工确认，不能把自动化点击等同于人工手工验收。

- [ ] **Step 6: Commit**

```bash
rtk git add main/src/v9_music_player/music_player_home_view.c main/src/v9_music_player/music_player_radio_view.c main/src/v9_music_player/music_player_local_view.c main/src/v9_music_player/music_player_playlist_view.c main/src/v9_music_player/music_player_settings_view.c main/src/v9_music_player/music_player_shell.c
rtk git commit -m "feat: add lvgl music player pages"
```

### Task 6: 接入 SDL 主入口和构建系统，替换 Fruit Ninja 启动路径

**Files:**
- Modify: `main/src/main.c`
- Modify: `CMakeLists.txt`

- [x] **Step 1: 对 `main()` 修改前再跑一次 impact**

Run: `gitnexus impact --repo lv_port_pc_vscode --target main --file_path main/src/main.c --kind Function --direction upstream`
Expected: 变更范围可控；若风险升级，先记录 blast radius 再继续。

Current evidence: `main` impact 为 LOW，0 direct callers，0 affected processes。

- [x] **Step 2: 把 `main.c` 入口切到 `music_player_start()`，并把分辨率改到 800x480**

```c
#include "v9_music_player/music_player.h"

int main(int argc, char **argv)
{
    lv_init();
    hal_init(800, 480);
    music_player_start();
    while(1) {
        lv_timer_handler();
        usleep(5 * 1000);
    }
}
```

- [x] **Step 3: 在 `CMakeLists.txt` 中挂上新源码 glob**

```cmake
file(GLOB MUSIC_PLAYER_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/main/src/v9_music_player/*.c"
)
target_sources(main PRIVATE ${MUSIC_PLAYER_SOURCES})
```

- [x] **Step 4: 重新构建并确认主程序编译通过**

Run: `rtk cmake -S . -B build && rtk cmake --build build --target main`
Expected: `main` 编译通过，不再依赖 `FreeRTOS` 路径。

Current evidence: `main` 编译通过；`main` 目标只挂 `MUSIC_PLAYER_SOURCES`，不再编入 `FRUIT_NINJA_SOURCES`，也不再强制链接 `SDL2_mixer`。

- [ ] **Step 5: Commit**

```bash
rtk git add main/src/main.c CMakeLists.txt
rtk git commit -m "feat: wire music player into sdl entry"
```

### Task 7: 完整验证、记录风险，并在提交前跑变更影响检查

**Files:**
- Modify: `docs/superpowers/plans/2026-05-29-apple-music-player-ui.md`

- [x] **Step 1: 跑单元测试**

Run: `rtk ctest --test-dir build --output-on-failure`
Expected: `fruit_ninja_*` 原有测试继续 PASS，新增 `music_player_theme_test` 与 `music_player_mock_data_test` 也 PASS。

Current evidence: 2026-05-30 `rtk cmake -S . -B build && rtk cmake --build build --target music_player_theme_test music_player_mock_data_test music_player_html_mockup_test music_player_shell_contract_test music_player_shell_lvgl_test main && rtk ctest --test-dir build --output-on-failure` 结果 8/8 PASS。

- [ ] **Step 2: 跑 SDL 手工 smoke**

Run: `rtk ./bin/main`
Expected: 默认进入 `主页`；默认主题颜色为 `Cyan`（淡青色）；左栏五项可切换；`设置` 页提供 `Cyan / Blue / Mint / 自动一` 四个颜色入口，其中 `自动一` 为用户可自定义色槽；底部播放条常驻；窗口尺寸为 `800x480`。

Current evidence: 进程 smoke 可运行，`rtk ./bin/main` 运行 3 秒后终止，`/tmp/music_player_sdl_smoke.log` 为 0 行，无 glyph/Warning/Error 输出；长驻运行时 `pgrep/ps` 证明 `bin/main` 存活，`lsappinfo visibleProcessList`、CoreGraphics 窗口枚举与 ScreenCaptureKit `SCShareableContent` 均证明真实 SDL 窗口存在，窗口名为 `LVGL Simulator`，owner 为 `main`，bounds/frame 为 `1328,453,784x502`。`music_player_shell_lvgl_test` 可在 LVGL 对象层证明 shell、五页切换、主题刷新、常驻 mini-player、左栏 click 切页、设置分组 click 和 `Auto 1` 色卡 click 均能驱动对应状态变化，并通过离屏 framebuffer 非单色断言证明 LVGL 渲染路径产生可见像素变化。测试导出的 `/tmp/music_player_lvgl_offscreen.png` 已可查看 Home 首屏布局，但截图 API 在当前 macOS 权限/会话下不能产出真实 SDL 窗口图像，因此本项仍不能标记完成。

- [x] **Step 3: 提交前跑 GitNexus 变更检测**

Run: `gitnexus detect_changes --repo lv_port_pc_vscode --scope all`
Expected: 受影响范围集中在 `main/src/main.c`、`main/src/v9_music_player/*`、`main/tests/*`、`CMakeLists.txt` 和 HTML 样机目录；不应误伤 `v9-fruit_ninja`。

Current evidence: 已运行 `npx gitnexus analyze` 刷新索引；最新 `detect_changes --scope all` 风险 medium，识别到 `main`、HTML 样机 `themePresets/state`、计划文档和 GitNexus 生成的 `AGENTS.md/CLAUDE.md` 计数更新。受影响流程仍集中在 `Main -> lv_group_set_default / lv_indev_set_group / lv_indev_set_display / lv_display_set_default`，`music_player_start` impact 为 LOW。`v9-fruit_ninja` 仅保留测试目标引用，未进入 `main`。

- [x] **Step 4: 记录 HTML -> LVGL 视觉差异**

```md
- 如果 HTML hero 渐变和 LVGL 实机观感存在差异，优先调整 token，而不是重写页面结构。
- 如果某个页面只能靠绝对定位才能成立，先退回并重做成 flex/grid 容器。
- 本轮目标已由用户明确推进到“开始 LVGL v9 开发”，因此 Task 2 的 HTML 用户确认 gate 被上层目标覆盖；最终验收不能把这一步表述为已完成的人工确认，只能记录为 LVGL 开发继续推进的前提变更。
- `main` 目标应只接入 Apple Music UI 所需源码；Fruit Ninja 源码和 `SDL2_mixer` 不再作为本 UI 的构建前置依赖，旧测试目标继续保留。
```

- [ ] **Step 5: 最终提交**

```bash
rtk git add third-party/apple_music_player_mockup main/src/v9_music_player main/src/main.c main/tests CMakeLists.txt
rtk git commit -m "feat: add apple music player ui"
```

## 阶段性收口审计（2026-05-30）

已具备强证据的范围：

- `HTML` 样机文件存在，默认 `Cyan`，包含 `主页 / 广播 / 本地 / 歌单 / 设置` 五个入口与 `Cyan / Blue / Mint / 自动一` 四个主题色卡；`music_player_html_mockup_test` 覆盖默认主题、`auto-1` 色槽和 `800x480` 外壳。
- `LVGL v9` 实现已拆为主题、假数据、shell 和五个页面 view；`main` 入口已切到 `music_player_start()`，SDL 初始化尺寸为 `800x480`。
- `main` 目标只挂 `MUSIC_PLAYER_SOURCES`，不再编入 `FRUIT_NINJA_SOURCES`，也不再强制链接 `SDL2_mixer`。
- 页面主结构使用 `LV_LAYOUT_GRID` / `LV_LAYOUT_FLEX`，当前 `main/src/v9_music_player/*` 未使用 `lv_obj_set_pos`。
- `music_player_shell_lvgl_test` 已覆盖 shell 对象树、五页切换、常驻 mini-player、`Blue` / `Auto 1` 主题刷新、左栏 click 切页、设置分组 click 和 `Auto 1` 色卡 click。
- 最新完整验证命令 `rtk cmake -S . -B build && rtk cmake --build build --target music_player_theme_test music_player_mock_data_test music_player_html_mockup_test music_player_shell_contract_test music_player_shell_lvgl_test main && rtk ctest --test-dir build --output-on-failure` 为 `8/8 PASS`。
- SDL 进程 smoke 无启动期日志输出，`/tmp/music_player_sdl_smoke.log` 为 `0` 行；窗口存在性已由 `pgrep/ps`、`lsappinfo visibleProcessList`、CoreGraphics 窗口枚举和 ScreenCaptureKit `SCShareableContent` 证明。
- GitNexus `detect_changes --scope all` 最新风险为 `medium`，受影响流程集中在 `Main -> lv_group_set_default / lv_indev_set_group / lv_indev_set_display / lv_display_set_default`，无 `HIGH` / `CRITICAL`。
- 2026-05-30 follow-up 修复后，普通页面切换路径只更新左栏选中态并替换 `content_host`，不再重建 `mini_player`；导航、设置分组和主题色卡事件上下文已改为 `music_player_app_t` 实例持有，避免多实例之间互相覆盖。`music_player_shell_lvgl_test` 新增覆盖：点击切页后 `mini_player` 指针保持不变；创建两个 `music_player_app_t` 后，点击第一个实例的 nav/settings/theme 不会改变第二个实例状态。

仍不能表述为完成的范围：

- `HTML` 样机没有用户人工确认 gate；本轮进入 `LVGL` 是用户继续目标覆盖 gate，不等于样机人工认可。
- 当前 macOS 会话无法产出真实 SDL 窗口截图：`screencapture -x` 得到黑图，`screencapture -l` / `-R` 无法创建图像，Computer Use 与 `System Events` 被 Apple Events 权限拒绝，ScreenCaptureKit 在 `SCContentFilter(desktopIndependentWindow:)` 阶段触发 SkyLight `CGS_REQUIRE_INIT` 断言崩溃。
- 因此 `Task 4 Step 5`、`Task 5 Step 5`、`Task 7 Step 2` 仍只能视为“自动化与窗口存在性证据充分，但真实屏幕截图/人工交互验收未完成”。
