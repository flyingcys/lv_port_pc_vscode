#include "fruit_ninja_physics.h"
#include "fruit_ninja_internal.h"
#include "fruit_ninja_easing.h"
#include "fruit_ninja_audio.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* shadow 素材路径(与 scene.c init_ui_asset_paths 保持一致) */
#define PHYSICS_SHADOW_PATH "images/shadow.png"

static const fruit_ninja_fruit_def_t g_fruit_defs[] = {
    /* [0] peach(桃):桃黄色汁液 */
    { "peach", "images/fruit/peach.png", "images/fruit/peach-1.png", "images/fruit/peach-2.png", 62, 59, 37.0f, -50, false, false, true, 0xe6, 0xc7, 0x31 },
    /* [1] sandia(西瓜):红色汁液 */
    { "sandia", "images/fruit/sandia.png", "images/fruit/sandia-1.png", "images/fruit/sandia-2.png", 98, 85, 38.0f, -100, false, false, true, 0xcc, 0x00, 0x00 },
    /* [2] apple(苹果):黄绿色汁液 */
    { "apple", "images/fruit/apple.png", "images/fruit/apple-1.png", "images/fruit/apple-2.png", 66, 66, 31.0f, -54, false, false, true, 0xc8, 0xe9, 0x25 },
    /* [3] banana(香蕉):无汁液 */
    { "banana", "images/fruit/banana.png", "images/fruit/banana-1.png", "images/fruit/banana-2.png", 126, 50, 43.0f, 90, false, false, false, 0x00, 0x00, 0x00 },
    /* [4] basaha:红色汁液 */
    { "basaha", "images/fruit/basaha.png", "images/fruit/basaha-1.png", "images/fruit/basaha-2.png", 68, 72, 32.0f, -135, false, false, true, 0xcc, 0x00, 0x00 },
    /* [5] boom(炸弹):无汁液 */
    { "boom", "images/fruit/boom.png", NULL, NULL, 66, 68, 26.0f, 0, false, true, false, 0x00, 0x00, 0x00 },
};

static inline float frand_range(float min_value, float max_value)
{
    return min_value + ((float)rand() / (float)RAND_MAX) * (max_value - min_value);
}

const fruit_ninja_fruit_def_t * fruit_ninja_physics_get_fruit_def(uint32_t index)
{
    if(index >= (sizeof(g_fruit_defs) / sizeof(g_fruit_defs[0]))) return NULL;
    return &g_fruit_defs[index];
}

const fruit_ninja_fruit_def_t * fruit_ninja_physics_choose_def(void)
{
    if((rand() % 8) == 4) {
        return &g_fruit_defs[5];
    }

    return &g_fruit_defs[rand() % 5];
}

uint32_t fruit_ninja_physics_active_fruits(const fruit_ninja_game_t * game)
{
    uint32_t i;
    uint32_t count = 0;

    for(i = 0; i < FRUIT_NINJA_MAX_FRUITS; ++i) {
        const fruit_ninja_fruit_t * fruit = &game->fruits[i];
        if(fruit->active && !fruit->sliced) {
            count++;
        }
    }

    return count;
}

uint32_t fruit_ninja_physics_target_count(const fruit_ninja_game_t * game)
{
    return game->volley_num > 0U ? game->volley_num : 2U;
}

static fruit_ninja_fragment_t * alloc_fragment(fruit_ninja_game_t * game)
{
    uint32_t i;
    for(i = 0; i < FRUIT_NINJA_MAX_FRAGMENTS; ++i) {
        if(!game->fragments[i].active) {
            memset(&game->fragments[i], 0, sizeof(game->fragments[i]));
            game->fragments[i].active = true;
            return &game->fragments[i];
        }
    }
    return NULL;
}

