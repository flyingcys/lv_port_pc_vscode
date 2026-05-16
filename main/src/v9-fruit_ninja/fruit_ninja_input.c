#include "fruit_ninja_model.h"

#include <math.h>
#include <string.h>

#include "lvgl/lvgl.h"

static fruit_ninja_segment_t empty_segment(void)
{
    fruit_ninja_segment_t segment;
    memset(&segment, 0, sizeof(segment));
    return segment;
}

static void update_trail_points(fruit_ninja_game_t * game)
{
    uint16_t i;
    fruit_ninja_trail_t * trail = &game->trail;

    for(i = 0; i < trail->count; ++i) {
        trail->points[i].x = trail->points[i].x;
        trail->points[i].y = trail->points[i].y;
    }

    if(trail->count >= 2) {
        lv_line_set_points(trail->line, trail->points, trail->count);
        lv_obj_clear_flag(trail->line, LV_OBJ_FLAG_HIDDEN);
    }
    else {
        lv_obj_add_flag(trail->line, LV_OBJ_FLAG_HIDDEN);
    }
}

static fruit_ninja_segment_t push_internal(fruit_ninja_game_t * game, float x, float y, bool reset_chain)
{
    fruit_ninja_trail_t * trail = &game->trail;
    fruit_ninja_segment_t segment = empty_segment();
    float dx;
    float dy;

    if(reset_chain) {
        trail->count = 0;
        trail->has_last_point = false;
    }

    if(trail->count == FRUIT_NINJA_MAX_TRAIL_POINTS) {
        memmove(&trail->points[0], &trail->points[1], sizeof(trail->points[0]) * (FRUIT_NINJA_MAX_TRAIL_POINTS - 1));
        trail->count--;
    }

    if(trail->has_last_point) {
        dx = x - trail->last_x;
        dy = y - trail->last_y;
        if((dx * dx + dy * dy) < (FRUIT_NINJA_SEGMENT_MIN_DIST * FRUIT_NINJA_SEGMENT_MIN_DIST)) {
            return segment;
        }
        segment.valid = true;
        segment.x1 = trail->last_x;
        segment.y1 = trail->last_y;
        segment.x2 = x;
        segment.y2 = y;
    }

    trail->points[trail->count].x = x;
    trail->points[trail->count].y = y;
    trail->count++;
    trail->last_x = x;
    trail->last_y = y;
    trail->has_last_point = true;
    trail->active = true;
    trail->fade_ms = 0;

    update_trail_points(game);
    return segment;
}

void fruit_ninja_input_init(fruit_ninja_game_t * game)
{
    fruit_ninja_trail_t * trail = &game->trail;

    memset(trail, 0, sizeof(*trail));
    trail->line = lv_line_create(game->effect_layer);
    lv_obj_remove_style_all(trail->line);
    lv_obj_set_style_line_width(trail->line, 10, 0);
    lv_obj_set_style_line_color(trail->line, lv_color_hex(0xcbd3db), 0);
    lv_obj_set_style_line_opa(trail->line, LV_OPA_90, 0);
    lv_obj_add_flag(trail->line, LV_OBJ_FLAG_HIDDEN);
}

void fruit_ninja_input_reset(fruit_ninja_game_t * game)
{
    fruit_ninja_trail_t * trail = &game->trail;

    trail->active = false;
    trail->pressing = false;
    trail->has_last_point = false;
    trail->count = 0;
    trail->fade_ms = 0;
    lv_obj_add_flag(trail->line, LV_OBJ_FLAG_HIDDEN);
}

fruit_ninja_segment_t fruit_ninja_input_begin(fruit_ninja_game_t * game, float x, float y)
{
    game->trail.pressing = true;
    return push_internal(game, x, y, true);
}

fruit_ninja_segment_t fruit_ninja_input_push_point(fruit_ninja_game_t * game, float x, float y)
{
    if(!game->trail.pressing) {
        return fruit_ninja_input_begin(game, x, y);
    }

    return push_internal(game, x, y, false);
}

void fruit_ninja_input_end(fruit_ninja_game_t * game)
{
    game->trail.pressing = false;
    game->trail.has_last_point = false;
}

void fruit_ninja_input_tick(fruit_ninja_game_t * game, uint32_t delta_ms)
{
    fruit_ninja_trail_t * trail = &game->trail;
    uint32_t new_opa;

    if(trail->pressing) {
        lv_obj_set_style_line_opa(trail->line, LV_OPA_90, 0);
        return;
    }

    if(!trail->active) {
        return;
    }

    trail->fade_ms += delta_ms;
    if(trail->fade_ms >= 180) {
        fruit_ninja_input_reset(game);
        return;
    }

    new_opa = (uint32_t)((180U - trail->fade_ms) * LV_OPA_90 / 180U);
    lv_obj_set_style_line_opa(trail->line, (lv_opa_t)new_opa, 0);
}
