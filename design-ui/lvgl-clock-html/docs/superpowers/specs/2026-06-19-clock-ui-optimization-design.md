# 时钟 UI 优化设计文档

**日期：** 2026-06-19  
**目标：** 全面优化 HTML 复刻时钟 UI，提升视觉还原度、数字动画流畅度、并添加启动动画

---

## 背景

当前 `index.html` / `styles.css` / `script.js` 是根据截图 `1.jpg` 和视频 `IMG_4025.mp4` 复刻的 800×480 电子时钟界面。存在三个主要问题：

1. **视觉还原度差**：字体不匹配、数字太小、digit-slot 有明显圆角背景框
2. **数字动画不流畅**：slot 高度与字体实际渲染高度不匹配，裁切不干净
3. **无启动动画**：页面加载时无入场过渡

---

## 方案选择

采用 **方案 B（完整重建视觉）**：重写 CSS，精确对齐截图比例，保持 HTML 语义结构和 script.js 核心逻辑不变。

---

## 设计规格

### 一、视觉规格

#### 字体

| 用途 | 字体 |
|------|------|
| 数字、标题、所有文字 | `Bebas Neue`（Google Fonts） |
| fallback | `Impact, 'Arial Narrow', sans-serif` |

加载方式：`<link>` 引入 `https://fonts.googleapis.com/css2?family=Bebas+Neue&display=swap`

#### 布局比例（800×480）

| 区域 | 高度 | 占比 |
|------|------|------|
| 顶部 header（青绿色） | 56px | 11.7% |
| 主区域（白色背景） | 368px | 76.7% |
| 底部 footer（蓝色） | 56px | 11.7% |

CSS grid：`grid-template-rows: 56px 1fr 56px`

#### 颜色

| 变量 | 值 | 用途 |
|------|----|------|
| `--header-bg` | `linear-gradient(180deg, #4dd8d8 0%, #38b8cc 100%)` | 顶部背景 |
| `--footer-bg` | `linear-gradient(180deg, #5599ee 0%, #2d74d4 100%)` | 底部背景 |
| `--main-bg` | `#f0f8ff`（近白带浅蓝） | 主区域背景 |
| `--digit-color` | `#e8302a` | 数字红色 |
| `--period-color` | `#e8302a` | MORNING/EVENING 文字颜色 |
| `--temp-value-color` | `#e8c030` | 温度数值金黄色 |
| `--ink` | `#ffffff` | header/footer 文字颜色 |

#### 数字尺寸

| 元素 | 值 |
|------|-----|
| digit-slot 宽度 | `96px` |
| digit-slot 高度 | `220px` |
| 数字 font-size | `200px` |
| 数字 font-weight | `400`（Bebas Neue 本身即超粗） |
| 分隔符 `:` font-size | `160px` |
| 分隔符间距 gap | `4px` |
| digit-slot 背景 | 完全透明（无圆角卡片） |

#### 图标

| 位置 | 当前 | 替换为 |
|------|------|--------|
| 时钟右侧闹钟 | ⏰ emoji | 内联 SVG 小闹钟（红色，24×24） |
| 温度前 | 无 | 内联 SVG 温度计（白色，16×16） |
| 湿度前 | 无 | 内联 SVG 水滴（白色，16×16） |

---

### 二、动画规格

#### 数字滚动动画

每秒数字变化时触发，方向：新数字从下向上进入，旧数字向上退出。

| 参数 | 值 |
|------|-----|
| 时长 | `250ms` |
| easing | `cubic-bezier(0.25, 0.46, 0.45, 0.94)` |
| 旧数字退出 | `translateY(0) + opacity:1` → `translateY(-100%) + opacity:0` |
| 新数字进入 | `translateY(100%)` → `translateY(0)` |

#### 冒号呼吸动画

| 参数 | 值 |
|------|-----|
| 周期 | `1.8s ease-in-out infinite` |
| opacity 范围 | `0.95 ↔ 0.4` |

#### 启动淡入动画（页面加载）

| 步骤 | 说明 |
|------|------|
| `.screen` 整体 | `opacity:0, scale(0.97)` → `opacity:1, scale(1)`，600ms，ease-out，延迟 200ms |
| 6 个 digit-slot | 依次错落淡入，delay 分别为 `0, 60, 120, 180, 240, 300ms`，各 400ms |

---

### 三、实现范围

#### 修改文件

| 文件 | 改动内容 |
|------|---------|
| `index.html` | 1. `<head>` 增加 Google Fonts link；2. 将 ⏰ emoji 替换为内联 SVG；3. 温度/湿度添加 SVG 图标 |
| `styles.css` | 全面重写：字体、比例、颜色、数字尺寸、动画 keyframes |
| `script.js` | 仅在 `startClock()` 中触发 `.screen` 启动动画 class，其余逻辑不动 |

#### 不改动内容

- `script.js` 时间计算、DOM 更新、`animateDigit` 核心逻辑
- HTML 语义结构（header/clock/footer 分区不变）
- 测试文件

---

## 验收标准

1. 字体显示为 Bebas Neue 压缩粗体
2. 数字高度占主区域 ≥ 55%，无可见 slot 背景框
3. 每秒数字变化时有流畅的向上滑动动画，无裁切撕裂
4. 页面加载时有整体淡入 + 数字错落入场效果
5. 闹钟图标为红色 SVG，温度/湿度有对应 SVG 图标
6. 温度数值显示为金黄色
