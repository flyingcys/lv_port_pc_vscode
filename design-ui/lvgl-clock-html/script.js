/* ========================================================================
   横屏电子时钟 800×480 —— 行为逻辑
   - 读取本机系统时间，按“下一秒边界”对齐刷新（避免长跑漂移）
   - 仅发生变化的数字位向上滚动，未变化的保持静止
   - 进位时（如 59→00）多个数字位同时滚动
   - 以经典脚本方式编写，可直接 file:// 打开；末尾按需导出供测试使用
   ======================================================================== */

const MONTHS = [
  'JANUARY', 'FEBRUARY', 'MARCH', 'APRIL', 'MAY', 'JUNE',
  'JULY', 'AUGUST', 'SEPTEMBER', 'OCTOBER', 'NOVEMBER', 'DECEMBER',
];

const WEEKDAYS = [
  'SUNDAY', 'MONDAY', 'TUESDAY', 'WEDNESDAY', 'THURSDAY', 'FRIDAY', 'SATURDAY',
];

/* 固定示例值（与素材一致，设备默认展示） */
const TEMP_VALUE = '25.6';
const HUMIDITY_VALUE = '65';

function getMonthLabel(date) {
  return MONTHS[date.getMonth()];
}

function getDayLabel(date) {
  return WEEKDAYS[date.getDay()];
}

/* 时段映射：与参考图一致，22:00 仍属 EVENING，23:00 起进入 NIGHT */
function getPeriodLabel(hour) {
  if (hour >= 5 && hour <= 11) return 'MORNING';
  if (hour >= 12 && hour <= 16) return 'AFTERNOON';
  if (hour >= 17 && hour <= 22) return 'EVENING';
  return 'NIGHT';
}

/* 把当前时间拆成 6 个字符：H1 H2 M1 M2 S1 S2 */
function getTimeParts(date) {
  const pad = (n) => String(n).padStart(2, '0');
  return [
    ...pad(date.getHours()),
    ...pad(date.getMinutes()),
    ...pad(date.getSeconds()),
  ];
}

/* 距离下一整秒的毫秒数，用于对齐刷新 */
function getDelayToNextSecond(date) {
  return 1000 - date.getMilliseconds();
}

function buildViewModel(date) {
  return {
    year: String(date.getFullYear()),
    month: getMonthLabel(date),
    date: String(date.getDate()),
    weekday: getDayLabel(date),
    period: getPeriodLabel(date.getHours()),
    digits: getTimeParts(date),
    temp: TEMP_VALUE,
    humidity: HUMIDITY_VALUE,
  };
}

/* ---- 单个数字位滚动 ----
   新数字从底部进入并停稳，旧数字向上移出并淡出。
   值未变化时保持静止，不产生任何动画。 */
function animateDigit(slot, nextValue) {
  const next = String(nextValue);
  const current = slot.querySelector('.digit-slot__current') || slot.firstElementChild;
  const currentValue = current ? current.textContent : null;

  if (currentValue === next) {
    if (current && current.className !== 'digit-slot__current') {
      current.className = 'digit-slot__current';
    }
    return;
  }

  if (current && current.className !== 'digit-slot__current') {
    current.className = 'digit-slot__current';
  }

  const incoming = document.createElement('div');
  incoming.className = 'digit-slot__incoming';
  incoming.textContent = next;
  slot.append(incoming);

  incoming.addEventListener(
    'animationend',
    () => {
      incoming.className = 'digit-slot__current';
      if (current) {
        if (typeof current.remove === 'function') {
          current.remove();
        } else {
          current.className = 'digit-slot__retired';
        }
      }
      slot.classList.remove('digit-slot--rolling');
    },
    { once: true },
  );

  requestAnimationFrame(() => {
    slot.classList.add('digit-slot--rolling');
  });
}

function hasCompleteClockDom(dom) {
  if (!dom) return false;
  const textNodes = [dom.year, dom.month, dom.date, dom.weekday, dom.period, dom.temp, dom.humidity];
  if (textNodes.some((node) => !node)) return false;
  return Array.isArray(dom.slots) && dom.slots.length === 6;
}

function renderClock(dom, viewModel) {
  dom.year.textContent = viewModel.year;
  dom.month.textContent = viewModel.month;
  dom.date.textContent = viewModel.date;
  dom.weekday.textContent = viewModel.weekday;
  dom.period.textContent = viewModel.period;
  dom.temp.textContent = viewModel.temp;
  dom.humidity.textContent = viewModel.humidity;

  viewModel.digits.forEach((digit, index) => {
    animateDigit(dom.slots[index], digit);
  });
}

