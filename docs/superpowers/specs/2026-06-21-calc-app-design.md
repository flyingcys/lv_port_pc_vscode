# 计算器 App 复刻设计（calc.html → LVGL v9 + 接入桌面）

- 日期：2026-06-21
- 源：`design-ui/calc/calc.html`（macOS 风计算器，紫渐变底 + 居中 360px 卡片）
- 目标：LVGL v9 原生控件复刻，接入现有大 home 桌面，点 "计算器" 图标打开，三档分辨率自适应。

## 1. 关键决策（已与用户确认）

| 维度 | 决策 |
|---|---|
| 背景 | 打开 calc 时保留桌面壁纸 + 半透明暗化遮罩 + calc 卡片居中 |
| 自适应 | 卡片按 360 宽基准绘制，用 `transform_scale` 整体缩放至各档 |
| 字体 | calc 新建 DejaVuSans 子集字体（含 `÷ × √ ² ³` 及 ASCII），不碰桌面 SimSun/Montserrat |
| 字体源 | SimSun 是微软版权字体且本地缺失；calc 无中文，改用 lvgl 仓库自带 `DejaVuSans.ttf`（已验证含全部所需码点）|
| 科学面板 | 完整实现运算（`<math.h>` 三角/对数/幂/阶乘） |
| 接入 | 通用 launcher 框架，calc 图标点击打开 |

## 2. 架构与模块边界

```
main/desktop/
  calc/
    calc.h          // calc_create(parent, screen_w, screen_h) -> lv_obj_t*；零桌面依赖
    calc.c          // UI 构建 + 计算引擎(<math.h>)
  desktop_app_launcher.h   // app 打开/关闭框架
  desktop_app_launcher.c
  desktop.c (改)    // released_cb 增加短按启动分支
  desktop_data.h/c (改) // desktop_app_t 加 launch 回调；calc 项挂 calc_app_launch
  main.c (改)       // 新增 AM_APP env，截 calc 态
  CMakeLists.txt (改) // GLOB 追加 main/desktop/calc/*.c
```

### launcher 框架（最小）

```c
// desktop_app_launcher.h
lv_obj_t * desktop_app_launcher_open(lv_obj_t *(*builder)(lv_obj_t*, int32_t, int32_t),
                                     int32_t screen_w, int32_t screen_h);
void desktop_app_launcher_close(void);
```

- `open`：在 `lv_screen_active()` 顶层建全屏遮罩容器（黑 `opa≈160`，`LV_OBJ_FLAG_CLICKABLE` 吞点击防穿透桌面），调 `builder(遮罩容器, w, h)` 创建 calc 卡片并居中。
- 遮罩容器单例：重复 `open` 时若已存在则不重开。
- calc 红点（窗口头左侧红圆点）点击 → `desktop_app_launcher_close()`（`lv_obj_delete` 遮罩容器）。

### calc_app_launch 胶水

```c
void calc_app_launch(void) {
    const desktop_metrics_t * m = desktop_metrics();
    desktop_app_launcher_open(calc_create, m->screen_w, m->screen_h);
}
```

## 3. calc UI 构成与自适应

### 基准画布（360 宽，1:1 模拟 mockup）

- **卡片根**：宽 360，高 = 内容自适应。圆角 20，bg `#1c1c1e`，pad `20/20/25/20`。
- **窗口头**：高 30。左三圆点（红 `#ff5f56` / 黄 `#ffbd2e` / 灰 `#444446`，Ø12）。红点点击 = 关闭。右展开钮（Ø32，bg `#2a2a2c`，DejaVuSans 字符 `≡` U+2261，切换展开/收起态文字不变只换颜色）。
- **显示屏**：高 100，右对齐。
  - expression 行：`#8e8e93`，`desktop_font_calc_18`（DejaVuSans 18px）。
  - current_input 行：`desktop_font_calc_48`（DejaVuSans 48px 细近似），白。
  - `#333` 下边框分隔。
  - 字号按位数自适应：`len>12`→40px，`len>8`→51px，否则 64px（对应 mockup 2.5/3.2/4rem）。
- **科学面板**：4 列 grid，gap 12，按钮高 40，圆角 12，bg `#3a3a3c`，白字，字体 `desktop_font_calc_16`。默认折叠（height 0 / opa 0）；展开切换 `lv_anim` 改 height 0↔内容高 + opa 0↔255，同步换展开钮图标 glyph。12 键：sin/cos/tan/log/ln/√x/x²/x³/(/)/x^y/n!。
- **主键盘**：4 列 grid，gap 15，按钮 Ø65 圆形，字体 `desktop_font_calc_20`。
  - 数字 `#333333` / 白字
  - 功能 `#a5a5a5` / 黑字（DEL/AC/%/+/-）
  - 运算符 `#ff9f0a` / 白字（÷ × − + =）
- **按下反馈**：`LV_STATE_PRESSED` 改色 + `transform_scale 0.92` 近似 mockup `:active scale(0.92)`。
- 5×4 布局（对应 mockup，DEL/AC 已互换）：
  - 行1：DEL AC % ÷
  - 行2：7 8 9 ×
  - 行3：4 5 6 −
  - 行4：1 2 3 +
  - 行5：+/- 0 . =

### 自适应（transform_scale 整体缩）

