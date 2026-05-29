# Fruit Ninja v9 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在当前 PC simulator 工程里实现一个启动即进入、资源全部走文件、图片支持 png/jpg、声音支持 ogg/mp3、鼠标拖拽可真实游玩的 LVGL v9 Fruit Ninja。

**Architecture:** 以 `main/src/v9-fruit_ninja` 为独立实现目录，拆分为资源、音频、模型/状态、视图、输入与场景驱动几个边界清晰的模块。`main/src/main.c` 保持 SDL/LVGL 启动流程不变，只替换 demo 入口；构建层打开图片解码和 SDL2_mixer 链接，使 `third-party/FruitNinja` 的文件资源可以直接被 LVGL 和音频层消费。

**Tech Stack:** C99, LVGL v9, SDL2, SDL2_mixer, CMake

---

## 文件结构

### 新建文件

- `main/src/v9-fruit_ninja/fruit_ninja.h`：对外暴露游戏入口。
- `main/src/v9-fruit_ninja/fruit_ninja_assets.h`
- `main/src/v9-fruit_ninja/fruit_ninja_assets.c`：资源根目录、路径拼接、资源存在性检查。
- `main/src/v9-fruit_ninja/fruit_ninja_audio.h`
- `main/src/v9-fruit_ninja/fruit_ninja_audio.c`：SDL2_mixer 初始化、背景音乐、音效播放与停止。
- `main/src/v9-fruit_ninja/fruit_ninja_model.h`：类型、状态、实体结构体、常量。
- `main/src/v9-fruit_ninja/fruit_ninja_scene.c`：场景创建、首页、开局、主循环、结算。
- `main/src/v9-fruit_ninja/fruit_ninja_input.c`：鼠标拖拽轨迹、刀痕和线段抽取。
- `main/src/v9-fruit_ninja/fruit_ninja_collision.h`
- `main/src/v9-fruit_ninja/fruit_ninja_collision.c`：线段与圆形碰撞判定。
- `main/tests/fruit_ninja_collision_test.c`：碰撞算法最小回归测试。
- `main/tests/fruit_ninja_assets_test.c`：资源路径与关键资源存在性测试。

### 修改文件

- `CMakeLists.txt`：加入 `main/src/v9-fruit_ninja/*.c`、测试目标、SDL2_mixer、PNG/JPEG 选项。
- `main/src/main.c`：把 `lv_demo_widgets()` 替换为 Fruit Ninja 入口。
- `lv_conf.h`：确认并打开文件系统 / PNG / JPEG 相关开关。

### 可能新增但仅在需要时创建

- `main/src/v9-fruit_ninja/fruit_ninja_scene.h`：如果 `main.c` 只需要前置声明，可不单独建此头文件。
- `main/tests/CMakeLists.txt`：如果根 CMake 做测试聚合更清晰，可单独新增。

### 资源读取约定

- 图片路径使用标准文件系统前缀，统一由 `fruit_ninja_assets.c` 生成，例如：`S:/absolute/or/workdir/path/to/file.png`。
- 音频路径传给 SDL2_mixer 原始文件路径，不带 LVGL 文件系统前缀。

---

### Task 1: 打通构建依赖与最小图片文件解码链

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `lv_conf.h`
- Test: `rtk cmake -S . -B build`

- [ ] **Step 1: 写出最小构建改动草案**

```cmake
option(LV_USE_LIBPNG "Use libpng to decode PNG" ON)
option(LV_USE_LIBJPEG_TURBO "Use libjpeg turbo to decode JPEG" ON)

file(GLOB FRUIT_NINJA_SOURCES
    "${PROJECT_SOURCE_DIR}/main/src/v9-fruit_ninja/*.c"
)

find_package(SDL2_mixer REQUIRED)
```

- [ ] **Step 2: 先让配置失败在“缺文件”而不是“缺库”之前暴露**

Run: `rtk cmake -S . -B build`
Expected: 当前会因为 `main/src/v9-fruit_ninja/*.c` 尚未创建或 `SDL2_mixer` 尚未接入而失败，证明构建链正在覆盖新模块。

- [ ] **Step 3: 在 `lv_conf.h` 明确启用文件和解码能力**

```c
#define LV_USE_FS_STDIO 1
#define LV_USE_PNG 0
#define LV_USE_LODEPNG 0
#define LV_USE_LIBPNG 1
#define LV_USE_TJPGD 0
#define LV_USE_LIBJPEG_TURBO 1
```

