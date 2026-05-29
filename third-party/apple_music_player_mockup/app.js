const navItems = [
  { id: "home", label: "主页", icon: "⌂" },
  { id: "radio", label: "广播", icon: "◉" },
  { id: "local", label: "本地", icon: "♫" },
  { id: "playlist", label: "歌单", icon: "≣" },
  { id: "settings", label: "设置", icon: "⚙" },
];

const settingsTabs = [
  { id: "appearance", title: "外观", desc: "主题、颜色与氛围" },
  { id: "playback", title: "播放", desc: "默认队列与过渡策略" },
  { id: "about", title: "关于", desc: "设备信息与版本说明" },
];

const themePresets = [
  { id: "cyan", label: "Cyan", desc: "淡青主调" },
  { id: "blue", label: "Blue", desc: "夜间轻冷" },
  { id: "mint", label: "Mint", desc: "清透早晨" },
  { id: "orange", label: "Orange", desc: "暖调黄金时刻" },
];

const state = {
  page: "home",
  theme: "mint",
  settingsTab: "appearance",
};

const homeData = {
  hero: {
    topline: "Live Radio Selection",
    title: "从广播开始，今天节奏已经选好",
    desc: "先听推荐广播，再回到最近播放。",
    badge: "今日主打",
  },
  recommends: [
    { title: "Lo-Fi 早班电台", subtitle: "27 min continuous mix" },
    { title: "City Pop Avenue", subtitle: "东京夜风与合成器" },
    { title: "Pure Piano Focus", subtitle: "安静但不失推进感" },
  ],
  recent: [
    { kind: "广播", title: "New Music Daily", subtitle: "Apple Music 1" },
    { kind: "歌单", title: "Coding Without Hurry", subtitle: "18 首歌曲" },
    { kind: "单曲", title: "Golden Hour", subtitle: "JVKE" },
  ],
  metrics: [
    { label: "广播收藏", value: "12" },
    { label: "连续收听", value: "48m" },
    { label: "今日推荐", value: "06" },
  ],
};

const pageData = {
  radio: {
    eyebrow: "广播目录",
    title: "广播",
    subtitle: "把首页广播优先的心智展开成频道页，适合快速挑到当下想听的气氛。",
    bannerTitle: "Apple Music 电台精选",
    bannerDesc: "从编辑推荐、DJ 专栏到 mood station，全部先做静态视觉确认，再迁回 LVGL 结构。",
    badges: ["24h Live", "Editor Picks", "Trending"],
    queueLabel: "正在预排 4 个节目",
    list: [
      { kind: "radio", title: "The New Music Station", subtitle: "全球新歌 / 每小时刷新", meta: "现在开始", aux: "LIVE" },
      { kind: "radio", title: "Chill Sunday", subtitle: "低压氛围 / 柔和女声", meta: "42 分钟", aux: "Mix" },
      { kind: "radio", title: "After Midnight Jazz", subtitle: "夜色铜管 / 慢速鼓刷", meta: "28 分钟", aux: "HD" },
      { kind: "radio", title: "Electro Run Club", subtitle: "高 BPM / 晨跑编排", meta: "56 分钟", aux: "New" },
    ],
    queue: [
      { title: "主持人开场", subtitle: "02:14 后切到主节目" },
      { title: "新歌连播", subtitle: "含 3 首首发曲目" },
      { title: "DJ 访谈", subtitle: "片段预留" },
    ],
  },
  local: {
    eyebrow: "本地资料库",
    title: "本地",
    subtitle: "先验证列表型页面在 800x480 下的信息密度、层级和留白控制。",
    bannerTitle: "最近导入的专辑与单曲",
    bannerDesc: "不接真实扫描逻辑，只用静态假数据模拟专辑、单曲和收藏混合视图。",
    badges: ["Albums", "Lossless", "Recently Added"],
    queueLabel: "资料库总量 148 项",
    list: [
      { kind: "album", title: "In Rainbows", subtitle: "Radiohead / 专辑", meta: "10 tracks", aux: "Album" },
      { kind: "album", title: "Souvlaki", subtitle: "Slowdive / 专辑", meta: "9 tracks", aux: "Album" },
      { kind: "song", title: "Nights", subtitle: "Frank Ocean / 单曲", meta: "5:07", aux: "Song" },
      { kind: "song", title: "Sunset Rollercoaster", subtitle: "My Jinji / 单曲", meta: "4:26", aux: "Fav" },
    ],
    queue: [
      { title: "下载完成", subtitle: "3 张专辑可离线播放" },
      { title: "上次播放", subtitle: "昨晚 22:14 停在 track 07" },
      { title: "同步状态", subtitle: "iCloud 占位 UI" },
    ],
  },
  playlist: {
    eyebrow: "歌单集合",
    title: "歌单",
    subtitle: "用更轻的卡片感展示个人歌单与推荐歌单，观察标题长度和二级信息的压缩方式。",
    bannerTitle: "你的歌单今天适合这样排",
    bannerDesc: "先把结构与节奏做顺，再决定 LVGL 里是 grid 卡片还是列表卡片混排。",
    badges: ["Curated", "Personal", "Smart Mix"],
    queueLabel: "收藏歌单 9 个",
    list: [
      { kind: "playlist", title: "晨间编译", subtitle: "15 首 / 稳定推进型", meta: "35 min", aux: "Pinned" },
      { kind: "playlist", title: "夜行公路", subtitle: "22 首 / 合成器和鼓机", meta: "1h 24m", aux: "Road" },
      { kind: "playlist", title: "低功耗工作流", subtitle: "13 首 / 人声克制", meta: "48 min", aux: "Focus" },
      { kind: "playlist", title: "广播回放收藏", subtitle: "8 集 / 长节目", meta: "2h 03m", aux: "Radio" },
    ],
    queue: [
      { title: "智能续播", subtitle: "按最近收听顺序推荐" },
      { title: "封面候选", subtitle: "后续可接主题色联动" },
      { title: "共享入口", subtitle: "本轮只保留视觉占位" },
    ],
  },
};

