/* main/src/v9_apple_music/am_widgets.c */
#include "am_widgets.h"
#include "am_theme.h"
#include "am_fonts.h"
#include "am_metrics.h"

/* 2 段:by-value 样式 API,无指针生命周期问题 */
void am_fill_grad2(lv_obj_t *o, lv_color_t top, lv_color_t bottom)
{
    lv_obj_set_style_bg_color(o, top, 0);
    lv_obj_set_style_bg_grad_color(o, bottom, 0);
    lv_obj_set_style_bg_grad_dir(o, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
}

/* 3 段:lv_grad_dsc_t 按指针被样式引用,必须持久;每对象 malloc 一份,删除时释放 */
static void am_free_grad_cb(lv_event_t *e){ lv_free(lv_event_get_user_data(e)); }
void am_fill_grad3(lv_obj_t *o, lv_color_t c1, lv_color_t c2, lv_color_t c3)
{
#if LV_GRADIENT_MAX_STOPS < 3
    LV_UNUSED(c2);
    am_fill_grad2(o, c1, c3);
#else
    lv_grad_dsc_t *d = lv_malloc(sizeof(lv_grad_dsc_t));
    if(!d) { am_fill_grad2(o, c1, c3); return; }   /* heap exhausted -> graceful 2-stop fallback */
    lv_memzero(d, sizeof(*d));
    d->dir = LV_GRAD_DIR_VER; d->stops_count = 3;
    d->stops[0].color = c1; d->stops[0].frac = 0;   d->stops[0].opa = LV_OPA_COVER;
    d->stops[1].color = c2; d->stops[1].frac = 108; d->stops[1].opa = LV_OPA_COVER;  /* ~42% */
    d->stops[2].color = c3; d->stops[2].frac = 255; d->stops[2].opa = LV_OPA_COVER;
    lv_obj_set_style_bg_grad(o, d, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(o, am_free_grad_cb, LV_EVENT_DELETE, d);
#endif
}

/* 装饰圆辅助:创建纯白半透明圆形,清除交互标志 */
static lv_obj_t *am_deco_circle(lv_obj_t *parent, int sz, lv_align_t align, int x, int y)
{
    lv_obj_t *w = lv_obj_create(parent);
    lv_obj_remove_style_all(w);
    lv_obj_set_size(w, sz, sz);
    lv_obj_set_style_radius(w, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(w, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(w, 148, 0);
    lv_obj_clear_flag(w, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(w, align, x, y);
    return w;
}

lv_obj_t *am_cover(lv_obj_t *parent, int size, lv_color_t a, lv_color_t b)
{
    lv_obj_t *cov = lv_obj_create(parent);
    lv_obj_remove_style_all(cov);
    lv_obj_set_size(cov, size, size);
    lv_obj_set_style_radius(cov, 16, 0);
    lv_obj_set_style_clip_corner(cov, true, 0);
    am_fill_grad2(cov, a, b);                        /* tile_a -> tile_c */
    /* ::before 大圆 左下 */
    am_deco_circle(cov, 54, LV_ALIGN_BOTTOM_LEFT, -8, 24);
    /* ::after 小圆 右上 */
    am_deco_circle(cov, 24, LV_ALIGN_TOP_RIGHT, 8, 8);
    return cov;
}

lv_obj_t *am_nav_item(lv_obj_t *parent, const char *icon, const char *label, bool active)
{
    const am_theme_t *t = am_theme_get(am_theme_current());
    const am_metrics_t *m = am_metrics();
    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, LV_PCT(100), m->nav_h);
    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(btn, 12, 0);
    lv_obj_set_style_pad_column(btn, 10, 0);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    if(active){
        lv_obj_set_style_bg_color(btn, t->accent, 0);
        lv_obj_set_style_bg_opa(btn, AM_OPA_ACCENT_12, 0);
    }
    lv_color_t label_color = active ? t->accent : AM_MUTED_STRONG;
    lv_obj_t *ic = lv_obj_create(btn); lv_obj_remove_style_all(ic);
    lv_obj_set_size(ic, 28, 28); lv_obj_set_style_radius(ic, 12, 0);
    lv_obj_set_style_bg_color(ic, AM_WHITE, 0); lv_obj_set_style_bg_opa(ic, 153, 0);
    lv_obj_clear_flag(ic, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *icl = lv_label_create(ic); lv_label_set_text(icl, icon);
    lv_obj_center(icl); lv_obj_set_style_text_font(icl, m->f_icon, 0);
    lv_obj_set_style_text_color(icl, label_color, 0);
    lv_obj_t *lbl = lv_label_create(btn); lv_label_set_text(lbl, label);
    lv_obj_set_style_text_font(lbl, m->f_strong, 0);
    lv_obj_set_style_text_color(lbl, label_color, 0);
    return btn;
}

lv_obj_t *am_text(lv_obj_t *parent, const char *txt, const lv_font_t *font, lv_color_t color)
{
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, txt);
    lv_obj_set_style_text_font(lbl, font, 0);
    lv_obj_set_style_text_color(lbl, color, 0);
    return lbl;
}

lv_obj_t *am_panel(lv_obj_t *parent)
{
    lv_obj_t *p = lv_obj_create(parent);
    lv_obj_remove_style_all(p);
    lv_obj_set_style_radius(p, 22, 0);
    lv_obj_set_style_bg_color(p, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(p, AM_OPA_SURFACE, 0);
    lv_obj_set_style_border_width(p, 1, 0);
    lv_obj_set_style_border_color(p, AM_WHITE, 0);
    lv_obj_set_style_border_opa(p, AM_OPA_BORDER, 0);
    lv_obj_set_style_shadow_width(p, 30, 0);
    lv_obj_set_style_shadow_offset_y(p, 12, 0);
    lv_obj_set_style_shadow_color(p, lv_color_hex(0x363446), 0);
    lv_obj_set_style_shadow_opa(p, 26, 0);
    lv_obj_set_scrollbar_mode(p, LV_SCROLLBAR_MODE_OFF);
    return p;
}

lv_obj_t *am_card(lv_obj_t *parent, int radius, lv_opa_t bg_opa)
{
    lv_obj_t *c = lv_obj_create(parent);
    lv_obj_remove_style_all(c);
    lv_obj_set_style_radius(c, radius, 0);
    lv_obj_set_style_bg_color(c, AM_WHITE, 0);
    lv_obj_set_style_bg_opa(c, bg_opa, 0);
    lv_obj_set_scrollbar_mode(c, LV_SCROLLBAR_MODE_OFF);
    return c;
}

lv_obj_t *am_pill(lv_obj_t *parent, const char *text, bool active)
{
    const am_theme_t *t = am_theme_get(am_theme_current());
    lv_color_t text_color = active ? t->accent : AM_MUTED_STRONG;

    lv_obj_t *pill = lv_obj_create(parent);
    lv_obj_remove_style_all(pill);
    lv_obj_set_size(pill, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /* 贴合内容 -> 胶囊(否则默认尺寸+CIRCLE=大圆) */
    lv_obj_set_style_radius(pill, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_ver(pill, 9, 0);
    lv_obj_set_style_pad_hor(pill, 12, 0);
    if(active){
        lv_obj_set_style_bg_color(pill, t->accent, 0);
        lv_obj_set_style_bg_opa(pill, AM_OPA_ACCENT_12, 0);
    } else {
        lv_obj_set_style_bg_color(pill, AM_WHITE, 0);
        lv_obj_set_style_bg_opa(pill, 179, 0);
    }
    const am_metrics_t *m = am_metrics();
    lv_obj_t *lbl = am_text(pill, text, m->f_label, text_color);
    lv_obj_center(lbl);
    return pill;
}

lv_obj_t *am_section_title(lv_obj_t *parent, const char *text)
{
    const am_metrics_t *m = am_metrics();
    lv_obj_t *lbl = am_text(parent, text, m->f_label, AM_MUTED);
    lv_obj_set_style_text_letter_space(lbl, 2, 0);
    return lbl;
}

lv_obj_t *am_item_action(lv_obj_t *parent, const char *glyph)
{
    const am_theme_t *t = am_theme_get(am_theme_current());
    const am_metrics_t *m = am_metrics();
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, m->item_action, m->item_action);
    lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(obj, t->accent, 0);
    lv_obj_set_style_bg_opa(obj, 36, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *lbl = am_text(obj, glyph, m->f_label, t->accent);
    lv_obj_center(lbl);
    return obj;
}