fruit_ninja_fragment_t * fruit_ninja_physics_spawn_fragment(fruit_ninja_game_t * game, const char * relative_path, float x, float y,
                                                            float vx, float vy, float angular_velocity, float angle)
{
    fruit_ninja_fragment_t * fragment = alloc_fragment(game);
    char path[512];

    if(fragment == NULL || relative_path == NULL) return NULL;

    fragment->x = x;
    fragment->y = y;
    fragment->vx = vx;
    fragment->vy = vy;
    fragment->gravity = FRUIT_NINJA_GRAVITY;
    fragment->angle = angle;
    fragment->angular_velocity = angular_velocity;
    fragment->life_ms = 1400;
    fragment->start_x = x;
    fragment->start_y = y;
    fragment->target_x = x;
    fragment->target_y = y;
    fragment->start_angle = angle;
    fragment->target_angle = angle;
    fragment->phase_elapsed_ms = 0;
    fragment->image = lv_image_create(game->fruit_layer);
    if(fruit_ninja_make_image_path(path, sizeof(path), relative_path)) {
        lv_image_set_src(fragment->image, path);
    }
    lv_image_set_pivot(fragment->image, 32, 32);
    lv_image_set_scale(fragment->image, (uint16_t)(fruit_ninja_viewport_scale() * 256.0f));
    /* pivot=(32,32) 原始像素:pos = viewport(逻辑 pivot 点) - 32。 */
    lv_obj_set_pos(fragment->image,
                   (int32_t)lroundf(fruit_ninja_viewport_x(x + 32.0f) - 32.0f),
                   (int32_t)lroundf(fruit_ninja_viewport_y(y + 32.0f) - 32.0f));
    lv_image_set_rotation(fragment->image, (int32_t)(angle * 10.0f));
    return fragment;
}

fruit_ninja_fruit_t * fruit_ninja_physics_alloc_fruit(fruit_ninja_game_t * game)
{
    uint32_t i;
    for(i = 0; i < FRUIT_NINJA_MAX_FRUITS; ++i) {
        if(!game->fruits[i].active) {
            memset(&game->fruits[i], 0, sizeof(game->fruits[i]));
            game->fruits[i].active = true;
            return &game->fruits[i];
        }
    }
    return NULL;
}

void fruit_ninja_physics_update_single_fruit_visual(fruit_ninja_fruit_t * fruit)
{
    /* fruit->x/y 为逻辑坐标(640x480 系):显示时经 viewport 等比映射到物理屏。
     * lv_image 经 set_scale 围绕 pivot(原始位图中心)缩放,obj pos 仍定位"原始位图左上角",
     * 故:pos = viewport(逻辑中心) - 原始位图半宽高(原始像素,非缩放后)。 */
    uint16_t img_scale = (uint16_t)(fruit_ninja_viewport_scale() * 256.0f);

    if(fruit->shadow_image != NULL) {
        /* shadow 默认 pivot 为位图中心;原始逻辑左上角为 (x-53, y+45)。 */
        float sw = (float)lv_obj_get_width(fruit->shadow_image);
        float sh = (float)lv_obj_get_height(fruit->shadow_image);
        float scx = (fruit->x - 53.0f) + sw * 0.5f;
        float scy = (fruit->y - 5.0f + 50.0f) + sh * 0.5f;
        lv_image_set_scale(fruit->shadow_image, img_scale);
        lv_obj_set_pos(fruit->shadow_image,
                       (int32_t)lroundf(fruit_ninja_viewport_x(scx) - sw * 0.5f),
                       (int32_t)lroundf(fruit_ninja_viewport_y(scy) - sh * 0.5f));
    }
    if(fruit->whole_image != NULL) {
        /* whole_image pivot 设为 (width/2, height/2),逻辑中心即 (fruit->x, fruit->y)。 */
        float hw = (float)fruit->def->width * 0.5f;
        float hh = (float)fruit->def->height * 0.5f;
        lv_image_set_scale(fruit->whole_image, img_scale);
        lv_obj_set_pos(fruit->whole_image,
                       (int32_t)lroundf(fruit_ninja_viewport_x(fruit->x) - hw),
                       (int32_t)lroundf(fruit_ninja_viewport_y(fruit->y) - hh));
        lv_image_set_rotation(fruit->whole_image, (int32_t)(fruit->angle * 10.0f));
    }
}

