#include <assert.h>
#include <string.h>
#include "../src/v9_apple_music/am_theme.h"

int main(void)
{
    const am_theme_t *mint = am_theme_get(AM_THEME_MINT);
    assert(strcmp(mint->id, "mint") == 0);
    assert(lv_color_to_u32(mint->accent) == lv_color_to_u32(lv_color_hex(0x23b497)));
    assert(lv_color_to_u32(mint->hero_c) == lv_color_to_u32(lv_color_hex(0x1a6662)));
    am_theme_set(AM_THEME_ORANGE);
    assert(am_theme_current() == AM_THEME_ORANGE);
    assert(lv_color_to_u32(am_theme_get(am_theme_current())->accent)
           == lv_color_to_u32(lv_color_hex(0xf08d3c)));
    return 0;
}
