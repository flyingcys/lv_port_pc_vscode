#include "desktop.h"
#include "system_app.h"
#include "game_2048.h"

LV_IMG_DECLARE(game_2048_icon);
LV_FONT_DECLARE(game_2048_font_32);
LV_FONT_DECLARE(game_2048_font_28);
LV_FONT_DECLARE(game_2048_font_24);
LV_FONT_DECLARE(game_2048_font_20);
LV_FONT_DECLARE(game_2048_font_16);

static RTC_FAST_ATTR uint16_t pix_array_save[4][4];
static RTC_FAST_ATTR uint8_t first_flg = 1;

static uint16_t pix_array[4][4];
static lv_obj_t *pix_array_obj[4][4];
static uint8_t pix_cnt = 0;

static key_value_handle_t btn_click_handle = NULL;
static key_value_handle_t btn_double_handle = NULL;

static uint8_t scr_loaded_flg = 0;

#define PIX_NULL 65535

static const uint32_t color_array[] = {
    GAME_2048_2_COLOR,
    GAME_2048_4_COLOR,
    GAME_2048_8_COLOR,
    GAME_2048_16_COLOR,
    GAME_2048_32_COLOR,
    GAME_2048_64_COLOR,
    GAME_2048_128_COLOR,
    GAME_2048_256_COLOR,
    GAME_2048_512_COLOR,
    GAME_2048_1024_COLOR,
    GAME_2048_2048_COLOR,
    GAME_2048_4096_COLOR,
    GAME_2048_8192_COLOR,
    GAME_2048_16384_COLOR,
    GAME_2048_32768_COLOR,
};

static void show_pix(lv_obj_t *scr);
static uint8_t create_pix(lv_obj_t *scr);

static void chek_pix_cnt()
{
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            if (pix_array[i][j] != PIX_NULL)
                pix_cnt++;
        }
    }
}
static void scr_del_ch(lv_obj_t *scr)
{
    if (lv_obj_has_flag(scr, LV_OBJ_FLAG_USER_1))
    {
        lv_obj_clear_flag(scr, LV_OBJ_FLAG_USER_1);
        lv_obj_del(lv_obj_get_child(scr, 0));

        if (lv_obj_has_flag(scr, LV_OBJ_FLAG_USER_2))
        {
            lv_obj_clear_flag(scr, LV_OBJ_FLAG_USER_2);
            chek_pix_cnt();
            if (pix_cnt == 0)
            {
                create_pix(scr);
                create_pix(scr);
            }
            else
                show_pix(scr);
        }
    }
}

static void pix_set_pos(lv_obj_t *pix_obj, uint8_t line, uint8_t row)
{
    int16_t pix_width = GAME_2048_WIDTH_HEIGHT / 4;
    int16_t dx;
    int16_t dy;

    dx = pix_width * line + GAME_2048_PIX_SPACE / 2;
    dy = pix_width * row + GAME_2048_PIX_SPACE / 2;

    lv_obj_set_pos(pix_obj, dx + GAME_2048_OFFSET_X, dy + GAME_2048_OFFSET_Y);
}

