# Apple Music UI 多档固定自适应布局 设计

- 日期:2026-06-14
- 状态:设计已确认,待实现计划
- 前置:`docs/superpowers/specs/2026-06-14-apple-music-lvgl-design.md`(原固定 800×480 复刻)

---

## 1. 背景

现有 Apple Music LVGL UI 为**固定 800×480** 写死(原 spec §10.1 明确"不做响应式")。实测改到 640×480 / 480×272:三大区会重定位、页面部分回流(用了 FR/`LV_PCT`),但**不优雅自适应**——

- **写死像素**:`sidebar_w=164`、`player_h=78`(apple_music.c);迷你播放条列宽 `235/171/205`(=636 派生,am_shell.c);`queue_w=244`、`settings_nav_w=180`(am_page_list/settings);`cover=44`、各 padding、副标题 max-width。宽度一变就溢出/挤压。
- **位图字体固定像素**(`am_font_11…34`),不缩放;无按分辨率选字号的逻辑 → 小屏字过大、重叠。

实测:640×480 推荐面板文字逐字换行、迷你条进度被裁、浮标压标题;480×272 严重重叠不可用。

## 2. 目标与决策

让 UI 在 **3 个固定档**下都干净显示:**800×480 / 640×480 / 480×272**。

| 决策 | 选定 |
|------|------|
| 自适应方式 | **多档固定**(非流式):每档一张尺寸表,启动按分辨率选档 |
| 字体 | **每档预生成子集位图字号**(沿用 `lv_font_conv`);640 档复用 800 档字体集(等高),480 档单独生成小字号集 |
| 结构方案 | **A. 中心化 `am_metrics` 表 + 语义字体角色**:消费端读 `am_metrics()`,不再写死数字 |
| 范围(YAGNI) | 仅这 3 档;不引矢量字体;不做无级缩放;`music_player.c` 不动 |

**关键观察**:640×480 与 800×480 **等高**(均 480),仅宽少 160px → 640 档主要是"窄"(侧栏/列宽收一收),字号可沿用 800 档;480×272 两维都小,需更小字号且部分两列布局要**改为单列堆叠**。

## 3. 架构

### 3.1 新模块 `am_metrics.{h,c}`(单一事实源)
```c
typedef enum { AM_TIER_800x480, AM_TIER_640x480, AM_TIER_480x272, AM_TIER_COUNT } am_tier_t;

typedef struct {
    const char *id;
    int16_t sidebar_w, player_h;
    int16_t content_pad, page_gap;          /* 内容区 padding / 页内主 gap */
    int16_t cover, item_action;             /* 列表/推荐封面、圆形动作钮 */
    int16_t queue_w, settings_nav_w;        /* 列表页队列列宽、设置左栏宽 */
    int16_t panel_pad, card_radius;
    bool    stack_content;                  /* true=两列内容区改单列堆叠(480 档) */
    /* 语义字体角色(指向该档字体) */
    const lv_font_t *f_title;   /* 页标题 */
    const lv_font_t *f_h2;      /* hero/banner 标题 */
    const lv_font_t *f_strong;  /* 面板标题/导航标签 */
    const lv_font_t *f_body;    /* 条目标题 */
    const lv_font_t *f_label;   /* 副标题/说明/徽标/小标题 */
    const lv_font_t *f_metric;  /* 指标数值/品牌徽标 */
    const lv_font_t *f_icon;    /* 导航/控制图标字形 */
} am_metrics_t;

const am_metrics_t *am_metrics(void);            /* 当前档 */
void am_metrics_init(int hor_res, int ver_res);  /* 选档:精确匹配→否则按宽就近向下取档 */
```

### 3.2 三档起始值(实现时按截图微调)
| 字段 | 800×480 | 640×480 | 480×272 |
|------|---------|---------|---------|
| sidebar_w | 164 | 140 | 108 |
| player_h | 78 | 78 | 56 |
| content_pad | 20 | 18 | 12 |
| page_gap | 16 | 14 | 10 |
| cover | 44 | 40 | 34 |
| item_action | 30 | 28 | 24 |
| queue_w | 244 | 200 | (stack) |
| settings_nav_w | 180 | 160 | 120 |
| panel_pad | 18 | 16 | 12 |
| card_radius | 18 | 16 | 14 |
| stack_content | false | false | **true** |

