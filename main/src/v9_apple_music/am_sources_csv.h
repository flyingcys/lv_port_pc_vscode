#ifndef AM_SOURCES_CSV_H
#define AM_SOURCES_CSV_H

#include <stddef.h>

#include "am_data.h"

int am_sources_csv_load(const char *path, am_radio_item_t **items, size_t *count);
void am_sources_csv_free(am_radio_item_t *items);

#endif /* AM_SOURCES_CSV_H */
