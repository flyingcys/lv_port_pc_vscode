#include "desktop.h"
#include "game_minesweeper.h"

LV_IMG_DECLARE(game_minesweeper_icon);
LV_IMG_DECLARE(game_minesweeper_img_space);
LV_IMG_DECLARE(game_minesweeper_img_boom);
LV_IMG_DECLARE(game_minesweeper_img_cover);
LV_IMG_DECLARE(game_minesweeper_img_pointer);
LV_IMG_DECLARE(game_minesweeper_img_red_boom);
LV_IMG_DECLARE(game_minesweeper_img_flag);
LV_IMG_DECLARE(game_minesweeper_img_flag_big);
LV_IMG_DECLARE(game_minesweeper_img_smile);
LV_IMG_DECLARE(game_minesweeper_img_cry);
LV_IMG_DECLARE(game_minesweeper_img_cool);
LV_FONT_DECLARE(game_minesweeper_font_24);
LV_FONT_DECLARE(game_minesweeper_font_16);

typedef enum
{
    pix_sta_space, // 空格已移出遮盖
    pix_sta_boom,  // 显示炸弹
    pix_sta_flag,  // 显示旗子
    pix_sta_cover  // 显示遮盖
} pix_sta_t;

typedef enum
{
    game_sta_start,   // 第一次运行
    game_sta_end,     // G了
    game_sta_success, // 成功
    game_sta_playing  // 游戏中
} game_sta_t;

typedef enum
{
    bcd_none,
    bcd_del_texts,
    bcd_down,
    bcd_up,
    bcd_back,
    bcd_ref_flags,
    bcd_smile,
    bcd_del_first_text,
} user_flag_bcd_t;

static const uint32_t number_color[] = {0x0803ED, 0x08760E, 0xF10204, 0x081B7F, 0x6C0B10, 0x0E7972, 0x0B082C, 0x817E7F};
static uint16_t *pix_array_boom_sta;
static uint32_t *pix_array_cover_sta;
#if MINESWEEPER_CLIP_PIX_EN
static uint16_t *pix_array_clip;
static const uint8_t pix_clip_list[] =
    {
#include "game_minesweeper_clip_pix.h"
};
#endif
static RTC_FAST_ATTR uint16_t pix_array_boom_sta_save[MINESWEEPER_LINE_NUM];
static RTC_FAST_ATTR uint32_t pix_array_cover_sta_save[MINESWEEPER_LINE_NUM];
static RTC_FAST_ATTR game_sta_t game_sta = game_sta_start;
static RTC_FAST_ATTR uint16_t boom_number = BOOM_NUMBER_DEFAULT;
static RTC_FAST_ATTR uint16_t flag_cnt = 0;
static RTC_FAST_ATTR uint16_t used_time_cnt = 0;
static RTC_FAST_ATTR uint16_t removed_pix_cnt = 0;
static uint8_t time_cnt_en = 0; // 游戏时长计时
static key_value_handle_t btn_click_handle = NULL;
static key_value_handle_t btn_double_handle = NULL;
static lv_obj_t **pix_obj = NULL;
static lv_obj_t *scr = NULL;
static lv_obj_t *game_box = NULL;
static uint8_t click_cnt = 0;
static lv_timer_t *click_timer = NULL;
static lv_timer_t *game_time_timer = NULL;
static lv_point_t point;

