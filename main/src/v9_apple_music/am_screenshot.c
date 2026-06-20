#include <stdio.h>
#include "lvgl/lvgl.h"
#include "am_screenshot.h"

/**
 * Render the active screen to a binary PPM (P6) file at `path`.
 *
 * Pixel format: LV_COLOR_FORMAT_ARGB8888
 * In-memory byte order (little-endian): B, G, R, A per pixel.
 * PPM requires R, G, B — so we reorder accordingly.
 * Row addressing uses `stride` (may exceed w*4 due to alignment padding).
 */
void am_screenshot_take(const char *path)
{
    lv_draw_buf_t *snap = lv_snapshot_take(lv_screen_active(), LV_COLOR_FORMAT_ARGB8888);
    if(snap == NULL) {
        fprintf(stderr, "[am_screenshot] lv_snapshot_take() returned NULL\n");
        return;
    }

    uint32_t w      = snap->header.w;
    uint32_t h      = snap->header.h;
    uint32_t stride = snap->header.stride;  /* bytes per row (may be padded) */
    uint8_t *data   = snap->data;

    FILE *fp = fopen(path, "wb");
    if(!fp) {
        fprintf(stderr, "[am_screenshot] fopen(%s) failed\n", path);
        lv_draw_buf_destroy(snap);
        return;
    }

    /* Write PPM header */
    fprintf(fp, "P6\n%u %u\n255\n", (unsigned)w, (unsigned)h);

    /* Write pixel data: convert BGRA -> RGB */
    for(uint32_t row = 0; row < h; row++) {
        const uint8_t *line = data + (size_t)row * stride;
        for(uint32_t col = 0; col < w; col++) {
            /* ARGB8888 little-endian in memory: B=0, G=1, R=2, A=3 */
            uint8_t b = line[col * 4 + 0];
            uint8_t g = line[col * 4 + 1];
            uint8_t r = line[col * 4 + 2];
            uint8_t rgb[3] = {r, g, b};
            fwrite(rgb, 1, 3, fp);
        }
    }

    fclose(fp);
    lv_draw_buf_destroy(snap);
    printf("[am_screenshot] saved %ux%u PPM to %s\n", (unsigned)w, (unsigned)h, path);
}
