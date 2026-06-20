/**
 * @file tetris_game.c
 * @brief Tetris game engine implementation - Core game logic
 */

/*********************
 *      INCLUDES
 *********************/
#include "tetris_game.h"
#include <string.h>
#include <stdlib.h>
#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define SOFT_DROP_SPEED 50  /* Fast drop speed in ms */

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void spawn_piece(tetris_game_t *game, tetris_piece_t *piece);
static bool check_collision(tetris_game_t *game, tetris_piece_t *piece);
static void lock_piece(tetris_game_t *game);
static uint32_t clear_lines(tetris_game_t *game);
static void update_score(tetris_game_t *game, uint32_t lines_cleared);
static bool is_game_over(tetris_game_t *game);

/**********************
 *  STATIC VARIABLES
 **********************/

/* Tetromino shape definitions - 4x4 matrices for each piece and rotation */
static const uint8_t tetromino_shapes[TETROMINO_COUNT][ROTATION_COUNT][TETROMINO_SIZE][TETROMINO_SIZE] = {
    /* I piece */
    {
        {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0}},
        {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}}
    },
    /* J piece */
    {
        {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {0,0,1,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,0,0}}
    },
    /* L piece */
    {
        {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {1,0,0,0}, {0,0,0,0}},
        {{1,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    /* O piece */
    {
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}
    },
    /* S piece */
    {
        {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,0,0}},
        {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
        {{1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    /* T piece */
    {
        {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    /* Z piece */
    {
        {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,1,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
        {{0,1,0,0}, {1,1,0,0}, {1,0,0,0}, {0,0,0,0}}
    }
};

/* Tetromino colors (RGB) */
static const uint8_t tetromino_colors[TETROMINO_COUNT][3] = {
    {0, 255, 255},   /* I - Cyan */
    {0, 0, 255},     /* J - Blue */
    {255, 165, 0},   /* L - Orange */
    {255, 255, 0},   /* O - Yellow */
    {0, 255, 0},     /* S - Green */
    {160, 32, 240},  /* T - Purple */
    {255, 0, 0}      /* Z - Red */
};

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void tetris_game_init(tetris_game_t *game)
{
    memset(game, 0, sizeof(tetris_game_t));
    game->state = GAME_STATE_IDLE;
    game->stats.fall_speed = INITIAL_FALL_SPEED;
}

void tetris_game_start(tetris_game_t *game)
{
    /* Clear board */
    memset(game->board, 0, sizeof(game->board));
    
    /* Reset statistics */
    game->stats.score = 0;
    game->stats.lines = 0;
    game->stats.level = 1;
    game->stats.fall_speed = INITIAL_FALL_SPEED;
    
    /* Spawn first pieces */
    spawn_piece(game, &game->next_piece);
    spawn_piece(game, &game->current_piece);
    
    /* Set state */
    game->state = GAME_STATE_PLAYING;
    game->last_fall_time = lv_tick_get();
    game->soft_drop_active = false;
}

void tetris_game_update(tetris_game_t *game, uint32_t current_time)
{
    if (game->state != GAME_STATE_PLAYING) {
        return;
    }
    
    /* Calculate fall speed (use soft drop speed if active) */
    uint32_t fall_speed = game->soft_drop_active ? SOFT_DROP_SPEED : game->stats.fall_speed;
    
    /* Check if it's time to move piece down */
    if (current_time - game->last_fall_time >= fall_speed) {
        game->last_fall_time = current_time;
        
        /* Try to move piece down */
        tetris_piece_t test_piece = game->current_piece;
        test_piece.y++;
        
        if (check_collision(game, &test_piece)) {
            /* Collision - lock piece and spawn new one */
            lock_piece(game);
            
            /* Clear completed lines */
            uint32_t lines_cleared = clear_lines(game);
            if (lines_cleared > 0) {
                update_score(game, lines_cleared);
            }
            
            /* Spawn next piece */
            game->current_piece = game->next_piece;
            spawn_piece(game, &game->next_piece);
            
            /* Check for game over */
            if (is_game_over(game)) {
                game->state = GAME_STATE_GAME_OVER;
            }
        } else {
            /* Move piece down */
            game->current_piece.y++;
        }
    }
}

void tetris_game_toggle_pause(tetris_game_t *game)
{
    if (game->state == GAME_STATE_PLAYING) {
        game->state = GAME_STATE_PAUSED;
    } else if (game->state == GAME_STATE_PAUSED) {
        game->state = GAME_STATE_PLAYING;
        game->last_fall_time = lv_tick_get(); /* Reset timer to prevent immediate fall */
    }
}

bool tetris_game_move_left(tetris_game_t *game)
{
    if (game->state != GAME_STATE_PLAYING) {
        return false;
    }
    
    tetris_piece_t test_piece = game->current_piece;
    test_piece.x--;
    
    if (!check_collision(game, &test_piece)) {
        game->current_piece.x--;
        return true;
    }
    
    return false;
}

bool tetris_game_move_right(tetris_game_t *game)
{
    if (game->state != GAME_STATE_PLAYING) {
        return false;
    }
    
    tetris_piece_t test_piece = game->current_piece;
    test_piece.x++;
    
    if (!check_collision(game, &test_piece)) {
        game->current_piece.x++;
        return true;
    }
    
    return false;
}

bool tetris_game_rotate(tetris_game_t *game)
{
    if (game->state != GAME_STATE_PLAYING) {
        return false;
    }
    
    tetris_piece_t test_piece = game->current_piece;
    test_piece.rotation = (test_piece.rotation + 1) % ROTATION_COUNT;
    
    if (!check_collision(game, &test_piece)) {
        game->current_piece.rotation = test_piece.rotation;
        return true;
    }
    
    return false;
}

void tetris_game_set_soft_drop(tetris_game_t *game, bool active)
{
    game->soft_drop_active = active;
}

const uint8_t (*tetris_get_shape(uint8_t type, uint8_t rotation))[TETROMINO_SIZE]
{
    if (type >= TETROMINO_COUNT || rotation >= ROTATION_COUNT) {
        return NULL;
    }
    return tetromino_shapes[type][rotation];
}

void tetris_get_color(uint8_t type, uint8_t *r, uint8_t *g, uint8_t *b)
{
    if (type == 0 || type > TETROMINO_COUNT) {
        *r = *g = *b = 50; /* Dark gray for empty cells */
        return;
    }
    
    *r = tetromino_colors[type - 1][0];
    *g = tetromino_colors[type - 1][1];
    *b = tetromino_colors[type - 1][2];
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void spawn_piece(tetris_game_t *game, tetris_piece_t *piece)
{
    piece->type = (lv_rand(0, TETROMINO_COUNT - 1)) % TETROMINO_COUNT;
    piece->rotation = ROTATION_0;
    piece->x = BOARD_WIDTH / 2 - 2;
    piece->y = 0;
}

static bool check_collision(tetris_game_t *game, tetris_piece_t *piece)
{
    const uint8_t (*shape)[TETROMINO_SIZE] = tetris_get_shape(piece->type, piece->rotation);
    
    if (shape == NULL) {
        return true; /* Invalid piece */
    }
    
    for (int y = 0; y < TETROMINO_SIZE; y++) {
        for (int x = 0; x < TETROMINO_SIZE; x++) {
            if (shape[y][x]) {
                int board_x = piece->x + x;
                int board_y = piece->y + y;
                
                /* Check boundaries */
                if (board_x < 0 || board_x >= BOARD_WIDTH || 
                    board_y < 0 || board_y >= BOARD_HEIGHT) {
                    return true;
                }
                
                /* Check collision with placed pieces */
                if (game->board[board_y][board_x] != 0) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

static void lock_piece(tetris_game_t *game)
{
    const uint8_t (*shape)[TETROMINO_SIZE] = tetris_get_shape(
        game->current_piece.type, 
        game->current_piece.rotation
    );
    
    if (shape == NULL) {
        return;
    }
    
    for (int y = 0; y < TETROMINO_SIZE; y++) {
        for (int x = 0; x < TETROMINO_SIZE; x++) {
            if (shape[y][x]) {
                int board_x = game->current_piece.x + x;
                int board_y = game->current_piece.y + y;
                
                if (board_x >= 0 && board_x < BOARD_WIDTH && 
                    board_y >= 0 && board_y < BOARD_HEIGHT) {
                    /* Store piece type + 1 (0 is empty) */
                    game->board[board_y][board_x] = game->current_piece.type + 1;
                }
            }
        }
    }
}

static uint32_t clear_lines(tetris_game_t *game)
{
    uint32_t lines_cleared = 0;
    
    /* Check each row from bottom to top */
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        bool line_full = true;
        
        /* Check if line is full */
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (game->board[y][x] == 0) {
                line_full = false;
                break;
            }
        }
        
        if (line_full) {
            lines_cleared++;
            
            /* Shift all rows above down */
            for (int shift_y = y; shift_y > 0; shift_y--) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    game->board[shift_y][x] = game->board[shift_y - 1][x];
                }
            }
            
            /* Clear top row */
            for (int x = 0; x < BOARD_WIDTH; x++) {
                game->board[0][x] = 0;
            }
            
            /* Check same row again (it now contains the row above) */
            y++;
        }
    }
    
    return lines_cleared;
}

static void update_score(tetris_game_t *game, uint32_t lines_cleared)
{
    /* Update lines count */
    game->stats.lines += lines_cleared;
    
    /* Calculate score based on lines cleared */
    switch (lines_cleared) {
        case 1:
            game->stats.score += SCORE_1_LINE;
            break;
        case 2:
            game->stats.score += SCORE_2_LINES;
            break;
        case 3:
            game->stats.score += SCORE_3_LINES;
            break;
        case 4:
            game->stats.score += SCORE_4_LINES;
            break;
        default:
            break;
    }
    
    /* Update level and speed */
    uint32_t new_level = (game->stats.lines / LINES_PER_LEVEL) + 1;
    if (new_level > game->stats.level) {
        game->stats.level = new_level;
        
        /* Increase speed (decrease fall time) */
        if (game->stats.fall_speed > MIN_FALL_SPEED) {
            game->stats.fall_speed -= SPEED_DECREASE;
            if (game->stats.fall_speed < MIN_FALL_SPEED) {
                game->stats.fall_speed = MIN_FALL_SPEED;
            }
        }
    }
}

static bool is_game_over(tetris_game_t *game)
{
    /* Check if the newly spawned piece collides immediately */
    return check_collision(game, &game->current_piece);
}
