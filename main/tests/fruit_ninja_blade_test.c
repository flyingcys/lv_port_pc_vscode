#include <assert.h>
#include <math.h>
#include "../src/v9-fruit_ninja/fruit_ninja_effects.h"
#define APPROX(a,b) (fabsf((a)-(b)) < 1e-3f)
int main(void) {
    assert(APPROX(fruit_ninja_blade_width(0), 10.0f));
    assert(APPROX(fruit_ninja_blade_width(100), 5.0f));
    assert(APPROX(fruit_ninja_blade_width(200), 0.0f));
    assert(APPROX(fruit_ninja_blade_width(300), 0.0f)); /* 钳位 */
    return 0;
}
