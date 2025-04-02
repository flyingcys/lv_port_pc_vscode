#include "lv_nes.h"

#include <stdio.h>
#include <stdlib.h>

#define MY_CLASS &lv_nes_class

static void lv_nes_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
static void lv_nes_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
static void lv_nes_event(const lv_obj_class_t *class_p, lv_event_t *e);

static void lv_nes_run_timer(lv_timer_t *timer);
static void clock_all_units(lv_obj_t *obj);

static void ctl_btn_event(lv_event_t *e);
static void init_ctl_btn(lv_obj_t *obj);

const lv_obj_class_t lv_nes_class = {
    .constructor_cb = lv_nes_constructor,
    .destructor_cb = lv_nes_destructor,
    .event_cb = lv_nes_event,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(lv_nes_t),
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .base_class = &lv_obj_class};

lv_obj_t *lv_nes_create(lv_obj_t *parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t *obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

uint16_t lv_nes_get_key_value(lv_obj_t *obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_nes_t *nes = (lv_nes_t *)obj;

    return nes->key_value;
}

uint16_t lv_nes_get_flag(lv_obj_t *obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_nes_t *nes = (lv_nes_t *)obj;

    return nes->flag;
}

static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(256, 240)];

static void *nes(void *arg)
{
    lv_obj_t *obj = (lv_obj_t *)arg;
    lv_nes_t *nes = (lv_nes_t *)obj;

    while (1)
    {
        if (nes->ppu->cpu_ppu_io->ppu_status & 0x80)
        {
            uint16_t flag = lv_nes_get_flag(obj);
            uint16_t key_value = lv_nes_get_key_value(obj);
            if (flag == 1)
            {
                switch (key_value)
                {
                case SELECT_BUTTON:
                    nes->cpu->player_1_controller &= ~SELECT_BUTTON;
                    break;
                case START_BUTTON:
                    nes->cpu->player_1_controller &= ~START_BUTTON;
                    break;
                case UP_BUTTON:
                    nes->cpu->player_1_controller &= ~UP_BUTTON;
                    break;
                case DOWN_BUTTON:
                    nes->cpu->player_1_controller &= ~DOWN_BUTTON;
                    break;
                case LEFT_BUTTON:
                    nes->cpu->player_1_controller &= ~LEFT_BUTTON;
                    break;
                case RIGHT_BUTTON:
                    nes->cpu->player_1_controller &= ~RIGHT_BUTTON;
                    break;
                case A_BUTTON:
                    nes->cpu->player_1_controller &= ~A_BUTTON;
                    break;
                case B_BUTTON:
                    nes->cpu->player_1_controller &= ~B_BUTTON;
                    break;
                default:
                    break;
                }
                nes->flag = 0;
            }
        }
        if (nes->ppu->cpu_ppu_io->ppu_status & 0x80)
        {
            uint16_t flag = lv_nes_get_flag(obj);
            uint16_t key_value = lv_nes_get_key_value(obj);
            if (flag == 2)
            {
                switch (key_value)
                {
                case SELECT_BUTTON:
                    nes->cpu->player_1_controller |= SELECT_BUTTON;
                    break;
                case START_BUTTON:
                    nes->cpu->player_1_controller |= START_BUTTON;
                    break;
                case UP_BUTTON:
                    nes->cpu->player_1_controller |= UP_BUTTON;
                    break;
                case DOWN_BUTTON:
                    nes->cpu->player_1_controller |= DOWN_BUTTON;
                    break;
                case LEFT_BUTTON:
                    nes->cpu->player_1_controller |= LEFT_BUTTON;
                    break;
                case RIGHT_BUTTON:
                    nes->cpu->player_1_controller |= RIGHT_BUTTON;
                    break;
                case A_BUTTON:
                    nes->cpu->player_1_controller |= A_BUTTON;
                    break;
                case B_BUTTON:
                    nes->cpu->player_1_controller |= B_BUTTON;
                    break;
                default:
                    break;
                }
            }
        }
        lv_nes_clock_all_units(obj);
    }
}

void lv_nes_simple_test(void)
{
    lv_obj_t *nes_obj = lv_nes_create(lv_scr_act());
    lv_obj_center(nes_obj);

    pthread_t nesThread;
    pthread_create(&nesThread, NULL, nes, nes_obj);
}

static void lv_nes_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_nes_t *nes = (lv_nes_t *)obj;

    nes->key_value = 0;
    nes->flag = 0;

    nes->dsc = lv_mem_alloc(sizeof(lv_img_dsc_t));
    lv_memset_00(nes->dsc, sizeof(lv_img_dsc_t));
    nes->dsc->data = NULL;
    nes->dsc->header.w = 256;
    nes->dsc->header.h = 240;
    nes->dsc->header.cf = LV_IMG_CF_TRUE_COLOR;

    nes->img = lv_img_create(lv_scr_act());
    lv_obj_center(nes->img);
    lv_img_set_zoom(nes->img, 512);
    lv_img_set_src(nes->img, nes->dsc);
    
    nes->cart = cart_init();
    nes->cpu_mapper = cpu_mapper_init(nes->cart);
    nes->cpu_ppu = mmio_init();
    nes->cpu = cpu_init(0xC000, nes->cpu_ppu, nes->cpu_mapper);
    nes->ppu = ppu_init(nes->cpu_ppu);

    init_ctl_btn(obj);

    char *cart_file_name = "A:/home/share/samba/lv_port_pc_vscode-v8/main/src/lv_nes/dummy.nes";

    if (load_cart(nes->cart, cart_file_name, nes->cpu, nes->ppu) != 0)
    {
        LV_LOG_ERROR("Cartage File Not Found: %s", cart_file_name);
        return;
    }
    else
    {
        init_pc(nes->cpu);
        update_cpu_info(nes->cpu);
    }

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_nes_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_nes_t *nes = (lv_nes_t *)obj;

    free(nes->cart);
    free(nes->cpu_mapper);
    free(nes->cpu_ppu);
    free(nes->cpu);
    free(nes->ppu);
}

