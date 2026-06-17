#ifndef ICON_REPLACE_2_LAYOUT_H
#define ICON_REPLACE_2_LAYOUT_H

/* 纯尺寸/计数常量（不依赖 lvgl）——供 page_config、单元测试等无 lvgl 依赖的单元复用。
 * lvgl 相关的逐档 metrics（屏宽高/字号/字体角色等）见 icon_replace_2_metrics.h。 */
#define IR2_PAGE_COUNT 3
#define IR2_GRID_COLS  5
#define IR2_GRID_ROWS  2
#define IR2_SLOT_COUNT (IR2_GRID_COLS * IR2_GRID_ROWS)   /* 10 */

#endif
