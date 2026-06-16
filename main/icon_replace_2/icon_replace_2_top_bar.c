#include "icon_replace_2_top_bar.h"

#include "icon_replace_2_glyphs.h"
#include "icon_replace_2_metrics.h"
#include "icon_replace_2_theme.h"

#include <sys/time.h>
#include <time.h>

/* -----------------------------------------------------------------------
 * 内部结构
 * left_slot:        左槽（时钟，左对齐）
 * center_slot:      中槽（保留接口；HTML 状态栏不显示，默认隐藏）
 * system_right_slot: 右槽（信号 + WiFi + 电池图标 + 电量文字）
 * --------------------------------------------------------------------- */
struct icon_replace_2_top_bar {
    lv_obj_t * root;
    lv_obj_t * left_slot;
    lv_obj_t * center_slot;
    lv_obj_t * system_right_slot;

    /* 左槽：时钟文字（Montserrat，白色）*/
    lv_obj_t * clock_label;

    /* 中槽：保留接口用（默认不显示）*/
    lv_obj_t * center_label;

    /* 右槽：Phosphor 图标 + 电量百分比 */
    lv_obj_t * signal_label;
    lv_obj_t * wifi_label;
    lv_obj_t * battery_label;
    lv_obj_t * battery_pct_label;

    lv_timer_t * minute_timer;
};

/* -----------------------------------------------------------------------
 * 前向声明
 * --------------------------------------------------------------------- */
static void top_bar_delete_cb(lv_event_t * e);
static void top_bar_minute_timer_cb(lv_timer_t * timer);
static void top_bar_apply_slot(lv_obj_t * slot, lv_obj_t * label, topbar_slot_type_t type, const char * text,
                               lv_align_t align);
static void top_bar_apply_page_mode(icon_replace_2_top_bar_t * top_bar, topbar_page_mode_t page_mode);
static const char * top_bar_wifi_glyph(wifi_state_t state);
static const char * top_bar_slot_placeholder_text(topbar_slot_type_t type, const char * text);
static void top_bar_update_current_time(icon_replace_2_top_bar_t * top_bar);
static uint32_t top_bar_get_initial_period_ms(void);
static void top_bar_configure_root(icon_replace_2_top_bar_t * top_bar, lv_obj_t * parent);
static void top_bar_configure_slots(icon_replace_2_top_bar_t * top_bar);
static void top_bar_configure_slot(lv_obj_t * slot, lv_coord_t width, lv_align_t align, lv_coord_t x_ofs);
static lv_obj_t * top_bar_create_clock_label(lv_obj_t * parent, lv_align_t align);
static lv_obj_t * top_bar_create_glyph_label(lv_obj_t * parent, lv_align_t align);

/* -----------------------------------------------------------------------
 * 公开 API
 * --------------------------------------------------------------------- */

icon_replace_2_top_bar_t * icon_replace_2_top_bar_create(lv_obj_t * parent)
{
    icon_replace_2_top_bar_t * top_bar = lv_malloc_zeroed(sizeof(*top_bar));

    if(top_bar == NULL) {
        return NULL;
    }

    top_bar_configure_root(top_bar, parent);
    top_bar_configure_slots(top_bar);

    /* 初始化右槽图标文本 */
    lv_label_set_text(top_bar->signal_label,  IR2_GLYPH_SIGNAL);
    lv_label_set_text(top_bar->wifi_label,    IR2_GLYPH_WIFI);
    lv_label_set_text(top_bar->battery_label, IR2_GLYPH_BATTERY_FULL);
    lv_label_set_text(top_bar->battery_pct_label, "100%");

    /* 初始化时钟 */
    icon_replace_2_top_bar_set_time(top_bar, "--:--");

    /* 中槽默认不显示（HTML 状态栏无中间文案）*/
    lv_obj_add_flag(top_bar->center_slot, LV_OBJ_FLAG_HIDDEN);

    lv_obj_move_foreground(top_bar->root);

    return top_bar;
}

void icon_replace_2_top_bar_destroy(icon_replace_2_top_bar_t * top_bar)
{
    if(top_bar == NULL || top_bar->root == NULL) {
        return;
    }

    lv_obj_delete(top_bar->root);
}

