# Apple Music 完整还原 + 播放稳健化 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 `main/src/v9_apple_music` 在 800×480 下做到与 `design-ui/music/music-player-macos.html` 视觉结构/风格/图标/封面一致,并把播放接线(基于 `music_player.h` → hls_player_demo)改稳健(一次 init+select 切歌、拖动 seek、4 态模式、音量/静音、与 lv_demo_music 解耦)。

**Architecture:** 复用现有 `v9_apple_music` 骨架(sidebar + main + header + content 三视图 + player bar + 播放列表弹层),不重写。视觉用 LVGL 原生控件/样式 + 真实矢量图标字体 + 烘焙火焰封面 PNG。播放复用 `music_player.h` 全局单例封装(进程级单音频)。验证 = headless 渲染 mockup 为 PNG 基线 + LVGL `lv_snapshot` 出图,逐块 Read 比对。

**Tech Stack:** C (LVGL v9) + SDL(模拟器) + CMake;hls_player_demo(player_controller/stream_player/ALSA);headless google-chrome(mockup 渲染)+ ImageMagick `convert`;`npx lv_font_conv`(图标/字体子集)+ `lvgl/scripts/LVGLImage.py`(PNG→C)。

## Global Constraints

- 支持 macOS / Linux 编译运行,采用 SDL(项目 CLAUDE.md)。
- 分辨率自适应:800×480 / 640×480 / 480×272;**800×480 为像素级还原基准**,640/480 保证自适应可用(480 单列堆叠)。
- 视觉唯一来源:`design-ui/music/music-player-macos.html`。**数据 = 真实**(test_file 扫描 + sources.tsv),屏幕文案反映真实文件/流,不是 mockup 假名。
- 不改 `main/src/local_music_demo/music_player.c` 与 `main/inc/music_player.h`(除非接口不足,且仅小范围扩展)。
- 不改 desktop 图标映射(`Apple Music → apple_music_app_launch`)。
- 每个 UI 任务的验收 = 截图与 mockup 对应视图/态并排比对,差异修到肉眼一致。
- 提交信息用中文;仅在用户要求时才 `git push`。当前分支 `release/v9.5-destop-music`(非 master),可在此分支提交。
- 颜色静态表用 `LV_COLOR_MAKE`/自定义 `C()` 宏(`lv_color_t` 内存序 {B,G,R}),运行期才用 `lv_color_hex()`。
- 装饰性容器显式清 `LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE`;可点元素显式加 `CLICKABLE`。

## 关键路径 / 命令速查

- 构建:`cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && make -C build main -j$(nproc)` → 产物 `bin/main`。
- **已验证**:本机 SDL 无法建窗(offscreen/dummy 均不可用)。`main.c` 的 `hal_init` 已加"AM_SHOT 触发的软件显示(`lv_display_create` + no-op flush + 真实时钟 tick)"分支,`lv_snapshot` 出图正常(已冒烟 design_fruit → 800×480 非黑 PNG)。
- headless LVGL 截图(无需 SDL 环境变量,`AM_SHOT` 自动走软件显示):
  `AM_APP=apple_music AM_PAGE=<now|radio|fav> AM_SHOT=/tmp/lv.ppm ./bin/main && convert /tmp/lv.ppm /tmp/lv.png`
- mockup 基线(Task 1 落地 `render.html`):
  `google-chrome --headless=new --disable-gpu --no-sandbox --hide-scrollbars --force-device-scale-factor=1 --run-all-compositor-stages-before-draw --virtual-time-budget=4000 --screenshot=/tmp/mk_now.png --window-size=800,480 "file://$PWD/design-ui/music/render.html?view=now"`
- 图标字体子集:`npx -y lv_font_conv --no-compress --format lvgl --bpp 4 --size <px> --font <icons.ttf> --range <0x..,..> --lv-font-name <name> -o <out.c>`
- PNG→C:`python3 lvgl/scripts/LVGLImage.py --ofmt C --cf ARGB8888 -o <outdir> <in.png>`(符号名=输入文件名)

---

### Task 1: 验证回路地基(build + headless 启动 + mockup 基线)

**目的:** 建立"两边都出 PNG"的比对回路——后续每个视觉任务都靠它验收。这是地基,先做。

**Files:**
- Modify: `main/src/main.c`(✅已完成:`hal_init` 加 AM_SHOT 触发的软件显示分支;**待做**:`AM_APP` 分支加 `apple_music`)
- Modify: `main/src/v9_apple_music/apple_music.h`(如需暴露设置初始视图/暂停态的 setter)
- Modify: `main/src/v9_apple_music/apple_music.c`(读 `AM_PAGE`/`AM_PAUSED` 设初始态)
- Create: `design-ui/music/render.html`(去装饰、支持 `?view=` 的隔离渲染页)
- Create: `scripts/am_shots.sh`(批量出图:mockup + LVGL 各视图/态)

