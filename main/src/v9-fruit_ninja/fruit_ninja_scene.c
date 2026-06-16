#include "fruit_ninja.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "fruit_ninja_assets.h"
#include "fruit_ninja_audio.h"
#include "fruit_ninja_collision.h"
#include "fruit_ninja_internal.h"
#include "fruit_ninja_physics.h"

typedef struct {
    const char * background;
    const char * home_mask;
    const char * logo;
    const char * home_desc;
    const char * ninja;
    const char * dojo;
    const char * new_game;
    const char * new_sign;
    const char * score;
    const char * lose_full[3];
    const char * lose_empty[3];
    const char * game_over;
    const char * flash;
    const char * shadow;
    const char * smoke;
} fruit_ninja_ui_assets_t;

static fruit_ninja_game_t g_game;
static fruit_ninja_ui_assets_t g_ui_assets;
static bool g_seeded_random = false;

static const fruit_ninja_fruit_def_t * g_home_menu_defs[3];

static const lv_point_t g_home_menu_positions[3] = {
    { 137, 333 },
    { 330, 322 },
    { 552, 367 },
};

static void build_home_menu_fruits(fruit_ninja_game_t * game);
static void start_running_timer_cb(lv_timer_t * timer);
static void restore_home_fruit_timer_cb(lv_timer_t * timer);

