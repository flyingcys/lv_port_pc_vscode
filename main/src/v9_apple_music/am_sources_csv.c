#include "am_sources_csv.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *am_trim(char *text)
{
    char *end;

    if(text == NULL) return NULL;
    while(*text != '\0' && isspace((unsigned char)*text)) text++;
    if(*text == '\0') return text;

    end = text + strlen(text) - 1;
    while(end > text && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    return text;
}

static void am_copy_text(char *dst, size_t dst_size, const char *src)
{
    if(dst_size == 0) return;
    if(src == NULL) src = "";
    strncpy(dst, src, dst_size - 1U);
    dst[dst_size - 1U] = '\0';
}

static FILE *am_open_sources_file(const char *path, char *resolved, size_t resolved_size)
{
    FILE *fp;
    const char *ext;
    const char *alt_ext;
    size_t base_len;

    if(path == NULL || resolved == NULL || resolved_size == 0U) return NULL;

    fp = fopen(path, "r");
    if(fp != NULL) {
        am_copy_text(resolved, resolved_size, path);
        return fp;
    }

    ext = strrchr(path, '.');
    if(ext == NULL) return NULL;
    if(strcmp(ext, ".tsv") == 0) alt_ext = ".csv";
    else if(strcmp(ext, ".csv") == 0) alt_ext = ".tsv";
    else return NULL;

    base_len = (size_t)(ext - path);
    if(base_len + strlen(alt_ext) + 1U > resolved_size) return NULL;

    memcpy(resolved, path, base_len);
    strcpy(resolved + base_len, alt_ext);
    return fopen(resolved, "r");
}

int am_sources_csv_load(const char *path, am_radio_item_t **items, size_t *count)
{
    FILE *fp;
    char line[4096];
    char resolved_path[512];
    am_radio_item_t *buffer = NULL;
    size_t used = 0;
    size_t cap = 0;
    char delim = ',';

    if(items == NULL || count == NULL || path == NULL) return -1;
    *items = NULL;
    *count = 0;

    fp = am_open_sources_file(path, resolved_path, sizeof(resolved_path));
    if(fp == NULL) return -2;

    if(fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return -3;
    }
    if(strchr(line, '\t') != NULL) delim = '\t';

    while(fgets(line, sizeof(line), fp) != NULL) {
        char *fields[5] = {0};
        char *cursor = line;
        size_t field_count = 0;

        while(field_count < 5U && cursor != NULL) {
            char *next = strchr(cursor, delim);
            if(next != NULL) {
                *next = '\0';
                next++;
            }
            fields[field_count++] = am_trim(cursor);
            cursor = next;
        }

        if(field_count < 5U || fields[1] == NULL || fields[2] == NULL) continue;
        if(fields[1][0] == '\0' || fields[2][0] == '\0') continue;

        if(used == cap) {
            size_t new_cap = (cap == 0U) ? 16U : (cap * 2U);
            am_radio_item_t *next_buffer =
                (am_radio_item_t *)realloc(buffer, new_cap * sizeof(*buffer));
            if(next_buffer == NULL) {
                free(buffer);
                fclose(fp);
                return -4;
            }
            buffer = next_buffer;
            cap = new_cap;
        }

        memset(&buffer[used], 0, sizeof(buffer[used]));
        am_copy_text(buffer[used].title, sizeof(buffer[used].title), fields[1]);
        am_copy_text(buffer[used].url, sizeof(buffer[used].url), fields[2]);
        buffer[used].duration_ms = (uint32_t)strtoul(fields[3] ? fields[3] : "0", NULL, 10);
        buffer[used].network_cache_ms = (uint32_t)strtoul(fields[4] ? fields[4] : "0", NULL, 10);
        used++;
    }

    fclose(fp);

    if(used == 0U) {
        free(buffer);
        return -5;
    }

    *items = buffer;
    *count = used;
    return 0;
}

void am_sources_csv_free(am_radio_item_t *items)
{
    free(items);
}