static lv_obj_t *create_pix_obj(lv_obj_t *scr)
{
    lv_obj_t *pix_obj = lv_obj_create(scr);
    lv_obj_clear_flag(pix_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(pix_obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_outline_width(pix_obj, 0, 0);
    lv_obj_set_style_border_width(pix_obj, 0, 0);
    lv_obj_set_size(pix_obj, (GAME_2048_WIDTH_HEIGHT - GAME_2048_PIX_SPACE * 4) / 4, (GAME_2048_WIDTH_HEIGHT - GAME_2048_PIX_SPACE * 4) / 4);
    lv_obj_set_style_radius(pix_obj, (GAME_2048_WIDTH_HEIGHT - GAME_2048_PIX_SPACE * 4) / 40, 0);
    lv_obj_t *label = lv_label_create(pix_obj);
    lv_obj_center(label);
    return pix_obj;
}

static void pix_obj_chek_font_pos(uint8_t line, uint8_t row)
{
    if (pix_array[line][row] == PIX_NULL)
        return;

    if (pix_array_obj[line][row] == NULL)
        return;

    lv_obj_t *label = lv_obj_get_child(pix_array_obj[line][row], 0);

    if (pix_array[line][row] < 10)
        lv_obj_set_style_text_font(label, &game_2048_font_32, 0);
    else if (pix_array[line][row] < 100)
        lv_obj_set_style_text_font(label, &game_2048_font_28, 0);
    else if (pix_array[line][row] < 1000)
        lv_obj_set_style_text_font(label, &game_2048_font_24, 0);
    else if (pix_array[line][row] < 10000)
        lv_obj_set_style_text_font(label, &game_2048_font_20, 0);
    else
        lv_obj_set_style_text_font(label, &game_2048_font_16, 0);

    lv_label_set_text_fmt(label, "%d", pix_array[line][row]);

    uint8_t color_num = 0;

    switch (pix_array[line][row])
    {
    case 4:
        color_num = 1;
        break;
    case 8:
        color_num = 2;
        break;
    case 16:
        color_num = 3;
        break;
    case 32:
        color_num = 4;
        break;
    case 64:
        color_num = 5;
        break;
    case 128:
        color_num = 6;
        break;
    case 256:
        color_num = 7;
        break;
    case 512:
        color_num = 8;
        break;
    case 1024:
        color_num = 9;
        break;
    case 2048:
        color_num = 10;
        break;
    case 4096:
        color_num = 11;
        break;
    case 8192:
        color_num = 12;
        break;
    case 16384:
        color_num = 13;
        break;
    case 32768:
        color_num = 14;
        break;
    default:
        break;
    }
    lv_obj_set_style_bg_color(pix_array_obj[line][row], lv_color_hex(color_array[color_num]), 0);

    pix_set_pos(pix_array_obj[line][row], line, row);
}

static void show_pix(lv_obj_t *scr)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            if (pix_array[i][j] == PIX_NULL)
                continue;

            if (pix_array_obj[i][j] != NULL)
                continue;

            pix_array_obj[i][j] = create_pix_obj(scr);
            pix_obj_chek_font_pos(i, j);
        }
    }
}

static uint8_t create_pix(lv_obj_t *scr)
{
    if (pix_cnt == 16)
        return 0;

    srand(esp_timer_get_time() / 1000);
    uint8_t x, y;

    for (;;)
    {
        x = rand() % 4;
        y = rand() % 4;
        if (pix_array[x][y] == PIX_NULL)
        {
            if ((rand() % 2))
                pix_array[x][y] = 4;
            else
                pix_array[x][y] = 2;
            break;
        }
    }

    show_pix(scr);
    pix_cnt++;
    return 1;
}

static uint8_t chek_row_add(lv_dir_t dir)
{
    uint8_t change_cnt = 0;

    for (size_t row = 0; row < 4; row++)
    {
        int8_t line;
        if (dir == LV_DIR_LEFT)
            line = 0;
        else
            line = 3;

        do
        {
            if (pix_array[line][row] == PIX_NULL)
                goto change_i;

            if (dir == LV_DIR_LEFT && line == 3)
                break;
            if (dir == LV_DIR_RIGHT && line == 0)
                break;

            int8_t i;

            if (dir == LV_DIR_LEFT)
                i = line + 1;
            else
                i = line - 1;

            while (i >= 0 && i < 4)
            {

                if (pix_array[i][row] == PIX_NULL)
                {
                    if (dir == LV_DIR_LEFT)
                        i++;
                    else
                        i--;
                    continue;
                }

                if (pix_array[i][row] != pix_array[line][row])
                    break;

                pix_array[line][row] <<= 1;
                pix_obj_chek_font_pos(line, row);
                pix_array[i][row] = PIX_NULL;
                lv_obj_del(pix_array_obj[i][row]);
                pix_array_obj[i][row] = NULL;
                change_cnt++;
                pix_cnt--;

                break;
            }

        change_i:
            if (dir == LV_DIR_LEFT)
                line++;
            else
                line--;
        } while (line >= 0 && line < 4);
    }

    return change_cnt;
}