### 3.3 字体角色 → 字号(每档)
| 角色 | 800 & 640 | 480 |
|------|-----------|-----|
| f_title | 34 | 22 |
| f_h2 | 24 | 18 |
| f_metric | 18 | 15 |
| f_strong | 14 | 12 |
| f_body | 13 | 12 |
| f_label | 11 | 10 |
| f_icon | 14 | 12 |
- 800 & 640 复用同一套字体(现有 `am_font_*`,按角色映射;`f_body=13/f_strong=14/f_label=11/f_h2=24/f_title=34/f_metric=18/f_icon=14`)。**角色是对现有 8 个字号的合并**:现有用量为 12/16 的文本就近并入相近角色(12→f_label 或 f_body,16→f_strong 或 f_metric),实现时定稿。
- 480 档新生成一套:5 个字号 `10/12/15/18/22`(子集字符集与现有相同,`lv_font_conv` 跑一遍),命名 `am_font_480_{10,12,15,18,22}`。

### 3.4 消费端重构(把写死数字换成 `am_metrics()`)
- `apple_music.c`:根 grid 用 `m->sidebar_w / m->player_h`;内容区 `content_pad`;换肤重建时不变。
- `am_shell.c`:**迷你播放条改 FR 网格列**(`[FR(115), CONTENT, FR(100)]` + `flex_grow`),彻底去掉 `235/171/205` 硬编码;品牌/导航/listener 用角色字体与 `sidebar_w` 相关留白。
- `am_widgets.c`:工厂里写死的字号改为接收/读取角色字体(或工厂内部读 `am_metrics()`);`am_nav_item`/`am_pill`/`am_section_title` 等用对应角色。
- `am_page_home/list/settings.c`:封面 `m->cover`、列宽 `m->queue_w/settings_nav_w`、padding/gap、所有 `&am_font_N` → `m->f_*`;**当 `m->stack_content` 为真**,列表页 content-grid 与设置页 settings-layout 由两列改单列(catalog 满宽,queue 在下;settings tabs 在上、main 在下)。
- `main.c`:`./main [W H]`(默认 800×480)→ 用 W×H 建显示器 → `am_metrics_init(W,H)`。
- 字体声明:`am_fonts.h` 增加 480 档字体的 `LV_FONT_DECLARE`;`lv_conf.h` 的 `LV_FONT_CUSTOM_DECLARE` 补全。

### 3.5 选档逻辑
`am_metrics_init(w,h)`:精确匹配 (800,480)/(640,480)/(480,272);否则按 `w` 就近向下取(≥800→800档,≥640→640档,否则→480档),保证未知分辨率也有合理回退。

## 4. 验证
用现有截图基建(`AM_SHOT` + headless),`./main W H` 跑三档 × 各页,逐档与"该档应有的布局"对照(800 档须与原 mockup 一致,640/480 看是否干净不溢出/不重叠),微调 `am_metrics` 表与 480 字号到位。回归 `ctest`(data/theme 测试不受影响;可加 `am_metrics_init` 选档单测)。

## 5. 明确不做(YAGNI)
- 流式无级缩放、矢量字体。
- 800/640/480 以外的精调档(其他分辨率走就近回退)。
- 动 `music_player.c` 后端。
- 横竖屏切换、运行时动态改分辨率(启动时定档)。

## 6. 风险
- **480×272 最难**:两维都小,除缩放外需结构简化(`stack_content` 单列、可能隐藏次要装饰);实现时以截图为准微调,必要时再砍非关键元素。
- 字体角色重构面广但机械;按文件分批改,逐档截图验收。
- 迷你条由硬编码改 FR 后,需重新核对三档下的对齐(进度条/控制键)。