**Interfaces:**
- Produces:
  - `main.c` 支持 `AM_APP=apple_music`,配合 `AM_PAGE∈{now,radio,fav}`、`AM_PAUSED=1`、`AM_SHOT=<ppm>` 出图退出。
  - `apple_music_create()`(已存在)在 `lv_screen_active()` 建 UI;初始 view 由 env 决定。
  - `scripts/am_shots.sh` 产出 `/tmp/am/mk_<view>.png` 与 `/tmp/am/lv_<view>.png`。

- [ ] **Step 1: 确认构建通过** — `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && make -C build main -j$(nproc)`;期望 `bin/main` 生成,`echo $?`=0。

- [ ] **Step 2: main.c 加 apple_music 分支** — 在 `AM_APP` 处理块(现有 calc/2048/… 之后)加:

```c
} else if(app && strcmp(app, "apple_music") == 0) {
  fprintf(stderr, "DBG: launching apple_music\n"); fflush(stderr);
  apple_music_create();
}
```
并在文件顶部 `#include "v9_apple_music/apple_music.h"`(注意 include 路径:根 CMake 已 `include_directories(main/src)`? 若否则用相对路径 `#include "apple_music.h"` 并补 include dir——实现时按报错调整)。

- [ ] **Step 3: apple_music.c 消费 AM_PAGE/AM_PAUSED** — 在 `am_build_root` 末尾用 `getenv` 决定初始 view 与暂停态:

```c
const char *pg = getenv("AM_PAGE");
am_view_t init = AM_VIEW_NOW;
if(pg && strcmp(pg,"radio")==0) init = AM_VIEW_RADIO;
else if(pg && strcmp(pg,"fav")==0) init = AM_VIEW_FAVORITES;
am_show_view(init);
```
(替换现有末尾固定的 `am_show_view(AM_VIEW_NOW)`。)`AM_PAUSED` 的消费待 Task 4/6 有暂停态渲染后接入,本任务先占位注释。

- [ ] **Step 4: 建 render.html** — 拷贝 mockup 为隔离渲染页,去 `place-items:center` 舞台、让 `.window` 占满视口,后置脚本读 `?view=`:

```html
<!DOCTYPE html><html lang="zh-CN"><head><meta charset="UTF-8">
<style>html,body{margin:0;padding:0;overflow:hidden;background:#fff;display:block!important}
.window{margin:0!important}</style></head><body>
<!-- 复用 mockup 的 .window 全部 DOM;最简做法:整段 <body> 内容从 music-player-macos.html 复制 -->
<!-- ...(复制 mockup 的 .window ... /script 全部内容)... -->
<script>const q=new URLSearchParams(location.search); if(q.get('view')) showView(q.get('view')); if(q.has('paused')){playing=false;render();} if(q.has('playlist')) openPlaylist();</script>
</body></html>
```
实现提示:直接 `cp design-ui/music/music-player-macos.html design-ui/music/render.html`,再把 `html,body{...place-items:center...}` 改成上面的占满视口样式,并在 `</script>` 前追加读 `?view=` 的脚本(mockup 的 `showView/openPlaylist/render` 是顶层函数,可直接调)。

- [ ] **Step 5: 出 mockup 基线三视图** — 建 `/tmp/am/`,跑三次 chrome:
```bash
mkdir -p /tmp/am
for V in now radio fav; do
 google-chrome --headless=new --disable-gpu --no-sandbox --hide-scrollbars \
  --force-device-scale-factor=1 --run-all-compositor-stages-before-draw \
  --virtual-time-budget=4000 --screenshot=/tmp/am/mk_$V.png --window-size=800,480 \
  "file://$PWD/design-ui/music/render.html?view=$V"; done
```
期望:三张 PNG 生成,`identify /tmp/am/mk_now.png` 为 800x480。

- [ ] **Step 6: 出 LVGL 当前态三视图** — 
```bash
make -C build main -j$(nproc)
for V in now radio fav; do
 SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy AM_APP=apple_music AM_PAGE=$V \
   AM_SHOT=/tmp/am/lv_$V.ppm ./bin/main && convert /tmp/am/lv_$V.ppm /tmp/am/lv_$V.png; done
```
期望:退出码 0,三张 lv_*.png 生成(800x480,非全黑)。(出图机制已验证:`hal_init` 的 AM_SHOT 软件显示分支 + `maybe_take_snapshot`;无需 SDL。)把命令固化进 `scripts/am_shots.sh`。

