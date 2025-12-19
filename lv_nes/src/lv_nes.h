#ifndef LV_NES_H
#define LV_NES_H

#include <pthread.h>

#include "lvgl/lvgl.h"
#include "cart.h"
#include "cpu.h"
#include "ppu.h"

#define A_BUTTON 0x01U
#define B_BUTTON 0x02U
#define SELECT_BUTTON 0x04U
#define START_BUTTON 0x08U
#define UP_BUTTON 0x10U
#define DOWN_BUTTON 0x20U
#define LEFT_BUTTON 0x40U
#define RIGHT_BUTTON 0x80U

typedef struct
{
    lv_obj_t obj;
    uint16_t key_value;
    uint16_t flag;
    Cartridge *cart;
    CpuMapperShare *cpu_mapper;
    CpuPpuShare *cpu_ppu;
    Cpu6502 *cpu;
    Ppu2A03 *ppu;
    lv_obj_t *canvas;
    void *nes_pixels;
    lv_draw_img_dsc_t img_rect_dsc;
    lv_img_dsc_t *dsc;
    lv_obj_t *img;
} lv_nes_t;

void lv_nes_simple_test(void);
// static void *nes(void *arg);
lv_obj_t *lv_nes_create(lv_obj_t *parent);
uint16_t lv_nes_get_flag(lv_obj_t *obj);
uint16_t lv_nes_get_key_value(lv_obj_t *obj);
void lv_nes_clock_all_units(lv_obj_t *obj);

#endif /*LV_SKETCHPAD_H*/
