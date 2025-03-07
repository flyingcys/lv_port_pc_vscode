#include "weather_app.h"

static weather_info_t weather;

static void get_weather_cb(void *value, size_t lenth)
{
    *(weather_info_t *)value = weather;
    weather.icon++;
    if (weather.icon == 7)
        weather.icon = 0;

    weather.low_temp++;
    if (weather.low_temp == 30)
        weather.low_temp = 10;
    weather.high_temp = weather.low_temp + 5;
}

static void weather_app_init(void *arg)
{
    weather.high_temp = 36;
    weather.low_temp = 22;
    weather.icon = cloudy;
    key_value_register(NULL, "weather_get", get_weather_cb);
}

void weather_app_install()
{

    app_config_t cfg = {
        .app_close = NULL,
        .app_init = weather_app_init,
        .app_kill = NULL,
        .app_load = NULL,
        .has_gui = 0,
        .icon = NULL,
        .name = "weather_app",
        .name_font = NULL};
    app_install(&cfg, NULL);
}