- [ ] **Step 7: 首次比对(建立基线认知)** — 主线程 `Read /tmp/am/mk_now.png` 与 `/tmp/am/lv_now.png` 并列对比,记录差异清单(播放条 meta、图标、封面、列表副标…)。此清单指导后续任务。

- [ ] **Step 8: Commit** —
```bash
git add main/src/main.c main/src/v9_apple_music/apple_music.c design-ui/music/render.html scripts/am_shots.sh
git commit -m "feat(apple_music): 增加 AM_APP=apple_music 无头启动与截图比对回路"
```

---

### Task 2: 数据与路径修复(sources.tsv、歌词、死文件清理)

**目的:** 修掉电台列表为空的路径 bug;歌词换成 mockup 诗句;清理未编译死文件,降噪。非视觉,可单测/构建验证。

**Files:**
- Modify: `main/src/v9_apple_music/apple_music.c:195`(sources.csv → sources.tsv)
- Modify: `main/src/v9_apple_music/am_data.c`(`am_mock_lyrics` 换 mockup 4 句)
- Delete: `main/src/v9_apple_music/am_page_home.c`、`am_page_settings.c`、`am_config.c`、`am_config.h`(确认无引用)
- Modify: `main/src/v9_apple_music/am_page_home.h`、`am_page_settings.h`(若无引用一并删)
- Test: `main/tests/apple_music_data_test.c`(已存在,复用/补断言)

**Interfaces:**
- Consumes: `am_sources_csv_load(path, &items, &count)`(已存在,按 tab 自动识别分隔符)。
- Produces: 电台数据从 `third-party/hls_player_demo/qa/production_test/config/sources.tsv` 加载;`am_mock_lyrics[4]` = mockup 歌词。

- [ ] **Step 1: 确认死文件无引用** — 
```bash
for f in am_page_home am_page_settings am_config; do
  echo "== $f =="; rtk proxy /bin/grep -rn "$f" main --include=\*.c --include=\*.h 2>/dev/null | rtk proxy /bin/grep -v "v9_apple_music/$f";
done
```
期望:除各自 .c/.h 自身外无其它引用(尤其不在根 `CMakeLists.txt`、不被 tests 引用)。**若有引用则先解耦,勿删。**

- [ ] **Step 2: 写/更新失败测试** — 在 `main/tests/apple_music_data_test.c` 加断言:从 `sources.tsv` 加载 count>0 且首项 title 非空;`am_mock_lyrics[1]` == mockup 第 2 句。若该 test target 未在 CMake,注册一个 `add_executable(apple_music_data_test ...)`(参照现有 test targets)。

- [ ] **Step 3: 运行确认失败** — `make -C build apple_music_data_test && ./bin/apple_music_data_test`;期望 FAIL(当前指向 sources.csv / 旧歌词)。

- [ ] **Step 4: 改路径 + 歌词** — `apple_music.c` 的 `am_sources_csv_load(".../sources.tsv", ...)`;`am_data.c`:
```c
const char *am_mock_lyrics[4] = {
    "燃烧每一寸的渴望",
    "火力全开 势不可挡",
    "让心跳点燃整个舞台",
    "无所畏惧 一路向光",
};
```
(注意:歌词新增汉字需并入图标/中文字体子集 charset,见 Task 3;若字体缺字会豆腐块。)

- [ ] **Step 5: 删死文件** — `git rm main/src/v9_apple_music/am_page_home.c am_page_home.h am_page_settings.c am_page_settings.h am_config.c am_config.h`(仅确认无引用后);重 `cmake -S . -B build`(CONFIGURE_DEPENDS 重扫)。

- [ ] **Step 6: 构建 + 测试通过** — `make -C build main apple_music_data_test -j && ./bin/apple_music_data_test`;期望 PASS,main 构建 0。

- [ ] **Step 7: 截图确认电台非空** — `AM_APP=apple_music AM_PAGE=radio AM_SHOT=... ./bin/main`;Read 截图确认电台列表出现真实台名(中国之声等)。

- [ ] **Step 8: Commit** — `git add -A && git commit -m "fix(apple_music): 电台改读 sources.tsv、歌词还原 mockup、清理死文件"`

---

### Task 3: 图标与封面资产(矢量图标字体子集 + 火焰封面 PNG)

**目的:** 产出真实矢量图标字体(nav/传输/模式/歌单/音量)与烘焙火焰封面 PNG,供后续视觉任务使用。

