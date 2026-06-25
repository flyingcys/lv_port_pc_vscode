# Clock UI Optimization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 全面优化 HTML 时钟 UI，提升字体、布局比例、数字动画流畅度，并添加启动淡入动画。

**Architecture:** 保持 HTML 语义结构与 script.js 逻辑不变，仅重写 `styles.css` 视觉规格，更新 `index.html` 的字体引入和 SVG 图标，同步更新测试合约断言。启动动画使用纯 CSS（无需 JS 触发），数字滚动动画修正 easing 和时长。

**Tech Stack:** 原生 HTML/CSS/JS，Bebas Neue（Google Fonts），Node.js `node:test` 测试框架

---

## 文件总览

| 文件 | 操作 | 说明 |
|------|------|------|
| `tests/clock.test.js` | Modify | 更新两处合约断言（alarm emoji→SVG，colon opacity 0.55→0.4），新增字体/图标/尺寸断言 |
| `index.html` | Modify | 添加 Google Fonts `<link>`，alarm-mark 替换为内联 SVG，温度/湿度添加 SVG 图标 |
| `styles.css` | Modify | 全面重写：字体、颜色变量、布局比例、数字尺寸、动画曲线、启动动画 |

---

## Task 1: 更新测试合约（TDD 先行）

**Files:**
- Modify: `tests/clock.test.js:263` — alarm-mark emoji → SVG 断言
- Modify: `tests/clock.test.js:278` — colon-breathe opacity 0.55 → 0.4
- Modify: `tests/clock.test.js` — 新增 4 条断言（字体 link、Bebas Neue、digit-slot、screen animation）

- [ ] **Step 1: 更新 alarm-mark 断言（line 263）**

将：
```js
assert.match(html, /class="alarm-mark"[\s\S]*⏰/);
```
改为：
```js
assert.match(html, /class="alarm-mark"[\s\S]*<svg/);
```

- [ ] **Step 2: 更新 colon-breathe opacity 断言（line 278）**

将：
```js
assert.match(css, /@keyframes colon-breathe\s*\{[\s\S]*50%\s*\{\s*opacity:\s*0\.55;\s*\}/);
```
改为：
```js
assert.match(css, /@keyframes colon-breathe\s*\{[\s\S]*50%\s*\{\s*opacity:\s*0\.4;\s*\}/);
```

- [ ] **Step 3: 在测试文件末尾追加新合约测试**

在 `tests/clock.test.js` 末尾添加：
```js
test('optimization: index.html links Bebas Neue from Google Fonts', () => {
  const html = readFileSync(new URL('../index.html', import.meta.url), 'utf8');
  assert.match(html, /fonts\.googleapis\.com[\s\S]*Bebas\+Neue/);
});

test('optimization: styles.css uses Bebas Neue as primary font', () => {
  const css = readFileSync(new URL('../styles.css', import.meta.url), 'utf8');
  assert.match(css, /font-family:\s*['"]?Bebas Neue['"]?/);
});

test('optimization: digit-slot height is 220px in styles.css', () => {
  const css = readFileSync(new URL('../styles.css', import.meta.url), 'utf8');
  assert.match(css, /\.digit-slot\s*\{[\s\S]*height:\s*220px/);
});

test('optimization: screen has startup fadein animation', () => {
  const css = readFileSync(new URL('../styles.css', import.meta.url), 'utf8');
  assert.match(css, /@keyframes screen-fadein/);
  assert.match(css, /\.screen\s*\{[\s\S]*animation:[\s\S]*screen-fadein/);
});
```

- [ ] **Step 4: 运行测试，确认新断言均失败**

```bash
cd /home/share/samba/flyingcys/lvgl-clock-html && node --test tests/clock.test.js 2>&1 | tail -20
```

预期：新增的 4 条 `optimization:` 测试 FAIL，alarm-mark 和 colon-breathe 合约测试 FAIL，其他原有测试全部 PASS。

- [ ] **Step 5: 提交**

```bash
git add tests/clock.test.js
git commit -m "test: update alarm-mark and colon-breathe contracts, add optimization assertions"
```

---

## Task 2: 更新 index.html（字体 + SVG 图标）

**Files:**
- Modify: `index.html`

- [ ] **Step 1: 在 `<head>` 中添加 Google Fonts link**

