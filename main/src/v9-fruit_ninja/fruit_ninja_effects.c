#include "fruit_ninja_effects.h"
#include "fruit_ninja_internal.h"

void fruit_ninja_effects_spawn_flash(fruit_ninja_game_t * game, float x, float y)
{
    lv_obj_t * flash = fruit_ninja_create_file_image(game->effect_layer, "images/flash.png");
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

void fruit_ninja_effects_update_flash(fruit_ninja_game_t * game)
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

void fruit_ninja_effects_update_score_pulse(fruit_ninja_game_t * game)
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

void fruit_ninja_effects_clear_explosion(fruit_ninja_game_t * game)
{
    fruit_ninja_destroy_if_present(&game->smoke_overlay);
    fruit_ninja_destroy_if_present(&game->white_flash_overlay);
}