static uint8_t chek_line_add(lv_dir_t dir)
{

    uint8_t change_cnt = 0;

    for (size_t line = 0; line < 4; line++)
    {
        int8_t row;
        if (dir == LV_DIR_TOP)
            row = 0;
        else
            row = 3;

        do
        {

            if (pix_array[line][row] == PIX_NULL)
                goto change_i;

            if (dir == LV_DIR_TOP && row == 3)
                break;
            if (dir == LV_DIR_BOTTOM && row == 0)
                break;

            int8_t i;

            if (dir == LV_DIR_TOP)
                i = row + 1;
            else
                i = row - 1;

            while (i >= 0 && i < 4)
            {

                if (pix_array[line][i] == PIX_NULL)
                {
                    if (dir == LV_DIR_TOP)
                        i++;
                    else
                        i--;
                    continue;
                }

                if (pix_array[line][i] != pix_array[line][row])
                    break;

                pix_array[line][row] <<= 1;
                pix_obj_chek_font_pos(line, row);
                pix_array[line][i] = PIX_NULL;
                lv_obj_del(pix_array_obj[line][i]);
                pix_array_obj[line][i] = NULL;
                change_cnt++;
                pix_cnt--;
                break;
            }

        change_i:
            if (dir == LV_DIR_TOP)
                row++;
            else
                row--;
        } while (row >= 0 && row < 4);
    }
    return change_cnt;
}

static uint8_t chek_row_move(lv_dir_t dir)
{

    uint8_t change_cnt = 0;

    for (size_t row = 0; row < 4; row++)
    {

        int8_t line;
        if (dir == LV_DIR_LEFT)
            line = 0;
        else
            line = 3;

        do
        {
            if (pix_array[line][row] != PIX_NULL)
                goto change_i;

            int8_t next = line;

            while (next >= 0 && next < 4)
            {
                if (pix_array[next][row] == PIX_NULL)
                    goto change_n;

                change_cnt++;

                pix_array[line][row] = pix_array[next][row];
                pix_array[next][row] = PIX_NULL;
                pix_array_obj[line][row] = pix_array_obj[next][row];
                pix_array_obj[next][row] = NULL;
                pix_set_pos(pix_array_obj[line][row], line, row);

                while (1)
                {

                    if (dir == LV_DIR_LEFT)
                        line++;
                    else
                        line--;
                    if (pix_array[line][row] == PIX_NULL)
                        break;
                }

            change_n:

                if (dir == LV_DIR_LEFT)
                    next++;
                else
                    next--;
            }
            break;

        change_i:
            if (dir == LV_DIR_LEFT)
                line++;
            else
                line--;
        } while (line >= 0 && line < 4);
    }
    return change_cnt;
}

static uint8_t chek_line_move(lv_dir_t dir)
{

    uint8_t change_cnt = 0;

    for (size_t line = 0; line < 4; line++)
    {

        int8_t row;
        if (dir == LV_DIR_TOP)
            row = 0;
        else
            row = 3;

        do
        {
            if (pix_array[line][row] != PIX_NULL)
                goto change_i;

            int8_t next = row;

            while (next >= 0 && next < 4)
            {
                if (pix_array[line][next] == PIX_NULL)
                    goto change_n;

                change_cnt++;

                pix_array[line][row] = pix_array[line][next];
                pix_array[line][next] = PIX_NULL;
                pix_array_obj[line][row] = pix_array_obj[line][next];
                pix_array_obj[line][next] = NULL;
                pix_set_pos(pix_array_obj[line][row], line, row);

                while (1)
                {

                    if (dir == LV_DIR_TOP)
                        row++;
                    else
                        row--;
                    if (pix_array[line][row] == PIX_NULL)
                        break;
                }

            change_n:

                if (dir == LV_DIR_TOP)
                    next++;
                else
                    next--;
            }
            break;

        change_i:
            if (dir == LV_DIR_TOP)
                row++;
            else
                row--;
        } while (row >= 0 && row < 4);
    }

    return change_cnt;
}

static void pix_move(lv_obj_t *scr, lv_dir_t dir)
{
    uint8_t cnt = 0;

    switch (dir)
    {
    case LV_DIR_LEFT:
        cnt = chek_row_add(dir);
        cnt += chek_row_move(dir);

        break;

    case LV_DIR_RIGHT:
        cnt = chek_row_add(dir);
        cnt += chek_row_move(dir);
        break;

    case LV_DIR_TOP:
        cnt = chek_line_add(dir);
        cnt += chek_line_move(dir);
        break;

    case LV_DIR_BOTTOM:
        cnt = chek_line_add(dir);
        cnt += chek_line_move(dir);
        break;

    default:
        break;
    }
    if (cnt)
        create_pix(scr);
}

static void pix_obj_clear()
{
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            if (pix_array_obj[i][j] == NULL)
                continue;
            lv_obj_del(pix_array_obj[i][j]);
            pix_array_obj[i][j] = NULL;
        }
    }
}