static pix_sta_t pix_get_sta(uint8_t line, uint8_t row);              // 获取点当前状态
static void pix_set_sta(uint8_t line, uint8_t row, pix_sta_t sta);    // 设置点当前状态
static uint16_t pix_has_boom(uint8_t line, uint8_t row);              // 获取点是否为炸弹
static void pix_set_boom(uint8_t line, uint8_t row, uint8_t sta);     // 设置点是否为炸弹
static size_t get_pix_around_booms(uint8_t line, uint8_t row);        // 获取周围炸弹数量
static void create_boom_srand(uint8_t boom_num);                      // 创建随机炸弹
static void refresh_show();                                           // 刷新显示
static void change_pix_sta(uint8_t line, uint8_t row, pix_sta_t sta); // 更新点状态
static void change_smile_img(lv_obj_t *obj);                          // 刷新笑脸显示
static void create_around_booms_label(uint8_t line, uint8_t row);     // 创建显示周围炸弹数量
static void img_event(lv_event_t *e);                                 // 游戏整图事件
static void lv_obj_set_bcd(lv_obj_t *obj, uint8_t bcd);               // 设置状态码
static uint8_t lv_obj_get_bcd(lv_obj_t *obj);                         // 清除状态码
static void create_pix(uint8_t line, uint8_t row, pix_sta_t sta);     // 创建点
static void game_restart();                                           // 重开
static void space_grow(uint8_t line, uint8_t row);                    // 深度查找空白
static void chek_success();                                           // 判断游戏是否结束
static void click_timer_cb(lv_timer_t *e);                            // 单击判断定时
static void time_cnt_cb(lv_timer_t *e);                               // 游戏时长定时
static void timer_setting_cb(lv_timer_t *e);                          // 设置定时
#if MINESWEEPER_CLIP_PIX_EN
static void chek_clip_pix();
static uint16_t pix_is_clip(uint8_t line, uint8_t row);
#endif

static void change_smile_img(lv_obj_t *obj) // 更新笑脸图标
{
    if (game_sta == game_sta_end) // 游戏失败
        lv_img_set_src(obj, &game_minesweeper_img_cry);
    else if (game_sta == game_sta_success) // 通关
        lv_img_set_src(obj, &game_minesweeper_img_cool);
    else
        lv_img_set_src(obj, &game_minesweeper_img_smile); // 平时
}

static void setting_click_event(lv_event_t *e)
{
    user_flag_bcd_t bcd = lv_obj_get_bcd(e->target); // 获取事件码

    if (bcd == bcd_up) // 加
    {
        if (boom_number < (MINESWEEPER_LINE_NUM * MINESWEEPER_COL_NUM  - sizeof(pix_clip_list) / 2))
            boom_number++;
        lv_label_set_text_fmt((lv_obj_t *)e->user_data, "%d", boom_number);
        return;
    }
    else if (bcd == bcd_down) // 减
    {
        if (boom_number > 0)
            boom_number--;
        lv_label_set_text_fmt((lv_obj_t *)e->user_data, "%d", boom_number);
        return;
    }
    else if (bcd == bcd_back) // 返回
    {
        lv_obj_set_bcd(game_box, bcd_del_texts);
        lv_obj_clear_flag(game_box, LV_OBJ_FLAG_HIDDEN);
        lv_event_send(game_box, LV_EVENT_RELEASED, NULL);
        key_value_msg("tp_mouse", NULL, 0);
        return;
    }
}

static void smile_click_event(lv_event_t *e)
{
    game_restart();
}

