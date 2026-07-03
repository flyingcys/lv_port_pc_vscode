#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "../src/v9_apple_music/am_data.h"
#include "../src/v9_apple_music/am_local_scan.h"
#include "../src/v9_apple_music/am_sources_csv.h"

static void test_csv_loader(void)
{
    am_radio_item_t *radio = NULL;
    size_t radio_count = 0;
    int rc = am_sources_csv_load(
        "third-party/hls_player_demo/qa/production_test/config/sources.tsv",
        &radio,
        &radio_count
    );

    assert(rc == 0);
    assert(radio != NULL);
    assert(radio_count > 0);
    assert(radio[0].title[0] != '\0');
    assert(radio[0].url[0] != '\0');
    assert(radio_count > 10);
    assert(strcmp(radio[0].title, "中国之声") == 0);
    assert(strstr(radio[0].url, "m3u8") != NULL);
    assert(radio[0].duration_ms > 0);
    assert(radio[0].network_cache_ms == 1000);

    am_sources_csv_free(radio);
}

static void test_mock_lyrics(void)
{
    assert(strcmp(am_mock_lyrics[1], "火力全开 势不可挡") == 0);
}

static void test_local_scan(void)
{
    am_local_item_t *local = NULL;
    size_t local_count = 0;
    int rc = am_local_scan_dir(
        "third-party/hls_player_demo/test_file",
        &local,
        &local_count
    );

    assert(rc == 0);
    assert(local != NULL);
    assert(local_count > 3);
    assert(local[0].path[0] != '\0');
    assert(local[0].title[0] != '\0');
    assert(strstr(local[0].path, "third-party/hls_player_demo/test_file/") != NULL);

    am_local_scan_free(local);
}

int main(void)
{
    test_csv_loader();
    test_local_scan();
    test_mock_lyrics();
    return 0;
}