const miniPlayer = {
  title: "Lo-Fi 早班电台",
  subtitle: "Morning Transit Session",
  current: "01:42",
  total: "03:58",
  progress: "44%",
};

function render() {
  document.body.dataset.theme = state.theme;

  const app = document.getElementById("app");
  app.innerHTML = `
    <aside class="sidebar">${renderSidebar()}</aside>
    <main class="content-shell">
      <div class="content-scroll">${renderContent()}</div>
    </main>
    <footer class="mini-player">${renderMiniPlayer()}</footer>
  `;

  bindEvents();
}

function renderSidebar() {
  const primaryItems = navItems
    .filter((item) => item.id !== "settings")
    .map((item) => renderNavItem(item))
    .join("");
  const settingsItem = renderNavItem(navItems.find((item) => item.id === "settings"));

  return `
    <div class="brand">
      <div class="brand-badge">♪</div>
      <div class="brand-copy">
        <strong>Broadcast First</strong>
        <span>Apple Music mockup</span>
      </div>
    </div>
    <div class="sidebar-section-title">导航</div>
    <div class="nav-list">${primaryItems}</div>
    <div class="sidebar-footer">
      <div class="sidebar-section-title">偏好</div>
      <div class="nav-list">${settingsItem}</div>
      <div class="listener-card">
        <span>正在收听</span>
        <strong>今天先从广播开始，稍后再回到本地资料库。</strong>
        <p>当前样机只验证 UI 层次、主题联动和页面边界。</p>
      </div>
    </div>
  `;
}

function renderNavItem(item) {
  const activeClass = item.id === state.page ? "active" : "";
  return `
    <button class="nav-item ${activeClass}" data-page="${item.id}">
      <span class="nav-icon">${item.icon}</span>
      <span class="nav-label">${item.label}</span>
    </button>
  `;
}

function renderContent() {
  if (state.page === "home") {
    return renderHomePage();
  }

  if (state.page === "settings") {
    return renderSettingsPage();
  }

  return renderListPage(pageData[state.page]);
}

function renderHomePage() {
  return `
    <section class="page">
      <header class="page-header">
        <div>
          <div class="eyebrow"><span class="eyebrow-dot"></span> 广播优先首页</div>
          <h1 class="page-title">主页</h1>
          <p class="page-subtitle">早上好，现在适合先听一会广播，再从最近播放回到你的收藏。</p>
        </div>
        <div class="search-chip">
          <span class="search-glyph">⌕</span>
          <span>搜索占位，本轮不接真实功能</span>
        </div>
      </header>
      <div class="home-grid">
        <section class="hero-card">
          <div class="hero-copy">
            <div class="hero-topline">${homeData.hero.topline}</div>
            <h2 class="hero-title">${homeData.hero.title}</h2>
            <p class="hero-desc">${homeData.hero.desc}</p>
          </div>
          <div class="hero-actions hero-actions-docked">
            <button class="button button-primary">继续收听</button>
            <button class="button button-secondary">探索更多</button>
          </div>
          <div class="floating-pill">${homeData.hero.badge}</div>
          <div class="hero-visual">
            <div class="vinyl-ring"></div>
            <div class="vinyl-core"></div>
          </div>
        </section>
        <div class="side-stack">
          <section class="panel recommend-panel">
            <div class="panel-header">
              <div class="panel-title">推荐广播</div>
              <div class="linkish">查看全部</div>
            </div>
            <div class="recommend-list">
              ${homeData.recommends.map(renderRecommendItem).join("")}
            </div>
          </section>
          <section class="panel recent-panel">
            <div class="panel-header">
              <div class="panel-title">最近播放</div>
              <div class="linkish">继续</div>
            </div>
            <div class="recent-list">
              ${homeData.recent.map(renderRecentItem).join("")}
            </div>
            <div class="recent-metrics">
              ${homeData.metrics
                .map(
                  (metric) => `
                    <div class="metric">
                      <span>${metric.label}</span>
                      <strong>${metric.value}</strong>
                    </div>
                  `
                )
                .join("")}
            </div>
          </section>
        </div>
      </div>
    </section>
  `;
}

