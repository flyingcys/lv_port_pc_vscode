#include <assert.h>

#include "../src/v9-fruit_ninja/fruit_ninja_collision.h"

static void test_segment_hits_circle_through_center(void)
{
    assert(fruit_ninja_segment_hits_circle(0.0f, 0.0f, 10.0f, 0.0f, 5.0f, 0.0f, 2.0f));
}

static void test_segment_misses_distant_circle(void)
{
    assert(!fruit_ninja_segment_hits_circle(0.0f, 0.0f, 10.0f, 0.0f, 5.0f, 10.0f, 2.0f));
}

static void test_segment_touches_circle_tangentially(void)
{
    assert(fruit_ninja_segment_hits_circle(0.0f, 0.0f, 10.0f, 0.0f, 10.0f, 2.0f, 2.0f));
}

static void test_degenerate_segment_uses_endpoint_distance(void)
{
    assert(fruit_ninja_segment_hits_circle(3.0f, 4.0f, 3.0f, 4.0f, 0.0f, 0.0f, 5.0f));
    assert(!fruit_ninja_segment_hits_circle(6.0f, 8.0f, 6.0f, 8.0f, 0.0f, 0.0f, 5.0f));
}

int main(void)
{
    test_segment_hits_circle_through_center();
    test_segment_misses_distant_circle();
    test_segment_touches_circle_tangentially();
    test_degenerate_segment_uses_endpoint_distance();
    return 0;
}
