#include "fruit_ninja_state.h"
#include "fruit_ninja_internal.h"
#include "fruit_ninja_physics.h"
#include "fruit_ninja_audio.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* ---- asset path constants (mirrors scene.c init_ui_asset_paths) ---- */
static const char * const s_lose_full[3]  = { "images/xf.png",  "images/xxf.png",  "images/xxxf.png" };
static const char * const s_lose_empty[3] = { "images/x.png",   "images/xx.png",   "images/xxx.png"  };
static const char *       s_shadow        = "images/shadow.png";
static const char *       s_smoke         = "images/smoke.png";
static const char *       s_flash         = "images/flash.png";

static const lv_point_t s_home_menu_positions[3] = {
    { 137, 333 },
    { 330, 322 },
    { 552, 367 },
};

/* ---- static helpers ---- */

static void clear_fragments(fruit_ninja_game_t * game)
{
    uint32_t i;
    for(i = 0; i < FRUIT_NINJA_MAX_FRAGMENTS; ++i) {
        if(game->fragments[i].active) {
            fruit_ninja_destroy_if_present(&game->fragments[i].image);
            memset(&game->fragments[i], 0, sizeof(game->fragments[i]));
        }
    }
}

static void clear_home_menu_fruits(fruit_ninja_game_t * game)
{
    uint32_t i;

    for(i = 0; i < 3; ++i) {
        fruit_ninja_destroy_if_present(&game->home_menu_fruits[i].whole_image);
        fruit_ninja_destroy_if_present(&game->home_menu_fruits[i].shadow_image);
        fruit_ninja_destroy_if_present(&game->home_menu_fruits[i].slice_flash);
        memset(&game->home_menu_fruits[i], 0, sizeof(game->home_menu_fruits[i]));
    }
}

static void clear_fruits(fruit_ninja_game_t * game)
{
    uint32_t i;
    for(i = 0; i < FRUIT_NINJA_MAX_FRUITS; ++i) {
        if(game->fruits[i].active) {
            fruit_ninja_destroy_if_present(&game->fruits[i].whole_image);
            fruit_ninja_destroy_if_present(&game->fruits[i].shadow_image);
            fruit_ninja_destroy_if_present(&game->fruits[i].slice_flash);
            memset(&game->fruits[i], 0, sizeof(game->fruits[i]));
        }
    }
}

static void stage_home_object(lv_obj_t * obj, bool visible, lv_opa_t opa, int32_t y)
{
    if(obj == NULL) return;
    if(visible) fruit_ninja_show_obj(obj);
    else fruit_ninja_hide_obj(obj);
    lv_obj_set_style_opa(obj, opa, 0);
    lv_obj_set_y(obj, y);
}

static void spawn_flash(fruit_ninja_game_t * game, float x, float y)
{
    lv_obj_t * flash = fruit_ninja_create_file_image(game->effect_layer, s_flash);
    fruit_ninja_set_image_geometry(flash, (int32_t)x - 179, (int32_t)y - 10, 358, 20);
    lv_obj_set_style_opa(flash, LV_OPA_100, 0);
    lv_image_set_pivot(flash, 179, 10);
    lv_image_set_scale(flash, 1);

    if(game->flash_overlay != NULL) {
        lv_obj_delete(game->flash_overlay);
    }
    game->flash_overlay = flash;
    game->flash_age_ms = 0;
}

static void clear_explosion_overlays(fruit_ninja_game_t * game)
{
    fruit_ninja_destroy_if_present(&game->smoke_overlay);
    fruit_ninja_destroy_if_present(&game->white_flash_overlay);
}

void fruit_ninja_state_start_running_timer_cb(lv_timer_t * timer)
{
    fruit_ninja_game_t * game = timer->user_data;
    lv_timer_delete(timer);
    fruit_ninja_state_enter_running(game);
}

void fruit_ninja_state_restore_home_fruit_timer_cb(lv_timer_t * timer)
{
    fruit_ninja_fruit_t * fruit = timer->user_data;
    fruit->sliced = false;
    fruit_ninja_show_obj(fruit->whole_image);
    fruit_ninja_show_obj(fruit->shadow_image);
    lv_timer_delete(timer);
}

/* ---- public functions ---- */

void fruit_ninja_state_update_score_label(fruit_ninja_game_t * game)
{
    lv_label_set_text_fmt(game->score_label, "%u", game->score);
}

void fruit_ninja_state_update_miss_icons(fruit_ninja_game_t * game)
{
    uint32_t i;
    char path[512];

    for(i = 0; i < 3; ++i) {
        if(fruit_ninja_make_image_path(path, sizeof(path), i < game->misses ? s_lose_full[i] : s_lose_empty[i])) {
            lv_image_set_src(game->miss_icons[i], path);
        }
    }
}

