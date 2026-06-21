# icon_replace_2 — HTML 高保真复刻 + 多分辨率 + 真实图标 设计文档

- 日期：2026-06-16
- 分支：release/v9.3-replace2
- 事实来源：`design-ui/`（HTML/CSS/JS 样机，仅 800×480 档为像素级真值）
- 目标模块：`main/icon_replace_2/`
- 适用技能：`html-to-lvgl`（截图比对回路 + 多分辨率 + 字体/图标坑）

## 1. 背景与目标

`main/icon_replace_2/` 是早先按 `design-ui` 的 HTML 复刻的 LVGL v9 版本，但实际落地的是**另一套设计**（docs 里 2026-05-14 那版：锁屏首页 + 拖拽换位 + 长按摇晃编辑 + Lock/Home/Custom 三槽位顶栏），视觉上与 HTML 样机相去甚远，"效果一般"：

- 图标用 LVGL demo 自带图片循环填充，无 app 名标签、无渐变方块；
- 无控制中心 / 通知中心面板、无分页圆点、无完整状态栏（信号/电池）；
- 800×480 写死（尺寸已集中在 `icon_replace_2_layout.h`）。

本次目标：

1. **视觉向 HTML 看齐**（HTML 为唯一事实来源），补齐缺失的视觉层；
2. **保留现有桌面交互**（拖拽换位 / 长按摇晃 / 编辑模式）作为增强；
3. **多分辨率自适应**：800×480 / 640×480 / 480×272，同布局按档缩放；
4. **图标替换为真实图标**：用 `main/images/` 下 12 个真实 app 图标作为网格内容。

## 2. 已拍板决策

| 项 | 决策 |
|---|---|
| 对齐范围 | HTML 视觉为准 **+ 保留现有交互**（拖拽换位 / 长按摇晃 / 编辑模式） |
| 图标策略 | 12 个真实 PNG 圆角裁切 + 中文名，作为网格内容（弃用"渐变方块 + Phosphor 字形"画法） |
| 多分辨率 | 同布局，按档缩放（中心化 metrics 表 + 每档字号 + 语义字体角色） |
| 滑出面板 | 纳入控制中心 / 通知中心，毛玻璃用调校过的半透明深色填充近似 |
| 壁纸背景 | 移植真实壁纸图（从 HTML 的 Unsplash 取，按档降采样，上下暗化遮罩烘焙进图） |
| 锁屏首页 | 保留 page_0 锁屏（大时钟 + Lock 顶栏模式） |
| 状态/控制图标 | Phosphor 子集字体（联网拉取已验证可达：unpkg 返回 200，449KB TTF） |

## 3. 总体策略

**演进现有 `icon_replace_2`，不推倒重来**：复用其已验证骨架（`desktop_tileview` 水平翻页 + 顶栏三槽位 + 局部坐标拖拽换位），替换并新增视觉层。

**HTML 真值只在 800×480 档成立**。HTML 样机是固定 800×480 布局（不 reflow），640×480 与 480×272 是我们按 metrics 表缩放推导出来的版本，没有 HTML 真图可比；这两档的截图比对仅做"布局合理 / 不溢出 / 卡片贴合文字 / 不竖排换行"的健全性检查，800×480 档才做逐页逐面板的像素级"肉眼一致"比对。

## 4. 页面结构（保留锁屏 → 共 3 页）

```
desktop_tileview（水平翻页，保留现有滚动/吸附）
├─ page_0  锁屏      —— 大时钟居中，顶栏 Lock 模式
├─ page_1  app 第1页 —— 5×2 网格，10 个真实图标
└─ page_2  app 第2页 —— 剩余 2 个图标（默认从左上填，分布可调）
```

- **分页圆点**：仅在 app 页显示（2 个点），锁屏页隐藏/淡出（贴近 iOS 习惯）。
- **12 图标 → 中文名映射（已确认草案，实现时仍可微调）**：

| PNG 源文件 | 中文名 |
|---|---|
| Clock-iOS-512x512.png | 时钟 |
| Photos-iOS-512x512.png | 相册 |
| Calculator-iOS-512x512.png | 计算器 |
| Calculator₊-iOS-512x512.png | 计算器+ |
| 文件管理器…-iOS-512x512.png | 文件管理 |
| 网易云音乐…-iOS-512x512.png | 网易云音乐 |
| QQ音乐…-iOS-512x512.png | QQ音乐 |
| Apple Music-iOS-512x512.png | Apple Music |
| 2048…-iOS-512x512.png | 2048 |
| Block Puzzle…-iOS-512x512.png | 方块拼图 |
| Block Blast！-iOS-512x512.png | 方块爆炸 |
| Fruit Ninja®-iOS-512x512.png | 水果忍者 |

## 5. 模块划分（复用 + 新增）

每个模块单一职责、接口清晰、可独立理解与测试。