void fruit_ninja_physics_spawn_one_fruit(fruit_ninja_game_t * game)
{
    const fruit_ninja_fruit_def_t * def = fruit_ninja_physics_choose_def();
    fruit_ninja_fruit_t * fruit = fruit_ninja_physics_alloc_fruit(game);
    char path[512];

    if(fruit == NULL) return;

    fruit->def = def;
    fruit->radius = def->radius;
    fruit->x = frand_range(0.0f, (float)game->screen_width - 1.0f);
    fruit->y = FRUIT_NINJA_JS_START_Y;
    fruit->vx = 0.0f;
    fruit->vy = 0.0f;
    fruit->gravity = 0.0f;
    fruit->angle = (float)def->base_rotation_deg;
    fruit->angular_velocity = 0.0f;
    fruit->shot_out_start_x = fruit->x;
    fruit->shot_out_start_y = fruit->y;
    fruit->fall_target_x = frand_range(0.0f, (float)game->screen_width - 1.0f);
    fruit->shot_out_end_x = (fruit->shot_out_start_x + fruit->fall_target_x) * 0.5f;
    fruit->shot_out_end_y = fminf(FRUIT_NINJA_JS_START_Y - (float)(rand() % 500), 200.0f);
    fruit->fall_target_y = FRUIT_NINJA_JS_START_Y;
    fruit->phase_elapsed_ms = 0;
    fruit->falling = false;

    if(!def->is_bomb) {
        static const float SPINS[3] = {60.0f, 50.0f, 40.0f};
        float sign = (rand() % 2 == 0) ? -1.0f : 1.0f;
        fruit->angular_velocity = sign * SPINS[rand() % 3];
    }

    fruit->shadow_image = lv_image_create(game->fruit_layer);
    if(fruit_ninja_make_image_path(path, sizeof(path), PHYSICS_SHADOW_PATH)) {
        lv_image_set_src(fruit->shadow_image, path);
    }

    fruit->whole_image = lv_image_create(game->fruit_layer);
    if(fruit_ninja_make_image_path(path, sizeof(path), def->whole_rel_path)) {
        lv_image_set_src(fruit->whole_image, path);
    }
    lv_image_set_pivot(fruit->whole_image, def->width / 2, def->height / 2);
    fruit_ninja_physics_update_single_fruit_visual(fruit);

    if(game->audio_ready) {
        fruit_ninja_audio_play_throw();
    }
}

