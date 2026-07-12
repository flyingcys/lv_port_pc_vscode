/*******************************************************************************
 * Smoke test: clock homepage fonts
 *
 * Verifies all newly generated LVGL C font files for the clock homepage
 * can be loaded and return valid glyph descriptors via lv_font_get_glyph_dsc.
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include <stdio.h>
#include <stdbool.h>

/* Fonts are compiled as separate translation units (listed in CMake).
   Declare their public lv_font_t objects here for linker resolution. */
LV_FONT_DECLARE(desktop_font_calc_148)
LV_FONT_DECLARE(desktop_font_calc_34)
LV_FONT_DECLARE(clock_label_font_13)
LV_FONT_DECLARE(clock_label_font_14)
LV_FONT_DECLARE(clock_label_font_28)
LV_FONT_DECLARE(clock_label_font_30)
LV_FONT_DECLARE(clock_label_font_32)
LV_FONT_DECLARE(clock_label_font_33)
LV_FONT_DECLARE(clock_label_font_34)
LV_FONT_DECLARE(clock_label_font_35)

typedef struct {
    const char *name;
    const lv_font_t *font;
    uint32_t probe_letter;
} probe_case_t;

int main(void)
{
    const lv_font_t *null_font = NULL;
    if (null_font != NULL) {
        /* unreachable, guards against unused */
    }

    probe_case_t cases[] = {
        {"desktop_font_calc_148", &desktop_font_calc_148, '0'},
        {"desktop_font_calc_148", &desktop_font_calc_148, '9'},
        {"desktop_font_calc_34",  &desktop_font_calc_34,  '5'},
        {"clock_label_font_13",   &clock_label_font_13,   'J'},
        {"clock_label_font_14",   &clock_label_font_14,   0x00B0}, /* degree */
        {"clock_label_font_28",   &clock_label_font_28,   'E'},  /* EVENING */
        {"clock_label_font_30",   &clock_label_font_30,   '2'},  /* year digits */
        {"clock_label_font_32",   &clock_label_font_32,   '1'},  /* date digits */
        {"clock_label_font_33",   &clock_label_font_33,   'M'},  /* MONDAY */
        {"clock_label_font_34",   &clock_label_font_34,   '.'},  /* temp/humidity value dot */
        {"clock_label_font_35",   &clock_label_font_35,   'A'},  /* JANUARY */
        /* extra coverage across the label set */
        {"clock_label_font_13",   &clock_label_font_13,   'T'},  /* TEMP */
        {"clock_label_font_28",   &clock_label_font_28,   '%'},
        {"clock_label_font_32",   &clock_label_font_33,   'W'},
        {"clock_label_font_34",   &clock_label_font_34,   ':'},
    };

    int failures = 0;
    int total = sizeof(cases) / sizeof(cases[0]);

    for (int i = 0; i < total; i++) {
        lv_font_glyph_dsc_t dsc;
        bool ok = lv_font_get_glyph_dsc(cases[i].font, &dsc,
                                        cases[i].probe_letter, 0x0000);
        uint32_t cp = cases[i].probe_letter;
        if (!ok) {
            printf("FAIL: %s glyph U+%04lX missing\n",
                   cases[i].name, (unsigned long)cp);
            failures++;
        } else {
            printf("PASS: %s U+%04lX -> w=%d adv_w=%d\n",
                   cases[i].name, (unsigned long)cp,
                   dsc.box_w, dsc.adv_w);
        }
    }

    printf("\n%d/%d probes passed\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
