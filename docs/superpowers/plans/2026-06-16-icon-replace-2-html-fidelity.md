# icon_replace_2 HTML 高保真复刻 + 多分辨率 + 真实图标 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: 用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans` 按任务逐个实现。步骤用 `- [ ]` 复选框跟踪。

**Goal:** 把 `main/icon_replace_2/` 演进为与 `design-ui` HTML 视觉一致、支持 800×480/640×480/480×272 三档自适应、用 `main/images/` 真实图标作网格内容的 LVGL v9 桌面 UI，并保留现有拖拽/摇晃/编辑交互。

**Architecture:** 复用现有 tileview + 顶栏三槽位 + 局部坐标拖拽骨架；新增中心化 metrics 表（三档 static 数组）、单套深色 theme token 表、widget 工厂、控制/通知中心面板；图标/壁纸预转 LVGL C 数组，字体（SimSun 标签 / Phosphor 图标）按三档子集化。验收靠"headless Chrome 渲 HTML 真图 + LVGL `lv_snapshot` 出图，逐页 PNG 比对"，HTML 真值仅 800×480 档，另两档做健全性检查。

**Tech Stack:** LVGL v9 (SDL 模拟器)、CMake、google-chrome(headless)、ImageMagick `convert`、`LVGLImage.py`(PNG→C 数组)、`lv_font_conv`(字体子集化)、python3+PIL。

**设计文档:** `docs/superpowers/specs/2026-06-16-icon-replace-2-html-fidelity-design.md`

---

## 约定

- 工作目录：`/home/share/samba/lvgl/lv_port_pc_vscode_v3-music`（下文路径均相对此）。
- 构建：`cmake -S . -B build && cmake --build build --target main -j`。产物 `bin/main`。
- 截图比对验收（每个 UI 任务通用）：
  1. 渲 HTML 真图：`scripts/render_html.sh <page> <panel> out_html.png`（仅 800×480 有真值）。
  2. 出 LVGL 图：`scripts/shot.sh <res> <page> <panel> out_lvgl.png`。
  3. 主线程 `Read` 两张 PNG 并排看，差异立刻修；640×480/480×272 只看"不溢出/卡片贴合/中文不竖排"。
- 提交粒度：每个 Task 末尾 commit；commit message 末尾加 `Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>`。

## 文件结构（创建/修改总览）

| 文件 | 动作 | 职责 |
|---|---|---|
| `lv_conf.h` | 改 | 开 SNAPSHOT、抬 MEM_SIZE、抬 GRADIENT_MAX_STOPS、字体声明 |
| `main/src/main.c` | 改 | AM_RES 多分辨率入口 + 快照钩子 |
| `scripts/LVGLImage.py` | 建（复制） | PNG→C 数组 |
| `scripts/render_html.sh` | 建 | HTML 真图基线 |
| `design-ui/render.html` | 建 | 去 device-frame、按 ?page/?panel 设初始态 |
| `scripts/shot.sh` | 建 | LVGL 无头出图 |
| `scripts/compare.sh` | 建 | 并排拼图辅助 |
| `scripts/gen_assets.sh` | 建 | 壁纸/图标/字体一键生成 |
| `main/icon_replace_2/icon_replace_2_metrics.h/.c` | 建（替 layout.h） | 三档 metrics 表 + getter |
| `main/icon_replace_2/icon_replace_2_theme.h/.c` | 建 | 深色 token 表 |
| `main/icon_replace_2/icon_replace_2_data.h/.c` | 建 | app/通知/控制项数据常量 |
| `main/icon_replace_2/icon_replace_2_widgets.h/.c` | 建 | 控件工厂 |
| `main/icon_replace_2/icon_replace_2_panels.h/.c` | 建 | 控制/通知中心 + 手势 |
| `main/icon_replace_2/assets/*.c` | 建（生成） | 图标/壁纸 C 数组 |
| `main/icon_replace_2/assets/fonts/*.c` | 建（生成） | SimSun/Phosphor 子集字体 |
| `main/icon_replace_2/icon_replace_2_assets.h/.c` | 改 | 资产声明（替换 demo 图） |
| `main/icon_replace_2/icon_replace_2_assets_bundle.c` | 删 | 弃用 demo 图聚合 |
| `main/icon_replace_2/icon_replace_2_desktop.c` | 改 | metrics 化 + 真实图标瓷砖 |
| `main/icon_replace_2/icon_replace_2_top_bar.c` | 改 | HTML 状态栏样式 |
| `main/icon_replace_2/icon_replace_2_page_config.c` | 改 | 页配置贴合 HTML |
| `main/icon_replace_2/icon_replace_2.c` | 改 | 组装 + 壁纸 + 面板 + 分页点 |
| `CMakeLists.txt:76-82` | 改 | GLOB 接入模块目录 |

---

# Phase 0：基础设施（截图回路 + 构建开关）

## Task 1: lv_conf 开关 + 构建冒烟

**Files:** Modify: `lv_conf.h`

- [ ] **Step 1: 开 SNAPSHOT。** 定位（`grep -n "LV_USE_SNAPSHOT" lv_conf.h`，约 1006 行）并把值改为 1：
```c
#define LV_USE_SNAPSHOT 1
```

- [ ] **Step 2: 抬高 LV_MEM_SIZE。** `lv_conf.h:71` 当前 `(1024 * 1024)`。快照 800×480×4≈1.5MB，改为 4MB：
```c
    #define LV_MEM_SIZE (4 * 1024 * 1024)     /*[bytes]*/
```

- [ ] **Step 3: 抬高渐变色段数。** `lv_conf.h:463` 改：
```c
#define LV_GRADIENT_MAX_STOPS   4
```

- [ ] **Step 4: 启用本计划要用的内置 Montserrat 字号（时钟/大时钟）。** 确认 `LV_FONT_MONTSERRAT_40` 与 `_48` 为 1（`grep -nE "LV_FONT_MONTSERRAT_(40|48)" lv_conf.h`）；若为 0 改 1。clock 用 14/18、big-clock 用 40/48（取最近内置档，最终在截图回路微调）。

- [ ] **Step 5: 构建冒烟。**
```bash
cmake -S . -B build >/dev/null && cmake --build build --target main -j 2>&1 | tail -5
```
Expected: 链接出 `bin/main`，无报错。

- [ ] **Step 6: Commit。**
```bash
git add lv_conf.h && git commit -m "chore(lvgl): 开启 SNAPSHOT、抬高 MEM_SIZE 与渐变色段数"
```

## Task 2: main.c 多分辨率入口 + 快照钩子

**Files:** Modify: `main/src/main.c`

- [ ] **Step 1: 加入头文件与分辨率解析。** 在 `#include "icon_replace_2.h"` 后加：
```c
#include <string.h>
#include "lvgl/src/others/snapshot/lv_snapshot.h"

/* 三档分辨率，由环境变量 AM_RES 选择，默认 800x480 */
static void resolve_resolution(int32_t * w, int32_t * h)
{
    const char * res = getenv("AM_RES");
    if(res && strcmp(res, "640x480") == 0)      { *w = 640; *h = 480; }
    else if(res && strcmp(res, "480x272") == 0) { *w = 480; *h = 272; }
    else                                        { *w = 800; *h = 480; }
}
```

- [ ] **Step 2: main() 用解析出的分辨率。** 把 `hal_init(800, 480);` 替换为：
```c
  int32_t scr_w, scr_h;
  resolve_resolution(&scr_w, &scr_h);
  icon_replace_2_set_resolution(scr_w, scr_h);   /* Task 5 提供 */
  hal_init(scr_w, scr_h);
```

- [ ] **Step 3: 加快照钩子函数。** 在 `STATIC FUNCTIONS` 区加（ARGB8888，跳过 alpha 写 PPM P6；按 stride 逐行取）：
```c
static void maybe_take_snapshot(void)
{
    const char * out = getenv("AM_SHOT");
    if(out == NULL) return;

    /* 跑足帧让布局/动画首帧完成 */
    for(int i = 0; i < 200; i++) { lv_timer_handler(); usleep(2 * 1000); }

    lv_draw_buf_t * snap = lv_snapshot_take(lv_screen_active(), LV_COLOR_FORMAT_ARGB8888);
    if(snap == NULL) { fprintf(stderr, "snapshot failed\n"); exit(2); }

    int32_t w = snap->header.w, h = snap->header.h;
    uint32_t stride = snap->header.stride;
    FILE * f = fopen(out, "wb");
    if(f == NULL) { exit(3); }
    fprintf(f, "P6\n%d %d\n255\n", (int)w, (int)h);
    for(int32_t y = 0; y < h; y++) {
        const uint8_t * row = snap->data + (size_t)y * stride;
        for(int32_t x = 0; x < w; x++) {
            const uint8_t * px = row + (size_t)x * 4;   /* B,G,R,A 内存序 */
            uint8_t rgb[3] = { px[2], px[1], px[0] };
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
    lv_draw_buf_destroy(snap);
    exit(0);
}
```

- [ ] **Step 4: 在事件循环前调用钩子。** 把 `icon_replace_demo_2();` 之后改为：
```c
  icon_replace_demo_2();
  maybe_take_snapshot();   /* 若 AM_SHOT 置位则出图后退出，否则继续 */

  while(1) {
    lv_timer_handler();
    usleep(5 * 1000);
  }
```

- [ ] **Step 5: 构建冒烟。**
```bash
cmake --build build --target main -j 2>&1 | tail -5
```
Expected: 通过（`icon_replace_2_set_resolution` 此刻未定义会报错——若报错，先做 Task 5 再回来；或本步暂时桩一个 weak 定义）。
> 注：Task 5 会提供 `icon_replace_2_set_resolution`。建议先做 Task 5 再编译本任务，二者强耦合。

- [ ] **Step 6: Commit。**
```bash
git add main/src/main.c && git commit -m "feat(main): AM_RES 多分辨率入口与 AM_SHOT 快照钩子"
```

