#include "icon_replace_2_panels_geom.h"
#include <stdio.h>
#include <stdbool.h>

static int report_check(bool cond, const char * msg)
{
    if(cond) return 0;
    fprintf(stderr, "test_panels_geom: %s\n", msg);
    return 1;
}

int main(void)
{
    int failures = 0;
    const int32_t H = 300, SH = 480;

    /* drag_y: control 端点与中段 */
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_CONTROL, 0,    H, SH) == -H,        "control closed y=-H");
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_CONTROL, H,    H, SH) == 0,         "control open y=0");
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_CONTROL, H/2,  H, SH) == -H + H/2,  "control mid y");
    /* drag_y: control 越界钳制 */
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_CONTROL, -50,  H, SH) == -H,        "control reveal<0 clamps closed");
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_CONTROL, H+50, H, SH) == 0,         "control reveal>H clamps open");

    /* drag_y: notify 端点与越界 */
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_NOTIFY, 0,    H, SH) == SH,         "notify closed y=SH");
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_NOTIFY, H,    H, SH) == SH - H,     "notify open y=SH-H");
    failures += report_check(ir2_panel_drag_y(IR2_PANEL_NOTIFY, H+50, H, SH) == SH - H,     "notify reveal>H clamps open");

    /* snap: 位置阈值 50% */
    failures += report_check(ir2_panel_snap_open(H/2,     H, 0) == true,  "reveal=50% -> open");
    failures += report_check(ir2_panel_snap_open(H/2 - 1, H, 0) == false, "reveal<50% -> close");
    failures += report_check(ir2_panel_snap_open(0,       H, 0) == false, "reveal=0 -> close");

    /* snap: 甩动方向兜底（忽略位置） */
    failures += report_check(ir2_panel_snap_open(10,    H,  IR2_PANEL_FLICK_PX) == true,  "flick-open overrides position");
    failures += report_check(ir2_panel_snap_open(H-10,  H, -IR2_PANEL_FLICK_PX) == false, "flick-close overrides position");

    /* 退化保护 */
    failures += report_check(ir2_panel_snap_open(0, 0, 0) == false, "panel_h=0 -> close");

    if(failures != 0) {
        fprintf(stderr, "test_panels_geom: FAIL (%d checks failed)\n", failures);
        return 1;
    }
    printf("test_panels_geom: PASS\n");
    return 0;
}
