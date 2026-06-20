/* main/src/v9-tetris/tetris.h
 * 竖屏俄罗斯方块(复刻 ds1.html)。在当前活动屏上构建,自适应竖屏分辨率
 * (480×800 / 480×640 / 272×480)。
 */
#ifndef TETRIS_H
#define TETRIS_H

#include <stdbool.h>

/* 在当前活动屏上构建并启动俄罗斯方块。*/
void tetris_start(void);

/* 暂停/恢复(离开/返回桌面时调用)。未启动时为 no-op。*/
void tetris_set_active(bool active);

#endif /* TETRIS_H */
