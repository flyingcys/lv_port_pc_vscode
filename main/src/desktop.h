/* main/src/desktop.h
 * 应用桌面启动器：状态栏 + 图标网格，点击图标进入对应 app。
 * 音乐(apple_music)与切水果(fruit_ninja)为真实可用入口，其余为占位图标。
 */
#ifndef DESKTOP_H
#define DESKTOP_H

/* 在当前活动屏幕上构建桌面，并安装全局“返回桌面”按钮(lv_layer_top)。*/
void desktop_create(void);

#endif /* DESKTOP_H */
