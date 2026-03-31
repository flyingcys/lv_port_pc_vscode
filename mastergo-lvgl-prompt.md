# MasterGo 转 LVGL 提示词
## 通用简版
```text
你要把 MasterGo 设计稿还原成高质量的 LVGL v9 页面。
必须遵守：
1. 先读取并使用 `mastergo-magic-mcp` 的 schema，再调用 `mcp__getMeta` 和 `mcp__getDsl` 获取真实设计数据，不允许只靠截图或主观猜测布局。
2. 必须优先按 DSL 还原：尺寸、坐标、层级、字体、颜色、圆角、阴影、透明度、图片资源都要尽量对齐设计稿。
3. 页面必须用 LVGL 原生方式实现，不要生成 HTML 替代，不要用整张大图糊成页面。
4. 图标和背景等素材必须下载到本地，优先保存为 `png`；如果导出文件真实格式和扩展名不一致，必须先校验再修正。
5. 字体必须尽量使用设计稿同款字体；如果缺失，要补到本地后接入 LVGL；如果无法完全一致，必须明确说明替代方案和偏差。
6. 代码结构要清晰，不要把所有逻辑都塞进 `main.c`，应拆成页面模块和资源模块。
7. 先完成静态高保真页面，再做基础交互；不要擅自增加业务逻辑、网络请求、设备控制或多页面功能。
8. 每完成一个关键阶段都必须验证：至少执行构建和运行 smoke check，不能只改代码不验证。
输出要求：
- 先给出从 MasterGo DSL 提取到的页面结构摘要
- 再给出要修改/新增的文件列表
- 再开始实现
- 最后明确汇报：资源落地情况、字体情况、构建结果、运行结果、剩余偏差
```
## 当前项目专用简版
```text
你现在要在当前 LVGL v9 项目中，还原 MasterGo 首页为高质量 LVGL 页面。
项目信息：
- 编译：`cmake -B build -S . && cmake --build build -j$(nproc)`
- 运行：`./bin/main`
- 目标是 LVGL 原生页面，不是 HTML
- 参考文件可用：
  - `./.worktrees/feature-mastergo-home-page/design-preview/home/index.html`
  - `./.worktrees/feature-mastergo-home-page/design-preview/home/styles.css`
  - `./.worktrees/feature-mastergo-home-page/design-preview/home/assets/`
MasterGo 信息：
- 链接：`https://design.tuya-inc.com:7799/prototype/131850344368735?zs=1&pageId=160%3A2775&layerId=sa281%3A8161`
- fileId：`131850344368735`
- layerId：`sa281:8161`
必须要求：
1. 先读取 MCP schema，再调用 `mcp__getMeta(fileId, layerId)` 和 `mcp__getDsl(fileId, layerId)`。
2. 必须按 DSL 还原 `480x480` 页面，优先使用绝对布局，严格对齐设计稿。
3. 必须使用“LVGL 原生绘制 + 必要图片资源本地化”方案，不要偷懒做整图页面。
4. 所有图标素材都要下载到本地，优先为 `png`；字体也要本地化并接入 LVGL。
5. 页面至少拆成：背景层、天气区、时间区、Wi-Fi 区、开关卡片区、分页区。
6. 默认先做：首页静态高保真还原 + 开关卡片基础点击切换；不要擅自增加动态天气、真实设备控制、多页面跳转。
7. 代码不要全写进 `main/src/main.c`，应拆出类似 `home_page`、`home_assets` 模块。
8. 每一步必须验证，至少执行：
   - `cmake -B build -S . && cmake --build build -j$(nproc)`
   - `./bin/main`
9. 如果有无法 1:1 还原的地方，必须先说明原因，再给出最接近设计稿的替代实现。
开始前先输出：
- DSL 提取到的页面结构摘要
- 计划修改/新增的文件
- 资源落地方案
- 字体接入方案
完成后必须输出：
- 修改文件列表
- 本地化资源列表
- 字体是否完全匹配
- 构建结果
- 运行结果
- 剩余偏差点
```