# Figma 参考

## 适用场景

用户给出 `figma.com/design/...`、`figma.com/board/...`、`figma.com/make/...` 链接，或明确说“按 Figma 设计稿还原”为 LVGL 页面时使用。

本文件只保留 `Figma` 特有信息。通用规则、实施流程和输出模板统一看：

- `common-rules.md`
- `implementation-playbook.md`
- `output-contract.md`

## 入口判断

先确认这是：

- 普通设计文件
- `FigJam`
- `Figma Make`

调用 Figma MCP 工具前，先读对应 schema 或当前服务器说明。

## 常用入口

### 普通设计文件

优先工具：

- `get_design_context`
- 必要时再用 `get_screenshot`
- 必要时再用 `get_metadata`

`get_design_context` 返回的 React + Tailwind 代码只是参考，不能直接当成最终 `LVGL` 实现。对 `LVGL` 来说，真正重要的是结构、层级、token、约束、资源和截图，而不是把 Web 代码机械翻译一遍。

### FigJam

优先工具：

- `get_figjam`

## URL 处理

从 Figma URL 提取：

- `fileKey`
- `nodeId`

常见规则：

- `figma.com/design/:fileKey/:fileName?node-id=:nodeId`
- `figma.com/design/:fileKey/branch/:branchKey/:fileName` 时，用 `branchKey` 作为 `fileKey`
- `figma.com/make/:makeFileKey/:makeFileName` 时，用 `makeFileKey`
- `figma.com/board/:fileKey/:fileName` 通常属于 FigJam

如果 URL 里的 `node-id` 用连字符编码，按服务端要求转换回冒号形式。

## 推荐读取顺序

1. 读取 Figma MCP 入口说明
2. 从 URL 提取 `fileKey/nodeId`
3. 用 `get_design_context` 拿设计上下文
4. 如果结构不清晰或有复杂视觉细节，再补 `get_screenshot`
5. 把返回结果映射成 `LVGL` 可实现的结构，而不是照抄 Web 代码

## Figma 特有提醒

- `Auto Layout` 不等于 `LVGL flex`，先理解约束再映射
- 固定尺寸页面优先回到绝对布局，按设计值逐项落地，不做“差不多”的布局推导
- 颜色、间距、字号、圆角、阴影应提炼成 token 或集中常量
- 图标、插图、复杂背景要本地化
- 对复杂模糊、混合模式、发光效果，必要时做局部预合成资源
- 不要为了实现简单就擅自调整层级、间距、字号或对齐

## 什么时候优先看截图

以下情况优先同时拿截图辅助判断：

- `get_design_context` 给出的代码包含大量绝对定位，语义不清
- 设计中有复杂半透明叠层、模糊、阴影或发光
- 你需要校准字体基线、留白和视觉权重

## 常见坑

- 直接把 React/Tailwind 代码翻译成 `LVGL`，导致结构臃肿且不自然
- 忽略 `Auto Layout` 约束，布局只在一组文案下凑巧看起来对
- 已拿到真实节点尺寸，却又按肉眼手调布局
- 不处理 `nodeId` 编码格式，导致节点取错
- 只看导出的样式，不处理本地图片和字体
- 把一个复杂页面全部做成整图，后续无法维护
