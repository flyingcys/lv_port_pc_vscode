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
/* 按索引获取水果定义(供 home menu 使用,0-4 为普通水果,5 为炸弹) */
const fruit_ninja_fruit_def_t * fruit_ninja_physics_get_fruit_def(uint32_t index);
/* update_single_fruit_visual 供 scene.c 的 home 动画调用 */
void fruit_ninja_physics_update_single_fruit_visual(fruit_ninja_fruit_t * fruit);
#endif