static void ctl_btn_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_user_data(e);
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);

    lv_nes_t *nes = (lv_nes_t *)obj;

    char *label_text = NULL;
    if (code == LV_EVENT_RELEASED)
    {
        label_text = lv_label_get_text(label);
        if (strcmp(label_text, "SELECT") == 0)
        {
            nes->key_value = SELECT_BUTTON;
        }
        else if (strcmp(label_text, "START") == 0)
        {
            nes->key_value = START_BUTTON;
        }
        else if (strcmp(label_text, "A") == 0)
        {
            nes->key_value = A_BUTTON;
        }
        else if (strcmp(label_text, "B") == 0)
        {
            nes->key_value = B_BUTTON;
        }
        else if (strcmp(label_text, LV_SYMBOL_UP) == 0)
        {
            nes->key_value = UP_BUTTON;
        }
        else if (strcmp(label_text, LV_SYMBOL_DOWN) == 0)
        {
            nes->key_value = DOWN_BUTTON;
        }
        else if (strcmp(label_text, LV_SYMBOL_LEFT) == 0)
        {
            nes->key_value = LEFT_BUTTON;
        }
        else if (strcmp(label_text, LV_SYMBOL_RIGHT) == 0)
        {
            nes->key_value = RIGHT_BUTTON;
        }
        nes->flag = 1;
    }
    else if (code == LV_EVENT_PRESSING)
    {
        label_text = lv_label_get_text(label);
        if (strcmp(label_text, "SELECT") == 0)
        {
            nes->key_value = SELECT_BUTTON;
        }
        else if (strcmp(label_text, "START") == 0)
        {
            nes->key_value = START_BUTTON;
        }
        else if (strcmp(label_text, "A") == 0)
        {
            nes->key_value = A_BUTTON;
        }
        else if (strcmp(label_text, "B") == 0)
        {
            nes->key_value = B_BUTTON;
        }
        else if (strcmp(label_text, LV_SYMBOL_UP) == 0)
        {
            nes->key_value = UP_BUTTON;
        }
        else if (strcmp(label_text, LV_SYMBOL_DOWN) == 0)
        {
            nes->key_value = DOWN_BUTTON;
        }
        else if (strcmp(label_text, LV_SYMBOL_LEFT) == 0)
        {
            nes->key_value = LEFT_BUTTON;
        }
        else if (strcmp(label_text, LV_SYMBOL_RIGHT) == 0)
        {
            nes->key_value = RIGHT_BUTTON;
        }
        nes->flag = 2;
    }
}

static void init_ctl_btn(lv_obj_t *obj)
{
    lv_obj_t *label;
    lv_obj_t *btn;

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 150, 75);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, -140, -20);
    lv_obj_add_event_cb(btn, ctl_btn_event, LV_EVENT_ALL, obj);

    label = lv_label_create(btn);
    lv_label_set_text(label, "SELECT");
    lv_obj_center(label);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 150, 75);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 140, -20);
    lv_obj_add_event_cb(btn, ctl_btn_event, LV_EVENT_ALL, obj);

    label = lv_label_create(btn);
    lv_label_set_text(label, "START");
    lv_obj_center(label);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 100, 100);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -140, -100);
    lv_obj_add_event_cb(btn, ctl_btn_event, LV_EVENT_ALL, obj);

    label = lv_label_create(btn);
    lv_label_set_text(label, "A");
    lv_obj_center(label);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 100, 100);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -20, -100);
    lv_obj_add_event_cb(btn, ctl_btn_event, LV_EVENT_ALL, obj);

    label = lv_label_create(btn);
    lv_label_set_text(label, "B");
    lv_obj_center(label);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 100, 100);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 150, -140);
    lv_obj_add_event_cb(btn, ctl_btn_event, LV_EVENT_ALL, obj);

    label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_UP);
    lv_obj_center(label);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 100, 100);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 150, -20);
    lv_obj_add_event_cb(btn, ctl_btn_event, LV_EVENT_ALL, obj);

    label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_DOWN);
    lv_obj_center(label);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 100, 100);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 40, -80);
    lv_obj_add_event_cb(btn, ctl_btn_event, LV_EVENT_ALL, obj);

    label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_center(label);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 100, 100);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 260, -80);
    lv_obj_add_event_cb(btn, ctl_btn_event, LV_EVENT_ALL, obj);

    label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_RIGHT);
    lv_obj_center(label);
}

static void lv_nes_event(const lv_obj_class_t *class_p, lv_event_t *e)
{
    static bool key_state = false;

    LV_UNUSED(class_p);

    lv_res_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RES_OK)
        return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_nes_t *nes = (lv_nes_t *)obj;
}

void lv_nes_clock_all_units(lv_obj_t *obj)
{
    lv_nes_t *nes = (lv_nes_t *)obj;
    clock_cpu(nes->cpu);
    clock_ppu(nes->ppu, nes->cpu, obj);
}
