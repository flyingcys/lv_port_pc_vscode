# `icon_replace_2` 手机桌面化改造技术方案

## 1. 目标与已确认约束

基于当前 `icon_replace_2`，改造成更接近手机桌面的结构，并满足以下已确认约束：

- 顶部区域高度全局固定。
- 右上角默认显示时间和 `WiFi`。
- 时间默认开机后每分钟自动刷新。
- `WiFi` 状态跟随硬件状态变化。
- 锁屏不是独立场景，而是桌面分页体系里的第一页/特殊页。
- 每一页都可以单独定义顶部区域内容和对齐方式。
- 某些页面下，顶部区域除了右侧系统状态外，还可能在左侧或中间显示额外内容。
- 保留当前 `icon_replace_2` 的多页滑动桌面、长按抖动、拖拽换位、跨页拖拽能力。

当前项目分辨率为 `800x480`，本方案默认先按固定分辨率设计。

## 2. 当前实现分析

当前实现位于 [main/icon_replace_2/icon_replace_2.c](/home/share/samba/lvgl/lv_port_pc_vscode_v9.1-test/main/icon_replace_2/icon_replace_2.c:1)，核心结构如下：

- 直接把整屏创建为一个 `lv_tileview`
- 每个页面 tile 都是整屏 `800x480`
- 图标布局和拖拽判断以整屏坐标为基准
- 左右跨页边界直接使用整屏宽度判断

关键代码锚点：

- `lv_tileview_create(lv_screen_active())`
- `lv_obj_set_size(screen, PAGE_WIDTH, PAGE_HEIGHT)`
- `index_by_xy()` 直接基于整屏触点计算索引
- `lv_indev_get_point()` 获取的是屏幕坐标
- 左右跨页使用 `PAGE_WIDTH` 和边缘计数器判断

这套实现适合“全屏分页桌面”，但不适合直接扩成“顶部支持系统状态和按页变化内容”的结构，主要问题有：

- 顶部区域如果只是简单覆盖在 `tileview` 上，系统状态和页面扩展内容的边界会混乱。
- 锁屏页和普通桌面页的顶部内容差异无法自然建模。
- 每页顶部布局不同的需求，不能只靠一个固定右侧容器解决。
- 继续沿用整屏坐标，会让桌面区和顶部区混在一起，后续拖拽换位容易偏移。

结论：需要把“系统状态”和“页面扩展内容”从单一 `tileview` 里拆出来，但又不能退化成“每页复制一整套顶栏”。

## 3. 备选方案对比

### 方案 A：全局单一顶栏，所有内容都按页切换

做法：

- 顶部只保留一套全局 `top_bar`
- `left / center / right` 都通过页配置整体切换

优点：

- 结构统一
- 页面切换逻辑集中

缺点：

- 时间和 `WiFi` 这类系统状态被混进页内容配置，职责不清
- 页面差异一多，顶栏切换控制器会变得很重

### 方案 B：每一页都自带一套顶栏

做法：

- 每个 tile 页面内部都自带自己的顶部区域
- 切页时顶部和页面一起滑动

优点：

- 每页布局最自由
- 锁屏页天然可以做成特殊模板

缺点：

- 时间和 `WiFi` 要同步更新到每一页
- 系统状态存在多份对象，维护成本高
- 编辑态、硬件态和跨页联动会更复杂

### 方案 C：全局系统状态层 + 页级扩展层

做法：

- 顶部区域仍然是全局固定高度的一层
- 右侧系统状态区全局唯一，负责时间和 `WiFi`
- 左侧和中间定义为页级扩展区，由当前页决定是否显示、显示什么

优点：

- 系统状态与页面扩展职责分离
- 时间和 `WiFi` 只维护一份
- 满足“某些页面左侧/中间显示更多内容”的需求
- 锁屏页可以作为第一页，只通过页模式切换顶部扩展内容

缺点：

- 比纯固定右侧顶栏稍复杂
- 需要建立“当前页 -> 顶部扩展布局”的控制器

结论：推荐方案 C。

## 4. 推荐架构

### 4.1 顶层容器结构

```text
screen_root
|- top_bar
|  |- top_bar_left_slot
|  |- top_bar_center_slot
|  `- top_bar_system_right
|     |- wifi_icon
|     `- time_label
`- desktop_host
   `- desktop_tileview
      |- page_0_lock
      |- page_1_home
      |- page_2_home
      `- ...
