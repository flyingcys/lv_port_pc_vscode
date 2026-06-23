#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "../desktop/apple_music_app.h"
#include "../desktop/desktop_data.h"
#include "../desktop/desktop_layout.h"
#include "../desktop/tetris_app.h"

void calc_app_launch(void) {}
void fruit_ninja_app_launch(void) {}
void design_fruit_app_launch(void) {}
void local_music_demo_launch(void) {}
void apple_music_app_launch(void) {}
void tetris_app_launch(void) {}
void game_2048_app_launch(void) {}

static const desktop_app_t *find_app(const char *name)
{
    for(uint32_t i = 0; i < desktop_app_count; i++) {
        if(strcmp(desktop_apps[i].name, name) == 0) {
            return &desktop_apps[i];
        }
    }
    return NULL;
}

int main(void)
{
    uint32_t page_counts[DESKTOP_PAGE_COUNT] = {0};

    const desktop_app_t *app = find_app("Apple Music");
    assert(app != NULL);
    assert(app->launch == apple_music_app_launch);

    app = find_app("俄罗斯方块");
    assert(app != NULL);
    assert(app->launch == tetris_app_launch);

    app = find_app("2048");
    assert(app != NULL);
    assert(app->launch == game_2048_app_launch);

    app = find_app("水果对对碰");
    assert(app != NULL);
    assert(app->launch == design_fruit_app_launch);

    for(uint32_t i = 0; i < desktop_app_count; i++) {
        assert(desktop_apps[i].page < DESKTOP_PAGE_COUNT);
        page_counts[desktop_apps[i].page]++;
    }

    assert(desktop_app_count == 13);
    assert(page_counts[0] == 0);
    assert(page_counts[1] <= DESKTOP_SLOT_COUNT);
    assert(page_counts[2] <= DESKTOP_SLOT_COUNT);
    assert(page_counts[1] == 10);
    assert(page_counts[2] == 3);
    assert(desktop_app_page_count(0) == 0);
    assert(desktop_app_page_count(1) == page_counts[1]);
    assert(desktop_app_page_count(2) == page_counts[2]);
    return 0;
}
