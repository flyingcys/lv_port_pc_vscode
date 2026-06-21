#include "fruit_ninja_app.h"

#include "desktop_app_launcher.h"
#include "desktop_metrics.h"
#include "../src/v9-fruit_ninja/fruit_ninja.h"

static lv_obj_t * fruit_ninja_builder(lv_obj_t * overlay, int32_t screen_w, int32_t screen_h)
{
    return fruit_ninja_create(overlay, screen_w, screen_h);
}

void fruit_ninja_app_launch(void)
{
    const desktop_metrics_t * m = desktop_metrics();
    desktop_app_launcher_open_with_close(fruit_ninja_builder, fruit_ninja_app_close, m->screen_w, m->screen_h);
}

void fruit_ninja_app_close(void)
{
    fruit_ninja_stop();
}