## Task 3: HTML 真图基线（render.html + render_html.sh）

**Files:** Create: `design-ui/render.html`, `scripts/render_html.sh`

- [ ] **Step 1: 确认面板隐藏机制。** 读 `design-ui/style.css` 中 `.top-panel`/`.bottom-panel`（控制/通知中心）与 `.home-screen` 的定位规则，确认面板靠 `transform: translateY(±100%)` 隐藏（本任务的 render.html 用 `transform:none` 强开，与 JS 内部 class 无关）。

- [ ] **Step 2: 写 render.html。** 它复用 design-ui 的 `style.css`，去掉 device-frame 装饰让内容占满视口，并按 `?page=&panel=` 设初始态。创建 `design-ui/render.html`：
```html
<!DOCTYPE html>
<html lang="zh-CN"><head><meta charset="UTF-8">
<link rel="stylesheet" href="style.css">
<script src="https://unpkg.com/@phosphor-icons/web"></script>
<style>
  /* 去掉浏览器舞台装饰：让 app 本体占满视口 */
  html,body{margin:0;padding:0;background:#000;overflow:hidden;}
  .device-container{margin:0!important;border-radius:0!important;box-shadow:none!important;
    width:100vw!important;height:100vh!important;}
  /* 强制开/关面板（覆盖 translateY 隐藏） */
  body.show-control .top-panel{transform:none!important;}
  body.show-notify  .bottom-panel{transform:none!important;}
</style></head>
<body>
<!-- 把 design-ui/index.html 中 <body> 内的 .device-container 整块原样粘贴到此处 -->
<script>
  const q = new URLSearchParams(location.search);
  const page = parseInt(q.get('page') || '0', 10);
  const panel = q.get('panel');               /* control | notify | '' */
  window.addEventListener('load', () => {
    const hs = document.querySelector('.home-screen');
    if(hs){ hs.style.scrollBehavior='auto'; hs.scrollLeft = page * hs.clientWidth; }
    if(panel === 'control') document.body.classList.add('show-control');
    if(panel === 'notify')  document.body.classList.add('show-notify');
  });
</script>
</body></html>
```
> 实现时：把 `design-ui/index.html` 的 `<body>` 内 `.device-container` 整块 markup 粘进 `<!-- ... -->` 处（保持 class 不变）。

- [ ] **Step 3: 写 render_html.sh。** 创建 `scripts/render_html.sh` 并 `chmod +x`：
```bash
#!/usr/bin/env bash
# 用法: render_html.sh <page:0..2> <panel:none|control|notify> <out.png>
set -euo pipefail
PAGE="${1:-0}"; PANEL="${2:-none}"; OUT="${3:-out_html.png}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PARAM="page=${PAGE}"; [ "$PANEL" != "none" ] && PARAM="${PARAM}&panel=${PANEL}"
google-chrome --headless --disable-gpu --hide-scrollbars \
  --force-device-scale-factor=1 --window-size=800,480 \
  --screenshot="$OUT" \
  "file://${ROOT}/design-ui/render.html?${PARAM}" >/dev/null 2>&1
echo "wrote $OUT"
```

- [ ] **Step 4: 生成 800×480 基线并目检。**
```bash
mkdir -p /tmp/baseline
scripts/render_html.sh 0 none /tmp/baseline/p0.png
scripts/render_html.sh 1 none /tmp/baseline/p1.png
scripts/render_html.sh 0 control /tmp/baseline/ctrl.png
scripts/render_html.sh 0 notify  /tmp/baseline/notify.png
```
主线程 `Read` 这些 PNG，确认：去掉了边框/阴影、内容占满 800×480、面板能正确开合、Phosphor 图标已加载（不是豆腐块）。

- [ ] **Step 5: Commit。**
```bash
git add design-ui/render.html scripts/render_html.sh
git commit -m "test(html): headless Chrome 渲染 HTML 真图基线（去 device-frame，?page/?panel）"
```

## Task 4: LVGL 出图与比对脚本

**Files:** Create: `scripts/shot.sh`, `scripts/compare.sh`

- [ ] **Step 1: 写 shot.sh。** 创建 `scripts/shot.sh` 并 `chmod +x`（offscreen 无头 + AM_* 设初始态 + PPM→PNG）：
```bash
#!/usr/bin/env bash
# 用法: shot.sh <res:800x480|640x480|480x272> <page> <panel:none|control|notify> <out.png>
set -euo pipefail
RES="${1:-800x480}"; PAGE="${2:-0}"; PANEL="${3:-none}"; OUT="${4:-out_lvgl.png}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PPM="$(mktemp /tmp/lvgl_shot_XXXX.ppm)"
SDL_VIDEODRIVER=offscreen AM_RES="$RES" AM_PAGE="$PAGE" AM_PANEL="$PANEL" AM_SHOT="$PPM" \
  "${ROOT}/bin/main" || true
convert "$PPM" "$OUT" && rm -f "$PPM"
echo "wrote $OUT"
```

- [ ] **Step 2: 写 compare.sh（并排拼图，便于一眼看差异）。** 创建 `scripts/compare.sh` 并 `chmod +x`：
```bash
#!/usr/bin/env bash
# 用法: compare.sh <left.png> <right.png> <out.png>
set -euo pipefail
convert "$1" "$2" +append "$3"
echo "wrote $3"
```

- [ ] **Step 3: 验证出图链路（当前还是旧 demo 画面，只验证能出图）。**
```bash
scripts/shot.sh 800x480 0 none /tmp/lvgl_p0.png && ls -la /tmp/lvgl_p0.png
```
Expected: 写出非空 PNG（画面是旧 demo，无所谓）。若全黑/为空：检查 Task 1 的 SNAPSHOT/MEM_SIZE 与 Task 2 的帧数。

- [ ] **Step 4: Commit。**
```bash
git add scripts/shot.sh scripts/compare.sh
git commit -m "test(lvgl): 无头出图 shot.sh 与并排比对 compare.sh"
```

---

# Phase 1：数据 + metrics + theme tokens

## Task 5: metrics 表（替换 layout.h）

**Files:** Create: `main/icon_replace_2/icon_replace_2_metrics.h`, `icon_replace_2_metrics.c`; Modify: `icon_replace_2.h`(加 setter 声明)

- [ ] **Step 1: 写 metrics.h。** 创建 `main/icon_replace_2/icon_replace_2_metrics.h`：
```c
#ifndef ICON_REPLACE_2_METRICS_H
#define ICON_REPLACE_2_METRICS_H
#include "lvgl.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define IR2_PAGE_COUNT 3
#define IR2_GRID_COLS  5
#define IR2_GRID_ROWS  2
#define IR2_SLOT_COUNT (IR2_GRID_COLS * IR2_GRID_ROWS)   /* 10 */

typedef struct {
    int32_t screen_w, screen_h;
    int32_t top_bar_h;
    int32_t pager_h;            /* 分页点区域高 */
    int32_t icon_size;          /* app 图标边长 */
    int32_t icon_radius;
    int32_t panel_h;            /* 面板高（≈85%） */
    const lv_font_t * font_label;     /* SimSun: app 名/面板标题/通知 */
    const lv_font_t * font_clock;     /* Montserrat: 状态栏时钟 */
    const lv_font_t * font_big_clock; /* Montserrat: 锁屏大时钟 */
    const lv_font_t * font_glyph;     /* Phosphor: 状态栏/控制中心图标 */
} ir2_metrics_t;

/* 由 AM_RES（经 icon_replace_2_set_resolution）选择当前档 */
const ir2_metrics_t * ir2_metrics(void);
void icon_replace_2_set_resolution(int32_t w, int32_t h);

/* 网格坐标辅助（行轴对齐 START，单元内居中由调用方布局） */
int32_t ir2_grid_col_w(void);   /* screen_w / IR2_GRID_COLS */
int32_t ir2_grid_row_h(void);   /* (screen_h - top_bar_h - pager_h) / IR2_GRID_ROWS */

#ifdef __cplusplus
}
#endif
#endif
```

- [ ] **Step 2: 写 metrics.c。** 创建 `main/icon_replace_2/icon_replace_2_metrics.c`。字体声明用生成后的子集字体（Task 10/11 提供）；本步先用占位的内置字体，待 Task 11 回填子集字体符号名。**注意：metrics 表为文件级 static（grid 描述符按指针引用）。**
```c
#include "icon_replace_2_metrics.h"

/* Task 11 会把这些替换为生成的子集字体外部声明 */
LV_FONT_DECLARE(lv_font_simsun_16_cjk);

typedef enum { IR2_RES_800, IR2_RES_640, IR2_RES_480, IR2_RES_NUM } ir2_res_t;
static ir2_res_t s_res = IR2_RES_800;

static const ir2_metrics_t s_metrics[IR2_RES_NUM] = {
    /* 800x480 */ { 800,480, 40,24, 96,20, 408,
                    &lv_font_simsun_16_cjk, &lv_font_montserrat_18,
                    &lv_font_montserrat_48, &lv_font_simsun_16_cjk /*glyph: Task10 回填*/ },
    /* 640x480 */ { 640,480, 40,24, 88,20, 408,
                    &lv_font_simsun_16_cjk, &lv_font_montserrat_18,
                    &lv_font_montserrat_48, &lv_font_simsun_16_cjk },
    /* 480x272 */ { 480,272, 28,16, 60,14, 231,
                    &lv_font_simsun_16_cjk, &lv_font_montserrat_14,
                    &lv_font_montserrat_40, &lv_font_simsun_16_cjk },
};

void icon_replace_2_set_resolution(int32_t w, int32_t h)
{
    if(w == 640 && h == 480)      s_res = IR2_RES_640;
    else if(w == 480 && h == 272) s_res = IR2_RES_480;
    else                          s_res = IR2_RES_800;
}

const ir2_metrics_t * ir2_metrics(void) { return &s_metrics[s_res]; }
int32_t ir2_grid_col_w(void) { return ir2_metrics()->screen_w / IR2_GRID_COLS; }
int32_t ir2_grid_row_h(void) {
    const ir2_metrics_t * m = ir2_metrics();
    return (m->screen_h - m->top_bar_h - m->pager_h) / IR2_GRID_ROWS;
}
```

