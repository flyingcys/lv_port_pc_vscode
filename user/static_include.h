#ifndef static_include_H
#define static_include_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>

#define GUI_SIMULATOR_SPACE 1


#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "key_value_transation.h"
#include "app_manage.h"
#include "fast_lib.h"

// 1:开发者模式，不安装成熟的APP，加快烧录速度,0：全部安装，具体白名单列表可在app_list.c内自行修改
#define QF_ZERO_V2_DEV_EN 1

// 2：隐藏全部桌面表盘和菜单背景，加快开发烧录速度,1：隐藏除默认桌面表盘以外的所有表盘，0：使用全部表盘
#define QF_ZERO_V2_DESKTOP_HIDDEN_EN 1

typedef struct
{
    uint16_t now_x;   // 实时X坐标
    uint16_t now_y;   // 实时Y坐标
    uint16_t start_x; // 单次触摸的起始点x坐标
    uint16_t start_y; // 单次触摸的起始点y坐标
    uint16_t end_x;   // 单次触摸的结束点x坐标
    uint16_t end_y;   // 单次触摸的结束点y坐标
} cst816_point_t;

typedef enum
    {
        tp_gesture_none = 0,
        tp_gesture_down = 1,
        tp_gesture_up = 2,
        tp_gesture_right = 3,
        tp_gesture_left = 4,
        tp_gesture_long = 12
    } tp_gesture_type; // 根据芯片实际情况更改顺序

    typedef enum
    {
        tp_not_touch,
        tp_touching
    } tp_touch_state;

    typedef enum
    {
        tp_motion_left_right = 0x01,
        tp_motion_up_down = 0x02,
        tp_motion_long_press = 0x04
    } tp_motion_type;

typedef struct
{
    uint8_t chip_id;         // 芯片ID
    tp_touch_state state;    // 触摸状态,tp_not_touch,tp_touching
    tp_gesture_type gesture; // 滑动手势,tp_gesture_down/up/left/right/none
    cst816_point_t point;    // 起始、当前、结束的坐标值
} cst816_info_t;

typedef enum
{
    cst816_click,
    cst816_slided,
    cst816_pressed,
} cst816_slide_type_t;

typedef struct
{
    int16_t x_min;
    int16_t x_max;
    int16_t y_min;
    int16_t y_max;
    int16_t z_min;
    int16_t z_max;
} qmc_xyz_range_t;

#define RTC_FAST_ATTR


#define FS_PATH "/spiffs/"

int64_t esp_timer_get_time();

#define MALLOC_CAP_EXEC             (1<<0)  ///< Memory must be able to run executable code
#define MALLOC_CAP_32BIT            (1<<1)  ///< Memory must allow for aligned 32-bit data accesses
#define MALLOC_CAP_8BIT             (1<<2)  ///< Memory must allow for 8/16/...-bit data accesses
#define MALLOC_CAP_DMA              (1<<3)  ///< Memory must be able to accessed by DMA
#define MALLOC_CAP_PID2             (1<<4)  ///< Memory must be mapped to PID2 memory space (PIDs are not currently used)
#define MALLOC_CAP_PID3             (1<<5)  ///< Memory must be mapped to PID3 memory space (PIDs are not currently used)
#define MALLOC_CAP_PID4             (1<<6)  ///< Memory must be mapped to PID4 memory space (PIDs are not currently used)
#define MALLOC_CAP_PID5             (1<<7)  ///< Memory must be mapped to PID5 memory space (PIDs are not currently used)
#define MALLOC_CAP_PID6             (1<<8)  ///< Memory must be mapped to PID6 memory space (PIDs are not currently used)
#define MALLOC_CAP_PID7             (1<<9)  ///< Memory must be mapped to PID7 memory space (PIDs are not currently used)
#define MALLOC_CAP_SPIRAM           (1<<10) ///< Memory must be in SPI RAM
#define MALLOC_CAP_INTERNAL         (1<<11) ///< Memory must be internal; specifically it should not disappear when flash/spiram cache is switched off
#define MALLOC_CAP_DEFAULT          (1<<12) ///< Memory can be returned in a non-capability-specific memory allocation (e.g. malloc(), calloc()) call
#define MALLOC_CAP_IRAM_8BIT        (1<<13) ///< Memory must be in IRAM and allow unaligned access
#define MALLOC_CAP_RETENTION        (1<<14) ///< Memory must be able to accessed by retention DMA
#define MALLOC_CAP_RTCRAM           (1<<15) ///< Memory must be in RTC fast memory


void *heap_caps_malloc(size_t size,uint16_t malloc_cap);




#endif
