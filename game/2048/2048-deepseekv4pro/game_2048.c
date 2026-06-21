// game_2048.c - 2048 game engine implementation
#include "game_2048.h"
#include <stdlib.h>
#include <string.h>

/* ---- internal helpers (forward decls) ---- */
static void left_shift_row(uint16_t row[4], uint32_t *score);
static void add_random_tile(game_2048_t *game);
static void transpose(game_2048_t *game);
static void reverse_rows(game_2048_t *game);
static game_state_t check_state(const game_2048_t *game);

/* ---- public API ---- */

void game_2048_init(game_2048_t *game)
{
    memset(game, 0, sizeof(*game));
    game->state = STATE_PLAYING;
    /* spawn two initial tiles */
    add_random_tile(game);
    add_random_tile(game);
}

void game_2048_reset(game_2048_t *game)
{
    /* keep best_score across resets */
    uint32_t best = game->best_score;
    memset(game, 0, sizeof(*game));
    game->best_score = best;
    game->state = STATE_PLAYING;
    add_random_tile(game);
    add_random_tile(game);
}

/* ---- internal helpers ---- */

static void left_shift_row(uint16_t row[4], uint32_t *score)
{
    /* 1. strip zeros (compact left) */
    int pos = 0;
    for (int i = 0; i < 4; i++) {
        if (row[i] != 0) row[pos++] = row[i];
    }
    while (pos < 4) row[pos++] = 0;

    /* 2. merge adjacent equals */
    for (int i = 0; i < 3; i++) {
        if (row[i] != 0 && row[i] == row[i + 1]) {
            row[i] *= 2;
            *score += row[i];
            row[i + 1] = 0;
            i++; /* skip past the merged pair */
        }
    }

    /* 3. strip zeros again */
    pos = 0;
    for (int i = 0; i < 4; i++) {
        if (row[i] != 0) row[pos++] = row[i];
    }
    while (pos < 4) row[pos++] = 0;
}

static void transpose(game_2048_t *game)
{
    for (int r = 0; r < 4; r++) {
        for (int c = r + 1; c < 4; c++) {
            uint16_t tmp = game->cells[r][c];
            game->cells[r][c] = game->cells[c][r];
            game->cells[c][r] = tmp;
        }
    }
}

static void reverse_rows(game_2048_t *game)
{
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 2; c++) {
            uint16_t tmp = game->cells[r][c];
            game->cells[r][c] = game->cells[r][3 - c];
            game->cells[r][3 - c] = tmp;
        }
    }
}

static void add_random_tile(game_2048_t *game)
{
    int empty[16][2];
    int count = 0;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (game->cells[r][c] == 0) {
                empty[count][0] = r;
                empty[count][1] = c;
                count++;
            }
        }
    }
    if (count == 0) return;

    int idx = rand() % count;
    uint16_t val = (rand() % 10 == 0) ? 4 : 2;  /* 10% chance of 4 */
    game->cells[empty[idx][0]][empty[idx][1]] = val;
}

static game_state_t check_state(const game_2048_t *game)
{
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (game->cells[r][c] == 2048) return STATE_WON;
            /* check right neighbor */
            if (c < 3 && game->cells[r][c] == game->cells[r][c + 1]) return STATE_PLAYING;
            /* check bottom neighbor */
            if (r < 3 && game->cells[r][c] == game->cells[r + 1][c]) return STATE_PLAYING;
            /* empty cell exists */
            if (game->cells[r][c] == 0) return STATE_PLAYING;
        }
    }
    return STATE_LOST;
}

void game_2048_move(game_2048_t *game, game_dir_t dir)
{
    /* save old board for UI to compare */
    uint16_t old_cells[4][4];
    memcpy(old_cells, game->cells, sizeof(old_cells));

    game->moved = false;
    memset(game->merged, 0, sizeof(game->merged));

    /* transform board so we always do a LEFT shift */
    if (dir == DIR_UP) {
        transpose(game);
    } else if (dir == DIR_DOWN) {
        transpose(game);
        reverse_rows(game);
    } else if (dir == DIR_RIGHT) {
        reverse_rows(game);
    }

    /* apply LEFT shift to each row, detect movement */
    for (int r = 0; r < 4; r++) {
        uint16_t old_row[4];
        memcpy(old_row, game->cells[r], sizeof(old_row));

        left_shift_row(game->cells[r], &game->score);

        for (int c = 0; c < 4; c++) {
            if (game->cells[r][c] != old_row[c]) {
                game->moved = true;
            }
        }
    }

    /* undo transform */
    if (dir == DIR_UP) {
        transpose(game);
    } else if (dir == DIR_DOWN) {
        reverse_rows(game);
        transpose(game);
    } else if (dir == DIR_RIGHT) {
        reverse_rows(game);
    }

    /* compare old vs new to determine merged cells */
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (game->cells[r][c] != 0 && old_cells[r][c] != 0
                && game->cells[r][c] != old_cells[r][c]) {
                game->merged[r][c] = true;
            }
        }
    }

    if (game->moved) {
        add_random_tile(game);
    }

    /* update score tracking */
    if (game->score > game->best_score) {
        game->best_score = game->score;
    }

    /* check state last (preserve WON if already achieved, allow continue) */
    game_state_t new_state = check_state(game);
    if (game->state != STATE_WON) {  /* don't downgrade from WON */
        game->state = new_state;
    } else if (new_state == STATE_LOST) {
        game->state = STATE_LOST;   /* LOST always applies */
    }
}
