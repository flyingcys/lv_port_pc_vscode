#ifndef FRUIT_NINJA_MODEL_H
#define FRUIT_NINJA_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#include "lvgl/lvgl.h"

#define FRUIT_NINJA_MAX_FRUITS 16
#define FRUIT_NINJA_MAX_FRAGMENTS 32
#define FRUIT_NINJA_MAX_TRAIL_POINTS 24
#define FRUIT_NINJA_SEGMENT_MIN_DIST 12.0f
#define FRUIT_NINJA_MAX_BLADE_SEGMENTS 48
#define FRUIT_NINJA_MAX_JUICE 80

typedef struct {
    bool     active;
    float    sx, sy, ex, ey;   /* 逻辑坐标 */
    uint32_t age_ms;
} fruit_ninja_blade_seg_t;

typedef struct {
    bool     active;
    float    origin_x, origin_y;  /* 逻辑 */
    float    angle_rad, distance; /* 径向方向与最大距离 */
    uint32_t age_ms, life_ms;
    uint8_t  cr, cg, cb;          /* 果色 */
} fruit_ninja_juice_t;
#define FRUIT_NINJA_SCREEN_WIDTH 640
#define FRUIT_NINJA_SCREEN_HEIGHT 480

typedef enum {
    FRUIT_NINJA_STATE_HOME = 0,
    FRUIT_NINJA_STATE_RUNNING,
    FRUIT_NINJA_STATE_EXPLODING,
    FRUIT_NINJA_STATE_GAME_OVER,
} fruit_ninja_state_t;

typedef struct {
    const char * type_name;
    const char * whole_rel_path;
    const char * split_left_rel_path;
    const char * split_right_rel_path;
    uint16_t width;
    uint16_t height;
    float radius;
    int16_t base_rotation_deg;
    bool reverse_spin;
    bool is_bomb;
    bool has_juice;
    uint8_t juice_r, juice_g, juice_b;
} fruit_ninja_fruit_def_t;

typedef struct {
    bool active;
    bool sliced;
    bool counted_as_miss;
    bool has_been_visible;
    bool falling;
    const fruit_ninja_fruit_def_t * def;
    float x;
    float y;
    float vx;
    float vy;
    float gravity;
    float angle;
    float angular_velocity;
    float radius;
    float shot_out_start_x;
    float shot_out_start_y;
    float shot_out_end_x;
    float shot_out_end_y;
    float fall_target_x;
    float fall_target_y;
    uint32_t phase_elapsed_ms;
    lv_obj_t * whole_image;
    lv_obj_t * shadow_image;
    lv_obj_t * slice_flash;
} fruit_ninja_fruit_t;

static inline bool fruit_ninja_fruit_is_visible_on_screen(const fruit_ninja_fruit_t * fruit, uint32_t screen_height)
{
    float half_height;
    float top;
    float bottom;

    if(fruit == NULL || fruit->def == NULL) return false;

    half_height = (float)fruit->def->height * 0.5f;
    top = fruit->y - half_height;
    bottom = fruit->y + half_height;
    return bottom >= 0.0f && top <= (float)screen_height;
}

static inline bool fruit_ninja_fruit_has_left_bottom(const fruit_ninja_fruit_t * fruit, uint32_t screen_height)
{
    float half_height;
    float top;

    if(fruit == NULL || fruit->def == NULL) return false;

    half_height = (float)fruit->def->height * 0.5f;
    top = fruit->y - half_height;
    return top > (float)screen_height;
}

static inline bool fruit_ninja_fruit_should_count_miss(const fruit_ninja_fruit_t * fruit, uint32_t screen_height)
{
    if(fruit == NULL || fruit->def == NULL) return false;
    if(fruit->sliced || fruit->def->is_bomb || fruit->counted_as_miss) return false;
    if(!fruit->has_been_visible) return false;
    return fruit_ninja_fruit_has_left_bottom(fruit, screen_height);
}

