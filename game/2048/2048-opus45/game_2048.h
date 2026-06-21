/**
 * @file game_2048.h
 * @brief 2048 Game implementation for LVGL v9
 * @author Generated for LVGL v9.3.0
 */

#ifndef GAME_2048_H
#define GAME_2048_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define GAME_2048_GRID_SIZE     4       /* 4x4 grid */
#define GAME_2048_WIN_VALUE     2048    /* Win condition */

/**********************
 *      TYPEDEFS
 **********************/

/**
 * @brief Game state enumeration
 */
typedef enum {
    GAME_2048_STATE_PLAYING,    /* Game in progress */
    GAME_2048_STATE_WON,        /* Player reached 2048 */
    GAME_2048_STATE_OVER,       /* No more moves */
    GAME_2048_STATE_CONTINUE,   /* Won but continuing */
} game_2048_state_t;

/**
 * @brief Move direction enumeration
 */
typedef enum {
    GAME_2048_DIR_UP,
    GAME_2048_DIR_DOWN,
    GAME_2048_DIR_LEFT,
    GAME_2048_DIR_RIGHT,
} game_2048_dir_t;

/**
 * @brief 2048 Game structure
 */
typedef struct {
    /* Game data */
    uint16_t grid[GAME_2048_GRID_SIZE][GAME_2048_GRID_SIZE];  /* Cell values */
    uint32_t score;              /* Current score */
    uint32_t best_score;         /* Best score */
    game_2048_state_t state;     /* Game state */
    
    /* UI elements */
    lv_obj_t *main_container;    /* Main container */
    lv_obj_t *board;             /* Game board container */
    lv_obj_t *cells[GAME_2048_GRID_SIZE][GAME_2048_GRID_SIZE]; /* Cell objects */
    lv_obj_t *score_value_label; /* Score display */
    lv_obj_t *best_value_label;  /* Best score display */
    lv_obj_t *overlay;           /* Game over/win overlay */
    lv_obj_t *overlay_label;     /* Overlay message */
    
    /* Layout configuration */
    int32_t cell_size;           /* Cell size in pixels */
    int32_t cell_gap;            /* Gap between cells */
    int32_t board_padding;       /* Board padding */
} game_2048_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Create and initialize the 2048 game
 * @param parent Parent object to attach the game
 * @return Pointer to game structure, or NULL on failure
 */
game_2048_t *game_2048_create(lv_obj_t *parent);

/**
 * @brief Reset the game to initial state
 * @param game Pointer to game structure
 */
void game_2048_reset(game_2048_t *game);

/**
 * @brief Move tiles in specified direction
 * @param game Pointer to game structure
 * @param dir Direction to move
 * @return true if any tiles moved, false otherwise
 */
bool game_2048_move(game_2048_t *game, game_2048_dir_t dir);

/**
 * @brief Get current game state
 * @param game Pointer to game structure
 * @return Current game state
 */
game_2048_state_t game_2048_get_state(game_2048_t *game);

/**
 * @brief Get current score
 * @param game Pointer to game structure
 * @return Current score
 */
uint32_t game_2048_get_score(game_2048_t *game);

/**
 * @brief Continue playing after winning
 * @param game Pointer to game structure
 */
void game_2048_continue(game_2048_t *game);

/**
 * @brief Delete the game and free resources
 * @param game Pointer to game structure
 */
void game_2048_delete(game_2048_t *game);

#ifdef __cplusplus
}
#endif

#endif /* GAME_2048_H */
