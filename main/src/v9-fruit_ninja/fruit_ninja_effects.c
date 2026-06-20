#include "fruit_ninja_effects.h"
#include "fruit_ninja_easing.h"
#include "fruit_ninja_internal.h"
#include "fruit_ninja_viewport.h"

#include <math.h>
#include <stdlib.h>

/* 静态 canvas 缓冲区:按最大物理屏 800x480 ARGB8888 = 1.5 MB */
static uint8_t g_canvas_buf[800 * 480 * 4];

void fruit_ninja_effects_spawn_flash(fruit_ninja_game_t * game, float x, float y)
{
    lv_obj_t * flash = fruit_ninja_create_file_image(game->effect_layer, "images/flash.png");
    /* x/y 为逻辑切割点中心:set_image_geometry 以逻辑左上角(x-179,y-10)+尺寸 358x20
     * 推出逻辑中心(x,y)并映射到物理(pivot 默认中心,与下方显式 pivot 一致)。 */
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

        /* age >= 200ms: 销毁 flash 对象 */
        if(age >= FRUIT_NINJA_FLASH_MS) {
            lv_obj_delete(game->flash_overlay);
            game->flash_overlay = NULL;
            game->flash_age_ms = 0;
            return;
        }

        /* 对齐 JS: scale 1e-5→1→1e-5 over 200ms
         * 放大相(0–100ms): scale 0→256
         * 缩小相(100–200ms): scale 256→0
         * 注意先乘后除避免截断;scale=0 时图不可见,等价 JS 的 1e-5 */
        if(age < FRUIT_NINJA_FLASH_MS / 2U) {
            scale = age * 256U / (FRUIT_NINJA_FLASH_MS / 2U);
        }
        else {
            uint32_t down_age = age - FRUIT_NINJA_FLASH_MS / 2U;
            scale = 256U - down_age * 256U / (FRUIT_NINJA_FLASH_MS / 2U);
        }
        /* scale=0 时 LVGL 可能异常,用下限 1 */
        if(scale == 0U) scale = 1U;

        /* opa 同步线性淡出:(200 - age) * 255 / 200 */
        opa = (FRUIT_NINJA_FLASH_MS - age) * 255U / FRUIT_NINJA_FLASH_MS;

        /* 动画 scale 为相对原始位图(256=原始);叠加 viewport scale 以适配 letterbox。 */
        lv_image_set_scale(game->flash_overlay,
                           (uint16_t)((float)scale * fruit_ninja_viewport_scale()));
        lv_obj_set_style_opa(game->flash_overlay, (lv_opa_t)opa, 0);
    }
}

void fruit_ninja_effects_update_score_pulse(fruit_ninja_game_t * game)
{
    uint32_t scale;
    /* 脉冲 scale 相对原始位图(256=原始);叠加 viewport scale 适配 letterbox。 */
    float vp = fruit_ninja_viewport_scale();
    uint16_t base = (uint16_t)(256.0f * vp);

    if(game->score_image == NULL) return;

    if(game->score_pulse_ms == 0U) {
        lv_image_set_scale(game->score_image, base);
        return;
    }

    if(game->score_pulse_ms >= FRUIT_NINJA_SCORE_PULSE_MS / 2U) {
        uint32_t age = FRUIT_NINJA_SCORE_PULSE_MS - game->score_pulse_ms;
        scale = 256U + (age * (307U - 256U)) / (FRUIT_NINJA_SCORE_PULSE_MS / 2U);
    }
    else {
        scale = 256U + (game->score_pulse_ms * (307U - 256U)) / (FRUIT_NINJA_SCORE_PULSE_MS / 2U);
    }

    lv_image_set_scale(game->score_image, (uint16_t)((float)scale * vp));
    if(game->score_pulse_ms > FRUIT_NINJA_UPDATE_MS) {
        game->score_pulse_ms -= FRUIT_NINJA_UPDATE_MS;
    }
    else {
        game->score_pulse_ms = 0U;
        lv_image_set_scale(game->score_image, base);
    }
}

