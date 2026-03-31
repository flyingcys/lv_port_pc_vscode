/**
 * @file scenes_state.c
 * @brief Scenes page card selection state implementation
 * @version 1.0
 * @date 2026-03-31
 * @copyright Copyright (c) 2026
 */

#include "scenes_state.h"
#include <stddef.h>

/* ---------------------------------------------------------------------------
 * Function implementations
 * --------------------------------------------------------------------------- */
/**
 * @brief Initialize scenes selection state
 * @param[out] state State object to initialize
 * @return none
 */
void scenes_state_init(SCENES_STATE_T * state)
{
    if (state == NULL) {
        return;
    }
    state->selected_index = (uint8_t)SCENES_CARD_MORNING;
}

/**
 * @brief Select a card if the card index is valid
 * @param[in,out] state State object to update
 * @param[in] card Card index to select
 * @return none
 */
void scenes_state_select(SCENES_STATE_T * state, SCENES_CARD_E card)
{
    if (state == NULL) {
        return;
    }
    if (card < SCENES_CARD_HOME || card >= SCENES_CARD_COUNT) {
        return;
    }
    state->selected_index = (uint8_t)card;
}

/**
 * @brief Get the currently selected card index
 * @param[in] state State object to query
 * @return Selected card, or SCENES_CARD_MORNING if state is NULL
 */
SCENES_CARD_E scenes_state_get_selected(const SCENES_STATE_T * state)
{
    if (state == NULL) {
        return SCENES_CARD_MORNING;
    }
    if (state->selected_index >= (uint8_t)SCENES_CARD_COUNT) {
        return SCENES_CARD_MORNING;
    }
    return (SCENES_CARD_E)state->selected_index;
}