- [ ] **Step 3: 加 setter 声明到入口头。** 在 `main/icon_replace_2/icon_replace_2.h` 的 `void icon_replace_demo_2(void);` 前加 `#include "icon_replace_2_metrics.h"`（让 main.c 通过 icon_replace_2.h 拿到 setter 声明）。

- [ ] **Step 4: 暂不删 layout.h。** 其它 .c 仍引用旧宏，后续任务逐个迁移；本步只新增 metrics，确保两套并存可编译。

- [ ] **Step 5: 构建冒烟。**
```bash
cmake -S . -B build >/dev/null 2>&1; cmake --build build --target main -j 2>&1 | tail -5
```
> 此时 metrics.c 尚未被 CMake GLOB 接入（Task 11 改 CMake）。可临时把 metrics.c 加进 `ICON_REPLACE_2_SOURCES` 验证编译，或推迟到 Task 11 一并验证。建议：本步只 `gcc -fsyntax-only -I lvgl -I main/icon_replace_2 main/icon_replace_2/icon_replace_2_metrics.c` 做语法检查。
```bash
gcc -fsyntax-only -I. -Ilvgl -Imain/icon_replace_2 -DLV_CONF_INCLUDE_SIMPLE \
  main/icon_replace_2/icon_replace_2_metrics.c && echo SYNTAX_OK
```
Expected: `SYNTAX_OK`。

- [ ] **Step 6: Commit。**
```bash
git add main/icon_replace_2/icon_replace_2_metrics.h main/icon_replace_2/icon_replace_2_metrics.c main/icon_replace_2/icon_replace_2.h
git commit -m "feat(metrics): 三档中心化 metrics 表与分辨率选择"
```

## Task 6: theme token 表

**Files:** Create: `main/icon_replace_2/icon_replace_2_theme.h`, `icon_replace_2_theme.c`

- [ ] **Step 1: 写 theme.h。** token 取自 HTML CSS（设计文档 §第10节映射）。`rgba(c,a)` → `color` + `opa=round(a*255)`。
```c
#ifndef ICON_REPLACE_2_THEME_H
#define ICON_REPLACE_2_THEME_H
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    lv_color_t accent;        /* #007AFF */
    lv_color_t text_primary;  /* #FFFFFF */
    lv_color_t panel_bg;      /* rgba(20,25,40,.75) */
    lv_opa_t   panel_opa;     /* 191 */
    lv_color_t glass_border;  /* rgba(255,255,255,.1) */
    lv_opa_t   glass_border_opa;
    lv_color_t glass_hi;      /* 顶部高光 rgba(255,255,255,.25) */
    lv_opa_t   glass_hi_opa;
    lv_color_t overlay_dark;  /* 暗化遮罩黑 */
    lv_opa_t   overlay_opa;
} ir2_theme_t;
const ir2_theme_t * ir2_theme(void);
#ifdef __cplusplus
}
#endif
#endif
```

- [ ] **Step 2: 写 theme.c。** 单套深色主题（HTML 无多主题）：
```c
#include "icon_replace_2_theme.h"
static const ir2_theme_t s_theme = {
    .accent           = LV_COLOR_MAKE(0x00,0x7A,0xFF),
    .text_primary     = LV_COLOR_MAKE(0xFF,0xFF,0xFF),
    .panel_bg         = LV_COLOR_MAKE(0x14,0x19,0x28),
    .panel_opa        = 191,     /* 0.75*255 */
    .glass_border     = LV_COLOR_MAKE(0xFF,0xFF,0xFF),
    .glass_border_opa = 26,      /* 0.1*255 */
    .glass_hi         = LV_COLOR_MAKE(0xFF,0xFF,0xFF),
    .glass_hi_opa     = 64,      /* 0.25*255 */
    .overlay_dark     = LV_COLOR_MAKE(0x00,0x00,0x00),
    .overlay_opa      = 102,     /* 0.4*255 */
};
const ir2_theme_t * ir2_theme(void) { return &s_theme; }
```
> 坑：`lv_color_hex()` 在 v9 是 inline，不能放进 static 初始化器；static 表用 `LV_COLOR_MAKE`。

- [ ] **Step 3: 语法检查。**
```bash
gcc -fsyntax-only -I. -Ilvgl -Imain/icon_replace_2 -DLV_CONF_INCLUDE_SIMPLE \
  main/icon_replace_2/icon_replace_2_theme.c && echo SYNTAX_OK
```
Expected: `SYNTAX_OK`。

- [ ] **Step 4: Commit。**
```bash
git add main/icon_replace_2/icon_replace_2_theme.h main/icon_replace_2/icon_replace_2_theme.c
git commit -m "feat(theme): 深色 token 表（accent/panel/glass/overlay）"
```

## Task 7: 数据常量（app / 通知 / 控制项）

**Files:** Create: `main/icon_replace_2/icon_replace_2_data.h`, `icon_replace_2_data.c`

- [ ] **Step 1: 写 data.h。** 逐字镜像（设计文档 §4 映射 + HTML 通知文案）。`img_*` 为 Task 9 生成的图标 C 数组符号名（命名规则见 Task 9）。
```c
#ifndef ICON_REPLACE_2_DATA_H
#define ICON_REPLACE_2_DATA_H
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    const lv_image_dsc_t * icon;   /* 真实图标 C 数组 */
    const char * name;             /* 中文名 */
    uint8_t page;                  /* 1 或 2（page_0 为锁屏） */
} ir2_app_t;
typedef struct { const char * title; const char * body; const char * time; } ir2_notify_t;

extern const ir2_app_t    ir2_apps[];
extern const uint32_t     ir2_app_count;      /* 12 */
extern const ir2_notify_t ir2_notifies[];
extern const uint32_t     ir2_notify_count;
#ifdef __cplusplus
}
#endif
#endif
```

- [ ] **Step 2: 写 data.c。** 12 图标→中文名映射（设计文档表）；通知文案逐字来自 HTML（注意标点）。`img_*` 符号在 Task 9 生成的 .c 里，本步先 `LV_IMAGE_DECLARE` 引用：
```c
#include "icon_replace_2_data.h"
LV_IMAGE_DECLARE(img_app_clock);   LV_IMAGE_DECLARE(img_app_photos);
LV_IMAGE_DECLARE(img_app_calc);    LV_IMAGE_DECLARE(img_app_calc_plus);
LV_IMAGE_DECLARE(img_app_files);   LV_IMAGE_DECLARE(img_app_netease);
LV_IMAGE_DECLARE(img_app_qqmusic); LV_IMAGE_DECLARE(img_app_applemusic);
LV_IMAGE_DECLARE(img_app_2048);    LV_IMAGE_DECLARE(img_app_blockpuzzle);
LV_IMAGE_DECLARE(img_app_blockblast); LV_IMAGE_DECLARE(img_app_fruitninja);

const ir2_app_t ir2_apps[] = {
    { &img_app_clock,      "时钟",        1 },
    { &img_app_photos,     "相册",        1 },
    { &img_app_calc,       "计算器",      1 },
    { &img_app_calc_plus,  "计算器+",     1 },
    { &img_app_files,      "文件管理",    1 },
    { &img_app_netease,    "网易云音乐",  1 },
    { &img_app_qqmusic,    "QQ音乐",      1 },
    { &img_app_applemusic, "Apple Music", 1 },
    { &img_app_2048,       "2048",        1 },
    { &img_app_blockpuzzle,"方块拼图",    1 },   /* page1 第10个 */
    { &img_app_blockblast, "方块爆炸",    2 },
    { &img_app_fruitninja, "水果忍者",    2 },
};
const uint32_t ir2_app_count = sizeof(ir2_apps)/sizeof(ir2_apps[0]);

const ir2_notify_t ir2_notifies[] = {
    { "系统消息", "欢迎使用全新横屏系统UI！左右滑动切换应用，上下滑动调出面板。", "刚刚" },
    { "日程提醒", "下午 2:00 有一个产品设计评审会议。", "1小时前" },
};
const uint32_t ir2_notify_count = sizeof(ir2_notifies)/sizeof(ir2_notifies[0]);
```

- [ ] **Step 3: 语法检查（图标符号未定义，仅查结构体语法，用 `-fsyntax-only` 不链接故可过 DECLARE）。**
```bash
gcc -fsyntax-only -I. -Ilvgl -Imain/icon_replace_2 -DLV_CONF_INCLUDE_SIMPLE \
  main/icon_replace_2/icon_replace_2_data.c && echo SYNTAX_OK
```
Expected: `SYNTAX_OK`。

- [ ] **Step 4: Commit。**
```bash
git add main/icon_replace_2/icon_replace_2_data.h main/icon_replace_2/icon_replace_2_data.c
git commit -m "feat(data): app 列表/通知文案数据常量（逐字镜像 HTML）"
```

---

# Phase 2：资源生成（图标 / 壁纸 / 字体 → C 数组）

> 所有生成产物落到 `main/icon_replace_2/assets/` 与 `assets/fonts/`，由 `scripts/gen_assets.sh` 一键产出。生成脚本是确定性步骤，先把脚本写对再跑。

## Task 8: 壁纸生成（下载 + 降采样 + 烘焙遮罩 + RGB565 C 数组）

**Files:** Create: `scripts/LVGLImage.py`(复制), `scripts/gen_assets.sh`(壁纸段); Output: `main/icon_replace_2/assets/img_wallpaper_{800x480,640x480,480x272}.c`

