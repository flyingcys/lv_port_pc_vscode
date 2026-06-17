# 控制中心 / 通知中心面板交互重做 设计文档

- 日期：2026-06-17
- 分支：release/v9.3-replace2
- 目标模块：`main/icon_replace_2/`
- 前序：`docs/superpowers/specs/2026-06-16-icon-replace-2-html-fidelity-design.md`
- 事实来源：`design-ui/`（HTML/CSS/JS 样机）；本次两个面板按用户决策**有意偏离** HTML 的 85% 大面板

## 1. 背景与问题

`icon_replace_2` 的图标与壁纸已正常，但两个滑出面板效果不理想：

- **通知中心上滑会卡在中间状态**（截图 `1.png`：面板停在屏幕中部，上方留大片壁纸空白，既非全开也非全关）。
- 面板观感一般：固定高度 `panel_h = 408`（屏高 480 的 85%），内容仅占顶部约 250px，下方一大片空暗区。

### 现状根因

两个面板都是「阈值触发 + 一次性固定动画」模型：

- 顶/底各一条 28px 感应带（`edge_top`/`edge_bot`），按下后位移超过 `EDGE_THRESHOLD=40` 即调用 `ir2_panels_show_*(true)`，播放 250ms 动画滑到**固定目标位**；面板**完全不跟手**。
- 收起依赖在多个重叠对象（边缘带 + 面板根）上监听 `LV_EVENT_GESTURE` 的方向判定（`panel_gesture_cb`），脆弱。
- 该模型在真机手势下没有「松手吸附」兜底，是「卡在中间」的结构性来源；85% 固定高又制造空暗区。

## 2. 已拍板决策

| 项 | 决策 |
|---|---|
| 交互模型 | **跟手拖拽 + 松手吸附**（面板实时跟随手指，松手按位置/甩动吸附到全开或全关） |
| 面板高度 | **内容自适应高度**（`LV_SIZE_CONTENT`，消除空暗区） |
| 收起方式 | **边缘把手 + 反向拖拽收起**（补 HTML 的抓取把手；从把手反向拖拽收起） |
| 不做 | 不加变暗遮罩、不加点击把手收起、不接真实数据、不做高斯模糊、不改其他页面 |

## 3. 交互模型：跟手拖拽 + 松手吸附

### 3.1 展开（从屏幕边缘）

- 顶/底各保留一条 28px 感应带（`edge_top`/`edge_bot`）。
- 边缘条 `PRESSED` 记起点 Y；`PRESSING` 时把面板 y **实时跟随手指**：
  - 控制中心：`reveal = pt.y - press_y`（下滑为正），`panel.y = clamp(-H + reveal, -H, 0)`
  - 通知中心：`reveal = press_y - pt.y`（上滑为正），`panel.y = clamp(screen_h - reveal, screen_h - H, screen_h)`
  - 其中 `H` 为面板实测内容高度（见 §4.1）。
- `RELEASED` 时**吸附**：
  - 主判据：露出比例 `reveal / H > 0.5` → 动画到全开，否则动画回全关。
  - 兜底：末段为明显甩动（最后一次 `PRESSING` 位移大于阈值，如 ≥12px）时按甩动方向吸附，忽略位置。
- **松手必落到全开/全关两态之一，结构上杜绝中间态。**

### 3.2 收起（反向拖拽把手）

- 面板内缘加一条把手区（控制中心在**底**、通知中心在**顶**，与 HTML 同位）。
- 把手区 `PRESSED/PRESSING/RELEASED` 跟手移动面板，同样按 0.5 阈值吸附；中途回拖可重新展开。
  - 控制中心：向上拖收起；通知中心：向下拖收起。
- 收起拖拽**只挂在把手区**，不碰滑条/开关/通知卡，避免与内部控件冲突。

### 3.3 互斥与门控

- panels 内部维护 `active` 状态（0=无, 1=控制中心, 2=通知中心）。
- `active != 0` 时，边缘条按下被忽略，不会两个面板同时展开。
- 同一时刻只允许一个拖拽流程（panels 内部 `dragging` 标志保护重入）。

## 4. 面板几何

### 4.1 内容自适应高度

- 面板 `height = LV_SIZE_CONTENT`（标题 + 内容撑高），设 `max_height = screen_h` 兜底；内容超出则面板内部滚动。
- 创建后 `lv_obj_update_layout()` 读 `lv_obj_get_height()` 得 `H`；全开/全关位置与 §3 钳制公式均基于该 `H`。
- `metrics.panel_h` 退化为「上限/兜底」语义，不再表示真实高度（字段保留，避免 metrics 表大改）。

### 4.2 把手

