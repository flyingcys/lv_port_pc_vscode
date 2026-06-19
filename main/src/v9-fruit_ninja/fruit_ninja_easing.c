#include "fruit_ninja_easing.h"

#include <math.h>

float fruit_ninja_ease_out_quad(float t) {
    float inv = 1.0f - t;
    return 1.0f - inv * inv;
}

float fruit_ninja_ease_in_quad(float t) {
    return t * t;
}

float fruit_ninja_ease_out_expo(float t) {
    if(t >= 1.0f) return 1.0f;
    return 1.0f - powf(2.0f, -10.0f * t);
}

float fruit_ninja_ease_out_back(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    float p = t - 1.0f;
    return 1.0f + c3 * p * p * p + c1 * p * p;
}
