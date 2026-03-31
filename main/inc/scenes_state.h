/**
 * @file scenes_state.h
 * @brief Scenes page card selection state
 * @version 1.0
 * @date 2026-03-31
 * @copyright Copyright (c) 2026
 */
#ifndef __SCENES_STATE_H__
#define __SCENES_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Type definitions
 * --------------------------------------------------------------------------- */
/**
 * @brief Card indices used by the scenes page
 */
typedef enum {
    SCENES_CARD_HOME = 0,
    SCENES_CARD_LEAVING_HOME,
    SCENES_CARD_MORNING,
    SCENES_CARD_ALL_LIGHTS_ON,
    SCENES_CARD_COUNT
} SCENES_CARD_E;

/**
 * @brief Runtime selection state for the scenes page
 */
typedef struct {
    uint8_t selected_index;
} SCENES_STATE_T;

/* ---------------------------------------------------------------------------
 * Function declarations
 * --------------------------------------------------------------------------- */
/**
 * @brief Initialize scenes selection state
 * @param[out] state State object to initialize
 * @return none
 */
void scenes_state_init(SCENES_STATE_T * state);

/**
 * @brief Select a card if the card index is valid
 * @param[in,out] state State object to update
 * @param[in] card Card index to select
 * @return none
 */
void scenes_state_select(SCENES_STATE_T * state, SCENES_CARD_E card);

/**
 * @brief Get the currently selected card index
 * @param[in] state State object to query
 * @return Selected card, or SCENES_CARD_MORNING if state is NULL
 */
SCENES_CARD_E scenes_state_get_selected(const SCENES_STATE_T * state);

#ifdef __cplusplus
}
#endif

#endif /* __SCENES_STATE_H__ */