- [ ] **Step 1: 复制 LVGLImage.py 并确认 CLI。**
```bash
mkdir -p scripts main/icon_replace_2/assets main/icon_replace_2/assets/fonts
cp /home/share/samba/openclaw/tuyaopen/src/liblvgl/v9/lvgl/scripts/LVGLImage.py scripts/
python3 scripts/LVGLImage.py --help 2>&1 | head -30
```
确认参数名（典型：`--ofmt C --cf RGB565 -o <outdir> <input.png>`；若该版本不同，以 `--help` 为准并在脚本里对齐）。

- [ ] **Step 2: 写 gen_assets.sh 壁纸段。** 创建 `scripts/gen_assets.sh`（`chmod +x`），下载壁纸→按档 `extent`→烘焙上下暗化遮罩→PNG→RGB565 C 数组：
```bash
#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/main/icon_replace_2/assets"; FONTS="$OUT/fonts"; TMP="$(mktemp -d)"
WP_URL='https://images.unsplash.com/photo-1579546929518-9e396f3cc809?q=80&w=2070&auto=format&fit=crop'

gen_wallpaper() {  # $1=W $2=H
  local W=$1 H=$2 base="$TMP/wp_${1}x${2}"
  convert "$TMP/wp_src.jpg" -resize ${W}x${H}^ -gravity center -extent ${W}x${H} "$base.png"
  # 上 25% 黑→透明(0.4)，下 20% 透明→黑(0.3)，烘焙进图
  convert -size ${W}x$((H/4)) gradient:'rgba(0,0,0,0.4)-rgba(0,0,0,0)' "$TMP/top.png"
  convert -size ${W}x$((H/5)) gradient:'rgba(0,0,0,0)-rgba(0,0,0,0.3)' "$TMP/bot.png"
  convert "$base.png" "$TMP/top.png" -gravity north -composite \
          "$TMP/bot.png" -gravity south -composite "$base.png"
  python3 "$ROOT/scripts/LVGLImage.py" --ofmt C --cf RGB565 -o "$OUT" "$base.png"
  mv "$OUT/wp_${W}x${H}.c" "$OUT/img_wallpaper_${W}x${H}.c"
}

echo "[wallpaper] downloading..."; curl -sSL "$WP_URL" -o "$TMP/wp_src.jpg"
gen_wallpaper 800 480; gen_wallpaper 640 480; gen_wallpaper 480 272
echo "[wallpaper] done"
```
> 生成的 C 数组符号名取决于文件名，故重命名为 `img_wallpaper_WxH`。实现时跑一次确认符号名（`grep lv_image_dsc_t img_wallpaper_800x480.c`），与代码引用一致。

- [ ] **Step 3: 跑壁纸生成并目检。**
```bash
scripts/gen_assets.sh   # 此刻只有壁纸段；后续任务往脚本追加图标/字体段
ls -la main/icon_replace_2/assets/img_wallpaper_*.c
```
Expected: 三个 .c 生成，含 `lv_image_dsc_t`。

- [ ] **Step 4: Commit。**
```bash
git add scripts/LVGLImage.py scripts/gen_assets.sh main/icon_replace_2/assets/img_wallpaper_*.c
git commit -m "feat(assets): 三档壁纸（降采样+烘焙暗化遮罩+RGB565 C 数组）"
```

## Task 9: 图标生成（缩放 + 圆角 alpha + ARGB8888 C 数组）

**Files:** Modify: `scripts/gen_assets.sh`(图标段); Output: `main/icon_replace_2/assets/img_app_*.c`

- [ ] **Step 1: 追加图标段到 gen_assets.sh。** 文件名→符号名映射 + 圆角蒙版。图标按 800 档 `icon_size=96`、圆角 20 生成（瓷砖在三档间用 `lv_image_set_scale` 微缩，避免 12×3 份；最终尺寸以截图回路定）：
```bash
IMG="$ROOT/main/images"
declare -A MAP=(
  ["Clock-iOS-512x512.png"]="img_app_clock"
  ["Photos-iOS-512x512.png"]="img_app_photos"
  ["Calculator-iOS-512x512.png"]="img_app_calc"
  ["Calculator₊-iOS-512x512.png"]="img_app_calc_plus"
  ["文件管理器 - 文件浏览器ZIP RAR 7Z 解压和压缩-iOS-512x512.png"]="img_app_files"
  ["网易云音乐-数亿音乐畅听-iOS-512x512.png"]="img_app_netease"
  ["QQ音乐 - 听我想听-iOS-512x512.png"]="img_app_qqmusic"
  ["Apple Music-iOS-512x512.png"]="img_app_applemusic"
  ["2048_ Number Puzzle Game-iOS-512x512.png"]="img_app_2048"
  ["Block Puzzle_ Puzzle Games-iOS-512x512.png"]="img_app_blockpuzzle"
  ["Block Blast！-iOS-512x512.png"]="img_app_blockblast"
  ["Fruit Ninja®-iOS-512x512.png"]="img_app_fruitninja"
)
ICON=96; R=20
for f in "${!MAP[@]}"; do
  sym="${MAP[$f]}"; src="$IMG/$f"
  # 缩放到 ICON，再用圆角矩形做 alpha 蒙版
  convert "$src" -resize ${ICON}x${ICON} "$TMP/$sym.png"
  convert -size ${ICON}x${ICON} xc:none -draw "roundrectangle 0,0,$((ICON-1)),$((ICON-1)),$R,$R" "$TMP/mask.png"
  convert "$TMP/$sym.png" "$TMP/mask.png" -alpha set -compose DstIn -composite "$TMP/$sym.png"
  python3 "$ROOT/scripts/LVGLImage.py" --ofmt C --cf ARGB8888 -o "$OUT" "$TMP/$sym.png"
done
echo "[icons] done"
```
> 若生成的符号名带前缀（如 `img_app_clock`），与 data.c 的 `LV_IMAGE_DECLARE` 名一致即可。确认：`grep -h "lv_image_dsc_t img_app" main/icon_replace_2/assets/img_app_*.c`。

- [ ] **Step 2: 跑图标生成并目检。**
```bash
scripts/gen_assets.sh
ls main/icon_replace_2/assets/img_app_*.c | wc -l   # 期望 12
```
主线程 `Read` 1-2 个生成的 PNG（`/tmp` 临时的）确认圆角与缩放正确。

- [ ] **Step 3: Commit。**
```bash
git add scripts/gen_assets.sh main/icon_replace_2/assets/img_app_*.c
git commit -m "feat(assets): 12 真实图标（缩放+圆角 alpha+ARGB8888 C 数组）"
```

## Task 10: 字体生成（SimSun 标签 + Phosphor 图标，按档字号）

**Files:** Modify: `scripts/gen_assets.sh`(字体段); Output: `main/icon_replace_2/assets/fonts/*.c`; Modify: `lv_conf.h`(LV_FONT_CUSTOM_DECLARE)

- [ ] **Step 1: 取 Phosphor 码点映射。** 追加到 gen_assets.sh：下载 Phosphor-Fill TTF + 其 CSS，提取所需 12 个图标的 PUA 码点：
```bash
PH_TTF="$TMP/Phosphor-Fill.ttf"; PH_CSS="$TMP/phosphor.css"
curl -sSL "https://unpkg.com/@phosphor-icons/web/src/fill/Phosphor-Fill.ttf" -o "$PH_TTF"
curl -sSL "https://unpkg.com/@phosphor-icons/web/src/fill/style.css" -o "$PH_CSS"
# 需要的图标名（状态栏 + 控制中心）
NEEDED="cell-signal-full wifi-high battery-full battery-medium battery-warning bluetooth airplane-tilt moon sun speaker-high"
# 从 CSS 抽 .ph-fill.ph-<name>:before{content:"\eXXX"} 的码点，输出逗号分隔 0xXXXX
RANGE=$(python3 - "$PH_CSS" $NEEDED <<'PY'
import re,sys
css=open(sys.argv[1],encoding='utf-8').read(); names=sys.argv[2:]; out=[]
for n in names:
    m=re.search(r'\.ph-'+re.escape(n)+r':before\s*{\s*content:\s*"\\([0-9a-fA-F]+)"',css)
    if m: out.append('0x'+m.group(1))
print(','.join(out))
PY
)
echo "phosphor range: $RANGE"
```
> 若该 CSS 选择器格式不同，用 `grep -i "cell-signal-full" "$PH_CSS"` 确认实际写法再调正则。

- [ ] **Step 2: 追加 SimSun + Phosphor 子集化。** SimSun 字符集 = HTML 全部中文 + 标点；Phosphor 用上面的 RANGE。按档字号（label 16/16/12，glyph 20/20/14）：
```bash
SIMSUN="/home/share/samba/lvgl/lv_binding_js/deps/lvgl/scripts/built_in_font/SimSun.woff"
# 从 data.c 与通知文案汇总的字符集（去重）
LABELS="时钟相册计算器+文件管理网易云音乐QQApple Music方块拼爆炸水果忍者控制中通知系统消息欢迎用全新横屏左右切换应上下调出面板。！日程提醒午有个产设审会议刚时前信号电池蓝牙飞太阳量："
gen_simsun(){ # $1=size $2=out
  lv_font_conv --font "$SIMSUN" --symbols "$LABELS" --size "$1" --bpp 4 \
    --format lvgl --no-compress --no-prefilter -o "$FONTS/$2"; }
gen_phosphor(){ # $1=size $2=out
  lv_font_conv --font "$PH_TTF" --range "$RANGE" --size "$1" --bpp 4 \
    --format lvgl --no-compress --no-prefilter -o "$FONTS/$2"; }
gen_simsun 16 ir2_simsun_16.c; gen_simsun 12 ir2_simsun_12.c
gen_phosphor 20 ir2_phosphor_20.c; gen_phosphor 14 ir2_phosphor_14.c
echo "[fonts] done"
```
> lv_font_conv 生成的字体变量名来自 `-o` 文件名（如 `ir2_simsun_16`）。确认：`grep -h "lv_font_t ir2_" $FONTS/*.c`。

