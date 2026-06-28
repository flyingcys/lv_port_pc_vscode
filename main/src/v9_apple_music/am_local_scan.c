#define _DEFAULT_SOURCE

#include "am_local_scan.h"

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static bool am_has_audio_ext(const char *name)
{
    static const char *exts[] = {
        ".mp3", ".wav", ".flac", ".aac", ".m4a", ".ogg", ".opus",
        ".wma", ".mp4", ".ts", ".aiff", ".ac3"
    };
    const char *dot = strrchr(name, '.');
    size_t i;

    if(dot == NULL) return false;
    for(i = 0; i < sizeof(exts) / sizeof(exts[0]); i++) {
        if(strcasecmp(dot, exts[i]) == 0) return true;
    }
    return false;
}

static int am_compare_local_item(const void *lhs, const void *rhs)
{
    const am_local_item_t *a = (const am_local_item_t *)lhs;
    const am_local_item_t *b = (const am_local_item_t *)rhs;
    return strcmp(a->title, b->title);
}

static void am_copy_text(char *dst, size_t dst_size, const char *src)
{
    if(dst_size == 0U) return;
    if(src == NULL) src = "";
    strncpy(dst, src, dst_size - 1U);
    dst[dst_size - 1U] = '\0';
}

static void am_title_from_name(char *dst, size_t dst_size, const char *name)
{
    char scratch[512];
    char *dot;

    am_copy_text(scratch, sizeof(scratch), name);
    dot = strrchr(scratch, '.');
    if(dot != NULL) *dot = '\0';
    am_copy_text(dst, dst_size, scratch);
}

int am_local_scan_dir(const char *dir, am_local_item_t **items, size_t *count)
{
    DIR *dp;
    struct dirent *entry;
    am_local_item_t *buffer = NULL;
    size_t used = 0U;
    size_t cap = 0U;

    if(items == NULL || count == NULL || dir == NULL) return -1;
    *items = NULL;
    *count = 0U;

    dp = opendir(dir);
    if(dp == NULL) return -2;

    while((entry = readdir(dp)) != NULL) {
        char full_path[1400];
        struct stat st;

        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        if(!am_has_audio_ext(entry->d_name)) continue;

        snprintf(full_path, sizeof(full_path), "%s/%s", dir, entry->d_name);
        if(stat(full_path, &st) != 0 || !S_ISREG(st.st_mode)) continue;

        if(used == cap) {
            size_t new_cap = (cap == 0U) ? 16U : (cap * 2U);
            am_local_item_t *next_buffer =
                (am_local_item_t *)realloc(buffer, new_cap * sizeof(*buffer));
            if(next_buffer == NULL) {
                free(buffer);
                closedir(dp);
                return -3;
            }
            buffer = next_buffer;
            cap = new_cap;
        }

        memset(&buffer[used], 0, sizeof(buffer[used]));
        am_copy_text(buffer[used].path, sizeof(buffer[used].path), full_path);
        am_title_from_name(buffer[used].title, sizeof(buffer[used].title), entry->d_name);
        buffer[used].favorite = (used < 4U);
        used++;
    }

    closedir(dp);

    if(used == 0U) {
        free(buffer);
        return -4;
    }

    qsort(buffer, used, sizeof(*buffer), am_compare_local_item);
    *items = buffer;
    *count = used;
    return 0;
}

void am_local_scan_free(am_local_item_t *items)
{
    free(items);
}