| 文件 | 状态 | 职责 / 接口边界 |
|---|---|---|
| `icon_replace_2_metrics.c/.h` | 改造自 `layout.h` | **三档 metrics 表**（尺寸/间距/圆角/字号/字体角色 + grid 描述符）。grid 描述符数组靠指针引用、不拷贝 → **必须文件级 static**。对外提供 `metrics_get(res)` 取当前档参数 |
| `icon_replace_2_theme.c/.h` | 新增 | 单套深色 **token 表**（accent `#007AFF`、面板 `rgba(20,25,40,.75)`、玻璃边 `rgba(255,255,255,.1)`、玻璃高光、文字色等）。`rgba(c,a)` → `bg_color=c; bg_opa=round(a*255)`。对外提供颜色/opa getter |
| `icon_replace_2_widgets.c/.h` | 新增 | 控件工厂（吃当前 metrics + theme）：app 瓷砖（圆角图片 + 中文名标签）、面板容器、控制中心开关钮、亮度/音量滑条、分页圆点。非交互装饰对象一律清 `LV_OBJ_FLAG_CLICKABLE\|SCROLLABLE` |
| `icon_replace_2_desktop.c/.h` | 改造 | tileview + 真实图标瓷砖 + 中文名标签；保留拖拽/换位/跨页/摇晃/编辑（局部坐标系不变） |
| `icon_replace_2_top_bar.c/.h` | 改造 | 重做为 HTML 状态栏：左槽=时钟（Montserrat），右槽=信号+WiFi+电池（Phosphor）。保留 Lock/Home 页模式切换；锁屏页系统状态仍保留 |
| `icon_replace_2_panels.c/.h` | 新增 | 控制中心（顶部下滑）/ 通知中心（底部上滑）+ 边缘滑动手势 + 毛玻璃近似。覆盖层，独立于 tileview |
| `icon_replace_2_page_config.c/.h` | 微调 | page_0=Lock，page_1/2=Home |
| `icon_replace_2_assets/`（含 `fonts/`） | 重做 | 12 图标 C 数组 + 3 档壁纸 C 数组 + 生成的子集字体 C 文件；弃用 demo 图聚合 |
| `icon_replace_2.c` | 改造 | 入口 / 整体组装；按 `AM_RES` 选 metrics 档 |

**CMake**：该模块目录改用 `file(GLOB CONFIGURE_DEPENDS "main/icon_replace_2/*.c" "main/icon_replace_2/**/*.c")` 接入，新增文件后重新 configure 即可，免手改显式列表。

## 6. metrics 表（三档初始值，截图回路中微调）

数值为起点，最终以截图比对结果为准。

| 参数 | 800×480 | 640×480 | 480×272 |
|---|---|---|---|
| 顶栏高 top_bar_h | 40 | 40 | 28 |
| 分页区高 | 24 | 24 | 16 |
| 网格 列×行 | 5×2 | 5×2 | 5×2 |
| 图标边长 icon_size | 96 | 88 | 60 |
| 图标圆角 | 20 | 20 | 14 |
| app 名字号 (label/SimSun) | 16 | 16 | 12 |
| 状态栏时钟 (clock/Montserrat) | 18 | 18 | 14 |
| 锁屏大时钟 (big-clock/Montserrat) | 64 | 64 | 40 |
| 图标字形 (glyph/Phosphor) | 20 | 20 | 14 |
| 面板高（≈85%） | 408 | 408 | 231 |

约束：列宽 = 屏宽 / 5；行高 = (屏高 − 顶栏 − 分页区) / 2；图标在单元内居中。grid 行轴用 `LV_GRID_ALIGN_START`（避免矮卡被撑高留白）。

## 7. 字体方案（语义角色 × 三档子集化）

| 角色 | 字体源 | 用途 | 800/640/480 字号 |
|---|---|---|---|
| label | SimSun（仓库 `SimSun.woff`） | app 中文名、面板标题、通知文案 | 16 / 16 / 12 |
| clock | Montserrat（仓库已编译） | 状态栏时钟、电量% | 18 / 18 / 14 |
| big-clock | Montserrat | 锁屏大时钟 | 64 / 64 / 40 |
| glyph | Phosphor-Fill / Phosphor-Bold（unpkg 拉取） | 信号/WiFi/电池 + 控制中心图标 | 20 / 20 / 14 |

- **字符集从 HTML 提取**：app 名 + 面板标题（控制中心/通知中心）+ 通知文案 + 其中的标点（如冒号、感叹号），避免漏字豆腐块。
- **Phosphor 仅子集化用到的约 12 个 PUA 码点**：cell-signal-full、wifi-high、battery-full/medium/warning、bluetooth、airplane-tilt、moon、sun、speaker-high（码点从 Phosphor 的字符映射取）。
- Montserrat 已在 `lv_conf.h` 编译 12–48pt；64pt 若缺则补编。CJK 当前仅 SimSun_16，需按三档字号补出子集。

## 8. 资源生成（一键脚本 `scripts/gen_assets.sh`）