lv_obj_t * fruit_ninja_create_layer(lv_obj_t * parent)
{
    lv_obj_t * layer = lv_obj_create(parent);
    lv_obj_remove_style_all(layer);
    lv_obj_set_size(layer, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(layer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(layer, 0, 0);
    lv_obj_set_style_pad_all(layer, 0, 0);
    lv_obj_clear_flag(layer, LV_OBJ_FLAG_SCROLLABLE);
    return layer;
}

bool fruit_ninja_make_image_path(char * out, size_t out_size, const char * relative_path)
{
    return fruit_ninja_assets_build_image_path(out, out_size, relative_path);
}

lv_obj_t * fruit_ninja_create_file_image(lv_obj_t * parent, const char * relative_path)
{
    char path[512];
    lv_obj_t * image = lv_image_create(parent);

    if(fruit_ninja_make_image_path(path, sizeof(path), relative_path)) {
        lv_image_set_src(image, path);
    }
    return image;
}

void fruit_ninja_set_image_geometry(lv_obj_t * obj, int32_t x, int32_t y, int32_t w, int32_t h)
{
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
}

static void init_ui_asset_paths(void)
{
    g_ui_assets.background = "images/background.jpg";
    g_ui_assets.home_mask = "images/home-mask.png";
    g_ui_assets.logo = "images/logo.png";
    g_ui_assets.home_desc = "images/home-desc.png";
    g_ui_assets.ninja = "images/ninja.png";
    g_ui_assets.dojo = "images/dojo.png";
    g_ui_assets.new_game = "images/new-game.png";
    g_ui_assets.new_sign = "images/new.png";
    g_ui_assets.score = "images/score.png";
    g_ui_assets.lose_empty[0] = "images/x.png";
    g_ui_assets.lose_empty[1] = "images/xx.png";
    g_ui_assets.lose_empty[2] = "images/xxx.png";
    g_ui_assets.lose_full[0] = "images/xf.png";
    g_ui_assets.lose_full[1] = "images/xxf.png";
    g_ui_assets.lose_full[2] = "images/xxxf.png";
    g_ui_assets.game_over = "images/game-over.png";
    g_ui_assets.flash = "images/flash.png";
    g_ui_assets.shadow = "images/shadow.png";
    g_ui_assets.smoke = "images/smoke.png";
    g_home_menu_defs[0] = fruit_ninja_physics_get_fruit_def(0);
    g_home_menu_defs[1] = fruit_ninja_physics_get_fruit_def(1);
    g_home_menu_defs[2] = fruit_ninja_physics_get_fruit_def(5);
}

static void update_score_label(fruit_ninja_game_t * game)
{
    lv_label_set_text_fmt(game->score_label, "%u", game->score);
}

void fruit_ninja_scene_update_miss_icons(fruit_ninja_game_t * game)
{
    uint32_t i;
    char path[512];

    for(i = 0; i < 3; ++i) {
        if(fruit_ninja_make_image_path(path, sizeof(path), i < game->misses ? g_ui_assets.lose_full[i] : g_ui_assets.lose_empty[i])) {
            lv_image_set_src(game->miss_icons[i], path);
        }
    }
}

void fruit_ninja_hide_obj(lv_obj_t * obj)
{
    if(obj != NULL) {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

void fruit_ninja_show_obj(lv_obj_t * obj)
{
    if(obj != NULL) {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

void fruit_ninja_destroy_if_present(lv_obj_t ** obj)
{
    if(*obj != NULL) {
        lv_obj_delete(*obj);
        *obj = NULL;
    }
}

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

static void spawn_flash(fruit_ninja_game_t * game, float x, float y)
{
    lv_obj_t * flash = fruit_ninja_create_file_image(game->effect_layer, g_ui_assets.flash);
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

static void stage_home_object(lv_obj_t * obj, bool visible, lv_opa_t opa, int32_t y)
{
    if(obj == NULL) return;
    if(visible) fruit_ninja_show_obj(obj);
    else fruit_ninja_hide_obj(obj);
    lv_obj_set_style_opa(obj, opa, 0);
    lv_obj_set_y(obj, y);
}

static void clear_flash_if_needed(fruit_ninja_game_t * game)
{
    if(game->flash_overlay != NULL) {
        uint32_t age = game->flash_age_ms;
        uint32_t scale;
        uint32_t opa;

        if(age < FRUIT_NINJA_FLASH_MS / 2U) {
            scale = 32U + (age * (256U - 32U)) / (FRUIT_NINJA_FLASH_MS / 2U);
        }
        else if(age < FRUIT_NINJA_FLASH_MS) {
            uint32_t down_age = age - FRUIT_NINJA_FLASH_MS / 2U;
            scale = 256U - (down_age * (256U - 48U)) / (FRUIT_NINJA_FLASH_MS / 2U);
        }
        else {
            scale = 48U;
        }

        if(age >= FRUIT_NINJA_FLASH_MS) {
            lv_obj_delete(game->flash_overlay);
            game->flash_overlay = NULL;
            game->flash_age_ms = 0;
            return;
        }

        opa = (uint32_t)((FRUIT_NINJA_FLASH_MS - age) * LV_OPA_100 / FRUIT_NINJA_FLASH_MS);
        lv_image_set_scale(game->flash_overlay, scale);
        lv_obj_set_style_opa(game->flash_overlay, (lv_opa_t)opa, 0);
    }
}

static void update_score_pulse(fruit_ninja_game_t * game)
{
    uint32_t scale;

    if(game->score_image == NULL) return;

    if(game->score_pulse_ms == 0U) {
        lv_image_set_scale(game->score_image, 256);
        return;
    }

    if(game->score_pulse_ms >= FRUIT_NINJA_SCORE_PULSE_MS / 2U) {
        uint32_t age = FRUIT_NINJA_SCORE_PULSE_MS - game->score_pulse_ms;
        scale = 256U + (age * (307U - 256U)) / (FRUIT_NINJA_SCORE_PULSE_MS / 2U);
    }
    else {
        scale = 256U + (game->score_pulse_ms * (307U - 256U)) / (FRUIT_NINJA_SCORE_PULSE_MS / 2U);
    }

    lv_image_set_scale(game->score_image, scale);
    if(game->score_pulse_ms > FRUIT_NINJA_UPDATE_MS) {
        game->score_pulse_ms -= FRUIT_NINJA_UPDATE_MS;
    }
    else {
        game->score_pulse_ms = 0U;
        lv_image_set_scale(game->score_image, 256);
    }
}

static void enter_home(fruit_ninja_game_t * game)
{
    game->state = FRUIT_NINJA_STATE_HOME;
    game->state_elapsed_ms = 0;
    game->spawn_elapsed_ms = 0;
    game->spawn_interval_ms = 1000;
    clear_fruits(game);
    clear_fragments(game);
    clear_home_menu_fruits(game);
    build_home_menu_fruits(game);
    fruit_ninja_input_reset(game);
    game->flash_age_ms = 0;
    game->score_pulse_ms = 0;
    game->score = 0;
    game->misses = 0;
    game->volley_num = 2U;
    game->volley_multiple = 5U;
    update_score_label(game);
    fruit_ninja_scene_update_miss_icons(game);
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

static void enter_running(fruit_ninja_game_t * game)
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
    update_score_label(game);
    fruit_ninja_scene_update_miss_icons(game);
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

void fruit_ninja_scene_enter_game_over(fruit_ninja_game_t * game)
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

static void enter_exploding(fruit_ninja_game_t * game, float x, float y)
{
    uint32_t i;
    char path[512];

    game->state = FRUIT_NINJA_STATE_EXPLODING;
    game->state_elapsed_ms = 0;
    spawn_flash(game, x, y);
    if(game->smoke_overlay == NULL) {
        game->smoke_overlay = lv_image_create(game->overlay_layer);
        if(fruit_ninja_make_image_path(path, sizeof(path), g_ui_assets.smoke)) {
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

static void build_home_menu_fruits(fruit_ninja_game_t * game)
{
    uint32_t i;
    char path[512];

    for(i = 0; i < 3; ++i) {
        fruit_ninja_fruit_t * fruit = &game->home_menu_fruits[i];
        const fruit_ninja_fruit_def_t * def = g_home_menu_defs[i];

        memset(fruit, 0, sizeof(*fruit));
        fruit->active = true;
        fruit->def = def;
        fruit->radius = def->radius;
        fruit->x = (float)g_home_menu_positions[i].x;
        fruit->y = (float)g_home_menu_positions[i].y;
        fruit->angle = (float)def->base_rotation_deg;
        fruit->angular_velocity = (i == 2U) ? 0.0f : 2.0f + (float)i;

        fruit->shadow_image = lv_image_create(game->home_layer);
        if(fruit_ninja_make_image_path(path, sizeof(path), g_ui_assets.shadow)) {
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

static void start_running_timer_cb(lv_timer_t * timer)
{
    fruit_ninja_game_t * game = timer->user_data;
    lv_timer_delete(timer);
    enter_running(game);
}

static void restore_home_fruit_timer_cb(lv_timer_t * timer)
{
    fruit_ninja_fruit_t * fruit = timer->user_data;
    fruit->sliced = false;
    fruit_ninja_show_obj(fruit->whole_image);
    fruit_ninja_show_obj(fruit->shadow_image);
    lv_timer_delete(timer);
}

static void update_home_animation(fruit_ninja_game_t * game)
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

static void slice_fruit(fruit_ninja_game_t * game, fruit_ninja_fruit_t * fruit, float dx, float dy)
{
    fruit_ninja_fragment_t * left_fragment;
    fruit_ninja_fragment_t * right_fragment;
    float left_target_x;
    float right_target_x;
    float target_y = FRUIT_NINJA_JS_START_Y;
    float left_target_angle;
    float right_target_angle;
    float x = fruit->x - (float)fruit->def->width / 2.0f;
    float y = fruit->y - (float)fruit->def->height / 2.0f;

    if(fruit->sliced) return;

    if(fruit->def->is_bomb) {
        fruit_ninja_hide_obj(fruit->whole_image);
        fruit_ninja_hide_obj(fruit->shadow_image);
        enter_exploding(game, fruit->x, fruit->y);
        return;
    }

    fruit->sliced = true;
    fruit_ninja_hide_obj(fruit->whole_image);
    fruit_ninja_hide_obj(fruit->shadow_image);
    left_target_x = -(float)((rand() % 200) + 75);
    right_target_x = (float)(rand() % 275);
    left_target_angle = -(float)(rand() % 150) - 50.0f;
    right_target_angle = (float)(rand() % 150) + 50.0f;
    left_fragment = fruit_ninja_physics_spawn_fragment(game, fruit->def->split_left_rel_path, x - 6.0f, y, 0.0f, 0.0f, 0.0f, fruit->angle);
    right_fragment = fruit_ninja_physics_spawn_fragment(game, fruit->def->split_right_rel_path, x + 6.0f, y, 0.0f, 0.0f, 0.0f, fruit->angle);
    if(left_fragment != NULL) {
        left_fragment->target_x = left_target_x;
        left_fragment->target_y = target_y;
        left_fragment->start_angle = fruit->angle;
        left_fragment->target_angle = left_target_angle;
        left_fragment->life_ms = FRUIT_NINJA_DROP_TIME_MS;
        left_fragment->phase_elapsed_ms = 0U;
    }
    if(right_fragment != NULL) {
        right_fragment->target_x = right_target_x;
        right_fragment->target_y = target_y;
        right_fragment->start_angle = fruit->angle;
        right_fragment->target_angle = right_target_angle;
        right_fragment->life_ms = FRUIT_NINJA_DROP_TIME_MS;
        right_fragment->phase_elapsed_ms = 0U;
    }
    spawn_flash(game, fruit->x + dx * 0.1f, fruit->y + dy * 0.1f);
    game->score += 1;
    game->score_pulse_ms = FRUIT_NINJA_SCORE_PULSE_MS;
    if(game->score > game->volley_num * game->volley_multiple) {
        game->volley_num += 1U;
        game->volley_multiple += 50U;
    }
    update_score_label(game);
    if(game->audio_ready) {
        fruit_ninja_audio_play_slice();
    }
}

static void handle_home_menu_hits(fruit_ninja_game_t * game, fruit_ninja_segment_t segment)
{
    uint32_t i;

    if(!segment.valid || game->state != FRUIT_NINJA_STATE_HOME) return;

    for(i = 0; i < 3; ++i) {
        fruit_ninja_fruit_t * fruit = &game->home_menu_fruits[i];
        if(!fruit->active || fruit->sliced) continue;

        if(fruit_ninja_segment_hits_circle(segment.x1, segment.y1, segment.x2, segment.y2,
                                           fruit->x, fruit->y, fruit->radius)) {
            spawn_flash(game, fruit->x, fruit->y);
            if(i == 1U) {
                fruit_ninja_hide_obj(fruit->whole_image);
                fruit_ninja_hide_obj(fruit->shadow_image);
                fruit->sliced = true;
                if(game->audio_ready) fruit_ninja_audio_play_slice();
                lv_timer_create(start_running_timer_cb, FRUIT_NINJA_HOME_SLICE_FEEDBACK_MS, game);
                fruit_ninja_hide_obj(game->hint_label);
                return;
            }

            fruit->sliced = true;
            fruit_ninja_hide_obj(fruit->whole_image);
            fruit_ninja_hide_obj(fruit->shadow_image);
            if(game->audio_ready) {
                if(i == 2U) fruit_ninja_audio_play_boom();
                else fruit_ninja_audio_play_slice();
            }
            lv_timer_create(restore_home_fruit_timer_cb, FRUIT_NINJA_HOME_SLICE_FEEDBACK_MS, fruit);
            return;
        }
    }
}

static void handle_segment_hits(fruit_ninja_game_t * game, fruit_ninja_segment_t segment)
{
    uint32_t i;
    if(!segment.valid || game->state != FRUIT_NINJA_STATE_RUNNING) return;

    for(i = 0; i < FRUIT_NINJA_MAX_FRUITS; ++i) {
        fruit_ninja_fruit_t * fruit = &game->fruits[i];
        if(!fruit->active || fruit->sliced) continue;

        if(fruit_ninja_segment_hits_circle(segment.x1, segment.y1, segment.x2, segment.y2,
                                           fruit->x, fruit->y, fruit->radius)) {
            slice_fruit(game, fruit, segment.x2 - segment.x1, segment.y2 - segment.y1);
            if(game->state != FRUIT_NINJA_STATE_RUNNING) {
                return;
            }
        }
    }
}

static void input_event_cb(lv_event_t * e)
{
    fruit_ninja_game_t * game = lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t * indev = lv_indev_active();
    lv_point_t point;
    fruit_ninja_segment_t segment;

    if(indev == NULL) return;
    lv_indev_get_point(indev, &point);

    if(code == LV_EVENT_PRESSED) {
        if(game->state == FRUIT_NINJA_STATE_HOME) {
            fruit_ninja_input_begin(game, (float)point.x, (float)point.y);
            return;
        }
        if(game->state == FRUIT_NINJA_STATE_GAME_OVER) {
            enter_home(game);
            return;
        }
        if(game->state == FRUIT_NINJA_STATE_RUNNING) {
            fruit_ninja_input_begin(game, (float)point.x, (float)point.y);
        }
        return;
    }

    if(code == LV_EVENT_PRESSING && game->state == FRUIT_NINJA_STATE_RUNNING) {
        segment = fruit_ninja_input_push_point(game, (float)point.x, (float)point.y);
        handle_segment_hits(game, segment);
        return;
    }

    if(code == LV_EVENT_PRESSING && game->state == FRUIT_NINJA_STATE_HOME) {
        segment = fruit_ninja_input_push_point(game, (float)point.x, (float)point.y);
        handle_home_menu_hits(game, segment);
        return;
    }

    if((code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
       && (game->state == FRUIT_NINJA_STATE_RUNNING || game->state == FRUIT_NINJA_STATE_HOME)) {
        fruit_ninja_input_end(game);
    }
}

static void update_timer_cb(lv_timer_t * timer)
{
    fruit_ninja_game_t * game = timer->user_data;
    uint32_t spawn_count;
    uint32_t target_count;

    game->tick_count += FRUIT_NINJA_UPDATE_MS;
    game->state_elapsed_ms += FRUIT_NINJA_UPDATE_MS;
    if(game->flash_overlay != NULL) {
        game->flash_age_ms += FRUIT_NINJA_UPDATE_MS;
    }

    clear_flash_if_needed(game);
    update_score_pulse(game);
    fruit_ninja_input_tick(game, FRUIT_NINJA_UPDATE_MS);

    if(game->state == FRUIT_NINJA_STATE_HOME) {
        update_home_animation(game);
    }

    if(game->state == FRUIT_NINJA_STATE_RUNNING) {
        game->spawn_elapsed_ms += FRUIT_NINJA_UPDATE_MS;
        if(game->spawn_elapsed_ms >= game->spawn_interval_ms) {
            game->spawn_elapsed_ms = 0;
            target_count = fruit_ninja_physics_target_count(game);
            spawn_count = target_count > fruit_ninja_physics_active_fruits(game) ? target_count - fruit_ninja_physics_active_fruits(game) : 0U;
            while(spawn_count-- > 0U) {
                fruit_ninja_physics_spawn_one_fruit(game);
            }
        }
    }

    fruit_ninja_physics_update_fruits(game);
    fruit_ninja_physics_update_fragments(game);

    if(game->state == FRUIT_NINJA_STATE_EXPLODING) {
        if(game->white_flash_overlay != NULL) {
            uint32_t flash_opa = (game->state_elapsed_ms >= FRUIT_NINJA_EXPLODING_MS)
                               ? 0U
                               : (uint32_t)((FRUIT_NINJA_EXPLODING_MS - game->state_elapsed_ms) * LV_OPA_80 / FRUIT_NINJA_EXPLODING_MS);
            lv_obj_set_style_bg_opa(game->white_flash_overlay, (lv_opa_t)flash_opa, 0);
        }
        if(game->smoke_overlay != NULL) {
            uint32_t smoke_opa = (game->state_elapsed_ms >= 1200U)
                               ? 0U
                               : (uint32_t)((1200U - game->state_elapsed_ms) * LV_OPA_90 / 1200U);
            lv_obj_set_style_opa(game->smoke_overlay, (lv_opa_t)smoke_opa, 0);
        }
        if(game->state_elapsed_ms >= FRUIT_NINJA_EXPLODING_MS) {
            clear_explosion_overlays(game);
            fruit_ninja_scene_enter_game_over(game);
        }
    }
}

static void create_static_scene(fruit_ninja_game_t * game)
{
    game->screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(game->screen);
    lv_obj_set_style_bg_color(game->screen, lv_color_hex(FRUIT_NINJA_HOME_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(game->screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(game->screen, LV_OBJ_FLAG_SCROLLABLE);

    game->background = fruit_ninja_create_file_image(game->screen, g_ui_assets.background);
    fruit_ninja_set_image_geometry(game->background, 0, 0, FRUIT_NINJA_SCREEN_WIDTH, FRUIT_NINJA_SCREEN_HEIGHT);

    game->home_layer = fruit_ninja_create_layer(game->screen);
    game->fruit_layer = fruit_ninja_create_layer(game->screen);
    game->effect_layer = fruit_ninja_create_layer(game->screen);
    game->hud_layer = fruit_ninja_create_layer(game->screen);
    game->overlay_layer = fruit_ninja_create_layer(game->screen);
    game->input_layer = fruit_ninja_create_layer(game->screen);
    lv_obj_move_foreground(game->input_layer);

    game->home_mask_image = fruit_ninja_create_file_image(game->home_layer, g_ui_assets.home_mask);
    fruit_ninja_set_image_geometry(game->home_mask_image, 0, 0, 640, 183);

    game->logo_image = fruit_ninja_create_file_image(game->home_layer, g_ui_assets.logo);
    fruit_ninja_set_image_geometry(game->logo_image, 180, 32, 288, 135);

    game->home_desc_image = fruit_ninja_create_file_image(game->home_layer, g_ui_assets.home_desc);
    fruit_ninja_set_image_geometry(game->home_desc_image, 376, 206, 161, 91);

    game->ninja_image = fruit_ninja_create_file_image(game->home_layer, g_ui_assets.ninja);
    fruit_ninja_set_image_geometry(game->ninja_image, 200, 162, 244, 81);

    game->new_game_image = fruit_ninja_create_file_image(game->home_layer, g_ui_assets.new_game);
    fruit_ninja_set_image_geometry(game->new_game_image, 222, 280, 190, 112);

    game->dojo_image = fruit_ninja_create_file_image(game->home_layer, g_ui_assets.dojo);
    fruit_ninja_set_image_geometry(game->dojo_image, 44, 278, 141, 141);

    game->new_sign_image = fruit_ninja_create_file_image(game->home_layer, g_ui_assets.new_sign);
    fruit_ninja_set_image_geometry(game->new_sign_image, 388, 252, 70, 42);

    fruit_ninja_hide_obj(game->home_desc_image);
    fruit_ninja_hide_obj(game->dojo_image);
    fruit_ninja_hide_obj(game->new_game_image);
    fruit_ninja_hide_obj(game->new_sign_image);

    build_home_menu_fruits(game);

    game->hint_label = lv_label_create(game->home_layer);
    lv_label_set_text(game->hint_label, "Slice the middle fruit to start");
    lv_obj_set_style_text_color(game->hint_label, lv_color_hex(0xffffff), 0);
    lv_obj_align(game->hint_label, LV_ALIGN_BOTTOM_MID, 0, -20);

    game->score_image = fruit_ninja_create_file_image(game->hud_layer, g_ui_assets.score);
    fruit_ninja_set_image_geometry(game->score_image, 6, 8, 29, 31);

    game->score_label = lv_label_create(game->hud_layer);
    lv_label_set_text(game->score_label, "0");
    lv_obj_set_style_text_color(game->score_label, lv_color_hex(0xffec53), 0);
    lv_obj_set_style_text_font(game->score_label, LV_FONT_DEFAULT, 0);
    lv_obj_set_pos(game->score_label, 44, 18);

    for(uint32_t i = 0; i < 3; ++i) {
        game->miss_icons[i] = fruit_ninja_create_file_image(game->hud_layer, g_ui_assets.lose_empty[i]);
        fruit_ninja_set_image_geometry(game->miss_icons[i], 460 + (int32_t)i * 56, 12, 45, 45);
    }

    game->game_over_image = fruit_ninja_create_file_image(game->overlay_layer, g_ui_assets.game_over);
    fruit_ninja_set_image_geometry(game->game_over_image, 75, 188, 490, 85);
    fruit_ninja_hide_obj(game->game_over_image);

    game->restart_label = lv_label_create(game->overlay_layer);
    lv_label_set_text(game->restart_label, "Click to return home");
    lv_obj_set_style_text_color(game->restart_label, lv_color_hex(0xffffff), 0);
    lv_obj_align(game->restart_label, LV_ALIGN_CENTER, 0, 80);
    fruit_ninja_hide_obj(game->restart_label);

    lv_obj_add_flag(game->input_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(game->input_layer, input_event_cb, LV_EVENT_PRESSED, game);
    lv_obj_add_event_cb(game->input_layer, input_event_cb, LV_EVENT_PRESSING, game);
    lv_obj_add_event_cb(game->input_layer, input_event_cb, LV_EVENT_RELEASED, game);
    lv_obj_add_event_cb(game->input_layer, input_event_cb, LV_EVENT_PRESS_LOST, game);

    fruit_ninja_input_init(game);
}

void fruit_ninja_start(void)
{
    char cwd[512];

    memset(&g_game, 0, sizeof(g_game));
    g_game.screen_width = FRUIT_NINJA_SCREEN_WIDTH;
    g_game.screen_height = FRUIT_NINJA_SCREEN_HEIGHT;

    if(!g_seeded_random) {
        srand((unsigned int)time(NULL));
        g_seeded_random = true;
    }

    init_ui_asset_paths();
    if(getcwd(cwd, sizeof(cwd)) != NULL) {
        g_game.resources_ready = fruit_ninja_assets_init(cwd);
    }
    else {
        g_game.resources_ready = fruit_ninja_assets_init(".");
    }

    g_game.resources_ready = g_game.resources_ready && fruit_ninja_assets_validate_core_files();
    g_game.audio_ready = fruit_ninja_audio_init();

    create_static_scene(&g_game);
    g_game.update_timer = lv_timer_create(update_timer_cb, FRUIT_NINJA_UPDATE_MS, &g_game);

    if(!g_game.resources_ready) {
        LV_LOG_WARN("Fruit Ninja resources are incomplete");
    }

    lv_screen_load(g_game.screen);
    enter_home(&g_game);
}
