/**
 * @file game_2048.c
 * @brief 2048 Game Core Logic Implementation
 */

/*********************
 *      INCLUDES
 *********************/
#include "game_2048.h"
#include "game_2048_ui.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void spawn_random_tile(void);
static bool can_move(void);
static bool move_tiles(direction_t dir);
static void merge_tiles(direction_t dir);
static void compress_tiles(direction_t dir);

/**********************
 *  STATIC VARIABLES
 **********************/
static game_2048_t game;
static game_update_cb_t update_callback = NULL;
static bool random_initialized = false;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void game_2048_init(void)
{
    // Initialize random seed
    if (!random_initialized) {
        srand((unsigned int)time(NULL));
        random_initialized = true;
    }
    
    // Create UI
    game_2048_ui_init();
    
    // Start new game
    game_2048_new_game();
}

void game_2048_new_game(void)
{
    // Clear the grid
    memset(&game, 0, sizeof(game_2048_t));
    
    // Set initial state
    game.state = GAME_STATE_PLAYING;
    game.score = 0;
    game.moved = false;
    
    // Spawn two initial tiles
    spawn_random_tile();
    spawn_random_tile();
    
    // Update UI
    if (update_callback) {
        update_callback();
    }
}

bool game_2048_move(direction_t dir)
{
    if (game.state != GAME_STATE_PLAYING) {
        return false;
    }
    
    // Save grid state to detect changes
    uint16_t old_grid[GRID_SIZE][GRID_SIZE];
    memcpy(old_grid, game.grid, sizeof(game.grid));
    
    game.moved = false;
    
    // Execute move
    move_tiles(dir);
    
    // Check if anything changed
    if (memcmp(old_grid, game.grid, sizeof(game.grid)) != 0) {
        game.moved = true;
        
        // Spawn new tile after successful move
        spawn_random_tile();
        
        // Check for win condition
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                if (game.grid[i][j] >= WIN_TILE_VALUE && game.state == GAME_STATE_PLAYING) {
                    game.state = GAME_STATE_WON;
                }
            }
        }
        
        // Check for lose condition
        if (game_2048_is_game_over()) {
            game.state = GAME_STATE_LOST;
        }
        
        // Update UI
        if (update_callback) {
            update_callback();
        }
    }
    
    return game.moved;
}

game_2048_t* game_2048_get_state(void)
{
    return &game;
}

bool game_2048_is_game_over(void)
{
    // Check if any empty cells exist
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game.grid[i][j] == 0) {
                return false;
            }
        }
    }
    
    // Check if any adjacent cells can be merged
    return !can_move();
}

void game_2048_set_update_callback(game_update_cb_t cb)
{
    update_callback = cb;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Spawn a random tile (2 or 4) at an empty position
 */
static void spawn_random_tile(void)
{
    // Find all empty cells
    int empty_cells[GRID_SIZE * GRID_SIZE][2];
    int empty_count = 0;
    
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game.grid[i][j] == 0) {
                empty_cells[empty_count][0] = i;
                empty_cells[empty_count][1] = j;
                empty_count++;
            }
        }
    }
    
    if (empty_count == 0) {
        return;
    }
    
    // Choose random empty cell
    int idx = rand() % empty_count;
    int row = empty_cells[idx][0];
    int col = empty_cells[idx][1];
    
    // Spawn tile (90% chance of 2, 10% chance of 4)
    game.grid[row][col] = (rand() % 10 == 0) ? 4 : 2;
}

/**
 * Check if any move is possible
 */
static bool can_move(void)
{
    // Check horizontal adjacent cells
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE - 1; j++) {
            if (game.grid[i][j] == game.grid[i][j + 1]) {
                return true;
            }
        }
    }
    
    // Check vertical adjacent cells
    for (int i = 0; i < GRID_SIZE - 1; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game.grid[i][j] == game.grid[i + 1][j]) {
                return true;
            }
        }
    }
    
    return false;
}

/**
 * Compress tiles (remove zeros) in the specified direction
 */
