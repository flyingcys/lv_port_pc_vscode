/* Stub implementations for symbols referenced by fruit_ninja_effects.c
   but defined in fruit_ninja_scene.c.  The blade_width test only exercises
   the pure-function fruit_ninja_blade_width, so these stubs are never called. */
#include "lvgl/lvgl.h"
#include "../src/v9-fruit_ninja/fruit_ninja_model.h"

lv_obj_t * fruit_ninja_create_file_image(lv_obj_t * parent, const char * relative_path)
{
    (void)parent; (void)relative_path;
    return NULL;
}

void fruit_ninja_set_image_geometry(lv_obj_t * obj, int32_t x, int32_t y, int32_t w, int32_t h)
{
    (void)obj; (void)x; (void)y; (void)w; (void)h;
}

void fruit_ninja_destroy_if_present(lv_obj_t ** obj)
{
    (void)obj;
}
