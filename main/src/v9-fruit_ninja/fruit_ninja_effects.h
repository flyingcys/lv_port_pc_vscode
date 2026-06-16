#ifndef FRUIT_NINJA_EFFECTS_H
#define FRUIT_NINJA_EFFECTS_H
#include "fruit_ninja_model.h"
void fruit_ninja_effects_spawn_flash(fruit_ninja_game_t * game, float x, float y);
void fruit_ninja_effects_update_flash(fruit_ninja_game_t * game);
void fruit_ninja_effects_update_score_pulse(fruit_ninja_game_t * game);
void fruit_ninja_effects_clear_explosion(fruit_ninja_game_t * game);
#endif
