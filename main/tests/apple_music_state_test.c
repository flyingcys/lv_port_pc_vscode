#define _DEFAULT_SOURCE

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/v9_apple_music/am_data.h"
#include "../src/v9_apple_music/am_state.h"

#define CHECK(cond) check_true((cond), #cond, __FILE__, __LINE__)

static void check_true(bool cond, const char *expr, const char *file, int line)
{
    if(cond) return;

    fprintf(stderr, "CHECK failed: %s (%s:%d)\n", expr, file, line);
    abort();
}

static void fill_item(am_local_item_t *item, const char *path, const char *title,
                      bool favorite, uint64_t recent_seq)
{
    memset(item, 0, sizeof(*item));
    snprintf(item->path, sizeof(item->path), "%s", path);
    snprintf(item->title, sizeof(item->title), "%s", title);
    item->favorite = favorite;
    item->recent_seq = recent_seq;
}

static void test_state_roundtrip(void)
{
    am_local_item_t items[3];
    am_local_item_t loaded[3];
    uint64_t max_recent = 0U;
    char state_path[] = "/tmp/am_state_roundtrip_XXXXXX";
    int fd = mkstemp(state_path);

    CHECK(fd >= 0);
    close(fd);
    unlink(state_path);

    fill_item(&items[0], "/music/a.mp3", "A", true, 7U);
    fill_item(&items[1], "/music/b.mp3", "B", false, 11U);
    fill_item(&items[2], "/music/c.mp3", "C", true, 0U);

    memset(loaded, 0, sizeof(loaded));
    fill_item(&loaded[0], "/music/a.mp3", "A", false, 0U);
    fill_item(&loaded[1], "/music/b.mp3", "B", false, 0U);
    fill_item(&loaded[2], "/music/c.mp3", "C", false, 0U);

    CHECK(am_state_save(state_path, items, 3U) == 0);
    CHECK(am_state_load(state_path, loaded, 3U, &max_recent) == 0);
    CHECK(loaded[0].favorite == true);
    CHECK(loaded[1].recent_seq == 11U);
    CHECK(loaded[2].favorite == true);
    CHECK(max_recent == 11U);
    unlink(state_path);
}

static void test_state_path_constant(void)
{
    CHECK(strcmp(AM_STATE_PATH, "apple_music_state.tsv") == 0);
}

static void test_state_save_skips_plain_local_items(void)
{
    am_local_item_t items[4];
    char state_path[] = "/tmp/am_state_filtered_XXXXXX";
    char line[1600];
    FILE *fp;
    int fd = mkstemp(state_path);
    size_t line_count = 0U;

    CHECK(fd >= 0);
    close(fd);
    unlink(state_path);

    fill_item(&items[0], "/music/a.mp3", "A", false, 0U);
    fill_item(&items[1], "/music/b.mp3", "B", true, 0U);
    fill_item(&items[2], "/music/c.mp3", "C", false, 3U);
    fill_item(&items[3], "/music/d.mp3", "D", true, 5U);

    CHECK(am_state_save(state_path, items, 4U) == 0);

    fp = fopen(state_path, "r");
    CHECK(fp != NULL);
    while(fgets(line, sizeof(line), fp) != NULL) {
        line_count++;
        CHECK(strstr(line, "/music/a.mp3") == NULL);
    }
    fclose(fp);

    CHECK(line_count == 3U);
    unlink(state_path);
}

static void test_collect_recent_top4(void)
{
    am_local_item_t items[5];
    size_t out[4] = {0};

    fill_item(&items[0], "/music/a.mp3", "A", false, 2U);
    fill_item(&items[1], "/music/b.mp3", "B", false, 9U);
    fill_item(&items[2], "/music/c.mp3", "C", false, 0U);
    fill_item(&items[3], "/music/d.mp3", "D", false, 5U);
    fill_item(&items[4], "/music/e.mp3", "E", false, 8U);

    CHECK(am_state_collect_recent(items, 5U, out, 4U) == 4U);
    CHECK(out[0] == 1U);
    CHECK(out[1] == 4U);
    CHECK(out[2] == 3U);
    CHECK(out[3] == 0U);
}

static void test_load_skips_malformed_lines(void)
{
    am_local_item_t items[2];
    uint64_t max_recent = 0U;
    char state_path[] = "/tmp/am_state_badline_XXXXXX";
    FILE *fp;
    int fd = mkstemp(state_path);

    CHECK(fd >= 0);
    fp = fdopen(fd, "w");
    CHECK(fp != NULL);
    fprintf(fp, "/music/a.mp3\t1\t4\n");
    fprintf(fp, "bad line without tabs\n");
    fprintf(fp, "/music/b.mp3\t0\t12\n");
    fclose(fp);

    fill_item(&items[0], "/music/a.mp3", "A", false, 0U);
    fill_item(&items[1], "/music/b.mp3", "B", true, 1U);

    CHECK(am_state_load(state_path, items, 2U, &max_recent) == 0);
    CHECK(items[0].favorite == true);
    CHECK(items[1].favorite == false);
    CHECK(items[1].recent_seq == 12U);
    CHECK(max_recent == 12U);
    unlink(state_path);
}

int main(void)
{
    test_state_roundtrip();
    test_state_path_constant();
    test_state_save_skips_plain_local_items();
    test_collect_recent_top4();
    test_load_skips_malformed_lines();
    return 0;
}
