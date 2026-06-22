#include "tetris.h"

#include <string.h>

#include "tetris_game.h"
#include "tetris_input.h"
#include "tetris_ui.h"

#define TETRIS_TICK_PERIOD_MS 50

static tetris_game_t s_game;
static tetris_ui_t s_ui;
static lv_obj_t *s_root;
static lv_timer_t *s_timer;
static bool s_created;

static void tetris_tick_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    if(!s_created) return;

    tetris_game_update(&s_game, lv_tick_get());
    tetris_ui_update(&s_ui);
}

lv_obj_t * tetris_create(lv_obj_t *parent, int32_t screen_w, int32_t screen_h)
{
    if(s_created && s_root != NULL) {
        return s_root;
    }

    if(parent == NULL) {
        parent = lv_screen_active();
    }

    memset(&s_game, 0, sizeof(s_game));
    memset(&s_ui, 0, sizeof(s_ui));

    s_root = lv_obj_create(parent);
    if(s_root == NULL) return NULL;

    lv_obj_remove_style_all(s_root);
    lv_obj_set_pos(s_root, 0, 0);
    lv_obj_set_size(s_root, screen_w, screen_h);
    lv_obj_clear_flag(s_root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(s_root, LV_OBJ_FLAG_CLICKABLE);

    tetris_game_init(&s_game);
    tetris_ui_init(&s_ui, &s_game, s_root, screen_w, screen_h);
    tetris_input_init(&s_game, &s_ui, s_root);

    s_timer = lv_timer_create(tetris_tick_cb, TETRIS_TICK_PERIOD_MS, NULL);
    if(s_timer != NULL) {
        lv_timer_pause(s_timer);
    }

    s_created = true;
    tetris_ui_update(&s_ui);
    return s_root;
}

void tetris_start(void)
{
    if(!s_created) return;

    tetris_game_start(&s_game);
    tetris_ui_update(&s_ui);
    if(s_timer != NULL) {
        lv_timer_resume(s_timer);
    }
}

void tetris_stop(void)
{
    if(s_timer != NULL) {
        lv_timer_delete(s_timer);
        s_timer = NULL;
    }

    if(s_root != NULL) {
        lv_obj_delete(s_root);
        s_root = NULL;
    }

    if(s_created) {
        tetris_ui_cleanup(&s_ui);
    }

    memset(&s_game, 0, sizeof(s_game));
    memset(&s_ui, 0, sizeof(s_ui));
    s_created = false;
}
