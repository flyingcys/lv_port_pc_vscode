# Fruit Ninja LVGL v9 1:1 复刻 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 `main/src/v9-fruit_ninja/`(LVGL v9.1)的效果做到与 JS 原版 `third-party/FruitNinja` 一模一样,并自适应 800×480 / 640×480 / 480×272 三种分辨率。

**Architecture:** 逻辑坐标固定 640×480,新增 viewport 模块做 letterbox 等比缩放映射到物理屏;矢量特效(刀光拖尾 / 汁液 / 炸弹光线 / 火焰)统一画到一个全屏 `lv_canvas` 特效层,每帧重绘;位图(水果 / 分裂两半 / 闪光 / UI)用 `lv_image`。顺势把 1110 行的 `fruit_ninja_scene.c` 按职责拆成 viewport / easing / physics / effects / state / input 模块。

**Tech Stack:** C99, LVGL 9.1, SDL2 模拟器, SDL2_mixer 音频, libpng/tjpgd 解码, CMake + CTest, assert.h 单测。

**参考文档:** 设计 spec 见 `docs/superpowers/specs/2026-06-14-fruit-ninja-1to1-design.md`(含完整 JS 数值常量对照表)。

---

## 关键事实(执行前必读)

- **构建配置:** `cmake -S . -B build`(首次或改了 CMakeLists 后);**增量构建:** `cmake --build build -j`
- **主程序:** 产物在 `./bin/main`(根目录 `bin/`,不是 `build/bin/`)。**必须在项目根目录运行**(资源用 `S:` 前缀映射到相对工作目录)。
- **三档运行:** `./bin/main 800 480` / `./bin/main 640 480` / `./bin/main 480 272`(`argv[1]`=宽,`argv[2]`=高;不传参默认 800×480)。
- **截图:** `AM_SHOT=/abs/path/out.png ./bin/main 800 480`(跑 200 帧后截图退出)。游戏是动态的,截图用于验证静态布局(首页 / letterbox 居中),特效以肉眼观察运行为主。
- **跑测试:** `cmake --build build -j && ctest --test-dir build --output-on-failure`;单个:`ctest --test-dir build -R fruit_ninja_viewport_test -V`,或直接 `./bin/fruit_ninja_viewport_test`。
- **源文件 GLOB:** `main/src/v9-fruit_ninja/*.c` 自动纳入 `main`(CMakeLists:51-53,156),**新增 .c 无需改 CMakeLists**(但需重跑 `cmake -S . -B build` 让 CONFIGURE_DEPENDS 重扫)。
- **测试非 GLOB:** 新增单测必须手工加到 `CMakeLists.txt`(范本见 226-231 行 `fruit_ninja_collision_test`)。
- **代码风格:** 4 空格缩进;函数加模块前缀 `fruit_ninja_*`(对外)或文件内 `static`;宏 `FRUIT_NINJA_*` 全大写;`*_t` typedef;全局静态变量 `g_` 前缀;文件直接以 `#include` 开头,无版权头。
- **测试风格:** `#include <assert.h>` + `static void test_xxx(void)` + `int main(void){ test_xxx(); return 0; }`;浮点比较用容差 `assert(fabsf(a-b) < 1e-3f)`。

---

## 文件结构总览

拆分后 `main/src/v9-fruit_ninja/` 的目标结构(★=新建):

| 文件 | 职责 |
|------|------|
| `fruit_ninja.h` | 入口声明(不变) |
| `fruit_ninja_model.h` | 数据结构(扩展:特效粒子结构体 + 逻辑尺寸常量) |
| `fruit_ninja_easing.h/.c` ★ | 缓动函数(纯数学,复用) |
| `fruit_ninja_viewport.h/.c` ★ | 逻辑↔物理坐标 letterbox 映射(纯数学) |
| `fruit_ninja_internal.h` ★ | 跨模块共享的 static helper 声明 + 常量集中 |
| `fruit_ninja_physics.c` ★ | 水果抛物线 / 旋转 / 生成 / 碎片物理 |
| `fruit_ninja_effects.h/.c` ★ | Canvas 特效层:刀光 / 汁液 / 闪光 / 炸弹光线 / 火焰 / 白闪 |
| `fruit_ninja_state.c` ★ | 状态机 / HUD / 首页菜单动画 |
| `fruit_ninja_input.c` | 轨迹采集(扩展:切割派发 slice/hits) |
| `fruit_ninja_collision.c/.h` | 线段-圆碰撞(不变) |
| `fruit_ninja_assets.c/.h` | 资源路径(小改:补登记 UI 图) |
| `fruit_ninja_audio.c/.h` | 音效(小改:对齐触发时机) |
| `fruit_ninja_scene.c` | 瘦身:入口 + 图层/canvas 创建 + 主循环编排 |

> 说明:Phase 2(重构)只**移动**既有代码、不改行为,因此其 Task 给"函数迁移清单 + 新头文件契约 + 编译回归",不重复粘贴被移动的函数体。Phase 4–6 的特效是**新增代码**,给完整实现。

---

# Phase 0:集成基线

目的:让 fruit_ninja 能跑起来并建立三档截图基线,作为"行为不变"重构的对照。

### Task 0:启用 fruit_ninja 入口 + 三档基线截图 ✅

**Files:**
- Modify: `main/src/main.c:86-88`

- [x] **Step 1:改 main.c 启用 fruit_ninja**

把 `main/src/main.c` 第 86-88 行:
```c
  // fruit_ninja_start();
  // lv_demo_music();
  apple_music_create();
```
改为:
```c
  fruit_ninja_start();
  // lv_demo_music();
  // apple_music_create();
```

- [x] **Step 2:构建**

Run: `cmake -S . -B build && cmake --build build -j`
Expected: 编译成功,生成 `./bin/main`,无报错。

- [x] **Step 3:三档运行,肉眼确认现状能跑**(改用 AM_SHOT 截图验证:800/640 首页正常,480×272 超界——预期基线)

Run(在项目根,有图形界面):
```bash
./bin/main 800 480
```
Expected: 出现 Fruit Ninja 首页;能进游戏、划水果。记录现状刀光/切水果效果(作为"待改进"基线)。再分别试 `./bin/main 640 480`、`./bin/main 480 272`,记录现状在小屏下的表现(预期 480×272 下会错位/超界——这正是要修的)。

- [x] **Step 4:提交** (commit `c4f70eb`)

```bash
git add main/src/main.c
git commit -m "build(fruit-ninja): 启用 fruit_ninja_start 入口作为开发基线"
```

---

# Phase 1:纯逻辑地基(TDD)

新增两个纯数学模块,严格 TDD。它们不依赖 LVGL 运行时,可独立单测。

### Task 1:easing 缓动模块 ✅

把现有散落在 `scene.c` 的 `ease_out_quad/ease_in_quad` 提取为公共模块,并补齐 JS 用到的 `ease_out_expo`(汁液距离)、`ease_out_back`(生命图标弹出)。

**Files:**
- Create: `main/src/v9-fruit_ninja/fruit_ninja_easing.h`
- Create: `main/src/v9-fruit_ninja/fruit_ninja_easing.c`
- Create: `main/tests/fruit_ninja_easing_test.c`
- Modify: `CMakeLists.txt`(测试段)

- [x] **Step 1:写头文件**

