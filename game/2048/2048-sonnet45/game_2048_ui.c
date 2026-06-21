/**
 * @file game_2048_ui.c
 * @brief 2048 Game UI Implementation
 */

/*********************
 *      INCLUDES
 *********************/
#include "game_2048_ui.h"
#include "game_2048.h"
#include "game_2048_input.h"
#include <stdio.h>

/*********************
 *      DEFINES
 *********************/
#define TILE_SIZE 70
#define TILE_PADDING 8
#define ANIM_TIME 150

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void create_ui(void);
static void update_tiles(void);
static void update_score(void);
static void show_game_status(void);
static void reset_btn_event_cb(lv_event_t * e);
static uint32_t get_tile_color(uint16_t value);
static uint32_t get_tile_text_color(uint16_t value);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * main_container = NULL;
static lv_obj_t * grid_container = NULL;
static lv_obj_t * tile_labels[GRID_SIZE][GRID_SIZE];
static lv_obj_t * score_label = NULL;
static lv_obj_t * status_label = NULL;
static lv_obj_t * reset_btn = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void game_2048_ui_init(void)
{
    create_ui();
    game_2048_set_update_callback(game_2048_ui_update);
    // Pass grid container for better mouse gesture detection
    game_2048_input_init(grid_container);
}

void game_2048_ui_update(void)
{
    update_tiles();
    update_score();
    show_game_status();
}

