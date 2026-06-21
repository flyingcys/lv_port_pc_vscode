# `third-party/FruitNinja/scripts/all.js` 学习文档

## 1. 文档目标

这份文档专门讲 `third-party/FruitNinja/scripts/all.js` 这一个文件，目标不是分析打包器，也不是研究 Raphael 框架细节，而是把这个原版网页 Fruit Ninja 的玩法组织方式、模块关系、场景切换、刷水果、刀痕、计分、失败与爆炸流程梳理清楚，方便你把它当成当前 LVGL v9 版本的“玩法真值参考”。

这份文档最适合两种用法：

- 你想先吃透原版网页实现，再回头看当前 `main/src/v9-fruit_ninja`
- 你已经在看当前 C 版，但想知道“这个行为在原版 JS 里到底是什么逻辑”

---

## 2. 先建立正确认识：`all.js` 不是单一脚本，而是整包模块拼起来的产物

虽然文件名叫 `all.js`，但它并不是一段线性脚本，而是把很多模块打进同一个 bundle 之后的结果。里面至少包含这些类型的模块：

- 场景与切换
- 游戏主逻辑
- 水果工厂与水果类
- 刀痕/输入系统
- 分数系统
- miss/失败系统
- 爆炸光效系统
- timeline 动画系统
- message 总线
- 一组静态 UI 对象模块

所以阅读这个文件时，最容易犯的错误就是：

- 试图从头到尾顺着看
- 被 bundle 结构、辅助库、动画工具淹没

正确方法应该是：

1. 先找到“场景系统”
2. 再找到“game 主逻辑”
3. 再找到“fruit 类与工厂”
4. 再找到“knife / score / lose / light”
5. 最后再补 timeline / message 的机制

---

## 3. 原版架构的核心特征

原版网页实现最重要的 3 个架构特征是：

### 3.1 事件驱动

很多模块之间不是直接函数互调，而是通过：

- `message.postMessage(...)`
- `message.addEventListener(...)`

来广播事件。

这意味着：

- “谁触发了什么”比较解耦
- 模块边界更清楚
- 但调试时需要跨模块追踪消息流

### 3.2 timeline 驱动动画

原版大量视觉行为不是靠一个总帧循环硬编码推进，而是靠：

- `timeline.createTask(...)`
- `timeline.setTimeout(...)`
- `timeline.setInterval(...)`

所以很多对象的 show/hide/rotate/drop/break 都是“注册一个动画任务”，而不是自己在一个全局 `update()` 里手动推进。

### 3.3 模块拆分非常细

原版喜欢把很多看似小的视觉元素都拆成独立模块，例如：

- `score`
- `lose`
- `home-mask`
- `home-desc`
- `new-game`
- `game-over`
- `light`

这和当前 C 版“全部收进 `fruit_ninja_scene.c`”形成鲜明对比。

---

## 4. 真正的玩法主线：你应该先看哪些模块

如果你只关心 Fruit Ninja 是怎么玩起来的，最关键的模块是：

1. `scripts/game`
2. `scripts/sence`
3. `fruit` 工厂 / `ClassFruit`
4. `knife`
5. `score`
6. `lose`
7. `light`

可以把它们理解成下面的职责关系：

- `sence`：负责现在处于首页还是游戏中，何时切换
- `game`：负责刷水果、计分、失败、爆炸后的玩法主逻辑
- `fruit`：负责每一颗水果是什么、怎么飞、怎么切开
- `knife`：负责输入轨迹与切割事件
- `score`：负责分数显示与更新反馈
- `lose`：负责 miss 的表现与 game over 触发
- `light`：负责炸弹后的强闪光演出

---

## 5. 游戏完整流程：从打开页面到一局结束

## 5.1 首页阶段

原版首页不是一张静态海报，而是一组按时间排布的对象动画：

- 背景
- home mask
- logo
- ninja
- home desc
- dojo
- new game
- quit
- new sign
- 3 个首页水果

在 `showMenu()` 里：

- `peach = fruit.create("peach", 137, 333, true)`
- `sandia = fruit.create("sandia", 330, 322, true)`
- `boom = fruit.create("boom", 552, 367, true, 2500)`

然后通过一个 `group` 列表，按延迟时间统一 show 出来。

这说明首页其实是一个“编排场景”，而不是临时堆 UI。

## 5.2 从首页进入新游戏

进入新游戏不是点击普通按钮，而是通过场景切换：

- `showNewGame()`
- `game.start()`
- 播放开始音效

`game.start()` 本身又不会立刻开刷，而是：

- 延迟 500ms
- 把 `game-state` 设为 `playing`
- 开始每秒触发一次刷水果逻辑

这个 500ms 节奏非常重要，当前 C 版也保留了这个体感。