- [ ] **Step 3: 跑字体生成并目检无报错。**
```bash
scripts/gen_assets.sh && ls main/icon_replace_2/assets/fonts/*.c
```
Expected: 4 个字体 .c。若 Phosphor RANGE 为空 → 回到 Step 1 修正 CSS 正则。

- [ ] **Step 4: 在 lv_conf.h 声明自定义字体。** 定位 `LV_FONT_CUSTOM_DECLARE`（约 628 行）改：
```c
#define LV_FONT_CUSTOM_DECLARE  LV_FONT_DECLARE(ir2_simsun_16) LV_FONT_DECLARE(ir2_simsun_12) \
                                LV_FONT_DECLARE(ir2_phosphor_20) LV_FONT_DECLARE(ir2_phosphor_14)
```

- [ ] **Step 5: Commit。**
```bash
git add scripts/gen_assets.sh main/icon_replace_2/assets/fonts/*.c lv_conf.h
git commit -m "feat(fonts): SimSun 标签 + Phosphor 图标 三档子集字体"
```

## Task 11: 资产聚合 + 字体回填 + CMake GLOB

**Files:** Modify: `icon_replace_2_assets.h/.c`, delete `icon_replace_2_assets_bundle.c`, modify `icon_replace_2_metrics.c`(回填字体), `CMakeLists.txt:76-91`

- [ ] **Step 1: 改 assets.h 为 Phosphor 码点别名 + 字体外部声明。** 重写 `main/icon_replace_2/icon_replace_2_assets.h`（图标已在 data.c 用 `LV_IMAGE_DECLARE`，此处集中字体与 Phosphor 码点宏，码点取自 Task 10 Step1 输出）：
```c
#ifndef ICON_REPLACE_2_ASSETS_H
#define ICON_REPLACE_2_ASSETS_H
#include "lvgl.h"
LV_FONT_DECLARE(ir2_simsun_16); LV_FONT_DECLARE(ir2_simsun_12);
LV_FONT_DECLARE(ir2_phosphor_20); LV_FONT_DECLARE(ir2_phosphor_14);
/* Phosphor 码点（UTF-8 字符串字面量），值=Task10 抽出的 \eXXXX 对应 UTF-8。
   实现时用 python: chr(0xXXXX).encode('utf-8') 转义后填入，例如： */
#define IR2_GLYPH_SIGNAL   "\xee\x88\xaf"   /* 占位：以实际码点为准 */
#define IR2_GLYPH_WIFI     "\xee\x8a\x80"
#define IR2_GLYPH_BATTERY  "\xee\x88\x9a"
#define IR2_GLYPH_BLUETOOTH "\xee\x88\xa0"
#define IR2_GLYPH_AIRPLANE "\xee\x88\x90"
#define IR2_GLYPH_MOON     "\xee\x8b\x86"
#define IR2_GLYPH_SUN      "\xee\x8d\x8e"
#define IR2_GLYPH_SPEAKER  "\xee\x8c\x9a"
#endif
```
> **去占位步骤**：跑 `python3 -c "print(repr(chr(0xXXXX).encode('utf-8')))"` 用每个图标真实码点生成 UTF-8 字节串，回填上面每个宏。码点来自 Task 10 Step1 的 `$RANGE`。

- [ ] **Step 2: 删 assets_bundle.c，清空旧 assets.c 数组。** 删除 `main/icon_replace_2/icon_replace_2_assets_bundle.c`。若 `icon_replace_2_assets.c` 不存在则跳过；旧 `icon_replace_2_assets[]`（demo 图数组）不再使用，由 data.c 的 `ir2_apps[]` 取代。

- [ ] **Step 3: metrics.c 回填真实字体。** 把 Task 5 metrics.c 顶部的 `LV_FONT_DECLARE(lv_font_simsun_16_cjk);` 改为 `#include "icon_replace_2_assets.h"`，并把 `s_metrics` 各档的 `font_label`/`font_glyph` 改为：800/640 档 `&ir2_simsun_16` / `&ir2_phosphor_20`，480 档 `&ir2_simsun_12` / `&ir2_phosphor_14`。

- [ ] **Step 4: CMake 改 GLOB。** 把 `CMakeLists.txt:76-82` 的显式列表替换为：
```cmake
file(GLOB ICON_REPLACE_2_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/main/icon_replace_2/*.c"
    "${PROJECT_SOURCE_DIR}/main/icon_replace_2/assets/*.c"
    "${PROJECT_SOURCE_DIR}/main/icon_replace_2/assets/fonts/*.c"
)
```
> `test_page_config`(84-91) 保持独立编译 page_config.c，不受影响（page_config 不依赖 metrics）。

- [ ] **Step 5: 全量构建。**
```bash
cmake -S . -B build >/dev/null && cmake --build build --target main -j 2>&1 | tail -15
```
Expected: 链接成功。若报 `img_app_*` / 字体符号未定义 → 确认 Task 8-10 已生成且在 GLOB 目录内。

- [ ] **Step 6: 出图冒烟（画面会变，但应能出图不崩）。**
```bash
scripts/shot.sh 800x480 0 none /tmp/s.png && echo OK
```

- [ ] **Step 7: Commit。**
```bash
git add -A main/icon_replace_2 CMakeLists.txt
git commit -m "feat(assets): 资产聚合切到真实图标/字体，CMake 改 GLOB，删 demo 图聚合"
```

---

# Phase 3：widget 工厂

## Task 12: app 瓷砖工厂

**Files:** Create: `main/icon_replace_2/icon_replace_2_widgets.h`, `icon_replace_2_widgets.c`

- [ ] **Step 1: 写 widgets.h（先放瓷砖 + 公共清 flag 助手）。**
```c
#ifndef ICON_REPLACE_2_WIDGETS_H
#define ICON_REPLACE_2_WIDGETS_H
#include "lvgl.h"
#include "icon_replace_2_data.h"
#ifdef __cplusplus
extern "C" {
#endif
/* 在 parent 内创建一个 app 瓷砖（图标 + 中文名），返回根容器（可点击/可拖拽） */
lv_obj_t * ir2_widget_app_tile(lv_obj_t * parent, const ir2_app_t * app);
/* 非交互装饰对象去掉点击与滚动 */
void ir2_make_decorative(lv_obj_t * obj);
#ifdef __cplusplus
}
#endif
#endif
```

- [ ] **Step 2: 写 widgets.c 的瓷砖。** 关键坑：图标用 `lv_image`，按 metrics 缩放；中文名单行省略（`LV_LABEL_LONG_DOT` 且**约束单行高**，否则窄列竖排换行）。
```c
#include "icon_replace_2_widgets.h"
#include "icon_replace_2_metrics.h"
#include "icon_replace_2_theme.h"

void ir2_make_decorative(lv_obj_t * obj){
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

lv_obj_t * ir2_widget_app_tile(lv_obj_t * parent, const ir2_app_t * app){
    const ir2_metrics_t * m = ir2_metrics();
    const ir2_theme_t * th = ir2_theme();
    int32_t col_w = ir2_grid_col_w();

    lv_obj_t * tile = lv_obj_create(parent);
    lv_obj_remove_style_all(tile);
    lv_obj_set_size(tile, col_w, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(tile, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * img = lv_image_create(tile);
    lv_image_set_src(img, app->icon);
    /* 图标 C 数组按 800 档 96px 生成；其它档用缩放贴合 m->icon_size */
    if(m->icon_size != 96) lv_image_set_scale(img, (int32_t)(256 * m->icon_size / 96));
    ir2_make_decorative(img);

    lv_obj_t * lbl = lv_label_create(tile);
    lv_label_set_text(lbl, app->name);
    lv_obj_set_style_text_font(lbl, m->font_label, 0);
    lv_obj_set_style_text_color(lbl, th->text_primary, 0);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
    lv_obj_set_width(lbl, col_w - 8);
    lv_obj_set_height(lbl, lv_font_get_line_height(m->font_label)); /* 锁单行高，防竖排 */
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    ir2_make_decorative(lbl);
    return tile;
}
```

- [ ] **Step 3: 构建。**
```bash
cmake --build build --target main -j 2>&1 | tail -5
```
Expected: 通过（widgets.c 已被 GLOB 接入）。

- [ ] **Step 4: Commit。**
```bash
git add main/icon_replace_2/icon_replace_2_widgets.* && git commit -m "feat(widgets): app 瓷砖工厂（圆角图标+单行中文名）"
```

## Task 13: 面板/胶囊/开关钮/滑条/分页圆点工厂

**Files:** Modify: `icon_replace_2_widgets.h/.c`

- [ ] **Step 1: 加工厂声明到 widgets.h。**
```c
lv_obj_t * ir2_widget_glass_panel(lv_obj_t * parent, int32_t w, int32_t h); /* 毛玻璃近似 */
lv_obj_t * ir2_widget_toggle(lv_obj_t * parent, const char * glyph, bool on);/* 控制中心圆钮 */
lv_obj_t * ir2_widget_slider(lv_obj_t * parent, const char * glyph, int32_t val);
lv_obj_t * ir2_widget_dots(lv_obj_t * parent, uint32_t count, uint32_t active);
void       ir2_widget_dots_set_active(lv_obj_t * dots, uint32_t active);
```

- [ ] **Step 2: 实现毛玻璃面板（半透明深色 + 顶部高光描边 + 大圆角）。**
```c
#include "icon_replace_2_assets.h"   /* Phosphor 码点宏 */
lv_obj_t * ir2_widget_glass_panel(lv_obj_t * parent, int32_t w, int32_t h){
    const ir2_theme_t * th = ir2_theme();
    lv_obj_t * p = lv_obj_create(parent);
    lv_obj_remove_style_all(p);
    lv_obj_set_size(p, w, h);
    lv_obj_set_style_bg_color(p, th->panel_bg, 0);
    lv_obj_set_style_bg_opa(p, th->panel_opa, 0);          /* 毛玻璃近似 */
    lv_obj_set_style_radius(p, 24, 0);
    lv_obj_set_style_border_color(p, th->glass_border, 0);
    lv_obj_set_style_border_opa(p, th->glass_border_opa, 0);
    lv_obj_set_style_border_width(p, 1, 0);
    lv_obj_set_style_shadow_width(p, 24, 0);
    lv_obj_set_style_shadow_opa(p, 80, 0);
    lv_obj_remove_flag(p, LV_OBJ_FLAG_SCROLLABLE);
    return p;
}
```

