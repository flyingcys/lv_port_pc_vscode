#include <assert.h>
#include <string.h>
#include "../src/v9_apple_music/am_metrics.h"
int main(void){
    am_metrics_init(800,480); assert(am_metrics()->sidebar_w==164); assert(!am_metrics()->stack_content);
    am_metrics_init(640,480); assert(am_metrics()->sidebar_w==140); assert(am_metrics()->player_h==78);
    am_metrics_init(480,272); assert(am_metrics()->sidebar_w==108); assert(am_metrics()->stack_content);
    am_metrics_init(1024,600); assert(strcmp(am_metrics()->id,"800x480")==0);
    am_metrics_init(700,480);  assert(strcmp(am_metrics()->id,"640x480")==0);
    am_metrics_init(320,240);  assert(strcmp(am_metrics()->id,"480x272")==0);
    return 0;
}
