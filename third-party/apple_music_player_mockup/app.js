const navigationItems = [
  { id: "home", label: "主页", icon: "⌂", badge: "" },
  { id: "radio", label: "广播", icon: "◉", badge: "Live" },
  { id: "local", label: "本地", icon: "♫", badge: "24" },
  { id: "playlist", label: "歌单", icon: "≋", badge: "7" },
  { id: "settings", label: "设置", icon: "⚙", badge: "" }
];

const themeOptions = [
  {
    id: "pink",
    name: "Pink",
    description: "偏 Apple Music 经典高亮，柔和却不寡淡。",
    swatches: ["#ffd0db", "#ff7d98", "#ff3f67"]
  },
  {
    id: "blue",
    name: "Blue",
    description: "更冷一点的广播氛围，适合通勤和夜航节目。",
    swatches: ["#d5e5ff", "#80a8ff", "#3f76ff"]
  },
  {
    id: "mint",
    name: "Mint",
    description: "轻盈的新鲜感，更像清晨电台和舒缓播客。",
    swatches: ["#d7f5ea", "#76d8bf", "#26a58d"]
  },
  {
    id: "orange",
    name: "Orange",
    description: "更温暖的日落频道感，适合精选歌单与现场。",
    swatches: ["#ffe1b7", "#f8b167", "#ef8730"]
  }
];

const settingsTabs = [
  { id: "appearance", label: "外观", hint: "主题" },
  { id: "playback", label: "播放", hint: "占位" },
  { id: "about", label: "关于", hint: "信息" }
];

const data = {
  hero: {
    tag: "广播优先",
    title: "Apple Music Radio",
    description: "从一张更轻、更亮的主卡开始。把广播放在进入应用后的第一视线，先给用户一个继续收听的理由。"
  },
  radio: [
    {
      title: "Apple Pop Radio",
      subtitle: "新歌首播与榜单切片",
      eyebrow: "推荐广播",
      cover: ["#ffd0db", "#ff7394"]
    },
    {
      title: "Morning Acoustic",
      subtitle: "清晨木吉他与低密度人声",
      eyebrow: "清晨精选",
      cover: ["#d2f2e9", "#70d0bc"]
    },
    {
      title: "City Drive FM",
      subtitle: "通勤速度感与电子流行",
      eyebrow: "路上听什么",
      cover: ["#dbe5ff", "#7c96ff"]
    }
  ],
  recents: [
    {
      title: "静谧空间",
      subtitle: "最近播放 · 广播",
      eyebrow: "继续收听",
      cover: ["#ffe4b8", "#f6a753"]
    },
    {
      title: "Late Night Notes",
      subtitle: "最近播放 · 播客",
      eyebrow: "夜间节目",
      cover: ["#d9dbff", "#8b7dff"]
    },
    {
      title: "Studio Drafts",
      subtitle: "最近播放 · 歌单",
      eyebrow: "收藏歌单",
      cover: ["#fad1eb", "#f186bd"]
    }
  ],
  localTracks: [
    {
      title: "Amber Window",
      subtitle: "Miya Lane · 3:42",
      eyebrow: "本地曲库",
      cover: ["#fde7be", "#f8b15f"]
    },
    {
      title: "Soft Focus",
      subtitle: "June Harbor · 4:08",
      eyebrow: "最近导入",
      cover: ["#dff2ff", "#77beff"]
    },
    {
      title: "Paper Sun",
      subtitle: "North Vale · 2:54",
      eyebrow: "精选单曲",
      cover: ["#f8d8e9", "#f487b4"]
    },
    {
      title: "Cabin Light",
      subtitle: "Sora Kim · 3:26",
      eyebrow: "离线可播",
      cover: ["#d8f3ea", "#5cc9b4"]
    }
  ],
  playlists: [
    {
      title: "日落通勤",
      subtitle: "更暖的合成器、恰到好处的鼓点和晚高峰。",
      pill: "12 首",
      cover: ["rgba(255, 220, 180, 0.72)", "rgba(246, 145, 89, 0.82)"]
    },
    {
      title: "低饱和书房",
      subtitle: "把注意力拉回文字和留白，适合慢速工作。",
      pill: "8 首",
      cover: ["rgba(217, 238, 255, 0.72)", "rgba(101, 145, 255, 0.8)"]
    },
    {
      title: "玻璃感电子",
      subtitle: "更贴近这版界面的材质，空灵但有节奏骨架。",
      pill: "14 首",
      cover: ["rgba(255, 213, 236, 0.72)", "rgba(246, 121, 174, 0.84)"]
    },
    {
      title: "晨雾吉他",
      subtitle: "拨弦靠前，氛围在后，适合把音量开得很轻。",
      pill: "10 首",
      cover: ["rgba(209, 247, 234, 0.76)", "rgba(79, 200, 168, 0.82)"]
    }
  ],
  nowPlaying: {
    title: "In the Quiet Air",
    artist: "Aster Bloom · Radio Session",
    eyebrow: "正在播放",
    cover: ["#ffd2de", "#f36a90"],
    progress: 46,
    elapsed: "1:42",
    duration: "3:48"
  },
  ambientTags: ["广播优先", "浅色玻璃", "800×480", "静态假数据"],
  categoryNotes: {
    radio: {
      title: "广播页占位",
      text: "强调频道、节目与立即收听的路径，布局保持视觉完整，但目前全部为静态假数据。"
    },
    local: {
      title: "本地页占位",
      text: "预留未来接入本地扫描与媒体库的结构，现在只验证列表节奏、密度和层级。"
    },
    playlist: {
      title: "歌单页占位",
      text: "用卡片承接主题化歌单和推荐入口，先把色块、留白和信息层级站稳。"
    }
  }
};

