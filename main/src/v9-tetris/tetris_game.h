/**
 * @file tetris_game.h
 * @brief Tetris game engine header - Core game logic and data structures
 */

#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

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
#define BOARD_WIDTH     10
#define BOARD_HEIGHT    20
#define TETROMINO_SIZE  4

/* Tetromino types */
#define TETROMINO_I     0
#define TETROMINO_J     1
#define TETROMINO_L     2
#define TETROMINO_O     3
#define TETROMINO_S     4
#define TETROMINO_T     5
#define TETROMINO_Z     6
#define TETROMINO_COUNT 7

/* Rotation states */
#define ROTATION_0      0
#define ROTATION_90     1
#define ROTATION_180    2
#define ROTATION_270    3
#define ROTATION_COUNT  4

/* Game timing (in milliseconds) */
#define INITIAL_FALL_SPEED  1000
#define SPEED_DECREASE      100
#define LINES_PER_LEVEL     10
#define MIN_FALL_SPEED      100

/* Scoring */
#define SCORE_1_LINE    100
#define SCORE_2_LINES   300
#define SCORE_3_LINES   600
#define SCORE_4_LINES   1000

/**********************
 *      TYPEDEFS
 **********************/

/**
 * @brief Game state enumeration
 */
typedef enum {
    GAME_STATE_IDLE = 0,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAME_OVER
} tetris_game_state_t;

/**
 * @brief Tetromino piece structure
 */
typedef struct {
    uint8_t type;           /* Tetromino type (I, J, L, O, S, T, Z) */
    uint8_t rotation;       /* Current rotation state (0-3) */
    int8_t x;               /* X position on board */
    int8_t y;               /* Y position on board */
} tetris_piece_t;

/**
 * @brief Game statistics structure
 */
typedef struct {
    uint32_t score;         /* Current score */
    uint32_t lines;         /* Total lines cleared */
    uint32_t level;         /* Current level */
    uint32_t fall_speed;    /* Current fall speed in ms */
} tetris_stats_t;

/**
 * @brief Main game structure
 */
typedef struct {
    uint8_t board[BOARD_HEIGHT][BOARD_WIDTH];  /* Game board (0=empty, 1-7=piece type) */
    tetris_piece_t current_piece;               /* Currently falling piece */
    tetris_piece_t next_piece;                  /* Next piece to spawn */
    tetris_stats_t stats;                       /* Game statistics */
    tetris_game_state_t state;                  /* Current game state */
    uint32_t last_fall_time;                    /* Last time piece fell (for timing) */
    bool soft_drop_active;                      /* Is soft drop key held? */
} tetris_game_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Initialize the Tetris game
 * @param game Pointer to game structure
 */
void tetris_game_init(tetris_game_t *game);

/**
 * @brief Start a new game
 * @param game Pointer to game structure
 */
void tetris_game_start(tetris_game_t *game);

/**
 * @brief Update game state (call periodically)
 * @param game Pointer to game structure
 * @param current_time Current time in milliseconds
 */
void tetris_game_update(tetris_game_t *game, uint32_t current_time);

/**
 * @brief Pause/Resume the game
 * @param game Pointer to game structure
 */
void tetris_game_toggle_pause(tetris_game_t *game);

/**
 * @brief Move piece left
 * @param game Pointer to game structure
 * @return true if move was successful
 */
bool tetris_game_move_left(tetris_game_t *game);

/**
 * @brief Move piece right
 * @param game Pointer to game structure
 * @return true if move was successful
 */
bool tetris_game_move_right(tetris_game_t *game);

/**
 * @brief Rotate piece clockwise
 * @param game Pointer to game structure
 * @return true if rotation was successful
 */
bool tetris_game_rotate(tetris_game_t *game);

/**
 * @brief Enable/disable soft drop
 * @param game Pointer to game structure
 * @param active true to enable soft drop
 */
void tetris_game_set_soft_drop(tetris_game_t *game, bool active);

/**
 * @brief Get tetromino shape data
 * @param type Tetromino type
 * @param rotation Rotation state
 * @return Pointer to 4x4 shape matrix
 */
const uint8_t (*tetris_get_shape(uint8_t type, uint8_t rotation))[TETROMINO_SIZE];

/**
 * @brief Get color for tetromino type
 * @param type Tetromino type
 * @param r Pointer to red component (0-255)
 * @param g Pointer to green component (0-255)
 * @param b Pointer to blue component (0-255)
 */
void tetris_get_color(uint8_t type, uint8_t *r, uint8_t *g, uint8_t *b);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TETRIS_GAME_H */
