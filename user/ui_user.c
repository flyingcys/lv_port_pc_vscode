
// #include "lv_drivers/win32drv/win32drv.h"
// #include <sysinfoapi.h>

#include "system_app.h"
#include "weather_app.h"

/**
 * @brief 此示例工程修改了鼠标输入的坐标偏移以方便的查看鼠标指针图标，
 * 如不需要偏移，可在此文件跳转到第一个win32drv.h内修改宏定义为0即可
 */

cst816_info_t cst_info;

void *heap_caps_malloc(size_t size, uint16_t malloc_cap)
{
    return malloc(size);
}

void cst816_read_point(cst816_point_t *_point)
{
    *_point = cst_info.point;
}

cst816_slide_type_t cst816_read_slide_type()
{
    if (cst_info.state == 0) // 无触摸
    {
        int16_t cha = cst_info.point.start_x - cst_info.point.end_x;
        if (cha < 2 && cha > -2)
        {
            cha = cst_info.point.start_y - cst_info.point.end_y;
            if (cha < 2 && cha > -2)
            {
                return cst816_click;
            }
        }
        return cst816_slided;
    }
    // 有触摸
    int16_t cha = cst_info.point.start_x - cst_info.point.now_x;
    if (cha < 2 && cha > -2)
    {
        cha = cst_info.point.start_y - cst_info.point.now_y;
        if (cha < 2 && cha > -2)
        {
            return cst816_pressed;
        }
    }
    return cst816_slided;
}

int64_t esp_timer_get_time()
{
    static uint64_t t = 356;
    t += 1600;
    return t * 1000;
}

static void get_step_cb(void *value, size_t lenth)
{
    static uint32_t step = 99100;
    *(uint32_t *)value = step++;
    if (step == 100000)
        step = 0;

#if 1

    static cw2015_bat_info_t last_soc = {
        .soc = 0,
        .rtt = 1234,
        .voltage = 4123,
        .low_soc = 3};
    last_soc.soc++;
    if (last_soc.soc == 101)
        last_soc.soc = 0;
    system_upload_bat(&last_soc);

    static clock_time_t time;

    time.day++;
    if (time.day == 31)
    {
        time.day = 0;
        time.month++;
        if (time.month == 13)
            time.month = 0;
    }
    time.sec++;
    if (time.sec == 60)
    {
        time.sec = 0;
        time.min++;
        if (time.min == 60)
        {
            time.min = 0;
            time.hour++;
            if (time.hour = 24)
                time.hour = 0;
        }
    }

    system_upload_time(time);

#endif
}

static void get_altitude_cb(void *value, size_t lenth)
{
    static int16_t altitude = 1536;
    *(int16_t *)value = altitude++;
    if (altitude == 5000)
        altitude = 0;
}

static void get_angle_cb(void *value, size_t lenth)
{
    static int16_t angle = 0;

    angle += 1;
    if (angle > 180)
    {
        angle = -179 + (angle - 180);
        // key_value_msg("btn_click",NULL,0);
    }

    *(int16_t *)value = angle;
}

static void calibration_cb(void *value, size_t lenth)
{
    uint8_t flg = *(uint8_t *)value;

    if (flg)
    {
        printf("start calibration\n");
    }
    else
    {
        printf("end calibration\n");
    }
}

void ui_test()
{
    key_value_register(NULL, "mems_get_step", get_step_cb);
    key_value_register(NULL, "altitude_get", get_altitude_cb);
    key_value_register(NULL, "qmc_get_angle", get_angle_cb);
    key_value_register(NULL, "qmc_calibration", calibration_cb);

    system_init();

    app_load("时钟", app_handle_none, NULL);
}
uint8_t db_flg = 0;

void loop_task()
{
    if (db_flg == 0)
        return;
    db_flg = 0;
    printf("double\n");
    key_value_msg("tp_double", NULL, 0);
}