`main/src/v9-fruit_ninja/fruit_ninja_easing.h`:
```c
#ifndef FRUIT_NINJA_EASING_H
#define FRUIT_NINJA_EASING_H

/* 所有缓动入参 t、返回值均归一化到 [0,1](ease_out_back 返回值会短暂越界,属预期回弹) */
float fruit_ninja_ease_out_quad(float t);  /* JS quadratic.co:减速 */
float fruit_ninja_ease_in_quad(float t);   /* JS quadratic.ci:加速 */
float fruit_ninja_ease_out_expo(float t);  /* JS exponential.co */
float fruit_ninja_ease_out_back(float t);  /* JS back.co:末端回弹 */

#endif
```

- [x] **Step 2:写失败测试**

`main/tests/fruit_ninja_easing_test.c`:
```c
#include <assert.h>
#include <math.h>
#include "../src/v9-fruit_ninja/fruit_ninja_easing.h"

#define APPROX(a, b) (fabsf((a) - (b)) < 1e-3f)

static void test_quad_endpoints(void) {
    assert(APPROX(fruit_ninja_ease_out_quad(0.0f), 0.0f));
    assert(APPROX(fruit_ninja_ease_out_quad(1.0f), 1.0f));
    assert(APPROX(fruit_ninja_ease_out_quad(0.5f), 0.75f)); /* 1-(0.5)^2 */
    assert(APPROX(fruit_ninja_ease_in_quad(0.0f), 0.0f));
    assert(APPROX(fruit_ninja_ease_in_quad(1.0f), 1.0f));
    assert(APPROX(fruit_ninja_ease_in_quad(0.5f), 0.25f)); /* 0.5^2 */
}

static void test_expo_endpoints(void) {
    assert(APPROX(fruit_ninja_ease_out_expo(0.0f), 0.0f));
    assert(APPROX(fruit_ninja_ease_out_expo(1.0f), 1.0f));
    assert(fruit_ninja_ease_out_expo(0.5f) > 0.9f); /* 指数 out 前段快 */
}

static void test_back_overshoot(void) {
    assert(APPROX(fruit_ninja_ease_out_back(0.0f), 0.0f));
    assert(APPROX(fruit_ninja_ease_out_back(1.0f), 1.0f));
    /* back.co 在中后段会越过 1 再回落 */
    float peak = fruit_ninja_ease_out_back(0.6f);
    assert(peak > 1.0f);
}

int main(void) {
    test_quad_endpoints();
    test_expo_endpoints();
    test_back_overshoot();
    return 0;
}
```

- [x] **Step 3:加 CMake 测试目标**

在 `CMakeLists.txt` 第 254 行(`fruit_ninja_model_test` 的 `set_tests_properties` 之后)插入:
```cmake
add_executable(fruit_ninja_easing_test
    ${PROJECT_SOURCE_DIR}/main/tests/fruit_ninja_easing_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9-fruit_ninja/fruit_ninja_easing.c
)
target_include_directories(fruit_ninja_easing_test PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(fruit_ninja_easing_test m)
add_test(NAME fruit_ninja_easing_test COMMAND $<TARGET_FILE:fruit_ninja_easing_test>)
set_tests_properties(fruit_ninja_easing_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
```

- [x] **Step 4:跑测试确认失败(无实现)**

Run: `cmake -S . -B build && cmake --build build -j 2>&1 | tail -20`
Expected: 链接失败,`undefined reference to 'fruit_ninja_ease_out_quad'`。

- [x] **Step 5:写实现**

`main/src/v9-fruit_ninja/fruit_ninja_easing.c`:
```c
#include "fruit_ninja_easing.h"

#include <math.h>

float fruit_ninja_ease_out_quad(float t) {
    float inv = 1.0f - t;
    return 1.0f - inv * inv;
}

float fruit_ninja_ease_in_quad(float t) {
    return t * t;
}

float fruit_ninja_ease_out_expo(float t) {
    if(t >= 1.0f) return 1.0f;
    return 1.0f - powf(2.0f, -10.0f * t);
}

float fruit_ninja_ease_out_back(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    float p = t - 1.0f;
    return 1.0f + c3 * p * p * p + c1 * p * p;
}
```

- [x] **Step 6:跑测试确认通过**

Run: `cmake --build build -j && ctest --test-dir build -R fruit_ninja_easing_test -V`
Expected: `1 test passed`。

- [x] **Step 7:提交**

```bash
git add main/src/v9-fruit_ninja/fruit_ninja_easing.h main/src/v9-fruit_ninja/fruit_ninja_easing.c main/tests/fruit_ninja_easing_test.c CMakeLists.txt
git commit -m "feat(fruit-ninja): 新增 easing 缓动模块(quad/expo/back)+ 单测"
```

### Task 2:viewport letterbox 映射模块 ✅

**Files:**
- Create: `main/src/v9-fruit_ninja/fruit_ninja_viewport.h`
- Create: `main/src/v9-fruit_ninja/fruit_ninja_viewport.c`
- Create: `main/tests/fruit_ninja_viewport_test.c`
- Modify: `CMakeLists.txt`(测试段)

- [x] **Step 1:写头文件**

`main/src/v9-fruit_ninja/fruit_ninja_viewport.h`:
```c
#ifndef FRUIT_NINJA_VIEWPORT_H
#define FRUIT_NINJA_VIEWPORT_H

/* 逻辑坐标系固定 640x480;按物理屏等比缩放居中(letterbox) */
void  fruit_ninja_viewport_init(int phys_w, int phys_h);
float fruit_ninja_viewport_x(float logic_x);        /* 逻辑X -> 物理X */
float fruit_ninja_viewport_y(float logic_y);        /* 逻辑Y -> 物理Y */
float fruit_ninja_viewport_len(float logic_len);    /* 标量缩放(线宽/半径) */
float fruit_ninja_viewport_scale(void);             /* 当前 scale */
float fruit_ninja_viewport_to_logic_x(float phys_x);/* 物理X -> 逻辑X(输入命中) */
float fruit_ninja_viewport_to_logic_y(float phys_y);

#endif
```

- [x] **Step 2:写失败测试**

`main/tests/fruit_ninja_viewport_test.c`:
```c
#include <assert.h>
#include <math.h>
#include "../src/v9-fruit_ninja/fruit_ninja_viewport.h"

#define APPROX(a, b) (fabsf((a) - (b)) < 0.05f)

static void test_800x480(void) {
    fruit_ninja_viewport_init(800, 480);
    assert(APPROX(fruit_ninja_viewport_scale(), 1.0f));
    assert(APPROX(fruit_ninja_viewport_x(0.0f), 80.0f));     /* 左留边 80 */
    assert(APPROX(fruit_ninja_viewport_x(320.0f), 400.0f));  /* 逻辑中心 -> 屏幕中心 */
    assert(APPROX(fruit_ninja_viewport_y(240.0f), 240.0f));
    assert(APPROX(fruit_ninja_viewport_len(10.0f), 10.0f));
}

static void test_640x480(void) {
    fruit_ninja_viewport_init(640, 480);
    assert(APPROX(fruit_ninja_viewport_scale(), 1.0f));
    assert(APPROX(fruit_ninja_viewport_x(320.0f), 320.0f));
}

static void test_480x272(void) {
    fruit_ninja_viewport_init(480, 272);
    assert(APPROX(fruit_ninja_viewport_scale(), 272.0f / 480.0f)); /* 0.5667 */
    assert(APPROX(fruit_ninja_viewport_x(320.0f), 240.0f));  /* 屏幕水平中心 */
    assert(APPROX(fruit_ninja_viewport_y(240.0f), 136.0f));  /* 屏幕垂直中心 */
}

static void test_roundtrip(void) {
    fruit_ninja_viewport_init(480, 272);
    float px = fruit_ninja_viewport_x(321.0f);
    assert(APPROX(fruit_ninja_viewport_to_logic_x(px), 321.0f));
    float py = fruit_ninja_viewport_y(123.0f);
    assert(APPROX(fruit_ninja_viewport_to_logic_y(py), 123.0f));
}

int main(void) {
    test_800x480();
    test_640x480();
    test_480x272();
    test_roundtrip();
    return 0;
}
```