## 5.3 游戏进行中

进入玩法态后，主线是：

1. 每秒检查是否需要补足当前波次水果数
2. 新水果从底部飞出
3. 刀痕切中水果
4. 普通水果被切开、得分增加
5. 炸弹被切中则进入爆炸演出
6. 普通水果漏掉则触发 miss
7. miss 三次 game over

所以网页原版虽然模块拆得细，但主玩法骨架和当前 C 版几乎一致。

## 5.4 游戏结束后

结束后主要有两条路径：

- `lose.showLoseAt()` 到 3 次 miss 触发 `game.over`
- 炸弹白闪演出结束后触发 `game.over`

然后通过 scene 系统回到首页菜单。

---

## 6. `scripts/game`：玩法主逻辑模块

这一段是最值得吃透的模块之一。

它维护的核心状态有：

- `scoreNumber`
- `volleyNum`
- `volleyMultipleNumber`
- `fruits[]`
- `gameInterval`

### 6.1 `barbette()`：刷水果核心

这个函数逻辑非常直接：

- 如果当前水果数已经达到 `volleyNum`，就停止
- 否则创建一个新水果，从 `startY = 600` 起飞
- 放进 `fruits[]`
- 播放 throw 音效
- 再递归调用自己，直到补齐这一波

它的关键思想和当前 C 版完全一致：

**不是固定每秒加一个，而是每轮补齐到目标水果数。**

### 6.2 `start()`：游戏开局

- 创建 throw 音效和 boom 音效
- 500ms 后切到 `playing`
- 启动每 1000ms 触发一次的 `barbette()`

这段等价于当前 C 版里：

- `enter_running()`
- `spawn_elapsed_ms = 500`
- `spawn_interval_ms = 1000`

### 6.3 `applyScore()`：难度增长规则

规则是：

- 如果 `score > volleyNum * volleyMultipleNumber`
- `volleyNum++`
- `volleyMultipleNumber += 50`

这就是后续水果越来越多的根源。

### 6.4 `sliceAt()`：命中结果分发

- 普通水果：切开、更新分数、应用难度增长
- 炸弹：播放 boom、暂停所有水果、触发背景 wobble 和 light 演出

当前 C 版的 `slice_fruit()` 就是它的直系对应思想，只是重写成了同步函数模式。

---

## 7. 水果系统：`infos`、`ClassFruit`、工厂函数

这是原版实现的玩法基石。

## 7.1 `infos`：水果真值表

这里定义了：

- `boom`
- `peach`
- `sandia`
- `apple`
- `banana`
- `basaha`

每种都有：

- 图片路径
- 宽高
- 碰撞半径
- 角度修正
- 果汁颜色

当前 LVGL C 版的 `g_fruit_defs[]` 明显就是按这张表重写过去的。

## 7.2 `getType()`：炸弹概率

规则是：

- `random(8) == 4` -> `boom`
- 否则从普通水果里随机

当前 C 版的 `choose_fruit_def()` 几乎一模一样。

## 7.3 `ClassFruit` 的职责

它负责单颗水果的全部局部行为：

- 创建本体图与阴影
- 旋转
- 上抛
- 下落
- 切开
- 分裂
- 暂停
- 移除

也就是说，在原版里，水果对象本身是一个更“面向对象”的实体；
而在当前 C 版里，这些职责很多都被 scene 主循环接管了。

## 7.4 `shotOut()` 与 `fallOff()`

原版的水果运动不是统一全局 update，而是每个水果通过 timeline task 驱动：

- 上抛任务
- 结束后切下落任务
- 旋转任务并行跑

当前 C 版把这套任务化模型压平到了 `update_fruits()` 里统一推进。

## 7.5 `broken()` 与 `apart()`

普通水果被切开后：

- 触发 flash
- 触发 juice
- 生成左右半片
- 左右半片继续做掉落和旋转动画

当前 C 版保留了 flash 和 apart，但省略了 juice 系统，这是两者的重要差异之一。

---

## 8. 首页场景系统：`showMenu()` / `hideMenu()` / `showNewGame()`

原版 scene 系统承担的是“场景编排”职责，而不是只做一个 if/else 跳转。

## 8.1 `showMenu()`

它会：

- 创建首页 3 个水果
- 标记它们是 home menu fruit
- 给不同页面元素安排不同的 show 时间
- 播放菜单音乐
- 2500ms 后让场景进入 ready

这说明原版首页是一个完整的时间轴动画场景。

## 8.2 `hideMenu()`

它会：

- 隐藏文字和 UI
- 让首页水果以 `fallOff` 的方式离场
- 停止菜单音乐
- 等待水果掉落动画时长结束后切下一个场景

这也是当前 C 版明显简化掉的一块：当前版本没有完整的“首页离场动画链”。