在 `<link rel="stylesheet" href="./styles.css" />` 之前插入：
```html
<link rel="preconnect" href="https://fonts.googleapis.com" />
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
<link href="https://fonts.googleapis.com/css2?family=Bebas+Neue&display=swap" rel="stylesheet" />
```

- [ ] **Step 2: 替换 alarm-mark 内容为内联 SVG**

将：
```html
<div class="alarm-mark" aria-hidden="true">⏰</div>
```
改为：
```html
<div class="alarm-mark" aria-hidden="true"><svg xmlns="http://www.w3.org/2000/svg" width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="#e8302a" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="13" r="8"/><path d="M12 9v4l2 2"/><path d="M5 3L2 6"/><path d="M22 6l-3-3"/></svg></div>
```

- [ ] **Step 3: 在温度 metric 中添加温度计 SVG 图标**

将：
```html
<div class="metric">
  <span class="metric__label">TEMP</span>
  <strong id="temp">25.6</strong>
  <span class="metric__unit">°C</span>
</div>
```
改为：
```html
<div class="metric">
  <span class="metric__icon"><svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="rgba(255,255,255,0.75)" stroke-width="2" aria-hidden="true"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg></span>
  <span class="metric__label">TEMP</span>
  <strong id="temp">25.6</strong>
  <span class="metric__unit">°C</span>
</div>
```

- [ ] **Step 4: 在湿度 metric 中添加水滴 SVG 图标**

将：
```html
<div class="metric metric--end">
  <span class="metric__label">HUMIDITY</span>
  <strong id="humidity">65</strong>
  <span class="metric__unit">%</span>
</div>
```
改为：
```html
<div class="metric metric--end">
  <span class="metric__label">HUMIDITY</span>
  <span class="metric__icon"><svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 24 24" fill="rgba(255,255,255,0.75)" stroke="none" aria-hidden="true"><path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/></svg></span>
  <strong id="humidity">65</strong>
  <span class="metric__unit">%</span>
</div>
```

- [ ] **Step 5: 运行测试，确认 HTML 相关断言通过**

```bash
cd /home/share/samba/flyingcys/lvgl-clock-html && node --test tests/clock.test.js 2>&1 | grep -E "alarm-mark|Bebas Neue|PASS|FAIL" | head -20
```

预期：`alarm-mark` 合约通过，`optimization: index.html links Bebas Neue` 通过。

- [ ] **Step 6: 提交**

```bash
git add index.html
git commit -m "feat: add Bebas Neue font, replace alarm/temp/humidity emoji with SVG icons"
```

---

## Task 3: 重写 styles.css — 视觉基础（变量 + 布局 + 字体）

**Files:**
- Modify: `styles.css`

- [ ] **Step 1: 替换文件开头的 `@import` 和 `:root` 变量块**

将现有 `:root { ... }` 块（lines 1–13）完整替换为：
```css
:root {
  --screen-width: 800px;
  --screen-height: 480px;
  --header-bg: linear-gradient(180deg, #4dd8d8 0%, #38b8cc 100%);
  --footer-bg: linear-gradient(180deg, #5599ee 0%, #2d74d4 100%);
  --main-bg: linear-gradient(180deg, #f2f9ff 0%, #e8f4ff 100%);
  --digit-color: #e8302a;
  --digit-shadow: rgba(232, 48, 42, 0.2);
  --period-color: #e8302a;
  --temp-value-color: #e8c030;
  --ink: #ffffff;
}
```

- [ ] **Step 2: 更新 `body` 字体**

将：
```css
body {
  display: grid;
  place-items: center;
  background: linear-gradient(180deg, #d4dde6 0%, #b4c2d0 100%);
  font-family: Georgia, 'Times New Roman', serif;
}
```
改为：
```css
body {
  display: grid;
  place-items: center;
  background: linear-gradient(180deg, #d4dde6 0%, #b4c2d0 100%);
  font-family: 'Bebas Neue', Impact, 'Arial Narrow', sans-serif;
}
```

- [ ] **Step 3: 更新 `.screen` grid 行高**