- [x] **Step 3:加 CMake 测试目标**

在 Task 1 插入的 easing 测试块之后,继续插入:
```cmake
add_executable(fruit_ninja_viewport_test
    ${PROJECT_SOURCE_DIR}/main/tests/fruit_ninja_viewport_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9-fruit_ninja/fruit_ninja_viewport.c
)
target_include_directories(fruit_ninja_viewport_test PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(fruit_ninja_viewport_test m)
add_test(NAME fruit_ninja_viewport_test COMMAND $<TARGET_FILE:fruit_ninja_viewport_test>)
set_tests_properties(fruit_ninja_viewport_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
```

- [x] **Step 4:跑测试确认失败**

Run: `cmake -S . -B build && cmake --build build -j 2>&1 | tail -20`
Expected: `undefined reference to 'fruit_ninja_viewport_init'`。

- [x] **Step 5:写实现**

`main/src/v9-fruit_ninja/fruit_ninja_viewport.c`:
```c
#include "fruit_ninja_viewport.h"

#define FN_LOGIC_W 640.0f
#define FN_LOGIC_H 480.0f

static float g_scale = 1.0f;
static float g_off_x = 0.0f;
static float g_off_y = 0.0f;

void fruit_ninja_viewport_init(int phys_w, int phys_h) {
    float sx = (float)phys_w / FN_LOGIC_W;
    float sy = (float)phys_h / FN_LOGIC_H;
    g_scale = (sx < sy) ? sx : sy;
    g_off_x = ((float)phys_w - FN_LOGIC_W * g_scale) * 0.5f;
    g_off_y = ((float)phys_h - FN_LOGIC_H * g_scale) * 0.5f;
}

float fruit_ninja_viewport_x(float logic_x)   { return g_off_x + logic_x * g_scale; }
float fruit_ninja_viewport_y(float logic_y)   { return g_off_y + logic_y * g_scale; }
float fruit_ninja_viewport_len(float l)       { return l * g_scale; }
float fruit_ninja_viewport_scale(void)        { return g_scale; }
float fruit_ninja_viewport_to_logic_x(float px) { return (px - g_off_x) / g_scale; }
float fruit_ninja_viewport_to_logic_y(float py) { return (py - g_off_y) / g_scale; }
```

- [x] **Step 6:跑测试确认通过**

Run: `cmake --build build -j && ctest --test-dir build -R fruit_ninja_viewport_test -V`
Expected: `1 test passed`。

- [x] **Step 7:提交**

```bash
git add main/src/v9-fruit_ninja/fruit_ninja_viewport.h main/src/v9-fruit_ninja/fruit_ninja_viewport.c main/tests/fruit_ninja_viewport_test.c CMakeLists.txt
git commit -m "feat(fruit-ninja): 新增 viewport letterbox 映射模块 + 三档单测"
```

---

# Phase 2:模块拆分(行为不变重构)

把 `scene.c` 按职责拆开。**目标:每步编译通过、运行行为与基线一致**(无单测保护,靠编译 + 运行回归)。先建共享接口头,再逐模块迁移。每个 Task 迁移一组强相关函数。

> 通用验证(每个 Task 的最后一步都做):
> Run: `cmake -S . -B build && cmake --build build -j && ./bin/main 800 480`
> Expected: 编译无误;游戏行为与 Phase 0 基线一致(首页、抛水果、切水果、炸弹爆炸均正常)。

### Task 3:抽取共享 helper 到 internal.h + 常量集中 ✅

把 `scene.c` 顶部宏常量与跨模块要共享的 static helper 暴露出来,供后续各模块 include。

**Files:**
- Create: `main/src/v9-fruit_ninja/fruit_ninja_internal.h`
- Modify: `main/src/v9-fruit_ninja/fruit_ninja_scene.c`(去掉将共享的 helper 的 `static`,改为非 static;宏移到 internal.h)

- [x] **Step 1:建 internal.h**,集中常量(从 scene.c:17-27 迁入)并声明共享 helper:
```c
#ifndef FRUIT_NINJA_INTERNAL_H
#define FRUIT_NINJA_INTERNAL_H

#include "lvgl/lvgl.h"
#include "fruit_ninja_model.h"

/* —— 时间/物理常量(原 scene.c 顶部)—— */
#define FRUIT_NINJA_UPDATE_MS       16U
#define FRUIT_NINJA_EXPLODING_MS    4000U
#define FRUIT_NINJA_FLASH_MS        200U
#define FRUIT_NINJA_DROP_TIME_MS    1200U
#define FRUIT_NINJA_JS_START_Y      600.0f
#define FRUIT_NINJA_SCORE_PULSE_MS  90U
#define FRUIT_NINJA_HOME_FLOAT_AMPLITUDE 8.0f
#define FRUIT_NINJA_HOME_SLICE_FEEDBACK_MS 240U

/* —— 共享 helper(实现留在 scene.c,去掉 static)—— */
lv_obj_t * fruit_ninja_create_layer(lv_obj_t * parent);
bool       fruit_ninja_make_image_path(char * out, size_t out_size, const char * relative_path);
lv_obj_t * fruit_ninja_create_file_image(lv_obj_t * parent, const char * relative_path);
void       fruit_ninja_set_image_geometry(lv_obj_t * obj, int32_t x, int32_t y, int32_t w, int32_t h);
void       fruit_ninja_hide_obj(lv_obj_t * obj);
void       fruit_ninja_show_obj(lv_obj_t * obj);
void       fruit_ninja_destroy_if_present(lv_obj_t ** obj);

#endif
```

- [x] **Step 2:** 在 `scene.c` 中,把上述 helper 的定义去掉 `static`、改名加 `fruit_ninja_` 前缀(`create_layer`→`fruit_ninja_create_layer` 等),删除 scene.c 里重复的宏定义改为 `#include "fruit_ninja_internal.h"`。全局把这些 helper 的调用处改成新名(scene.c 内)。

- [x] **Step 3:** 通用验证(编译 + 运行回归)。

- [x] **Step 4:提交**
```bash
git add main/src/v9-fruit_ninja/fruit_ninja_internal.h main/src/v9-fruit_ninja/fruit_ninja_scene.c
git commit -m "refactor(fruit-ninja): 抽取共享 helper/常量到 internal.h"
```

### Task 4:抽取 physics 模块 ✅

迁移水果与碎片的物理/生成逻辑。