static void img_event(lv_event_t *e)
{

    static lv_timer_t *setting_timer = NULL;
    user_flag_bcd_t bcd = lv_obj_get_bcd(e->target); // 获取事件码

    if (bcd == bcd_del_first_text) // 删除第一个提示语的事件
    {
        lv_obj_set_bcd(e->target, bcd_none);  // 清除事件码
        while (lv_obj_get_child_cnt(scr) > 1) // 清除空游戏场景以外所有内容
            lv_obj_del(lv_obj_get_child(scr, 1));
        lv_obj_clear_flag(game_box, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_event_cb(scr, img_event);
        return;
    }

    if (bcd == bcd_del_texts) // 删除提示语的事件
    {

        lv_obj_set_bcd(e->target, bcd_none);  // 清除事件码
        while (lv_obj_get_child_cnt(scr) > 1) // 清除空游戏场景以外所有内容
            lv_obj_del(lv_obj_get_child(scr, 1));

        lv_obj_t *label = lv_label_create(scr); // 显示旗帜数量
#if FLAG_NUM_SHOW_HES
        lv_label_set_text_fmt(label, "%d", flag_cnt);
#else
        lv_label_set_text_fmt(label, "%d\n%d\n%d", (flag_cnt / 100) % 10, (flag_cnt / 10) % 10, flag_cnt % 10);
#endif
        lv_obj_set_style_text_font(label, &FLAG_NUM_FONT, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_hex(0xff0000), LV_PART_MAIN);
        lv_obj_align(label, FLAG_NUM_POS_ALIGN, FLAG_NUM_POS_OFFSET_X, FLAG_NUM_POS_OFFSET_Y);
        lv_obj_t *flag = lv_img_create(scr);
        lv_img_set_src(flag, &game_minesweeper_img_flag_big); // 显示旗帜图标
        lv_obj_align(flag, FLAG_IMG_POS_ALIGN, FLAG_IMG_POS_OFFSET_X, FLAG_IMG_POS_OFFSET_Y);

        label = lv_label_create(scr); // 显示游戏时长
#if TIME_NUM_SHOW_HES
        lv_label_set_text_fmt(label, "%d", used_time_cnt);
#else
        lv_label_set_text_fmt(label, "%d\n%d\n%d", (used_time_cnt / 100) % 10, (used_time_cnt / 10) % 10, used_time_cnt % 10);
#endif
        lv_obj_set_style_text_font(label, &TIME_NUM_FONT, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_hex(0xff0000), LV_PART_MAIN);
        lv_obj_align(label, TIME_NUM_POS_ALIGN, TIME_NUM_POS_OFFSET_X, TIME_NUM_POS_OFFSET_Y);
        flag = lv_img_create(scr); // 显示笑脸图标
        lv_obj_align(flag, SMILE_IMG_POS_ALIGN, SMILE_IMG_POS_OFFSET_X, SMILE_IMG_POS_OFFSET_Y);
        lv_obj_add_flag(flag, LV_OBJ_FLAG_CLICKABLE);                         // 笑脸图标可点击
        lv_obj_add_event_cb(flag, smile_click_event, LV_EVENT_CLICKED, NULL); // 添加点击事件
        change_smile_img(flag);                                               // 显示笑脸

        time_cnt_en = 1; // 允许游戏时长计时
        return;
    }

    if (bcd == bcd_ref_flags)
    {
        lv_obj_set_bcd(e->target, bcd_none);
#if FLAG_NUM_SHOW_HES
        lv_label_set_text_fmt(lv_obj_get_child(scr, 1), "%d", flag_cnt);
#else
        lv_label_set_text_fmt(lv_obj_get_child(scr, 1), "%d\n%d\n%d", (flag_cnt / 100) % 10, (flag_cnt / 10) % 10, flag_cnt % 10);
#endif
    }

    if (e->code == LV_EVENT_PRESSED)
    {
        setting_timer = lv_timer_create(timer_setting_cb, 1500, &setting_timer);
        lv_timer_set_repeat_count(setting_timer, 1);
        return;
    }

    if (e->code == LV_EVENT_RELEASED)
    {
        if (setting_timer != NULL && setting_timer != (void *)1)
        {
            lv_timer_del(setting_timer);
            setting_timer = NULL;
        }
        return;
    }

    if (game_sta != game_sta_playing)
        return;

    if (setting_timer == (void *)1)
        return;

    uint8_t tp_type;
    key_value_msg("tp_type", &tp_type, sizeof(tp_type));
    if (tp_type)
        return;

    lv_indev_t *indev = NULL;
    key_value_msg("sys_get_tp_indev", &indev, sizeof(lv_indev_t *));

    if (indev == NULL)
        return;
    lv_indev_get_point(indev, &point);
    if (point.x < MINESWEEPER_OFFSET_X && point.x >= (MINESWEEPER_OFFSET_X + 16 * MINESWEEPER_LINE_NUM))
        return;
    if (point.y < MINESWEEPER_OFFSET_Y && point.y >= (MINESWEEPER_OFFSET_Y + 16 * MINESWEEPER_COL_NUM))
        return;
    point.x -= MINESWEEPER_OFFSET_X;
    point.y -= MINESWEEPER_OFFSET_Y;
    point.x = point.x * MINESWEEPER_LINE_NUM / (16 * MINESWEEPER_LINE_NUM);
    point.y = point.y * MINESWEEPER_COL_NUM / (16 * MINESWEEPER_COL_NUM);

    if (click_cnt == 0)
    {
        click_timer = lv_timer_create(click_timer_cb, 300, &setting_timer);
        lv_timer_set_repeat_count(click_timer, 1);
        click_cnt++;
        return;
    }
    else
    {
        lv_timer_del(click_timer);
        click_timer = NULL;
        click_cnt = 0;

        if (pix_get_sta(point.x, point.y) != pix_sta_cover)
            return;
        if (game_sta == game_sta_end)
            return;

        if (pix_has_boom(point.x, point.y))
        {
            change_pix_sta(point.x, point.y, pix_sta_boom);
            return;
        }

        change_pix_sta(point.x, point.y, pix_sta_space);
        if (get_pix_around_booms(point.x, point.y))
        {
            chek_success();
            return;
        }

        space_grow(point.x, point.y);
        chek_success();
    }
}

static void lv_obj_set_bcd(lv_obj_t *obj, uint8_t bcd)
{
    if (bcd & 1)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_1);
    else
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_USER_1);
    if (bcd & 2)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_2);
    else
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_USER_2);
    if (bcd & 4)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_3);
    else
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_USER_3);
    if (bcd & 8)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_4);
    else
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_USER_4);
}

