# AI 语音屏幕开关 MasterGo 到 LVGL v9 实施计划

> **面向代理执行者：** 必须使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans` 子技能，按任务逐项实现本计划。步骤使用复选框（`- [ ]`）语法进行跟踪。

**目标：** 将 AI 语音屏幕开关的 3 个 MasterGo 页面转换为可维护的 LVGL v9 页面，并补齐所需的 UI 交互和面向软件功能的状态结构，为后续功能集成做准备。

**架构：** 使用固定尺寸 `480x480` 的 LVGL 应用壳层，按语义拆分为 `home`、`scenes`、`settings` 页面模块，以及独立的 `voice_popup` 覆盖层。页面级布局必须遵循 MasterGo DSL 的绝对几何值；只有在不改变最终坐标结果的前提下，重复的小模块才允许使用局部容器。

**技术栈：** LVGL v9.1、SDL 模拟器、C/CMake、通过 MCP 获取 MasterGo DSL、本地 PNG 资源、本地 TTF 字体或预生成字体兜底方案。

---

## 实施前摘要

- 设计源：MasterGo
- 需求来源：`docs/requirements/ai_voice_screen_switch_lvgl_requirements.md`
- 屏幕尺寸：`480x480`
- 当前仓库基线：
  - `main/src/main.c` 目前仍然运行 `lv_demo_widgets()`
  - `main/` 下还没有现成的应用/页面模块结构
  - `lv_conf.h` 当前配置为 `LV_USE_FS_STDIO=0`、`LV_USE_LIBPNG=0`、`LV_USE_TINY_TTF=0`、`LV_TINY_TTF_FILE_SUPPORT=0`
- MasterGo 文件：
  - `fileId=131850344368735`
- 目标页面节点：
  - 首页：`layerId=160:3454`
  - 场景页：`layerId=160:3601`
  - 设置页：`layerId=162:05153`
- 当前范围：
  - 转换 `home`、`scenes`、`settings` 3 个页面
  - 包含页面切换、顶部下拉进入设置、语音文字弹窗壳层
  - 包含开关、场景、Wi-Fi、时钟、天气、屏幕设置等面向软件功能的 UI 状态占位
  - 本阶段不实现真实云端/API/设备后端
- 布局策略：
  - 页面根层和主要视觉块使用 MasterGo DSL 的固定坐标
  - 重复子组，如开关卡片、场景项，只有在不改变最终几何结果时才允许使用局部容器
  - 截图仅用于视觉对比，DSL 才是事实来源

## 规划文件结构

- 修改：
  - `main/src/main.c`
  - `CMakeLists.txt`
  - `lv_conf.h`
- 新建：
  - `main/inc/app/app_root.h`
  - `main/src/app/app_root.c`
  - `main/inc/app/app_router.h`
  - `main/src/app/app_router.c`
  - `main/inc/model/device_state.h`
  - `main/src/model/device_state.c`
  - `main/inc/ui/home_page.h`
  - `main/src/ui/home_page.c`
  - `main/inc/ui/scenes_page.h`
  - `main/src/ui/scenes_page.c`
  - `main/inc/ui/settings_panel.h`
  - `main/src/ui/settings_panel.c`
  - `main/inc/ui/voice_popup.h`
  - `main/src/ui/voice_popup.c`
  - `main/inc/ui/top_status_bar.h`
  - `main/src/ui/top_status_bar.c`
  - `main/inc/ui/ui_fonts.h`
  - `main/src/ui/ui_fonts.c`
  - `main/inc/ui/ui_assets.h`
  - `main/src/ui/ui_assets.c`
- 新建资源目录：
  - `main/assets/home/`
  - `main/assets/scenes/`
  - `main/assets/settings/`
  - `main/assets/shared/`
  - `main/assets/fonts/`
- 新建设计参考文档：
  - `docs/design/mastergo_home_values.md`
  - `docs/design/mastergo_scenes_values.md`
  - `docs/design/mastergo_settings_values.md`
  - `docs/design/mastergo_navigation_map.md`

## 交付物

- 与 MasterGo 布局及首页软件功能相匹配的 `home_page` LVGL 页面
- 与 MasterGo 布局及场景软件功能相匹配的 `scenes_page` LVGL 页面
- 与 MasterGo 布局及屏幕设置软件功能相匹配的 `settings_panel` LVGL 面板
- 用于显示 AI 输入/输出文字的 `voice_popup` 覆盖层
- 替换 `lv_demo_widgets()` 的应用壳层与路由结构
- 已提交到仓库的本地资源与字体方案
- 可构建的模拟器应用和人工 smoke test 说明

### 任务 1：冻结设计输入并提取 DSL

**文件：**
- 新建：`docs/design/mastergo_home_values.md`
- 新建：`docs/design/mastergo_scenes_values.md`
- 新建：`docs/design/mastergo_settings_values.md`
- 新建：`docs/design/mastergo_navigation_map.md`
- 参考：`docs/requirements/ai_voice_screen_switch_lvgl_requirements.md`

- [x] 确认页面范围仍然严格限定为需求文档中的 `home`、`scenes`、`settings`，外加 `voice_popup` 覆盖层。
- [x] 分别对 `160:3454`、`160:3601` 和 `162:05153` 调用 `mcp__getDsl`。
- [x] 将每个页面提取出的关键数值写入三个 `docs/design/mastergo_*_values.md` 文件。
- [x] 对每个页面记录以下内容：
  - 根 Frame 尺寸
  - 背景资源和填充颜色
  - 顶部状态区几何信息
  - 主卡片/列表项几何信息
  - 字体家族、字号、字重和颜色
  - 圆角、透明度、渐变和阴影
  - 必须下载到本地的图片或 SVG 资源
- [x] 检查所有节点中的 `interactive` 元数据，并将跨页面导航关系写入 `docs/design/mastergo_navigation_map.md`。
- [x] 标记所有 LVGL 无法 1:1 还原的 DSL 效果，并将其归类为以下之一：
  - 原生 LVGL 实现
  - 本地光栅资源
  - 可接受的受控偏差
- [x] 在每个页面文档中增加“必须严格对齐”的章节，列出不可妥协的设计值。

**完成标准：**
- 3 个 DSL 页面均已提取
- 每个页面都有对应的数值文档
- 页面跳转关系明确
- 资源下载清单明确

### 任务 2：建立 LVGL 应用壳层，替换 Demo 入口

**文件：**
- 修改：`main/src/main.c`
- 修改：`CMakeLists.txt`
- 新建：`main/inc/app/app_root.h`
- 新建：`main/src/app/app_root.c`
- 新建：`main/inc/app/app_router.h`
- 新建：`main/src/app/app_router.c`

- [x] 创建 `main/inc/` 及其 `app`、`ui`、`model` 子目录。
- [x] 将 `main/src/main.c` 中的 `lv_demo_widgets()` 替换为 `app_root_start()` 调用。
- [x] 在 `CMakeLists.txt` 中增加新的 app/ui/model 源文件。
- [x] 定义应用层职责：
  - `app_root`：启动与顶层屏幕创建
  - `app_router`：页面切换与覆盖层显示控制
- [x] 只使用语义化命名；不要新增 `page.c`、`ui.c`、`screen.c` 之类泛化文件名。
- [x] 在头文件中定义生命周期规则：
  - create/init 路径
  - destroy/deinit 路径
  - 根对象指针所有权明确
- [x] 增加一个最小占位加载流程：
  - 创建根屏幕
  - 创建 app router
  - 加载 home 页面

**完成标准：**
- 模拟器启动后不再运行 `lv_demo_widgets()`
- 应用能进入一个空白或占位的自定义根屏幕
- 代码结构支持后续按页面逐步实现

### 任务 3：为本地资源和字体准备 LVGL 运行能力

**文件：**
- 修改：`lv_conf.h`
- 修改：`CMakeLists.txt`
- 新建：`main/inc/ui/ui_fonts.h`
- 新建：`main/src/ui/ui_fonts.c`
- 新建：`main/inc/ui/ui_assets.h`
- 新建：`main/src/ui/ui_assets.c`
- 新建：`main/assets/fonts/`
- 新建：`main/assets/shared/`

- [x] 在 `lv_conf.h` 中启用基于文件的资源加载能力。
- [x] 在 `lv_conf.h` 和 `CMakeLists.txt` 中启用最小必要的图片解码能力。
- [x] 结合实际设计字体清单，评估以下两种字体方案：
  - 模拟器优先方案：本地 `.ttf`，并启用 `LV_USE_TINY_TTF=1`、`LV_TINY_TTF_FILE_SUPPORT=1`、`LV_USE_FS_STDIO=1`
  - 后续嵌入式目标兜底方案：将子集字体预生成并以 C 数组形式提交
- [x] 在 `ui_fonts.c` 注释中记录最终采用的字体策略，并避免隐藏所有权。
- [x] 将所有页面资源下载或导出到语义化目录：
  - `main/assets/home/`
  - `main/assets/scenes/`
  - `main/assets/settings/`
  - `main/assets/shared/`
- [x] 将下载后的资源重命名为语义化 `snake_case` 名称。
- [x] 建立集中式的资源路径/字体 getter 层，避免在页面代码里散落硬编码路径。
- [x] 如果渐变或复杂效果无法原生还原，先创建局部光栅化清单，而不是把整页做成一张截图。

**完成标准：**
- 本地图片加载已启用
- 字体方案明确
- 资源目录结构稳定
- 所有下载资源均使用语义化文件名

### 任务 4：构建 3 个页面共用的 UI 基础层

**文件：**
- 新建：`main/inc/model/device_state.h`
- 新建：`main/src/model/device_state.c`
- 新建：`main/inc/ui/top_status_bar.h`
- 新建：`main/src/ui/top_status_bar.c`
- 修改：`main/src/app/app_root.c`
- 修改：`main/src/app/app_router.c`

- [x] 定义一个 `device_state` 模型，仅覆盖当前页面实际需要的字段：
  - 时钟/日期
  - 12/24 小时制
  - 天气摘要和温度
  - Wi-Fi 信号和 SSID
  - 两路开关名称和状态
  - 场景列表元数据
  - 亮度、自动熄屏、接近唤醒
  - 语音弹窗输入/输出文本
  - Mute 和音量占位状态
- [x] 创建一个共用的 `top_status_bar` 模块，用于展示：
  - 天气图标/文本
  - 温度
  - 日期/时间
  - Wi-Fi 信号强度图标
- [x] 保持共享 UI 最小化，不要强行把整个页面根层改成全局 flex 布局。
- [x] 定义 app router 状态：
  - 当前主页面索引
  - 设置面板可见/隐藏
  - 语音弹窗可见/隐藏
  - 屏幕亮/灭视觉状态
- [x] 用稳定的 mock 数据初始化应用，而不是后端调用。

**完成标准：**
- 共享状态模型已存在
- 可复用的顶部状态栏已存在
- app router 可以基于 mock 状态控制页面/覆盖层显示

### 任务 5：实现首页静态布局

**文件：**
- 新建：`main/inc/ui/home_page.h`
- 新建：`main/src/ui/home_page.c`
- 修改：`main/src/ui/ui_assets.c`
- 修改：`main/src/ui/ui_fonts.c`
- 资源：`main/assets/home/`
- 参考：`docs/design/mastergo_home_values.md`

- [x] 构建固定 `480x480` 的 home 页面根层。
- [x] 按精确 DSL 策略还原首页背景：
  - 原生填充
  - 本地光栅背景
  - 混合方案
- [x] 使用记录下来的设计值摆放时钟/日期/天气/Wi-Fi 区域。
- [x] 按精确几何和排版实现两张开关卡片。
- [x] 确保 `Switch 1` 和 `Switch 2` 名称来自 `device_state`。
- [x] 增加底部分页指示器，并保持首页选中态。
- [x] 本阶段只把开关控件绑定到 mock 状态。
- [x] 将剩余视觉差异记录到 `docs/design/mastergo_home_values.md`。

**完成标准：**
- 首页视觉结构与设计对齐
- 开关名称和状态来自本地状态模型
- 页面内容不再依赖任何 `lv_demo_*`

### 任务 6：实现场景页静态布局

**文件：**
- 新建：`main/inc/ui/scenes_page.h`
- 新建：`main/src/ui/scenes_page.c`
- 修改：`main/src/ui/ui_assets.c`
- 修改：`main/src/ui/ui_fonts.c`
- 资源：`main/assets/scenes/`
- 参考：`docs/design/mastergo_scenes_values.md`

- [x] 构建固定 `480x480` 的 scenes 页面根层。
- [x] 复用共享顶部状态数据，但不要把首页布局强行套到场景页。
- [x] 按 DSL 数值实现单个场景项的展示样式。
- [x] 即使初始视觉设计中可见项较少，也要支持最多 `8` 个场景的数据模型。
- [x] 在 DSL 提取完成后确定展示策略：
  - 分页场景块
  - 可垂直滚动的场景列表
  - 固定可见子集，后续再分页
- [x] 为场景执行弹窗行为预留成功/失败反馈钩子。
- [x] 增加底部分页指示器，并保持场景页选中态。
- [x] 将“视觉上 2 个槽位 vs 功能上支持 8 个场景”的差异记录到页面说明中，并明确最终策略。

**完成标准：**
- 场景页视觉已落地
- 页面背后有场景列表模型支撑
- 最终支持 8 个场景这一约束不会被初始布局卡死

### 任务 7：实现设置面板静态布局

**文件：**
- 新建：`main/inc/ui/settings_panel.h`
- 新建：`main/src/ui/settings_panel.c`
- 修改：`main/src/ui/ui_assets.c`
- 修改：`main/src/ui/ui_fonts.c`
- 资源：`main/assets/settings/`
- 参考：`docs/design/mastergo_settings_values.md`

- [x] 将设置页实现为覆盖层/面板模块，而不是直接替换整屏，除非 DSL 明确证明它本身就是全屏页面。
- [x] 实现可见的设置项：
  - 亮度
  - 接近亮屏
  - 自动熄屏
  - 关于 / 配网 / 重置设备入口区域
- [x] 还原设计中的顶部下拉进入形态。
- [x] 对自动熄屏使用需求文档中的精确选项文本：
  - off
  - 15s
  - 30s
  - 1min
  - 5min
  - 15min
- [x] 本阶段只把这些值绑定到 `device_state` 占位状态。
- [x] 明确并记录“关于”“配网”“重置设备”是停留在同一面板内，还是扩展为二级子视图。

**完成标准：**
- 设置面板可以作为覆盖层显示/隐藏
- 所有屏幕设置项都能用占位数据渲染
- 后续功能接线所需的交互模型已文档化

### 任务 8：增加跨页面导航和语音弹窗壳层

**文件：**
- 修改：`main/src/app/app_router.c`
- 新建：`main/inc/ui/voice_popup.h`
- 新建：`main/src/ui/voice_popup.c`
- 修改：`main/src/ui/home_page.c`
- 修改：`main/src/ui/scenes_page.c`
- 修改：`main/src/ui/settings_panel.c`

- [x] 实现 `home` 与 `scenes` 之间的左右滑动切换。
- [x] 实现顶部下拉手势，或等效事件路径，用于打开 `settings_panel`。
- [x] 实现设置面板关闭行为：
  - 反向拖回
  - 点击关闭区域
  - 显式关闭操作
- [x] 将 `voice_popup` 实现为位于最上层的覆盖层，与当前页面解耦。
- [x] 在语音弹窗中显示输入文本区和输出文本区。
- [x] 明确覆盖层优先级规则：
  - 语音弹窗高于页面
  - 设置面板高于主页面
  - 熄屏态下先处理亮屏事件，再决定是否透传普通点击
- [x] 确保覆盖层创建/销毁是可重复执行的，且不会泄漏根对象或字体句柄。

**完成标准：**
- app router 可以在两个主页面之间切换
- 设置面板可以打开和关闭
- 语音弹窗可以覆盖在任何页面之上

### 任务 9：接入所需的软件侧 UI 行为

**文件：**
- 修改：`main/src/model/device_state.c`
- 修改：`main/src/ui/home_page.c`
- 修改：`main/src/ui/scenes_page.c`
- 修改：`main/src/ui/settings_panel.c`
- 修改：`main/src/ui/voice_popup.c`

- [x] 首页：
  - 切换两路开关的 mock 状态
  - 根据状态刷新开关名称
- [x] 场景页：
  - 触发场景动作回调
  - 显示成功/失败的占位弹窗状态
- [x] 设置面板：
  - 更新亮度值
  - 更新自动熄屏选项
  - 更新接近亮屏开关
- [x] 全局：
  - 通过 mock 定时器更新时间/日期显示
  - 在状态层支持 12/24 小时制切换
  - 在状态层支持 Wi-Fi 信号图标等级
  - 在状态层支持天气/温度占位数据
- [x] 语音弹窗：
  - 显示 mock ASR 文本
  - 显示 mock AI 回复文本
  - 支持 listening / thinking / speaking / muted 等基础可视状态
- [x] 本阶段不接入真实后端；只保留函数桩和后续适配注释。

**完成标准：**
- 3 个页面已经暴露所有所需的软件侧 UI 状态，且由 mock 数据驱动
- 模拟器中可以跑通整套 UI 交互，不依赖真实后端

### 任务 10：验证与设计差异复核

**文件：**
- 修改：`docs/design/mastergo_home_values.md`
- 修改：`docs/design/mastergo_scenes_values.md`
- 修改：`docs/design/mastergo_settings_values.md`

- [x] 构建项目：
  - `cmake -B build -S .`
  - `cmake --build build -j$(nproc)`
- [x] 运行模拟器：
  - `./bin/main`
- [x] 检查是否存在明显运行问题：
  - 空白屏
  - 白色容器遮挡
  - 字体缺失
  - 图片解码失败
  - 页面切换异常
  - 重复创建覆盖层导致泄漏或悬挂指针
- [x] 逐页对比 MasterGo DSL 和原型。
- [x] 在每个页面设计值文档中更新：
  - 已精确对齐项
  - 可接受偏差项
  - 尚未对齐项及原因
- [x] 总结当前采用的字体方案，判断它是否仍适合后续移植到嵌入式目标。

**完成标准：**
- 构建通过
- 模拟器可运行
- 剩余设计差异均已文档化，而不是被隐藏

## 2026-04-01 收尾复核

- [x] 已补齐 review 回归项：
  - 设置面板改为 `480x480 viewport + 480x656 panel`，完整长面板内容可访问。
  - 场景页补齐 `8` 个场景的可达路径，当前实现为 2 页分页、每页 4 卡。
  - 首页天气与 Wi-Fi 状态改为读取 `device_state`，不再硬编码。
  - `home/scenes` 事件绑定改为实例级处理，避免多实例互相串扰。
  - mock 时间推进补齐跨天日期和星期滚动。
  - 手势处理补齐 `lv_indev_active()==NULL` 保护，避免程序化事件崩溃。
- [x] 已增加对应回归测试：
  - `tests/foundation_layer_test.c`
  - `tests/page_modules_test.c`
- [x] 已重新完成构建、关键测试集和模拟器启动验证。

**当前残余低风险项：**

- `home_page_apply_state()` 现已按可写状态模型使用；如果后续需要只读快照渲染模式，应再拆分只读渲染接口。
- 场景页分页切换目前由底部圆点触发，尚未补横滑动画。
- 设置页关闭当前以把手点击/手势为主，未做独立空白遮罩点击区。

## 执行过程中需要解决的风险

- 场景设计从视觉上看可见槽位较少，但需求层面要求最多支持 `8` 个场景。
- 调用 MasterGo 工具时，设置页节点 id `162:05153` 必须原样保留；除非工具明确证明需要转换，否则不要擅自修改冒号格式。
- 设计中使用了 `Urbanist`、`Source Han Sans CN`、`SFProDisplay-Medium`、`SF UI Display` 等字体；在提交最终资源前，需要确认本地可用性和许可证问题。
- 首页中的渐变和复合视觉效果，可能需要采用原生实现和光栅资源混合方案。
- 当前仓库优先面向 SDL 模拟器；如果后续迁移到 MCU 硬件，运行时 TTF 和 PNG 加载可能还需要补第二阶段的资源打包方案。

## 推荐执行顺序

- [x] 在写任何视觉代码前，先完成任务 1。
- [x] 在实现页面视觉前，先完成任务 2 和任务 3。
- [x] 页面实现顺序按以下先后推进：
  - home
  - scenes
  - settings
  - voice popup
- [x] 静态视觉完成后，再补交互。
- [x] 在任务 10 的模拟器验证稳定前，不要开始接入真实后端。
