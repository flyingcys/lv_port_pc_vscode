#include "icon_replace_2_page_config.h"

#include "icon_replace_2_layout.h"

#include <stddef.h>

static const topbar_page_config_t g_page_configs[PAGE_COUNT] = {
    {
        .left_type = TOPBAR_SLOT_NONE,
        .left_text = NULL,
        .center_type = TOPBAR_SLOT_TEXT,
        .center_text = "09:41",
        .page_mode = TOPBAR_PAGE_MODE_LOCK,
        .show_system_right = true,
    },
    {
        .left_type = TOPBAR_SLOT_NONE,
        .left_text = NULL,
        .center_type = TOPBAR_SLOT_NONE,
        .center_text = NULL,
        .page_mode = TOPBAR_PAGE_MODE_HOME,
        .show_system_right = true,
    },
    {
        .left_type = TOPBAR_SLOT_NONE,
        .left_text = NULL,
        .center_type = TOPBAR_SLOT_NONE,
        .center_text = NULL,
        .page_mode = TOPBAR_PAGE_MODE_HOME,
        .show_system_right = true,
    },
};

const topbar_page_config_t * icon_replace_2_get_page_config(uint32_t page_index)
{
    if(page_index >= PAGE_COUNT) {
        return &g_page_configs[1];
    }

    return &g_page_configs[page_index];
}
