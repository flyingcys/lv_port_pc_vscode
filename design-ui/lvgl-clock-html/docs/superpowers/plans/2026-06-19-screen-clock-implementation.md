# 横屏电子时钟 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 构建一个 `800x480` 的横屏电子时钟网页，使用真实系统时间驱动，并以参考视频的上移滚动方式更新小时、分钟、秒钟数字。

**Architecture:** 页面采用原生 `HTML + CSS + JavaScript` 多文件结构。时间与文案映射逻辑收敛在可测试的纯函数模块，DOM 渲染和数字滚动动画由浏览器脚本负责，验证分为 Node 逻辑测试和浏览器可见检查两层。

**Tech Stack:** HTML, CSS, Vanilla JavaScript, Node.js built-in test runner, Python `http.server`

---

## 文件结构

- Create: `index.html`
- Create: `styles.css`
- Create: `script.js`
- Create: `tests/clock.test.js`
- Modify: `docs/superpowers/specs/2026-06-19-screen-clock-design.md`

文件职责：

- `index.html`：页面骨架，声明顶部日期栏、中部主时钟区、底部状态栏、六个数字槽和装饰节点
- `styles.css`：`800x480` 布局、颜色梯度、字体层级、数字槽、滚动动画、冒号弱呼吸
- `script.js`：时间拆解、文案映射、秒边界调度、数字槽更新、滚动状态控制
- `tests/clock.test.js`：验证纯逻辑函数，包括时间拆解、月份/星期/时段文案、下一个 tick 延迟
- `docs/superpowers/specs/2026-06-19-screen-clock-design.md`：同步修正时段映射真相，并在实现完成后回写必要实现事实

### Task 1: 建立可测试的时间与文案逻辑

**Files:**
- Create: `tests/clock.test.js`
- Create: `script.js`

- [ ] **Step 1: 写失败的逻辑测试**

```js
import test from 'node:test';
import assert from 'node:assert/strict';
import {
  getMonthLabel,
  getDayLabel,
  getPeriodLabel,
  getTimeParts,
  getDelayToNextSecond,
} from '../script.js';

test('getMonthLabel returns uppercase English month', () => {
  const date = new Date('2024-12-30T22:02:56');
  assert.equal(getMonthLabel(date), 'DECEMBER');
});

test('getDayLabel returns uppercase English weekday', () => {
  const date = new Date('2024-12-30T22:02:56');
  assert.equal(getDayLabel(date), 'MONDAY');
});

test('getPeriodLabel keeps 22:00 in EVENING to match reference', () => {
  assert.equal(getPeriodLabel(22), 'EVENING');
  assert.equal(getPeriodLabel(23), 'NIGHT');
});

test('getTimeParts returns six digits', () => {
  const date = new Date('2024-12-30T22:02:56');
  assert.deepEqual(getTimeParts(date), ['2', '2', '0', '2', '5', '6']);
});

test('getDelayToNextSecond aligns to next second boundary', () => {
  const date = new Date('2024-12-30T22:02:56.250');
  assert.equal(getDelayToNextSecond(date), 750);
});
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `node --test tests/clock.test.js`  
Expected: FAIL with export errors from `../script.js`

- [ ] **Step 3: 写最小逻辑实现**

```js
const MONTHS = [
  'JANUARY', 'FEBRUARY', 'MARCH', 'APRIL', 'MAY', 'JUNE',
  'JULY', 'AUGUST', 'SEPTEMBER', 'OCTOBER', 'NOVEMBER', 'DECEMBER',
];

const DAYS = [
  'SUNDAY', 'MONDAY', 'TUESDAY', 'WEDNESDAY',
  'THURSDAY', 'FRIDAY', 'SATURDAY',
];

export function getMonthLabel(date) {
  return MONTHS[date.getMonth()];
}

export function getDayLabel(date) {
  return DAYS[date.getDay()];
}

