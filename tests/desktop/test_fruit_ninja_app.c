#include "desktop_data.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int report_check(bool condition, const char * message)
{
    if(condition) {
        return 0;
    }

    fprintf(stderr, "test_fruit_ninja_app: %s\n", message);
    return 1;
}

int main(void)
{
    int failures = 0;
    bool found = false;

    for(uint32_t i = 0; i < desktop_app_count; i++) {
        if(strcmp(desktop_apps[i].name, "水果忍者") == 0) {
            found = true;
            failures += report_check(desktop_apps[i].launch != NULL,
                                     "Fruit Ninja app should have a launch callback");
            failures += report_check(desktop_apps[i].page == 2,
                                     "Fruit Ninja app should stay on page 2");
        }
    }

    failures += report_check(found, "Fruit Ninja app should exist");

    if(failures != 0) {
        fprintf(stderr, "test_fruit_ninja_app: FAIL (%d checks failed)\n", failures);
        return 1;
    }

    printf("test_fruit_ninja_app: PASS\n");
    return 0;
}
