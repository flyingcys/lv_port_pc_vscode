#ifndef FRUIT_NINJA_COLLISION_H
#define FRUIT_NINJA_COLLISION_H

#include <stdbool.h>

bool fruit_ninja_segment_hits_circle(float x1,
                                     float y1,
                                     float x2,
                                     float y2,
                                     float cx,
                                     float cy,
                                     float radius);

#endif