const state = {
  page: "home",
  theme: "pink",
  themeMode: "light",
  settingsTab: "appearance"
};

function appShell() {
  return `
    <div class="mockup">
      <div class="device">
        <div class="device__screen">
          <div class="app-shell">
            <aside class="sidebar glass">${renderSidebar()}</aside>
            <main class="content-shell glass">${renderContent()}</main>
            <footer class="mini-player glass">${renderMiniPlayer()}</footer>
          </div>
        </div>
      </div>
    </div>
  `;
}

function renderSidebar() {
  const mainItems = navigationItems.filter((item) => item.id !== "settings");
  const settingsItem = navigationItems.find((item) => item.id === "settings");

  return `
    <div class="sidebar__brand">
      <div class="sidebar__logo">♪</div>
      <div>
        <span class="sidebar__eyebrow">Now in glass</span>
        <span class="sidebar__title">Music Mockup</span>
      </div>
    </div>
    <nav class="sidebar__nav" aria-label="主导航">
      <div class="sidebar__nav-group">
        ${mainItems.map(renderNavItem).join("")}
      </div>
    </nav>
    <div class="sidebar__footer">
      ${renderNavItem(settingsItem)}
    </div>
  `;
}

function renderNavItem(item) {
  const activeClass = item.id === state.page ? " is-active" : "";
  return `
    <button class="nav-item${activeClass}" type="button" data-action="page" data-page="${item.id}">
      <span class="nav-item__icon">${item.icon}</span>
      <span class="nav-item__label">${item.label}</span>
      <span class="nav-item__meta">${item.badge}</span>
    </button>
  `;
}

function renderContent() {
  switch (state.page) {
    case "home":
      return renderHomePage();
    case "radio":
      return renderListPage({
        label: "全天在线",
        title: "广播",
        subtitle: "把首页那张 hero 拆成更完整的频道视图，继续保持广播优先的进入路径。",
        items: data.radio,
        sideTitle: data.categoryNotes.radio.title,
        sideText: data.categoryNotes.radio.text,
        sideBadges: ["Live", "精选主持", "每周更新"]
      });
    case "local":
      return renderListPage({
        label: "离线资料库",
        title: "本地",
        subtitle: "静态曲库列表先验证层次、字重和横向空间，未来再接入真实文件扫描。",
        items: data.localTracks,
        sideTitle: data.categoryNotes.local.title,
        sideText: data.categoryNotes.local.text,
        sideBadges: ["无损", "最近导入", "按专辑排序"]
      });
    case "playlist":
      return renderPlaylistPage();
    case "settings":
      return renderSettingsPage();
    default:
      return "";
  }
}