lv_obj_t* game_2048_ui_get_container(void)
{
    return main_container;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void create_ui(void)
{
    lv_obj_t * scr = lv_screen_active();
    
    // Main container
    main_container = lv_obj_create(scr);
    lv_obj_set_size(main_container, LV_PCT(100), LV_PCT(100));
    lv_obj_center(main_container);
    lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(main_container, lv_color_hex(0xFAF8EF), 0);
    lv_obj_set_style_pad_all(main_container, 10, 0);
    lv_obj_set_style_pad_gap(main_container, 10, 0);
    
    // Header container (title, score, reset button)
    lv_obj_t * header = lv_obj_create(main_container);
    lv_obj_set_size(header, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 5, 0);
    
    // Title
    lv_obj_t * title = lv_label_create(header);
    lv_label_set_text(title, "2048");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x776E65), 0);
    
    // Score container
    lv_obj_t * score_cont = lv_obj_create(header);
    lv_obj_set_size(score_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(score_cont, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_border_width(score_cont, 0, 0);
    lv_obj_set_style_radius(score_cont, 5, 0);
    lv_obj_set_style_pad_all(score_cont, 8, 0);
    
    lv_obj_t * score_title = lv_label_create(score_cont);
    lv_label_set_text(score_title, "SCORE");
    lv_obj_set_style_text_color(score_title, lv_color_hex(0xEEE4DA), 0);
    lv_obj_set_style_text_font(score_title, &lv_font_montserrat_12, 0);
    lv_obj_align(score_title, LV_ALIGN_TOP_MID, 0, 0);
    
    score_label = lv_label_create(score_cont);
    lv_label_set_text(score_label, "0");
    lv_obj_set_style_text_color(score_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(score_label, &lv_font_montserrat_20, 0);
    lv_obj_align(score_label, LV_ALIGN_BOTTOM_MID, 0, 0);
    
    // Reset button
    reset_btn = lv_button_create(header);
    lv_obj_set_size(reset_btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(reset_btn, lv_color_hex(0x8F7A66), 0);
    lv_obj_set_style_radius(reset_btn, 5, 0);
    lv_obj_add_event_cb(reset_btn, reset_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t * reset_label = lv_label_create(reset_btn);
    lv_label_set_text(reset_label, "New Game");
    lv_obj_set_style_text_color(reset_label, lv_color_hex(0xF9F6F2), 0);
    lv_obj_center(reset_label);
    
    // Grid container
    grid_container = lv_obj_create(main_container);
    lv_obj_set_size(grid_container, GRID_SIZE * TILE_SIZE + (GRID_SIZE + 1) * TILE_PADDING,
                                     GRID_SIZE * TILE_SIZE + (GRID_SIZE + 1) * TILE_PADDING);
    lv_obj_set_style_bg_color(grid_container, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_border_width(grid_container, 0, 0);
    lv_obj_set_style_radius(grid_container, 5, 0);
    lv_obj_set_style_pad_all(grid_container, TILE_PADDING, 0);
    lv_obj_set_style_pad_gap(grid_container, TILE_PADDING, 0);
    
    // Create tile grid
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            // Background tile
            lv_obj_t * bg_tile = lv_obj_create(grid_container);
            lv_obj_set_size(bg_tile, TILE_SIZE, TILE_SIZE);
            lv_obj_set_style_bg_color(bg_tile, lv_color_hex(0xCDC1B4), 0);
            lv_obj_set_style_border_width(bg_tile, 0, 0);
            lv_obj_set_style_radius(bg_tile, 3, 0);
            lv_obj_set_pos(bg_tile, j * (TILE_SIZE + TILE_PADDING), i * (TILE_SIZE + TILE_PADDING));
            // Prevent bg_tile from blocking events
            lv_obj_remove_flag(bg_tile, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(bg_tile, LV_OBJ_FLAG_EVENT_BUBBLE);
            
            // Tile label (will show tile value)
            tile_labels[i][j] = lv_label_create(grid_container);
            lv_obj_set_size(tile_labels[i][j], TILE_SIZE, TILE_SIZE);
            lv_obj_set_pos(tile_labels[i][j], j * (TILE_SIZE + TILE_PADDING), i * (TILE_SIZE + TILE_PADDING));
            lv_obj_set_style_text_align(tile_labels[i][j], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_font(tile_labels[i][j], &lv_font_montserrat_24, 0);
            lv_obj_set_style_bg_opa(tile_labels[i][j], LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(tile_labels[i][j], 0, 0);
            lv_obj_set_style_radius(tile_labels[i][j], 3, 0);
            lv_obj_set_style_pad_top(tile_labels[i][j], 20, 0);
            lv_label_set_text(tile_labels[i][j], "");
            // Prevent labels from blocking events
            lv_obj_remove_flag(tile_labels[i][j], LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(tile_labels[i][j], LV_OBJ_FLAG_EVENT_BUBBLE);
        }
    }
    
    // Status label (for win/lose messages)
    status_label = lv_label_create(main_container);
    lv_label_set_text(status_label, "");
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x776E65), 0);
}

static void update_tiles(void)
{
    game_2048_t * game = game_2048_get_state();
    
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            uint16_t value = game->grid[i][j];
            
            if (value == 0) {
                lv_label_set_text(tile_labels[i][j], "");
                lv_obj_set_style_bg_opa(tile_labels[i][j], LV_OPA_TRANSP, 0);
            } else {
                char buf[8];
                snprintf(buf, sizeof(buf), "%u", value);
                lv_label_set_text(tile_labels[i][j], buf);
                lv_obj_set_style_bg_opa(tile_labels[i][j], LV_OPA_COVER, 0);
                lv_obj_set_style_bg_color(tile_labels[i][j], lv_color_hex(get_tile_color(value)), 0);
                lv_obj_set_style_text_color(tile_labels[i][j], lv_color_hex(get_tile_text_color(value)), 0);
                
                // Adjust font size for larger numbers
                if (value >= 1000) {
                    lv_obj_set_style_text_font(tile_labels[i][j], &lv_font_montserrat_18, 0);
                    lv_obj_set_style_pad_top(tile_labels[i][j], 24, 0);
                } else if (value >= 100) {
                    lv_obj_set_style_text_font(tile_labels[i][j], &lv_font_montserrat_20, 0);
                    lv_obj_set_style_pad_top(tile_labels[i][j], 22, 0);
                } else {
                    lv_obj_set_style_text_font(tile_labels[i][j], &lv_font_montserrat_24, 0);
                    lv_obj_set_style_pad_top(tile_labels[i][j], 20, 0);
                }
            }
        }
    }
}

static void update_score(void)
{
    game_2048_t * game = game_2048_get_state();
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)game->score);
    lv_label_set_text(score_label, buf);
}

static void show_game_status(void)
{
    game_2048_t * game = game_2048_get_state();
    
    if (game->state == GAME_STATE_WON) {
        lv_label_set_text(status_label, "🎉 You Win! 🎉");
        lv_obj_set_style_text_color(status_label, lv_color_hex(0x00AA00), 0);
    } else if (game->state == GAME_STATE_LOST) {
        lv_label_set_text(status_label, "Game Over!");
        lv_obj_set_style_text_color(status_label, lv_color_hex(0xCC0000), 0);
    } else {
        lv_label_set_text(status_label, "");
    }
}

static void reset_btn_event_cb(lv_event_t * e)
{
    (void)e;
    game_2048_new_game();
}

static uint32_t get_tile_color(uint16_t value)
{
    switch (value) {
        case 2:    return 0xEEE4DA;
        case 4:    return 0xEDE0C8;
        case 8:    return 0xF2B179;
        case 16:   return 0xF59563;
        case 32:   return 0xF67C5F;
        case 64:   return 0xF65E3B;
        case 128:  return 0xEDCF72;
        case 256:  return 0xEDCC61;
        case 512:  return 0xEDC850;
        case 1024: return 0xEDC53F;
        case 2048: return 0xEDC22E;
        default:   return 0x3C3A32;
    }
}

static uint32_t get_tile_text_color(uint16_t value)
{
    if (value <= 4) {
        return 0x776E65;  // Dark text for light tiles
    } else {
        return 0xF9F6F2;  // Light text for dark tiles
    }
}
