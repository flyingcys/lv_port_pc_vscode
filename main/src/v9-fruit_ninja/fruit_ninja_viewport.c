#include "fruit_ninja_viewport.h"

#define FN_LOGIC_W 640.0f
#define FN_LOGIC_H 480.0f

static float g_scale = 1.0f;
static float g_off_x = 0.0f;
static float g_off_y = 0.0f;

void fruit_ninja_viewport_init(int phys_w, int phys_h) {
    float sx = (float)phys_w / FN_LOGIC_W;
    float sy = (float)phys_h / FN_LOGIC_H;
    g_scale = (sx < sy) ? sx : sy;
    g_off_x = ((float)phys_w - FN_LOGIC_W * g_scale) * 0.5f;
    g_off_y = ((float)phys_h - FN_LOGIC_H * g_scale) * 0.5f;
}

float fruit_ninja_viewport_x(float logic_x)   { return g_off_x + logic_x * g_scale; }
float fruit_ninja_viewport_y(float logic_y)   { return g_off_y + logic_y * g_scale; }
float fruit_ninja_viewport_len(float l)       { return l * g_scale; }
float fruit_ninja_viewport_scale(void)        { return g_scale; }
float fruit_ninja_viewport_to_logic_x(float px) { return (px - g_off_x) / g_scale; }
float fruit_ninja_viewport_to_logic_y(float py) { return (py - g_off_y) / g_scale; }