**Files:**
- Create: `main/src/v9_apple_music/fonts/am_icons_20.c` 等(每个用到字号一份图标字体,或基准字号 + 运行期 scale)
- Create: `main/src/v9_apple_music/assets/am_cover_fire.c`(188×188 火焰 PNG → C,ARGB8888)
- Modify: `main/src/v9_apple_music/am_icons.h`(图标宏 → 新码点 UTF-8 字面量)
- Modify: `main/src/v9_apple_music/am_fonts.h`(`LV_FONT_DECLARE` 图标字体;`LV_IMAGE_DECLARE` 火焰封面)
- Modify: `main/src/v9_apple_music/am_metrics.c`(字体角色 `f_icon` 指向图标字体)
- Modify: `CMakeLists.txt`(若新增 `assets/*.c` 需并入 `V9_APPLE_MUSIC_SOURCES` GLOB/列表)
- Create: `main/src/v9_apple_music/fonts/src/gen_icons.sh`(记录生成命令,可复现)

**Interfaces:**
- Produces:
  - 图标字体符号(如 `am_icons_20`),`am_icons.h` 宏:`AM_ICON_NAV_NOW/NAV_RADIO/NAV_FAV/PREV/PLAY/PAUSE/NEXT/MODE_SHUFFLE/MODE_ORDER/MODE_ONE/MODE_LIST/QUEUE/SPEAKER`。
  - `LV_IMAGE_DECLARE(am_cover_fire)`(188×188 ARGB8888)。

- [ ] **Step 1: 选图标源 + 抽码点** — 用 Lucide/Feather 的 TTF(若仓库无,`npx` 拉取或从 web 包取)。列出需要的字形与其码点(PUA 或标准)。命令示例(记录进 `gen_icons.sh`):
```bash
ICONS_TTF=<lucide.ttf>
npx -y lv_font_conv --no-compress --format lvgl --bpp 4 --size 20 \
  --font "$ICONS_TTF" --range 0xE000-0xF8FF \
  --lv-font-name am_icons_20 -o main/src/v9_apple_music/fonts/am_icons_20.c --force-fast-kern-format
```
(实际 `--range` 用抽到的具体码点集合,勿全量 PUA 以免体积膨胀。填充版 play/pause/prev/next 若图标库缺,则本步只做 stroke 图标,填充键在 Step 3 用自绘/小 PNG 兜底。)

- [ ] **Step 2: 校验码点已嵌入** — `rtk proxy /bin/grep -c "0x" main/src/v9_apple_music/fonts/am_icons_20.c`;并抽查目标码点在 cmap(避免豆腐块:源字体缺字形会被静默跳过)。

- [ ] **Step 3: 烘焙火焰封面 PNG** — 用 ImageMagick 生成 188×188 径向火焰 + 圆角:
```bash
cd main/src/v9_apple_music/assets
convert -size 188x188 radial-gradient:'#ffe27a'-'#4a1303' fire_base.png
# 叠暖色中段(近似 mockup 的 ffb02f/ff7a18/e0440f/9a2407)+ 圆角 alpha 蒙版:
convert -size 188x188 xc:none -draw "roundrectangle 0,0,187,187,14,14" mask.png
convert fire_base.png mask.png -alpha off -compose CopyOpacity -composite am_cover_fire.png
python3 ../../../../lvgl/scripts/LVGLImage.py --ofmt C --cf ARGB8888 -o . am_cover_fire.png
```
(色停可迭代逼近 mockup 的 `radial-gradient(circle at 44% 40%, ...)`;输入文件名=生成的 C 符号名 `am_cover_fire`。)

- [ ] **Step 4: 更新 am_icons.h / am_fonts.h / am_metrics.c** — 图标宏改为新字体的 UTF-8 字面量(用 `python3 -c "print(chr(0xXXXX).encode('utf-8'))"` 取字节);`am_fonts.h` 加 `LV_FONT_DECLARE(am_icons_20)` 与 `LV_IMAGE_DECLARE(am_cover_fire)`;`am_metrics.c` 的 `f_icon` 各档指向图标字体(等高档可共用)。

- [ ] **Step 5: CMake 接入 assets** — 若新增 `assets/*.c`,在根 `CMakeLists.txt` 的 `V9_APPLE_MUSIC_SOURCES` 加一条 `file(GLOB ... CONFIGURE_DEPENDS "main/src/v9_apple_music/assets/*.c")` 并 append;重 configure。

- [ ] **Step 6: 构建通过 + 图标冒烟** — `make -C build main -j`;临时把某处标签设为新图标宏,截图确认非豆腐块(或直接等 Task 5/7 用到时验)。

