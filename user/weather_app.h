#ifndef weather_app_H
#define weather_app_H

#include "static_include.h"
typedef enum
{
    sunny,   // 晴天
    cloudy,  // 多云
    rain,    // 雨
    snow,    // 雪
    haze,    // 雾霾
    dust,    // 扬尘
    overcast // 阴天
} weather_icon_type_t;

typedef struct
{
    weather_icon_type_t icon;
    uint8_t low_temp;
    uint8_t high_temp;
} weather_info_t;

void weather_app_install();

#endif
