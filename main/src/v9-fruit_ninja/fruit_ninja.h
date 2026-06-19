#ifndef FRUIT_NINJA_H
#define FRUIT_NINJA_H

#include <stdbool.h>

void fruit_ninja_start(void);

/* 暂停/恢复主循环(离开/返回桌面时调用)。未启动时为 no-op。*/
void fruit_ninja_set_active(bool active);

#endif