void fruit_ninja_physics_update_fruits(fruit_ninja_game_t * game)
{
    uint32_t i;

    /* 每帧重置炸弹跟踪;循环内若找到 active 未切炸弹则置 true */
    game->bomb_alive = false;

    for(i = 0; i < FRUIT_NINJA_MAX_FRUITS; ++i) {
        fruit_ninja_fruit_t * fruit = &game->fruits[i];
        if(!fruit->active) continue;

        /* 炸弹位置跟踪:active 且未被切割的炸弹 */
        if(fruit->def != NULL && fruit->def->is_bomb && !fruit->sliced) {
            game->bomb_alive = true;
            game->bomb_x = fruit->x;
            game->bomb_y = fruit->y;
        }

        if(game->state == FRUIT_NINJA_STATE_RUNNING) {
            float prev_x = fruit->x;
            float prev_y = fruit->y;
            float progress;

            fruit->phase_elapsed_ms += FRUIT_NINJA_UPDATE_MS;
            if(!fruit->falling) {
                progress = (float)fruit->phase_elapsed_ms / (float)FRUIT_NINJA_DROP_TIME_MS;
                if(progress > 1.0f) progress = 1.0f;
                fruit->x = fruit->shot_out_start_x + (fruit->shot_out_end_x - fruit->shot_out_start_x) * progress;
                fruit->y = fruit->shot_out_start_y + (fruit->shot_out_end_y - fruit->shot_out_start_y) * fruit_ninja_ease_out_quad(progress);
                if(fruit->phase_elapsed_ms >= FRUIT_NINJA_DROP_TIME_MS) {
                    fruit->falling = true;
                    fruit->phase_elapsed_ms = 0;
                    fruit->shot_out_start_x = fruit->x;
                    fruit->shot_out_start_y = fruit->y;
                }
            }
            else {
                progress = (float)fruit->phase_elapsed_ms / (float)FRUIT_NINJA_DROP_TIME_MS;
                if(progress > 1.0f) progress = 1.0f;
                fruit->x = fruit->shot_out_start_x + (fruit->fall_target_x - fruit->shot_out_start_x) * progress;
                fruit->y = fruit->shot_out_start_y + (fruit->fall_target_y - fruit->shot_out_start_y) * fruit_ninja_ease_in_quad(progress);
            }

            if(!fruit->def->is_bomb) {
                fruit->angle += fruit->angular_velocity * ((float)FRUIT_NINJA_UPDATE_MS / 1000.0f);
            }

            fruit->vx = fruit->x - prev_x;
            fruit->vy = fruit->y - prev_y;
        }

        fruit_ninja_physics_update_single_fruit_visual(fruit);

        if(!fruit->has_been_visible && fruit_ninja_fruit_is_visible_on_screen(fruit, game->screen_height)) {
            fruit->has_been_visible = true;
        }

        if(fruit_ninja_fruit_should_count_miss(fruit, game->screen_height)) {
            fruit->counted_as_miss = true;
            fruit->active = false;
            fruit_ninja_destroy_if_present(&fruit->whole_image);
            fruit_ninja_destroy_if_present(&fruit->shadow_image);
            game->misses += 1;
            fruit_ninja_state_update_miss_icons(game);
            if(game->misses >= 3) {
                fruit_ninja_state_enter_game_over(game);
                return;
            }
            continue;
        }

        if((fruit->sliced || fruit->def->is_bomb) && fruit->y > (float)game->screen_height + 120.0f) {
            fruit->active = false;
            fruit_ninja_destroy_if_present(&fruit->whole_image);
            fruit_ninja_destroy_if_present(&fruit->shadow_image);
        }
    }
}

void fruit_ninja_physics_update_fragments(fruit_ninja_game_t * game)
{
    uint32_t i;

    for(i = 0; i < FRUIT_NINJA_MAX_FRAGMENTS; ++i) {
        fruit_ninja_fragment_t * fragment = &game->fragments[i];
        float progress;
        if(!fragment->active) continue;

        fragment->phase_elapsed_ms += FRUIT_NINJA_UPDATE_MS;
        progress = (float)fragment->phase_elapsed_ms / (float)FRUIT_NINJA_DROP_TIME_MS;
        if(progress > 1.0f) progress = 1.0f;
        fragment->x = fragment->start_x + (fragment->target_x - fragment->start_x) * progress;
        fragment->y = fragment->start_y + (fragment->target_y - fragment->start_y) * fruit_ninja_ease_in_quad(progress);
        fragment->angle = fragment->start_angle + (fragment->target_angle - fragment->start_angle) * progress;
        if(fragment->life_ms > FRUIT_NINJA_UPDATE_MS) {
            fragment->life_ms -= FRUIT_NINJA_UPDATE_MS;
        }
        else {
            fragment->life_ms = 0;
        }

        if(fragment->image != NULL) {
            /* pivot=(32,32) 原始像素:pos = viewport(逻辑 pivot 点) - 32。 */
            lv_obj_set_pos(fragment->image,
                           (int32_t)lroundf(fruit_ninja_viewport_x(fragment->x + 32.0f) - 32.0f),
                           (int32_t)lroundf(fruit_ninja_viewport_y(fragment->y + 32.0f) - 32.0f));
            lv_image_set_rotation(fragment->image, (int32_t)(fragment->angle * 10.0f));
        }

        if(fragment->life_ms == 0 || fragment->y > (float)game->screen_height + 120.0f) {
            fruit_ninja_destroy_if_present(&fragment->image);
            memset(fragment, 0, sizeof(*fragment));
        }
    }
}