void fruit_ninja_effects_start_blast(fruit_ninja_game_t * game, float x, float y)
{
    game->blast.active = true;
    game->blast.cx     = x;
    game->blast.cy     = y;
    game->blast.age_ms = 0U;
}

void fruit_ninja_effects_clear_explosion(fruit_ninja_game_t * game)
{
    fruit_ninja_destroy_if_present(&game->smoke_overlay);
    fruit_ninja_destroy_if_present(&game->white_flash_overlay);
}

/* ---- Canvas 特效层 ---- */

float fruit_ninja_blade_width(uint32_t age_ms)
{
    if(age_ms >= 200U) return 0.0f;
    return 10.0f * (1.0f - (float)age_ms / 200.0f);
}

void fruit_ninja_effects_init_canvas(fruit_ninja_game_t * game, int phys_w, int phys_h)
{
    /* 缓冲区上限 800x480 ARGB8888;超界分辨率退化为只用左上角区域 */
    if(phys_w > 800) phys_w = 800;
    if(phys_h > 480) phys_h = 480;
    game->effect_canvas = lv_canvas_create(game->effect_layer);
    lv_canvas_set_buffer(game->effect_canvas, g_canvas_buf, phys_w, phys_h,
                         LV_COLOR_FORMAT_ARGB8888);
    lv_obj_set_pos(game->effect_canvas, 0, 0);
    lv_canvas_fill_bg(game->effect_canvas, lv_color_black(), LV_OPA_TRANSP);
}

void fruit_ninja_effects_spawn_juice(fruit_ninja_game_t * game, float x, float y,
                                     uint8_t r, uint8_t g, uint8_t b)
{
    int spawned = 0;
    int i;

    for(i = 0; i < FRUIT_NINJA_MAX_JUICE && spawned < 10; ++i) {
        if(game->juice[i].active) continue;
        fruit_ninja_juice_t * j = &game->juice[i];
        j->active    = true;
        j->origin_x  = x;
        j->origin_y  = y;
        /* 随机角度 0..2pi,距离 100..300 */
        j->angle_rad = ((float)(rand() % 360)) * 0.01745329f;
        j->distance  = 100.0f + (float)(rand() % 200);
        j->age_ms    = 0;
        j->life_ms   = 1500;
        j->cr        = r;
        j->cg        = g;
        j->cb        = b;
        ++spawned;
    }
}

void fruit_ninja_effects_push_blade(fruit_ninja_game_t * game,
                                    float sx, float sy, float ex, float ey)
{
    int i;
    int oldest;
    uint32_t max_age;

    for(i = 0; i < FRUIT_NINJA_MAX_BLADE_SEGMENTS; ++i) {
        if(!game->blades[i].active) {
            game->blades[i].active = true;
            game->blades[i].sx     = sx;
            game->blades[i].sy     = sy;
            game->blades[i].ex     = ex;
            game->blades[i].ey     = ey;
            game->blades[i].age_ms = 0U;
            return;
        }
    }
    /* 满了:覆盖最老的 */
    oldest = 0;
    max_age = 0U;
    for(i = 0; i < FRUIT_NINJA_MAX_BLADE_SEGMENTS; ++i) {
        if(game->blades[i].age_ms >= max_age) {
            max_age = game->blades[i].age_ms;
            oldest = i;
        }
    }
    game->blades[oldest].active = true;
    game->blades[oldest].sx     = sx;
    game->blades[oldest].sy     = sy;
    game->blades[oldest].ex     = ex;
    game->blades[oldest].ey     = ey;
    game->blades[oldest].age_ms = 0U;
}