- 透明命中条：宽满、高 ≈30（480 档 ≈20），`CLICKABLE`、非 `SCROLLABLE`，承载拖拽收起回调。
- 居中可见小药丸：60×6（480 档 ≈40×4），圆角 3，`rgba(255,255,255,.3)`。
- 位置：控制中心置于面板**底部**（内容之后）；通知中心置于面板**顶部**（内容之前）。

### 4.3 圆角

- 统一圆角 24 的卡片。
- 贴屏一侧「直角近似（让圆角溢出屏外） vs 留小边距做悬浮卡片」属微调，放到截图回路二选一定稿，不阻塞实现。

## 5. 模块改动与接口边界

每个模块单一职责、接口清晰。

| 文件 | 改动 | 职责 / 接口边界 |
|---|---|---|
| `icon_replace_2_panels.c/.h` | 主要改动 | 拥有全部面板几何 / 吸附动画 / 把手收起逻辑。新增对外接口供边缘条驱动展开拖拽 |
| `icon_replace_2.c` | 重接边缘条 | 边缘条回调改为转发 reveal 给 panels；**删除**旧 `panel_gesture_cb` 及 `LV_EVENT_GESTURE` 监听 |
| `icon_replace_2_widgets.c/.h` | 小改 | `ir2_widget_glass_panel` 支持内容自适应高度；新增把手控件工厂 |

### 5.1 panels 新增/调整接口（草案）

```c
/* 边缘条驱动的展开拖拽：which 1=控制中心 2=通知中心，reveal_px 为从全关起算的露出像素 */
void ir2_panels_open_drag(icon_replace_2_panels_t * p, int which, int32_t reveal_px);
void ir2_panels_open_release(icon_replace_2_panels_t * p, int which);
/* 当前展开态：0=无 1=控制中心 2=通知中心，供边缘条门控 */
int  ir2_panels_active(icon_replace_2_panels_t * p);
```

- 把手收起拖拽完全内聚在 panels.c（把手是 panels 的子对象，回调内部挂）。
- 保留 `ir2_panels_apply_initial(p, which)` 供截图钩子，但内部改为按实测 H 定位（先 `update_layout`）。
- 旧 `ir2_panels_show_control/notify(p, bool)` 可保留为「直接吸附到全开/全关」的内部/兼容实现（动画版）。

### 5.2 icon_replace_2.c 边缘条回调改造

- `edge_*_pressed_cb`：记 `press_y`；若 `ir2_panels_active() != 0` 则本次不发起展开拖拽。
- `edge_*_pressing_cb`：算 `reveal` → `ir2_panels_open_drag(panels, which, reveal)`。
- `edge_*_released_cb`（新增）：`ir2_panels_open_release(panels, which)`。
- 删除 `panel_gesture_cb` 及其在边缘条/面板上的 `LV_EVENT_GESTURE` 注册。

## 6. 截图钩子 / 多分辨率 / 与 HTML 偏离

- 三档（800×480 / 640×480 / 480×272）按实测 H 适配。
- 截图钩子 `AM_PANEL=control|notify` 经 `ir2_panels_apply_initial` 设全开静态态用于出图（无动画）。
- **明确偏离**：两个面板按决策改为内容自适应卡片，不再复刻 HTML 的 85% 大面板；因此面板部分由「像素级比对 HTML」降级为「健全性 + 手感」判据，其余页面仍以 HTML 为真值。

## 7. 验证 / 完成判据

1. 三档均能编译、启动、出图、不溢出；
2. 上滑/下滑跟手；松手必吸附到全开或全关，**复现不出中间态**；
3. 把手反向拖拽可收起、中途回拖可重新展开；两面板互斥不同时开；
4. 桌面翻页 / 拖拽换位 / 跨页 / 摇晃 / 编辑 / 锁屏交互回归不破；
5. 面板无大片空暗区、把手可见、圆角观感合理。

## 8. 非目标（YAGNI）

- 不加变暗遮罩；
- 不加点击把手收起（仅拖拽）；
- 不接真实业务数据；
- 不做真正高斯模糊（半透明近似沿用）；
- 不改其他页面 / 其他交互。

## 9. 风险与缓解

| 风险 | 缓解 |
|---|---|
| 收起拖拽与面板内滑条/通知卡冲突 | 收起拖拽只挂把手区，不挂面板根 |
| 内容自适应高 `H` 在布局完成前读取为 0 | 读取前 `lv_obj_update_layout()`；必要时跑足帧再定位 |
| 边缘条与桌面 tileview 横向滚动抢手势 | 边缘条 `CLICKABLE` 非 `SCROLLABLE`、关 `GESTURE_BUBBLE`，基于 PRESSING 跟手 |
| 面板展开时底部边缘条仍截获点击 | `active != 0` 门控边缘条按下；把手远离边缘条区域 |
| 截图钩子静态态位置随 H 变化 | `apply_initial` 内先 `update_layout` 再按 H 定位 |
