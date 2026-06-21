#include <assert.h>
#include <string.h>
#include "../src/v9_apple_music/am_data.h"

static void test_counts(void)
{
    assert(sizeof(am_nav_items)/sizeof(am_nav_items[0]) == 5);
    assert(sizeof(am_theme_presets)/sizeof(am_theme_presets[0]) == 4);
    assert(sizeof(am_settings_tabs)/sizeof(am_settings_tabs[0]) == 3);
}
static void test_first_values(void)
{
    assert(strcmp(am_nav_items[0].id, "home") == 0);
    assert(strcmp(am_theme_presets[2].id, "mint") == 0);   /* 默认主题 */
    assert(am_mini.progress_pct == 44);
    assert(strcmp(am_page_radio.list[0].kind, "radio") == 0);
}
int main(void){ test_counts(); test_first_values(); return 0; }