- [ ] **Step 7: Commit** — `git add -A && git commit -m "feat(apple_music): 增加矢量图标字体子集与烘焙火焰封面资产"`

---

### Task 4: 播放接线稳健化(init-once+select、seek、模式、音量/静音、解耦)

**目的:** 把 `am_player.c` 改为一次 init + select 切歌,接进度拖动 seek、4 态模式图标、音量/静音,且只靠自身轮询驱动 UI(与 lv_demo_music 解耦)。

**Files:**
- Modify: `main/src/v9_apple_music/am_player.c`(核心接线)
- Modify: `main/src/v9_apple_music/am_player.h`(如需新增 seek/mute API)
- Modify: `main/src/v9_apple_music/apple_music.c`(本地源首次进入时一次性 init 全部本地 URL)
- Test: `main/tests/apple_music_player_test.c`(新建,测索引/模式状态机,音频调用可 stub)

**Interfaces:**
- Consumes(`music_player.h`):`music_player_init(urls,count)`、`music_player_select(i)`、`music_player_prev/next/pause/resume`、`music_player_seek(ms)`、`music_player_get_position_ms/get_duration_ms/get_current_index/is_playing`、`music_player_set_volume(0..100)`、`music_player_mute_toggle`、`music_player_cycle_play_mode`、`music_player_get_play_mode`。
- Produces:`am_player_seek_percent(uint8_t pct)`、`am_player_toggle_mute(void)`、`am_player_play_mode_icon(void)`→返回当前模式图标宏。

- [ ] **Step 1: 写失败测试** — `apple_music_player_test.c`:模拟本地源设 8 首,`am_player_next` 循环推进后 `am_player_current_local_index` 正确回绕;`am_player_cycle_mode` 后 `am_player_play_mode_icon()` 返回 4 个不同宏。音频后端调用在测试里 stub(link 一个假 `music_player_*` 或用真实但 `SDL_AUDIODRIVER=dummy`)。

- [ ] **Step 2: 运行确认失败** — `make -C build apple_music_player_test && ./bin/apple_music_player_test`;期望 FAIL。

- [ ] **Step 3: 本地一次 init + select** — 改 `am_player_play_local_index`:仅首次(或列表变更时)`music_player_init(all_urls, n)`;之后 `music_player_select(index)`。移除每次 deinit/init。用一个 `g_local_inited` 标志或比较 url 集合。

- [ ] **Step 4: seek / 音量 / 静音 / 模式图标** — 新增 `am_player_seek_percent`(`music_player_seek(dur*pct/100)`,电台跳过)、`am_player_toggle_mute`(`music_player_mute_toggle`)、`am_player_play_mode_icon`(按 `music_player_get_play_mode()` 4 态返回 `AM_ICON_MODE_*`)。`am_player_cycle_mode` 里刷模式图标。

- [ ] **Step 5: 解耦驱动** — 确认 UI 刷新只在 120ms `am_player_timer_cb` 里做(读 position/duration/is_playing/current_index);不依赖 wrapper 的 `_lv_demo_music_*` 异步路径。若运行时该路径导致崩溃,记录并在 wrapper 侧加最小守卫(仅在必要时,属"接口不足"的小扩展)。

- [ ] **Step 6: 测试 + 构建通过** — `make -C build main apple_music_player_test -j && ./bin/apple_music_player_test`;PASS。

- [ ] **Step 7: 真机冒烟(有声/进度推进)** — 运行 `AM_APP=apple_music ./bin/main`(有显示环境)确认点列表能播、进度推进、模式/音量可用;无显示环境用日志确认 `music_player_get_position_ms` 递增。

- [ ] **Step 8: Commit** — `git add -A && git commit -m "refactor(apple_music): 播放接线稳健化(select 切歌/seek/模式/音量/解耦)"`

---

### Task 5: 播放条视觉还原(去 meta、控件居中、knob、模式图标、音量交互)

**目的:** 让底部播放条与 mockup 一致:传输键整组居中、无左侧标题块、进度末端白点 knob、模式图标随态切换、音量条可交互。

**Files:**
- Modify: `main/src/v9_apple_music/am_shell.c`(`am_shell_build_miniplayer`)
- Modify: `main/src/v9_apple_music/am_player.c`(`am_player_refresh_ui` 刷 knob 位置/模式图标;绑 seek/音量/静音手势)
- Modify: `main/src/v9_apple_music/am_player.h`(miniplayer handles 若需加 `knob`、`vol_track` 句柄)