typedef struct {
    bool active;
    float x;
    float y;
    float vx;
    float vy;
    float gravity;
    float angle;
    float angular_velocity;
    uint32_t life_ms;
    float start_x;
    float start_y;
    float target_x;
    float target_y;
    float start_angle;
    float target_angle;
    uint32_t phase_elapsed_ms;
    lv_obj_t * image;
} fruit_ninja_fragment_t;

typedef struct {
    bool active;
    bool pressing;
    bool has_last_point;
    uint16_t count;
    uint32_t fade_ms;
    float last_x;
    float last_y;
    lv_point_precise_t points[FRUIT_NINJA_MAX_TRAIL_POINTS];
    lv_obj_t * line;
} fruit_ninja_trail_t;

typedef struct {
    bool valid;
    float x1;
    float y1;
    float x2;
    float y2;
} fruit_ninja_segment_t;

typedef struct fruit_ninja_game {
    fruit_ninja_state_t state;
    bool resources_ready;
    bool audio_ready;
    /* 逻辑画面尺寸,恒为 640x480(FRUIT_NINJA_SCREEN_WIDTH/HEIGHT)。
     * 物理屏尺寸不存于此:显示经 fruit_ninja_viewport_* 等比 letterbox 映射,
     * 输入物理坐标经 viewport_to_logic_* 反映射后再喂给游戏逻辑。 */
    uint32_t screen_width;
    uint32_t screen_height;
    uint32_t tick_count;
    uint32_t state_elapsed_ms;
    uint32_t spawn_interval_ms;
    uint32_t spawn_elapsed_ms;
    uint32_t flash_age_ms;
    uint32_t score_pulse_ms;
    uint32_t score;
    uint32_t misses;
    uint32_t spawn_index;
    uint32_t volley_num;
    uint32_t volley_multiple;

    lv_obj_t * screen;
    lv_obj_t * background;
    lv_obj_t * home_layer;
    lv_obj_t * fruit_layer;
    lv_obj_t * effect_layer;
    lv_obj_t * effect_canvas;
    lv_obj_t * hud_layer;
    lv_obj_t * overlay_layer;
    lv_obj_t * input_layer;

    fruit_ninja_blade_seg_t blades[FRUIT_NINJA_MAX_BLADE_SEGMENTS];
    fruit_ninja_juice_t juice[FRUIT_NINJA_MAX_JUICE];

    lv_obj_t * logo_image;
    lv_obj_t * home_mask_image;
    lv_obj_t * home_desc_image;
    lv_obj_t * ninja_image;
    lv_obj_t * dojo_image;
    lv_obj_t * new_game_image;
    lv_obj_t * new_sign_image;
    lv_obj_t * score_image;
    lv_obj_t * score_label;
    lv_obj_t * miss_icons[3];
    lv_obj_t * game_over_image;
    lv_obj_t * restart_label;
    lv_obj_t * hint_label;
    lv_obj_t * flash_overlay;
    lv_obj_t * smoke_overlay;
    lv_obj_t * white_flash_overlay;

    lv_timer_t * update_timer;

    fruit_ninja_trail_t trail;
    fruit_ninja_fruit_t home_menu_fruits[3];
    fruit_ninja_fruit_t fruits[FRUIT_NINJA_MAX_FRUITS];
    fruit_ninja_fragment_t fragments[FRUIT_NINJA_MAX_FRAGMENTS];
} fruit_ninja_game_t;

void fruit_ninja_input_init(fruit_ninja_game_t * game);
void fruit_ninja_input_reset(fruit_ninja_game_t * game);
fruit_ninja_segment_t fruit_ninja_input_begin(fruit_ninja_game_t * game, float x, float y);
fruit_ninja_segment_t fruit_ninja_input_push_point(fruit_ninja_game_t * game, float x, float y);
void fruit_ninja_input_end(fruit_ninja_game_t * game);
void fruit_ninja_input_tick(fruit_ninja_game_t * game, uint32_t delta_ms);
void fruit_ninja_input_attach(fruit_ninja_game_t * game);

#endif
