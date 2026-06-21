#include "calc_app.h"
#include "calc.h"
#include "desktop_app_launcher.h"
#include "desktop_metrics.h"

static lv_obj_t * calc_builder(lv_obj_t * overlay, int32_t screen_w, int32_t screen_h)
{
    lv_obj_t * card = calc_create(overlay, screen_w, screen_h);
    if(card != NULL) {
        calc_set_close_cb(card, desktop_app_launcher_close);
    }
    return card;
}

void calc_app_launch(void)
{
    const desktop_metrics_t * m = desktop_metrics();
    desktop_app_launcher_open(calc_builder, m->screen_w, m->screen_h);
}