/* ---- 启动 ---- */
function collectDom() {
  const slots = Array.prototype.slice.call(document.querySelectorAll('.slot'));
  return {
    year: document.getElementById('year'),
    month: document.getElementById('month'),
    date: document.getElementById('day'),
    weekday: document.getElementById('weekday'),
    period: document.getElementById('period'),
    temp: document.getElementById('temp'),
    humidity: document.getElementById('humidity'),
    slots,
  };
}

/* 把 800×480 屏幕等比缩放以适配任意窗口 */
function fitScreen() {
  const screen = document.getElementById('screen');
  if (!screen) return;
  const margin = 24;
  const scale = Math.min(
    (window.innerWidth - margin) / 800,
    (window.innerHeight - margin) / 480,
    1.6,
  );
  screen.style.transform = `scale(${scale})`;
}

/* 直接写入数字（不滚动），用于冻结/首帧静态展示 */
function setDigit(slot, value) {
  slot.innerHTML = '';
  const node = document.createElement('div');
  node.className = 'digit-slot__current';
  node.textContent = String(value);
  slot.append(node);
}

/* 解析 ?freeze= 参数，支持 "HH:MM:SS" 或完整日期时间字符串 */
function parseFreeze(raw) {
  if (!raw) return null;
  let date;
  if (/^\d{1,2}:\d{2}:\d{2}$/.test(raw)) {
    const [h, m, s] = raw.split(':').map(Number);
    date = new Date();
    date.setHours(h, m, s, 0);
  } else {
    date = new Date(raw);
  }
  return Number.isNaN(date.getTime()) ? null : date;
}

function startClock() {
  const dom = collectDom();
  if (!hasCompleteClockDom(dom)) return;

  fitScreen();
  window.addEventListener('resize', fitScreen);

  // 冻结模式：渲染固定时间，不滚动、不走时（便于对照素材 / 截图）
  const frozen = parseFreeze(new URLSearchParams(window.location.search).get('freeze'));
  if (frozen) {
    const vm = buildViewModel(frozen);
    dom.year.textContent = vm.year;
    dom.month.textContent = vm.month;
    dom.date.textContent = vm.date;
    dom.weekday.textContent = vm.weekday;
    dom.period.textContent = vm.period;
    dom.temp.textContent = vm.temp;
    dom.humidity.textContent = vm.humidity;
    vm.digits.forEach((digit, index) => setDigit(dom.slots[index], digit));
    return;
  }

  // 首帧静态渲染，避免开机瞬间六位数字同时滚动
  const paintStatic = () => {
    const vm = buildViewModel(new Date());
    dom.year.textContent = vm.year;
    dom.month.textContent = vm.month;
    dom.date.textContent = vm.date;
    dom.weekday.textContent = vm.weekday;
    dom.period.textContent = vm.period;
    dom.temp.textContent = vm.temp;
    dom.humidity.textContent = vm.humidity;
    vm.digits.forEach((digit, index) => setDigit(dom.slots[index], digit));
  };

  let timer = null;
  const tick = () => {
    renderClock(dom, buildViewModel(new Date()));
    timer = setTimeout(tick, getDelayToNextSecond(new Date()));
  };

  paintStatic();
  timer = setTimeout(tick, getDelayToNextSecond(new Date()));

  // 标签页重新可见时立即校正（直接定格，不滚动），避免休眠后大幅跳动
  document.addEventListener('visibilitychange', () => {
    if (!document.hidden) {
      clearTimeout(timer);
      paintStatic();
      timer = setTimeout(tick, getDelayToNextSecond(new Date()));
    }
  });
}

if (typeof document !== 'undefined' && typeof window !== 'undefined') {
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', startClock);
  } else {
    startClock();
  }
}

/* 供 Node 测试使用（浏览器经典脚本下 module 不存在，自动跳过） */
if (typeof module !== 'undefined' && module.exports) {
  module.exports = {
    buildViewModel,
    animateDigit,
    getMonthLabel,
    getDayLabel,
    getPeriodLabel,
    getTimeParts,
    getDelayToNextSecond,
    hasCompleteClockDom,
    renderClock,
  };
}