- [ ] **Step 4: 完成 `CMakeLists.txt` 的最小实现**

```cmake
target_sources(main PRIVATE ${FRUIT_NINJA_SOURCES})
target_link_libraries(main ${SDL2_MIXER_LIBRARIES})
target_include_directories(main PRIVATE ${SDL2_MIXER_INCLUDE_DIRS})
```

- [ ] **Step 5: 再次运行配置确认进入下一类错误**

Run: `rtk cmake -S . -B build`
Expected: 不再因为 LVGL 图片解码选项未打开而卡住；若还失败，错误应聚焦到尚未实现的源码符号或测试目标。

- [ ] **Step 6: Commit**

```bash
rtk git add CMakeLists.txt lv_conf.h
rtk git commit -m "build: enable fruit ninja asset deps"
```

### Task 2: 先用 TDD 建立资源路径与资源存在性保护

**Files:**
- Create: `main/src/v9-fruit_ninja/fruit_ninja_assets.h`
- Create: `main/src/v9-fruit_ninja/fruit_ninja_assets.c`
- Create: `main/tests/fruit_ninja_assets_test.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 先写失败测试，锁定路径规则与关键资源检查**

```c
#include <assert.h>
#include <string.h>
#include "../src/v9-fruit_ninja/fruit_ninja_assets.h"

int main(void)
{
    char image_path[512];
    char audio_path[512];

    fruit_ninja_assets_init("/tmp/project-root");

    fruit_ninja_assets_image_path(image_path, sizeof(image_path), "images/background.jpg");
    fruit_ninja_assets_audio_path(audio_path, sizeof(audio_path), "sound/menu.ogg");

    assert(strncmp(image_path, "S:/tmp/project-root/third-party/FruitNinja/images/background.jpg", 56) == 0);
    assert(strcmp(audio_path, "/tmp/project-root/third-party/FruitNinja/sound/menu.ogg") == 0);
    return 0;
}
```

- [ ] **Step 2: 运行测试并确认正确失败**

Run: `rtk cc -I. main/tests/fruit_ninja_assets_test.c main/src/v9-fruit_ninja/fruit_ninja_assets.c -o /tmp/fruit_ninja_assets_test`
Expected: FAIL，提示头文件或实现文件不存在。

- [ ] **Step 3: 写最小头文件与实现让测试可编译**

```c
#ifndef FRUIT_NINJA_ASSETS_H
#define FRUIT_NINJA_ASSETS_H

#include <stddef.h>
#include <stdbool.h>

void fruit_ninja_assets_init(const char * project_root);
void fruit_ninja_assets_image_path(char * out, size_t out_size, const char * relative_path);
void fruit_ninja_assets_audio_path(char * out, size_t out_size, const char * relative_path);
bool fruit_ninja_assets_validate_core_files(void);

#endif
```

```c
static char g_project_root[256];

void fruit_ninja_assets_init(const char * project_root)
{
    snprintf(g_project_root, sizeof(g_project_root), "%s", project_root);
}

void fruit_ninja_assets_image_path(char * out, size_t out_size, const char * relative_path)
{
    snprintf(out, out_size, "S:%s/third-party/FruitNinja/%s", g_project_root, relative_path);
}

void fruit_ninja_assets_audio_path(char * out, size_t out_size, const char * relative_path)
{
    snprintf(out, out_size, "%s/third-party/FruitNinja/%s", g_project_root, relative_path);
}
```

- [ ] **Step 4: 再次运行测试确认转绿**

Run: `rtk cc -I. main/tests/fruit_ninja_assets_test.c main/src/v9-fruit_ninja/fruit_ninja_assets.c -o /tmp/fruit_ninja_assets_test && rtk /tmp/fruit_ninja_assets_test`
Expected: PASS，无输出直接退出 0。

- [ ] **Step 5: 扩展到真实仓库资源存在性校验**

```c
bool fruit_ninja_assets_validate_core_files(void)
{
    return file_exists("images/background.jpg")
        && file_exists("images/logo.png")
        && file_exists("images/new-game.png")
        && file_exists("images/fruit/apple.png")
        && file_exists("images/fruit/apple-1.png")
        && file_exists("images/fruit/apple-2.png")
        && file_exists("sound/menu.ogg");
}
```

- [ ] **Step 6: 运行一次真实资源校验**

Run: `rtk cc -I. main/tests/fruit_ninja_assets_test.c main/src/v9-fruit_ninja/fruit_ninja_assets.c -o /tmp/fruit_ninja_assets_test && rtk /tmp/fruit_ninja_assets_test`
Expected: PASS，说明关键文件可定位。

- [ ] **Step 7: Commit**

```bash
rtk git add main/src/v9-fruit_ninja/fruit_ninja_assets.h main/src/v9-fruit_ninja/fruit_ninja_assets.c main/tests/fruit_ninja_assets_test.c CMakeLists.txt
rtk git commit -m "test: add fruit ninja asset path guards"
```

### Task 3: 用 TDD 固化切割碰撞几何

**Files:**
- Create: `main/src/v9-fruit_ninja/fruit_ninja_collision.h`
- Create: `main/src/v9-fruit_ninja/fruit_ninja_collision.c`
- Create: `main/tests/fruit_ninja_collision_test.c`

- [ ] **Step 1: 先写失败测试，覆盖命中、未命中、擦边**

```c
#include <assert.h>
#include "../src/v9-fruit_ninja/fruit_ninja_collision.h"