void fruit_ninja_state_build_home_menu_fruits(fruit_ninja_game_t * game)
{
    uint32_t i;
    char path[512];

    for(i = 0; i < 3; ++i) {
        fruit_ninja_fruit_t * fruit = &game->home_menu_fruits[i];
        static const uint32_t s_home_menu_def_indices[3] = { 0, 1, 5 };
        const fruit_ninja_fruit_def_t * def = fruit_ninja_physics_get_fruit_def(s_home_menu_def_indices[i]);

        memset(fruit, 0, sizeof(*fruit));
        fruit->active = true;
        fruit->def = def;
        fruit->radius = def->radius;
        fruit->x = (float)s_home_menu_positions[i].x;
        fruit->y = (float)s_home_menu_positions[i].y;
        fruit->angle = (float)def->base_rotation_deg;
        fruit->angular_velocity = (i == 2U) ? 0.0f : 2.0f + (float)i;

        fruit->shadow_image = lv_image_create(game->home_layer);
        if(fruit_ninja_make_image_path(path, sizeof(path), s_shadow)) {
            lv_image_set_src(fruit->shadow_image, path);
        }
        fruit_ninja_hide_obj(fruit->shadow_image);

        fruit->whole_image = lv_image_create(game->home_layer);
        if(fruit_ninja_make_image_path(path, sizeof(path), def->whole_rel_path)) {
            lv_image_set_src(fruit->whole_image, path);
        }
        lv_image_set_pivot(fruit->whole_image, def->width / 2, def->height / 2);
        fruit_ninja_hide_obj(fruit->whole_image);
        fruit_ninja_physics_update_single_fruit_visual(fruit);
    }
}

void fruit_ninja_state_update_home_animation(fruit_ninja_game_t * game)
{
    uint32_t i;
    float t = (float)game->state_elapsed_ms;
    float bob = sinf(t / 260.0f) * FRUIT_NINJA_HOME_FLOAT_AMPLITUDE;
    bool stage0 = game->state_elapsed_ms >= 0U;
    bool stage1 = game->state_elapsed_ms >= 500U;
    bool stage2 = game->state_elapsed_ms >= 1500U;
    bool stage3 = game->state_elapsed_ms >= 2000U;

    if(game->home_mask_image != NULL) {
        stage_home_object(game->home_mask_image, stage0, LV_OPA_COVER, 0);
    }
    if(game->logo_image != NULL) {
        stage_home_object(game->logo_image, stage0, LV_OPA_COVER, 32 + (int32_t)(sinf(t / 380.0f) * 6.0f));
        lv_obj_set_y(game->logo_image, 32 + (int32_t)(sinf(t / 380.0f) * 6.0f));
    }
    if(game->ninja_image != NULL) {
        stage_home_object(game->ninja_image, stage1, LV_OPA_COVER, 162 + (int32_t)bob);
        lv_obj_set_y(game->ninja_image, 162 + (int32_t)bob);
    }
    if(game->home_desc_image != NULL) {
        stage_home_object(game->home_desc_image, stage2, LV_OPA_COVER, 206);
    }
    if(game->dojo_image != NULL) {
        stage_home_object(game->dojo_image, stage3, LV_OPA_COVER, 278 + (int32_t)(sinf(t / 220.0f) * 3.0f));
    }
    if(game->new_game_image != NULL) {
        stage_home_object(game->new_game_image, stage3, LV_OPA_COVER, 280 + (int32_t)(sinf(t / 210.0f) * 2.0f));
    }
    if(game->new_sign_image != NULL && stage3) {
        fruit_ninja_show_obj(game->new_sign_image);
        lv_obj_set_y(game->new_sign_image, 252 + (int32_t)(sinf(t / 180.0f) * 4.0f));
        lv_obj_set_style_opa(game->new_sign_image, LV_OPA_COVER, 0);
    }
    else if(game->new_sign_image != NULL) {
        fruit_ninja_hide_obj(game->new_sign_image);
    }

    if(stage3) {
        for(i = 0; i < 3; ++i) {
            fruit_ninja_fruit_t * fruit = &game->home_menu_fruits[i];
            if(!fruit->active || fruit->sliced) continue;
            fruit_ninja_show_obj(fruit->shadow_image);
            fruit_ninja_show_obj(fruit->whole_image);
            if(i != 2U) {
                fruit->angle += fruit->angular_velocity;
            }
            fruit_ninja_physics_update_single_fruit_visual(fruit);
        }
    }
}

