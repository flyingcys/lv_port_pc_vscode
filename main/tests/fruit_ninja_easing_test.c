#include <assert.h>
#include <math.h>
#include "../src/v9-fruit_ninja/fruit_ninja_easing.h"

#define APPROX(a, b) (fabsf((a) - (b)) < 1e-3f)

static void test_quad_endpoints(void) {
    assert(APPROX(fruit_ninja_ease_out_quad(0.0f), 0.0f));
    assert(APPROX(fruit_ninja_ease_out_quad(1.0f), 1.0f));
    assert(APPROX(fruit_ninja_ease_out_quad(0.5f), 0.75f)); /* 1-(0.5)^2 */
    assert(APPROX(fruit_ninja_ease_in_quad(0.0f), 0.0f));
    assert(APPROX(fruit_ninja_ease_in_quad(1.0f), 1.0f));
    assert(APPROX(fruit_ninja_ease_in_quad(0.5f), 0.25f)); /* 0.5^2 */
}

static void test_expo_endpoints(void) {
    assert(APPROX(fruit_ninja_ease_out_expo(0.0f), 0.0f));
    assert(APPROX(fruit_ninja_ease_out_expo(1.0f), 1.0f));
    assert(fruit_ninja_ease_out_expo(0.5f) > 0.9f); /* 指数 out 前段快 */
}

static void test_back_overshoot(void) {
    assert(APPROX(fruit_ninja_ease_out_back(0.0f), 0.0f));
    assert(APPROX(fruit_ninja_ease_out_back(1.0f), 1.0f));
    /* back.co 在中后段会越过 1 再回落 */
    float peak = fruit_ninja_ease_out_back(0.6f);
    assert(peak > 1.0f);
}

int main(void) {
    test_quad_endpoints();
    test_expo_endpoints();
    test_back_overshoot();
    return 0;
}
