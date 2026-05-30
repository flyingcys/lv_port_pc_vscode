#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char * read_file(const char * path)
{
    FILE * file = fopen(path, "rb");
    assert(file != 0);

    assert(fseek(file, 0, SEEK_END) == 0);
    long size = ftell(file);
    assert(size >= 0);
    assert(fseek(file, 0, SEEK_SET) == 0);

    char * data = malloc((size_t)size + 1U);
    assert(data != 0);
    assert(fread(data, 1U, (size_t)size, file) == (size_t)size);
    data[size] = '\0';
    assert(fclose(file) == 0);
    return data;
}

static void assert_contains(const char * haystack, const char * needle)
{
    assert(strstr(haystack, needle) != 0);
}

static void assert_not_contains(const char * haystack, const char * needle)
{
    assert(strstr(haystack, needle) == 0);
}

int main(void)
{
    char * html = read_file("third-party/apple_music_player_mockup/index.html");
    char * css = read_file("third-party/apple_music_player_mockup/styles.css");
    char * js = read_file("third-party/apple_music_player_mockup/app.js");

    assert_contains(html, "<html lang=\"zh-CN\">");
    assert_contains(html, "<div class=\"app-shell\" id=\"app\"></div>");
    assert_contains(html, "data-theme=\"cyan\"");

    assert_contains(css, "width: 800px;");
    assert_contains(css, "height: 480px;");
    assert_contains(css, "body[data-theme=\"cyan\"]");
    assert_contains(css, "body[data-theme=\"blue\"]");
    assert_contains(css, "body[data-theme=\"mint\"]");
    assert_contains(css, "body[data-theme=\"auto-1\"]");
    assert_contains(css, "--user-accent");

    assert_contains(js, "page: \"home\"");
    assert_contains(js, "theme: \"cyan\"");
    assert_contains(js, "id: \"auto-1\"");
    assert_contains(js, "label: \"自动一\"");
    assert_contains(js, "主页");
    assert_contains(js, "广播");
    assert_contains(js, "本地");
    assert_contains(js, "歌单");
    assert_contains(js, "设置");
    assert_not_contains(js, "id: \"orange\"");

    free(html);
    free(css);
    free(js);
    return 0;
}