static uint8_t lv_obj_get_bcd(lv_obj_t *obj)
{
    uint8_t bcd = 0;
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_1))
        bcd |= 1;
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_2))
        bcd |= 2;
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_3))
        bcd |= 4;
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_4))
        bcd |= 8;
    return bcd;
}

static pix_sta_t pix_get_sta(uint8_t line, uint8_t row)
{
    return (pix_sta_t)((pix_array_cover_sta[line] >> (row << 1)) & 0x03);
}

static void pix_set_sta(uint8_t line, uint8_t row, pix_sta_t sta)
{
    pix_array_cover_sta[line] &= ~(0x03 << (row << 1));
    pix_array_cover_sta[line] |= ((uint32_t)sta << (row << 1));
}

static uint16_t pix_has_boom(uint8_t line, uint8_t row)
{
    return (pix_array_boom_sta[line] & (1 << row));
}

static void pix_set_boom(uint8_t line, uint8_t row, uint8_t sta)
{
    if (sta)
        pix_array_boom_sta[line] |= (1 << row);
    else
        pix_array_boom_sta[line] &= ~(1 << row);
}

#if MINESWEEPER_CLIP_PIX_EN

static void chek_clip_pix()
{
    memset(pix_array_clip, 0, MINESWEEPER_LINE_NUM * sizeof(uint16_t));
    for (size_t i = 0; i < sizeof(pix_clip_list); i += 2)
        pix_array_clip[pix_clip_list[i]] |= (1 << pix_clip_list[i + 1]);
}

static uint16_t pix_is_clip(uint8_t line, uint8_t row)
{
    return (pix_array_clip[line] & (1 << row));
}

#endif

static size_t get_pix_around_booms(uint8_t line, uint8_t row)
{
    size_t cnt = 0;

    int dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

    for (size_t i = 0; i < 8; i++)
    {
        int x = line + dx[i];
        int y = row + dy[i];
        if (x < 0 || x >= MINESWEEPER_LINE_NUM || y < 0 || y >= MINESWEEPER_COL_NUM)
            continue;
        if (pix_has_boom(x, y))
            cnt++;
    }
    return cnt;
}

static void chek_success()
{
    if ((removed_pix_cnt + boom_number) == (MINESWEEPER_LINE_NUM * MINESWEEPER_COL_NUM))
    {
        game_sta = game_sta_success;
        change_smile_img(lv_obj_get_child(scr, 4));
    }
}

static void create_around_booms_label(uint8_t line, uint8_t row)
{
    uint8_t cnt = get_pix_around_booms(line, row);
    if (cnt == 0)
        return;
    lv_obj_t **pix = pix_obj + row * MINESWEEPER_LINE_NUM + line;
    *pix = lv_label_create(game_box);
    lv_obj_set_style_text_font(*pix, &game_minesweeper_font_16, LV_PART_MAIN);
    lv_label_set_text_fmt(*pix, "%d", cnt);
    lv_obj_set_pos(*pix, line * 16 + 3, row * 16 - 1);
    lv_obj_set_style_text_color(*pix, lv_color_hex(number_color[cnt - 1]), LV_PART_MAIN);
}