void fruit_ninja_effects_render(fruit_ninja_game_t * game, uint32_t delta_ms)
{
    int i;
    lv_layer_t layer;
    lv_draw_line_dsc_t ld;

    lv_canvas_fill_bg(game->effect_canvas, lv_color_black(), LV_OPA_TRANSP);
    lv_canvas_init_layer(game->effect_canvas, &layer);

    /* 刀光:逐段按年龄衰减线宽 */
    lv_draw_line_dsc_init(&ld);
    ld.color       = lv_color_hex(0xcbd3db);
    ld.opa         = LV_OPA_90;
    ld.round_start = 1;
    ld.round_end   = 1;

    for(i = 0; i < FRUIT_NINJA_MAX_BLADE_SEGMENTS; ++i) {
        fruit_ninja_blade_seg_t * b = &game->blades[i];
        float w;
        if(!b->active) continue;
        b->age_ms += delta_ms;
        w = fruit_ninja_blade_width(b->age_ms);
        if(w <= 0.1f) { b->active = false; continue; }
        ld.width = (int32_t)fruit_ninja_viewport_len(w);
        if(ld.width < 1) ld.width = 1;
        ld.p1.x = (lv_value_precise_t)fruit_ninja_viewport_x(b->sx);
        ld.p1.y = (lv_value_precise_t)fruit_ninja_viewport_y(b->sy);
        ld.p2.x = (lv_value_precise_t)fruit_ninja_viewport_x(b->ex);
        ld.p2.y = (lv_value_precise_t)fruit_ninja_viewport_y(b->ey);
        lv_draw_line(&layer, &ld);
    }
    /* Phase 4 Task10: 汁液飞溅粒子绘制 */
    {
        lv_draw_arc_dsc_t ad;
        lv_draw_arc_dsc_init(&ad);
        ad.start_angle = 0;
        ad.end_angle   = 3600;
        ad.rounded     = 1;

        for(i = 0; i < FRUIT_NINJA_MAX_JUICE; ++i) {
            float p;
            float dist;
            float lx;
            float ly;
            float radius_f;
            fruit_ninja_juice_t * j = &game->juice[i];

            if(!j->active) continue;
            j->age_ms += delta_ms;
            if(j->age_ms >= j->life_ms) { j->active = false; continue; }

            p      = (float)j->age_ms / (float)j->life_ms;           /* 0..1 */
            dist   = fruit_ninja_ease_out_expo(p) * j->distance;     /* 径向展开 */
            lx     = j->origin_x + cosf(j->angle_rad) * dist;
            ly     = j->origin_y + sinf(j->angle_rad) * dist
                     + fruit_ninja_ease_out_quad(p) * 200.0f;        /* 重力下坠 */
            radius_f = fruit_ninja_viewport_len(10.0f * (1.0f - p)); /* 缩小至 0 */

            if(radius_f < 1.0f) continue;

            ad.center.x = (int32_t)fruit_ninja_viewport_x(lx);
            ad.center.y = (int32_t)fruit_ninja_viewport_y(ly);
            ad.radius   = (uint16_t)radius_f;
            ad.width    = (int32_t)radius_f;   /* width=radius -> 实心圆 */
            ad.color    = lv_color_make(j->cr, j->cg, j->cb);
            ad.opa      = (lv_opa_t)(LV_OPA_COVER * (1.0f - p)); /* 同步淡出 */
            lv_draw_arc(&layer, &ad);
        }
    }
    /* Phase 5 Task12: 炸弹 10 道放射爆炸光线 */
    if(game->blast.active) {
        lv_draw_triangle_dsc_t td;
        float cx;
        float cy;
        float ray_len;
        float half_w;
        int rays_shown;
        int r;

        game->blast.age_ms += delta_ms;
        rays_shown = (int)(game->blast.age_ms / 100U) + 1;
        if(rays_shown > 10) rays_shown = 10;

        lv_draw_triangle_dsc_init(&td);
        td.color = lv_color_hex(0xffffff);
        td.opa   = LV_OPA_COVER;

        cx       = fruit_ninja_viewport_x(game->blast.cx);
        cy       = fruit_ninja_viewport_y(game->blast.cy);
        ray_len  = fruit_ninja_viewport_len(400.0f);
        half_w   = fruit_ninja_viewport_len(14.0f);

        for(r = 0; r < rays_shown; ++r) {
            float a  = (float)r * (6.2831853f / 10.0f);
            float dx = cosf(a);
            float dy = sinf(a);
            float px = -dy;   /* 垂直方向 */
            float py = dx;
            td.p[0].x = cx + dx * ray_len;  /* 尖端 */
            td.p[0].y = cy + dy * ray_len;
            td.p[1].x = cx + px * half_w;
            td.p[1].y = cy + py * half_w;
            td.p[2].x = cx - px * half_w;
            td.p[2].y = cy - py * half_w;
            lv_draw_triangle(&layer, &td);
        }

        if(game->blast.age_ms >= FRUIT_NINJA_EXPLODING_MS) {
            game->blast.active = false;
        }
    }

    /* Phase 5 Task14: 炸弹持续火焰效果 */
    {
        lv_draw_arc_dsc_t fd;
        int fi;

        /* 累加时间,每 40ms 尝试生成一簇火苗 */
        game->flame_accum_ms += delta_ms;
        while(game->flame_accum_ms >= 40U) {
            game->flame_accum_ms -= 40U;
            /* 只在炸弹存活时、90% 概率生成 */
            if(game->bomb_alive && (rand() % 100) < 90) {
                /* 找空闲槽 */
                for(fi = 0; fi < FRUIT_NINJA_MAX_FLAMES; ++fi) {
                    if(!game->flames[fi].active) {
                        fruit_ninja_flame_t * fl = &game->flames[fi];
                        fl->active  = true;
                        fl->x       = game->bomb_x + (float)((rand() % 13) - 6);  /* ±6 逻辑抖动 */
                        fl->y       = game->bomb_y + (float)((rand() % 13) - 6);
                        fl->age_ms  = 0U;
                        fl->life_ms = 200U + (uint32_t)(rand() % 500);             /* 200–700ms */
                        fl->seed    = (float)(rand() % 1000) / 1000.0f;
                        break;
                    }
                }
            }
        }

        /* 更新并绘制每个火苗 */
        lv_draw_arc_dsc_init(&fd);
        fd.start_angle = 0;
        fd.end_angle   = 3600;
        fd.rounded     = 1;

        for(fi = 0; fi < FRUIT_NINJA_MAX_FLAMES; ++fi) {
            fruit_ninja_flame_t * fl = &game->flames[fi];
            float p;
            float lx;
            float ly;
            float radius_f;
            uint8_t cr, cg, cb;

            if(!fl->active) continue;
            fl->age_ms += delta_ms;
            if(fl->age_ms >= fl->life_ms) {
                fl->active = false;
                continue;
            }

            p  = (float)fl->age_ms / (float)fl->life_ms;  /* 0..1 */

            /* 上浮 + 轻微横向摆动 */
            ly = fl->y - p * 40.0f;
            lx = fl->x + sinf((p + fl->seed) * 6.2831853f) * 4.0f;

            /* 半径:由 ~8 缩到 0 */
            radius_f = fruit_ninja_viewport_len((1.0f - p) * 8.0f);
            if(radius_f < 1.0f) continue;

            /* 颜色:#fafad9(250,250,217) → #f0ef9c(240,239,156) */
            cr = (uint8_t)(250.0f + (240.0f - 250.0f) * p);
            cg = (uint8_t)(250.0f + (239.0f - 250.0f) * p);
            cb = (uint8_t)(217.0f + (156.0f - 217.0f) * p);

            fd.center.x = (int32_t)fruit_ninja_viewport_x(lx);
            fd.center.y = (int32_t)fruit_ninja_viewport_y(ly);
            fd.radius   = (uint16_t)radius_f;
            fd.width    = (int32_t)radius_f;   /* width=radius -> 实心圆 */
            fd.color    = lv_color_make(cr, cg, cb);
            fd.opa      = (lv_opa_t)(LV_OPA_COVER * (1.0f - p));  /* 淡出 */
            lv_draw_arc(&layer, &fd);
        }
    }

    lv_canvas_finish_layer(game->effect_canvas, &layer);
}
