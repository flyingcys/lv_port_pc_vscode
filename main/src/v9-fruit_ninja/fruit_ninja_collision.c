#include "fruit_ninja_collision.h"

bool fruit_ninja_segment_hits_circle(float x1,
                                     float y1,
                                     float x2,
                                     float y2,
                                     float cx,
                                     float cy,
                                     float radius)
{
    const float dx = x2 - x1;
    const float dy = y2 - y1;
    const float len2 = (dx * dx) + (dy * dy);
    float t = 0.0f;
    float px;
    float py;
    float ddx;
    float ddy;

    if(radius < 0.0f) {
        return false;
    }

    if(len2 > 0.0f) {
        t = (((cx - x1) * dx) + ((cy - y1) * dy)) / len2;
        if(t < 0.0f) {
            t = 0.0f;
        }
        else if(t > 1.0f) {
            t = 1.0f;
        }
    }

    px = x1 + (t * dx);
    py = y1 + (t * dy);
    ddx = px - cx;
    ddy = py - cy;

    return (ddx * ddx) + (ddy * ddy) <= (radius * radius);
}