static void compress_tiles(direction_t dir)
{
    uint16_t temp[GRID_SIZE];
    
    if (dir == DIR_LEFT || dir == DIR_RIGHT) {
        for (int i = 0; i < GRID_SIZE; i++) {
            int pos = 0;
            
            // Collect non-zero tiles
            if (dir == DIR_LEFT) {
                for (int j = 0; j < GRID_SIZE; j++) {
                    if (game.grid[i][j] != 0) {
                        temp[pos++] = game.grid[i][j];
                    }
                }
                // Fill remaining with zeros
                while (pos < GRID_SIZE) {
                    temp[pos++] = 0;
                }
            } else { // DIR_RIGHT
                for (int j = GRID_SIZE - 1; j >= 0; j--) {
                    if (game.grid[i][j] != 0) {
                        temp[pos++] = game.grid[i][j];
                    }
                }
                // Reverse and fill
                for (int j = 0; j < GRID_SIZE; j++) {
                    game.grid[i][j] = (j < GRID_SIZE - pos) ? 0 : temp[GRID_SIZE - 1 - j];
                }
                continue;
            }
            
            // Copy back
            for (int j = 0; j < GRID_SIZE; j++) {
                game.grid[i][j] = temp[j];
            }
        }
    } else { // DIR_UP or DIR_DOWN
        for (int j = 0; j < GRID_SIZE; j++) {
            int pos = 0;
            
            // Collect non-zero tiles
            if (dir == DIR_UP) {
                for (int i = 0; i < GRID_SIZE; i++) {
                    if (game.grid[i][j] != 0) {
                        temp[pos++] = game.grid[i][j];
                    }
                }
                // Fill remaining with zeros
                while (pos < GRID_SIZE) {
                    temp[pos++] = 0;
                }
            } else { // DIR_DOWN
                for (int i = GRID_SIZE - 1; i >= 0; i--) {
                    if (game.grid[i][j] != 0) {
                        temp[pos++] = game.grid[i][j];
                    }
                }
                // Reverse and fill
                for (int i = 0; i < GRID_SIZE; i++) {
                    game.grid[i][j] = (i < GRID_SIZE - pos) ? 0 : temp[GRID_SIZE - 1 - i];
                }
                continue;
            }
            
            // Copy back
            for (int i = 0; i < GRID_SIZE; i++) {
                game.grid[i][j] = temp[i];
            }
        }
    }
}

/**
 * Merge adjacent tiles with same value in the specified direction
 */
static void merge_tiles(direction_t dir)
{
    if (dir == DIR_LEFT) {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE - 1; j++) {
                if (game.grid[i][j] != 0 && game.grid[i][j] == game.grid[i][j + 1]) {
                    game.grid[i][j] *= 2;
                    game.score += game.grid[i][j];
                    game.grid[i][j + 1] = 0;
                }
            }
        }
    } else if (dir == DIR_RIGHT) {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = GRID_SIZE - 1; j > 0; j--) {
                if (game.grid[i][j] != 0 && game.grid[i][j] == game.grid[i][j - 1]) {
                    game.grid[i][j] *= 2;
                    game.score += game.grid[i][j];
                    game.grid[i][j - 1] = 0;
                }
            }
        }
    } else if (dir == DIR_UP) {
        for (int j = 0; j < GRID_SIZE; j++) {
            for (int i = 0; i < GRID_SIZE - 1; i++) {
                if (game.grid[i][j] != 0 && game.grid[i][j] == game.grid[i + 1][j]) {
                    game.grid[i][j] *= 2;
                    game.score += game.grid[i][j];
                    game.grid[i + 1][j] = 0;
                }
            }
        }
    } else { // DIR_DOWN
        for (int j = 0; j < GRID_SIZE; j++) {
            for (int i = GRID_SIZE - 1; i > 0; i--) {
                if (game.grid[i][j] != 0 && game.grid[i][j] == game.grid[i - 1][j]) {
                    game.grid[i][j] *= 2;
                    game.score += game.grid[i][j];
                    game.grid[i - 1][j] = 0;
                }
            }
        }
    }
}

/**
 * Execute a move in the specified direction
 */
static bool move_tiles(direction_t dir)
{
    // Compress tiles to remove gaps
    compress_tiles(dir);
    
    // Merge adjacent tiles
    merge_tiles(dir);
    
    // Compress again after merging
    compress_tiles(dir);
    
    return true;
}