1. **壁纸**：下载 `images.unsplash.com/photo-1579546929518-…` → `convert` 降采样到三档尺寸 + 烘焙上下暗化遮罩（对应 HTML `::before` 渐变）→ `LVGLImage.py` 转 RGB565 C 数组（省内存）。
2. **图标**：12 个 PNG → 按各档 `icon_size` `convert` 缩放 + 圆角 alpha 蒙版 → `LVGLImage.py` 转 ARGB8888 C 数组。
3. **字体**：下载 Phosphor TTF；用仓库 `SimSun.woff`；`lv_font_conv` 按字符集 + 三档字号子集化输出 C 字体文件到 `main/icon_replace_2/assets/fonts/`。

工具链已核实可用：google-chrome 148、ImageMagick convert 6.9.11、lv_font_conv（全局）、python3 + PIL 9.0.1、`LVGLImage.py`（本机现成副本）。

## 9. 截图比对回路（本技能核心）

- **真图基线**：`scripts/render_html.sh` —— headless Chrome 渲染 `design-ui`，注入 CSS 去掉 device-frame 等舞台装饰（让 `.app-shell/.device-container` 内容占满视口），支持 `?page=&panel=` 切页/开面板 → 输出 800×480 PNG。
- **LVGL 出图**：`lv_conf.h` 开 `LV_USE_SNAPSHOT=1`；`main.c` 加 `AM_SHOT=路径` 钩子（先跑 ~200 帧 `lv_timer_handler` 让布局完成 → `lv_snapshot_take(screen)` → 写 PPM → `convert` 成 PNG → 退出）；`AM_RES/AM_PAGE/AM_PANEL` 环境变量设初始态；`SDL_VIDEODRIVER=offscreen` 无头出图；`LV_MEM_SIZE` 调大到容纳 800×480×4 ≈ 1.5MB 快照缓冲。
- **比对**：主线程 `Read` 两张 PNG 并排看，**每完成一个组件/页就比一次**，差异立刻修。640×480 / 480×272 只做健全性检查（无 HTML 真图）。

## 10. 难点近似映射（HTML → LVGL）

| HTML 效果 | LVGL 近似 |
|---|---|
| 毛玻璃 `backdrop-filter: blur(25px)`（面板） | 调校过的半透明深色填充（`bg_color {20,25,40}` + `bg_opa≈190`）+ 顶部细高光描边 |
| 135° 线性渐变（锁屏/面板等仍需处） | 竖向多段渐变（调高 `LV_GRADIENT_MAX_STOPS` 到 3–4） |
| iOS 玻璃高光 / inset 内阴影 / 多层投影 | LVGL `shadow_*` + 半透明白描边近似 |
| 照片壁纸 + `::before` 暗化遮罩 | 壁纸图烘焙暗化遮罩后作背景图 |
| `scroll-snap` 水平翻页 | 复用现有 tileview 翻页（已实现） |

## 11. 交互保留与回归

保留并回归验证：水平翻页、图标拖拽跟手、同页换位、跨页转移、长按摇晃、编辑模式进/出。新增：控制中心下滑 / 通知中心上滑手势。锁屏页与编辑态下系统状态（时间/WiFi）不消失、不重建。

## 12. 验证 / 完成判据

1. 三档（800/640/480×272）各页均能编译、启动、出图、不溢出、卡片贴合文字、中文不竖排换行；
2. **800×480 逐页 + 逐面板与 HTML 真图并排比对达成"肉眼一致"**；
3. 现有交互（翻页 / 拖拽换位 / 跨页 / 摇晃 / 编辑）回归不破；
4. 三档切换通过 `AM_RES` 或入口配置可达，grid 描述符为 static 不错乱。

## 13. 收尾：回顾 html-to-lvgl 技能

实现完成后，用 `superpowers:writing-skills` 回顾本项目过程中踩到的新坑 / 新技巧（如真实 PNG 图标圆角裁切、壁纸烘焙遮罩、Phosphor 子集化码点获取、锁屏页与多分辨率叠加等），评估是否需要把这些沉淀回 `html-to-lvgl` 技能（reference 表 / 坑表 / 流程）。

## 14. 范围与非目标（YAGNI）

- **不**接真实业务数据（纯视觉复刻 + 静态假数据，时间/电量可模拟）；
- **不**实现 HTML 之外的新页面 / 新功能；
- **不**做多主题切换（HTML 单深色主题）；
- **不**追求 640×480 / 480×272 的像素级真值（无 HTML 基线，仅健全性检查）；
- **不**实现真正的高斯模糊（半透明近似即可）。

## 15. 风险与缓解

| 风险 | 缓解 |
|---|---|
| Phosphor 码点映射易错（PUA） | 从 Phosphor 包的字符映射文件/CSS 取准确码点，子集化后截图核对字形 |
| 多档 grid 描述符指针引用导致换档错乱 | 按档数组一律文件级 static |
| 快照全黑 / 未渲染 | 快照前跑足帧数；`LV_MEM_SIZE` 留够 |
| 真实 PNG 图标自带方形底色与圆角不一致 | 统一圆角 alpha 蒙版裁切到目标圆角 |
| 壁纸内存偏大（嵌入式） | RGB565 + 按档降采样；必要时退化为渐变近似 |
| HTML 仅 800×480 真值 | 明确告知：另两档为推导版，仅健全性检查 |
