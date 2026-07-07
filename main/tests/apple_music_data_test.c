#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/v9_apple_music/am_data.h"
#include "../src/v9_apple_music/am_local_scan.h"
#include "../src/v9_apple_music/am_sources_csv.h"

#define CHECK(cond) check_true((cond), #cond, __FILE__, __LINE__)

static void check_true(bool cond, const char *expr, const char *file, int line)
{
    if(cond) return;

    fprintf(stderr, "CHECK failed: %s (%s:%d)\n", expr, file, line);
    abort();
}

static void test_csv_loader(void)
{
    am_radio_item_t *radio = NULL;
    size_t radio_count = 0;
    int rc = am_sources_csv_load(
        "third-party/hls_player_demo/qa/production_test/config/sources.tsv",
        &radio,
        &radio_count
    );

    CHECK(rc == 0);
    CHECK(radio != NULL);
    CHECK(radio_count > 0);
    CHECK(radio[0].title[0] != '\0');
    CHECK(radio[0].url[0] != '\0');
    CHECK(radio_count > 10);
    CHECK(strcmp(radio[0].title, "中国之声") == 0);
    CHECK(strstr(radio[0].url, "m3u8") != NULL);
    CHECK(radio[0].duration_ms > 0);
    CHECK(radio[0].network_cache_ms == 1000);

    am_sources_csv_free(radio);
}

static void test_mock_lyrics(void)
{
    CHECK(strcmp(am_mock_lyrics[1], "火力全开 势不可挡") == 0);
}

static void test_local_scan(void)
{
    am_local_item_t *local = NULL;
    size_t local_count = 0;
    size_t i;
    int rc = am_local_scan_dir(
        "third-party/hls_player_demo/test_file",
        &local,
        &local_count
    );

    CHECK(rc == 0);
    CHECK(local != NULL);
    CHECK(local_count > 3);
    CHECK(local[0].path[0] != '\0');
    CHECK(local[0].title[0] != '\0');
    CHECK(strstr(local[0].path, "third-party/hls_player_demo/test_file/") != NULL);
    for(i = 0; i < local_count; i++) {
        CHECK(local[i].favorite == false);
        CHECK(local[i].recent_seq == 0U);
    }

    am_local_scan_free(local);
}

int main(void)
{
    test_csv_loader();
    test_local_scan();
    test_mock_lyrics();
    return 0;
}