function renderHomePage() {
  return `
    <section class="page">
      <header class="page-header">
        <div>
          <p class="page-header__label">广播优先首页</p>
          <h1 class="page-header__title">主页</h1>
          <p class="page-header__subtitle">早上好，现在适合从广播开始。先给用户一张足够轻盈的主卡，再把继续收听和回流路径安静地摆在右侧。</p>
        </div>
        <div class="search-pill" aria-hidden="true">
          <span>⌕</span>
          <span class="search-pill__field">搜索音乐、节目或电台</span>
        </div>
      </header>
      <div class="home-grid">
        <div class="home-main">
          <section class="hero-card">
            <div class="hero-card__layout">
              <div>
                <span class="hero-card__tag">${data.hero.tag}</span>
                <h2 class="hero-card__title">${data.hero.title}</h2>
                <p class="hero-card__desc">${data.hero.description}</p>
                <div class="hero-card__actions">
                  <button class="button-primary" type="button">继续收听</button>
                  <button class="button-secondary" type="button">探索更多</button>
                </div>
              </div>
              <div class="hero-art" aria-hidden="true">
                <div class="hero-art__vinyl"></div>
              </div>
            </div>
          </section>
          <section class="section-card glass">
            <div class="section-card__header">
              <h3 class="section-card__title">推荐广播</h3>
              <span class="section-card__meta">3 个频道</span>
            </div>
            <div class="radio-list">
              ${data.radio.map((item) => mediaRow(item, true)).join("")}
            </div>
          </section>
        </div>
        <aside class="recent-panel">
          <section class="ambient-note glass">
            <p class="ambient-note__label">当前气质</p>
            <h3 class="ambient-note__title">浅色玻璃、柔和渐变、强留白。</h3>
            <p class="ambient-note__text">主题色会同时驱动 hero 主色、左栏选中态和底部进度高亮。设置页里切换色卡后，首页和播放条立即联动。</p>
          </section>
          <section class="section-card glass">
            <div class="section-card__header">
              <h3 class="section-card__title">最近播放</h3>
              <span class="section-card__meta">继续回流</span>
            </div>
            <div class="recent-list">
              ${data.recents.map((item) => mediaRow(item, false)).join("")}
            </div>
          </section>
        </aside>
      </div>
    </section>
  `;
}

function renderListPage(config) {
  return `
    <section class="page">
      <header class="page-header">
        <div>
          <p class="page-header__label">${config.label}</p>
          <h1 class="page-header__title">${config.title}</h1>
          <p class="page-header__subtitle">${config.subtitle}</p>
        </div>
        <div class="search-pill" aria-hidden="true">
          <span>⌕</span>
          <span class="search-pill__field">筛选与搜索暂作占位</span>
        </div>
      </header>
      <div class="list-page">
        <section class="list-page__main">
          <div class="section-card glass">
            <div class="section-card__header">
              <h3 class="section-card__title">${config.title}列表</h3>
              <span class="section-card__meta">静态内容</span>
            </div>
            <div class="list-stack">
              ${config.items.map((item) => mediaRow(item, true)).join("")}
            </div>
          </div>
        </section>
        <aside class="list-page__side">
          <div class="side-note glass">
            <h3 class="side-note__title">${config.sideTitle}</h3>
            <p class="side-note__text">${config.sideText}</p>
            <div class="badge-list">
              ${config.sideBadges.map((badge) => `<span class="badge">${badge}</span>`).join("")}
            </div>
          </div>
          <div class="section-card glass">
            <div class="section-card__header">
              <h3 class="section-card__title">最近播放</h3>
            </div>
            <div class="recent-list">
              ${data.recents.slice(0, 2).map((item) => mediaRow(item, false)).join("")}
            </div>
          </div>
        </aside>
      </div>
    </section>
  `;
}

function renderPlaylistPage() {
  return `
    <section class="page">
      <header class="page-header">
        <div>
          <p class="page-header__label">主题化集合</p>
          <h1 class="page-header__title">歌单</h1>
          <p class="page-header__subtitle">这里不做复杂逻辑，先把卡片密度、色块关系与更明显的 editorial 感做完整。</p>
        </div>
        <div class="search-pill" aria-hidden="true">
          <span>⌕</span>
          <span class="search-pill__field">未来可放搜索与筛选</span>
        </div>
      </header>
      <div class="list-page">
        <section class="list-page__main">
          <div class="list-grid">
            ${data.playlists.map(playlistCard).join("")}
          </div>
        </section>
        <aside class="list-page__side">
          <div class="side-note glass">
            <h3 class="side-note__title">${data.categoryNotes.playlist.title}</h3>
            <p class="side-note__text">${data.categoryNotes.playlist.text}</p>
            <div class="badge-list">
              <span class="badge">编辑推荐</span>
              <span class="badge">主题歌单</span>
              <span class="badge">静态占位</span>
            </div>
          </div>
          <div class="section-card glass">
            <div class="section-card__header">
              <h3 class="section-card__title">视觉标签</h3>
            </div>
            <div class="badge-list">
              ${data.ambientTags.map((tag) => `<span class="badge">${tag}</span>`).join("")}
            </div>
          </div>
        </aside>
      </div>
    </section>
  `;
}

