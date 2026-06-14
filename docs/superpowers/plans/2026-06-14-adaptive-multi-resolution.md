# 多档固定自适应布局 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让 Apple Music LVGL UI 在 800×480 / 640×480 / 480×272 三个固定档下都干净显示,启动按分辨率选档。

**Architecture:** 新建 `am_metrics` 单一事实源(每档一张尺寸表 + 语义字体角色),启动 `am_metrics_init(w,h)` 选档;所有消费端(controller/shell/widgets/pages)读 `am_metrics()` 不再写死数字;迷你条改 FR 网格;480 档两列内容改单列堆叠 + 单独小字号位图字体。

**Tech Stack:** C99、LVGL v9、SDL、CMake、`lv_font_conv`。

**事实来源:** spec `docs/superpowers/specs/2026-06-14-adaptive-multi-resolution-design.md`。

---

## 约定
- 构建:`cmake --build build -j"$(nproc)"`(改 CMake/lv_conf 后 `cmake -S . -B build` 重配)。
- 运行(选分辨率,Task 2 后生效):`./bin/main [W H]`,默认 `800 480`。
- 截图某档某页:`SDL_VIDEODRIVER=offscreen AM_PAGE=<home|radio|local|playlist|settings> AM_THEME=mint AM_SHOT=/tmp/x.ppm ./bin/main <W> <H> && convert /tmp/x.ppm /tmp/x.png`,主线程 `Read` 比对。
- 逻辑单测:`cd build && ctest -R apple_music --output-on-failure`。
- 每个重构任务**验收 = 800 档截图与改前一致(回归)+ 640/480 档截图干净不溢出/不重叠**。
- `git status` 有 `third-party/hls_player_demo`(子模块,**永不 stage**)。提交结尾带 `Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>`。

## 共享契约:`am_metrics_t`(Task 1 定义,后续任务全部引用此字段名)
```c
typedef enum { AM_TIER_800, AM_TIER_640, AM_TIER_480, AM_TIER_COUNT } am_tier_t;
typedef struct {
    const char *id;
    int16_t sidebar_w, player_h, nav_h;
    int16_t content_pad, page_gap, panel_pad, card_radius;
    int16_t cover, item_action;
    int16_t queue_w, settings_nav_w;
    bool    stack_content;                 /* true=两列内容区改单列(480 档) */
    const lv_font_t *f_title, *f_h2, *f_metric, *f_strong, *f_body, *f_label, *f_icon;
} am_metrics_t;
const am_metrics_t *am_metrics(void);
void am_metrics_init(int hor_res, int ver_res);
```
**字段对照(消费端按此把写死数字换成 `am_metrics()->字段`):**
`sidebar_w`←164 / `player_h`←78 / `nav_h`←nav 项高42 / `content_pad`←内容区 pad20 / `page_gap`←页内 gap16 / `panel_pad`←面板 pad18 / `card_radius`←卡片圆角 / `cover`←封面44 / `item_action`←动作钮30 / `queue_w`←列表队列列244 / `settings_nav_w`←设置左栏180。
**字体角色对照(把 `&am_font_N` 换成 `m->f_*`):** `f_title`←34 / `f_h2`←24 / `f_metric`←18(及16就近) / `f_strong`←14 / `f_body`←13 / `f_label`←11(及12就近) / `f_icon`←14(图标盒/控制键)。

---

## Task 1: `am_metrics` 模块 + 选档(TDD)
**Files:** Create `main/src/v9_apple_music/am_metrics.h`, `am_metrics.c`; Test `main/tests/apple_music_metrics_test.c`; Modify `CMakeLists.txt`。

- [ ] **Step 1: 写 `am_metrics.h`** —— 即上面「共享契约」全文(含 enum、struct、两个函数声明,`#include "lvgl/lvgl.h"`,头文件保护 `AM_METRICS_H`)。