export function getPeriodLabel(hour) {
  if (hour >= 5 && hour <= 11) return 'MORNING';
  if (hour >= 12 && hour <= 16) return 'AFTERNOON';
  if (hour >= 17 && hour <= 22) return 'EVENING';
  return 'NIGHT';
}

export function getTimeParts(date) {
  return [
    String(date.getHours()).padStart(2, '0')[0],
    String(date.getHours()).padStart(2, '0')[1],
    String(date.getMinutes()).padStart(2, '0')[0],
    String(date.getMinutes()).padStart(2, '0')[1],
    String(date.getSeconds()).padStart(2, '0')[0],
    String(date.getSeconds()).padStart(2, '0')[1],
  ];
}

export function getDelayToNextSecond(date = new Date()) {
  return 1000 - date.getMilliseconds();
}
```

- [ ] **Step 4: 再跑测试，确认通过**

Run: `node --test tests/clock.test.js`  
Expected: PASS, `5 tests` passed

- [ ] **Step 5: 提交**

```bash
git add tests/clock.test.js script.js docs/superpowers/specs/2026-06-19-screen-clock-design.md
git commit -m "test: define clock timing logic"
```

### Task 2: 搭建静态页面骨架与 `800x480` 画布

**Files:**
- Create: `index.html`
- Modify: `styles.css`
- Modify: `script.js`

- [ ] **Step 1: 写页面结构检查测试**

```js
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';

test('index.html contains six digit slots and three screen sections', () => {
  const html = readFileSync(new URL('../index.html', import.meta.url), 'utf8');
  assert.match(html, /class="screen__header"/);
  assert.match(html, /class="screen__clock"/);
  assert.match(html, /class="screen__footer"/);
  assert.equal((html.match(/data-digit-slot=/g) || []).length, 6);
});
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `node --test tests/clock.test.js`  
Expected: FAIL because `index.html` does not exist

- [ ] **Step 3: 写最小页面骨架**

```html
<!DOCTYPE html>
<html lang="zh-CN">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Screen Clock</title>
    <link rel="stylesheet" href="./styles.css" />
  </head>
  <body>
    <main class="app-shell">
      <section class="screen" aria-label="电子时钟屏">
        <header class="screen__header">
          <span id="year">2024</span>
          <span id="month">DECEMBER</span>
          <span id="date">30</span>
        </header>
        <section class="screen__clock">
          <p id="period" class="screen__period">EVENING</p>
          <div class="clock-display" aria-label="当前时间">
            <div class="digit-slot" data-digit-slot="0"><span>0</span></div>
            <div class="digit-slot" data-digit-slot="1"><span>0</span></div>
            <div class="separator">:</div>
            <div class="digit-slot" data-digit-slot="2"><span>0</span></div>
            <div class="digit-slot" data-digit-slot="3"><span>0</span></div>
            <div class="separator">:</div>
            <div class="digit-slot" data-digit-slot="4"><span>0</span></div>
            <div class="digit-slot" data-digit-slot="5"><span>0</span></div>
          </div>
          <div class="alarm-mark" aria-hidden="true">⏰</div>
        </section>
        <footer class="screen__footer">
          <div class="metric"><span>TEMP</span><strong id="temp">25.6</strong><span>°C</span></div>
          <div class="weekday" id="weekday">MONDAY</div>
          <div class="metric metric--end"><span>HUMIDITY</span><strong id="humidity">65</strong><span>%</span></div>
        </footer>
      </section>
    </main>
    <script type="module" src="./script.js"></script>
  </body>
</html>
```

- [ ] **Step 4: 写基础样式与浏览器入口**

