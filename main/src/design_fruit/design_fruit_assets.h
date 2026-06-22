#ifndef DESIGN_FRUIT_ASSETS_H
#define DESIGN_FRUIT_ASSETS_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool design_fruit_assets_init(const char *project_root);
bool design_fruit_assets_build_image_path(char *out, size_t out_size, const char *relative_path);
bool design_fruit_assets_build_audio_path(char *out, size_t out_size, const char *relative_path);

#ifdef __cplusplus
}
#endif

#endif /* DESIGN_FRUIT_ASSETS_H */