**Interfaces:**
- Consumes:Task 3 图标宏、Task 4 的 `am_player_seek_percent/toggle_mute/play_mode_icon`。
- Produces:播放条 DOM 结构与 mockup `.player`(`.scrub`+`.pbar`)一致。

- [ ] **Step 0(必做,已定位的真实 bug):恢复 player 几何** — `am_shell_build_miniplayer` 开头的 `lv_obj_remove_style_all(player)` **会清掉 `am_build_root` 给 player 设的 pos/size**,导致 player 塌成 (0,0,130,130) 落到顶部(实测)。修法:在 `am_build_root` 调用 `am_shell_build_miniplayer` **之后**重设 `lv_obj_set_pos(player,0,height-player_h)` + `lv_obj_set_size(player, main_width, player_h)`,或让 builder 内部重设几何。验收:player 出现在底部 78px 条。

- [ ] **Step 1: 去 meta、控件居中** — `am_shell_build_miniplayer` 删除 pbar 里 220px 的 `meta`(标题/副标)块;`ctrls` 容器设 `flex_grow(1)` + `LV_FLEX_ALIGN_CENTER` 使其整组居中;`vol` 固定 110px 右钉(pbar 用 `space_between` 或 ctrls grow + vol 定宽)。

- [ ] **Step 2: 进度 knob** — 在进度轨道加 12px 白色圆点(`radius CIRCLE`,阴影 `0 1 4 rgba .3`),句柄存 `h.knob`;`am_player_refresh_ui` 里按进度百分比 `lv_obj_align(knob, LV_ALIGN_LEFT_MID, track_w*pct/100 - 6, 0)`。

- [ ] **Step 3: 模式图标随态** — `btn_mode` 的标签文本用 `am_player_play_mode_icon()`;`am_player_refresh_ui` 里同步(模式键强调色)。

- [ ] **Step 4: 音量/进度交互** — 给进度轨道与音量轨道加 `CLICKABLE` + `LV_EVENT_PRESSING/RELEASED`,按 `lv_indev` 点位算比例 → `am_player_seek_percent` / `am_player_set_volume_percent`;喇叭图标点击 → `am_player_toggle_mute`。

- [ ] **Step 5: 出图比对** — `make -C build main -j`;`AM_APP=apple_music AM_PAGE=now AM_SHOT=... ./bin/main` → 与 `/tmp/am/mk_now.png` 的播放条区域并排比对;调间距/字号/图标至一致。

- [ ] **Step 6: Commit** — `git add -A && git commit -m "feat(apple_music): 播放条还原(控件居中/knob/模式图标/音量交互)"`

---

### Task 6: 正在播放页还原(火焰封面、歌词、暂停态、电台态)

**目的:** 正在播放页与 mockup 一致:火焰封面图 + 标题/歌手/歌词(第2行高亮),暂停缩放,电台纯色封面 + 直播副标。

**Files:**
- Modify: `main/src/v9_apple_music/apple_music.c`(`am_build_now_view` / `am_update_now_view`)
- Modify: `main/src/v9_apple_music/apple_music.c`(`AM_PAUSED` 消费,接 Task 1 占位)

**Interfaces:**
- Consumes:`LV_IMAGE_DECLARE(am_cover_fire)`(Task 3)、`am_player_is_playing`/`am_player_source_kind`。

- [ ] **Step 1: 火焰封面** — `am_build_now_view` 把 `cover` 从 `lv_obj`(渐变)改为 `lv_image`(`am_cover_fire`),188×188 圆角14 + 阴影。电台态:改回纯色/线性渐变块(隐藏图或叠色)。

- [ ] **Step 2: 歌词还原** — 歌词 4 行:字号 13 次要色,第 2 行 15/700 强调色;单行省略号(`LV_LABEL_LONG_DOT` + 约束单行高)。电台态隐藏歌词、副标显示"● 正在直播 · <类型>"。

- [ ] **Step 3: 暂停态缩放** — `am_update_now_view` 按 `am_player_is_playing()`:暂停 → 封面 `lv_image_set_scale(200*0.9)` 或容器 scale + 阴影变小;播放 → 恢复。`AM_PAUSED=1` env 设初始暂停用于截图。

- [ ] **Step 4: 出图比对(播放/暂停/电台三态)** — 分别截 `AM_PAGE=now`、`AM_PAGE=now AM_PAUSED=1`、`AM_PAGE=now`+电台源;与 mockup `render.html?view=now`、`?view=now&paused`、`?view=now&station=0` 比对。

- [ ] **Step 5: Commit** — `git add -A && git commit -m "feat(apple_music): 正在播放页还原(火焰封面/歌词/暂停态/电台态)"`

---

