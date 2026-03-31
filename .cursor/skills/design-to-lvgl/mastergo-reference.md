# MasterGo 参考

## 适用场景

用户给出 `MasterGo` 链接、`prototype` 链接、`fileId/layerId`，或明确说“按 MasterGo DSL 还原”时使用。

本文件只保留 `MasterGo` 特有信息。通用规则、实施流程和输出模板统一看：

- `common-rules.md`
- `implementation-playbook.md`
- `output-contract.md`

## 工具选择

优先关注：

- `mcp__getMeta`
- `mcp__getDsl`

调用前必须先读 `user-mastergo-magic-mcp` 对应工具的 schema。

## 常用 schema 要点

### `mcp__getMeta`

- 必填参数：
  - `fileId`
  - `layerId`
- 作用：拿高层页面规则、页面摘要和分析结果

### `mcp__getDsl`

- 可传：
  - `fileId`
  - `layerId`
  - 或 `shortLink`
- 作用：拿原始 DSL 结构、层级、样式和生成代码规则

如果用户已经给了明确的 `fileId/layerId`，优先显式传参，不要只依赖 `shortLink`。

## URL 处理

从用户提供的链接里提取这些信息：

- `fileId`
- `layerId`

如果是类似下面的链接：

```text
https://design.tuya-inc.com:7799/prototype/131850344368735?zs=1&pageId=160%3A2775&layerId=sa281%3A8161
```

通常可以直接得到：

- `fileId = 131850344368735`
- `layerId = sa281:8161`

注意：

- 不要把某个固定链接写死到 skill 里
- 不同页面的 `fileId/layerId` 都可能不同
- 如果 URL 不完整，就向用户要精确节点信息

## 推荐读取顺序

1. 读取 `mcp__getMeta` 和 `mcp__getDsl` 的 schema
2. 从用户链接提取 `fileId/layerId`
3. 先拿 `meta`，理解页面目标与规则
4. 再拿 `dsl`，提取层级、位置、尺寸、颜色、圆角、阴影、透明度、字体、图片资源

工具返回的 `rules` 也属于输入的一部分，不能只看 JSON 结果而忽略规则说明。

## MasterGo 特有提醒

- 固定尺寸屏幕优先按 DSL 的绝对坐标逐项还原，不凭经验估算位置和尺寸
- 图标、背景、插图资源落到本地
- 字体尽量按设计稿同款；无法一致时写明替代字体和偏差
- 如果 DSL 中某些视觉效果用 `LVGL` 很难稳定实现，可以只把局部做成预合成图，不要直接整页截图化
- 不要为了实现方便擅自改圆角、间距、字号、对齐或层级关系

## 截图的正确用法

截图只能用于：

- 对比当前实现和设计差异
- 辅助判断视觉细节
- 帮助定位“哪里不像”

截图不能替代：

- DSL
- 真实节点结构
- 设计 token

## 常见坑

- 只传 `shortLink`，但服务端未能稳定解析
- 忘记把 `layerId` 中的冒号格式保留正确
- 只看截图去猜坐标，忽略 DSL 精确值
- 看了 DSL 但实现时又按主观感觉微调布局
- 图标导出后真实格式和扩展名不一致，导致 `LVGL` 解码失败
- 使用设计字体失败后没有补本地字体，导致字重和字形偏差过大
