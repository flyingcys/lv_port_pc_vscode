#include "icon_replace_2_assets.h"

#include "../../lvgl/demos/render/assets/img_render_lvgl_logo_argb8888.c"
#include "../../lvgl/demos/render/assets/img_render_lvgl_logo_argb8888_premultiplied.c"
#include "../../lvgl/demos/render/assets/img_render_lvgl_logo_rgb888.c"
#include "../../lvgl/demos/render/assets/img_render_lvgl_logo_xrgb8888.c"
#include "../../lvgl/demos/render/assets/img_render_lvgl_logo_rgb565.c"
#include "../../lvgl/demos/render/assets/img_render_lvgl_logo_rgb565_swapped.c"
#include "../../lvgl/demos/render/assets/img_render_lvgl_logo_rgb565a8.c"
#include "../../lvgl/demos/render/assets/img_render_lvgl_logo_l8.c"
#include "../../lvgl/demos/render/assets/img_render_lvgl_logo_i1.c"
#include "../../lvgl/demos/widgets/assets/img_demo_widgets_needle.c"

const lv_image_dsc_t * const icon_replace_2_assets[] = {
    &img_render_lvgl_logo_argb8888,
    &img_render_lvgl_logo_argb8888_premultiplied,
    &img_render_lvgl_logo_rgb888,
    &img_render_lvgl_logo_xrgb8888,
    &img_render_lvgl_logo_rgb565,
    &img_render_lvgl_logo_rgb565_swapped,
    &img_render_lvgl_logo_rgb565a8,
    &img_render_lvgl_logo_l8,
    &img_render_lvgl_logo_i1,
    &img_demo_widgets_needle,
    &img_render_lvgl_logo_argb8888,
    &img_render_lvgl_logo_rgb888,
    &img_render_lvgl_logo_rgb565a8,
    &img_render_lvgl_logo_xrgb8888,
    &img_render_lvgl_logo_i1,
};

const uint32_t icon_replace_2_asset_count =
    (uint32_t)(sizeof(icon_replace_2_assets) / sizeof(icon_replace_2_assets[0]));
