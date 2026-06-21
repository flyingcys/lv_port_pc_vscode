/**
 * @file game_2048.h
 * @brief 2048 Game Core Logic Header
 * 
 * Core game logic for 2048 including tile management, movement, merging, and state detection
 */

#ifndef GAME_2048_H
#define GAME_2048_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/
#define GRID_SIZE 4
#define WIN_TILE_VALUE 2048

/**********************
 *      TYPEDEFS
 **********************/

/**
 * Game state enumeration
 */
typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_WON,
    GAME_STATE_LOST
} game_state_t;

/**
 * Movement direction enumeration
 */
typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} direction_t;

/**
 * Main game structure
 */
typedef struct {
    uint16_t grid[GRID_SIZE][GRID_SIZE];  // Game grid (2D array)
    uint32_t score;                        // Current score
    game_state_t state;                    // Game state (playing/won/lost)
    bool moved;                            // Flag indicating if last move was valid
} game_2048_t;

/**
 * Callback function type for UI updates
 */
typedef void (*game_update_cb_t)(void);

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the 2048 game
 * Creates the game UI and starts a new game
 */
void game_2048_init(void);

/**
 * Start a new game
 * Resets the board and spawns initial tiles
 */
void game_2048_new_game(void);

/**
 * Move tiles in the specified direction
 * @param dir Direction to move (UP/DOWN/LEFT/RIGHT)
 * @return true if move was valid and tiles moved, false otherwise
 */
bool game_2048_move(direction_t dir);

/**
 * Get the current game state
 * @return Pointer to the game state structure
 */
game_2048_t* game_2048_get_state(void);

/**
 * Check if the game is over (no valid moves remaining)
 * @return true if game is over, false if moves are still possible
 */
bool game_2048_is_game_over(void);

/**
 * Register callback for UI updates
 * @param cb Callback function to be called when game state changes
 */
void game_2048_set_update_callback(game_update_cb_t cb);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* GAME_2048_H */
