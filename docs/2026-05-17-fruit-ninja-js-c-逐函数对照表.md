# Fruit Ninja 原版 JS 与当前 C 版逐函数对照表

## 1. 文档目标

这份文档不是做宏观介绍，而是把 `third-party/FruitNinja/scripts/all.js` 里的关键函数/概念，与当前 `main/src/v9-fruit_ninja` 的实际实现做更细粒度的逐函数对照。

适合用法：

- 你正在看原版 JS，想知道当前 C 版该去哪里找对应实现
- 你准备继续做更高保真对齐，想快速定位“这一块是已经对齐、部分对齐、还是明显简化了”

---

## 2. 阅读方式

看这张表时，不要强行要求“一个 JS 函数一定等于一个 C 函数”。

因为两边架构不同：

- 原版 JS：消息总线 + timeline + 多模块拆分
- 当前 C：scene 集中调度 + 单 tick 驱动 + 辅助模块支撑

所以很多时候更合理的关系是：

- 一个 JS 函数 -> C 版 2-4 个函数组合
- 或多个 JS 模块 -> C 版一个 scene 子区块

---

## 3. 入口与场景切换对照

| 原版 JS | 当前 C | 对齐情况 | 说明 |
|---|---|---|---|
| `showMenu()` | `enter_home()` + `update_home_animation()` + `build_home_menu_fruits()` | 部分对齐 | 首页主要视觉与 3 水果入口保留，但未完整保留多菜单语义 |
| `hideMenu()` | `clear_home_menu_fruits()` + `hide_obj(home_layer)` 的状态切换效果 | 部分对齐 | 当前没有完整首页离场动画链 |
| `showNewGame()` | `enter_running()` | 高度对齐 | 都负责进入玩法态并打开 HUD |
| `hideNewGame()` | `enter_home()` / `enter_game_over()` 间接收口 | 部分对齐 | 当前没有独立“退出玩法态到别处”的 scene 函数 |
| `switchSence()` | `enter_home()` / `enter_running()` / `enter_exploding()` / `enter_game_over()` | 结构性重写 | JS 有统一场景切换器，C 版变成显式状态入口函数集合 |

---

## 4. 游戏主逻辑对照

| 原版 JS | 当前 C | 对齐情况 | 说明 |
|---|---|---|---|
| `game.start()` | `enter_running()` + `update_timer_cb()` | 高度对齐 | 开局后半秒进入刷怪节奏 |
| `barbette()` | `target_fruit_count()` + `active_running_fruits()` + `spawn_one_fruit()` | 高度对齐 | 都是“补齐到当前波次数量”而非固定追加 |
| `applyScore()` | `slice_fruit()` 内难度增长段 | 高度对齐 | 公式保持一致 |
| `gameOver()` | `enter_game_over()` | 高度对齐 | 都负责切结束态、显示结算、停止正常玩法 |
| `pauseAllFruit()` | `enter_exploding()` + `EXPLODING` 状态推进 | 部分对齐 | 当前没有显式 pause API，而是由爆炸状态接管行为 |

---

## 5. 水果模板与概率对照

| 原版 JS | 当前 C | 对齐情况 | 说明 |
|---|---|---|---|
| `infos` | `g_fruit_defs[]` | 高度对齐 | 水果种类、尺寸、半径、基础角度都基本沿用 |
| `getType()` | `choose_fruit_def()` | 高度对齐 | `1/8` 炸弹概率一致 |
| `types` | `g_fruit_defs[0..4]` 的普通水果集合 | 高度对齐 | 普通水果池一致 |

---

## 6. 单颗水果生命周期对照