- 卡片按 360 基准建好，`lv_obj_update_layout` 后取实际高 `card_h`。
- `scale = min(screen_w/360, screen_h*0.95/card_h) * 256`，转 `uint32_t`。
- `lv_obj_set_style_transform_scale(root, scale, 0)` + `lv_obj_center(root)`（transform 锚点 = 对象中心，居中正确）。
- 三档预期：800×480 ≈256（1:1 或微缩）；640×480 ≈227；480×272 含科学面板展开会超高→大幅缩，仍可显示。
- **命中区对策**：`transform_scale` 不缩放命中区（见 §8）。scale < 0.6（小屏档）时，卡片内按钮改按比例 `lv_obj_set_size` 缩小，使命中区与视觉一致；大屏档保持纯 transform_scale。阈值与实际缩减比在实现阶段实测确定。

## 4. 计算引擎（移植 calc.html JS → C）

### 状态机

```c
char current_input[64];   // "0"
char previous_input[64];  // ""
char operator;            // '+','-','×','÷','^' or 0
bool should_reset_display;
bool is_result_displayed;
```

### 函数映射

| JS | C |
|---|---|
| `handleNumber(v)` | 数字/`.` 追加；`0` 或 reset 态替换；`.` 去重 |
| `handleOperator(v)` | 先算挂起运算；记 prev+op；expression = `prev op` |
| `calculate()` | 四则 + `^`(pow)；除0→"Error"；`toPrecision(12)` → `%.12g`（`%g` 自动去尾零） |
| `handleAdvanced(action)` | sin/cos/tan/log10/ln/sqrt/pow2/pow3/factorial（`<math.h>`）；`(`/`)` 追加字符；factorial 负数/非整数→"Error" |
| toggleSign | 前缀 `-` 切换 |
| percent | `/100` |
| delete | 末位删除，空→"0" |
| clear | 全复位 |

- **Error 态**：`current_input=="Error"` 时，除 AC/DEL 外按数字视为 reset 替换（与 JS 一致）。
- **字符串↔double**：`strtod` / `snprintf("%.12g")`。
- **显示更新** `update_display()`：按长度切字号，刷新 current_input + expression 两个 label。

## 5. 接入桌面

### desktop_app_t 扩展

```c
typedef struct {
    const lv_image_dsc_t * icon;
    const char * name;
    int page;
    void (*launch)(void);   // 新增，可空
} desktop_app_t;
```

calc 项：`{ &img_app_calc, "计算器", 1, calc_app_launch }`；其余 app `launch = NULL`。

### 短按识别（released_cb 新增分支）

- 现有 `touch_time_count`（PRESSING 累加，>70 进抖动）+ 拖拽换位逻辑保留。
- 新增标志 `did_drag`：PRESSING 发生位移拖拽时置 true；released 时复位。
- released 末尾若 `!did_drag && !icon_shake` → 查 `desktop_apps` 找当前 meta 对应 app → `launch` 非 NULL 则调用。
- 不破坏现有拖拽换位流程（那些分支提前 `return`）。

## 6. 错误处理

- calc 计算溢出/无效 → 显示 "Error"（已有路径）。
- launcher 容器创建失败 → builder 返回 NULL，launcher 不挂载。
- calc 单例：launcher 内判重，重复点击 calc 图标不重开。

## 7. 测试与验证（截图回路，按 html-to-lvgl skill）

### 真图基线
headless Chrome 渲 `calc.html` 至 800×480（去 body 渐变居中、`.calculator` 占满视口），出 PNG。

### LVGL 出图
新增 `AM_APP=calc` env：`main.c` 在 `desktop_run()` 后若 `AM_APP=calc` 直接 `calc_app_launch()`，再 `maybe_take_snapshot()` 截 calc 态（而非桌面）。
`AM_SHOT=calc.png AM_APP=calc ./main` → PPM → PNG。

### 比对
主线程 Read 两 PNG 对比；三档各截一张。

### 单元测试（`tests/`，复用现有框架）
calc 计算引擎抽成可测函数（输入字符串序列 → 输出字符串），不依赖 lvgl。测：四则、科学函数、边界（除0、阶乘负数、大数精度）。

### CMake
`CMakeLists.txt:94` 的 `DESKTOP_SOURCES` glob 追加 `"${PROJECT_SOURCE_DIR}/main/desktop/calc/*.c"`，新增文件后重 configure。

## 8. 风险与坑（来自 html-to-lvgl skill）

- **`lv_color_hex()` 进静态初始化器报错**：v9 是 inline 函数，颜色表用 `LV_COLOR_MAKE` 或 `{B,G,R}`。
- **胶囊渲染成大圆**：按钮显式 `lv_obj_set_size`。
- **中文/符号豆腐块**：calc 无中文；`÷ × √ ² ³ ^ !` 确认 SimSun 含这些码点，缺失则子集补。
- **装饰对象吞点击**：窗口头圆点、expression 容器等非交互对象去 `LV_OBJ_FLAG_CLICKABLE`。
- **截图全黑**：snapshot 前跑 200 帧 `lv_timer_handler()`；`LV_MEM_SIZE` 已扩至 2MiB（本次会话已改）。
- **grid 行轴撑高**：键盘 grid 行轴用 `LV_GRID_ALIGN_START`，避免按钮被撑大。
- **transform_scale 命中区**：v9 `transform_scale` 仅视觉缩放，**不缩放命中区**——按钮事件仍按逻辑尺寸（Ø65）判定。小屏大幅缩放后，按钮视觉变小但命中区仍 65px，相邻按钮命中区会重叠导致误触。**对策**：缩放系数较小时（如 480×272 档），改用「每档独立缩减按钮基准尺寸」而非纯 transform_scale——即 scale 仍用于整体居中缩放，但小屏档直接按比例缩小卡片内按钮 `lv_obj_set_size`，使命中区与视觉一致。实现时以 scale < 0.6 为阈值切换。详见实现阶段实测。
