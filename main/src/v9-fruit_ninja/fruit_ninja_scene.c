#include "fruit_ninja.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "fruit_ninja_assets.h"
#include "fruit_ninja_audio.h"
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
    /* 入参为逻辑坐标(640x480 系),(x,y)=左上角,(w,h)=原始位图尺寸。
     * lv_image 经 set_scale 围绕 pivot(默认图像中心,原始像素)做缩放,而 obj 的 pos
     * 仅定位"原始位图左上角"。故要让"缩放后位图中心"落在逻辑中心的 viewport 映射处:
     *   pos = viewport(逻辑中心) - 原始位图半宽高。 */
    float cx = (float)x + (float)w * 0.5f;
    float cy = (float)y + (float)h * 0.5f;

    lv_image_set_scale(obj, (uint16_t)(fruit_ninja_viewport_scale() * 256.0f));
    lv_obj_set_size(obj, w, h);
    lv_obj_set_pos(obj,
                   (int32_t)lroundf(fruit_ninja_viewport_x(cx) - (float)w * 0.5f),
                   (int32_t)lroundf(fruit_ninja_viewport_y(cy) - (float)h * 0.5f));
}

void fruit_ninja_place_logic(lv_obj_t * obj, float lx, float ly, bool centered, bool scale_image)
{
    if(obj == NULL) return;

    if(scale_image) {
        lv_image_set_scale(obj, (uint16_t)(fruit_ninja_viewport_scale() * 256.0f));
    }

    if(centered) {
        /* lx/ly 为对象逻辑中心:pivot 默认在原始位图中心,故
         *   pos = viewport(中心) - 原始位图半宽高。 */
        float hw = (float)lv_obj_get_width(obj) * 0.5f;
        float hh = (float)lv_obj_get_height(obj) * 0.5f;
        lv_obj_set_pos(obj,
                       (int32_t)lroundf(fruit_ninja_viewport_x(lx) - hw),
                       (int32_t)lroundf(fruit_ninja_viewport_y(ly) - hh));
    }
    else {
        /* lx/ly 为对象左上角(用于 label 等非缩放对象)。 */
        lv_obj_set_pos(obj,
                       (int32_t)lroundf(fruit_ninja_viewport_x(lx)),
                       (int32_t)lroundf(fruit_ninja_viewport_y(ly)));
    }
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
    fruit_ninja_game_t * game = lv_timer_get_user_data(timer);
    uint32_t spawn_count;
    uint32_t target_count;

    game->tick_count += FRUIT_NINJA_UPDATE_MS;
    game->state_elapsed_ms += FRUIT_NINJA_UPDATE_MS;
    if(game->flash_overlay != NULL) {
        game->flash_age_ms += FRUIT_NINJA_UPDATE_MS;
    }

    fruit_ninja_effects_update_flash(game);
    fruit_ninja_effects_update_score_pulse(game);
    fruit_ninja_state_update_miss_pop(game);
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
            /* 全屏白闪:从 LV_OPA_COVER(255) 线性渐隐到 0,持续 4000ms。
             * 先乘后除避免整型截断。elapsed 超出时钳为 0。 */
            uint32_t flash_opa = (game->state_elapsed_ms >= FRUIT_NINJA_EXPLODING_MS)
                               ? 0U
                               : (uint32_t)((FRUIT_NINJA_EXPLODING_MS - game->state_elapsed_ms) * (uint32_t)LV_OPA_COVER / FRUIT_NINJA_EXPLODING_MS);
            lv_obj_set_style_bg_opa(game->white_flash_overlay, (lv_opa_t)flash_opa, 0);
        }
        if(game->smoke_overlay != NULL) {
            uint32_t smoke_opa = (game->state_elapsed_ms >= 1200U)
                               ? 0U
                               : (uint32_t)((1200U - game->state_elapsed_ms) * LV_OPA_90 / 1200U);
            lv_obj_set_style_opa(game->smoke_overlay, (lv_opa_t)smoke_opa, 0);
        }
        /* 背景抖动:每 50ms 随机偏移 ±6 物理像素(直接操作物理坐标,不经 viewport)。 */
        if(game->background != NULL) {
            game->shake_accum_ms += FRUIT_NINJA_UPDATE_MS;
            if(game->shake_accum_ms >= 50U) {
                game->shake_accum_ms = 0;
                lv_obj_set_pos(game->background,
                               (rand() % 13) - 6,
                               (rand() % 13) - 6);
            }
        }
        if(game->state_elapsed_ms >= FRUIT_NINJA_EXPLODING_MS) {
            fruit_ninja_effects_clear_explosion(game);
            fruit_ninja_state_enter_game_over(game);
        }
    }

    /* canvas 特效层收尾绘制(刀光逐段衰减) */
    if(game->effect_canvas != NULL) {
        fruit_ninja_effects_render(game, FRUIT_NINJA_UPDATE_MS);
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
    /* 背景铺满物理屏(允许非等比拉伸,不做 letterbox):obj 铺满物理屏,
     * inner_align=STRETCH 自动把位图缩放到 obj 尺寸(scale_x/scale_y 各自计算)。 */
    {
        lv_display_t * disp = lv_display_get_default();
        int32_t pw = lv_display_get_horizontal_resolution(disp);
        int32_t ph = lv_display_get_vertical_resolution(disp);
        lv_obj_set_pos(game->background, 0, 0);
        lv_obj_set_size(game->background, pw, ph);
        lv_image_set_inner_align(game->background, LV_IMAGE_ALIGN_STRETCH);
    }

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
    /* 逻辑底部居中(y≈460):用 viewport center + 等比纵向偏移,落在 letterbox 内容区内。 */
    lv_obj_align(game->hint_label, LV_ALIGN_CENTER, 0,
                 (int32_t)lroundf(fruit_ninja_viewport_len(220.0f)));

    game->score_image = fruit_ninja_create_file_image(game->hud_layer, g_ui_assets.score);
    fruit_ninja_set_image_geometry(game->score_image, 6, 8, 29, 31);

    game->score_label = lv_label_create(game->hud_layer);
    lv_label_set_text(game->score_label, "0");
    lv_obj_set_style_text_color(game->score_label, lv_color_hex(0xffec53), 0);
    lv_obj_set_style_text_font(game->score_label, LV_FONT_DEFAULT, 0);
    /* 逻辑 (44,18) -> 物理(label 不缩放位图,故 scale_image=false)。 */
    fruit_ninja_place_logic(game->score_label, 44.0f, 18.0f, false, false);

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
    /* 逻辑中心 +80:viewport center + 等比纵向偏移。 */
    lv_obj_align(game->restart_label, LV_ALIGN_CENTER, 0,
                 (int32_t)lroundf(fruit_ninja_viewport_len(80.0f)));
    fruit_ninja_hide_obj(game->restart_label);

    fruit_ninja_input_init(game);
    fruit_ninja_input_attach(game);

    /* 创建 canvas 特效层(effect_layer 已建,canvas 透明覆盖全物理屏) */
    {
        lv_display_t * disp = lv_display_get_default();
        int pw = (int)lv_display_get_horizontal_resolution(disp);
        int ph = (int)lv_display_get_vertical_resolution(disp);
        fruit_ninja_effects_init_canvas(game, pw, ph);
    }
}

void fruit_ninja_start(void)
{
    char cwd[512];

    memset(&g_game, 0, sizeof(g_game));
    /* 逻辑坐标系恒定 640x480;显示经 viewport 等比 letterbox 映射到物理屏。 */
    g_game.screen_width = FRUIT_NINJA_SCREEN_WIDTH;
    g_game.screen_height = FRUIT_NINJA_SCREEN_HEIGHT;
    /* miss 弹出动画初始无效 */
    g_game.miss_pop_index = -1;

    {
        lv_display_t * disp = lv_display_get_default();
        int phys_w = (int)lv_display_get_horizontal_resolution(disp);
        int phys_h = (int)lv_display_get_vertical_resolution(disp);
        fruit_ninja_viewport_init(phys_w, phys_h);
    }

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