| 原版 JS | 当前 C | 对齐情况 | 说明 |
|---|---|---|---|
| `fruit.create(...)` | `alloc_fruit()` + `spawn_one_fruit()` | 高度对齐 | 都是从模板构造出一个运行中水果实例 |
| `ClassFruit.set()` | `spawn_one_fruit()` 内创建 `shadow_image` / `whole_image` | 高度对齐 | 对象构建方式不同，但结果相同 |
| `pos()` | `update_single_fruit_visual()` | 高度对齐 | 负责把逻辑坐标映射到图像 |
| `rotate()` | `update_fruits()` 中 `angle += angular_velocity * dt` | 部分对齐 | JS 用 timeline，C 版用统一 tick |
| `shotOut()` | `spawn_one_fruit()` + `update_fruits()` 的上抛阶段 | 高度对齐 | 轨迹真值思路已对齐 |
| `fallOff()` | `update_fruits()` 的下落阶段 | 高度对齐 | 都是上抛结束后进入下落 |
| `pause()` | `EXPLODING` 里减速 + 状态接管 | 部分对齐 | 当前没有对单颗水果暴露 pause 函数 |
| `remove()` | `destroy_if_present()` + `active=false` + `memset` | 高度对齐 | 对象清理方式更偏 C 风格 |

---

## 7. 切开与碎片对照

| 原版 JS | 当前 C | 对齐情况 | 说明 |
|---|---|---|---|
| `broken(angle)` | `slice_fruit()` | 高度对齐 | 负责命中后结果分流 |
| `apart(angle)` | `spawn_fragment()` + `update_fragments()` | 高度对齐 | 左右半片掉落逻辑保留 |
| `bImage1` / `bImage2` | `fragments[]` 中两个 fragment | 高度对齐 | 当前用碎片对象池承载 |
| `juice.create(...)` | 无直接等价 | 明显简化 | 当前 C 版尚未实现果汁喷溅系统 |
| `flash.showAt(...)` | `spawn_flash()` + `clear_flash_if_needed()` | 高度对齐 | 刀光反馈保留 |

---

## 8. 首页菜单水果对照

| 原版 JS | 当前 C | 对齐情况 | 说明 |
|---|---|---|---|
| `fruit.create("peach", 137, 333, true)` | `g_home_menu_defs[0]` + `g_home_menu_positions[0]` | 高度对齐 | 位置与水果种类对齐 |
| `fruit.create("sandia", 330, 322, true)` | `g_home_menu_defs[1]` + `g_home_menu_positions[1]` | 高度对齐 | 中间水果仍承担开局入口 |
| `fruit.create("boom", 552, 367, true, 2500)` | `g_home_menu_defs[2]` + `g_home_menu_positions[2]` | 高度对齐 | 右侧炸弹菜单水果保留 |
| `isDojoIcon / isNewGameIcon / isQuitIcon` | `handle_home_menu_hits()` 内索引语义 | 部分对齐 | 当前只保留 new game 主路径，dojo/quit 未完整落地 |

---

## 9. 刀痕与输入对照

| 原版 JS | 当前 C | 对齐情况 | 说明 |
|---|---|---|---|
| `knife` 模块 | `fruit_ninja_input.c` | 结构性重写 | 输入轨迹功能保留，但封装形式不同 |
| `message.postMessage("slice", knife)` | `fruit_ninja_input_push_point()` 返回 `segment` 后同步调用 `handle_segment_hits()` | 结构性重写 | 从广播消息改为本地同步分发 |
| 刀痕持续存在并淡出 | `fruit_ninja_input_tick()` | 高度对齐 | 当前是简单线条淡出实现 |
| 轨迹点队列 | `trail.points[]` | 高度对齐 | 都会缓存最近一段轨迹 |

---

## 10. 命中检测对照

| 原版 JS | 当前 C | 对齐情况 | 说明 |
|---|---|---|---|
| `sliceAt()` 之前的角度/命中判断 | `handle_segment_hits()` + `fruit_ninja_segment_hits_circle()` | 高度对齐 | 当前把几何命中显式抽成独立 C 函数 |
| 首页水果命中 | `handle_home_menu_hits()` | 高度对齐 | 首页与运行态命中逻辑分开实现 |
| 运行中水果命中 | `handle_segment_hits()` | 高度对齐 | 只在 `RUNNING` 状态生效 |

---

## 11. 分数与难度对照