**Files:**
- Create: `main/src/v9-fruit_ninja/fruit_ninja_physics.c`
- Create: `main/src/v9-fruit_ninja/fruit_ninja_physics.h`
- Modify: `main/src/v9-fruit_ninja/fruit_ninja_scene.c`(删除被迁函数,改为 include physics.h)

**迁移清单**(从 `scene.c` 移到 `physics.c`,函数体不变,仅:`static`→对外的加前缀、内部 helper 保持 static、`ease_*` 改调用 `fruit_ninja_easing.h`):
- `frand_range`(76-79)→ physics.c 内部 static
- `choose_fruit_def`(149-156)、`g_fruit_defs[]` 定义(51-58)
- `alloc_fruit`(512-524)、`spawn_one_fruit`(525-571)
- `update_single_fruit_visual`(499-511)
- `update_fruits`(834-901)
- `alloc_fragment`(343-355)、`spawn_fragment`(356-387)、`update_fragments`(903-935)
- `active_running_fruits`(158-172)、`target_fruit_count`(173-177)

**physics.h 契约:**
```c
#ifndef FRUIT_NINJA_PHYSICS_H
#define FRUIT_NINJA_PHYSICS_H
#include "fruit_ninja_model.h"

void     fruit_ninja_physics_spawn_one_fruit(fruit_ninja_game_t * game);
void     fruit_ninja_physics_update_fruits(fruit_ninja_game_t * game);
void     fruit_ninja_physics_update_fragments(fruit_ninja_game_t * game);
uint32_t fruit_ninja_physics_active_fruits(const fruit_ninja_game_t * game);
uint32_t fruit_ninja_physics_target_count(const fruit_ninja_game_t * game);
/* 供 input/state 切水果时生成碎片 */
fruit_ninja_fragment_t * fruit_ninja_physics_spawn_fragment(
    fruit_ninja_game_t * game, const char * rel_path,
    float x, float y, float vx, float vy, float angular_velocity, float angle);
const fruit_ninja_fruit_def_t * fruit_ninja_physics_choose_def(void);
fruit_ninja_fruit_t * fruit_ninja_physics_alloc_fruit(fruit_ninja_game_t * game);
#endif
```
(`ease_in_quad`/`ease_out_quad` 调用改为 `fruit_ninja_ease_in_quad`/`_out_quad`,删除 scene.c 里的 `static inline ease_*`。)

- [x] **Step 1:** 创建 physics.c/.h,迁移上述函数;scene.c 删除被迁函数与 `ease_*`,在调用处改用 `fruit_ninja_physics_*`。
- [x] **Step 2:** 通用验证(编译 + 运行回归)。
- [x] **Step 3:提交** `git commit -m "refactor(fruit-ninja): 抽取 physics 模块(水果/碎片/生成)"`

### Task 5:抽取 state 模块(状态机 / HUD / 首页) ✅

**Files:**
- Create: `main/src/v9-fruit_ninja/fruit_ninja_state.c` + `.h`
- Modify: `fruit_ninja_scene.c`

**迁移清单:** `enter_home`(389-419)、`enter_running`(420-448)、`enter_game_over`(449-462)、`update_score_label`(178-182)、`update_miss_icons`(183-194)、`build_home_menu_fruits`(573-606)、`update_home_animation`(623-675)、`start_running_timer_cb`(607-613)、`restore_home_fruit_timer_cb`(614-622)、`clear_fragments`(217-227)、`clear_home_menu_fruits`(228-239)、`clear_fruits`(240-252)、`stage_home_object`(274-282)。

> 注:`enter_exploding`(463-497)留待 Phase 5 改造,本任务先迁到 state.c(行为不变),声明进 state.h。

**state.h 契约**(只列对外需要的):
```c
void fruit_ninja_state_enter_home(fruit_ninja_game_t * game);
void fruit_ninja_state_enter_running(fruit_ninja_game_t * game);
void fruit_ninja_state_enter_game_over(fruit_ninja_game_t * game);
void fruit_ninja_state_enter_exploding(fruit_ninja_game_t * game, float x, float y);
void fruit_ninja_state_update_score_label(fruit_ninja_game_t * game);
void fruit_ninja_state_update_miss_icons(fruit_ninja_game_t * game);
void fruit_ninja_state_update_home_animation(fruit_ninja_game_t * game);
```

- [x] **Step 1:** 创建 state.c/.h,迁移;scene.c 调用处改名。
- [x] **Step 2:** 通用验证。
- [x] **Step 3:提交** `git commit -m "refactor(fruit-ninja): 抽取 state 模块(状态机/HUD/首页动画)"`

### Task 6:抽取 effects 模块骨架 + 切割派发入 input ✅

把现有特效相关函数迁到 effects.c(本任务仍是旧实现,Phase 4 再换成 canvas),切割派发迁到 input.c。

**Files:**
- Create: `main/src/v9-fruit_ninja/fruit_ninja_effects.c` + `.h`
- Modify: `fruit_ninja_input.c`、`fruit_ninja_scene.c`

**迁到 effects.c:** `spawn_flash`(253-266)、`clear_flash_if_needed`(283-312)、`update_score_pulse`(314-342)、`clear_explosion_overlays`(268-273)。
**迁到 input.c:** `slice_fruit`(676-733)、`handle_home_menu_hits`(735-770)、`handle_segment_hits`(771-788)、`input_event_cb`(790-832)。(`slice_fruit` 里调用的 `spawn_fragment`→`fruit_ninja_physics_spawn_fragment`,`spawn_flash`→`fruit_ninja_effects_spawn_flash`,`enter_exploding`→`fruit_ninja_state_enter_exploding`,`update_score_label`→`fruit_ninja_state_update_score_label`。)

**effects.h 契约(本阶段):**
```c
void fruit_ninja_effects_spawn_flash(fruit_ninja_game_t * game, float x, float y);
void fruit_ninja_effects_update_flash(fruit_ninja_game_t * game);   /* 原 clear_flash_if_needed */
void fruit_ninja_effects_update_score_pulse(fruit_ninja_game_t * game);
void fruit_ninja_effects_clear_explosion(fruit_ninja_game_t * game);
```
**input.c 追加对外:** `void fruit_ninja_input_attach(fruit_ninja_game_t * game);`(在 input_layer 上注册 `input_event_cb`,原注册逻辑从 scene.c 迁来)。

- [x] **Step 1:** 迁移;scene.c 的 `create_static_scene` 里事件注册改调用 `fruit_ninja_input_attach`;`update_timer_cb` 里调用改名(`clear_flash_if_needed`→`fruit_ninja_effects_update_flash` 等)。
- [x] **Step 2:** 通用验证(尤其确认切水果、炸弹、加分、刀光、闪光均与基线一致)。
- [x] **Step 3:提交** `git commit -m "refactor(fruit-ninja): 抽取 effects 模块 + 切割派发入 input"`

### Task 7:scene.c 收尾瘦身 ✅

**Files:** Modify `fruit_ninja_scene.c`

此时 scene.c 应只剩:`fruit_ninja_start`、`create_static_scene`、`update_timer_cb`、共享 helper 定义,以及 game 单例。`update_timer_cb` 调用顺序改为调用各模块对外函数(physics/state/effects/input)。

