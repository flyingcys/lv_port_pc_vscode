#include "desktop_gesture.h"

#include <stdbool.h>
#include <stdio.h>

static int report_check(bool condition, const char * message)
{
    if(condition) {
        return 0;
    }

    fprintf(stderr, "test_gesture: %s\n", message);
    return 1;
}

int main(void)
{
    int failures = 0;

    failures += report_check(!desktop_gesture_cancels_tap(0, 0),
                             "stationary press should remain tap");
    failures += report_check(!desktop_gesture_cancels_tap(8, 8),
                             "threshold movement should remain tap");
    failures += report_check(desktop_gesture_cancels_tap(0, 9),
                             "vertical movement should cancel tap");
    failures += report_check(desktop_gesture_cancels_tap(9, 0),
                             "horizontal movement should cancel tap");

    failures += report_check(!desktop_gesture_is_vertical(0, 20),
                             "20px vertical movement is below panel axis threshold");
    failures += report_check(desktop_gesture_is_vertical(0, 21),
                             "vertical movement over 20px should pass axis test");
    failures += report_check(!desktop_gesture_is_vertical(30, 21),
                             "horizontal-dominant movement should not pass vertical axis test");

    if(failures != 0) {
        fprintf(stderr, "test_gesture: FAIL (%d checks failed)\n", failures);
        return 1;
    }

    printf("test_gesture: PASS\n");
    return 0;
}