static void create_pix(uint8_t line, uint8_t row, pix_sta_t sta)
{
    lv_obj_t **pix = pix_obj + row * MINESWEEPER_LINE_NUM + line;
    if (*pix != NULL) // 已存在点
        return;
    pix_set_sta(line, row, sta); // 同步状态

    if (sta != pix_sta_space)
        *pix = lv_img_create(game_box);

    switch (sta)
    {
    case pix_sta_cover:
        lv_img_set_src(*pix, &game_minesweeper_img_cover);
        break;

    case pix_sta_flag:
        lv_img_set_src(*pix, &game_minesweeper_img_flag);
        break;

    case pix_sta_boom:
        lv_img_set_src(*pix, &game_minesweeper_img_red_boom);
        break;

    case pix_sta_space:
        if (pix_has_boom(line, row))
        {
            *pix = lv_img_create(game_box);
            lv_img_set_src(*pix, &game_minesweeper_img_boom);
        }
        else
        {
            create_around_booms_label(line, row);
            return;
        }
        break;
    default:
        break;
    }
    lv_obj_set_pos(*pix, line * 16, row * 16);
}

static void change_pix_sta(uint8_t line, uint8_t row, pix_sta_t sta)
{
    pix_sta_t pix_sta = pix_get_sta(line, row);

    if (pix_sta == sta) // 状态无改变
        return;
    if (pix_sta == pix_sta_space) // 已经是空地
        return;
    if (pix_sta == pix_sta_boom) // 是结束的红色炸弹
        return;

    lv_obj_t **pix = pix_obj + row * MINESWEEPER_LINE_NUM + line;
    uint8_t send_event_code = 0;

    if (sta == pix_sta_flag) // 加旗子
    {
        flag_cnt++;
        lv_img_set_src(*pix, &game_minesweeper_img_flag);
        send_event_code = 1;
    }
    else if (sta == pix_sta_cover) // 去旗子
    {
        flag_cnt--;
        lv_img_set_src(*pix, &game_minesweeper_img_cover);
        send_event_code = 1;
    }
    else if (sta == pix_sta_boom) // 点到炸弹
    {
        time_cnt_en = 0;                                      // 停止计时
        game_sta = game_sta_end;                              // 结束游戏
        lv_img_set_src(*pix, &game_minesweeper_img_red_boom); // 显示红色炸弹
        pix_set_sta(line, row, sta);
        for (size_t i = 0; i < MINESWEEPER_LINE_NUM; i++)
        {
            for (size_t j = 0; j < MINESWEEPER_COL_NUM; j++)
            {
                if (pix_has_boom(i, j))
                    change_pix_sta(i, j, pix_sta_space);
            }
        }
        change_smile_img(lv_obj_get_child(scr, 4));
    }
    else // 打开空地
    {
        removed_pix_cnt++;
        if (pix_has_boom(line, row))
        {
            lv_img_set_src(*pix, &game_minesweeper_img_boom);
        }
        else
        {
            lv_obj_del(*pix);
            create_around_booms_label(line, row);
        }
    }
    if (send_event_code == 1) // 更新旗子数量
    {
        lv_obj_set_bcd(game_box, bcd_ref_flags);
        lv_event_send(game_box, LV_EVENT_RELEASED, NULL);
    }

    pix_set_sta(line, row, sta);
}

static void create_boom_srand(uint8_t boom_num)
{
    memset(pix_array_boom_sta, 0, sizeof(pix_array_boom_sta_save)); // 清除炸弹

    srand(MINESWEEPER_GET_MS_TIC); // 生成炸弹
    while (boom_num)
    {
        uint8_t row = rand() % MINESWEEPER_COL_NUM;
        uint8_t line = rand() % MINESWEEPER_LINE_NUM;
        if (pix_has_boom(line, row))
            continue;
#if MINESWEEPER_CLIP_PIX_EN
        if (pix_is_clip(line, row))
        {
            continue;
        }

#endif
        pix_set_boom(line, row, 1);
        boom_num--;
    }

    while (lv_obj_get_child_cnt(game_box)) // 清除屏幕所有对象
        lv_obj_del(lv_obj_get_child(game_box, 0));

    for (size_t i = 0; i < MINESWEEPER_LINE_NUM * MINESWEEPER_COL_NUM; i++) // 清空指针
        *(pix_obj + i) = NULL;

    for (size_t i = 0; i < MINESWEEPER_LINE_NUM; i++) // 重置所有点状态
    {
        for (size_t j = 0; j < MINESWEEPER_COL_NUM; j++)
            pix_set_sta(i, j, pix_sta_cover);
    }
}

