#include "icon_replace_2_layout.h"
#include "icon_replace_2_page_config.h"

#include <stdbool.h>
#include <stdio.h>

static int report_check(bool condition, const char * message)
{
    if(condition) {
        return 0;
    }

    fprintf(stderr, "test_page_config: %s\n", message);
    return 1;
}

int main(void)
{
    const topbar_page_config_t * lock_cfg = icon_replace_2_get_page_config(0);
    const topbar_page_config_t * home_cfg = icon_replace_2_get_page_config(1);
    const topbar_page_config_t * custom_cfg = icon_replace_2_get_page_config(2);
    const topbar_page_config_t * fallback_cfg = icon_replace_2_get_page_config(PAGE_COUNT);
    int failures = 0;

    failures += report_check(lock_cfg != NULL, "lock_cfg is NULL");
    failures += report_check(home_cfg != NULL, "home_cfg is NULL");
    failures += report_check(custom_cfg != NULL, "custom_cfg is NULL");
    failures += report_check(fallback_cfg != NULL, "fallback_cfg is NULL");

    failures += report_check(SCREEN_W == 800, "SCREEN_W should be 800");
    failures += report_check(SCREEN_H == 480, "SCREEN_H should be 480");
    failures += report_check(TOP_BAR_H == 40, "TOP_BAR_H should be 40");
    failures += report_check(DESKTOP_W == SCREEN_W, "DESKTOP_W should match SCREEN_W");
    failures += report_check(DESKTOP_H == (SCREEN_H - TOP_BAR_H), "DESKTOP_H should exclude top bar");
    failures += report_check(ICON_START_X == 90, "ICON_START_X should match legacy layout");
    failures += report_check(ICON_START_Y == 50, "ICON_START_Y should match legacy layout");
    failures += report_check(ICON_X_DISTANCE == 140, "ICON_X_DISTANCE should match legacy layout");
    failures += report_check(ICON_Y_DISTANCE == 140, "ICON_Y_DISTANCE should match legacy layout");
    failures += report_check(ICON_SIZE == 60, "ICON_SIZE should match legacy layout");
    failures += report_check(PAGE_COUNT == 3, "PAGE_COUNT should be 3");
    failures += report_check(ICON_SLOT_COUNT == (ICON_MAX_ROW * ICON_MAX_COL),
                             "ICON_SLOT_COUNT should match grid size");

    if(lock_cfg != NULL) {
        failures += report_check(lock_cfg->page_mode == TOPBAR_PAGE_MODE_LOCK,
                                 "page_0 should be lock mode");
        failures += report_check(lock_cfg->left_type == TOPBAR_SLOT_NONE,
                                 "page_0 left_type should be none");
        failures += report_check(lock_cfg->left_text == NULL,
                                 "page_0 left_text should be NULL");
        failures += report_check(lock_cfg->center_type == TOPBAR_SLOT_TEXT,
                                 "page_0 center_type should be text");
        failures += report_check(lock_cfg->center_text != NULL,
                                 "page_0 center_text should not be NULL");
        failures += report_check(lock_cfg->show_system_right == true,
                                 "page_0 should show system right slot");
    }

    if(home_cfg != NULL) {
        failures += report_check(home_cfg->page_mode == TOPBAR_PAGE_MODE_HOME,
                                 "page_1 should be home mode");
        failures += report_check(home_cfg->left_type == TOPBAR_SLOT_NONE,
                                 "page_1 left_type should be none");
        failures += report_check(home_cfg->left_text == NULL,
                                 "page_1 left_text should be NULL");
        failures += report_check(home_cfg->center_type == TOPBAR_SLOT_NONE,
                                 "page_1 center_type should be none");
        failures += report_check(home_cfg->center_text == NULL,
                                 "page_1 center_text should be NULL");
        failures += report_check(home_cfg->show_system_right == true,
                                 "page_1 should show system right slot");
    }

    if(custom_cfg != NULL) {
        failures += report_check(custom_cfg->page_mode == TOPBAR_PAGE_MODE_CUSTOM,
                                 "page_2 should be custom mode");
        failures += report_check(custom_cfg->left_type == TOPBAR_SLOT_TEXT,
                                 "page_2 left_type should be text");
        failures += report_check(custom_cfg->left_text != NULL,
                                 "page_2 left_text should not be NULL");
        failures += report_check(custom_cfg->center_type == TOPBAR_SLOT_TEXT,
                                 "page_2 center_type should be text");
        failures += report_check(custom_cfg->center_text != NULL,
                                 "page_2 center_text should not be NULL");
    }

    if(home_cfg != NULL && fallback_cfg != NULL) {
        failures += report_check(fallback_cfg == home_cfg,
                                 "out-of-range page should fall back to page_1");
    }

    if(failures != 0) {
        fprintf(stderr, "test_page_config: FAIL (%d checks failed)\n", failures);
        return 1;
    }

    printf("test_page_config: PASS\n");
    return 0;
}
