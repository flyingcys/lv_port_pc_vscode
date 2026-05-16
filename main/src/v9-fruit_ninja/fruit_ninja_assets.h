#ifndef FRUIT_NINJA_ASSETS_H
#define FRUIT_NINJA_ASSETS_H

#include <stdbool.h>
#include <stddef.h>

bool fruit_ninja_assets_init(const char *project_root);
bool fruit_ninja_assets_build_image_path(char *out, size_t out_size, const char *relative_path);
bool fruit_ninja_assets_build_audio_path(char *out, size_t out_size, const char *relative_path);
bool fruit_ninja_assets_validate_core_files(void);

#endif
