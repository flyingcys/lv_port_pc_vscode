/* main/src/v9_apple_music/am_config.c */
#include "am_config.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void safe_copy(char *dst, size_t dsz, const char *src) {
    if(!src) { dst[0] = '\0'; return; }
    strncpy(dst, src, dsz - 1);
    dst[dsz - 1] = '\0';
}

int am_config_load_from_path(const char *path, am_config_t *out) {
    FILE *f;
    long  file_size;
    char *buf;
    size_t n;
    cJSON *root, *local, *radio, *item;
    int    i;

    if(!path || !out) return -1;
    memset(out, 0, sizeof(*out));

    f = fopen(path, "rb");
    if(!f) return -1;

    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    rewind(f);
    if(file_size <= 0) { fclose(f); return -2; }

    buf = (char *)malloc((size_t)file_size + 1u);
    if(!buf) { fclose(f); return -2; }

    n = fread(buf, 1, (size_t)file_size, f);
    fclose(f);
    buf[n] = '\0';

    root = cJSON_Parse(buf);
    free(buf);
    if(!root) return -2;

    /* local.dir */
    local = cJSON_GetObjectItemCaseSensitive(root, "local");
    if(cJSON_IsObject(local)) {
        cJSON *dir = cJSON_GetObjectItemCaseSensitive(local, "dir");
        if(cJSON_IsString(dir)) safe_copy(out->local_dir, sizeof(out->local_dir), dir->valuestring);
    }

    /* radio[] */
    radio = cJSON_GetObjectItemCaseSensitive(root, "radio");
    if(cJSON_IsArray(radio)) {
        i = 0;
        cJSON_ArrayForEach(item, radio) {
            cJSON *t;
            cJSON *s;
            cJSON *u;
            if(i >= AM_CONFIG_RADIO_MAX) break;
            if(!cJSON_IsObject(item)) continue;
            t  = cJSON_GetObjectItemCaseSensitive(item, "title");
            s  = cJSON_GetObjectItemCaseSensitive(item, "subtitle");
            u  = cJSON_GetObjectItemCaseSensitive(item, "url");
            if(!cJSON_IsString(u)) continue;
            safe_copy(out->radio[i].title,    sizeof(out->radio[i].title),    cJSON_IsString(t) ? t->valuestring : "");
            safe_copy(out->radio[i].subtitle, sizeof(out->radio[i].subtitle), cJSON_IsString(s) ? s->valuestring : "");
            safe_copy(out->radio[i].url,      sizeof(out->radio[i].url),      u->valuestring);
            i++;
        }
        out->radio_count = i;
    }

    cJSON_Delete(root);
    return 0;
}

int am_config_load(am_config_t *out) {
    const char *env;
    char        path[1024];
    const char *candidates[3];
    int         k;
    const char *home;

    if(!out) return -1;

    env = getenv("AM_CONFIG");
    candidates[0] = env;
    candidates[1] = "./am_config.json";

    /* ~/.config/apple_music/config.json */
    path[0] = '\0';
    home = getenv("HOME");
    if(home) {
        snprintf(path, sizeof(path), "%s/.config/apple_music/config.json", home);
        candidates[2] = path;
    } else {
        candidates[2] = NULL;
    }

    for(k = 0; k < 3; k++) {
        if(!candidates[k]) continue;
        if(am_config_load_from_path(candidates[k], out) == 0) return 0;
    }
    memset(out, 0, sizeof(*out));
    return -1;
}
