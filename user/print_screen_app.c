

// #include "system_app.h"
// #include "print_screen_app.h"

// typedef enum
// {
//     id_free,
//     id_num,
//     id_full = 255,
// } id_type_t;

// LV_FONT_DECLARE(prtscr_app_font_24);

// static lv_obj_t *prt_scr = NULL;

// static void msg_box_close_cb(lv_timer_t *e)
// {
//     lv_obj_add_flag(lv_scr_act(), LV_OBJ_FLAG_CLICKABLE);
//     lv_obj_del((lv_obj_t *)e->user_data);
// }

// static void id_to_file_name(char *name, uint8_t id)
// {
//     sprintf(name, FS_PATH "IMG%02d", id);
// }

// static uint8_t get_id(id_type_t type)
// {
//     uint8_t cnt = 0;
//     char name[24];
//     struct stat st;

//     for (size_t i = 0; i < PRTSCR_MAX_NUM; i++)
//     {
//         id_to_file_name(name, i);
//         if (stat(name, &st) == 0)
//         {
//             cnt++;
//         }
//         else if (type == id_free)
//         {
//             return i;
//         }
//     }
//     if (type == id_free)
//         return id_full;
//     else
//         return cnt;
// }

// static void load_scr_cb(lv_timer_t *e)
// {
//     lv_scr_load_anim(prt_scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, 1);
// }

// static void take_p_cb(lv_timer_t *e)
// {
//     uint8_t id = get_id(id_free);

//     if (id == id_full)
//     {

//         lv_obj_t *cont = lv_obj_create(lv_scr_act());
//         lv_obj_set_size(cont, 200, 60);
//         lv_obj_set_style_radius(cont, 60 / 4, 0);
//         lv_obj_t *label = lv_label_create(cont);
//         lv_label_set_text(label, "存储空间已满");
//         lv_obj_set_style_text_font(label, &prtscr_app_font_24, 0);
//         lv_obj_center(cont);
//         lv_obj_center(label);
//         lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_CLICKABLE);

//         // msg_box = lv_msgbox_create(NULL, NULL, "", NULL, 0);
//         lv_timer_t *timer = lv_timer_create(msg_box_close_cb, 1500, cont);
//         lv_timer_set_repeat_count(timer, 1);
//         // lv_obj_center(msg_box);
//         // lv_obj_set_style_text_font(msg_box, );
//         // lv_obj_t *label = lv_msgbox_get_text(msg_box);
//         // lv_obj_center(label);

//         return;
//     }

//     lv_img_dsc_t snapshot;

//     uint32_t size = lv_snapshot_buf_size_needed(lv_scr_act(), LV_IMG_CF_TRUE_COLOR);
//     snapshot.data = malloc(size);
//     lv_snapshot_take_to_buf(lv_scr_act(), LV_IMG_CF_TRUE_COLOR, &snapshot, (void *)snapshot.data, size);

//     char name[24];
//     memset(name, 0, sizeof(name));
//     id_to_file_name(name, id);

//     // struct stat st;
//     // if (stat("/fat/hello.txt", &st) == 0)
//     // {
//     //     // Delete it if it exists
//     //     unlink(name);
//     // }

//     // size_t used = 0;

//     // key_value_msg("fs_used", &used, 0);

//     // used = 0;

//     // uint32_t bw;
//     // lv_fs_file_t f;
//     // lv_fs_open(&f, name, LV_FS_MODE_WR);
//     // lv_fs_write(&f, snapshot.data, size, &bw);
//     // lv_fs_close(&f);

//     FILE *f = fopen(name, "w");
//     // if (f == NULL)
//     // {
//     //     printf("Failed to open file for writing");
//     // }

//     fwrite(snapshot.data, sizeof(uint16_t), 280 * 240, f);

//     fclose(f);

//     // fsync()

//     // key_value_msg("fs_used", &used, 0);
//     // printf("used:%d\n", used);

//     free((void *)snapshot.data);

//     prt_scr = lv_scr_act();
//     lv_obj_t *obj = lv_obj_create(NULL);
//     lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
//     lv_scr_load_anim(obj, LV_SCR_LOAD_ANIM_NONE, 0, 0, 0);
//     lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), 0);

//     lv_timer_t *timer = lv_timer_create(load_scr_cb, 100, NULL);
//     lv_timer_set_repeat_count(timer, 1);
// }

// static void prtscr_take_cb(void *value, size_t lenth)
// {
//     uint16_t delay_ms = *(uint16_t *)value;
//     lv_timer_t *timer = lv_timer_create(take_p_cb, delay_ms, NULL);
//     lv_timer_set_repeat_count(timer, 1);
// }

// static void prtscr_delete_cb(void *value, size_t lenth)
// {
//     uint8_t id = *(uint8_t *)value;
//     char name[24];
//     id_to_file_name(name, id);

//     // printf("del:%s\n", name);

//     struct stat st;
//     if (stat(name, &st) == 0)
//     {
//         unlink(name);

//         for (size_t i = id + 1; i < PRTSCR_MAX_NUM; i++)
//         {
//             id_to_file_name(name, i);

//             if (stat(name, &st) == 0)
//             {
//                 char new_name[24];
//                 id_to_file_name(new_name, i - 1);

//                 rename(name, new_name);
//             }
//         }
//     }
// }

// static void prtscr_get_num_cb(void *value, size_t lenth)
// {
//     *(uint8_t *)value = get_id(id_num);
//     // printf("num:%d\n", get_id(id_num));
// }

// static void prtscr_get_cb(void *value, size_t lenth)
// {
//     uint8_t id = *(uint8_t *)value;

//     char name[24];
//     id_to_file_name(name, id);

//     // printf("get:%s\n", name);

//     FILE *f = fopen(name, "r");
//     // if (f == NULL)
//     //     return;

//     fread(value, sizeof(uint16_t), 280 * 240, f); // 读取大图

//     fclose(f); // 关闭文件
// }

// static void app_init()
// {
//     key_value_register(NULL, "take_prtscr", prtscr_take_cb);
//     key_value_register(NULL, "del_prtscr", prtscr_delete_cb);
//     key_value_register(NULL, "prtscr_num", prtscr_get_num_cb);
//     key_value_register(NULL, "prtscr_get", prtscr_get_cb);
// }

void print_screen_app_install()
{
//     app_config_t cfg = {
//         .app_close = NULL,
//         .app_init = app_init,
//         .app_kill = NULL,
//         .app_load = NULL,
//         .has_gui = 0,
//         .icon = NULL,
//         .name = "print_screen_app",
//         .name_font = NULL};
//     app_install(&cfg, NULL);
}
