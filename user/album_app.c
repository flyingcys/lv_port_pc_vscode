#include "album_app.h"
#include "system_app.h"
#include "desktop.h"

LV_IMG_DECLARE(album_app_icon);
LV_IMG_DECLARE(album_app_img_back);
LV_IMG_DECLARE(album_app_img_delete);
// LV_IMG_DECLARE(setting_app_img_logo);
// LV_IMG_DECLARE(setting_app_img_add);
// LV_IMG_DECLARE(setting_app_img_minus);

LV_FONT_DECLARE(album_app_font_24);
// LV_FONT_DECLARE(setting_app_font_22);
// LV_FONT_DECLARE(setting_app_font_16);

typedef struct small_img_list
{
    lv_img_dsc_t *img;
    uint8_t id;
    struct small_img_list *next;
    struct small_img_list *last;
} small_img_list_node_t;

static small_img_list_node_t *head = NULL;
static small_img_list_node_t *tail = NULL;
static lv_img_dsc_t *big_img_para = NULL;
static lv_obj_t *scr_main = NULL;
static lv_obj_t *scr_img = NULL;
static uint8_t max_img_num = 0;
static uint8_t choosed_num = 0;
// static uint8_t ref_small_img_able = 0;

typedef enum
{
    bcd_back,
    bcd_delete,
    bcd_big_img,
    bcd_fs_fmt,
} user_flag_bcd_t;

static void scr_event(lv_event_t *e);

static void img_format_init(lv_img_dsc_t *img)
{
    img->header.cf = LV_IMG_CF_TRUE_COLOR,
    img->header.always_zero = 0;
    img->header.reserved = 0;
    img->header.w = PRTSCR_WIDTH_SMALL;
    img->header.h = PRTSCR_HEIGHT_SMALL;
    img->data_size = PRTSCR_WIDTH_SMALL * PRTSCR_HEIGHT_SMALL * LV_COLOR_SIZE / 8;
}

static small_img_list_node_t *list_add_node()
{
    small_img_list_node_t *new_node = malloc(sizeof(small_img_list_node_t));
    new_node->next = NULL;
    new_node->last = NULL;
    new_node->img = malloc(sizeof(lv_img_dsc_t));
    img_format_init(new_node->img);
    new_node->img->data = malloc(PRTSCR_WIDTH_SMALL * PRTSCR_HEIGHT_SMALL * LV_COLOR_SIZE / 8);
    if (head == NULL)
    {
        head = new_node;
        tail = new_node;
        head->id = 0;
    }
    else
    {
        new_node->id = tail->id + 1;
        new_node->last = tail;
        tail->next = new_node;
        tail = new_node;
    }
    return new_node;
}

static void ref_id(small_img_list_node_t *start, uint8_t id)
{
    while (start != NULL)
    {
        start->id = id++;
        start = start->next;
    }
}

static void list_del_node(uint8_t id)
{
    small_img_list_node_t *tmp = head;
    lv_obj_t *cont = lv_obj_get_child(scr_main, 0);
    lv_obj_del(lv_obj_get_child(cont, id));

    if (id == 0) // 删除第一张
    {
        if (max_img_num > 1) // 还有后续张
        {
            head = head->next;
            head->last = NULL;
            ref_id(head, 0);
        }
        else // 无后续张
        {
            head = NULL;
            tail = NULL;
        }
    }
    else if (id == (max_img_num - 1)) // 删除尾巴
    {
        tmp = tail;
        tail = tail->last;
        tail->next = NULL;
        choosed_num--;
    }
    else // 删除中间
    {
        while (id--)
        {
            tmp = tmp->next;
        }

        tmp->last->next = tmp->next;
        tmp->next->last = tmp->last;
        ref_id(tmp->next, tmp->id);
    }

    free((void *)tmp->img->data);
    free(tmp->img);
    free(tmp);
    max_img_num--;
}

static void list_del_all_node()
{
    small_img_list_node_t *tmp = head;
    while (head != NULL)
    {
        head = head->next;
        free((void *)tmp->img->data);
        free(tmp->img);
        free(tmp);
        tmp = head;
    }
    tail = NULL;
}

static void img_big_to_small(uint16_t *big, uint16_t *small)
{
    uint32_t cnt = 0;
    for (size_t i = 0; i < PRTSCR_HEIGHT_SMALL * PRTSCR_WIDTH_SMALL; i++)
    {
        *small++ = *big;
        big += PRTSCR_PERCRNT_SMALL;
        cnt++;
        if (cnt == PRTSCR_WIDTH_SMALL)
        {
            cnt = 0;
            big += 280 * 3;
        }
    }
}

static void lv_obj_set_bcd(lv_obj_t *obj, uint8_t bcd)
{
    if (bcd & 1)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_1);
    if (bcd & 2)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_2);
    if (bcd & 4)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_3);
    if (bcd & 8)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_4);
}