int main(void)
{
    assert(fruit_ninja_segment_hits_circle(0.0f, 0.0f, 10.0f, 0.0f, 5.0f, 0.0f, 2.0f));
    assert(!fruit_ninja_segment_hits_circle(0.0f, 0.0f, 10.0f, 0.0f, 5.0f, 10.0f, 2.0f));
    assert(fruit_ninja_segment_hits_circle(0.0f, 0.0f, 10.0f, 0.0f, 10.0f, 2.0f, 2.0f));
    return 0;
}
```

- [ ] **Step 2: 运行测试并确认因符号缺失而失败**

Run: `rtk cc -I. main/tests/fruit_ninja_collision_test.c main/src/v9-fruit_ninja/fruit_ninja_collision.c -lm -o /tmp/fruit_ninja_collision_test`
Expected: FAIL，提示缺少头文件或实现。

- [ ] **Step 3: 写最小实现**

```c
#ifndef FRUIT_NINJA_COLLISION_H
#define FRUIT_NINJA_COLLISION_H

#include <stdbool.h>

bool fruit_ninja_segment_hits_circle(float x1, float y1, float x2, float y2,
                                     float cx, float cy, float radius);

#endif
```

```c
bool fruit_ninja_segment_hits_circle(float x1, float y1, float x2, float y2,
                                     float cx, float cy, float radius)
{
    const float dx = x2 - x1;
    const float dy = y2 - y1;
    const float len2 = dx * dx + dy * dy;
    float t = 0.0f;
    if(len2 > 0.0f) {
        t = ((cx - x1) * dx + (cy - y1) * dy) / len2;
        if(t < 0.0f) t = 0.0f;
        if(t > 1.0f) t = 1.0f;
    }
    const float px = x1 + t * dx;
    const float py = y1 + t * dy;
    const float ddx = px - cx;
    const float ddy = py - cy;
    return ddx * ddx + ddy * ddy <= radius * radius;
}
```

- [ ] **Step 4: 再跑测试确认通过**

Run: `rtk cc -I. main/tests/fruit_ninja_collision_test.c main/src/v9-fruit_ninja/fruit_ninja_collision.c -lm -o /tmp/fruit_ninja_collision_test && rtk /tmp/fruit_ninja_collision_test`
Expected: PASS。

- [ ] **Step 5: Commit**

```bash
rtk git add main/src/v9-fruit_ninja/fruit_ninja_collision.h main/src/v9-fruit_ninja/fruit_ninja_collision.c main/tests/fruit_ninja_collision_test.c
rtk git commit -m "test: add fruit slice collision kernel"
```

### Task 4: 搭出最小游戏入口并替换当前 demo

**Files:**
- Create: `main/src/v9-fruit_ninja/fruit_ninja.h`
- Create: `main/src/v9-fruit_ninja/fruit_ninja_scene.c`
- Modify: `main/src/main.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 先写一个最小入口实现，目标是能替代 `lv_demo_widgets()`**

```c
#ifndef FRUIT_NINJA_H
#define FRUIT_NINJA_H

void fruit_ninja_start(void);

#endif
```

```c
#include "fruit_ninja.h"
#include "fruit_ninja_assets.h"
#include "lvgl/lvgl.h"

void fruit_ninja_start(void)
{
    lv_obj_t * screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_scr_load(screen);
}
```

