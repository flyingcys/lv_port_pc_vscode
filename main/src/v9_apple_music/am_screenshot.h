#ifndef AM_SCREENSHOT_H
#define AM_SCREENSHOT_H
/* Render the active screen to a PPM (P6) file at `path`. For headless visual verification. */
void am_screenshot_take(const char *path);
#endif
