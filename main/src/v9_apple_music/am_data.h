#ifndef AM_DATA_H
#define AM_DATA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    AM_VIEW_NOW = 0,
    AM_VIEW_RADIO,
    AM_VIEW_FAVORITES,
    AM_VIEW_COUNT,
} am_view_t;

typedef enum {
    AM_SOURCE_NONE = 0,
    AM_SOURCE_LOCAL,
    AM_SOURCE_RADIO,
} am_source_kind_t;

typedef struct {
    const char *id;
    const char *label;
    const char *icon;
} am_nav_item_t;

typedef struct {
    const char *id;
    const char *label;
    const char *desc;
} am_theme_preset_t;

typedef struct {
    const char *id;
    const char *title;
    const char *desc;
} am_settings_tab_t;

typedef struct {
    char title[128];
    char url[1024];
    uint32_t duration_ms;
    uint32_t network_cache_ms;
} am_radio_item_t;

typedef struct {
    char path[1024];
    char title[256];
    bool favorite;
} am_local_item_t;

extern const am_nav_item_t am_nav_items[AM_VIEW_COUNT];
extern const char *am_recent_titles[4];
extern const char *am_mock_lyrics[4];
extern const am_theme_preset_t am_theme_presets[4];
extern const am_settings_tab_t am_settings_tabs[3];

const char *am_view_label(am_view_t view);
const char *am_source_badge(am_source_kind_t kind);

#endif /* AM_DATA_H */
