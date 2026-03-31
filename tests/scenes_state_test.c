#include "scenes_state.h"
#include <assert.h>

int main(void)
{
    SCENES_STATE_T state;

    scenes_state_init(&state);
    assert(scenes_state_get_selected(&state) == SCENES_CARD_MORNING);

    scenes_state_select(&state, SCENES_CARD_HOME);
    assert(scenes_state_get_selected(&state) == SCENES_CARD_HOME);

    scenes_state_select(&state, SCENES_CARD_ALL_LIGHTS_ON);
    assert(scenes_state_get_selected(&state) == SCENES_CARD_ALL_LIGHTS_ON);

    scenes_state_select(&state, (SCENES_CARD_E)99);
    assert(scenes_state_get_selected(&state) == SCENES_CARD_ALL_LIGHTS_ON);

    scenes_state_select(&state, SCENES_CARD_ALL_LIGHTS_ON);
    assert(scenes_state_get_selected(&state) == SCENES_CARD_ALL_LIGHTS_ON);

    return 0;
}