- [ ] **Step 2: 修改主入口前先写出失败构建预期**

Run: `rtk cmake --build build`
Expected: FAIL，原因应是 `fruit_ninja_start` 尚未被 `main.c` 引用或 build 目录尚未重新配置。

- [ ] **Step 3: 在 `main/src/main.c` 替换 demo 调用**

```c
#include "v9-fruit_ninja/fruit_ninja.h"

#if LV_USE_OS == LV_OS_NONE

  fruit_ninja_start();

  while(1) {
      lv_timer_handler();
      usleep(5 * 1000);
  }
```

- [ ] **Step 4: 重新构建确认程序进入新入口**

Run: `rtk cmake -S . -B build && rtk cmake --build build -j`
Expected: PASS，生成新的 `bin/main`。

- [ ] **Step 5: Commit**

```bash
rtk git add main/src/main.c main/src/v9-fruit_ninja/fruit_ninja.h main/src/v9-fruit_ninja/fruit_ninja_scene.c CMakeLists.txt
rtk git commit -m "feat: wire fruit ninja startup scene"
```

### Task 5: 实现首页背景、Logo、开始按钮和资源文件直读

**Files:**
- Modify: `main/src/v9-fruit_ninja/fruit_ninja_scene.c`
- Modify: `main/src/v9-fruit_ninja/fruit_ninja_assets.c`
- Test: `rtk cmake --build build -j`

- [ ] **Step 1: 在场景里先写失败路径，直接尝试加载背景图与按钮图**

```c
lv_obj_t * bg = lv_image_create(screen);
lv_image_set_src(bg, background_path);
lv_obj_center(bg);

lv_obj_t * start_btn = lv_image_create(screen);
lv_image_set_src(start_btn, new_game_path);
lv_obj_align(start_btn, LV_ALIGN_CENTER, 0, 70);
```

- [ ] **Step 2: 构建并在运行前接受可能的运行时空图风险**

Run: `rtk cmake --build build -j`
Expected: PASS 编译；如果资源路径格式不对，运行时将显示空白图，这正是下一步要验证和修复的对象。

- [ ] **Step 3: 实现场景初始化的资源路径拼接与视图层级**

```c
char background_path[512];
char logo_path[512];
char new_game_path[512];
fruit_ninja_assets_image_path(background_path, sizeof(background_path), "images/background.jpg");
fruit_ninja_assets_image_path(logo_path, sizeof(logo_path), "images/logo.png");
fruit_ninja_assets_image_path(new_game_path, sizeof(new_game_path), "images/new-game.png");
```

- [ ] **Step 4: 补上开始按钮事件，点击后切到运行状态**

```c
lv_obj_add_flag(start_btn, LV_OBJ_FLAG_CLICKABLE);
lv_obj_add_event_cb(start_btn, start_button_clicked_cb, LV_EVENT_CLICKED, game);
```

- [ ] **Step 5: 构建并手工运行验证首页资源可见**

Run: `rtk cmake --build build -j && rtk ./bin/main`
Expected: 程序启动显示背景图、logo、开始按钮，且不是空白屏。

- [ ] **Step 6: Commit**

```bash
rtk git add main/src/v9-fruit_ninja/fruit_ninja_scene.c main/src/v9-fruit_ninja/fruit_ninja_assets.c
rtk git commit -m "feat: add fruit ninja home scene"
```

### Task 6: 接入 SDL2_mixer，并先打通背景音乐与结束音效

**Files:**
- Create: `main/src/v9-fruit_ninja/fruit_ninja_audio.h`
- Create: `main/src/v9-fruit_ninja/fruit_ninja_audio.c`
- Modify: `main/src/v9-fruit_ninja/fruit_ninja_scene.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 先写最小音频接口头文件**

```c
#ifndef FRUIT_NINJA_AUDIO_H
#define FRUIT_NINJA_AUDIO_H

#include <stdbool.h>

bool fruit_ninja_audio_init(void);
void fruit_ninja_audio_shutdown(void);
void fruit_ninja_audio_play_menu_music(void);
void fruit_ninja_audio_play_slice(void);
void fruit_ninja_audio_play_boom(void);
void fruit_ninja_audio_play_game_over(void);
void fruit_ninja_audio_stop_music(void);

