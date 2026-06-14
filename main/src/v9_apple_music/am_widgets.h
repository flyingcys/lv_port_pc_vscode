/* main/src/v9_apple_music/am_widgets.h */
#ifndef AM_WIDGETS_H
#define AM_WIDGETS_H
#include "lvgl/lvgl.h"

/* 竖向 2 段渐变(by-value 存色,无生命周期问题):top -> bottom */
void       am_fill_grad2(lv_obj_t *o, lv_color_t top, lv_color_t bottom);
/* 竖向 3 段渐变(hero/播放头像/色卡预览):持久 dsc,随对象删除自动释放 */
void       am_fill_grad3(lv_obj_t *o, lv_color_t c1, lv_color_t c2, lv_color_t c3);
/* 玻璃面板:radius22 + surface(白62%) + 1px 白边 + 柔阴影 */
lv_obj_t  *am_panel(lv_obj_t *parent);
/* 卡片:自定义 radius + 白底自定义 opa */
lv_obj_t  *am_card(lv_obj_t *parent, int radius, lv_opa_t bg_opa);
/* 胶囊标签 */
lv_obj_t  *am_pill(lv_obj_t *parent, const char *text, bool active);
/* 小标题(11px muted + 字距) */
lv_obj_t  *am_section_title(lv_obj_t *parent, const char *text);
/* 封面块:size×size,radius16,a->b 竖向渐变 + 两个波纹白圆 */
lv_obj_t  *am_cover(lv_obj_t *parent, int size, lv_color_t a, lv_color_t b);
/* 圆形动作按钮(accent@14% 底 + accent 图标) */
lv_obj_t  *am_item_action(lv_obj_t *parent, const char *glyph);
/* 侧栏导航项;active 时 accent 高亮;返回按钮对象(调用方挂事件) */
lv_obj_t  *am_nav_item(lv_obj_t *parent, const char *icon, const char *label, bool active);
/* 标签文本助手:设字号与颜色,返回 label */
lv_obj_t  *am_text(lv_obj_t *parent, const char *txt, const lv_font_t *font, lv_color_t color);

#endif
