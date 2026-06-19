/* main/tests/am_config_test.c */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/v9_apple_music/am_config.h"

static void write_tmp_config(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    assert(f);
    fputs(content, f);
    fclose(f);
}

static void test_load_valid_config(void) {
    const char *path = "/tmp/am_test_config.json";
    write_tmp_config(path,
        "{"
        "  \"local\": { \"dir\": \"/tmp/music\" },"
        "  \"radio\": ["
        "    { \"title\": \"Lo-Fi\", \"subtitle\": \"chill\", \"url\": \"https://example.com/lofi.m3u8\" },"
        "    { \"title\": \"Jazz\",  \"subtitle\": \"night\", \"url\": \"https://example.com/jazz.m3u8\" }"
        "  ]"
        "}"
    );

    am_config_t cfg = {0};
    int ret = am_config_load_from_path(path, &cfg);
    assert(ret == 0);
    assert(strcmp(cfg.local_dir, "/tmp/music") == 0);
    assert(cfg.radio_count == 2);
    assert(strcmp(cfg.radio[0].title, "Lo-Fi") == 0);
    assert(strcmp(cfg.radio[0].subtitle, "chill") == 0);
    assert(strcmp(cfg.radio[0].url, "https://example.com/lofi.m3u8") == 0);
    assert(strcmp(cfg.radio[1].title, "Jazz") == 0);
    remove(path);
}

static void test_load_missing_file(void) {
    am_config_t cfg = {0};
    int ret = am_config_load_from_path("/tmp/nonexistent_config_xyz.json", &cfg);
    assert(ret == -1);
}

static void test_load_invalid_json(void) {
    const char *path = "/tmp/am_test_bad.json";
    write_tmp_config(path, "{ not valid json }}}");
    am_config_t cfg = {0};
    int ret = am_config_load_from_path(path, &cfg);
    assert(ret == -2);
    remove(path);
}

static void test_empty_radio_array(void) {
    const char *path = "/tmp/am_test_empty.json";
    write_tmp_config(path, "{ \"local\": { \"dir\": \"/tmp\" }, \"radio\": [] }");
    am_config_t cfg = {0};
    int ret = am_config_load_from_path(path, &cfg);
    assert(ret == 0);
    assert(cfg.radio_count == 0);
    assert(strcmp(cfg.local_dir, "/tmp") == 0);
    remove(path);
}

int main(void) {
    test_load_valid_config();
    test_load_missing_file();
    test_load_invalid_json();
    test_empty_radio_array();
    printf("am_config_test: all tests passed\n");
    return 0;
}
