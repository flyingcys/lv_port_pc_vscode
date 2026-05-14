#ifndef ICON_REPLACE_2_PAGE_CONFIG_H
#define ICON_REPLACE_2_PAGE_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TOPBAR_PAGE_MODE_LOCK = 0,
    TOPBAR_PAGE_MODE_HOME,
    TOPBAR_PAGE_MODE_CUSTOM,
} topbar_page_mode_t;

typedef enum {
    TOPBAR_SLOT_NONE = 0,
    TOPBAR_SLOT_TEXT,
    TOPBAR_SLOT_CUSTOM_OBJ,
} topbar_slot_type_t;

typedef struct {
    topbar_slot_type_t left_type;
    const char * left_text;
    topbar_slot_type_t center_type;
    const char * center_text;
    topbar_page_mode_t page_mode;
    bool show_system_right;
} topbar_page_config_t;

const topbar_page_config_t * icon_replace_2_get_page_config(uint32_t page_index);

#ifdef __cplusplus
}
#endif

#endif
