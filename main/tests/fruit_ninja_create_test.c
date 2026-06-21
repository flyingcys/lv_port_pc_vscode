#include <assert.h>

#include "lvgl/lvgl.h"
#include "../src/v9-fruit_ninja/fruit_ninja.h"

int main(void)
{
    lv_init();

    lv_display_t * disp = lv_display_create(800, 480);
    assert(disp != NULL);
    lv_display_set_default(disp);

    lv_obj_t * parent = lv_obj_create(lv_screen_active());
    lv_obj_set_size(parent, 800, 480);

    lv_obj_t * game = fruit_ninja_create(parent, 800, 480);
    assert(game != NULL);
    assert(lv_obj_get_parent(game) == parent);
    assert(lv_obj_get_width(game) == 800);
    assert(lv_obj_get_height(game) == 480);

    lv_obj_delete(parent);
    lv_display_delete(disp);
    return 0;
}