```

职责划分：

- `screen_root`
  - 整屏根节点
  - 负责统一背景和基础布局
- `top_bar`
  - 全局固定高度
  - 不随页面滑动
  - 承载页级扩展区和系统状态区
- `top_bar_left_slot`
  - 当前页可选的左侧扩展内容
- `top_bar_center_slot`
  - 当前页可选的中间扩展内容
- `top_bar_system_right`
  - 全局系统状态区
  - 默认持有 `WiFi` 和时间
- `desktop_host`
  - 顶部条以下的桌面承载区
  - 负责裁剪和桌面局部坐标系
- `desktop_tileview`
  - 保留现有多页滑动桌面逻辑

### 4.2 推荐尺寸

建议先按以下固定尺寸设计：

- `SCREEN_W = 800`
- `SCREEN_H = 480`
- `TOP_BAR_H = 40`
- `DESKTOP_W = 800`
- `DESKTOP_H = 440`

推荐位置：

- `top_bar`：`(0, 0, 800, 40)`
- `desktop_host`：`(0, 40, 800, 440)`

固定高度是关键约束。后续页面差异只体现在顶部内容布局，不体现在顶部高度变化。

## 5. 顶部区域设计

### 5.1 分层原则

顶部区域分成两类内容：

- 系统状态内容：全局唯一
  - 时间
  - `WiFi`
- 页面扩展内容：按页切换
  - 锁屏页的大时间、状态文本
  - 某些页左侧标题
  - 某些页中间提示、搜索栏、页名等

这里最重要的原则是：右侧系统状态不跟着每页复制，只存在一份对象；左侧和中间才根据当前页模式切换。

### 5.2 顶部布局模型

建议把顶部条抽象成固定三槽位模型：

- `left_slot`
- `center_slot`
- `system_right_slot`

其中：

- `system_right_slot` 常驻
- `left_slot` 和 `center_slot` 根据当前页配置决定是否显示及显示何种组件

这样可以覆盖三类典型场景：

1. 普通桌面页
   - 左侧空
   - 中间空
   - 右侧显示 `WiFi + 时间`
2. 锁屏首页
   - 左侧可空
   - 中间显示锁屏态核心信息
   - 右侧仍显示系统状态
3. 特殊功能页
   - 左侧标题
   - 中间状态信息
   - 右侧系统状态

### 5.3 按页配置模型

建议为每一页定义一个顶部配置结构，而不是把布局写死在事件回调里。

示意结构：

```c
typedef enum {
    TOPBAR_PAGE_MODE_LOCK,
    TOPBAR_PAGE_MODE_HOME,
    TOPBAR_PAGE_MODE_CUSTOM,
} topbar_page_mode_t;

typedef enum {
    TOPBAR_SLOT_NONE,
    TOPBAR_SLOT_TEXT,
    TOPBAR_SLOT_CUSTOM_OBJ,
} topbar_slot_type_t;

typedef struct {
    topbar_slot_type_t left_type;
    const char * left_text;
    topbar_slot_type_t center_type;
    const char * center_text;
    topbar_page_mode_t page_mode;
    bool show_system_right;
} topbar_page_config_t;
```

页切换时，根据当前 `page_index` 应用对应配置：

- 决定左侧是否显示
- 决定中间是否显示
- 决定使用何种文字或对象
- 决定是否叠加锁屏态/编辑态样式

## 6. 锁屏首页设计

锁屏页已经确认属于桌面体系里的第一页/特殊页，因此不建议做独立 screen。

推荐做法：

- `page_0` 作为锁屏页
- 仍然存在于 `desktop_tileview` 中
- 但该页使用 `TOPBAR_PAGE_MODE_LOCK`

这样有几个好处：

- 左右翻页逻辑仍然统一
- 锁屏和普通桌面共用同一套分页底座
- 顶部区域只是在切到 `page_0` 时切换扩展内容和样式

锁屏页建议至少影响以下部分：

- `center_slot` 显示锁屏核心内容
- 图标桌面区域可根据需求为空、弱化、或显示不同布局
- 如需后续加入锁图标、日期、提示文案，也仍在该模式下扩展

## 7. 编辑态联动规则

你已经明确“某些状态下顶部也参与联动或编辑态变化”，这里建议只让顶部扩展区参与模式切换，不让系统状态区失控变化。

推荐规则：

### 7.1 普通态

- 右侧系统状态正常显示
- 左/中扩展区按当前页配置显示

### 7.2 编辑态

- 长按图标进入编辑态后，`desktop_tileview` 禁止滚动
- 顶部扩展区可切换为编辑态文案或弱化显示
- 右侧 `WiFi` 和时间继续保留，不建议隐藏

原因：

- 系统状态是系统级信息，不应因桌面编辑态完全消失
- 只让扩展区变化，边界更清晰

### 7.3 跨页拖拽态

- 当图标触发左右跨页时，顶部扩展区跟随目标页配置切换
- 右侧系统状态不重建，只保持实时更新

这样页面切换与顶部变化的关系会保持一致。

## 8. 时间刷新机制

时间需要默认开机后每分钟自动刷新，建议做成独立系统状态模块。

推荐接口：

```c
void top_bar_set_time(struct top_bar * bar, const char * hhmm);
void top_bar_start_minute_timer(struct top_bar * bar);
```

建议实现：

- 初始化时立即刷新一次
- 启动一个 `lv_timer`
- 定时周期可以先简单设成 `60000 ms`
- 每次 tick 重新格式化 `HH:MM` 并更新右侧时间标签

如果后续对“整分钟对齐”有要求，可以进一步改成：

- 启动时先计算距离下一分钟的剩余毫秒
- 首次短定时对齐到分钟边界
- 之后每 `60000 ms` 刷新一次

第一阶段不强求做到分钟边界精确对齐，但接口建议预留。

## 9. WiFi 状态联动机制

`WiFi` 需要跟随硬件状态变化，因此不建议把它写死在页面创建逻辑里。

推荐接口：

```c
typedef enum {
    WIFI_STATE_OFF,
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_WEAK,
    WIFI_STATE_NORMAL,
    WIFI_STATE_STRONG,
} wifi_state_t;

