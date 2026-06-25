import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import {
  buildViewModel,
  animateDigit,
  getMonthLabel,
  getDayLabel,
  getPeriodLabel,
  getTimeParts,
  getDelayToNextSecond,
  hasCompleteClockDom,
  renderClock,
} from '../script.js';

class FakeClassList {
  constructor() {
    this.values = new Set();
  }

  add(...names) {
    names.forEach((name) => this.values.add(name));
  }

  remove(...names) {
    names.forEach((name) => this.values.delete(name));
  }

  contains(name) {
    return this.values.has(name);
  }
}

class FakeDigitNode {
  constructor(className, textContent) {
    this.className = className;
    this.textContent = textContent;
    this.parentNode = null;
    this.listeners = new Map();
  }

  addEventListener(type, handler, options = {}) {
    const queue = this.listeners.get(type) || [];
    queue.push({ handler, once: Boolean(options.once) });
    this.listeners.set(type, queue);
  }

  dispatchEvent(event) {
    const queue = [...(this.listeners.get(event.type) || [])];
    const kept = [];
    queue.forEach((entry) => {
      entry.handler.call(this, event);
      if (!entry.once) kept.push(entry);
    });
    this.listeners.set(event.type, kept);
  }
}

class FakeSlot {
  constructor(textContent = '0') {
    this.classList = new FakeClassList();
    this.dataset = {};
    this.nodes = [];
    this.innerHTMLValue = '';
    this.append(new FakeDigitNode('', textContent));
  }

  get firstElementChild() {
    return this.nodes[0] || null;
  }

  querySelector(selector) {
    if (selector === '.digit-slot__current') {
      return this.nodes.find((node) => node.className === 'digit-slot__current') || null;
    }
    if (selector === '.digit-slot__incoming') {
      return this.nodes.find((node) => node.className === 'digit-slot__incoming') || null;
    }
    return null;
  }

  querySelectorAll(selector) {
    if (selector === '.digit-slot__current') {
      return this.nodes.filter((node) => node.className === 'digit-slot__current');
    }
    if (selector === '.digit-slot__incoming') {
      return this.nodes.filter((node) => node.className === 'digit-slot__incoming');
    }
    return [];
  }

  append(node) {
    node.parentNode = this;
    this.nodes.push(node);
  }