- [x] **Step 1:** 清理 scene.c 残留 include / 未用声明,确认按模块对外接口编排主循环。
- [x] **Step 2:** 通用验证 + 跑全部单测 `ctest --test-dir build --output-on-failure`(应全过)。
- [x] **Step 3:提交** `git commit -m "refactor(fruit-ninja): scene.c 瘦身为入口+场景创建+主循环编排"`

---

# Phase 3:接入 viewport(三分辨率 letterbox)

让游戏逻辑固定 640×480、显示经 viewport 映射到物理屏。

### Task 8:逻辑尺寸固定 + 位图/输入/背景走 viewport ✅

**Files:**
- Modify: `fruit_ninja_scene.c`(`fruit_ninja_start` / `create_static_scene`)
- Modify: `fruit_ninja_state.c`、`fruit_ninja_physics.c`、`fruit_ninja_input.c`(凡用到坐标放置 lv_image、读 indev 物理坐标处)
- Modify: `fruit_ninja_model.h`(若 `screen_width/height` 语义改为逻辑尺寸,加注释)

**做法要点:**
1. `fruit_ninja_start`:取物理屏尺寸 `lv_display_get_horizontal/vertical_resolution(lv_display_get_default())`,调 `fruit_ninja_viewport_init(pw, ph)`;`game->screen_width=640; game->screen_height=480`(逻辑尺寸恒定)。
2. 背景:铺满物理屏 —— `lv_obj_set_size(background, pw, ph); lv_obj_set_pos(background,0,0)`(背景图可非等比拉伸填满)。
3. 所有水果/碎片/UI 的 `lv_obj_set_pos(img, X, Y)`:X/Y 由逻辑坐标经 `fruit_ninja_viewport_x/y` 映射;图片缩放 `lv_image_set_scale(img, (uint16_t)(fruit_ninja_viewport_scale()*256))`。
4. 输入命中:`input_event_cb` 读到 `point.x/point.y`(物理)后,先 `fruit_ninja_viewport_to_logic_x/y` 转逻辑,再传给 trail/碰撞(碰撞全程逻辑坐标)。
5. 闪光/烟雾等定位同样走映射。

> 提示:封装一个内部 helper `place_logic(lv_obj_t*, float lx, float ly, bool centered)` 统一"映射+缩放+定位",减少重复(DRY)。可放 internal.h / scene.c。

- [x] **Step 1:** 实现上述 1–5。
- [x] **Step 2:截图验证三档 letterbox**
```bash
AM_SHOT=/tmp/fn_800.png ./bin/main 800 480
AM_SHOT=/tmp/fn_640.png ./bin/main 640 480
AM_SHOT=/tmp/fn_480.png ./bin/main 480 272
```
Expected:三张首页截图中,游戏内容等比、水平居中;800×480 左右有留边(背景填充);480×272 整体缩小居中、无超界/裁切。用 Read 工具查看三张 PNG 确认。
- [x] **Step 3:** 运行 `./bin/main 480 272` 实玩,确认划水果命中准确(输入映射正确)。
- [x] **Step 4:提交** `git commit -m "feat(fruit-ninja): 接入 viewport,三分辨率 letterbox 自适应"`

---

# Phase 4:Canvas 特效层(核心手感)

把 `effect_layer` 换成全屏 `lv_canvas`,每帧重绘矢量特效。先建骨架与刀光拖尾,再加汁液、对齐闪光。

### Task 9:Canvas 特效层骨架 + 刀光拖尾 ✅

**Files:**
- Modify: `fruit_ninja_model.h`(加刀光段结构)
- Modify: `fruit_ninja_effects.c/.h`(canvas 创建 + 每帧 render + 刀光)
- Modify: `fruit_ninja_input.c`(轨迹改为投喂刀光段,不再用 lv_line)
- Modify: `fruit_ninja_scene.c`(`create_static_scene` 建 canvas;`update_timer_cb` 调 render)
- Create: `main/tests/fruit_ninja_blade_test.c` + CMakeLists(测试刀光宽度衰减纯函数)

**model.h 追加:**
```c
#define FRUIT_NINJA_MAX_BLADE_SEGMENTS 48
typedef struct {
    bool     active;
    float    sx, sy, ex, ey;   /* 逻辑坐标 */
    uint32_t age_ms;
} fruit_ninja_blade_seg_t;
```
在 `fruit_ninja_game_t` 增:`lv_obj_t * effect_canvas; fruit_ninja_blade_seg_t blades[FRUIT_NINJA_MAX_BLADE_SEGMENTS];`

**effects.h 追加:**
```c
void  fruit_ninja_effects_init_canvas(fruit_ninja_game_t * game, int phys_w, int phys_h);
void  fruit_ninja_effects_push_blade(fruit_ninja_game_t * game, float sx, float sy, float ex, float ey);
void  fruit_ninja_effects_render(fruit_ninja_game_t * game, uint32_t delta_ms);
float fruit_ninja_blade_width(uint32_t age_ms); /* 纯函数:10*(1-age/200),钳到0 */
```

- [x] **Step 1:写刀光宽度纯函数测试** `main/tests/fruit_ninja_blade_test.c`:
```c
#include <assert.h>
#include <math.h>
#include "../src/v9-fruit_ninja/fruit_ninja_effects.h"
#define APPROX(a,b) (fabsf((a)-(b)) < 1e-3f)
int main(void) {
    assert(APPROX(fruit_ninja_blade_width(0), 10.0f));
    assert(APPROX(fruit_ninja_blade_width(100), 5.0f));
    assert(APPROX(fruit_ninja_blade_width(200), 0.0f));
    assert(APPROX(fruit_ninja_blade_width(300), 0.0f)); /* 钳位 */
    return 0;
}
```
加 CMake(注意 effects.c 依赖 lvgl,需链接;但 `fruit_ninja_blade_width` 是纯函数——为可测,把它实现在 effects.c 但不触 lvgl 符号即可,测试链接 lvgl 以满足其它符号):
```cmake
add_executable(fruit_ninja_blade_test
    ${PROJECT_SOURCE_DIR}/main/tests/fruit_ninja_blade_test.c
    ${PROJECT_SOURCE_DIR}/main/src/v9-fruit_ninja/fruit_ninja_effects.c
    ${PROJECT_SOURCE_DIR}/main/src/v9-fruit_ninja/fruit_ninja_easing.c
    ${PROJECT_SOURCE_DIR}/main/src/v9-fruit_ninja/fruit_ninja_viewport.c)
target_include_directories(fruit_ninja_blade_test PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(fruit_ninja_blade_test PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(fruit_ninja_blade_test lvgl m)
add_test(NAME fruit_ninja_blade_test COMMAND $<TARGET_FILE:fruit_ninja_blade_test>)
set_tests_properties(fruit_ninja_blade_test PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
```
Run: `cmake -S . -B build && cmake --build build -j 2>&1 | tail` → Expected: 链接失败(`fruit_ninja_blade_width` 未定义)。