function renderRecommendItem(item) {
  return `
    <article class="recommend-item">
      <div class="recommend-cover cover-wave"></div>
      <div>
        <div class="item-title">${item.title}</div>
        <div class="item-subtitle">${item.subtitle}</div>
      </div>
      <div class="item-action">▶</div>
    </article>
  `;
}

function renderRecentItem(item) {
  return `
    <article class="recommend-item">
      <div class="recommend-cover cover-wave"></div>
      <div>
        <div class="item-title">${item.title}</div>
        <div class="item-subtitle">${item.kind} · ${item.subtitle}</div>
      </div>
      <div class="item-action">↺</div>
    </article>
  `;
}

function renderListPage(data) {
  return `
    <section class="page list-page">
      <header class="page-header">
        <div>
          <div class="eyebrow"><span class="eyebrow-dot"></span> ${data.eyebrow}</div>
          <h1 class="page-title">${data.title}</h1>
          <p class="page-subtitle">${data.subtitle}</p>
        </div>
        <div class="search-chip">
          <span class="search-glyph">⌕</span>
          <span>静态目录视图</span>
        </div>
      </header>
      <section class="feature-banner">
        <div>
          <h3>${data.bannerTitle}</h3>
          <p>${data.bannerDesc}</p>
          <div class="feature-badges">
            ${data.badges.map((badge) => `<span class="feature-badge">${badge}</span>`).join("")}
          </div>
        </div>
        <div class="feature-visual">
          <div class="feature-cardlet">
            <strong>${data.queueLabel}</strong>
            <span>先用静态密度确认布局，再迁入 LVGL flex/grid。</span>
          </div>
          <div class="feature-cardlet">
            <strong>视觉重点</strong>
            <span>玻璃卡片、主题色高亮、二级信息尽量轻。</span>
          </div>
        </div>
      </section>
      <div class="content-grid">
        <section class="panel catalog-panel">
          <div class="panel-header">
            <div class="panel-title">${data.title} 列表</div>
            <div class="linkish">4 项假数据</div>
          </div>
          <div class="catalog-list">
            ${data.list.map(renderMediaItem).join("")}
          </div>
        </section>
        <aside class="panel queue-panel">
          <div class="queue-headline">下一步结构观察</div>
          <div class="queue-badge">${data.queueLabel}</div>
          <div class="queue-list">
            ${data.queue
              .map(
                (item) => `
                  <div class="queue-item">
                    <strong>${item.title}</strong>
                    <span>${item.subtitle}</span>
                  </div>
                `
              )
              .join("")}
          </div>
        </aside>
      </div>
    </section>
  `;
}

function renderMediaItem(item) {
  return `
    <article class="media-item" data-kind="${item.kind}">
      <div class="media-cover cover-wave"></div>
      <div class="media-meta">
        <div class="item-title">${item.title}</div>
        <div class="item-subtitle">${item.subtitle}</div>
      </div>
      <div class="media-extra">
        <span>${item.meta}</span>
        <div class="item-action">${item.aux}</div>
      </div>
    </article>
  `;
}

function renderSettingsPage() {
  return `
    <section class="page">
      <header class="page-header">
        <div>
          <div class="eyebrow"><span class="eyebrow-dot"></span> Appearance Controls</div>
          <h1 class="page-title">设置</h1>
          <p class="page-subtitle">本轮重点验证主题色切换、全局 shell 稳定性，以及设置页作为独立整页的结构感。</p>
        </div>
        <div class="search-chip">
          <span class="search-glyph">◌</span>
          <span>右侧内容切页，不销毁左栏与播放条</span>
        </div>
      </header>
      <div class="settings-layout">
        <aside class="panel settings-sidebar">
          <div class="settings-nav">
            ${settingsTabs.map(renderSettingsTab).join("")}
          </div>
        </aside>
        <section class="panel settings-main">
          ${renderSettingsPanel()}
        </section>
      </div>
    </section>
  `;
}

function renderSettingsTab(tab) {
  const activeClass = tab.id === state.settingsTab ? "active" : "";
  return `
    <button class="settings-tab ${activeClass}" data-settings-tab="${tab.id}">
      <strong>${tab.title}</strong>
      <span>${tab.desc}</span>
    </button>
  `;
}