  set innerHTML(value) {
    this.innerHTMLValue = value;
    const match = value.match(/digit-slot__current">([^<]+)</);
    const textContent = match ? match[1] : '';
    this.nodes = [];
    this.append(new FakeDigitNode('digit-slot__current', textContent));
  }

  get innerHTML() {
    return this.innerHTMLValue;
  }
}

function createTextNode(initialValue = '') {
  return { textContent: initialValue };
}

function installFakeBrowserHooks() {
  globalThis.requestAnimationFrame = (callback) => {
    installFakeBrowserHooks.frames.push(callback);
    return installFakeBrowserHooks.frames.length;
  };
  globalThis.document = {
    createElement() {
      return new FakeDigitNode('', '');
    },
  };
}

installFakeBrowserHooks.frames = [];

function flushFrames() {
  const queued = [...installFakeBrowserHooks.frames];
  installFakeBrowserHooks.frames.length = 0;
  queued.forEach((callback) => callback());
}

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

test('animateDigit keeps slot idle when digit does not change', () => {
  const slot = new FakeSlot('7');
  installFakeBrowserHooks();
  globalThis.requestAnimationFrame = () => {
    throw new Error('should not schedule animation frame for unchanged digit');
  };

  animateDigit(slot, '7');

  assert.equal(slot.querySelectorAll('.digit-slot__incoming').length, 0);
  assert.equal(slot.classList.contains('digit-slot--rolling'), false);
});

test('animateDigit keeps a single current node after incoming animation ends', () => {
  const slot = new FakeSlot('1');
  installFakeBrowserHooks();

  animateDigit(slot, '2');
  flushFrames();
  slot.querySelector('.digit-slot__incoming').dispatchEvent({ type: 'animationend' });

  assert.equal(slot.querySelectorAll('.digit-slot__current').length, 1);
  assert.equal(slot.querySelector('.digit-slot__current').textContent, '2');
  assert.equal(slot.querySelectorAll('.digit-slot__incoming').length, 0);
  assert.equal(slot.classList.contains('digit-slot--rolling'), false);
});

test('animateDigit ignores current-node animationend bubbling before incoming finishes', () => {
  const slot = new FakeSlot('3');
  installFakeBrowserHooks();

  animateDigit(slot, '4');
  flushFrames();
  slot.querySelector('.digit-slot__current').dispatchEvent({ type: 'animationend' });

  assert.equal(slot.querySelector('.digit-slot__incoming').textContent, '4');
  assert.equal(slot.querySelector('.digit-slot__current').textContent, '3');
  assert.equal(slot.classList.contains('digit-slot--rolling'), true);
});

test('renderClock writes labels and digits into provided dom shape', () => {
  const slots = Array.from({ length: 6 }, () => new FakeSlot('0'));
  const dom = {
    year: createTextNode(),
    month: createTextNode(),
    date: createTextNode(),
    weekday: createTextNode(),
    period: createTextNode(),
    temp: createTextNode(),
    humidity: createTextNode(),
    slots,
  };
  installFakeBrowserHooks();

  renderClock(dom, buildViewModel(new Date('2024-12-30T22:02:56')));
  flushFrames();
  slots.forEach((slot) => slot.querySelector('.digit-slot__incoming')?.dispatchEvent({ type: 'animationend' }));

  assert.equal(dom.year.textContent, '2024');
  assert.equal(dom.period.textContent, 'EVENING');
  assert.deepEqual(slots.map((slot) => slot.querySelector('.digit-slot__current').textContent), ['2', '2', '0', '2', '5', '6']);
});

test('hasCompleteClockDom rejects missing key text nodes', () => {
  assert.equal(hasCompleteClockDom({
    year: createTextNode(),
    month: createTextNode(),
    date: null,
    weekday: createTextNode(),
    period: createTextNode(),
    temp: createTextNode(),
    humidity: createTextNode(),
    slots: Array.from({ length: 6 }, () => new FakeSlot('0')),
  }), false);
});

test('index.html contains six digit slots and three screen sections', () => {
  const html = readFileSync(new URL('../index.html', import.meta.url), 'utf8');
  assert.match(html, /class="screen__header"/);
  assert.match(html, /class="screen__main"/);
  assert.match(html, /class="screen__footer"/);
  assert.equal((html.match(/class="slot"/g) || []).length, 6);
});

test('html contract keeps date anchors, alarm icon and metric units', () => {
  const html = readFileSync(new URL('../index.html', import.meta.url), 'utf8');
  assert.match(html, /id="year"/);
  assert.match(html, /id="month"/);
  assert.match(html, /id="day"/);
  assert.match(html, /id="weekday"/);
  assert.match(html, /id="period"/);
  assert.match(html, /class="alarm"[\s\S]*<\/svg>/);
  assert.match(html, /class="metric__label">TEMP</);
  assert.match(html, /class="metric__unit">°C</);
  assert.match(html, /HUMIDITY/);
  assert.match(html, /class="metric__unit">%</);
});

test('styles lock fixed 800x480 canvas and visual accent contracts', () => {
  const css = readFileSync(new URL('../styles.css', import.meta.url), 'utf8');
  assert.match(css, /--screen-w:\s*800px/);
  assert.match(css, /--screen-h:\s*480px/);
  assert.match(css, /\.period\s*\{[\s\S]*color:\s*var\(--digit-color\)/);
  assert.match(css, /\.colon\s*\{[\s\S]*animation:\s*colon-breathe 1\.8s ease-in-out infinite/);
  assert.match(css, /\.alarm\s*\{[\s\S]*position:\s*absolute/);
  assert.match(css, /@keyframes colon-breathe/);
});

test('digit roll animation defines roll-in and roll-out keyframes', () => {
  const css = readFileSync(new URL('../styles.css', import.meta.url), 'utf8');
  assert.match(css, /@keyframes roll-out\s*\{[\s\S]*translateY\(-100%\)/);
  assert.match(css, /@keyframes roll-in\s*\{[\s\S]*translateY\(100%\)/);
  assert.match(css, /\.digit-slot--rolling\s+\.digit-slot__incoming/);
});

test('index.html links Rubik and Noto Serif from Google Fonts', () => {
  const html = readFileSync(new URL('../index.html', import.meta.url), 'utf8');
  assert.match(html, /fonts\.googleapis\.com[\s\S]*Rubik/);
  assert.match(html, /Noto\+Serif/);
});

test('styles.css uses Rubik for digits and Noto Serif for labels', () => {
  const css = readFileSync(new URL('../styles.css', import.meta.url), 'utf8');
  assert.match(css, /--font-digit:\s*"Rubik"/);
  assert.match(css, /--font-serif:\s*"Noto Serif"/);
});

test('digit roll uses a slow, fade-free vertical slide', () => {
  const css = readFileSync(new URL('../styles.css', import.meta.url), 'utf8');
  // 纯位移，无 opacity 淡出
  assert.doesNotMatch(css, /@keyframes roll-out\s*\{[\s\S]*opacity/);
  // 时长不少于 450ms，体现“缓慢”
  const m = css.match(/--roll-dur:\s*(\d+)ms/);
  assert.ok(m && Number(m[1]) >= 450, 'roll duration should be >= 450ms');
});

test('screen has startup fade-in animation', () => {
  const css = readFileSync(new URL('../styles.css', import.meta.url), 'utf8');
  assert.match(css, /@keyframes screen-in/);
  assert.match(css, /\.screen\s*\{[\s\S]*animation:[\s\S]*screen-in/);
});
