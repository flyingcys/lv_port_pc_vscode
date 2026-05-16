#include <assert.h>

#include "../src/v9-fruit_ninja/fruit_ninja_model.h"

static const fruit_ninja_fruit_def_t g_test_fruit_def = {
    "apple", "images/fruit/apple.png", "images/fruit/apple-1.png", "images/fruit/apple-2.png",
    66, 66, 31.0f, 0, false, false
};

static const fruit_ninja_fruit_def_t g_test_bomb_def = {
    "boom", "images/fruit/boom.png", NULL, NULL,
    66, 68, 26.0f, 0, false, true
};

static void test_spawn_below_screen_is_not_visible(void)
{
    fruit_ninja_fruit_t fruit = {0};
    fruit.def = &g_test_fruit_def;
    fruit.y = 560.0f;

    assert(!fruit_ninja_fruit_is_visible_on_screen(&fruit, FRUIT_NINJA_SCREEN_HEIGHT));
    assert(!fruit_ninja_fruit_should_count_miss(&fruit, FRUIT_NINJA_SCREEN_HEIGHT));
}

static void test_visible_then_leave_bottom_counts_as_miss(void)
{
    fruit_ninja_fruit_t fruit = {0};
    fruit.def = &g_test_fruit_def;
    fruit.y = 240.0f;

    assert(fruit_ninja_fruit_is_visible_on_screen(&fruit, FRUIT_NINJA_SCREEN_HEIGHT));
    fruit.has_been_visible = true;
    fruit.y = 520.0f;
    assert(fruit_ninja_fruit_has_left_bottom(&fruit, FRUIT_NINJA_SCREEN_HEIGHT));
    assert(fruit_ninja_fruit_should_count_miss(&fruit, FRUIT_NINJA_SCREEN_HEIGHT));
}

static void test_bomb_never_counts_as_miss(void)
{
    fruit_ninja_fruit_t fruit = {0};
    fruit.def = &g_test_bomb_def;
    fruit.has_been_visible = true;
    fruit.y = 540.0f;

    assert(!fruit_ninja_fruit_should_count_miss(&fruit, FRUIT_NINJA_SCREEN_HEIGHT));
}

int main(void)
{
    test_spawn_below_screen_is_not_visible();
    test_visible_then_leave_bottom_counts_as_miss();
    test_bomb_never_counts_as_miss();
    return 0;
}