将：
```css
.screen {
  width: var(--screen-width);
  height: var(--screen-height);
  display: grid;
  grid-template-rows: 16fr 68fr 16fr;
  border: 5px solid #2c82dd;
  border-radius: 16px;
  overflow: hidden;
  box-shadow: 0 20px 40px rgba(31, 70, 114, 0.2);
}
```
改为：
```css
.screen {
  width: var(--screen-width);
  height: var(--screen-height);
  display: grid;
  grid-template-rows: 56px 1fr 56px;
  border: 5px solid #2c82dd;
  border-radius: 16px;
  overflow: hidden;
  box-shadow: 0 20px 40px rgba(31, 70, 114, 0.2);
  animation: screen-fadein 600ms ease-out 200ms both;
}
```

- [ ] **Step 4: 更新 `.screen__header` 字体尺寸**

将：
```css
.screen__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 26px;
  background: var(--header-bg);
  position: relative;
  font-size: 18px;
  font-weight: 700;
  text-transform: uppercase;
}
```
改为：
```css
.screen__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 26px;
  background: var(--header-bg);
  position: relative;
  font-size: 22px;
  letter-spacing: 0.08em;
  color: var(--ink);
}
```

- [ ] **Step 5: 更新 `.screen__header-title` 和 `.screen__header-anchor`**

将：
```css
.screen__header-title {
  position: absolute;
  left: 50%;
  transform: translateX(-50%);
  font-size: 31px;
  font-weight: 800;
  letter-spacing: 0.12em;
  text-shadow: 0 1px 0 rgba(255, 255, 255, 0.25);
}

.screen__header-anchor {
  min-width: 72px;
  font-size: 17px;
  opacity: 0.88;
}
```
改为：
```css
.screen__header-title {
  position: absolute;
  left: 50%;
  transform: translateX(-50%);
  font-size: 34px;
  letter-spacing: 0.14em;
}

.screen__header-anchor {
  min-width: 72px;
  font-size: 22px;
}
```

- [ ] **Step 6: 更新 `.screen__footer` 及其子元素**

将：
```css
.screen__footer {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 26px;
  background: var(--footer-bg);
  font-size: 18px;
  font-weight: 700;
}

.metric,
.weekday {
  display: flex;
  align-items: baseline;
  gap: 8px;
  color: #f7fbff;
}

.metric {
  min-width: 200px;
}

.metric strong {
  font-size: 29px;
}

.metric__label,
.metric__unit {
  font-size: 14px;
  opacity: 0.78;
}

.weekday {
  justify-content: center;
  flex: 1;
  font-size: 32px;
  font-weight: 800;
  letter-spacing: 0.14em;
  text-shadow: 0 1px 0 rgba(255, 255, 255, 0.22);
}

.metric--end {
  justify-content: flex-end;
}
```
改为：
```css
.screen__footer {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 26px;
  background: var(--footer-bg);
  font-size: 18px;
  color: var(--ink);
}

.metric,
.weekday {
  display: flex;
  align-items: center;
  gap: 6px;
  color: var(--ink);
}

.metric {
  min-width: 200px;
  align-items: baseline;
}

.metric strong {
  font-size: 30px;
}

.metric__icon {
  opacity: 0.8;
  display: flex;
  align-items: center;
}

.metric__label,
.metric__unit {
  font-size: 13px;
  opacity: 0.75;
}

#temp {
  color: var(--temp-value-color);
}

.weekday {
  justify-content: center;
  flex: 1;
  font-size: 34px;
  letter-spacing: 0.14em;
}

.metric--end {
  justify-content: flex-end;
}
```

- [ ] **Step 7: 运行测试**

```bash
cd /home/share/samba/flyingcys/lvgl-clock-html && node --test tests/clock.test.js 2>&1 | tail -20
```

预期：`optimization: styles.css uses Bebas Neue` 通过，`task 4 styles lock` 中 `--screen-width: 800px` / `--screen-height: 480px` 仍通过。

- [ ] **Step 8: 提交**

```bash
git add styles.css
git commit -m "feat: rewrite CSS visual base — Bebas Neue font, 56px header/footer, ink colors"
```

---

## Task 4: 重写 styles.css — 数字样式

**Files:**
- Modify: `styles.css`

- [ ] **Step 1: 更新 `.screen__clock` padding**