static void refresh_show()
{
    for (size_t i = 0; i < MINESWEEPER_LINE_NUM; i++)
    {
        for (size_t j = 0; j < MINESWEEPER_COL_NUM; j++)
            create_pix(i, j, pix_get_sta(i, j));
    }
}

static void game_restart()
{
    removed_pix_cnt = 0;
    flag_cnt = 0;
    used_time_cnt = 0;
    create_boom_srand(boom_number);
    refresh_show(game_box);
    game_sta = game_sta_playing;
    lv_obj_set_bcd(game_box, bcd_del_texts); // 清除显示重新生成
    lv_event_send(game_box, LV_EVENT_RELEASED, NULL);
}

static void time_cnt_cb(lv_timer_t *e) // 游戏时长定时
{
    if (game_sta != game_sta_playing) // 没玩游戏不能计时
        return;

    if (time_cnt_en != 1) // 使能没开不能计时
        return;

    if (used_time_cnt < 999) // 大于999
        used_time_cnt++;
    else
        return;

// 刷新显示
#if TIME_NUM_SHOW_HES
    lv_label_set_text_fmt(lv_obj_get_child(scr, 3), "%d", used_time_cnt);
#else
    lv_label_set_text_fmt(lv_obj_get_child(scr, 3), "%d\n%d\n%d", (used_time_cnt / 100) % 10, (used_time_cnt / 10) % 10, used_time_cnt % 10);
#endif
}

static void click_timer_cb(lv_timer_t *e) // 单击判断定时
{
    click_cnt = 0;
    click_timer = NULL;

    if (*(lv_timer_t **)e->user_data != NULL) // 长按中
        return;

    if (pix_get_sta(point.x, point.y) == pix_sta_cover) // 翻转标记状态
        change_pix_sta(point.x, point.y, pix_sta_flag);
    else if (pix_get_sta(point.x, point.y) == pix_sta_flag)
        change_pix_sta(point.x, point.y, pix_sta_cover);
}

static void timer_setting_cb(lv_timer_t *e) // 长按开启设置定时
{

    *(lv_timer_t **)e->user_data = (void *)1; // 定时器清除

    uint8_t tp_type;
    key_value_msg("tp_type", &tp_type, sizeof(tp_type));
    if (tp_type) // 过程中有滑动
        return;

    time_cnt_en = 0; // 游戏时长暂停

    while (lv_obj_get_child_cnt(scr) > 1) // 清除左右两侧内容
        lv_obj_del(lv_obj_get_child(scr, 1));

    // 创建菜单
    lv_obj_add_flag(game_box, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "地雷数量  重开生效");
    lv_obj_set_style_text_font(label, &game_minesweeper_font_24, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 60);

    lv_obj_t *label_num = lv_label_create(scr);
    lv_label_set_text_fmt(label_num, "%d", boom_number);
    lv_obj_set_style_text_font(label_num, &game_minesweeper_font_24, LV_PART_MAIN);
    lv_obj_align(label_num, LV_ALIGN_CENTER, 0, 0);

    label = lv_label_create(scr);
    lv_label_set_text(label, "一");
    lv_obj_set_style_text_font(label, &game_minesweeper_font_24, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, -60, 0);
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_bcd(label, bcd_down);
    lv_obj_add_event_cb(label, setting_click_event, LV_EVENT_CLICKED, label_num);
    lv_obj_set_ext_click_area(label, 20);

    label = lv_label_create(scr);
    lv_label_set_text(label, "十");
    lv_obj_set_style_text_font(label, &game_minesweeper_font_24, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 60, 0);
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_bcd(label, bcd_up);
    lv_obj_add_event_cb(label, setting_click_event, LV_EVENT_CLICKED, label_num);
    lv_obj_set_ext_click_area(label, 20);

    label = lv_label_create(scr);
    lv_label_set_text(label, "X");
    lv_obj_set_style_text_font(label, &game_minesweeper_font_24, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -60);
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_bcd(label, bcd_back);
    lv_obj_set_ext_click_area(label, 20);

    lv_obj_add_event_cb(label, setting_click_event, LV_EVENT_CLICKED, label_num);

    key_value_msg("tp_touch", NULL, 0);
}