- [ ] **Step 3: 实现 toggle/slider/dots。** toggle = 圆形容器 + Phosphor 字形 label（用 `m->font_glyph`），on 态 `bg_color=accent`；slider = `lv_slider` + 左侧 glyph；dots = 行容器内 N 个小圆，active 为白色宽条（HTML `.dot.active`）。代码骨架：
```c
lv_obj_t * ir2_widget_toggle(lv_obj_t * parent, const char * glyph, bool on){
    const ir2_metrics_t * m = ir2_metrics(); const ir2_theme_t * th = ir2_theme();
    lv_obj_t * b = lv_obj_create(parent);
    lv_obj_remove_style_all(b); lv_obj_set_size(b, 56, 56);
    lv_obj_set_style_radius(b, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(b, on ? LV_OPA_COVER : 64, 0);
    lv_obj_set_style_bg_color(b, on ? th->accent : th->glass_hi, 0);
    lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE); lv_obj_remove_flag(b, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * g = lv_label_create(b);
    lv_obj_set_style_text_font(g, m->font_glyph, 0);
    lv_obj_set_style_text_color(g, th->text_primary, 0);
    lv_label_set_text(g, glyph); lv_obj_center(g); ir2_make_decorative(g);
    return b;
}
lv_obj_t * ir2_widget_dots(lv_obj_t * parent, uint32_t count, uint32_t active){
    const ir2_theme_t * th = ir2_theme();
    lv_obj_t * row = lv_obj_create(parent); lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 8, 0); ir2_make_decorative(row);
    for(uint32_t i=0;i<count;i++){
        lv_obj_t * d = lv_obj_create(row); lv_obj_remove_style_all(d);
        bool a = (i==active);
        lv_obj_set_size(d, a?18:8, 8);
        lv_obj_set_style_radius(d, 4, 0);
        lv_obj_set_style_bg_color(d, th->text_primary, 0);
        lv_obj_set_style_bg_opa(d, a?LV_OPA_COVER:96, 0);
        ir2_make_decorative(d);
    }
    return row;
}
void ir2_widget_dots_set_active(lv_obj_t * dots, uint32_t active){
    uint32_t n = lv_obj_get_child_count(dots);
    for(uint32_t i=0;i<n;i++){ lv_obj_t*d=lv_obj_get_child(dots,i); bool a=(i==active);
        lv_obj_set_width(d, a?18:8); lv_obj_set_style_bg_opa(d, a?LV_OPA_COVER:96, 0); }
}
/* slider 略：lv_slider_create + 左 glyph，indicator/knob 用 accent，留给截图回路调样式 */
```

- [ ] **Step 4: 构建。**
```bash
cmake --build build --target main -j 2>&1 | tail -5
```
Expected: 通过。

- [ ] **Step 5: Commit。**
```bash
git add main/icon_replace_2/icon_replace_2_widgets.* && git commit -m "feat(widgets): 毛玻璃面板/开关钮/滑条/分页圆点工厂"
```

---

# Phase 4：常驻外壳（壁纸背景 + 状态栏）

## Task 14: 壁纸背景层

**Files:** Modify: `main/icon_replace_2/icon_replace_2.c`

- [ ] **Step 1: 引壁纸符号 + 加背景。** 在 icon_replace_2.c 顶部 `LV_IMAGE_DECLARE` 三档壁纸；在 `icon_replace_demo_2()` 里 `lv_obj_clean(lv_screen_active())` 后、创建 desktop 前，铺一张全屏壁纸 image 作为最底层：
```c
LV_IMAGE_DECLARE(img_wallpaper_800x480);
LV_IMAGE_DECLARE(img_wallpaper_640x480);
LV_IMAGE_DECLARE(img_wallpaper_480x272);
static const lv_image_dsc_t * wallpaper_for(const ir2_metrics_t * m){
    if(m->screen_w==640) return &img_wallpaper_640x480;
    if(m->screen_w==480) return &img_wallpaper_480x272;
    return &img_wallpaper_800x480;
}
```
组装处：
```c
    const ir2_metrics_t * m = ir2_metrics();
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), 0);
    lv_obj_t * wp = lv_image_create(lv_screen_active());
    lv_image_set_src(wp, wallpaper_for(m));
    lv_obj_set_pos(wp, 0, 0);
    ir2_make_decorative(wp);   /* 需 #include "icon_replace_2_widgets.h" */
    lv_obj_move_background(wp);
```

- [ ] **Step 2: 构建 + 出图比对（壁纸）。**
```bash
cmake --build build --target main -j 2>&1 | tail -3
scripts/shot.sh 800x480 0 none /tmp/wp.png
scripts/render_html.sh 0 none /tmp/wp_html.png
scripts/compare.sh /tmp/wp.png /tmp/wp_html.png /tmp/wp_cmp.png
```
主线程 `Read /tmp/wp_cmp.png`：壁纸与暗化观感应接近 HTML。

- [ ] **Step 3: Commit。**
```bash
git add main/icon_replace_2/icon_replace_2.c && git commit -m "feat(shell): 三档壁纸背景层"
```

## Task 15: 状态栏改造（HTML 风格）

**Files:** Modify: `main/icon_replace_2/icon_replace_2_top_bar.c`

- [ ] **Step 1: 阅读现有 top_bar.c。** 确认 `create/apply/set_time/set_wifi_state` 内部如何摆 left/center/system_right 槽（约 328 行）。本任务把视觉改成 HTML 状态栏：左槽=时钟（`font_clock`），right=信号+WiFi+电池三个 Phosphor 字形 label，背景透明（叠在壁纸上），文字加细阴影。

- [ ] **Step 2: 用 metrics/theme/Phosphor 重做样式。** 关键改动（在现有 create/apply 内）：
  - 顶栏高 = `ir2_metrics()->top_bar_h`，`bg_opa = LV_OPA_TRANSP`；
  - 时钟 label 用 `m->font_clock`、`th->text_primary`，加 `lv_obj_set_style_text_opa` 与轻阴影 `text_shadow`（近似 HTML text-shadow）；
  - system_right：信号/WiFi/电池三个 label 用 `m->font_glyph` + 对应 `IR2_GLYPH_*` 码点；`set_wifi_state` 切换 `IR2_GLYPH_WIFI`（强/弱可后续细化）。
  - 旧 `lv_color_hex` 若在 static 初始化器报错 → 改 `LV_COLOR_MAKE`。

- [ ] **Step 3: 构建 + 比对状态栏。**
```bash
cmake --build build --target main -j 2>&1 | tail -3
scripts/shot.sh 800x480 1 none /tmp/bar.png
scripts/render_html.sh 1 none /tmp/bar_html.png
scripts/compare.sh /tmp/bar.png /tmp/bar_html.png /tmp/bar_cmp.png
```
`Read /tmp/bar_cmp.png`：时钟在左、信号/WiFi/电池在右，无豆腐块。

- [ ] **Step 4: Commit。**
```bash
git add main/icon_replace_2/icon_replace_2_top_bar.c && git commit -m "feat(shell): 状态栏改造为 HTML 风格（时钟+信号/WiFi/电池 Phosphor）"
```

---

# Phase 5：各页面

## Task 16: 锁屏首页（page_0）

**Files:** Modify: `icon_replace_2_page_config.c`, `icon_replace_2.c`

- [ ] **Step 1: page_config 调整。** `g_page_configs[0]` 保持 LOCK 模式；`[1]`=HOME；`[2]`=HOME（去掉 Custom，贴合"两页 app"）。把 `[2]` 的 left/center 改为 NONE、`page_mode=TOPBAR_PAGE_MODE_HOME`。

- [ ] **Step 2: 在 page_0 放大时钟。** 在 icon_replace_2.c 组装 desktop 后，往 `page[0]` 放一个居中大时钟 label（`m->font_big_clock`）：
```c
    lv_obj_t * big = lv_label_create(page[0]);
    lv_obj_set_style_text_font(big, m->font_big_clock, 0);
    lv_obj_set_style_text_color(big, ir2_theme()->text_primary, 0);
    lv_label_set_text(big, "09:41");
    lv_obj_center(big);
    ir2_make_decorative(big);
```

- [ ] **Step 3: 构建 + 出图（锁屏页无 HTML 真值，做健全性 + 三档不溢出）。**
```bash
cmake --build build --target main -j 2>&1 | tail -3
for R in 800x480 640x480 480x272; do scripts/shot.sh $R 0 none /tmp/lock_$R.png; done
```
`Read` 三张：大时钟居中、不溢出、字号随档变化。

- [ ] **Step 4: Commit。**
```bash
git add main/icon_replace_2/icon_replace_2_page_config.c main/icon_replace_2/icon_replace_2.c
git commit -m "feat(page): 锁屏首页 page_0 大时钟 + 页配置贴合两页 app"
```

## Task 17: app 网格页（page_1/2）+ 接回拖拽

**Files:** Modify: `main/icon_replace_2/icon_replace_2.c`

