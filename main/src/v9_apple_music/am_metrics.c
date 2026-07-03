#include "am_metrics.h"
#include "am_fonts.h"
static const am_metrics_t g[AM_TIER_COUNT] = {
 [AM_TIER_800]={"800x480",196,78,42, 20,16,18,18, 44,30, 244,180, false,
    &am_font_34,&am_font_24,&am_font_18,&am_font_14,&am_font_13,&am_font_11,&am_font_14},
 [AM_TIER_640]={"640x480",140,78,42, 18,14,16,16, 40,28, 200,160, false,
    &am_font_34,&am_font_24,&am_font_18,&am_font_14,&am_font_13,&am_font_11,&am_font_14},
 [AM_TIER_480]={"480x272",108,56,34, 12,10,12,14, 34,24, 0,0, true,
    &am_font_480_22,&am_font_480_18,&am_font_480_15,&am_font_480_12,&am_font_480_12,&am_font_480_10,&am_font_480_12},
};
static const am_metrics_t *cur = &g[AM_TIER_800];
const am_metrics_t *am_metrics(void){ return cur; }
void am_metrics_init(int w, int h){ (void)h;
    if(w>=800) cur=&g[AM_TIER_800];
    else if(w>=640) cur=&g[AM_TIER_640];
    else cur=&g[AM_TIER_480];
}
