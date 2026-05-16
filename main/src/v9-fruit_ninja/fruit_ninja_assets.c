#include "fruit_ninja_assets.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define FRUIT_NINJA_ASSET_ROOT "main/assets/fruit_ninja"

static char g_project_root[PATH_MAX];

static bool fruit_ninja_assets_path_exists(const char *path)
{
    return path != NULL && access(path, F_OK) == 0;
}

static bool fruit_ninja_assets_copy_root(char *dst, size_t dst_size, const char *src)
{
    size_t len;

    if(dst == NULL || dst_size == 0 || src == NULL || src[0] == '\0') {
        return false;
    }

    len = strlen(src);
    while(len > 1 && src[len - 1] == '/') {
        len--;
    }

    if(len + 1 > dst_size) {
        return false;
    }

    memcpy(dst, src, len);
    dst[len] = '\0';
    return true;
}

static bool fruit_ninja_assets_has_repo_marker(const char *root)
{
    char marker_path[PATH_MAX];
    int written;

    written = snprintf(marker_path, sizeof(marker_path), "%s/%s", root, FRUIT_NINJA_ASSET_ROOT);
    if(written < 0 || (size_t)written >= sizeof(marker_path)) {
        return false;
    }

    return fruit_ninja_assets_path_exists(marker_path);
}

static bool fruit_ninja_assets_infer_project_root(char *out, size_t out_size)
{
    char cwd[PATH_MAX];
    char candidate[PATH_MAX];
    char *slash;

    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        return false;
    }

    if(!fruit_ninja_assets_copy_root(candidate, sizeof(candidate), cwd)) {
        return false;
    }

    while(true) {
        if(fruit_ninja_assets_has_repo_marker(candidate)) {
            return fruit_ninja_assets_copy_root(out, out_size, candidate);
        }

        slash = strrchr(candidate, '/');
        if(slash == NULL) {
            break;
        }
        if(slash == candidate) {
            candidate[1] = '\0';
            if(fruit_ninja_assets_has_repo_marker(candidate)) {
                return fruit_ninja_assets_copy_root(out, out_size, candidate);
            }
            break;
        }
        *slash = '\0';
    }

    return false;
}

static bool fruit_ninja_assets_build_path(char *out,
                                          size_t out_size,
                                          const char *prefix,
                                          const char *relative_path)
{
    int written;

    if(out == NULL || out_size == 0 || prefix == NULL || relative_path == NULL || relative_path[0] == '\0') {
        return false;
    }

    if(g_project_root[0] == '\0') {
        return false;
    }

    written = snprintf(out,
                       out_size,
                       "%s%s/%s/%s",
                       prefix,
                       g_project_root,
                       FRUIT_NINJA_ASSET_ROOT,
                       relative_path);
    return written >= 0 && (size_t)written < out_size;
}

static bool fruit_ninja_assets_core_file_exists(const char *relative_path)
{
    char path[PATH_MAX];

    if(!fruit_ninja_assets_build_audio_path(path, sizeof(path), relative_path)) {
        return false;
    }

    return fruit_ninja_assets_path_exists(path);
}

bool fruit_ninja_assets_init(const char *project_root)
{
    if(project_root != NULL && project_root[0] != '\0') {
        return fruit_ninja_assets_copy_root(g_project_root, sizeof(g_project_root), project_root);
    }

    return fruit_ninja_assets_infer_project_root(g_project_root, sizeof(g_project_root));
}

bool fruit_ninja_assets_build_image_path(char *out, size_t out_size, const char *relative_path)
{
    return fruit_ninja_assets_build_path(out, out_size, "S:", relative_path);
}

bool fruit_ninja_assets_build_audio_path(char *out, size_t out_size, const char *relative_path)
{
    return fruit_ninja_assets_build_path(out, out_size, "", relative_path);
}

bool fruit_ninja_assets_validate_core_files(void)
{
    static const char *const required_files[] = {
        "images/background.jpg",
        "images/logo.png",
        "images/new-game.png",
        "images/home-mask.png",
        "images/home-desc.png",
        "images/new.png",
        "images/shadow.png",
        "images/flash.png",
        "images/smoke.png",
        "images/game-over.png",
        "images/fruit/apple.png",
        "images/fruit/apple-1.png",
        "images/fruit/apple-2.png",
        "images/fruit/banana.png",
        "images/fruit/banana-1.png",
        "images/fruit/banana-2.png",
        "images/fruit/basaha.png",
        "images/fruit/basaha-1.png",
        "images/fruit/basaha-2.png",
        "images/fruit/boom.png",
        "images/fruit/peach.png",
        "images/fruit/peach-1.png",
        "images/fruit/peach-2.png",
        "images/fruit/sandia.png",
        "images/fruit/sandia-1.png",
        "images/fruit/sandia-2.png",
        "sound/menu.ogg",
        "sound/boom.ogg",
        "sound/over.ogg",
        "sound/splatter.ogg",
        "sound/start.ogg",
        "sound/throw.ogg",
    };
    size_t i;

    for(i = 0; i < sizeof(required_files) / sizeof(required_files[0]); i++) {
        if(!fruit_ninja_assets_core_file_exists(required_files[i])) {
            return false;
        }
    }

    return true;
}