#endif
```

- [ ] **Step 2: 实现最小初始化并允许它先失败暴露库问题**

```c
bool fruit_ninja_audio_init(void)
{
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        LV_LOG_ERROR("Mix_OpenAudio failed: %s", Mix_GetError());
        return false;
    }
    return true;
}
```

- [ ] **Step 3: 构建确认链接问题已解决**

Run: `rtk cmake -S . -B build && rtk cmake --build build -j`
Expected: PASS；若失败，错误应集中在 SDL2_mixer 的 include / link 配置。

- [ ] **Step 4: 加入菜单音乐与结束音效播放**

```c
void fruit_ninja_audio_play_menu_music(void)
{
    Mix_PlayMusic(g_menu_music, -1);
}

void fruit_ninja_audio_play_game_over(void)
{
    Mix_PlayChannel(-1, g_game_over_sound, 0);
}
```

- [ ] **Step 5: 在首页和 Game Over 状态接入音频调用**

```c
if(fruit_ninja_audio_init()) {
    fruit_ninja_audio_play_menu_music();
}
```

- [ ] **Step 6: 运行程序做最小听觉验证**

Run: `rtk ./bin/main`
Expected: 首页能播放菜单音乐，触发 Game Over 时能播放结束音效。

- [ ] **Step 7: Commit**

```bash
rtk git add main/src/v9-fruit_ninja/fruit_ninja_audio.h main/src/v9-fruit_ninja/fruit_ninja_audio.c main/src/v9-fruit_ninja/fruit_ninja_scene.c CMakeLists.txt
rtk git commit -m "feat: add fruit ninja audio playback"
```

### Task 7: 建立游戏运行态、果实模型和抛物线更新循环

**Files:**
- Create: `main/src/v9-fruit_ninja/fruit_ninja_model.h`
- Modify: `main/src/v9-fruit_ninja/fruit_ninja_scene.c`

- [ ] **Step 1: 先定义模型与状态常量**

```c
#define FRUIT_NINJA_MAX_FRUITS 16
#define FRUIT_NINJA_MAX_TRAIL_POINTS 24

