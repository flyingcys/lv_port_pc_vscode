#ifndef FRUIT_NINJA_STATE_H
#define FRUIT_NINJA_STATE_H

#include "fruit_ninja_model.h"

void fruit_ninja_state_enter_home(fruit_ninja_game_t * game);
void fruit_ninja_state_enter_running(fruit_ninja_game_t * game);
void fruit_ninja_state_enter_game_over(fruit_ninja_game_t * game);
void fruit_ninja_state_enter_exploding(fruit_ninja_game_t * game, float x, float y);
void fruit_ninja_state_update_score_label(fruit_ninja_game_t * game);
void fruit_ninja_state_update_miss_icons(fruit_ninja_game_t * game);
void fruit_ninja_state_update_home_animation(fruit_ninja_game_t * game);
void fruit_ninja_state_build_home_menu_fruits(fruit_ninja_game_t * game);
void fruit_ninja_state_start_running_timer_cb(lv_timer_t * timer);
void fruit_ninja_state_restore_home_fruit_timer_cb(lv_timer_t * timer);

#endif