- [ ] **Step 1: 用 grid 摆瓷砖替换旧 image 循环。** 删除旧的 `lv_image_create + icon_replace_2_assets[...]` 填充块（icon_replace_2.c:110-125），改为按 `ir2_apps[]` 在对应页用 grid 摆 `ir2_widget_app_tile`。**坑：grid 行轴用 `LV_GRID_ALIGN_START`（防矮卡被撑高）；列模板 FR 用小数（如 `LV_GRID_FR(1)`，勿用 `LV_GRID_FR(100)`）；列描述数组必须文件级 static。**
```c
/* 文件级 static —— grid 描述符按指针引用不拷贝 */
static int32_t s_col_dsc[IR2_GRID_COLS + 1];
static int32_t s_row_dsc[IR2_GRID_ROWS + 1];
```
组装处（每个 app 页）：
```c
    for(uint32_t c=0;c<IR2_GRID_COLS;c++) s_col_dsc[c]=LV_GRID_FR(1);
    s_col_dsc[IR2_GRID_COLS]=LV_GRID_TEMPLATE_LAST;
    for(uint32_t r=0;r<IR2_GRID_ROWS;r++) s_row_dsc[r]=LV_GRID_FR(1);
    s_row_dsc[IR2_GRID_ROWS]=LV_GRID_TEMPLATE_LAST;

    for(uint32_t pg=1; pg<=2; pg++){
        lv_obj_set_grid_dsc_array(page[pg], s_col_dsc, s_row_dsc);
        lv_obj_set_layout(page[pg], LV_LAYOUT_GRID);
        lv_obj_set_style_pad_all(page[pg], 12, 0);
    }
    uint32_t slot[3] = {0,0,0};
    for(uint32_t i=0;i<ir2_app_count;i++){
        uint8_t pg = ir2_apps[i].page;            /* 1 或 2 */
        uint32_t k = slot[pg]++;                    /* 该页内序号 0..9 */
        lv_obj_t * tile = ir2_widget_app_tile(page[pg], &ir2_apps[i]);
        lv_obj_set_grid_cell(tile, LV_GRID_ALIGN_CENTER, k%IR2_GRID_COLS, 1,
                                   LV_GRID_ALIGN_START,  k/IR2_GRID_COLS, 1);
        /* 接回拖拽/摇晃事件（沿用现有 touching_cb/released_cb） */
        lv_obj_add_event_cb(tile, released_cb, LV_EVENT_RELEASED, NULL);
        lv_obj_add_event_cb(tile, touching_cb, LV_EVENT_PRESSING, NULL);
        /* icon_set_meta(...) 接入现有 icons[][] 元数据体系 */
    }
```
> 拖拽元数据（`icons[][]`、`x_by_index/y_by_index/index_by_xy`）原基于绝对坐标 + `ICON_*` 宏。本步将其改为以 grid 单元/瓷砖为单位：拖拽用 `lv_obj_set_parent` 跨页 + 释放时按落点 grid 单元换位。**保留摇晃/编辑模式动画不变（`icon_shake_cb` 等）。** 若改造量大，可分两步提交：先静态摆放出图比对，再接回拖拽。

- [ ] **Step 2: 迁移 layout.h 依赖。** 把 icon_replace_2.c 内 `ICON_MAX_COL/ICON_SLOT_COUNT/PAGE_COUNT/PAGE_WIDTH` 等宏改用 `IR2_*` 与 `ir2_metrics()`；其余仍引用 layout.h 的文件逐个迁移，最终删除 layout.h（在本任务末或单独提交）。

- [ ] **Step 3: 构建 + 三档比对。**
```bash
cmake --build build --target main -j 2>&1 | tail -3
scripts/shot.sh 800x480 1 none /tmp/g1.png; scripts/render_html.sh 1 none /tmp/g1h.png
scripts/compare.sh /tmp/g1.png /tmp/g1h.png /tmp/g1cmp.png
for R in 640x480 480x272; do scripts/shot.sh $R 1 none /tmp/g_$R.png; done
```
`Read`：800 档与 HTML 网格布局接近（真实图标替代渐变方块，可接受差异）；640/480 不溢出、中文不竖排、卡片贴合。

- [ ] **Step 4: 交互冒烟（非快照，手动起窗）。**
```bash
timeout 5s ./bin/main || true   # 确认能起、拖拽不崩（可视验收留到 Task 23）
```

- [ ] **Step 5: Commit。**
```bash
git add main/icon_replace_2/icon_replace_2.c && git commit -m "feat(page): app 网格用真实图标瓷砖+grid，接回拖拽/摇晃"
```

## Task 18: 分页圆点

**Files:** Modify: `main/icon_replace_2/icon_replace_2.c`

- [ ] **Step 1: 加底部分页点 + 随页更新 + 锁屏隐藏。** 组装末尾创建 dots（只代表 app 页，或代表 3 页——取 HTML 习惯，这里用 app 页数 2）：
```c
    lv_obj_t * dots = ir2_widget_dots(lv_screen_active(), 2, 0);
    lv_obj_align(dots, LV_ALIGN_BOTTOM_MID, 0, -8);
    /* 存到文件级 static dots 指针，desktop_page_changed_cb 里更新/隐藏 */
```
在 `desktop_page_changed_cb` 内：page==0（锁屏）`lv_obj_add_flag(dots, LV_OBJ_FLAG_HIDDEN)`；否则清隐藏并 `ir2_widget_dots_set_active(dots, page-1)`。

- [ ] **Step 2: 构建 + 出图。**
```bash
cmake --build build --target main -j 2>&1 | tail -3
scripts/shot.sh 800x480 1 none /tmp/d1.png   # app 页有点
scripts/shot.sh 800x480 0 none /tmp/d0.png   # 锁屏无点
```
`Read` 两张确认。

- [ ] **Step 3: Commit。**
```bash
git add main/icon_replace_2/icon_replace_2.c && git commit -m "feat(page): 分页圆点（app 页显示/锁屏隐藏，随页高亮）"
```

---

# Phase 6：控制器（面板 + 手势）

## Task 19: 控制中心（下滑面板）

**Files:** Create: `main/icon_replace_2/icon_replace_2_panels.h`, `icon_replace_2_panels.c`; Modify: `icon_replace_2.c`

- [ ] **Step 1: 写 panels.h。**
```c
#ifndef ICON_REPLACE_2_PANELS_H
#define ICON_REPLACE_2_PANELS_H
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct icon_replace_2_panels icon_replace_2_panels_t;
icon_replace_2_panels_t * ir2_panels_create(lv_obj_t * parent);
void ir2_panels_destroy(icon_replace_2_panels_t * p);
void ir2_panels_show_control(icon_replace_2_panels_t * p, bool show);
void ir2_panels_show_notify(icon_replace_2_panels_t * p, bool show);
/* 供快照钩子按 AM_PANEL 设初始态 */
void ir2_panels_apply_initial(icon_replace_2_panels_t * p, const char * which);
#ifdef __cplusplus
}
#endif
#endif
```

- [ ] **Step 2: 写 panels.c 控制中心。** 用 `ir2_widget_glass_panel` 做面板，内放标题"控制中心"(font_label) + 4 个 toggle（WiFi/蓝牙/飞行/夜间）+ 2 个 slider（亮度/音量）。面板初始位于屏幕上方外（`y = -panel_h`），show 时动画到 `y=0`。代码骨架（控制中心部分）：
```c
#include "icon_replace_2_panels.h"
#include "icon_replace_2_widgets.h"
#include "icon_replace_2_metrics.h"
#include "icon_replace_2_theme.h"
#include "icon_replace_2_assets.h"
struct icon_replace_2_panels { lv_obj_t * control; lv_obj_t * notify; };

static void slide_to(lv_obj_t * o, int32_t y){
    lv_anim_t a; lv_anim_init(&a); lv_anim_set_var(&a,o);
    lv_anim_set_exec_cb(&a,(lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_time(&a,250); lv_anim_set_values(&a, lv_obj_get_y(o), y); lv_anim_start(&a);
}
icon_replace_2_panels_t * ir2_panels_create(lv_obj_t * parent){
    const ir2_metrics_t * m = ir2_metrics();
    icon_replace_2_panels_t * p = lv_malloc_zeroed(sizeof(*p));
    p->control = ir2_widget_glass_panel(parent, m->screen_w, m->panel_h);
    lv_obj_set_pos(p->control, 0, -m->panel_h);   /* 顶部外 */
    /* 标题 + toggles + sliders（用 ir2_widget_toggle / ir2_widget_slider） */
    lv_obj_t * title = lv_label_create(p->control);
    lv_obj_set_style_text_font(title, m->font_label, 0);
    lv_obj_set_style_text_color(title, ir2_theme()->text_primary, 0);
    lv_label_set_text(title, "控制中心"); lv_obj_align(title, LV_ALIGN_TOP_LEFT, 16, 12);
    ir2_make_decorative(title);
    /* ...toggles: ir2_widget_toggle(p->control, IR2_GLYPH_WIFI, true) 等，flex/grid 摆放... */
    return p;
}
void ir2_panels_show_control(icon_replace_2_panels_t * p, bool show){
    const ir2_metrics_t * m = ir2_metrics();
    if(p && p->control) slide_to(p->control, show ? 0 : -m->panel_h);
}
```

- [ ] **Step 3: 在 icon_replace_2.c 创建 panels + AM_PANEL 初始态。** 组装末尾 `panels = ir2_panels_create(lv_screen_active());`；在 `maybe_take_snapshot` 之前（即 icon_replace_demo_2 末尾）按 `getenv("AM_PANEL")` 调 `ir2_panels_apply_initial`，使快照能截到开面板态。
```c
const char * which = getenv("AM_PANEL");
if(which) ir2_panels_apply_initial(panels, which);  /* "control"/"notify"/其它=none */
```
`ir2_panels_apply_initial`：control→`show_control(true)`（用 set_y 直接到 0，不走动画以免快照截到中途）。

- [ ] **Step 4: 构建 + 比对控制中心。**
```bash
cmake --build build --target main -j 2>&1 | tail -3
scripts/shot.sh 800x480 0 control /tmp/ctrl.png
scripts/render_html.sh 0 control /tmp/ctrl_html.png
scripts/compare.sh /tmp/ctrl.png /tmp/ctrl_html.png /tmp/ctrl_cmp.png
```
`Read /tmp/ctrl_cmp.png`：面板从顶部展开、毛玻璃近似、标题/开关/滑条接近 HTML。