将：
```css
.screen__clock {
  position: relative;
  display: grid;
  place-items: center;
  background: var(--main-bg);
  padding: 18px 28px 12px;
}
```
改为：
```css
.screen__clock {
  position: relative;
  display: grid;
  place-items: center;
  background: var(--main-bg);
  padding: 10px 28px 8px;
}
```

- [ ] **Step 2: 更新 `.screen__period` 样式**

将：
```css
.screen__period {
  margin: 0;
  position: absolute;
  top: 18px;
  left: 50%;
  transform: translateX(-50%);
  color: var(--period-color);
  font-size: 30px;
  font-weight: 700;
  letter-spacing: 0.34em;
  text-shadow: 0 1px 0 rgba(255, 255, 255, 0.85);
}
```
改为：
```css
.screen__period {
  margin: 0;
  position: absolute;
  top: 12px;
  left: 50%;
  transform: translateX(-50%);
  color: var(--period-color);
  font-size: 28px;
  letter-spacing: 0.34em;
  white-space: nowrap;
}
```

- [ ] **Step 3: 更新 `.clock-display` gap 和 margin**

将：
```css
.clock-display {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-top: 18px;
}
```
改为：
```css
.clock-display {
  display: flex;
  align-items: center;
  gap: 2px;
  margin-top: 12px;
}
```

- [ ] **Step 4: 更新 `.digit-slot` 尺寸和背景**

将：
```css
.digit-slot {
  position: relative;
  width: 84px;
  height: 144px;
  overflow: hidden;
  border-radius: 20px;
  background: linear-gradient(180deg, rgba(255, 116, 105, 0.06) 0%, rgba(255, 116, 105, 0.14) 100%);
}
```
改为：
```css
.digit-slot {
  position: relative;
  width: 96px;
  height: 220px;
  overflow: hidden;
  background: transparent;
}
```

- [ ] **Step 5: 更新 `.digit-slot__current` / `.digit-slot__incoming` 字体**

将：
```css
.digit-slot__current,
.digit-slot__incoming {
  position: absolute;
  inset: 0;
  display: grid;
  place-items: center;
  font-size: 132px;
  font-weight: 800;
  line-height: 1;
  color: var(--digit-color);
  text-shadow: 0 6px 14px var(--digit-shadow);
}
```
改为：
```css
.digit-slot__current,
.digit-slot__incoming {
  position: absolute;
  inset: 0;
  display: grid;
  place-items: center;
  font-size: 200px;
  font-weight: 400;
  line-height: 1;
  color: var(--digit-color);
  text-shadow: 0 4px 12px var(--digit-shadow);
}
```

- [ ] **Step 6: 更新 `.separator` 字体尺寸**

将：
```css
.separator {
  font-size: 92px;
  color: var(--digit-color);
  font-weight: 700;
  line-height: 1;
  text-shadow: 0 6px 14px var(--digit-shadow);
  animation: colon-breathe 1.8s ease-in-out infinite;
}
```
改为：
```css
.separator {
  font-size: 160px;
  color: var(--digit-color);
  font-weight: 400;
  line-height: 1;
  text-shadow: 0 4px 12px var(--digit-shadow);
  animation: colon-breathe 1.8s ease-in-out infinite;
  padding-bottom: 24px;
}
```

- [ ] **Step 7: 更新 `.alarm-mark` 字体尺寸（SVG 不需要 font-size）**

将：
```css
.alarm-mark {
  position: absolute;
  top: 18px;
  right: 86px;
  color: #ffb648;
  font-size: 30px;
  transform: rotate(-8deg);
  filter: drop-shadow(0 4px 8px rgba(241, 179, 67, 0.28));
}
```
改为：
```css
.alarm-mark {
  position: absolute;
  top: 18px;
  right: 86px;
  transform: rotate(-8deg);
  filter: drop-shadow(0 2px 6px rgba(232, 48, 42, 0.35));
}
```

- [ ] **Step 8: 运行测试**

```bash
cd /home/share/samba/flyingcys/lvgl-clock-html && node --test tests/clock.test.js 2>&1 | tail -20
```

预期：`optimization: digit-slot height is 220px` 通过，`task 4 styles lock` 中 `alarm-mark position/right` 仍通过。

- [ ] **Step 9: 提交**

```bash
git add styles.css
git commit -m "feat: update digit slot to 220px/200px font, remove slot background, enlarge separator"
```

---