static void ui_event(lv_event_t *e)
{
    static lv_point_t start_point;
    static lv_point_t end_point;
    lv_indev_t *indev = (lv_indev_t *)e->user_data;
    if (e->code == LV_EVENT_PRESSED)
    {
        lv_indev_get_point(indev, &start_point);
    }
    else
    {
        if (lv_obj_has_flag(e->target, LV_OBJ_FLAG_USER_1))
        {
            lv_obj_add_flag(e->target, LV_OBJ_FLAG_USER_2);
            scr_del_ch(e->target);
            return;
        }

        lv_indev_get_point(indev, &end_point);
        int dx = end_point.x - start_point.x;
        int dy = end_point.y - start_point.y;
        lv_dir_t dir;
        if (fabs(dx) > fabs(dy))
        {
            if (fabs(dx) < 20)
                return;
            if (dx > 0)
                dir = LV_DIR_RIGHT;
            else
                dir = LV_DIR_LEFT;
        }
        else
        {
            if (fabs(dy) < 20)
                return;
            if (dy > 0)
                dir = LV_DIR_BOTTOM;
            else
                dir = LV_DIR_TOP;
        }
        pix_move(e->target, dir);
    }
    if (e->code == LV_EVENT_GESTURE)
    {
        pix_move(e->target, lv_indev_get_gesture_dir(lv_indev_get_act()));
    }
}

static void click_event_cb(void *value, size_t size)
{
    system_take_gui_key();
    lv_scr_load_anim_t anim = LV_SCR_LOAD_ANIM_FADE_ON;
    key_value_msg("sys_home", &anim, sizeof(anim));
    system_give_gui_key();
}

static void double_click_event_cb(void *value, size_t size)
{

    system_take_gui_key();

    lv_obj_t *scr = lv_scr_act();

    scr_del_ch(scr);

    pix_obj_clear();
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 4; j++)
            pix_array[i][j] = PIX_NULL;
    }

    pix_cnt = 0;
    create_pix(scr);
    create_pix(scr);

    system_give_gui_key();
}

static void app_load_cb(void *arg)
{
    if (first_flg)
    {
        first_flg = 0;
        for (uint8_t i = 0; i < 4; i++)
        {
            for (uint8_t j = 0; j < 4; j++)
                pix_array[i][j] = PIX_NULL;
        }
    }
    else
        memcpy(pix_array, pix_array_save, sizeof(pix_array));

    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
            pix_array_obj[i][j] = NULL;
    }

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);

#if GUI_SIMULATOR_SPACE
    lv_obj_set_style_radius(scr, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(scr, true, 0);
#endif

    // lv_obj_add_flag(scr,LV_OBJ_FLAG_C)

    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_style_text_font(label, &game_2048_font_24, LV_PART_MAIN);
    lv_label_set_text(label, "单击按键退出\n\n双击按键重开");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    static lv_indev_t *indev = NULL; // 触屏输入对象
    key_value_msg("sys_get_tp_indev", &indev, sizeof(lv_indev_t *));

    lv_obj_add_flag(scr, LV_OBJ_FLAG_USER_1);
    lv_obj_add_event_cb(scr, ui_event, LV_EVENT_PRESSED, indev);
    lv_obj_add_event_cb(scr, ui_event, LV_EVENT_RELEASED, indev);
    app_scr_load(scr, LV_SCR_LOAD_ANIM_FADE_ON, 0, 100); // 加载APP界面

    key_value_register(&btn_click_handle, "btn_click", click_event_cb);
    key_value_register(&btn_double_handle, "btn_double", double_click_event_cb);
    uint8_t game = 1;
    key_value_msg("game_mode", &game, sizeof(game));
}

static void app_close_cb(void *arg)
{
    key_value_del(btn_click_handle);
    key_value_del(btn_double_handle);
    memcpy(pix_array_save, pix_array, sizeof(pix_array));
    pix_obj_clear();
    uint8_t game = 0;
    key_value_msg("game_mode", &game, sizeof(game));
}

void game_2048_install()
{
    app_config_t cfg = {
        .app_close = app_close_cb,
        .app_init = NULL,
        .app_kill = NULL,
        .app_load = app_load_cb,
        .app_power_off = NULL,
        .has_gui = 1,
        .icon = &game_2048_icon,
        .name = "2048",
        .name_font = &game_2048_font_24};
    app_install(&cfg, NULL);
}