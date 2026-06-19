#include <assert.h>
#include <math.h>
#include "../src/v9-fruit_ninja/fruit_ninja_viewport.h"

#define APPROX(a, b) (fabsf((a) - (b)) < 0.05f)

static void test_800x480(void) {
    fruit_ninja_viewport_init(800, 480);
    assert(APPROX(fruit_ninja_viewport_scale(), 1.0f));
    assert(APPROX(fruit_ninja_viewport_x(0.0f), 80.0f));     /* 左留边 80 */
    assert(APPROX(fruit_ninja_viewport_x(320.0f), 400.0f));  /* 逻辑中心 -> 屏幕中心 */
    assert(APPROX(fruit_ninja_viewport_y(240.0f), 240.0f));
    assert(APPROX(fruit_ninja_viewport_len(10.0f), 10.0f));
}

static void test_640x480(void) {
    fruit_ninja_viewport_init(640, 480);
    assert(APPROX(fruit_ninja_viewport_scale(), 1.0f));
    assert(APPROX(fruit_ninja_viewport_x(320.0f), 320.0f));
}

static void test_480x272(void) {
    fruit_ninja_viewport_init(480, 272);
    assert(APPROX(fruit_ninja_viewport_scale(), 272.0f / 480.0f)); /* 0.5667 */
    assert(APPROX(fruit_ninja_viewport_x(320.0f), 240.0f));  /* 屏幕水平中心 */
    assert(APPROX(fruit_ninja_viewport_y(240.0f), 136.0f));  /* 屏幕垂直中心 */
}

static void test_roundtrip(void) {
    fruit_ninja_viewport_init(480, 272);
    float px = fruit_ninja_viewport_x(321.0f);
    assert(APPROX(fruit_ninja_viewport_to_logic_x(px), 321.0f));
    float py = fruit_ninja_viewport_y(123.0f);
    assert(APPROX(fruit_ninja_viewport_to_logic_y(py), 123.0f));
}

int main(void) {
    test_800x480();
    test_640x480();
    test_480x272();
    test_roundtrip();
    return 0;
}
