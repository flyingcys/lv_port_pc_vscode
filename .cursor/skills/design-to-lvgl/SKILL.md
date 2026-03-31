---
name: design-to-lvgl
description: Use when converting MasterGo or Figma designs into LVGL pages in the current repository, especially when the user provides design links, node IDs, DSL, pixel-perfect restoration requirements, or asks to localize assets and fonts.
---
# Design To LVGL

## 概述

将 `MasterGo` 或 `Figma` 设计稿还原为当前仓库中的 `LVGL` 页面。

这个 skill 的目标是把真实设计数据映射成可维护的 `LVGL` 实现，而不是把截图、`HTML`、`React` 或整页大图当作最终结果。

## 触发条件

出现以下任一信号就使用本 skill：

- 用户给出 `MasterGo`、`Figma`、`prototype`、`design` 链接
- 用户要求把设计稿还原成 `LVGL` 页面
- 用户要求“像素级对齐”“1:1 还原”“按 DSL 对齐”
- 用户提到“本地化素材”“补字体”“把设计资源接进当前 LVGL 工程”

如果既没有链接，也没有 `fileId/layerId` 或 `fileKey/nodeId`，先向用户要最小必要信息，不要猜。

## 设计源分流

先判断设计源，只能二选一：

1. `MasterGo`
   先读 `mastergo-reference.md`
2. `Figma`
   先读 `figma-reference.md`

无论哪种来源，都必须同时遵守：

- `common-rules.md`
- `implementation-playbook.md`
- `output-contract.md`

建议阅读顺序：

1. 本文件
2. `common-rules.md`
3. 对应来源的 reference 文件
4. `implementation-playbook.md`
5. `output-contract.md`

## 快速索引

- 通用约束：`common-rules.md`
- 通用流程：`implementation-playbook.md`
- 输出模板：`output-contract.md`
- `MasterGo` 特化：`mastergo-reference.md`
- `Figma` 特化：`figma-reference.md`
- 资源处理脚本：`scripts/asset_tool.py`

## 资源脚本边界

`scripts/asset_tool.py` 是本 skill 默认的资源处理脚本，使用时要注意它的能力边界：

- 位图输入：直接支持 `png/jpg/jpeg/bmp/webp/gif` 等常见图片格式
- `SVG` 输入：通过系统里的 ImageMagick `magick` 或 `convert` 命令先光栅化，再进入统一流程
- 如果系统里没有 `magick/convert`，脚本不能直接处理 `SVG`

遇到 `SVG` 资源时，默认顺序是：

1. 优先用 `asset_tool.py` 转成项目内本地图片
2. 如果环境缺少 `magick/convert`，明确说明依赖缺失
3. 再决定改走 `LVGL` 原生绘制，或让用户补环境/改资源方案

不要在没有先验证 `asset_tool.py` 能力边界的情况下，直接绕开脚本重新造一套临时转换流程。

## 最小工作流

1. 锁定设计源、目标页面、节点和尺寸
2. 读取对应 MCP schema，并提取真实设计数据
3. 输出实现前摘要和关键设计值对照表
4. 确定 `LVGL` 实现策略、资源方案、字体方案
5. 落地页面、资源和字体
6. 构建、运行并汇报剩余偏差

## 验证

当前仓库默认验证命令：

```bash
cmake -B build -S . && cmake --build build -j$(nproc)
./bin/main
```

如果用户明确要求别的验证方式，再按用户要求覆盖。

## 说明

- 这是通用 `LVGL` skill，不绑定 `v8` 或 `v9`
- 版本差异属于具体仓库上下文，不应写死在 skill 主体中