## 8.3 `showNewGame()` / `hideNewGame()`

原版这里除了切换玩法态，还负责：

- score.show()
- lose.show()
- game.start()
- 播放开始音效

也就是说，HUD 是作为场景切换的一部分被打开/关闭的，不是玩法层临时想到再 show。

---

## 9. `knife`：输入与切割广播系统

虽然这份 bundle 里刀痕模块没有前面那些模块那样直观，但它的作用可以从 `message.postMessage("slice", knife)` 这一类调用反推出：

- 玩家拖拽会形成 knife 数据
- knife 数据里至少包含一段切割线的坐标
- 这段数据被广播给监听者
- 游戏逻辑再去判断切到了哪颗水果

所以原版的思路是：

- 输入系统先产出一个“刀痕事件”
- 游戏系统监听这个事件并做命中检测

当前 C 版把这条链压缩成了：

- `fruit_ninja_input_push_point()` 返回 `segment`
- 直接同步调用 `handle_segment_hits()`

少了消息层，但核心思想其实没有变。

---

## 10. `score` 与 `lose`：HUD 与失败系统

## 10.1 `score`

原版 `score` 模块负责：

- 左上角 score 图标
- 当前分数数字
- `BEST 999` 文本
- show/hide 动画
- 分数变化时图标轻微缩放反馈

当前 C 版只保留了：

- score 图标
- 当前分数
- 图标 pulse

也就是说当前版本保留了最核心信息，但删掉了 best 文本和滑入演出。

## 10.2 `lose`

原版 `lose` 负责两层反馈：

1. 顶部 3 个 X 图标逐步点亮
2. 屏幕底部在水果漏掉位置弹出 `lose.png` 的放大反馈

并且当 miss 达到 3 次时，会直接 `message.postMessage("game.over")`。

当前 C 版只保留了顶部 miss 图标，并在 `update_fruits()` 内直接判断是否进入 `enter_game_over()`。

所以当前版本在 failure UX 上是明显简化的。

---

## 11. `light`：炸弹之后的高亮白闪系统

原版炸弹命中后，不是直接 game over，而是：

1. 生成一组 light 动画
2. 再调用 `overWhiteLight.show`
3. 创建一个持续 4 秒的白色遮罩
4. 白遮罩透明度渐渐减弱
5. 最终才 `message.postMessage("game.over")`

这和当前 C 版的：

- `enter_exploding()`
- `white_flash_overlay`
- `smoke_overlay`
- 4 秒后 `enter_game_over()`

在玩法意图上是完全一致的，只是实现机制从消息 + task 变成了 scene 内显式状态推进。

---

## 12. 如果你要把原版 JS 当“行为真值”，最应该盯哪些点

不是所有 UI 细节都需要逐像素对齐。真正高价值的真值点主要有这些：

1. 刷水果规则
   - 波次补齐，不是固定单个追加
2. 炸弹概率
   - `1/8`
3. 起飞高度
   - `startY = 600`
4. 开局节奏
   - 500ms 后才进入真正刷怪
5. 难度增长公式
   - `score > volleyNum * volleyMultipleNumber`
6. 首页三水果位置
   - `137,333` / `330,322` / `552,367`
7. 爆炸不是立即结束，而是先有 4 秒左右演出

这些点比是否完全复制某个细小 easing 更有参考价值。

---

## 13. 推荐阅读顺序

如果你准备真正去啃 `all.js`，建议按这个顺序：

1. 先找 `scripts/game`
2. 再找 `showMenu()` / `showNewGame()` / `switchSence()`
3. 再找 `infos` / `ClassFruit` / `getType()`
4. 再看 `score` / `lose` / `light`
5. 最后回头理解 `timeline` / `message` 如何把这一切串起来

这样读，你会先抓住玩法主线，再补底层框架，不容易迷路。

---

## 14. 一句话总结

`third-party/FruitNinja/scripts/all.js` 的本质，是一个“通过 message 总线和 timeline 动画任务，把首页编排、刀痕输入、水果生命周期、计分、miss、炸弹爆炸和场景切换组织起来的网页小游戏框架”。

它的价值不在于所有实现细节都必须照抄，而在于它提供了这套 Fruit Ninja 的核心行为真值：

- 水果怎么出
- 玩家怎么切
- 分数怎么涨
- 炸弹怎么罚
- 首页怎么开局
- 一局怎么结束

如果你把它和当前 `main/src/v9-fruit_ninja` 对照起来看，会非常容易理解：

- 原版 JS 更像“模块消息驱动的浏览器实现”
- 当前 C 版更像“集中式 scene 驱动的本地 LVGL 重写版”

但两者的玩法骨架，其实是一脉相承的。
