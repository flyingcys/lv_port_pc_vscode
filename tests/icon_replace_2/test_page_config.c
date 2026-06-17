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
    const topbar_page_config_t * home1_cfg = icon_replace_2_get_page_config(1);
    const topbar_page_config_t * home2_cfg = icon_replace_2_get_page_config(2);
    const topbar_page_config_t * fallback_cfg = icon_replace_2_get_page_config(IR2_PAGE_COUNT);
    int failures = 0;

    failures += report_check(lock_cfg != NULL, "lock_cfg is NULL");
    failures += report_check(home1_cfg != NULL, "home1_cfg is NULL");
    failures += report_check(home2_cfg != NULL, "home2_cfg is NULL");
    failures += report_check(fallback_cfg != NULL, "fallback_cfg is NULL");

    /* 纯尺寸常量（lvgl-free，HTML 复刻后为 5x2 网格、3 页） */
    failures += report_check(IR2_PAGE_COUNT == 3, "IR2_PAGE_COUNT should be 3");
    failures += report_check(IR2_GRID_COLS == 5, "IR2_GRID_COLS should be 5");
    failures += report_check(IR2_GRID_ROWS == 2, "IR2_GRID_ROWS should be 2");
    failures += report_check(IR2_SLOT_COUNT == (IR2_GRID_COLS * IR2_GRID_ROWS),
                             "IR2_SLOT_COUNT should match grid size (10)");

    /* page_0：锁屏页，中间槽显示大时钟占位文案 */
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

    /* page_1 与 page_2：均为 app 桌面页（HOME），顶栏无左/中扩展文案 */
    if(home1_cfg != NULL) {
        failures += report_check(home1_cfg->page_mode == TOPBAR_PAGE_MODE_HOME,
                                 "page_1 should be home mode");
        failures += report_check(home1_cfg->left_type == TOPBAR_SLOT_NONE,
                                 "page_1 left_type should be none");
        failures += report_check(home1_cfg->center_type == TOPBAR_SLOT_NONE,
                                 "page_1 center_type should be none");
        failures += report_check(home1_cfg->show_system_right == true,
                                 "page_1 should show system right slot");
    }

    if(home2_cfg != NULL) {
        failures += report_check(home2_cfg->page_mode == TOPBAR_PAGE_MODE_HOME,
                                 "page_2 should be home mode");
        failures += report_check(home2_cfg->left_type == TOPBAR_SLOT_NONE,
                                 "page_2 left_type should be none");
        failures += report_check(home2_cfg->center_type == TOPBAR_SLOT_NONE,
                                 "page_2 center_type should be none");
    }

    /* 越界页索引回退到 page_1 */
    if(home1_cfg != NULL && fallback_cfg != NULL) {
        failures += report_check(fallback_cfg == home1_cfg,
                                 "out-of-range page should fall back to page_1");
    }

    if(failures != 0) {
        fprintf(stderr, "test_page_config: FAIL (%d checks failed)\n", failures);
        return 1;
    }

    printf("test_page_config: PASS\n");
    return 0;
}