typedef enum {
    FRUIT_NINJA_STATE_HOME,
    FRUIT_NINJA_STATE_RUNNING,
    FRUIT_NINJA_STATE_EXPLODING,
    FRUIT_NINJA_STATE_GAME_OVER,
} fruit_ninja_state_t;
```

- [ ] **Step 2: 定义活跃果实结构**

```c
typedef struct {
    bool active;
    bool sliced;
    bool counted_as_miss;
    bool is_bomb;
    float x;
    float y;
    float vx;
    float vy;
    float gravity;
    float angle;
    float angular_velocity;
    float radius;
    const char * whole_image_rel;
    const char * split_left_rel;
    const char * split_right_rel;
    lv_obj_t * whole_image;
    lv_obj_t * shadow_image;
    lv_obj_t * split_left_image;
    lv_obj_t * split_right_image;
} fruit_ninja_fruit_t;
```

- [ ] **Step 3: 写出最小 spawn 逻辑**

```c
static void spawn_fruit(fruit_ninja_game_t * game, bool is_bomb)
{
    fruit_ninja_fruit_t * fruit = find_free_fruit(game);
    fruit->active = true;
    fruit->is_bomb = is_bomb;
    fruit->x = 40 + rand() % 240;
    fruit->y = 500;
    fruit->vx = -2.5f + (float)(rand() % 50) / 10.0f;
    fruit->vy = -18.0f - (float)(rand() % 60) / 10.0f;
    fruit->gravity = 0.65f;
}
```

- [ ] **Step 4: 写出逐帧更新逻辑并绑定到 LVGL timer**

```c
fruit->x += fruit->vx;
fruit->y += fruit->vy;
fruit->vy += fruit->gravity;
fruit->angle += fruit->angular_velocity;
```

- [ ] **Step 5: 编译并运行最小可见飞行水果**

Run: `rtk cmake --build build -j && rtk ./bin/main`
Expected: 点击开始后，屏幕上出现从底部飞出的水果图片。

- [ ] **Step 6: Commit**

```bash
rtk git add main/src/v9-fruit_ninja/fruit_ninja_model.h main/src/v9-fruit_ninja/fruit_ninja_scene.c
rtk git commit -m "feat: add fruit spawn and motion loop"
```

### Task 8: 加入鼠标拖拽刀痕与切割事件流

**Files:**
- Create: `main/src/v9-fruit_ninja/fruit_ninja_input.c`
- Modify: `main/src/v9-fruit_ninja/fruit_ninja_scene.c`
- Modify: `main/src/v9-fruit_ninja/fruit_ninja_model.h`

- [ ] **Step 1: 先在模型里定义刀轨结构**

```c
typedef struct {
    bool active;
    uint16_t count;
    lv_point_precise_t points[FRUIT_NINJA_MAX_TRAIL_POINTS];
    lv_obj_t * polyline;
    float last_x;
    float last_y;
    bool has_last_point;
} fruit_ninja_trail_t;
```

- [ ] **Step 2: 在输入层写最小事件处理**

```c
static void game_pointer_event_cb(lv_event_t * e)
{
    fruit_ninja_game_t * game = lv_event_get_user_data(e);
    lv_indev_t * indev = lv_indev_get_act();
    lv_point_t p;
    lv_indev_get_point(indev, &p);
    fruit_ninja_input_push_point(game, (float)p.x, (float)p.y);
}
```

- [ ] **Step 3: 初版刀痕先用折线对象渲染**

```c
game->trail.polyline = lv_line_create(game->effect_layer);
lv_obj_set_style_line_width(game->trail.polyline, 10, 0);
lv_obj_set_style_line_color(game->trail.polyline, lv_color_hex(0xcbd3db), 0);
```

- [ ] **Step 4: 将拖拽事件绑定到运行态根容器**

```c
lv_obj_add_flag(game->input_layer, LV_OBJ_FLAG_CLICKABLE);
lv_obj_add_event_cb(game->input_layer, game_pointer_event_cb, LV_EVENT_PRESSING, game);
lv_obj_add_event_cb(game->input_layer, game_pointer_released_cb, LV_EVENT_RELEASED, game);
```

- [ ] **Step 5: 构建并手工验证可见刀痕**

Run: `rtk cmake --build build -j && rtk ./bin/main`
Expected: 鼠标按住拖拽时有明显刀痕，不按住时刀痕逐渐消失。

- [ ] **Step 6: Commit**

```bash
rtk git add main/src/v9-fruit_ninja/fruit_ninja_input.c main/src/v9-fruit_ninja/fruit_ninja_scene.c main/src/v9-fruit_ninja/fruit_ninja_model.h
rtk git commit -m "feat: add fruit ninja knife trail input"
```

### Task 9: 把碰撞内核接进运行态，实现切水果、分裂、计分和炸弹失败

**Files:**
- Modify: `main/src/v9-fruit_ninja/fruit_ninja_scene.c`
- Modify: `main/src/v9-fruit_ninja/fruit_ninja_audio.c`
- Modify: `main/src/v9-fruit_ninja/fruit_ninja_model.h`

- [ ] **Step 1: 在拖拽新线段生成时遍历活跃果实做碰撞判定**

```c
if(!fruit->sliced && fruit_ninja_segment_hits_circle(x1, y1, x2, y2,
                                                     fruit->x, fruit->y, fruit->radius)) {
    handle_fruit_slice(game, fruit, x2 - x1, y2 - y1);
}
```

- [ ] **Step 2: 先实现普通水果的最小切开表现**

```c
fruit->sliced = true;
lv_obj_add_flag(fruit->whole_image, LV_OBJ_FLAG_HIDDEN);
fruit->split_left_image = lv_image_create(game->fruit_layer);
fruit->split_right_image = lv_image_create(game->fruit_layer);
lv_image_set_src(fruit->split_left_image, split_left_path);
lv_image_set_src(fruit->split_right_image, split_right_path);
game->score += 1;
fruit_ninja_audio_play_slice();
```

- [ ] **Step 3: 实现炸弹命中逻辑**

```c
if(fruit->is_bomb) {
    fruit_ninja_audio_play_boom();
    game_enter_exploding(game);
    return;
}
```

- [ ] **Step 4: 将分裂半片纳入物理更新**

```c
split->x += split->vx;
split->y += split->vy;
split->vy += split->gravity;
split->angle += split->angular_velocity;
```

- [ ] **Step 5: 构建并手工验证切中、计分和炸弹结束**

Run: `rtk cmake --build build -j && rtk ./bin/main`
Expected: 普通水果能被切开并加分，炸弹被切中后立即结束。

- [ ] **Step 6: Commit**

```bash
rtk git add main/src/v9-fruit_ninja/fruit_ninja_scene.c main/src/v9-fruit_ninja/fruit_ninja_audio.c main/src/v9-fruit_ninja/fruit_ninja_model.h
rtk git commit -m "feat: add slicing, score and bomb failure"
```

### Task 10: 实现漏切次数、Game Over、重新开始与清理闭环

**Files:**
- Modify: `main/src/v9-fruit_ninja/fruit_ninja_scene.c`
- Modify: `main/src/v9-fruit_ninja/fruit_ninja_audio.c`

- [ ] **Step 1: 先写漏切与状态切换代码骨架**

```c
if(!fruit->sliced && fruit->y > game->screen_height + 80.0f && !fruit->counted_as_miss) {
    fruit->counted_as_miss = true;
    game->misses += 1;
    update_miss_icons(game);
    if(game->misses >= 3) {
        game_enter_game_over(game);
    }
}
```

- [ ] **Step 2: 实现 Game Over 画面与计时器停止**

```c
static void game_enter_game_over(fruit_ninja_game_t * game)
{
    game->state = FRUIT_NINJA_STATE_GAME_OVER;
    lv_timer_pause(game->spawn_timer);
    lv_timer_pause(game->update_timer);
    fruit_ninja_audio_stop_music();
    fruit_ninja_audio_play_game_over();
    lv_obj_clear_flag(game->game_over_image, LV_OBJ_FLAG_HIDDEN);
}
```

- [ ] **Step 3: 实现重开时的完整清理**

```c
static void reset_runtime_state(fruit_ninja_game_t * game)
{
    clear_all_fruits(game);
    clear_all_effects(game);
    game->score = 0;
    game->misses = 0;
    game->state = FRUIT_NINJA_STATE_HOME;
}
```

- [ ] **Step 4: 手工验证完整闭环**

Run: `rtk ./bin/main`
Expected: 可以开始一局、漏掉 3 个水果进入 Game Over、点击后重新开始且状态被清空。

- [ ] **Step 5: Commit**

```bash
rtk git add main/src/v9-fruit_ninja/fruit_ninja_scene.c main/src/v9-fruit_ninja/fruit_ninja_audio.c
rtk git commit -m "feat: close fruit ninja game loop"
```

### Task 11: 整体验证、补测试入口并收口运行说明

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `README.md` 或新增 `main/src/v9-fruit_ninja/README.md`（若确有必要）

- [ ] **Step 1: 把两个最小 C 测试接入 CMake 自定义目标**

```cmake
add_executable(fruit_ninja_assets_test main/tests/fruit_ninja_assets_test.c main/src/v9-fruit_ninja/fruit_ninja_assets.c)
add_executable(fruit_ninja_collision_test main/tests/fruit_ninja_collision_test.c main/src/v9-fruit_ninja/fruit_ninja_collision.c)
add_custom_target(fruit_ninja_tests
    COMMAND fruit_ninja_assets_test
    COMMAND fruit_ninja_collision_test
    DEPENDS fruit_ninja_assets_test fruit_ninja_collision_test)