| 原版 JS | 当前 C | 对齐情况 | 说明 |
|---|---|---|---|
| `score.number(++scoreNumber)` | `game->score += 1` + `update_score_label()` | 高度对齐 | 分数增长主逻辑一致 |
| score 图标缩放反馈 | `update_score_pulse()` | 高度对齐 | 当前也保留了 score icon pulse |
| `BEST 999` | 无直接等价 | 简化 | 当前未实现 best 分数 HUD |
| `applyScore()` 难度升级 | `slice_fruit()` 内阈值判断 | 高度对齐 | 波次成长规则一致 |

---

## 12. miss / lose / game over 对照

| 原版 JS | 当前 C | 对齐情况 | 说明 |
|---|---|---|---|
| `lose.showLoseAt(originX)` | `update_fruits()` 中 miss++ + `update_miss_icons()` | 部分对齐 | 当前保留顶部 miss UI，弱化底部提示演出 |
| miss 达到 3 次 -> `game.over` | `misses >= 3` -> `enter_game_over()` | 高度对齐 | 结束条件一致 |
| `gameOver.show()` | `show_obj(game_over_image)` + `show_obj(restart_label)` | 高度对齐 | 结束页 UI 语义保留 |
| 点击后回首页 | `input_event_cb()` -> `enter_home()` | 高度对齐 | 当前实现更直接 |

---

## 13. 炸弹爆炸对照

| 原版 JS | 当前 C | 对齐情况 | 说明 |
|---|---|---|---|
| `boomSnd.play()` | `fruit_ninja_audio_play_boom()` | 高度对齐 | 爆炸音效保留 |
| `background.wobble()` | 无直接等价 | 简化 | 当前未实现背景 wobble |
| `light.start(boom)` | `enter_exploding()` | 高度对齐 | 爆炸进入演出态 |
| `overWhiteLight.show` | `white_flash_overlay` + `smoke_overlay` + `update_timer_cb()` exploding 分支 | 高度对齐 | 当前显式状态推进代替 message/timeline |
| 4 秒后 `game.over` | `FRUIT_NINJA_EXPLODING_MS` 后 `enter_game_over()` | 高度对齐 | 时间尺度一致 |

---

## 14. 资源与音频对照

| 原版 JS | 当前 C | 对齐情况 | 说明 |
|---|---|---|---|
| 浏览器直接读 `images/...` | `fruit_ninja_assets_build_image_path()` | 结构性重写 | 当前通过 `S:` 路径接 LVGL 文件图片链 |
| `sound.create(...)` | `fruit_ninja_audio_init()` + `fruit_ninja_audio_play_*()` | 结构性重写 | 当前统一经由 SDL_mixer |
| 页面对象模块各自引用资源名 | `init_ui_asset_paths()` + `g_fruit_defs[]` | 高度对齐 | 当前也采用集中资源键名组织 |

---

## 15. 哪些是已经高度对齐的，哪些还值得继续补

### 已经高度对齐的高价值行为

- 500ms 后开刷
- 波次补齐式刷水果
- `1/8` 炸弹概率
- `startY = 600`
- 首页 3 水果位置
- score 驱动难度增长公式
- 炸弹进入 4 秒爆炸演出后 game over
- 线段切圆形碰撞体的玩法主路径

### 仍有明显简化/可继续补的点

- `juice` 果汁喷溅
- `background.wobble()` 背景抖动
- 首页 dojo / quit 多入口完整语义
- `lose.png` 底部漏切提示演出
- `BEST` 分数与更完整 HUD 动画
- 原版更细粒度的 timeline 动画层次

---

## 16. 最推荐的使用方式

这份表最适合和另外两份文档一起配合：

1. 先看 `docs/2026-05-17-third-party-fruit-ninja-all-js-学习文档.md`
   - 建立原版玩法骨架
2. 再看 `docs/2026-05-17-v9-fruit-ninja-学习文档.md`
   - 理解当前 C 版整体实现
3. 最后看这份逐函数对照表
   - 快速定位某个 JS 行为在 C 版到底落到了哪里

这样三份文档会形成一个很完整的学习闭环。