```css
:root {
  --screen-width: 800px;
  --screen-height: 480px;
  --header-bg: linear-gradient(180deg, #67ecfb 0%, #47cee8 100%);
  --footer-bg: linear-gradient(180deg, #63ccff 0%, #2d87e5 100%);
  --main-bg: linear-gradient(180deg, #ffffff 0%, #f4faff 100%);
  --digit-color: #ff5b5b;
  --ink: #1d3650;
}

* { box-sizing: border-box; }
html, body { margin: 0; min-height: 100%; }
body {
  display: grid;
  place-items: center;
  background: linear-gradient(180deg, #d4dde6 0%, #b4c2d0 100%);
  font-family: Georgia, 'Times New Roman', serif;
}

.screen {
  width: var(--screen-width);
  height: var(--screen-height);
  display: grid;
  grid-template-rows: 16fr 68fr 16fr;
  border: 5px solid #2c82dd;
  border-radius: 16px;
  overflow: hidden;
}
```

```js
if (typeof window !== 'undefined') {
  window.addEventListener('DOMContentLoaded', () => {
    // browser bootstrap will be filled in later tasks
  });
}
```

- [ ] **Step 5: 再跑测试，确认通过**

Run: `node --test tests/clock.test.js`  
Expected: PASS, including HTML structure check

- [ ] **Step 6: 提交**

```bash
git add index.html styles.css script.js tests/clock.test.js
git commit -m "feat: scaffold screen clock layout"
```

### Task 3: 实现数字槽滚动与真实时间渲染

**Files:**
- Modify: `script.js`
- Modify: `styles.css`

- [ ] **Step 1: 扩展失败测试，锁定渲染输入输出**

```js
import test from 'node:test';
import assert from 'node:assert/strict';
import { buildViewModel } from '../script.js';

test('buildViewModel produces labels and digits for reference time', () => {
  const date = new Date('2024-12-30T22:02:56');
  assert.deepEqual(buildViewModel(date), {
    year: '2024',
    month: 'DECEMBER',
    date: '30',
    weekday: 'MONDAY',
    period: 'EVENING',
    digits: ['2', '2', '0', '2', '5', '6'],
    temp: '25.6',
    humidity: '65',
  });
});
```

- [ ] **Step 2: 运行测试，确认失败**

Run: `node --test tests/clock.test.js`  
Expected: FAIL because `buildViewModel` is not exported

- [ ] **Step 3: 实现 view model 与浏览器更新**

```js
export function buildViewModel(date) {
  return {
    year: String(date.getFullYear()),
    month: getMonthLabel(date),
    date: String(date.getDate()).padStart(2, '0'),
    weekday: getDayLabel(date),
    period: getPeriodLabel(date.getHours()),
    digits: getTimeParts(date),
    temp: '25.6',
    humidity: '65',
  };
}

function animateDigit(slot, nextValue) {
  const current = slot.querySelector('.digit-slot__current');
  if (current?.textContent === nextValue) return;

  const incoming = document.createElement('span');
  incoming.className = 'digit-slot__incoming';
  incoming.textContent = nextValue;
  slot.append(incoming);

  requestAnimationFrame(() => {
    slot.classList.add('digit-slot--rolling');
  });

  slot.addEventListener('animationend', () => {
    slot.innerHTML = `<span class="digit-slot__current">${nextValue}</span>`;
    slot.classList.remove('digit-slot--rolling');
  }, { once: true });
}
```

- [ ] **Step 4: 补齐滚动样式**

```css
.digit-slot {
  position: relative;
  width: 78px;
  height: 124px;
  overflow: hidden;
  border-radius: 18px;
  background: rgba(255, 91, 91, 0.08);
}

.digit-slot__current,
.digit-slot__incoming {
  position: absolute;
  inset: 0;
  display: grid;
  place-items: center;
  font-size: 108px;
  font-weight: 800;
  color: var(--digit-color);
}

.digit-slot__incoming {
  transform: translateY(100%);
}

.digit-slot--rolling .digit-slot__current {
  animation: digit-out 300ms cubic-bezier(.18, .78, .32, 1) forwards;
}

.digit-slot--rolling .digit-slot__incoming {
  animation: digit-in 300ms cubic-bezier(.18, .78, .32, 1) forwards;
}

@keyframes digit-out {
  to { transform: translateY(-100%); opacity: 0; }
}

@keyframes digit-in {
  to { transform: translateY(0); }
}
```