function renderSettingsPage() {
  return `
    <section class="page">
      <header class="page-header">
        <div>
          <p class="page-header__label">独立整页设置</p>
          <h1 class="page-header__title">设置</h1>
          <p class="page-header__subtitle">右侧内容区保留整页结构，当前重点完成主题色卡。外观、播放、关于三组都可切换，但只有外观做完整联动。</p>
        </div>
      </header>
      <div class="settings-layout">
        <aside class="settings-nav glass">
          <p class="settings-nav__title">分组</p>
          ${settingsTabs.map(renderSettingsTab).join("")}
        </aside>
        <section class="settings-layout__detail">
          ${renderSettingsDetail()}
        </section>
      </div>
    </section>
  `;
}

function renderSettingsTab(tab) {
  const activeClass = tab.id === state.settingsTab ? " is-active" : "";
  return `
    <button class="settings-tab${activeClass}" type="button" data-action="settings-tab" data-tab="${tab.id}">
      <span>${tab.label}</span>
      <span class="settings-tab__hint">${tab.hint}</span>
    </button>
  `;
}

function renderSettingsDetail() {
  if (state.settingsTab === "playback") {
    return `
      <article class="settings-card glass">
        <h3 class="settings-card__title">播放</h3>
        <p class="settings-card__desc">当前阶段不接真实播放能力，这里只保留几项结构占位，方便后续接入状态和开关。</p>
        <div class="setting-line">
          <div>
            <p class="setting-line__title">自动续播</p>
            <p class="setting-line__desc">样机阶段固定开启，仅展示交互样式。</p>
          </div>
          <span class="setting-line__value">已开启</span>
        </div>
        <div class="setting-line">
          <div>
            <p class="setting-line__title">音质偏好</p>
            <p class="setting-line__desc">未来可分流媒体源，这里先展示为 Lossless。</p>
          </div>
          <span class="setting-line__value">Lossless</span>
        </div>
      </article>
    `;
  }

  if (state.settingsTab === "about") {
    return `
      <article class="settings-card glass">
        <h3 class="settings-card__title">关于</h3>
        <p class="settings-card__desc">这是第一版 HTML 高保真样机：只验证风格、结构与主题联动，不涉及真实音频、文件扫描或广播接入。</p>
        <div class="setting-line">
          <div>
            <p class="setting-line__title">目标视口</p>
            <p class="setting-line__desc">固定在 800×480 横屏设备壳里预览。</p>
          </div>
          <span class="setting-line__value">800×480</span>
        </div>
        <div class="setting-line">
          <div>
            <p class="setting-line__title">技术栈</p>
            <p class="setting-line__desc">原生 HTML / CSS / JS，无构建系统。</p>
          </div>
          <span class="setting-line__value">Vanilla</span>
        </div>
      </article>
    `;
  }

  return `
    <article class="settings-card glass">
      <h3 class="settings-card__title">外观</h3>
      <p class="settings-card__desc">浅色模式优先完成，深色与跟随系统先保留结构。主题色卡会直接驱动首页 hero、选中导航、按钮强调色和底部进度高亮。</p>
      <div class="segmented" role="tablist" aria-label="主题模式">
        ${["light", "dark", "system"].map((mode) => segmentedModeButton(mode)).join("")}
      </div>
    </article>
    <article class="settings-card glass">
      <h3 class="settings-card__title">主题颜色</h3>
      <p class="settings-card__desc">四套 token 只改视觉主色，不改信息架构。当前默认使用 Pink。</p>
      <div class="theme-grid">
        ${themeOptions.map(themeCard).join("")}
      </div>
    </article>
    <article class="settings-card glass">
      <h3 class="settings-card__title">外观细节</h3>
      <p class="settings-card__desc">这里只做 UI 可见，不要求真实联动。保留圆角和背景氛围两组占位，方便下一轮决定是否做成动态 token。</p>
      <div class="toggle-row">
        <button class="toggle-chip is-active" type="button">圆角强度 · 柔和</button>
        <button class="toggle-chip" type="button">背景氛围 · Clear Glass</button>
        <button class="toggle-chip" type="button">标题字重 · Bold</button>
      </div>
    </article>
  `;
}