uint8_t isValid(int16_t line, int16_t row)
{

    return line >= 0 && line < MINESWEEPER_LINE_NUM && row >= 0 && row < MINESWEEPER_COL_NUM;
}

static void grow_visited_set(uint16_t *grow_array, uint8_t line, uint8_t row)
{
    grow_array[line] |= (1 << row);
}

static uint16_t grow_visited_get(uint16_t *grow_array, uint8_t line, uint8_t row)
{
    return (grow_array[line] & (1 << row));
}

static void space_grow(uint8_t line, uint8_t row)
{

    uint16_t *grow_array = MINESWEEPER_MEMERY_MALLOC(sizeof(uint16_t) * MINESWEEPER_LINE_NUM);
    memset(grow_array, 0, sizeof(uint16_t) * MINESWEEPER_LINE_NUM);

    if (!isValid(line, row) || grow_visited_get(grow_array, line, row) || pix_has_boom(line, row) || pix_get_sta(line, row) == pix_sta_flag)
        return; // 越界、已访问或为地雷则返回

    grow_visited_set(grow_array, line, row); // 标记为已访问

    struct buf
    {
        int16_t line;
        int16_t row;
    };

    struct buf *buffer = MINESWEEPER_MEMERY_MALLOC(sizeof(struct buf) * MINESWEEPER_LINE_NUM * MINESWEEPER_COL_NUM);
    int16_t cnt = 0;

    buffer[cnt] = (struct buf){line, row};

    while (cnt >= 0)
    {
        struct buf current = buffer[cnt--];
        if (get_pix_around_booms(current.line, current.row) == 0)
        {
            int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
            int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

            for (int i = 0; i < 8; ++i)
            {
                int newX = current.line + dx[i];
                int newY = current.row + dy[i];
                if (!isValid(newX, newY) || grow_visited_get(grow_array, newX, newY) || pix_has_boom(newX, newY) || pix_get_sta(newX, newY) == pix_sta_flag)
                    continue;
                buffer[++cnt] = (struct buf){newX, newY};
                grow_visited_set(grow_array, newX, newY); // 标记为已访问
            }
        }
        change_pix_sta(current.line, current.row, pix_sta_space); // 清除点
    }

    MINESWEEPER_MEMERY_FREE(buffer);
    MINESWEEPER_MEMERY_FREE(grow_array);
}

static void btn_click_event_cb(void *value, size_t size) // 单击返回桌面
{
    key_value_msg("take_gui_key", NULL, 0);
    lv_scr_load_anim_t anim = LV_SCR_LOAD_ANIM_FADE_ON;
    key_value_msg("sys_home", &anim, sizeof(anim));
    key_value_msg("give_gui_key", NULL, 0);
}

static void btn_double_click_event_cb(void *value, size_t size) // 双击重开游戏
{
    key_value_msg("take_gui_key", NULL, 0);
    game_restart();
    key_value_msg("give_gui_key", NULL, 0);
}

static void app_load_cb(void *arg)
{
    key_value_msg("tp_mouse", NULL, 0); // 屏幕切换成触摸板模式

    pix_array_boom_sta = MINESWEEPER_MEMERY_MALLOC(sizeof(uint16_t) * MINESWEEPER_LINE_NUM);
    pix_array_cover_sta = MINESWEEPER_MEMERY_MALLOC(sizeof(uint32_t) * MINESWEEPER_LINE_NUM);
    pix_obj = MINESWEEPER_MEMERY_MALLOC(sizeof(lv_obj_t *) * MINESWEEPER_LINE_NUM * MINESWEEPER_COL_NUM); // 申请对象内存
    for (size_t i = 0; i < MINESWEEPER_LINE_NUM * MINESWEEPER_COL_NUM; i++)
        *(pix_obj + i) = NULL;

#if MINESWEEPER_CLIP_PIX_EN
    pix_array_clip = MINESWEEPER_MEMERY_MALLOC(sizeof(uint32_t) * MINESWEEPER_LINE_NUM);
    chek_clip_pix();
#endif

    scr = lv_obj_create(NULL);                      // 创建屏幕对象
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);

    game_box = lv_img_create(scr); // 创建空地图
    lv_img_set_src(game_box, &game_minesweeper_img_space);
    lv_obj_set_size(game_box, MINESWEEPER_LINE_NUM * 16, MINESWEEPER_COL_NUM * 16);
    lv_obj_set_pos(game_box, MINESWEEPER_OFFSET_X, MINESWEEPER_OFFSET_Y);
    lv_obj_add_flag(game_box, LV_OBJ_FLAG_HIDDEN);