- [ ] **Step 5: Commit。**
```bash
git add main/icon_replace_2/icon_replace_2_panels.* main/icon_replace_2/icon_replace_2.c
git commit -m "feat(panel): 控制中心（下滑面板+毛玻璃近似+开关/滑条）"
```

## Task 20: 通知中心（上滑面板）

**Files:** Modify: `icon_replace_2_panels.c`(notify 段)

- [ ] **Step 1: 加通知面板。** 同 control，但位于底部外（`y = screen_h`），内放标题"通知中心" + 按 `ir2_notifies[]` 渲染通知卡（标题 font_label、正文 font_label 小一号、时间右上）。show 到 `y = screen_h - panel_h`。
```c
/* create 内追加 */
    p->notify = ir2_widget_glass_panel(parent, m->screen_w, m->panel_h);
    lv_obj_set_pos(p->notify, 0, m->screen_h);
    lv_obj_t * nt = lv_label_create(p->notify); lv_label_set_text(nt, "通知中心");
    lv_obj_set_style_text_font(nt, m->font_label, 0);
    lv_obj_set_style_text_color(nt, ir2_theme()->text_primary, 0);
    lv_obj_align(nt, LV_ALIGN_TOP_LEFT, 16, 12); ir2_make_decorative(nt);
    for(uint32_t i=0;i<ir2_notify_count;i++){ /* 用 glass_panel 子卡或普通容器渲染 ir2_notifies[i] */ }
void ir2_panels_show_notify(icon_replace_2_panels_t * p, bool show){
    const ir2_metrics_t * m = ir2_metrics();
    if(p && p->notify) slide_to(p->notify, show ? (m->screen_h - m->panel_h) : m->screen_h);
}
```
`ir2_panels_apply_initial` 增加 `notify→show_notify(true)`（直接 set_y）。

- [ ] **Step 2: 构建 + 比对通知中心。**
```bash
cmake --build build --target main -j 2>&1 | tail -3
scripts/shot.sh 800x480 0 notify /tmp/nt.png
scripts/render_html.sh 0 notify /tmp/nt_html.png
scripts/compare.sh /tmp/nt.png /tmp/nt_html.png /tmp/nt_cmp.png
```
`Read /tmp/nt_cmp.png`：通知文案逐字一致、无漏字豆腐块。

- [ ] **Step 3: Commit。**
```bash
git add main/icon_replace_2/icon_replace_2_panels.c && git commit -m "feat(panel): 通知中心（上滑面板+通知卡）"
```

## Task 21: 边缘滑动手势接线

**Files:** Modify: `main/icon_replace_2/icon_replace_2.c`(或 panels.c)

- [ ] **Step 1: 顶/底边缘手势开合面板。** 在 screen 上加 `LV_EVENT_PRESSING`/`LV_EVENT_RELEASED` 手势检测：按下点 y < 阈值（顶部 ~20px）且下滑 → `ir2_panels_show_control(true)`；按下点 y > screen_h-阈值（底部）且上滑 → `show_notify(true)`；面板内向回滑 → 收起。沿用现有 `lv_indev_get_point` + 局部坐标式判断，避免与 tileview 水平滑动冲突（垂直位移 > 水平位移才触发）。
> 与现有桌面拖拽/翻页共存：仅当起点在屏幕上下边缘带内才进入面板手势，否则交给 tileview/图标拖拽。

- [ ] **Step 2: 构建 + 手动验收（手势靠快照难验，起窗手测）。**
```bash
cmake --build build --target main -j 2>&1 | tail -3
timeout 8s ./bin/main || true   # 手动下滑/上滑验证面板开合
```

- [ ] **Step 3: Commit。**
```bash
git add main/icon_replace_2/*.c && git commit -m "feat(gesture): 顶/底边缘下滑/上滑开合控制/通知中心"
```

---

# Phase 7：多分辨率总验 + 回归 + 技能回顾

## Task 22: 三档全页/全面板截图巡检

**Files:** Create: `scripts/snap_all.sh`

- [ ] **Step 1: 写 snap_all.sh 批量出图。** 创建 `scripts/snap_all.sh`（`chmod +x`）：
```bash
#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; OUT=/tmp/ir2_snaps; mkdir -p "$OUT"
for R in 800x480 640x480 480x272; do
  for P in 0 1 2; do "$ROOT/scripts/shot.sh" $R $P none "$OUT/lvgl_${R}_p${P}.png"; done
  "$ROOT/scripts/shot.sh" $R 0 control "$OUT/lvgl_${R}_control.png"
  "$ROOT/scripts/shot.sh" $R 0 notify  "$OUT/lvgl_${R}_notify.png"
done
for P in 0 1 2; do "$ROOT/scripts/render_html.sh" $P none "$OUT/html_p${P}.png"; done
"$ROOT/scripts/render_html.sh" 0 control "$OUT/html_control.png"
"$ROOT/scripts/render_html.sh" 0 notify  "$OUT/html_notify.png"
echo "snaps in $OUT"
```

- [ ] **Step 2: 跑全量并逐张目检。**
```bash
cmake --build build --target main -j 2>&1 | tail -3 && scripts/snap_all.sh
# 800 档逐页/逐面板与 html_* 并排比对
for k in p0 p1 p2 control notify; do
  scripts/compare.sh /tmp/ir2_snaps/lvgl_800x480_$k.png /tmp/ir2_snaps/html_$k.png /tmp/ir2_snaps/cmp_$k.png 2>/dev/null || true
done
```
主线程 `Read` 每张 `cmp_*.png`（800 档）+ 640/480 档每张：逐项核对设计文档 §12 判据，差异回到对应 Task 修。

- [ ] **Step 3: Commit。**
```bash
git add scripts/snap_all.sh && git commit -m "test: 三档全页/全面板批量截图巡检脚本"
```

## Task 23: 交互回归（可视手测）

**Files:** 无（验收任务）

- [ ] **Step 1: 起窗逐项手测。**
```bash
./bin/main    # 默认 800x480；另测 AM_RES=640x480 ./bin/main、AM_RES=480x272 ./bin/main
```
逐项确认（设计文档 §11）：水平翻页、图标拖拽跟手、同页换位、跨页转移、长按摇晃进/出编辑、控制中心下滑、通知中心上滑、锁屏页系统状态不消失。

- [ ] **Step 2: 记录结果。** 把通过/不通过逐条记到本任务下；不通过项回到对应 Task 修复（同一 subagent 内修，不新开）。

- [ ] **Step 3: Commit（若有修复）。** 修复随对应 Task 提交；本任务若纯验收无代码改动则跳过提交。

## Task 24: 回顾 html-to-lvgl 技能是否需更新

**Files:** 视结论修改 `/home/cys/.claude/skills/html-to-lvgl/SKILL.md` 及 reference

- [ ] **Step 1: 用 writing-skills 复盘。** 调用 `superpowers:writing-skills`，对照本项目实际踩的新坑/新技巧评估技能缺口，候选条目：
  - 真实 PNG 图标作网格内容（圆角 alpha 蒙版裁切、按档缩放 vs 多份生成）；
  - 壁纸烘焙暗化遮罩为单图（省去运行时遮罩层）；
  - Phosphor 子集化：从 web 包 CSS 抽 PUA 码点 → `--range` 子集化的完整流程；
  - 锁屏页/状态栏与"多分辨率同布局缩放"叠加时的页配置处理；
  - `render.html` 用 `transform:none` 强开面板（不依赖样机 JS 内部 class）。
- [ ] **Step 2: 按结论更新技能或记录"无需更新"。** 若更新，改 SKILL.md/reference 并说明改动；若评估后无需更新，在本计划/提交信息里记录结论与理由。
- [ ] **Step 3: Commit（技能仓库）。** 按 writing-skills 流程提交技能改动（注意技能在用户全局目录，非本项目仓库）。

---

## 自检（Self-Review）

**Spec 覆盖：**
- 视觉对齐 HTML → Task 12-20（瓷砖/状态栏/锁屏/面板，逐项截图比对）
- 保留现有交互 → Task 17（接回拖拽/摇晃）、Task 23（回归）
- 多分辨率同布局缩放 → Task 5（metrics 表）、Task 2（AM_RES 入口）、Task 22（三档巡检）
- 真实图标作网格内容 → Task 9（图标生成）、Task 7（映射）、Task 12/17（瓷砖+grid）
- 滑出面板+毛玻璃近似 → Task 13/19/20/21
- 壁纸移植 → Task 8/14
- 锁屏首页保留 → Task 16
- Phosphor 子集字体 → Task 10/11/15
- 截图比对回路 → Task 1-4（基础设施）+ 各 UI 任务验收
- 收尾回顾技能 → Task 24

**Placeholder 扫描：** 唯一显式占位是 `IR2_GLYPH_*` 的 Phosphor 码点（Task 11 Step1）——因码点须从联网拉取的 CSS 实测取得，已给出确切的"去占位"命令（`python3 -c "chr(0xXXXX).encode('utf-8')"`）与来源（Task 10 Step1 的 `$RANGE`），非"待办"。其余步骤均含可运行代码/命令与预期。

**类型一致性：** `ir2_metrics_t`/`ir2_theme_t`/`ir2_app_t`/`ir2_notify_t` 字段名跨任务一致；`ir2_metrics()`/`ir2_widget_*`/`ir2_panels_*` 签名前后一致；`icon_replace_2_set_resolution` 在 Task 5 定义、Task 2 调用（已注明二者强耦合、先做 Task 5）。

**已知风险与缓解：** 见设计文档 §15。最大不确定性在 Task 17（拖拽元数据从绝对坐标宏迁到 grid 单元）——已注明可拆"先静态摆放比对、再接回拖拽"两步降风险。