lv_obj_t * icon_replace_2_top_bar_get_root(const icon_replace_2_top_bar_t * top_bar)
{
    if(top_bar == NULL) {
        return NULL;
    }

    return top_bar->root;
}

void icon_replace_2_top_bar_apply(icon_replace_2_top_bar_t * top_bar, const topbar_page_config_t * config)
{
    if(top_bar == NULL || config == NULL) {
        return;
    }

    top_bar_apply_page_mode(top_bar, config->page_mode);

    /* left_slot: 保留旧接口；若 left_type == NONE 则仍显示时钟（保持常驻） */
    if(config->left_type != TOPBAR_SLOT_NONE) {
        top_bar_apply_slot(top_bar->left_slot, top_bar->center_label, config->left_type, config->left_text,
                           LV_ALIGN_LEFT_MID);
    }

    /* center_slot: HTML 状态栏无中间文案，始终隐藏 */
    lv_obj_add_flag(top_bar->center_slot, LV_OBJ_FLAG_HIDDEN);

    /* system_right_slot: 始终显示（HTML 状态栏系统图标在所有页出现） */
    if(config->show_system_right) {
        lv_obj_clear_flag(top_bar->system_right_slot, LV_OBJ_FLAG_HIDDEN);
    }
    else {
        lv_obj_add_flag(top_bar->system_right_slot, LV_OBJ_FLAG_HIDDEN);
    }
}

void icon_replace_2_top_bar_set_time(icon_replace_2_top_bar_t * top_bar, const char * time_text)
{
    if(top_bar == NULL) {
        return;
    }

    lv_label_set_text(top_bar->clock_label, time_text != NULL ? time_text : "--:--");
}

void icon_replace_2_top_bar_set_wifi_state(icon_replace_2_top_bar_t * top_bar, wifi_state_t state)
{
    if(top_bar == NULL) {
        return;
    }

    if(state == WIFI_STATE_OFF) {
        /* WiFi 关闭：淡化图标 */
        lv_obj_set_style_text_opa(top_bar->wifi_label, LV_OPA_30, 0);
    }
    else {
        lv_obj_set_style_text_opa(top_bar->wifi_label, LV_OPA_COVER, 0);
    }

    lv_label_set_text(top_bar->wifi_label, top_bar_wifi_glyph(state));
}

void icon_replace_2_top_bar_start_minute_timer(icon_replace_2_top_bar_t * top_bar)
{
    if(top_bar == NULL) {
        return;
    }

    top_bar_update_current_time(top_bar);

    if(top_bar->minute_timer == NULL) {
        top_bar->minute_timer = lv_timer_create(top_bar_minute_timer_cb, top_bar_get_initial_period_ms(), top_bar);
        return;
    }

    lv_timer_set_period(top_bar->minute_timer, top_bar_get_initial_period_ms());
    lv_timer_reset(top_bar->minute_timer);
    lv_timer_resume(top_bar->minute_timer);
}

/* -----------------------------------------------------------------------
 * 内部实现
 * --------------------------------------------------------------------- */

static void top_bar_delete_cb(lv_event_t * e)
{
    icon_replace_2_top_bar_t * top_bar = lv_event_get_user_data(e);

    if(top_bar == NULL) {
        return;
    }

    if(top_bar->minute_timer != NULL) {
        lv_timer_delete(top_bar->minute_timer);
        top_bar->minute_timer = NULL;
    }

    top_bar->root = NULL;
    lv_free(top_bar);
}

static void top_bar_minute_timer_cb(lv_timer_t * timer)
{
    icon_replace_2_top_bar_t * top_bar = lv_timer_get_user_data(timer);

    if(top_bar == NULL) {
        return;
    }

    top_bar_update_current_time(top_bar);
    lv_timer_set_period(timer, top_bar_get_initial_period_ms());
}

static void top_bar_apply_page_mode(icon_replace_2_top_bar_t * top_bar, topbar_page_mode_t page_mode)
{
    /* HTML 风格：背景始终透明，叠在壁纸上 */
    LV_UNUSED(page_mode);

    if(top_bar == NULL || top_bar->root == NULL) {
        return;
    }

    lv_obj_set_style_bg_opa(top_bar->root, LV_OPA_TRANSP, 0);
}

