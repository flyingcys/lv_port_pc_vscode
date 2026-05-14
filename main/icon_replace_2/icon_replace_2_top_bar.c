#include "icon_replace_2_top_bar.h"

#include "icon_replace_2_layout.h"

#include <sys/time.h>
#include <time.h>

struct icon_replace_2_top_bar {
    lv_obj_t * root;
    lv_obj_t * left_slot;
    lv_obj_t * center_slot;
    lv_obj_t * system_right_slot;
    lv_obj_t * left_label;
    lv_obj_t * center_label;
    lv_obj_t * wifi_label;
    lv_obj_t * time_label;
    lv_timer_t * minute_timer;
};

static void top_bar_delete_cb(lv_event_t * e);
static void top_bar_minute_timer_cb(lv_timer_t * timer);
static void top_bar_apply_slot(lv_obj_t * slot, lv_obj_t * label, topbar_slot_type_t type, const char * text,
                               lv_align_t align);
static void top_bar_apply_page_mode(icon_replace_2_top_bar_t * top_bar, topbar_page_mode_t page_mode);
static const char * top_bar_wifi_state_text(wifi_state_t state);
static const char * top_bar_slot_placeholder_text(topbar_slot_type_t type, const char * text);
static void top_bar_update_current_time(icon_replace_2_top_bar_t * top_bar);
static uint32_t top_bar_get_initial_period_ms(void);
static void top_bar_configure_root(icon_replace_2_top_bar_t * top_bar, lv_obj_t * parent);
static void top_bar_configure_slots(icon_replace_2_top_bar_t * top_bar);
static void top_bar_configure_slot(lv_obj_t * slot, lv_coord_t width, lv_align_t align, lv_coord_t x_ofs);
static lv_obj_t * top_bar_create_label(lv_obj_t * parent, lv_align_t align);

icon_replace_2_top_bar_t * icon_replace_2_top_bar_create(lv_obj_t * parent)
{
    icon_replace_2_top_bar_t * top_bar = lv_malloc_zeroed(sizeof(*top_bar));

    if(top_bar == NULL) {
        return NULL;
    }

    top_bar_configure_root(top_bar, parent);
    top_bar_configure_slots(top_bar);
    lv_label_set_text(top_bar->wifi_label, "WF ?");
    icon_replace_2_top_bar_set_time(top_bar, "--:--");
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
    top_bar_apply_slot(top_bar->left_slot, top_bar->left_label, config->left_type, config->left_text, LV_ALIGN_LEFT_MID);
    top_bar_apply_slot(top_bar->center_slot, top_bar->center_label, config->center_type, config->center_text,
                       LV_ALIGN_CENTER);

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

    lv_label_set_text(top_bar->time_label, time_text != NULL ? time_text : "--:--");
}

void icon_replace_2_top_bar_set_wifi_state(icon_replace_2_top_bar_t * top_bar, wifi_state_t state)
{
    if(top_bar == NULL) {
        return;
    }

    lv_label_set_text(top_bar->wifi_label, top_bar_wifi_state_text(state));
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
    lv_color_t bg_color = lv_color_hex(0x0F172A);
    lv_opa_t bg_opa = LV_OPA_COVER;

    if(top_bar == NULL || top_bar->root == NULL) {
        return;
    }

    switch(page_mode) {
        case TOPBAR_PAGE_MODE_LOCK:
            bg_color = lv_color_hex(0x020617);
            bg_opa = LV_OPA_COVER;
            break;
        case TOPBAR_PAGE_MODE_HOME:
            bg_color = lv_color_hex(0x0F172A);
            bg_opa = LV_OPA_80;
            break;
        case TOPBAR_PAGE_MODE_CUSTOM:
        default:
            bg_color = lv_color_hex(0x1E293B);
            bg_opa = LV_OPA_90;
            break;
    }

    lv_obj_set_style_bg_color(top_bar->root, bg_color, 0);
    lv_obj_set_style_bg_opa(top_bar->root, bg_opa, 0);
}

static const char * top_bar_wifi_state_text(wifi_state_t state)
{
    switch(state) {
        case WIFI_STATE_OFF:
            return "WF OFF";
        case WIFI_STATE_DISCONNECTED:
            return "WF X";
        case WIFI_STATE_WEAK:
            return "WF |";
        case WIFI_STATE_NORMAL:
            return "WF ||";
        case WIFI_STATE_STRONG:
            return "WF |||";
        default:
            return "WF ?";
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
    top_bar->root = lv_obj_create(parent);
    lv_obj_set_size(top_bar->root, SCREEN_W, TOP_BAR_H);
    lv_obj_align(top_bar->root, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(top_bar->root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(top_bar->root, 0, 0);
    lv_obj_set_style_border_width(top_bar->root, 0, 0);
    lv_obj_set_style_pad_all(top_bar->root, 0, 0);
    lv_obj_set_style_bg_color(top_bar->root, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_bg_opa(top_bar->root, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(top_bar->root, lv_color_hex(0xFFFFFF), 0);
    lv_obj_add_event_cb(top_bar->root, top_bar_delete_cb, LV_EVENT_DELETE, top_bar);
}

static void top_bar_configure_slots(icon_replace_2_top_bar_t * top_bar)
{
    top_bar->left_slot = lv_obj_create(top_bar->root);
    top_bar->center_slot = lv_obj_create(top_bar->root);
    top_bar->system_right_slot = lv_obj_create(top_bar->root);

    lv_obj_remove_style_all(top_bar->left_slot);
    lv_obj_remove_style_all(top_bar->center_slot);
    lv_obj_remove_style_all(top_bar->system_right_slot);

    top_bar_configure_slot(top_bar->left_slot, 180, LV_ALIGN_LEFT_MID, 12);
    top_bar_configure_slot(top_bar->center_slot, 220, LV_ALIGN_CENTER, 0);
    top_bar_configure_slot(top_bar->system_right_slot, 220, LV_ALIGN_RIGHT_MID, -12);

    top_bar->left_label = top_bar_create_label(top_bar->left_slot, LV_ALIGN_LEFT_MID);
    top_bar->center_label = top_bar_create_label(top_bar->center_slot, LV_ALIGN_CENTER);
    top_bar->wifi_label = top_bar_create_label(top_bar->system_right_slot, LV_ALIGN_LEFT_MID);
    top_bar->time_label = top_bar_create_label(top_bar->system_right_slot, LV_ALIGN_RIGHT_MID);
}

static void top_bar_configure_slot(lv_obj_t * slot, lv_coord_t width, lv_align_t align, lv_coord_t x_ofs)
{
    if(slot == NULL) {
        return;
    }

    lv_obj_set_size(slot, width, TOP_BAR_H);
    lv_obj_align(slot, align, x_ofs, 0);
    lv_obj_clear_flag(slot, LV_OBJ_FLAG_SCROLLABLE);
}

static lv_obj_t * top_bar_create_label(lv_obj_t * parent, lv_align_t align)
{
    lv_obj_t * label = lv_label_create(parent);

    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label, align, 0, 0);

    return label;
}