- [ ] **Step 2: 写失败测试 `main/tests/apple_music_metrics_test.c`**
```c
#include <assert.h>
#include <string.h>
#include "../src/v9_apple_music/am_metrics.h"
int main(void){
    am_metrics_init(800,480); assert(am_metrics()->sidebar_w==164); assert(!am_metrics()->stack_content);
    am_metrics_init(640,480); assert(am_metrics()->sidebar_w==140); assert(am_metrics()->player_h==78);
    am_metrics_init(480,272); assert(am_metrics()->sidebar_w==108); assert(am_metrics()->stack_content);
    am_metrics_init(1024,600); assert(strcmp(am_metrics()->id,"800x480")==0); /* ≥800 回退 800 档 */
    am_metrics_init(700,480);  assert(strcmp(am_metrics()->id,"640x480")==0); /* ≥640 回退 640 档 */
    am_metrics_init(320,240);  assert(strcmp(am_metrics()->id,"480x272")==0); /* 否则最小档 */
    return 0;
}
```

- [ ] **Step 3: 注册测试到 `CMakeLists.txt`**(在 apple_music_theme_test 块后)
```cmake
add_executable(apple_music_metrics_test
    ${PROJECT_SOURCE_DIR}/main/tests/apple_music_metrics_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9_apple_music/am_metrics.c)
target_include_directories(apple_music_metrics_test PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(apple_music_metrics_test PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(apple_music_metrics_test lvgl)
add_test(NAME apple_music_metrics_test COMMAND $<TARGET_FILE:apple_music_metrics_test>)
set_tests_properties(apple_music_metrics_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
```

- [ ] **Step 4: 构建确认失败** `cmake -S . -B build && cmake --build build --target apple_music_metrics_test -j"$(nproc)"` → 链接失败(无 am_metrics.c)。

- [ ] **Step 5: 写 `am_metrics.c`** —— 3 档表(字体**暂全部用现有 `am_font_*`**;480 字体 Task 3 再换)+ 选档:
```c
#include "am_metrics.h"
#include "am_fonts.h"
static const am_metrics_t g[AM_TIER_COUNT] = {
 [AM_TIER_800]={"800x480",164,78,42, 20,16,18,18, 44,30, 244,180, false,
    &am_font_34,&am_font_24,&am_font_18,&am_font_14,&am_font_13,&am_font_11,&am_font_14},
 [AM_TIER_640]={"640x480",140,78,42, 18,14,16,16, 40,28, 200,160, false,
    &am_font_34,&am_font_24,&am_font_18,&am_font_14,&am_font_13,&am_font_11,&am_font_14},
 [AM_TIER_480]={"480x272",108,56,34, 12,10,12,14, 34,24, 0,0, true,
    /* 暂用 800 档字体,Task 3 换成 am_font_480_* */
    &am_font_34,&am_font_24,&am_font_18,&am_font_14,&am_font_13,&am_font_11,&am_font_14},
};
static const am_metrics_t *cur = &g[AM_TIER_800];
const am_metrics_t *am_metrics(void){ return cur; }
void am_metrics_init(int w, int h){ (void)h;
    if(w>=800) cur=&g[AM_TIER_800];
    else if(w>=640) cur=&g[AM_TIER_640];
    else cur=&g[AM_TIER_480];
}
```

- [ ] **Step 6: 构建确认通过** `cmake --build build --target apple_music_metrics_test -j && ./bin/apple_music_metrics_test && echo PASS`;主 app 仍构建(`am_metrics.c` 被 glob 收入)。

- [ ] **Step 7: 提交** `git add main/src/v9_apple_music/am_metrics.* main/tests/apple_music_metrics_test.c CMakeLists.txt && git commit -m "feat(music): add am_metrics tier table + selection (TDD)"`

---