static const char * top_bar_wifi_glyph(wifi_state_t state)
{
    switch(state) {
        case WIFI_STATE_OFF:
        case WIFI_STATE_DISCONNECTED:
            return IR2_GLYPH_WIFI;   /* 淡化显示（opa 由调用方设置）*/
        case WIFI_STATE_WEAK:
        case WIFI_STATE_NORMAL:
        case WIFI_STATE_STRONG:
        default:
            return IR2_GLYPH_WIFI;
    }
}

static const char * top_bar_slot_placeholder_text(topbar_slot_type_t type, const char * text)
{
    if(text != NULL && text[0] != '\0') {
        return text;
    }

    if(type == TOPBAR_SLOT_CUSTOM_OBJ) {
        return "[custom]";
    }

    return "";
}

static void top_bar_apply_slot(lv_obj_t * slot, lv_obj_t * label, topbar_slot_type_t type, const char * text,
                               lv_align_t align)
{
    if(slot == NULL || label == NULL) {
        return;
    }

    switch(type) {
        case TOPBAR_SLOT_NONE:
            lv_label_set_text(label, "");
            lv_obj_add_flag(slot, LV_OBJ_FLAG_HIDDEN);
            return;
        case TOPBAR_SLOT_TEXT:
        case TOPBAR_SLOT_CUSTOM_OBJ:
            lv_label_set_text(label, top_bar_slot_placeholder_text(type, text));
            lv_obj_clear_flag(slot, LV_OBJ_FLAG_HIDDEN);
            lv_obj_align(label, align, 0, 0);
            return;
        default:
            lv_label_set_text(label, "[slot?]");
            lv_obj_clear_flag(slot, LV_OBJ_FLAG_HIDDEN);
            lv_obj_align(label, align, 0, 0);
            return;
    }
}

static void top_bar_update_current_time(icon_replace_2_top_bar_t * top_bar)
{
    time_t now;
    struct tm local_tm;
    char time_buf[6];

    if(top_bar == NULL) {
        return;
    }

    time(&now);
    localtime_r(&now, &local_tm);
    strftime(time_buf, sizeof(time_buf), "%H:%M", &local_tm);
    icon_replace_2_top_bar_set_time(top_bar, time_buf);
}

static uint32_t top_bar_get_initial_period_ms(void)
{
    struct timeval now;
    struct tm local_tm;
    uint32_t ms_in_second;
    uint32_t remaining_ms;

    gettimeofday(&now, NULL);
    localtime_r(&now.tv_sec, &local_tm);
    ms_in_second = (uint32_t)(now.tv_usec / 1000U);

    if(local_tm.tm_sec == 0 && ms_in_second == 0U) {
        return 60000U;
    }

    remaining_ms = (uint32_t)(59 - local_tm.tm_sec) * 1000U + (1000U - ms_in_second);
    return remaining_ms > 0U ? remaining_ms : 1U;
}