function renderSettingsPanel() {
  if (state.settingsTab === "appearance") {
    return `
      <section class="settings-card">
        <h3>主题模式</h3>
        <p>本阶段先完成浅色主方案，深色与系统跟随作为结构占位，后续在 LVGL 里保持同样信息边界。</p>
        <div class="mode-row">
          <div class="pill-option active">浅色</div>
          <div class="pill-option">深色</div>
          <div class="pill-option">跟随系统</div>
        </div>
      </section>
      <section class="settings-card">
        <h3>主题颜色</h3>
        <p>切换主题色时，只变更视觉 token，不改变导航、布局和信息结构。</p>
        <div class="theme-swatches">
          ${themePresets.map(renderThemeSwatch).join("")}
        </div>
      </section>
      <div class="settings-split">
        <section class="settings-card">
          <h3>圆角强度</h3>
          <p>偏柔和，贴近 Apple Music 的卡片与玻璃边界。</p>
          <div class="slider-row">
            <div class="slider"></div>
            <strong>62%</strong>
          </div>
        </section>
        <section class="settings-card">
          <h3>背景氛围</h3>
          <p>维持轻柔雾面，不让背景竞争信息层级。</p>
          <div class="toggle-row">
            <div class="pill-option active">柔光</div>
            <div class="pill-option">冷雾</div>
          </div>
        </section>
      </div>
      <section class="about-card">
        <span>当前主题</span>
        <strong>${themePresets.find((theme) => theme.id === state.theme).label}</strong>
        <span>联动区域：左栏选中态 / hero 渐变 / 迷你播放条进度 / 主要按钮。</span>
      </section>
    `;
  }

  if (state.settingsTab === "playback") {
    return `
      <section class="settings-card">
        <h3>播放占位</h3>
        <p>本轮不接真实播放器，但先确认设置页里的二级表单结构密度。</p>
        <div class="toggle-row">
          <div class="pill-option active">自动续播</div>
          <div class="pill-option">Crossfade 8s</div>
          <div class="pill-option">广播优先</div>
        </div>
      </section>
      <section class="settings-card">
        <h3>队列策略</h3>
        <p>进入广播页时展示最近频道；进入本地页时保持上次排序。</p>
      </section>
      <section class="about-card">
        <span>后续接入</span>
        <strong>只替换右侧内容区</strong>
        <span>这能让播放条与左栏保持稳定，降低后续接逻辑的复杂度。</span>
      </section>
    `;
  }

  return `
    <section class="settings-card">
      <h3>关于样机</h3>
      <p>这是先于 LVGL 的 HTML 高保真确认稿，目标是让布局、气质和主题联动先达成共识。</p>
    </section>
    <section class="settings-card">
      <h3>设备假设</h3>
      <p>目标分辨率 800x480，左栏常驻，右侧内容切页，底部迷你播放条固定。</p>
    </section>
    <section class="about-card">
      <span>下一步</span>
      <strong>LVGL v9 flex/grid 实现</strong>
      <span>样机确认后，再把同一信息架构迁移到 SDL 运行环境。</span>
    </section>
  `;
}

function renderThemeSwatch(theme) {
  const activeClass = theme.id === state.theme ? "active" : "";
  return `
    <button class="swatch ${activeClass}" data-theme="${theme.id}">
      <span class="swatch-preview swatch-${theme.id}"></span>
      <strong>${theme.label}</strong>
      <span>${theme.desc}</span>
    </button>
  `;
}

function renderMiniPlayer() {
  return `
    <div class="now-playing">
      <div class="player-art"></div>
      <div class="player-copy">
        <strong>${miniPlayer.title}</strong>
        <span>${miniPlayer.subtitle}</span>
      </div>
    </div>
    <div class="player-controls">
      <button class="control-button" aria-label="上一首">⏮</button>
      <button class="control-button primary" aria-label="播放">▶</button>
      <button class="control-button" aria-label="下一首">⏭</button>
    </div>
    <div class="progress-cluster" style="--progress: ${miniPlayer.progress}">
      <div class="progress-meta">
        <span>${miniPlayer.current}</span>
        <span>Now Playing</span>
        <span>${miniPlayer.total}</span>
      </div>
      <div class="progress-bar"></div>
    </div>
  `;
}

function bindEvents() {
  document.querySelectorAll("[data-page]").forEach((button) => {
    button.addEventListener("click", () => {
      state.page = button.dataset.page;
      render();
    });
  });

  document.querySelectorAll("[data-theme]").forEach((button) => {
    if (button.classList.contains("swatch")) {
      button.addEventListener("click", () => {
        state.theme = button.dataset.theme;
        render();
      });
    }
  });

  document.querySelectorAll("[data-settings-tab]").forEach((button) => {
    button.addEventListener("click", () => {
      state.settingsTab = button.dataset.settingsTab;
      render();
    });
  });
}

render();