function segmentedModeButton(mode) {
  const labels = {
    light: "浅色",
    dark: "深色",
    system: "跟随系统"
  };
  const activeClass = state.themeMode === mode ? " is-active" : "";
  return `
    <button class="segmented__button${activeClass}" type="button" data-action="theme-mode" data-mode="${mode}">
      ${labels[mode]}
    </button>
  `;
}

function themeCard(theme) {
  const activeClass = theme.id === state.theme ? " is-active" : "";
  return `
    <button class="theme-card${activeClass}" type="button" data-action="theme" data-theme="${theme.id}">
      ${theme.id === state.theme ? '<span class="theme-card__check">✓</span>' : ""}
      <div class="theme-card__swatch">
        ${theme.swatches.map((color) => `<span style="background:${color}"></span>`).join("")}
      </div>
      <p class="theme-card__title">${theme.name}</p>
      <p class="theme-card__desc">${theme.description}</p>
    </button>
  `;
}

function playlistCard(item) {
  return `
    <article class="playlist-card glass" style="${coverStyle(item.cover)}">
      <p class="playlist-card__label">编辑歌单</p>
      <h3 class="playlist-card__title">${item.title}</h3>
      <p class="playlist-card__subtitle">${item.subtitle}</p>
      <span class="playlist-card__pill">${item.pill}</span>
    </article>
  `;
}

function mediaRow(item, showAction) {
  return `
    <article class="media-row">
      <div class="media-row__cover" style="${coverStyle(item.cover)}"></div>
      <div>
        <p class="media-row__eyebrow">${item.eyebrow}</p>
        <h4 class="media-row__title">${item.title}</h4>
        <p class="media-row__subtitle">${item.subtitle}</p>
      </div>
      ${showAction ? '<button class="media-row__action" type="button">▶</button>' : '<span class="media-row__action" aria-hidden="true">···</span>'}
    </article>
  `;
}

function renderMiniPlayer() {
  const current = data.nowPlaying;
  return `
    <div class="mini-player__now">
      <div class="mini-player__cover" style="${coverStyle(current.cover)}"></div>
      <div>
        <p class="mini-player__eyebrow">${current.eyebrow}</p>
        <h3 class="mini-player__title">${current.title}</h3>
        <p class="mini-player__subtitle">${current.artist}</p>
      </div>
    </div>
    <div class="mini-player__controls" aria-label="播放控制">
      <button class="player-btn" type="button" aria-label="上一首">◁</button>
      <button class="player-btn player-btn--play" type="button" aria-label="播放">▶</button>
      <button class="player-btn" type="button" aria-label="下一首">▷</button>
    </div>
    <div class="mini-player__progress">
      <div class="mini-player__time">
        <span>${current.elapsed}</span>
        <span>${current.duration}</span>
      </div>
      <div class="progress-bar" aria-hidden="true">
        <div class="progress-bar__fill" style="--progress:${current.progress}%"></div>
      </div>
    </div>
  `;
}

function coverStyle(colors) {
  return `--cover-a:${colors[0]};--cover-b:${colors[1]};`;
}

function bindEvents(root) {
  root.querySelectorAll("[data-action='page']").forEach((button) => {
    button.addEventListener("click", () => {
      state.page = button.dataset.page;
      render();
    });
  });

  root.querySelectorAll("[data-action='theme']").forEach((button) => {
    button.addEventListener("click", () => {
      state.theme = button.dataset.theme;
      render();
    });
  });

  root.querySelectorAll("[data-action='settings-tab']").forEach((button) => {
    button.addEventListener("click", () => {
      state.settingsTab = button.dataset.tab;
      render();
    });
  });

  root.querySelectorAll("[data-action='theme-mode']").forEach((button) => {
    button.addEventListener("click", () => {
      state.themeMode = button.dataset.mode;
      render();
    });
  });
}

function applyBodyState() {
  document.body.dataset.theme = state.theme;
  document.body.dataset.page = state.page;
  document.body.dataset.themeMode = state.themeMode;
}

function render() {
  applyBodyState();
  const root = document.getElementById("app");
  root.innerHTML = appShell();
  bindEvents(root);
}

render();