static void top_bar_configure_root(icon_replace_2_top_bar_t * top_bar, lv_obj_t * parent)
{
    const ir2_metrics_t * m = ir2_metrics();

    top_bar->root = lv_obj_create(parent);
    lv_obj_set_size(top_bar->root, m->screen_w, m->top_bar_h);
    lv_obj_align(top_bar->root, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(top_bar->root, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_radius(top_bar->root, 0, 0);
    lv_obj_set_style_border_width(top_bar->root, 0, 0);
    lv_obj_set_style_pad_all(top_bar->root, 0, 0);

    /* 透明背景，叠在壁纸上 */
    lv_obj_set_style_bg_opa(top_bar->root, LV_OPA_TRANSP, 0);

    /* 文字色：主题白色 */
    lv_obj_set_style_text_color(top_bar->root, ir2_theme()->text_primary, 0);

    lv_obj_add_event_cb(top_bar->root, top_bar_delete_cb, LV_EVENT_DELETE, top_bar);
}

static void top_bar_configure_slots(icon_replace_2_top_bar_t * top_bar)
{
    const ir2_metrics_t * m     = ir2_metrics();
    lv_coord_t           bar_h  = (lv_coord_t)m->top_bar_h;
    lv_coord_t           scr_w  = (lv_coord_t)m->screen_w;
    /* 左槽宽度：屏宽 30% */
    lv_coord_t           left_w = scr_w * 30 / 100;
    /* 右槽宽度：屏宽 45%（容纳三图标 + 百分比）*/
    lv_coord_t           right_w = scr_w * 45 / 100;

    top_bar->left_slot         = lv_obj_create(top_bar->root);
    top_bar->center_slot       = lv_obj_create(top_bar->root);
    top_bar->system_right_slot = lv_obj_create(top_bar->root);

    lv_obj_remove_style_all(top_bar->left_slot);
    lv_obj_remove_style_all(top_bar->center_slot);
    lv_obj_remove_style_all(top_bar->system_right_slot);

    /* 左槽：靠左，水平边距 8px */
    top_bar_configure_slot(top_bar->left_slot,   left_w,  LV_ALIGN_LEFT_MID,  8);
    /* 中槽：居中（HTML 状态栏无中间文案，始终隐藏）*/
    top_bar_configure_slot(top_bar->center_slot, scr_w / 3, LV_ALIGN_CENTER, 0);
    /* 右槽：靠右，水平边距 -8px */
    top_bar_configure_slot(top_bar->system_right_slot, right_w, LV_ALIGN_RIGHT_MID, -8);

    /* -------- 左槽：时钟 label -------- */
    top_bar->clock_label = top_bar_create_clock_label(top_bar->left_slot, LV_ALIGN_LEFT_MID);

    /* -------- 中槽：保留 center_label（接口兼容）-------- */
    top_bar->center_label = top_bar_create_clock_label(top_bar->center_slot, LV_ALIGN_CENTER);

    /* -------- 右槽：使用 flex 横向排列 -------- */
    lv_obj_set_style_pad_all(top_bar->system_right_slot, 0, 0);
    lv_obj_set_layout(top_bar->system_right_slot, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(top_bar->system_right_slot, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_bar->system_right_slot,
                          LV_FLEX_ALIGN_END,    /* main: 右对齐 */
                          LV_FLEX_ALIGN_CENTER, /* cross: 垂直居中 */
                          LV_FLEX_ALIGN_CENTER);
    /* 图标之间的列间距 */
    lv_obj_set_style_pad_column(top_bar->system_right_slot, 4, 0);

    /* 信号图标 */
    top_bar->signal_label = top_bar_create_glyph_label(top_bar->system_right_slot, LV_ALIGN_DEFAULT);
    /* WiFi 图标 */
    top_bar->wifi_label = top_bar_create_glyph_label(top_bar->system_right_slot, LV_ALIGN_DEFAULT);
    /* 电量百分比文字 */
    top_bar->battery_pct_label = top_bar_create_clock_label(top_bar->system_right_slot, LV_ALIGN_DEFAULT);
    /* 电池图标 */
    top_bar->battery_label = top_bar_create_glyph_label(top_bar->system_right_slot, LV_ALIGN_DEFAULT);

    LV_UNUSED(bar_h);
}

static void top_bar_configure_slot(lv_obj_t * slot, lv_coord_t width, lv_align_t align, lv_coord_t x_ofs)
{
    const ir2_metrics_t * m = ir2_metrics();

    if(slot == NULL) {
        return;
    }

    lv_obj_set_size(slot, width, (lv_coord_t)m->top_bar_h);
    lv_obj_align(slot, align, x_ofs, 0);
    lv_obj_clear_flag(slot, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
}

/* 时钟/百分比字体 label（font_clock = Montserrat）*/
static lv_obj_t * top_bar_create_clock_label(lv_obj_t * parent, lv_align_t align)
{
    const ir2_metrics_t * m = ir2_metrics();
    lv_obj_t * label = lv_label_create(parent);

    lv_obj_set_style_text_color(label, ir2_theme()->text_primary, 0);
    lv_obj_set_style_text_font(label, m->font_clock, 0);
    lv_obj_align(label, align, 0, 0);

    return label;
}

/* Phosphor 图标 label（font_glyph）*/
static lv_obj_t * top_bar_create_glyph_label(lv_obj_t * parent, lv_align_t align)
{
    const ir2_metrics_t * m = ir2_metrics();
    lv_obj_t * label = lv_label_create(parent);

    lv_obj_set_style_text_color(label, ir2_theme()->text_primary, 0);
    lv_obj_set_style_text_font(label, m->font_glyph, 0);
    lv_obj_align(label, align, 0, 0);

    return label;
}
