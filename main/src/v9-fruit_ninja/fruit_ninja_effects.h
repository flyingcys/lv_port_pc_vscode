#ifndef FRUIT_NINJA_EFFECTS_H
#define FRUIT_NINJA_EFFECTS_H
#include <stdint.h>
#include "fruit_ninja_model.h"
void  fruit_ninja_effects_spawn_flash(fruit_ninja_game_t * game, float x, float y);
void  fruit_ninja_effects_update_flash(fruit_ninja_game_t * game);
void  fruit_ninja_effects_update_score_pulse(fruit_ninja_game_t * game);
void  fruit_ninja_effects_clear_explosion(fruit_ninja_game_t * game);
void  fruit_ninja_effects_init_canvas(fruit_ninja_game_t * game, int phys_w, int phys_h);
void  fruit_ninja_effects_push_blade(fruit_ninja_game_t * game, float sx, float sy, float ex, float ey);
void  fruit_ninja_effects_render(fruit_ninja_game_t * game, uint32_t delta_ms);
void  fruit_ninja_effects_spawn_juice(fruit_ninja_game_t * game, float x, float y,
                                      uint8_t r, uint8_t g, uint8_t b);
float fruit_ninja_blade_width(uint32_t age_ms); /* 纯函数:10*(1-age/200),钳到0 */
#endif
