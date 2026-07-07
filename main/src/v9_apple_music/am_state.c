#include "am_state.h"

#include <stdio.h>
#include <string.h>

static am_local_item_t *find_item_by_path(am_local_item_t *items, size_t count, const char *path)
{
    size_t i;

    for(i = 0; i < count; i++) {
        if(strcmp(items[i].path, path) == 0) return &items[i];
    }

    return NULL;
}

int am_state_load(const char *path, am_local_item_t *items, size_t count, uint64_t *max_recent_seq)
{
    FILE *fp;
    char line[1600];

    if(max_recent_seq != NULL) *max_recent_seq = 0U;

    fp = fopen(path, "r");
    if(fp == NULL) return 0;

    while(fgets(line, sizeof(line), fp) != NULL) {
        char rec_path[1024];
        unsigned favorite = 0U;
        unsigned long long recent_seq = 0ULL;
        am_local_item_t *item;

        if(sscanf(line, "%1023[^\t]\t%u\t%llu", rec_path, &favorite, &recent_seq) != 3) continue;

        item = find_item_by_path(items, count, rec_path);
        if(item == NULL) continue;

        item->favorite = (favorite != 0U);
        item->recent_seq = (uint64_t)recent_seq;
        if(max_recent_seq != NULL && item->recent_seq > *max_recent_seq) {
            *max_recent_seq = item->recent_seq;
        }
    }

    fclose(fp);
    return 0;
}

int am_state_save(const char *path, const am_local_item_t *items, size_t count)
{
    FILE *fp;
    size_t i;
    int rc = 0;

    fp = fopen(path, "w");
    if(fp == NULL) return -1;

    for(i = 0; i < count; i++) {
        if(items[i].path[0] == '\0') continue;
        if(!items[i].favorite && items[i].recent_seq == 0U) continue;

        if(fprintf(fp, "%s\t%u\t%llu\n",
                   items[i].path,
                   items[i].favorite ? 1U : 0U,
                   (unsigned long long)items[i].recent_seq) < 0) {
            rc = -1;
            break;
        }
    }

    if(fclose(fp) != 0) rc = -1;
    return rc;
}

size_t am_state_collect_recent(const am_local_item_t *items, size_t count, size_t *out_indices, size_t out_cap)
{
    size_t i;
    size_t used = 0U;
    size_t j;

    for(i = 0; i < count; i++) {
        if(items[i].recent_seq == 0U) continue;

        for(j = used; j > 0U; j--) {
            if(items[out_indices[j - 1U]].recent_seq >= items[i].recent_seq) break;
            if(j < out_cap) out_indices[j] = out_indices[j - 1U];
        }

        if(j < out_cap) out_indices[j] = i;
        if(used < out_cap) used++;
    }

    return used;
}

void am_state_mark_recent(am_local_item_t *items, size_t count, size_t index, uint64_t *next_recent_seq)
{
    if(items == NULL || next_recent_seq == NULL || index >= count) return;

    (*next_recent_seq)++;
    items[index].recent_seq = *next_recent_seq;
}

bool am_state_toggle_favorite(am_local_item_t *items, size_t count, size_t index)
{
    if(items == NULL || index >= count) return false;

    items[index].favorite = !items[index].favorite;
    return items[index].favorite;
}