```

- [ ] **Step 2: 运行自动化验证**

Run: `rtk cmake -S . -B build && rtk cmake --build build -j && rtk cmake --build build --target fruit_ninja_tests`
Expected: PASS。

- [ ] **Step 3: 运行最终手工验证**

Run: `rtk ./bin/main`
Expected: 启动后直接进入 Fruit Ninja，首页/音乐/开局/切水果/炸弹/漏切/Game Over/重开均可运行。

- [ ] **Step 4: 仅在确有必要时补最小说明**

```markdown
## Fruit Ninja v9

- 启动程序后直接进入游戏首页
- 资源来自 `third-party/FruitNinja`
- 鼠标按住拖拽即可切水果
```

- [ ] **Step 5: Commit**

```bash
rtk git add CMakeLists.txt README.md main/tests/fruit_ninja_assets_test.c main/tests/fruit_ninja_collision_test.c
rtk git commit -m "test: verify fruit ninja playable flow"
```

## 计划自检清单

- 规格覆盖：启动入口、文件图片、文件音频、首页、运行态、切割、计分、炸弹、漏切、结算、重开、验证命令都已映射到任务。
- 占位检查：没有使用 “TBD/TODO/以后再做” 之类占位语句作为执行步骤。
- 类型一致性：统一使用 `fruit_ninja_*` 前缀；图片走 LVGL 文件路径，音频走 SDL 原始路径；状态名、测试名、目录名保持一致。