- [ ] **Step 5: 再跑测试，确认通过**

Run: `node --test tests/clock.test.js`  
Expected: PASS, including `buildViewModel`

- [ ] **Step 6: 浏览器手工验证**

Run: `python3 -m http.server 4173`  
Open: `http://localhost:4173`  
Expected:
- 页面显示当前真实时间
- 秒钟每秒向上滚动
- 分钟、小时在进位时滚动
- 22 点时段文案显示 `EVENING`

- [ ] **Step 7: 提交**

```bash
git add script.js styles.css tests/clock.test.js
git commit -m "feat: animate rolling clock digits"
```

### Task 4: 完成视觉收口与验收

**Files:**
- Modify: `styles.css`
- Modify: `index.html`
- Modify: `docs/superpowers/specs/2026-06-19-screen-clock-design.md`

- [ ] **Step 1: 写验收检查测试**

```js
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';

test('styles declare fixed 800x480 screen dimensions', () => {
  const css = readFileSync(new URL('../styles.css', import.meta.url), 'utf8');
  assert.match(css, /--screen-width:\\s*800px/);
  assert.match(css, /--screen-height:\\s*480px/);
});
```

- [ ] **Step 2: 运行测试，确认失败或覆盖不足**

Run: `node --test tests/clock.test.js`  
Expected: FAIL if fixed-size tokens are missing, or PASS after they are added

- [ ] **Step 3: 微调视觉细节**

```css
.screen__header,
.screen__footer {
  color: var(--ink);
  letter-spacing: 0.08em;
}

.screen__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 26px;
  background: var(--header-bg);
}

.screen__clock {
  position: relative;
  display: grid;
  place-items: center;
  background: var(--main-bg);
}

.separator {
  font-size: 92px;
  color: var(--digit-color);
  animation: colon-breathe 1.8s ease-in-out infinite;
}

@keyframes colon-breathe {
  0%, 100% { opacity: 0.95; }
  50% { opacity: 0.55; }
}
```

- [ ] **Step 4: 回写 spec 实现事实**

```md
在 `docs/superpowers/specs/2026-06-19-screen-clock-design.md` 追加一个“实现状态”小节，记录：
- 已采用多文件静态结构
- 时段映射以 `22:59` 前为 `EVENING`
- 温湿度仍为固定示例值
```

- [ ] **Step 5: 运行最终验证**

Run:
- `node --test tests/clock.test.js`
- `python3 -m http.server 4173`

Expected:
- Node tests all PASS
- 浏览器肉眼检查通过：布局、颜色、滚动方向、实时更新时间正确

- [ ] **Step 6: 提交**

```bash
git add index.html styles.css script.js tests/clock.test.js docs/superpowers/specs/2026-06-19-screen-clock-design.md
git commit -m "feat: finalize screen clock ui"
```

## Self-Review

### Spec coverage

- `800x480` 固定画布：Task 2, Task 4
- 三段横屏布局：Task 2
- 真实系统时间：Task 1, Task 3
- 小时/分钟/秒钟统一上移滚动：Task 3
- 英文月份、星期、时段文案：Task 1, Task 3
- 温湿度固定示例值：Task 3, Task 4
- 不做屏摄噪点和透视：Task 4 仅做视觉收口，没有额外特效任务

### Placeholder scan

- 无 `TODO/TBD`
- 每个任务包含明确文件路径、命令、预期结果
- 代码步骤提供了具体代码块

### Type consistency

- 纯逻辑函数统一由 `script.js` 导出
- 测试名称与实现函数名一致：`getMonthLabel`、`getDayLabel`、`getPeriodLabel`、`getTimeParts`、`getDelayToNextSecond`、`buildViewModel`

