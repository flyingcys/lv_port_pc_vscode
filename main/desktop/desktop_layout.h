#ifndef DESKTOP_LAYOUT_H
#define DESKTOP_LAYOUT_H

/* 纯尺寸/计数常量（不依赖 lvgl）——供 page_config、单元测试等无 lvgl 依赖的单元复用。
 * lvgl 相关的逐档 metrics（屏宽高/字号/字体角色等）见 desktop_metrics.h。 */
#define DESKTOP_PAGE_COUNT 3
#define DESKTOP_GRID_COLS  5
#define DESKTOP_GRID_ROWS  2
#define DESKTOP_SLOT_COUNT (DESKTOP_GRID_COLS * DESKTOP_GRID_ROWS)   /* 10 */

#endif