#if GUI_SIMULATOR_SPACE
    lv_obj_set_style_radius(scr, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(scr, true, 0);
#endif

    if (game_sta != game_sta_start) // 恢复
    {
        memcpy(pix_array_boom_sta, pix_array_boom_sta_save, sizeof(pix_array_boom_sta_save));
        memcpy(pix_array_cover_sta, pix_array_cover_sta_save, sizeof(pix_array_cover_sta_save));
    }
    else // 初次
    {
        memset(pix_array_boom_sta, 0, sizeof(pix_array_boom_sta_save));
        memset(pix_array_cover_sta, 0, sizeof(pix_array_cover_sta_save));
        create_boom_srand(boom_number);
        game_sta = game_sta_playing;
    }

    refresh_show(); // 更新显示

    // 创建提示语
    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_style_text_font(label, &game_minesweeper_font_16, LV_PART_MAIN);
    lv_label_set_text(label, "        按键单击退出 双击重开\n\n触屏单击标记 双击排雷 长按设置");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_flag(game_box, LV_OBJ_FLAG_CLICKABLE); // 开启地图点击使能
    lv_obj_set_bcd(game_box, bcd_del_texts);
    lv_obj_add_event_cb(game_box, img_event, LV_EVENT_CLICKED, NULL);  // 点击事件
    lv_obj_add_event_cb(game_box, img_event, LV_EVENT_PRESSED, NULL);  // 按下事件
    lv_obj_add_event_cb(game_box, img_event, LV_EVENT_RELEASED, NULL); // 松开事件

    lv_obj_set_bcd(scr, bcd_del_first_text);                      // 点击后首先删除提示语
    lv_obj_add_event_cb(scr, img_event, LV_EVENT_RELEASED, NULL); // 回调
    app_scr_load(scr, LV_SCR_LOAD_ANIM_FADE_ON, 0, 100);          // 加载APP界面

    key_value_register(&btn_click_handle, "btn_click", btn_click_event_cb); // 订阅键值对
    key_value_register(&btn_double_handle, "btn_double", btn_double_click_event_cb);

    game_time_timer = lv_timer_create(time_cnt_cb, 1000, NULL); // 游戏时长计时
    time_cnt_en = 0;                                            // 有提示语不允许计时
    uint8_t game = 1;
    key_value_msg("game_mode", &game, sizeof(game));
}

static void app_close_cb(void *arg)
{
    // 切换回触摸输入

    key_value_msg("tp_touch", NULL, 0);

    lv_timer_del(game_time_timer); // 删除游戏时长计时

    if (click_timer != NULL)
        lv_timer_del(click_timer);

    // 存数据
    memcpy(pix_array_boom_sta_save, pix_array_boom_sta, sizeof(pix_array_boom_sta_save));
    memcpy(pix_array_cover_sta_save, pix_array_cover_sta, sizeof(pix_array_cover_sta_save));

    key_value_del(btn_click_handle); // 注销键值对
    key_value_del(btn_double_handle);
    uint8_t game = 0;
    key_value_msg("game_mode", &game, sizeof(game));

    MINESWEEPER_MEMERY_FREE(pix_obj); // 释放缓存
    MINESWEEPER_MEMERY_FREE(pix_array_boom_sta);
    MINESWEEPER_MEMERY_FREE(pix_array_cover_sta);
#if MINESWEEPER_CLIP_PIX_EN
    MINESWEEPER_MEMERY_FREE(pix_array_clip);
#endif
}

void game_minesweeper_install()
{
    app_config_t cfg = {
        .app_close = app_close_cb,
        .app_init = NULL,
        .app_kill = NULL,
        .app_load = app_load_cb,
        .app_power_off = NULL,
        .has_gui = 1,
        .icon = &game_minesweeper_icon,
        .name = "扫雷",
        .name_font = &game_minesweeper_font_24};
    app_install(&cfg, NULL);
}