## Task 2: `main.c` 选分辨率
**Files:** Modify `main/src/main.c`。
- [ ] **Step 1:** `#include "v9_apple_music/am_metrics.h"`。
- [ ] **Step 2:** 把 `hal_init(800, 480);` 改为读取 argv(默认 800×480)并初始化 metrics:
```c
int W = (argc>=3) ? atoi(argv[1]) : 800;
int H = (argc>=3) ? atoi(argv[2]) : 480;
hal_init(W, H);
am_metrics_init(W, H);
```
(`<stdlib.h>` 已包含。原 `hal_init(800,480)` 不保留注释——直接替换。)
- [ ] **Step 3: 构建 + 冒烟**:`cmake --build build -j && SDL_VIDEODRIVER=offscreen AM_SHOT=/tmp/m.ppm ./bin/main 800 480 && convert /tmp/m.ppm /tmp/m.png`;Read 确认 800 档仍正常(此时消费端尚未读 metrics,视觉应与改前一致)。再跑 `./bin/main 480 272` 不崩(布局仍旧、但能选到 480 档)。
- [ ] **Step 4: 提交** `git add main/src/main.c && git commit -m "feat(music): select resolution tier from argv in main"`

---

## Task 3: 480 档子集字体 + 接入 480 tier
**Files:** Create `main/src/v9_apple_music/fonts/am_font_480_{10,12,15,18,22}.c`; Modify `am_fonts.h`, `lv_conf.h`(`LV_FONT_CUSTOM_DECLARE`), `am_metrics.c`(480 tier 字体指针)。
- [ ] **Step 1: 生成 5 个小字号**(复用现有字符集 `main/src/v9_apple_music/fonts/charset.txt` 与源字体)。以 22 为例,其余换 `--size`/输出名为 10/12/15/18:
```bash
CJK="$(cat main/src/v9_apple_music/fonts/charset.txt)"
npx -y lv_font_conv --no-compress --format lvgl --bpp 4 --size 22 \
  --font lvgl/demos/multilang/assets/fonts/Montserrat-SemiBold.ttf -r 0x20-0x7F \
  --font lvgl/demos/multilang/assets/fonts/NotoSansSC-Medium.otf --symbols "$CJK" \
  --font main/src/v9_apple_music/fonts/src/DejaVuSansMono.ttf --symbols "⌂◉♫≣⚙♪⌕◌▶↺" \
  --font main/src/v9_apple_music/fonts/src/NotoSansSymbols2-Regular.ttf --symbols "⏮⏭" \
  --lv-font-name am_font_480_22 -o main/src/v9_apple_music/fonts/am_font_480_22.c --force-fast-kern-format
```
(符号源字体 `DejaVuSansMono.ttf` / `NotoSansSymbols2-Regular.ttf` 已在 `main/src/v9_apple_music/fonts/src/`,原复刻已下载;`⏮⏭` 用 NotoSansSymbols2。)
- [ ] **Step 2: `am_fonts.h`** 增 `LV_FONT_DECLARE(am_font_480_10) ... _22`(5 个)。
- [ ] **Step 3: `lv_conf.h`** 的 `LV_FONT_CUSTOM_DECLARE` 续行补上这 5 个。
- [ ] **Step 4: `am_metrics.c`** 把 `AM_TIER_480` 的 7 个字体指针改为:`f_title=&am_font_480_22, f_h2=&am_font_480_18, f_metric=&am_font_480_15, f_strong=&am_font_480_12, f_body=&am_font_480_12, f_label=&am_font_480_10, f_icon=&am_font_480_12`。
- [ ] **Step 5: 构建** `cmake -S . -B build && cmake --build build -j` → Built target main。(字体此刻还没人按角色用,后续任务才显效。)
- [ ] **Step 6: 提交** `git add main/src/v9_apple_music/fonts/ main/src/v9_apple_music/am_fonts.h lv_conf.h main/src/v9_apple_music/am_metrics.c && git commit -m "feat(music): add 480-tier subset fonts + wire to 480 metrics"`

---

