#include "design_fruit_assets.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define DESIGN_FRUIT_ASSET_ROOT "main/assets/design_fruit"

static char g_project_root[PATH_MAX];

static bool path_exists(const char *path)
{
    return path != NULL && access(path, F_OK) == 0;
}

static bool copy_root(char *dst, size_t dst_size, const char *src)
{
    size_t len;

    if(dst == NULL || dst_size == 0 || src == NULL || src[0] == '\0') return false;

    len = strlen(src);
    while(len > 1u && src[len - 1u] == '/') len--;
    if(len + 1u > dst_size) return false;

    memcpy(dst, src, len);
    dst[len] = '\0';
    return true;
}

static bool has_asset_root(const char *root)
{
    char marker[PATH_MAX];
    int written = snprintf(marker, sizeof(marker), "%s/%s", root, DESIGN_FRUIT_ASSET_ROOT);
    return written >= 0 && (size_t)written < sizeof(marker) && path_exists(marker);
}

static bool infer_project_root(char *out, size_t out_size)
{
    char cwd[PATH_MAX];
    char candidate[PATH_MAX];
    char *slash;

    if(getcwd(cwd, sizeof(cwd)) == NULL) return false;
    if(!copy_root(candidate, sizeof(candidate), cwd)) return false;

    while(true) {
        if(has_asset_root(candidate)) return copy_root(out, out_size, candidate);

        slash = strrchr(candidate, '/');
        if(slash == NULL) break;
        if(slash == candidate) {
            candidate[1] = '\0';
            if(has_asset_root(candidate)) return copy_root(out, out_size, candidate);
            break;
        }
        *slash = '\0';
    }

    return false;
}

static bool build_path(char *out, size_t out_size, const char *prefix, const char *relative_path)
{
    int written;

    if(out == NULL || out_size == 0 || prefix == NULL || relative_path == NULL || relative_path[0] == '\0') {
        return false;
    }
    if(g_project_root[0] == '\0') {
        (void)design_fruit_assets_init(NULL);
    }
    if(g_project_root[0] == '\0') return false;

    written = snprintf(out, out_size, "%s%s/%s/%s", prefix, g_project_root, DESIGN_FRUIT_ASSET_ROOT, relative_path);
    return written >= 0 && (size_t)written < out_size;
}

bool design_fruit_assets_init(const char *project_root)
{
    if(project_root != NULL && project_root[0] != '\0') {
        return copy_root(g_project_root, sizeof(g_project_root), project_root);
    }
    return infer_project_root(g_project_root, sizeof(g_project_root));
}

bool design_fruit_assets_build_image_path(char *out, size_t out_size, const char *relative_path)
{
    return build_path(out, out_size, "A:", relative_path);
}

bool design_fruit_assets_build_audio_path(char *out, size_t out_size, const char *relative_path)
{
    return build_path(out, out_size, "", relative_path);
}