- [x] **Step 2:实现 canvas 与刀光**(effects.c 关键代码):
```c
/* 文件顶部:静态 canvas buffer,按最大物理屏 800x480 ARGB8888 = 1.5MB */
static uint8_t g_canvas_buf[800 * 480 * 4];

float fruit_ninja_blade_width(uint32_t age_ms) {
    if(age_ms >= 200U) return 0.0f;
    return 10.0f * (1.0f - (float)age_ms / 200.0f);
}

void fruit_ninja_effects_init_canvas(fruit_ninja_game_t * game, int phys_w, int phys_h) {
    game->effect_canvas = lv_canvas_create(game->effect_layer);
    lv_canvas_set_buffer(game->effect_canvas, g_canvas_buf, phys_w, phys_h,
                         LV_COLOR_FORMAT_ARGB8888);
    lv_obj_set_pos(game->effect_canvas, 0, 0);
    lv_canvas_fill_bg(game->effect_canvas, lv_color_black(), LV_OPA_TRANSP);
}

void fruit_ninja_effects_push_blade(fruit_ninja_game_t * game,
                                    float sx, float sy, float ex, float ey) {
    for(int i = 0; i < FRUIT_NINJA_MAX_BLADE_SEGMENTS; ++i) {
        if(!game->blades[i].active) {
            game->blades[i] = (fruit_ninja_blade_seg_t){true, sx, sy, ex, ey, 0};
            return;
        }
    }
    /* 满了:覆盖最老的 */
    int oldest = 0; uint32_t max_age = 0;
    for(int i = 0; i < FRUIT_NINJA_MAX_BLADE_SEGMENTS; ++i)
        if(game->blades[i].age_ms >= max_age) { max_age = game->blades[i].age_ms; oldest = i; }
    game->blades[oldest] = (fruit_ninja_blade_seg_t){true, sx, sy, ex, ey, 0};
}

void fruit_ninja_effects_render(fruit_ninja_game_t * game, uint32_t delta_ms) {
    lv_canvas_fill_bg(game->effect_canvas, lv_color_black(), LV_OPA_TRANSP);
    lv_layer_t layer;
    lv_canvas_init_layer(game->effect_canvas, &layer);

    /* 刀光:逐段按年龄衰减线宽 */
    lv_draw_line_dsc_t ld;
    lv_draw_line_dsc_init(&ld);
    ld.color = lv_color_hex(0xcbd3db);
    ld.opa = LV_OPA_90;
    ld.round_start = 1; ld.round_end = 1;
    for(int i = 0; i < FRUIT_NINJA_MAX_BLADE_SEGMENTS; ++i) {
        fruit_ninja_blade_seg_t * b = &game->blades[i];
        if(!b->active) continue;
        b->age_ms += delta_ms;
        float w = fruit_ninja_blade_width(b->age_ms);
        if(w <= 0.1f) { b->active = false; continue; }
        ld.width = (int32_t)fruit_ninja_viewport_len(w);
        if(ld.width < 1) ld.width = 1;
        ld.p1.x = fruit_ninja_viewport_x(b->sx); ld.p1.y = fruit_ninja_viewport_y(b->sy);
        ld.p2.x = fruit_ninja_viewport_x(b->ex); ld.p2.y = fruit_ninja_viewport_y(b->ey);
        lv_draw_line(&layer, &ld);
    }
    /* Phase 4 Task10 在此追加汁液绘制;Phase 5 追加光线/火焰 */

    lv_canvas_finish_layer(game->effect_canvas, &layer);
}
```
- [x] **Step 3:input.c 改为投喂刀光段**:在 `handle_segment_hits` / `handle_home_menu_hits` 拿到 `segment`(已是逻辑坐标)后,若 `segment.valid` 调 `fruit_ninja_effects_push_blade(game, segment.x1,segment.y1,segment.x2,segment.y2)`。删除 `fruit_ninja_input_init` 里 lv_line 的创建与 `fruit_ninja_input_tick` 的 lv_line 渐隐(改由 effects render 管理);`trail.line` 字段可保留不用或移除。
- [x] **Step 4:scene.c 接线**:`create_static_scene` 末尾调 `fruit_ninja_effects_init_canvas(game, pw, ph)`;`update_timer_cb` 把原 `fruit_ninja_input_tick` 之外,加 `fruit_ninja_effects_render(game, FRUIT_NINJA_UPDATE_MS)`(放在所有逻辑更新之后,作为绘制收尾)。
- [x] **Step 5:跑测试 + 运行**
Run: `cmake --build build -j && ctest --test-dir build -R fruit_ninja_blade_test -V` → Expected: pass。
Run: `./bin/main 800 480` → Expected: 划动时出现**逐段衰减的拖尾刀光**(近段粗、远段细、200ms 内消失),明显比基线的单线渐隐更像 JS。
- [x] **Step 6:提交** `git commit -m "feat(fruit-ninja): canvas 特效层 + 刀光逐段衰减拖尾"`

### Task 10:汁液飞溅 ✅

**Files:** Modify `fruit_ninja_model.h`、`fruit_ninja_effects.c/.h`、`fruit_ninja_input.c`(slice 时触发)

**model.h 追加:**
```c
#define FRUIT_NINJA_MAX_JUICE 80
typedef struct {
    bool     active;
    float    origin_x, origin_y;  /* 逻辑 */
    float    angle_rad, distance; /* 径向方向与最大距离 */
    uint32_t age_ms, life_ms;
    uint8_t  cr, cg, cb;          /* 果色 */
} fruit_ninja_juice_t;
```
game_t 增:`fruit_ninja_juice_t juice[FRUIT_NINJA_MAX_JUICE];`
fruit_def 增字段(model.h 的 `fruit_ninja_fruit_def_t`):`bool has_juice; uint8_t juice_r, juice_g, juice_b;`,并在 physics.c 的 `g_fruit_defs[]` 填:peach(0xe6,0xc7,0x31)、sandia(0xcc,0,0)、apple(0xc8,0xe9,0x25)、banana(has_juice=false)、basaha(0xcc,0,0)、boom(false)。

**effects.h 追加:**
```c
void fruit_ninja_effects_spawn_juice(fruit_ninja_game_t * game, float x, float y,
                                     uint8_t r, uint8_t g, uint8_t b);
```