## Task 5: 重写 styles.css — 动画

**Files:**
- Modify: `styles.css`

- [ ] **Step 1: 更新 `.digit-slot--rolling` 动画时长和曲线**

将：
```css
.digit-slot--rolling .digit-slot__current {
  animation: digit-out 300ms cubic-bezier(.18, .78, .32, 1) forwards;
}

.digit-slot--rolling .digit-slot__incoming {
  animation: digit-in 300ms cubic-bezier(.18, .78, .32, 1) forwards;
}
```
改为：
```css
.digit-slot--rolling .digit-slot__current {
  animation: digit-out 250ms cubic-bezier(0.25, 0.46, 0.45, 0.94) forwards;
}

.digit-slot--rolling .digit-slot__incoming {
  animation: digit-in 250ms cubic-bezier(0.25, 0.46, 0.45, 0.94) forwards;
}
```

- [ ] **Step 2: 在 `.digit-slot--rolling` 规则之后，添加各 slot 启动错落动画**

在现有 `.digit-slot--rolling` CSS 块之后插入：
```css
[data-digit-slot="0"] { animation: slot-fadein 400ms ease-out 200ms both; }
[data-digit-slot="1"] { animation: slot-fadein 400ms ease-out 260ms both; }
[data-digit-slot="2"] { animation: slot-fadein 400ms ease-out 320ms both; }
[data-digit-slot="3"] { animation: slot-fadein 400ms ease-out 380ms both; }
[data-digit-slot="4"] { animation: slot-fadein 400ms ease-out 440ms both; }
[data-digit-slot="5"] { animation: slot-fadein 400ms ease-out 500ms both; }
```

- [ ] **Step 3: 更新 `@keyframes digit-out`（添加 opacity fade）**

将：
```css
@keyframes digit-out {
  to { transform: translateY(-100%); opacity: 0; }
}
```
保持不变（已包含 opacity: 0）✓

- [ ] **Step 4: 更新 `@keyframes colon-breathe` opacity 值**

将：
```css
@keyframes colon-breathe {
  0%, 100% { opacity: 0.95; }
  50% { opacity: 0.55; }
}
```
改为：
```css
@keyframes colon-breathe {
  0%, 100% { opacity: 0.95; }
  50% { opacity: 0.4; }
}
```

- [ ] **Step 5: 在 `@keyframes colon-breathe` 之后追加启动动画 keyframes**

在最后追加：
```css
@keyframes screen-fadein {
  from { opacity: 0; transform: scale(0.97); }
  to   { opacity: 1; transform: scale(1); }
}

@keyframes slot-fadein {
  from { opacity: 0; transform: translateY(16px); }
  to   { opacity: 1; transform: translateY(0); }
}
```

- [ ] **Step 6: 运行完整测试套件**

```bash
cd /home/share/samba/flyingcys/lvgl-clock-html && node --test tests/clock.test.js 2>&1
```

预期：全部测试 PASS，包括：
- `optimization: screen has startup fadein animation` ✓
- `optimization: digit-slot height is 220px` ✓
- `optimization: styles.css uses Bebas Neue` ✓
- `optimization: index.html links Bebas Neue` ✓
- `task 4 html contract keeps clock stage and visual hierarchy anchors` ✓（alarm-mark 已是 SVG）
- `task 4 styles lock fixed canvas and visual accent contracts` ✓（colon opacity 0.4，alarm-mark right 86px）

- [ ] **Step 7: 提交**

```bash
git add styles.css
git commit -m "feat: update digit/colon animations, add screen-fadein and slot-fadein startup animations"
```

---

## 验收核查清单

- [ ] `node --test tests/clock.test.js` 全部通过
- [ ] 浏览器打开 `index.html`：字体显示为 Bebas Neue 粗体压缩风格
- [ ] 数字高度明显变大，无圆角背景卡片
- [ ] 页面加载时有整体淡入 + 数字依次错落出现效果（约 500–900ms）
- [ ] 每秒数字变化时有流畅向上滑动，无撕裂感
- [ ] 闹钟位置有红色 SVG 小闹钟图标
- [ ] 温度数值为金黄色，有温度计图标
- [ ] 湿度有水滴图标
- [ ] 冒号呼吸动画闪烁幅度更明显（0.4 vs 原来 0.55）