// static void lv_obj_clear_all_flags(lv_obj_t *obj)
// {
//     lv_obj_clear_flag(obj, LV_OBJ_FLAG_USER_1);
//     lv_obj_clear_flag(obj, LV_OBJ_FLAG_USER_2);
//     lv_obj_clear_flag(obj, LV_OBJ_FLAG_USER_3);
//     lv_obj_clear_flag(obj, LV_OBJ_FLAG_USER_4);
// }

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

static void ref_img_small(lv_obj_t *parent)
{
    lv_obj_t *tmp;
    uint8_t prt_num = 0;
    key_value_msg("prtscr_num", &prt_num, 1);
    max_img_num = prt_num;
    for (size_t i = 0; i < prt_num; i++)
    {

        *(uint8_t *)big_img_para->data = i;

        key_value_msg("prtscr_get", (void *)big_img_para->data, 280 * 240 * 2);

        //    FILE *f = fopen(SPIFFS_PATH "IMG01","r");
        //    fread((void *)big_img_para->data,280*240*2,280*240*2,f);
        //    fclose(f);

        small_img_list_node_t *node = list_add_node();
        img_big_to_small((uint16_t *)big_img_para->data, (uint16_t *)node->img->data); // 提取缩略图

        tmp = lv_img_create(parent); // 创建图片
        lv_obj_set_style_pad_right(tmp, 8, 0);
        lv_img_set_src(tmp, node->img);
        lv_obj_add_flag(tmp, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_add_event_cb(tmp, scr_event, LV_EVENT_CLICKED, node);
    }
    if (max_img_num)
        return;
    tmp = lv_label_create(scr_main);
    lv_label_set_text(tmp, "无图片");
    lv_obj_align(tmp, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_style_text_font(tmp, &album_app_font_24, 0);

    // tmp = lv_label_create(scr_main);
    // lv_label_set_text(tmp, "格式化");
    // lv_obj_align(tmp, LV_ALIGN_CENTER, 0, 20);
    // lv_obj_set_style_text_font(tmp, &album_app_font_24, 0);
    // lv_obj_add_flag(tmp, LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_set_bcd(tmp, bcd_fs_fmt);
    // lv_obj_add_event_cb(tmp, scr_event, LV_EVENT_CLICKED, NULL);
}

static void scr_event(lv_event_t *e)
{
    if (e->code == LV_EVENT_GESTURE)
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir != LV_DIR_RIGHT && dir != LV_DIR_LEFT)
            return;

        lv_scr_load_anim_t anim;
        if (dir == LV_DIR_LEFT)
        {
            anim = LV_SCR_LOAD_ANIM_MOVE_LEFT;
        }
        else if (dir == LV_DIR_RIGHT)
        {
            anim = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
        }

        if (lv_scr_act() == scr_main)
        {
            key_value_msg("sys_home", &anim, sizeof(anim));
            return;
        }

        /////大图预览界面
        if (dir == LV_DIR_LEFT)
        {
            if (choosed_num >= (max_img_num - 1))
                return;
            choosed_num++;
        }
        else if (dir == LV_DIR_RIGHT)
        {
            if (choosed_num == 0)
                return;
            choosed_num--;
        }

        uint8_t *id = (uint8_t *)big_img_para->data;
        *id = choosed_num;
        key_value_msg("prtscr_get", (void *)big_img_para->data, 280 * 240 * 2);

        lv_obj_set_style_bg_img_src(scr_img, big_img_para, 0);
        lv_refr_now(NULL);
        lv_obj_t *cont = lv_obj_get_child(e->target, 0);
        lv_obj_t *label = lv_obj_get_child(cont, 2);
        lv_label_set_text_fmt(label, "%d/%d", choosed_num + 1, max_img_num);
    }
    if (e->code == LV_EVENT_CLICKED)
    {

        if (system_get_tp_type())
            return;

        if (lv_scr_act() == scr_main) // 点击缩略图
        {
            small_img_list_node_t *node = (small_img_list_node_t *)e->user_data; // 获取缩略图ID
            choosed_num = node->id;

            *(uint8_t *)big_img_para->data = choosed_num; // 获取大图
            key_value_msg("prtscr_get", (void *)big_img_para->data, 280 * 240 * 2);

            scr_img = lv_obj_create(NULL);                      // 创建大图页面
            lv_obj_clear_flag(scr_img, LV_OBJ_FLAG_SCROLLABLE); /// Flags

            lv_obj_set_style_bg_img_src(scr_img, big_img_para, 0);

            lv_obj_t *tmp = lv_obj_create(scr_img);
            lv_obj_remove_style_all(tmp);
            lv_obj_clear_flag(tmp, LV_OBJ_FLAG_SCROLLABLE); /// Flags
            lv_obj_set_size(tmp, LV_HOR_RES, 40);
            lv_obj_set_style_bg_color(tmp, lv_color_hex(0x808080), 0);
            lv_obj_set_style_bg_opa(tmp, 200, 0);
            lv_obj_align(tmp, LV_ALIGN_TOP_MID, 0, 0);

            lv_obj_t *img_back = lv_img_create(tmp);
            lv_img_set_src(img_back, &album_app_img_back);
            lv_obj_t *img_delete = lv_img_create(tmp);
            lv_img_set_src(img_delete, &album_app_img_delete);
            lv_obj_align(img_back, LV_ALIGN_LEFT_MID, 15, 0);
            lv_obj_align(img_delete, LV_ALIGN_RIGHT_MID, -20, 0);
            lv_obj_set_ext_click_area(img_back, 30);
            lv_obj_set_ext_click_area(img_delete, 30);

            lv_obj_set_bcd(img_back, bcd_back);
            lv_obj_set_bcd(img_delete, bcd_delete);
            lv_obj_set_bcd(scr_img, bcd_big_img);

            lv_obj_t *label = lv_label_create(tmp);
            lv_label_set_text_fmt(label, "%d/%d", choosed_num + 1, max_img_num);
            lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_style_text_font(label, &album_app_font_24, 0);

            lv_obj_add_event_cb(scr_img, scr_event, LV_EVENT_GESTURE, NULL);
            lv_obj_add_event_cb(scr_img, scr_event, LV_EVENT_CLICKED, NULL);
            lv_obj_add_event_cb(img_back, scr_event, LV_EVENT_CLICKED, NULL);
            lv_obj_add_event_cb(img_delete, scr_event, LV_EVENT_CLICKED, NULL);

            lv_obj_add_flag(img_back, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(img_delete, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(tmp, LV_OBJ_FLAG_HIDDEN);

            lv_scr_load_anim(scr_img, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, 0);

            return;
        }
        user_flag_bcd_t bcd = lv_obj_get_bcd(e->target);
        if (bcd == bcd_back)
        {
            lv_scr_load_anim(scr_main, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, 1);
            return;
        }
        if (bcd == bcd_delete)
        {
            if (max_img_num == 0)
                return;

            key_value_msg("del_prtscr", &choosed_num, 1); // 删文件
            list_del_node(choosed_num);                   // 删缩略图节点

            if (max_img_num == 0)
            {
                memset((void *)big_img_para->data, 0, 280 * 240 * 2);
                lv_obj_t *tmp = lv_label_create(scr_main);
                lv_label_set_text(tmp, "无图片");
                lv_obj_center(tmp);
                lv_obj_set_style_text_font(tmp, &album_app_font_24, 0);
            }
            else
            {
                *(uint8_t *)big_img_para->data = choosed_num; // 获取新大图
                key_value_msg("prtscr_get", (void *)big_img_para->data, 280 * 240 * 2);
            }

            lv_obj_set_style_bg_img_src(lv_obj_get_parent(lv_obj_get_parent(e->target)), big_img_para, 0); // 更新大图
            // lv_refr_now(lv_disp_get_default());
            //  lv_obj_redraw()

            lv_obj_t *cont = lv_obj_get_child(scr_img, 0); // 更新ID
            lv_obj_t *label = lv_obj_get_child(cont, 2);
            if (max_img_num)
                lv_label_set_text_fmt(label, "%d/%d", choosed_num + 1, max_img_num);
            else
                lv_label_set_text(label, "0/0");

            return;
        }
        if (bcd == bcd_big_img)
        {
            if (lv_obj_has_flag(lv_obj_get_child(e->target, 0), LV_OBJ_FLAG_HIDDEN))
            {
                lv_obj_clear_flag(lv_obj_get_child(e->target, 0), LV_OBJ_FLAG_HIDDEN);
            }
            else
            {
                lv_obj_add_flag(lv_obj_get_child(e->target, 0), LV_OBJ_FLAG_HIDDEN);
            }
            return;
        }
        if (bcd == bcd_fs_fmt)
        {
            key_value_msg("fs_fmt", NULL, 0);
        }
    }

    if (e->code == LV_EVENT_SCREEN_UNLOADED)
    {
        if (lv_scr_act() == scr_img)
            return;

        list_del_all_node();

        free((void *)big_img_para->data);
        free(big_img_para);
        return;
    }
}

static void album_app_load(void *arg)
{
    max_img_num = 0;

    big_img_para = malloc(sizeof(lv_img_dsc_t));
    img_format_init(big_img_para);
    big_img_para->header.h = 240;
    big_img_para->header.w = 280;
    big_img_para->data_size = 280 * 240 * LV_COLOR_SIZE / 8;
    big_img_para->data = malloc(280 * 240 * 2);

    scr_main = lv_obj_create(NULL);
    lv_obj_t *cont = lv_obj_create(scr_main);

    lv_obj_clear_flag(scr_main, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_center(cont);

    ref_img_small(cont);

    lv_obj_add_event_cb(scr_main, scr_event, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(scr_main, scr_event, LV_EVENT_SCREEN_UNLOADED, NULL);

    app_scr_load(scr_main, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0); // 加载APP界面
}

void album_app_install()
{
    app_config_t cfg = {
        .app_close = NULL,
        .app_init = NULL,
        .app_kill = NULL,
        .app_load = album_app_load,
        .app_power_off = NULL,
        .has_gui = 1,
        .icon = &album_app_icon,
        .name = "相册",
        .name_font = &album_app_font_24};
    app_install(&cfg, NULL);
}