- [x] **Step 1:实现 spawn + 更新 + 绘制**(effects.c):
```c
void fruit_ninja_effects_spawn_juice(fruit_ninja_game_t * game, float x, float y,
                                     uint8_t r, uint8_t g, uint8_t b) {
    int spawned = 0;
    for(int i = 0; i < FRUIT_NINJA_MAX_JUICE && spawned < 10; ++i) {
        if(game->juice[i].active) continue;
        fruit_ninja_juice_t * j = &game->juice[i];
        j->active = true;
        j->origin_x = x; j->origin_y = y;
        /* 随机角度 0..2pi,距离 100..300 */
        j->angle_rad = ((float)(rand() % 360)) * 0.01745329f;
        j->distance = 100.0f + (float)(rand() % 200);
        j->age_ms = 0; j->life_ms = 1500;
        j->cr = r; j->cg = g; j->cb = b;
        ++spawned;
    }
}
```
在 `fruit_ninja_effects_render` 的刀光绘制之后、`finish_layer` 之前,追加汁液绘制:
```c
    lv_draw_arc_dsc_t ad;
    lv_draw_arc_dsc_init(&ad);
    ad.start_angle = 0; ad.end_angle = 3600; ad.rounded = 1;
    for(int i = 0; i < FRUIT_NINJA_MAX_JUICE; ++i) {
        fruit_ninja_juice_t * j = &game->juice[i];
        if(!j->active) continue;
        j->age_ms += delta_ms;
        if(j->age_ms >= j->life_ms) { j->active = false; continue; }
        float p = (float)j->age_ms / (float)j->life_ms;          /* 0..1 */
        float dist = fruit_ninja_ease_out_expo(p) * j->distance; /* 径向展开 */
        float lx = j->origin_x + cosf(j->angle_rad) * dist;
        float ly = j->origin_y + sinf(j->angle_rad) * dist
                   + fruit_ninja_ease_out_quad(p) * 200.0f;      /* 重力下坠 */
        float radius = fruit_ninja_viewport_len(10.0f * (1.0f - p)); /* 缩小至 0 */
        if(radius < 1.0f) continue;
        ad.center.x = (int32_t)fruit_ninja_viewport_x(lx);
        ad.center.y = (int32_t)fruit_ninja_viewport_y(ly);
        ad.radius = (uint16_t)radius;
        ad.width = (int32_t)radius;       /* width=radius -> 实心圆 */
        ad.color = lv_color_make(j->cr, j->cg, j->cb);
        ad.opa = (lv_opa_t)(LV_OPA_COVER * (1.0f - p)); /* 同步淡出 */
        lv_draw_arc(&layer, &ad);
    }
```
- [x] **Step 2:slice 时触发**:在 `input.c` 的 `slice_fruit` 中,非炸弹切开后(现状 722 行 spawn_flash 附近)加:`if(fruit->def->has_juice) fruit_ninja_effects_spawn_juice(game, fruit->x, fruit->y, fruit->def->juice_r, fruit->def->juice_g, fruit->def->juice_b);`
- [x] **Step 3:运行验证**
Run: `./bin/main 800 480` → Expected: 切水果时迸出约 10 个果色圆点,向外径向飞散并下坠、1.5s 内缩小淡出;香蕉无果汁;颜色与水果匹配(苹果绿、西瓜红、桃黄)。
- [x] **Step 4:提交** `git commit -m "feat(fruit-ninja): 汁液飞溅粒子(果色/径向/重力/淡出)"`

### Task 11:切割闪光对齐 JS ✅

JS 的 flash 是 `scale 1e-5→1→1e-5` over ~100ms。现状是 32→256→48 over 200ms。对齐为:放大相(0–100ms)scale 0→256,缩小相(100–200ms)256→0,透明度同步。

**Files:** Modify `fruit_ninja_effects.c`(`fruit_ninja_effects_update_flash`,原 clear_flash_if_needed)

- [x] **Step 1:** 把缩放曲线改为:`age<100`→`scale = age*256/100`;`age<200`→`scale = 256 - (age-100)*256/100`;`opa = (200-age)*255/200`;`age>=200` 删除。闪光定位走 viewport(中心对齐切点)。
- [x] **Step 2:运行验证** `./bin/main 800 480` → Expected: 切水果瞬间闪光从无到大再到无,~200ms,位置贴合切点。
- [x] **Step 3:提交** `git commit -m "feat(fruit-ninja): 切割闪光缩放曲线对齐 JS"`

---

# Phase 5:炸弹三件套

把 `enter_exploding`(现位于 state.c)升级为 JS 的:10 道放射光线 + 背景抖动 4s + 持续火焰 + 白屏渐隐 4000ms。光线/火焰画到 canvas。

### Task 12:10 道放射爆炸光线 ✅

**Files:** Modify `fruit_ninja_model.h`、`fruit_ninja_effects.c/.h`、`fruit_ninja_state.c`(enter_exploding 触发)

**model.h 追加:**
```c
typedef struct {
    bool     active;
    float    cx, cy;        /* 爆心,逻辑 */
    uint32_t age_ms;        /* 整个爆炸已进行时间 */
} fruit_ninja_blast_t;
```
game_t 增:`fruit_ninja_blast_t blast;`

**effects.h 追加:**
```c
void fruit_ninja_effects_start_blast(fruit_ninja_game_t * game, float x, float y);
```

- [x] **Step 1:实现**:`start_blast` 置 `blast={true,x,y,0}`。在 `fruit_ninja_effects_render` 中追加光线绘制:
```c
    if(game->blast.active) {
        game->blast.age_ms += delta_ms;
        int rays_shown = (int)(game->blast.age_ms / 100U) + 1; /* 每100ms多一道 */
        if(rays_shown > 10) rays_shown = 10;
        lv_draw_triangle_dsc_t td;
        lv_draw_triangle_dsc_init(&td);
        td.bg_color = lv_color_hex(0xffffff);
        td.bg_opa = LV_OPA_COVER;
        float cx = fruit_ninja_viewport_x(game->blast.cx);
        float cy = fruit_ninja_viewport_y(game->blast.cy);
        float ray_len = fruit_ninja_viewport_len(400.0f);
        float half_w = fruit_ninja_viewport_len(14.0f);
        for(int r = 0; r < rays_shown; ++r) {
            float a = (float)r * (6.2831853f / 10.0f);
            float dx = cosf(a), dy = sinf(a);
            float px = -dy, py = dx; /* 垂直方向 */
            td.p[0].x = cx + dx * ray_len; td.p[0].y = cy + dy * ray_len; /* 尖端 */
            td.p[1].x = cx + px * half_w;  td.p[1].y = cy + py * half_w;
            td.p[2].x = cx - px * half_w;  td.p[2].y = cy - py * half_w;
            lv_draw_triangle(&layer, &td);
        }
        if(game->blast.age_ms >= FRUIT_NINJA_EXPLODING_MS) game->blast.active = false;
    }
```
- [x] **Step 2:触发**:在 `fruit_ninja_state_enter_exploding` 里调 `fruit_ninja_effects_start_blast(game, x, y)`。
- [x] **Step 3:运行验证**:`./bin/main 800 480`,切炸弹 → Expected: 爆心射出逐道增加(共10道)的白色放射光线。
- [x] **Step 4:提交** `git commit -m "feat(fruit-ninja): 炸弹 10 道放射爆炸光线"`

### Task 13:背景抖动 + 白屏渐隐对齐 ✅

**Files:** Modify `fruit_ninja_state.c`(enter_exploding/退出)、`fruit_ninja_scene.c`(update_timer_cb EXPLODING 分支)

- [x] **Step 1:背景抖动**:EXPLODING 期间(state_elapsed_ms < 4000),每 50ms 把 `background` 位置设为 `(rand在±6 + 原点)`;退出 EXPLODING 时复位到 letterbox 原点。注意原点是物理坐标(背景铺满,基准 0,0)。
- [x] **Step 2:白屏渐隐**:现状已有 white_flash_overlay opa 从 80% 线性到 0 over 4000ms(scene.c 973-977);改起始为 `LV_OPA_COVER`(JS 是全白 1→0),保持 4000ms。确认覆盖整个物理屏(`LV_PCT(100)`)。
- [x] **Step 3:运行验证**:切炸弹 → Expected: 全屏白闪由强到无(4s),背景明显抖动约 4s,然后进入 game over。
- [x] **Step 4:提交** `git commit -m "feat(fruit-ninja): 炸弹背景抖动 + 全屏白闪渐隐对齐 JS"`

### Task 14:持续火焰 ✅