### Task 7: 侧栏还原(矢量 nav 图标、间距、选中态)

**目的:** 侧栏与 mockup 一致:交通灯、"音乐"标题、3 导航项(矢量图标 + 文案 + 选中态)、"最近播放"分区与列表。

**Files:**
- Modify: `main/src/v9_apple_music/am_shell.c`(`am_shell_build_sidebar`)
- Modify: `main/src/v9_apple_music/am_data.c`(`am_nav_items` 图标宏 → 新 nav 图标)

**Interfaces:**
- Consumes:Task 3 nav 图标宏(`AM_ICON_NAV_NOW/RADIO/FAV`)。

- [ ] **Step 0(必做,同 player 的几何 bug):恢复 sidebar 几何** — `am_shell_build_sidebar` 开头 `lv_obj_remove_style_all(sidebar)` 会清掉 `am_build_root` 设的 sidebar size(sidebar_w×height)→ sidebar 尺寸塌陷。修法:builder 内重设 `lv_obj_set_size(sidebar, m->sidebar_w, <height>)`(或 `LV_PCT(100)` 高),或在 `am_build_root` build 之后重设。验收:侧栏满高、宽 196(800 档 sidebar_w 应对齐 mockup 的 196)。

- [ ] **Step 1: nav 图标 + 间距** — `am_nav_items` 图标改新矢量宏(17px 强调色);nav 项 padding 7/8、圆角7、gap9,选中 `rgba(0,0,0,.08)` + 文案加粗。**注**:mockup 侧栏宽 196,现 metrics `sidebar_w=164`,Task 7 顺带把 800 档 sidebar_w 对齐到 196。

- [ ] **Step 2: 分区/列表/标题微调** — "音乐" 18/700;"最近播放" 11 次要 letter-space .5;recent 13 次要省略号;整体 padding 与 mockup 对齐(侧栏 196px,padding 0/12/12)。

- [ ] **Step 3: 出图比对** — 截任一视图,与 mockup 侧栏区域比对(三视图侧栏一致,取 now)。

- [ ] **Step 4: Commit** — `git add -A && git commit -m "feat(apple_music): 侧栏还原(矢量 nav 图标/间距/选中态)"`

---

### Task 8: 电台/收藏列表行还原(副标题、渐变块、行尾图标)

**目的:** 列表行与 mockup 一致:电台=渐变块+声波图标+台名+类型+LIVE(仅当前);收藏=火焰块+曲名+歌手+心形+时长。

**Files:**
- Modify: `main/src/v9_apple_music/am_page_list.c`(`am_page_list_build_radio` / `build_favorites`)
- Modify: `main/src/v9_apple_music/am_data.h`/`am_data.c`(如需给 radio 项加"类型/genre"、给 local 项加"歌手/来源"派生字段)

**Interfaces:**
- Consumes:Task 3 声波/心形图标宏。

- [ ] **Step 1: 电台行修正** — 副标从 `缓存 Nms` 改为类型/来源(sources.tsv 无 genre → 用固定"广播电台"或从 title 派生);`LIVE`/`正在播放` 徽标仅当前播放行显示;42×42 按台配色线性渐变块 + 声波图标。

- [ ] **Step 2: 收藏行修正** — 副标从文件 `path` 改为歌手/来源(本地文件无歌手 → 用"本地资料库"或从文件名派生);行尾 心形(15 强调)+ 时长(`music_player_get_track_duration_ms` 格式化);42×42 火焰渐变块。

- [ ] **Step 3: 出图比对** — 截 `AM_PAGE=radio`、`AM_PAGE=fav` 与 mockup `?view=radio`、`?view=fav` 比对(注意文案为真实数据,比对结构/样式)。

- [ ] **Step 4: Commit** — `git add -A && git commit -m "feat(apple_music): 电台/收藏列表行还原(副标/渐变块/行尾图标)"`

---

### Task 9: 播放列表弹层还原

**目的:** 弹层与 mockup 一致:300px、右下浮层、标题"播放列表 N 首"、行(序号/名/副/时长)、当前高亮、遮罩点击关闭。

**Files:**
- Modify: `main/src/v9_apple_music/am_shell.c`(弹层构建)
- Modify: `main/src/v9_apple_music/am_player.c`(`am_refresh_playlist_popup`、计数、遮罩关闭)

**Interfaces:**
- Consumes:`am_player_playlist_open/set_playlist_open`(已存在)。

- [ ] **Step 1: 弹层结构** — 300px 宽,`LV_ALIGN_BOTTOM_RIGHT,-18,-96`,圆角12 阴影;标题行"播放列表 <N> 首"(名 13/700 + 计数 11 次要);列表可滚动。加一个全屏透明遮罩(在弹层下、player 之上),点击关闭。

