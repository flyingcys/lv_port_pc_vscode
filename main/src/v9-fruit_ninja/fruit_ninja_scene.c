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
#include "fruit_ninja_effects.h"
#include "fruit_ninja_internal.h"
#include "fruit_ninja_physics.h"
#include "fruit_ninja_state.h"

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
    const char * shadow;
    const char * smoke;
} fruit_ninja_ui_assets_t;

static fruit_ninja_game_t g_game;
static fruit_ninja_ui_assets_t g_ui_assets;
static bool g_seeded_random = false;

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
    g_ui_assets.shadow = "images/shadow.png";
    g_ui_assets.smoke = "images/smoke.png";
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

    fruit_ninja_effects_update_flash(game);
    fruit_ninja_effects_update_score_pulse(game);
    fruit_ninja_input_tick(game, FRUIT_NINJA_UPDATE_MS);

    if(game->state == FRUIT_NINJA_STATE_HOME) {
        fruit_ninja_state_update_home_animation(game);
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
            fruit_ninja_effects_clear_explosion(game);
            fruit_ninja_state_enter_game_over(game);
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

    fruit_ninja_state_build_home_menu_fruits(game);

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

    fruit_ninja_input_init(game);
    fruit_ninja_input_attach(game);
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
    fruit_ninja_state_enter_home(&g_game);
}