void fruit_ninja_state_enter_home(fruit_ninja_game_t * game)
{
    game->state = FRUIT_NINJA_STATE_HOME;
    game->state_elapsed_ms = 0;
    game->spawn_elapsed_ms = 0;
    game->spawn_interval_ms = 1000;
    clear_fruits(game);
    clear_fragments(game);
    clear_home_menu_fruits(game);
    fruit_ninja_state_build_home_menu_fruits(game);
    fruit_ninja_input_reset(game);
    game->flash_age_ms = 0;
    game->score_pulse_ms = 0;
    game->score = 0;
    game->misses = 0;
    game->volley_num = 2U;
    game->volley_multiple = 5U;
    fruit_ninja_state_update_score_label(game);
    fruit_ninja_state_update_miss_icons(game);
    fruit_ninja_show_obj(game->home_layer);
    fruit_ninja_hide_obj(game->hud_layer);
    fruit_ninja_hide_obj(game->game_over_image);
    fruit_ninja_hide_obj(game->restart_label);
    fruit_ninja_show_obj(game->hint_label);
    clear_explosion_overlays(game);
    fruit_ninja_audio_stop_music();
    if(game->audio_ready) {
        fruit_ninja_audio_play_menu_music();
    }
}

void fruit_ninja_state_enter_running(fruit_ninja_game_t * game)
{
    game->state = FRUIT_NINJA_STATE_RUNNING;
    game->state_elapsed_ms = 0;
    game->spawn_elapsed_ms = 500;
    game->spawn_interval_ms = 1000;
    clear_fruits(game);
    clear_fragments(game);
    clear_home_menu_fruits(game);
    fruit_ninja_input_reset(game);
    game->flash_age_ms = 0;
    game->score_pulse_ms = 0;
    game->score = 0;
    game->misses = 0;
    game->spawn_index = 0;
    fruit_ninja_state_update_score_label(game);
    fruit_ninja_state_update_miss_icons(game);
    fruit_ninja_hide_obj(game->home_layer);
    fruit_ninja_show_obj(game->hud_layer);
    fruit_ninja_hide_obj(game->game_over_image);
    fruit_ninja_hide_obj(game->restart_label);
    fruit_ninja_hide_obj(game->hint_label);
    clear_explosion_overlays(game);
    if(game->audio_ready) {
        fruit_ninja_audio_stop_music();
        fruit_ninja_audio_play_start();
    }
}

void fruit_ninja_state_enter_game_over(fruit_ninja_game_t * game)
{
    game->state = FRUIT_NINJA_STATE_GAME_OVER;
    game->state_elapsed_ms = 0;
    fruit_ninja_hide_obj(game->hint_label);
    fruit_ninja_show_obj(game->game_over_image);
    fruit_ninja_show_obj(game->restart_label);
    if(game->audio_ready) {
        fruit_ninja_audio_stop_music();
        fruit_ninja_audio_play_game_over();
    }
    clear_home_menu_fruits(game);
}

void fruit_ninja_state_enter_exploding(fruit_ninja_game_t * game, float x, float y)
{
    uint32_t i;
    char path[512];

    game->state = FRUIT_NINJA_STATE_EXPLODING;
    game->state_elapsed_ms = 0;
    spawn_flash(game, x, y);
    if(game->smoke_overlay == NULL) {
        game->smoke_overlay = lv_image_create(game->overlay_layer);
        if(fruit_ninja_make_image_path(path, sizeof(path), s_smoke)) {
            lv_image_set_src(game->smoke_overlay, path);
        }
    }
    lv_obj_set_pos(game->smoke_overlay, (int32_t)x - 22, (int32_t)y - 22);
    fruit_ninja_show_obj(game->smoke_overlay);
    if(game->white_flash_overlay == NULL) {
        game->white_flash_overlay = lv_obj_create(game->overlay_layer);
        lv_obj_remove_style_all(game->white_flash_overlay);
        lv_obj_set_size(game->white_flash_overlay, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(game->white_flash_overlay, lv_color_hex(0xffffff), 0);
    }
    lv_obj_set_style_bg_opa(game->white_flash_overlay, LV_OPA_80, 0);
    fruit_ninja_show_obj(game->white_flash_overlay);
    for(i = 0; i < FRUIT_NINJA_MAX_FRUITS; ++i) {
        if(game->fruits[i].active) {
            game->fruits[i].vx *= 0.2f;
            game->fruits[i].vy *= 0.2f;
            game->fruits[i].angular_velocity *= 0.2f;
        }
    }
    if(game->audio_ready) {
        fruit_ninja_audio_play_boom();
    }
}