- [ ] **Step 2: 行样式** — 每行:序号(18 宽,11 次要,居中)+ 名(13)/副(11 次要)+ 时长(11 次要);当前行 hover 背景 + 序号/名强调。

- [ ] **Step 3: 出图比对** — 加 `AM_PANEL=playlist` env(在 apple_music.c 消费 → `am_player_set_playlist_open(true)`),截 `AM_PAGE=now AM_PANEL=playlist`,与 mockup `?view=now&playlist` 比对。

- [ ] **Step 4: Commit** — `git add -A && git commit -m "feat(apple_music): 播放列表弹层还原(结构/行样式/遮罩关闭)"`

---

### Task 10: 多分辨率自适应(640×480、480×272)

**目的:** 640/480 档布局不错位、可用;480×272 单列堆叠。容器尺寸与坐标全走 metrics。

**Files:**
- Modify: `main/src/v9_apple_music/am_metrics.c`(补齐 640/480 结构尺寸;480 `stack_content`)
- Modify: `main/src/v9_apple_music/apple_music.c`(容器尺寸/坐标读 `M()`;480 单列)
- Modify: `main/src/v9_apple_music/am_page_list.c`(480 行紧凑)

**Interfaces:**
- Consumes:`am_metrics()`(各档尺寸/字体角色)。

- [ ] **Step 1: metrics 补档** — 640/480 的 sidebar_w/player_h/cover/pad 等逐档设值;480 `stack_content=true`,正在播放页封面上、信息下单列。

- [ ] **Step 2: 容器走 metrics** — `am_build_root` 的 sidebar/main/header/content/player 尺寸与坐标不再写死,读 `M()`;480 时正在播放 `if(M()->stack_content){单列}`。

- [ ] **Step 3: 逐档出图** — 用 `AM_RES=640x480`、`AM_RES=480x272`(main.c 已支持 `AM_RES`)+ `AM_APP=apple_music AM_PAGE=now` 截图;确认无错位/无越界(`lv_obj_center` 不偏出屏)。480 单列可读。

- [ ] **Step 4: 回归主档** — 再截 800×480 三视图,确认未回退。

- [ ] **Step 5: Commit** — `git add -A && git commit -m "feat(apple_music): 640/480 多分辨率自适应(480 单列堆叠)"`

---

### Task 11: 总验与卫生清理

**目的:** 全页×全态截图总验;检查 detect_changes / 无残留 / 测试全绿。

**Files:**
- Modify: 收尾微调涉及的文件(按比对结果)

- [ ] **Step 1: 全套截图** — `scripts/am_shots.sh` 跑全套:800/640/480 × {now, now+paused, radio, fav, now+playlist, 电台态};逐张与 mockup 对应态 Read 比对,列残差。

- [ ] **Step 2: 修残差** — 按残差清单微调(间距/色/字号/图标),每修一处重截确认。

- [ ] **Step 3: 测试全绿** — `make -C build main <所有 apple_music *_test> -j` 并逐个运行;期望全 PASS。

- [ ] **Step 4: 卫生检查** — 确认无遗留死文件、无 `music_player.c/.h` 侵入式改动(`git diff --stat` 复核);desktop 入口仍能开关(`desktop_apple_music_*_test` 通过)。

- [ ] **Step 5: Commit** — `git add -A && git commit -m "chore(apple_music): 总验收尾与残差修正"`

---

## Self-Review 记录

- **Spec 覆盖:** 视觉还原(Task 5-9)、真实数据(Task 2/8)、图标(Task 3/5/7)、火焰封面(Task 3/6)、播放稳健化+seek(Task 4)、多分辨率(Task 10)、验证回路(Task 1)、清理(Task 2/11)——spec 各节均有对应任务。
- **Placeholder 扫描:** UI 像素微调本质是"迭代至与 mockup 一致",已用截图比对+目标 token 明确验收标准,非模糊占位;确定性部分(路径/数据/命令/API 调用)给了具体代码/命令。
- **类型一致性:** `am_player_seek_percent`/`am_player_toggle_mute`/`am_player_play_mode_icon` 在 Task 4 定义、Task 5 消费,签名一致;图标宏命名在 Task 3 定义、5/7/8 消费一致。
- **已知风险:** 本机 SDL 出图不确定(Task 1 Step 6 给了 lv_snapshot harness 回退);图标源 TTF 需确认可得(Task 3 Step 1);`music_player` 异步耦合(Task 4 Step 5 以只轮询驱动规避)。
