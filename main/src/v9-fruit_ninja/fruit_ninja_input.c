#include "fruit_ninja_model.h"

#include <math.h>
#include <string.h>

#include "lvgl/lvgl.h"

#include "fruit_ninja_audio.h"
#include "fruit_ninja_collision.h"
#include "fruit_ninja_effects.h"
#include "fruit_ninja_internal.h"
#include "fruit_ninja_physics.h"
#include "fruit_ninja_state.h"
#include "fruit_ninja_viewport.h"

static fruit_ninja_segment_t empty_segment(void)
{
    fruit_ninja_segment_t segment;
    memset(&segment, 0, sizeof(segment));
    return segment;
}

static void update_trail_points(fruit_ninja_game_t * game)
{
    /* trail->points 存逻辑坐标,供碰撞检测使用。
     * 刀光绘制已改由 canvas 特效层(fruit_ninja_effects_push_blade/render)负责,
     * 此处不再操作 lv_line。 */
    (void)game;
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
        fruit->sliced = true;   /* 炸弹被切:置 sliced 防止 update_fruits 每帧重置 bomb_alive */
        fruit_ninja_hide_obj(fruit->whole_image);
        fruit_ninja_hide_obj(fruit->shadow_image);
        fruit_ninja_state_enter_exploding(game, fruit->x, fruit->y);
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
    fruit_ninja_effects_spawn_flash(game, fruit->x + dx * 0.1f, fruit->y + dy * 0.1f);
    if(fruit->def->has_juice) {
        fruit_ninja_effects_spawn_juice(game, fruit->x, fruit->y,
                                        fruit->def->juice_r, fruit->def->juice_g, fruit->def->juice_b);
    }
    game->score += 1;
    game->score_pulse_ms = FRUIT_NINJA_SCORE_PULSE_MS;
    if(game->score > game->volley_num * game->volley_multiple) {
        game->volley_num += 1U;
        game->volley_multiple += 50U;
    }
    fruit_ninja_state_update_score_label(game);
    if(game->audio_ready) {
        fruit_ninja_audio_play_slice();
    }
}

static void handle_home_menu_hits(fruit_ninja_game_t * game, fruit_ninja_segment_t segment)
{
    uint32_t i;

    if(!segment.valid || game->state != FRUIT_NINJA_STATE_HOME) return;

    /* 投喂刀光段(canvas 特效层渲染) */
    if(game->effect_canvas != NULL) {
        fruit_ninja_effects_push_blade(game, segment.x1, segment.y1, segment.x2, segment.y2);
    }

    for(i = 0; i < 3; ++i) {
        fruit_ninja_fruit_t * fruit = &game->home_menu_fruits[i];
        if(!fruit->active || fruit->sliced) continue;

        if(fruit_ninja_segment_hits_circle(segment.x1, segment.y1, segment.x2, segment.y2,
                                           fruit->x, fruit->y, fruit->radius)) {
            fruit_ninja_effects_spawn_flash(game, fruit->x, fruit->y);
            if(i == 1U) {
                fruit_ninja_hide_obj(fruit->whole_image);
                fruit_ninja_hide_obj(fruit->shadow_image);
                fruit->sliced = true;
                if(game->audio_ready) fruit_ninja_audio_play_slice();
                lv_timer_create(fruit_ninja_state_start_running_timer_cb, FRUIT_NINJA_HOME_SLICE_FEEDBACK_MS, game);
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
            lv_timer_create(fruit_ninja_state_restore_home_fruit_timer_cb, FRUIT_NINJA_HOME_SLICE_FEEDBACK_MS, fruit);
            return;
        }
    }
}

static void handle_segment_hits(fruit_ninja_game_t * game, fruit_ninja_segment_t segment)
{
    uint32_t i;
    if(!segment.valid || game->state != FRUIT_NINJA_STATE_RUNNING) return;

    /* 投喂刀光段(canvas 特效层渲染) */
    if(game->effect_canvas != NULL) {
        fruit_ninja_effects_push_blade(game, segment.x1, segment.y1, segment.x2, segment.y2);
    }

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

    /* indev 给出物理坐标:先反映射到逻辑系(640x480),再喂给轨迹/碰撞。 */
    float lx = fruit_ninja_viewport_to_logic_x((float)point.x);
    float ly = fruit_ninja_viewport_to_logic_y((float)point.y);

    if(code == LV_EVENT_PRESSED) {
        if(game->state == FRUIT_NINJA_STATE_HOME) {
            fruit_ninja_input_begin(game, lx, ly);
            return;
        }
        if(game->state == FRUIT_NINJA_STATE_GAME_OVER) {
            fruit_ninja_state_enter_home(game);
            return;
        }
        if(game->state == FRUIT_NINJA_STATE_RUNNING) {
            fruit_ninja_input_begin(game, lx, ly);
        }
        return;
    }

    if(code == LV_EVENT_PRESSING && game->state == FRUIT_NINJA_STATE_RUNNING) {
        segment = fruit_ninja_input_push_point(game, lx, ly);
        handle_segment_hits(game, segment);
        return;
    }

    if(code == LV_EVENT_PRESSING && game->state == FRUIT_NINJA_STATE_HOME) {
        segment = fruit_ninja_input_push_point(game, lx, ly);
        handle_home_menu_hits(game, segment);
        return;
    }

    if((code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
       && (game->state == FRUIT_NINJA_STATE_RUNNING || game->state == FRUIT_NINJA_STATE_HOME)) {
        fruit_ninja_input_end(game);
    }
}

void fruit_ninja_input_init(fruit_ninja_game_t * game)
{
    fruit_ninja_trail_t * trail = &game->trail;

    /* 刀光拖尾已由 canvas 特效层(fruit_ninja_effects_render)渲染,
     * lv_line 不再创建;逻辑点采集仍保留用于碰撞检测。 */
    memset(trail, 0, sizeof(*trail));
    trail->line = NULL;
}

void fruit_ninja_input_reset(fruit_ninja_game_t * game)
{
    fruit_ninja_trail_t * trail = &game->trail;

    trail->active = false;
    trail->pressing = false;
    trail->has_last_point = false;
    trail->count = 0;
    trail->fade_ms = 0;
    /* trail->line 不再使用(刀光由 canvas 特效层渲染) */
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

    /* 刀光拖尾渐隐已由 canvas 特效层(fruit_ninja_effects_render)逐段管理,
     * 此处仅维护 trail 活跃状态供碰撞逻辑使用。 */
    if(trail->pressing) return;

    if(!trail->active) return;

    trail->fade_ms += delta_ms;
    if(trail->fade_ms >= 180U) {
        fruit_ninja_input_reset(game);
    }
}

void fruit_ninja_input_attach(fruit_ninja_game_t * game)
{
    lv_obj_add_flag(game->input_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(game->input_layer, input_event_cb, LV_EVENT_PRESSED, game);
    lv_obj_add_event_cb(game->input_layer, input_event_cb, LV_EVENT_PRESSING, game);
    lv_obj_add_event_cb(game->input_layer, input_event_cb, LV_EVENT_RELEASED, game);
    lv_obj_add_event_cb(game->input_layer, input_event_cb, LV_EVENT_PRESS_LOST, game);
}
