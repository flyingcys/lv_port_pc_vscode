#ifndef FRUIT_NINJA_VIEWPORT_H
#define FRUIT_NINJA_VIEWPORT_H

/* 逻辑坐标系固定 640x480;按物理屏等比缩放居中(letterbox) */
void  fruit_ninja_viewport_init(int phys_w, int phys_h);
float fruit_ninja_viewport_x(float logic_x);        /* 逻辑X -> 物理X */
float fruit_ninja_viewport_y(float logic_y);        /* 逻辑Y -> 物理Y */
float fruit_ninja_viewport_len(float logic_len);    /* 标量缩放(线宽/半径) */
float fruit_ninja_viewport_scale(void);             /* 当前 scale */
float fruit_ninja_viewport_to_logic_x(float phys_x);/* 物理X -> 逻辑X(输入命中) */
float fruit_ninja_viewport_to_logic_y(float phys_y);

#endif