## Task 4: `am_widgets.c` 读 metrics(字体角色 + nav_h/item_action)
**Files:** Modify `main/src/v9_apple_music/am_widgets.c`(`#include "am_metrics.h"`)。
- [ ] **Step 1:** 工厂内写死的字号/尺寸改读 `am_metrics()`:`am_nav_item` 高 `42`→`m->nav_h`、图标/标签字体→`m->f_icon`/`m->f_strong`;`am_item_action` 尺寸 `30`→`m->item_action`、字体→`m->f_icon`;`am_pill` 字体→`m->f_label`;`am_section_title` 字体→`m->f_label`;`am_panel` 圆角 `22`/`am_card` 等可保留(或用 `m->card_radius` 视需要)。`am_cover` 仍由调用方传 size(传 `m->cover`)。
- [ ] **Step 2: 构建 + 三档截图**:`cmake --build build -j`;分别 `./bin/main 800 480` / `640 480` / `480 272` 截首页(组件随档变字号/高度)。**800 档须与改前一致**。
- [ ] **Step 3: 提交** `git commit -m "feat(music): am_widgets reads am_metrics (tier fonts/sizes)"`

---

## Task 5: 根 grid(`apple_music.c`)+ `am_shell.c` 读 metrics + 迷你条改 FR
**Files:** Modify `main/src/v9_apple_music/apple_music.c`, `am_shell.c`(均 `#include "am_metrics.h"`)。
- [ ] **Step 1: `apple_music.c` 根 grid 用 metrics**:原文件作用域 `static const col_dsc/row_dsc`(写死 164/78)改为**文件作用域非 const** `static int32_t col_dsc[3], row_dsc[3];`,在 `build_all()` 里按当前档填充后再 set(⚠️ `lv_obj_set_grid_dsc_array` 按**指针**引用数组,数组必须持久,**不能用 build_all 的局部数组**):
```c
const am_metrics_t *m = am_metrics();
col_dsc[0]=m->sidebar_w; col_dsc[1]=LV_GRID_FR(1); col_dsc[2]=LV_GRID_TEMPLATE_LAST;
row_dsc[0]=LV_GRID_FR(1); row_dsc[1]=m->player_h; row_dsc[2]=LV_GRID_TEMPLATE_LAST;
lv_obj_set_grid_dsc_array(s_root, col_dsc, row_dsc);
```
内容区 pad `20`→`m->content_pad`。换肤/切档重建重跑 build_all,自然生效。
- [ ] **Step 2: `am_shell.c` 侧栏**:padding/gap、品牌/listener 字体用 `am_metrics()`(`content_pad/page_gap/f_*`);侧栏内部 `LV_PCT(100)` 自适应(宽度由 Step 1 的 grid 给定)。
- [ ] **Step 3: 迷你播放条改 FR(关键)**:删掉硬编码 `235/171/205`,改用 grid 三列 FR + 内容列:
```c
static const int32_t mcols[] = {LV_GRID_FR(115), LV_GRID_CONTENT, LV_GRID_FR(100), LV_GRID_TEMPLATE_LAST};
static const int32_t mrows[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
lv_obj_set_grid_dsc_array(player, mcols, mrows);
/* now-playing 放 col0、控制键 col1(CONTENT 自适应)、progress-cluster col2;
   各 cell 用 LV_GRID_ALIGN_STRETCH 让宽度跟随;progress-bar fill 用 LV_PCT(progress_pct) */
```
now-playing/copy/progress 不再写死像素宽,改 `LV_PCT(100)` + `flex_grow`;字体用 `m->f_body/f_label/f_icon`;控制键尺寸可随 `m->player_h` 略缩(如 `player_h<70` 用更小的键)。
- [ ] **Step 4: 构建 + 三档截图**(首页,看底部迷你条):800 与改前一致;640/480 下迷你条**不溢出、进度条完整**。
- [ ] **Step 5: 提交** `git add main/src/v9_apple_music/apple_music.c main/src/v9_apple_music/am_shell.c && git commit -m "feat(music): root grid + shell read metrics, mini-player FR columns"`

---

