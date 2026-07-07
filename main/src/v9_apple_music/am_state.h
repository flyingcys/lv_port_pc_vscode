#ifndef AM_STATE_H
#define AM_STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "am_data.h"

#define AM_STATE_PATH "apple_music_state.tsv"

int am_state_load(const char *path, am_local_item_t *items, size_t count, uint64_t *max_recent_seq);
int am_state_save(const char *path, const am_local_item_t *items, size_t count);
size_t am_state_collect_recent(const am_local_item_t *items, size_t count, size_t *out_indices, size_t out_cap);
void am_state_mark_recent(am_local_item_t *items, size_t count, size_t index, uint64_t *next_recent_seq);
bool am_state_toggle_favorite(am_local_item_t *items, size_t count, size_t index);

#endif /* AM_STATE_H */