炸弹存在期间,其位置持续冒黄色火苗(JS:40ms 生成,单簇 life 200–700ms,90% 概率,色 #fafad9→#f0ef9c)。用 canvas 画小三角/圆簇近似。

**Files:** Modify `fruit_ninja_model.h`、`fruit_ninja_effects.c/.h`

**model.h 追加:**
```c
#define FRUIT_NINJA_MAX_FLAMES 32
typedef struct {
    bool active; float x, y;        /* 逻辑 */
    uint32_t age_ms, life_ms; float seed;
} fruit_ninja_flame_t;
```
game_t 增:`fruit_ninja_flame_t flames[FRUIT_NINJA_MAX_FLAMES]; uint32_t flame_accum_ms; bool bomb_alive; float bomb_x, bomb_y;`

- [x] **Step 1:** 炸弹生成时(physics spawn 到炸弹)记录 `bomb_alive=true; bomb_x/y`;炸弹被切或离场时 `bomb_alive=false`。在 render 中累加 `flame_accum_ms`,每 40ms 且 90% 概率在 `bomb_x/y` 附近生成一簇火苗(life=200+rand%500);逐簇按 age/life 上浮+淡出,用 `lv_draw_arc`(小圆,色在 0xfafad9→0xf0ef9c 间按进度插值)绘制。炸弹位置随物理更新,火苗跟随。
- [x] **Step 2:运行验证**:炸弹飞行时持续冒黄色火苗。
- [x] **Step 3:提交** `git commit -m "feat(fruit-ninja): 炸弹持续火焰效果"`

---

# Phase 6:物理/逻辑数值对齐 + 收尾

### Task 15:旋转/分裂散射对齐 JS ✅

**Files:** Modify `fruit_ninja_physics.c`

- [x] **Step 1:旋转速度**:`spawn_one_fruit` 里角速度从 `±(90+rand%180)` 改为 JS 池 `±{60,50,40}`:
```c
static const float SPINS[3] = {60.0f, 50.0f, 40.0f};
float sign = (rand() % 2 == 0) ? -1.0f : 1.0f;
fruit->angular_velocity = sign * SPINS[rand() % 3];
```
- [x] **Step 2:分裂下落缓动**:`update_fragments` 已用 `ease_in_quad` 于 Y(对齐 JS quadratic.ci),确认 X 为线性、角度线性;`slice_fruit` 的散射目标已是 `左 -(rand%200+75) / 右 rand%275`、角 `±(rand%150+50)`,与 spec 一致,保持。
- [x] **Step 3:验证**:运行观察水果自转更稳、切开两半散射自然。`ctest --test-dir build`(确认未破坏单测)。
- [x] **Step 4:提交** `git commit -m "feat(fruit-ninja): 旋转速度对齐 JS ±{60,50,40}"`

### Task 16:生命图标弹出 + 音效时机对齐 ✅

**Files:** Modify `fruit_ninja_state.c`(漏果/miss 显示)、`fruit_ninja_physics.c` 或 `input.c`(音效触发点)

- [x] **Step 1:生命弹出**:漏掉水果触发 miss 时,对应 `miss_icons` 用 `fruit_ninja_ease_out_back` 做 scale 1e-5→1(500ms)弹出(可用 `lv_image_set_scale` 配合每帧推进,或临时计时字段)。JS 还在漏点位置弹出 lose 图标(`lose.png`)停 1500ms 再缩回——如要完整复刻,加一个临时弹出 sprite(可选,记入验收清单)。
- [x] **Step 2:音效时机**:确认 `spawn_one_fruit` 抛出时播 `fruit_ninja_audio_play_throw()`(若现状缺则补);切水果 `play_slice`、炸弹 `play_boom`、进首页 `play_menu_music`、开始 `play_start`、game over `play_game_over`。
- [x] **Step 3:分数脉冲对齐**:JS 分数脉冲为 scale 1→1.2→1 over ~60ms。现状峰值 256→307→256(已是 1.2×,保持),仅时长偏大——把 `FRUIT_NINJA_SCORE_PULSE_MS`(internal.h)由 `90U` 改为 `60U`。
- [x] **Step 4:验证**:运行确认漏水果时生命图标回弹动画、各音效在正确时机响、加分时分数轻微脉冲(~60ms)。
- [x] **Step 5:提交** `git commit -m "feat(fruit-ninja): 生命弹出 + 音效时机 + 分数脉冲对齐 JS"`

### Task 17:三分辨率回归 + 对照验收 ⏳(自动化验证已过;实玩/JS对照待人工)

**Files:** 无(验证 + 可能的微调)

- [x] **Step 1:全单测** `cmake --build build -j && ctest --test-dir build --output-on-failure` → Expected: 全过。
- [ ] **Step 2:三档实玩**:`./bin/main 800 480`、`640 480`、`480 272` 各玩一局(切水果、连切、漏果扣命、切炸弹爆炸、game over、点击回首页),确认 letterbox 居中、命中准确、特效正常、无掉帧。
- [ ] **Step 3:对照 JS**:浏览器开 `third-party/FruitNinja/index.html`,逐项核对 spec 附录 A 清单(刀光拖尾、汁液、闪光、炸弹光线/抖动/火焰/白闪、抛物线节奏、旋转、难度递增、生命、计分、音效)。记录差异并微调。
- [x] **Step 4:截图存档** `AM_SHOT=/tmp/fn_final_800.png ./bin/main 800 480` 等三档,Read 确认。
- [ ] **Step 5:提交** `git commit -m "test(fruit-ninja): 三分辨率回归 + JS 对照验收通过"`

---

## Self-Review(执行者无需重跑,已在编写时核对)

- **Spec 覆盖:** 刀光拖尾(T9)、汁液(T10)、闪光对齐(T11)、炸弹光线/抖动+白闪/火焰(T12/13/14)、多分辨率 letterbox(T2/T8)、模块拆分(T3–T7)、物理数值/旋转/分裂(T15)、生命弹出/音效(T16)、验收(T17)——spec 各节均有对应任务。
- **类型一致:** `fruit_ninja_effects_render(game, delta_ms)`、`fruit_ninja_effects_push_blade`、`fruit_ninja_effects_spawn_juice`、`fruit_ninja_blade_width`、`fruit_ninja_viewport_*`、`fruit_ninja_ease_*` 在各任务间命名一致;新结构体 `fruit_ninja_blade_seg_t/juice_t/blast_t/flame_t` 定义与使用一致。
- **无占位:** 纯逻辑任务含完整测试与实现;特效任务含完整新函数代码;重构任务给精确迁移清单 + 接口契约 + 编译/运行回归(被移动代码已存在,不重复粘贴)。
- **命令准确:** 构建 `cmake -S . -B build && cmake --build build -j`;运行 `./bin/main W H`;测试 `ctest --test-dir build`。

## 风险与备注

- **Canvas 性能:** 每帧重绘最大 1.5MB canvas。若 480×272/嵌入式实测掉帧,退化方案:canvas 只覆盖 letterbox 游戏区(而非整屏),或对刀光/汁液做活跃数上限(已设 48/80)。在 T9/T17 关注帧率。
- **重构回归无单测保护:** Phase 2 各步必须运行实玩回归,确认行为与 Phase 0 基线一致后再提交。
- **炸弹概率:** 现状 `choose_fruit_def` 已是 `rand()%8==4`(1/8),与 JS 一致,无需改(spec 中"现状20%"为早期误记)。
- **火焰(T14):** JS 用贝塞尔火苗,本计划用小圆簇近似;若要更像,可在 T17 微调为多三角堆叠。属"尽量接近"项,记入验收时主观评估。