## Task 6: `am_page_home.c` 读 metrics
**Files:** Modify `main/src/v9_apple_music/am_page_home.c`(`#include "am_metrics.h"`)。
- [ ] **Step 1:** 所有 `&am_font_N`→`m->f_*`(按字段对照);`cover 44`→`m->cover`;padding/gap→`m->content_pad/page_gap/panel_pad`;副标题 max-width 改用 `LV_PCT` 或随档;hero `min_height` 可随档(480 更矮)。home-grid 的 `[FR125,FR92]` 保留(本就响应式)。
- [ ] **Step 2: 三档截图**(`AM_PAGE=home`):800 与改前一致;640/480 不溢出、文字不再逐字换行、浮标不压标题(必要时 480 下 hero 内边距/浮标位置随档调)。
- [ ] **Step 3: 提交** `git commit -m "feat(music): home page reads metrics"`

---

## Task 7: `am_page_list.c` 读 metrics + 480 单列堆叠
**Files:** Modify `main/src/v9_apple_music/am_page_list.c`。
- [ ] **Step 1:** 字体/封面/padding 同上换 metrics;content-grid 列宽 `244`→`m->queue_w`。
- [ ] **Step 2: stack_content**:当 `am_metrics()->stack_content` 为真,content-grid 由两列 `[FR1, queue_w]` 改为**单列**(catalog 满宽在上、queue 在下);banner 的 `[FR120,FR80]` 在 480 下也可改单列(标题块在上、cardlet 在下)。用 `if(m->stack_content){ 单列 grid/flex } else { 双列 }`。
- [ ] **Step 3: 三档截图**(`AM_PAGE=radio` 顶部 + 滚动):800 一致;640 队列列变窄但完整;480 单列堆叠、可滚动看全。
- [ ] **Step 4: 提交** `git commit -m "feat(music): list page reads metrics + 480 single-column stack"`

---

## Task 8: `am_page_settings.c` 读 metrics + 480 单列堆叠
**Files:** Modify `main/src/v9_apple_music/am_page_settings.c`。
- [ ] **Step 1:** 字体/padding 换 metrics;settings-layout 左栏宽 `180`→`m->settings_nav_w`;theme-swatches 4 列在 480 下可改 2 列。
- [ ] **Step 2: stack_content**:480 下 settings-layout 由 `[180,FR1]` 两列改单列(tabs 在上、main 在下),或 tabs 改横向胶囊行。
- [ ] **Step 3: 三档截图**(`AM_PAGE=settings`,含滚动看色卡/slider):800 一致;640 紧凑但完整;480 单列、色卡 2 列、字号更小不重叠。
- [ ] **Step 4: 提交** `git commit -m "feat(music): settings page reads metrics + 480 stack"`

---

## Task 9: 三档全量验证与调表
**Files:** 按需 Modify `am_metrics.c`(调尺寸表/480 字号)及各页。
- [ ] **Step 1: 矩阵截图**:3 档 × 5 页(`./bin/main W H` + `AM_PAGE`),mint 主题;另抽查 1-2 个非 mint 主题确认换肤不受影响。主线程逐张 Read。
- [ ] **Step 2: 调表**:对照"该档应有效果"微调 `am_metrics` 起始值与 480 字号(侧栏过宽/过窄、间距、堆叠断点、字号),直到三档都干净:无溢出、无重叠、无逐字换行、迷你条进度完整。**800 档须与原 mockup 保持一致**。
- [ ] **Step 3: 回归** `cd build && ctest -R apple_music --output-on-failure`(data/theme/metrics 全绿)+ 三档 `./bin/main` 各点一遍切页/换肤/滚动。
- [ ] **Step 4: 提交** `git commit -m "fix(music): tune am_metrics across 800/640/480 tiers"`

---

## 完成标准
- [ ] `./bin/main 800 480` / `640 480` / `480 272` 三档启动均干净,无溢出/重叠/逐字换行,迷你条进度完整。
- [ ] 800 档与原 mockup 视觉一致(回归)。
- [ ] 480 档两列内容单列堆叠 + 小字号位图字体生效。
- [ ] 未知分辨率按宽就近回退到合理档。
- [ ] `ctest -R apple_music` 全绿(含新增 metrics 选档测试)。
- [ ] `music_player.c` 未改动。
