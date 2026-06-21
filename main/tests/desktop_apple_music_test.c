#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "../desktop/apple_music_app.h"
#include "../desktop/desktop_data.h"

void calc_app_launch(void) {}
void fruit_ninja_app_launch(void) {}
void local_music_demo_launch(void) {}
void apple_music_app_launch(void) {}

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
    const desktop_app_t *app = find_app("Apple Music");
    assert(app != NULL);
    assert(app->launch == apple_music_app_launch);
    return 0;
}
