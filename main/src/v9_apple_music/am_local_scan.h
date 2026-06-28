#ifndef AM_LOCAL_SCAN_H
#define AM_LOCAL_SCAN_H

#include <stddef.h>

#include "am_data.h"

int am_local_scan_dir(const char *dir, am_local_item_t **items, size_t *count);
void am_local_scan_free(am_local_item_t *items);

#endif /* AM_LOCAL_SCAN_H */