void top_bar_set_wifi_state(struct top_bar * bar, wifi_state_t state);
```

建议实现原则：

- 顶部条只负责显示，不负责主动轮询硬件
- 由外部系统层在硬件状态变化时调用接口推送
- 如当前仓库只是 PC 模拟 demo，可先提供一个模拟状态更新入口

这样后续迁移到真实硬件平台时，不需要改顶部条结构。

## 10. 桌面区改造要点

### 10.1 `tileview` 下沉到 `desktop_host`

当前实现：

- `screen = lv_tileview_create(lv_screen_active())`
- 页面高度直接等于整屏高度

改造后：

- `desktop_tileview = lv_tileview_create(desktop_host)`
- 页面尺寸改为 `DESKTOP_W x DESKTOP_H`

### 10.2 坐标系切换到桌面局部坐标

当前拖拽和索引计算都直接基于屏幕坐标，改造后必须统一成：

1. 读取屏幕坐标
2. 转成 `desktop_host` 局部坐标
3. 再参与 `index_by_xy()`、吸附、换位和边缘跨页判断

这是整个改造里最容易出错的地方。

### 10.3 图标布局要按桌面区重算

建议拆分尺寸常量：

- `SCREEN_W / SCREEN_H`
- `TOP_BAR_H`
- `DESKTOP_W / DESKTOP_H`

图标网格仍可先保留 `3 x 5`，但 `ICON_START_Y` 应理解为桌面区内部顶部边距，而不是整屏顶部边距。

### 10.4 顶部变化不应影响桌面拖拽参考系

虽然顶部扩展区会按页变化，但由于顶部高度全局固定，桌面区的原点应始终稳定。

这点很重要：

- 页面不同，顶部内容可变
- 但桌面区的 `y = TOP_BAR_H` 不变

这样能避免锁屏页和普通页之间来回切换时，桌面拖拽逻辑抖动。

## 11. 推荐代码组织

建议把现有单文件逐步拆成以下模块：

- [main/icon_replace_2/icon_replace_2.c](/home/share/samba/lvgl/lv_port_pc_vscode_v9.1-test/main/icon_replace_2/icon_replace_2.c:1)
  - 负责页面整体入口和组装
- `main/icon_replace_2/icon_replace_2_top_bar.c/.h`
  - 负责顶部条创建、页级扩展区切换、时间/WiFi 更新接口
- `main/icon_replace_2/icon_replace_2_desktop.c/.h`
  - 负责 `tileview`、图标创建、拖拽换位、跨页逻辑
- `main/icon_replace_2/icon_replace_2_page_config.h`
  - 负责每页顶部配置定义
- `main/icon_replace_2/icon_replace_2_layout.h`
  - 负责尺寸常量和坐标辅助计算

如果第一阶段想快一点，也可以先不拆文件，但至少要先拆函数边界：

- `create_top_bar()`
- `apply_top_bar_page_config()`
- `create_desktop_host()`
- `create_desktop_pages()`
- `update_system_time()`
- `update_wifi_state()`

## 12. 推荐上下文结构

建议引入页面上下文结构，把顶部条和桌面状态统一挂进去：

```c
typedef struct {
    lv_obj_t * root;
    lv_obj_t * top_bar;
    lv_obj_t * top_bar_left_slot;
    lv_obj_t * top_bar_center_slot;
    lv_obj_t * top_bar_system_right;
    lv_obj_t * wifi_icon;
    lv_obj_t * time_label;
    lv_obj_t * desktop_host;
    lv_obj_t * tileview;
    lv_obj_t * page[PAGE_COUNT];
    int current_page;
    int page_icon_count[PAGE_COUNT];
    icon_type icons[PAGE_COUNT][ICON_SLOT_COUNT];
    wifi_state_t wifi_state;
    int drag_page;
    int touching;
    int touch_time_count;
    int offsetx;
    int offsety;
    int icon_shake;
    int border_lefttest_count;
    int border_righttest_count;
    lv_timer_t * minute_timer;
} desktop_ctx_t;
```

这样做的价值：

- 顶部和桌面共享同一份页面上下文
- 页切换时可以同时驱动桌面页索引和顶部扩展内容
- 时间与 `WiFi` 更新接口都有明确挂载点

## 13. 页面切换与顶部联动机制

建议把“顶栏联动”明确为页切换事件驱动，而不是散落在拖拽逻辑里。

推荐流程：

1. `desktop_tileview` 页面切换
2. 识别当前 `page_index`
3. 读取该页 `topbar_page_config_t`
4. 更新 `left_slot` 和 `center_slot`
5. 保持 `system_right_slot` 的时间和 `WiFi` 不重建

如果进入编辑态：

1. 桌面进入编辑态
2. 顶部扩展区根据“当前页 + 编辑态”做样式调整
3. 右侧系统状态继续常驻

## 14. 推荐实施步骤

### 第一阶段：建立全局顶部条骨架

- 新增 `screen_root`
- 新增 `top_bar`
- 新增 `left / center / system_right` 三槽位
- 把时间和 `WiFi` 放入 `system_right`
- 把 `tileview` 下沉到 `desktop_host`

目标：先搭出正确容器层级。

### 第二阶段：完成页级顶部扩展切换

- 建立 `topbar_page_config_t`
- 为各页接入顶部配置
- 让 `page_0` 作为锁屏页模式

目标：实现“同一套顶部条，不同页显示不同内容”。

### 第三阶段：补齐系统状态更新

- 接入分钟级时间刷新
- 接入 `WiFi` 状态更新接口
- 保证右侧系统区全局唯一

目标：让顶部条真正具备系统状态联动能力。

### 第四阶段：修正桌面拖拽参考系

- 所有拖拽逻辑改为桌面局部坐标
- 调整换位和跨页边界判断
- 验证锁屏页和普通页切换时桌面行为稳定

目标：保证结构升级后原有桌面交互不退化。

### 第五阶段：补编辑态联动

- 顶部扩展区支持编辑态样式切换
- 评估是否需要在编辑态展示不同文案或提示

目标：完成“某些状态下顶部参与联动”的需求闭环。

## 15. 验证清单

建议至少验证以下场景：

- 普通页时右上角稳定显示时间和 `WiFi`
- 时间能按分钟自动刷新
- `WiFi` 状态更新时右上角图标同步变化
- 切到锁屏首页时，顶部中间内容按锁屏模式显示
- 切到普通桌面页时，顶部恢复普通模式
- 切到特殊页时，左侧或中间可显示额外内容
- 长按进入编辑态后，顶部扩展区发生预期变化
- 编辑态下拖拽换位正常
- 编辑态下跨页拖拽正常
- 顶部内容变化不影响桌面区拖拽落点和索引计算

## 16. 最终建议

本次不建议做成“纯固定状态栏覆盖桌面”，也不建议做成“每页复制一整套顶栏”。

最合适的方式是：

- 顶部做成一条全局固定高度的条
- 右侧系统状态区全局唯一
- 左侧和中间做成按页切换的扩展区
- 锁屏页作为桌面分页体系里的特殊首页

这样可以同时满足：

- 系统状态统一维护
- 每页顶部内容可配置
- 锁屏/普通桌面/特殊页都能共存
- 后续编辑态和更多页面联动也有清晰边界

## 17. 实现状态

- 已完成 `top_bar + desktop_host` 容器拆分。
- 已完成 `page_0` 锁屏首页模式与按页顶部配置切换。
- 已完成右侧系统状态唯一实例。
- 已完成分钟级时间刷新与分钟边界重对齐。
- 已完成 `WiFi` 状态更新入口与演示级初始化。
- 已完成桌面局部坐标拖拽路径接线。

当前验证状态：

- `./bin/test_page_config` 已通过。
- `cmake --build build --target main -j` 已通过。
- `timeout 5s ./bin/main` 启动级冒烟已通过。
- 拖拽、换位、跨页的人工交互验收仍待补充；当前环境已确认程序可启动，但未完成可视窗口级手工操作闭环。
