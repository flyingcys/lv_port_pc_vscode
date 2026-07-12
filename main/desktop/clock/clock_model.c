#include "clock_model.h"

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

/* 与 script.js 逐字一致(JANUARY..DECEMBER) */
static const char * const g_months[] = {
    "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE",
    "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER",
};

/* 与 script.js 逐字一致(SUNDAY..SATURDAY) */
static const char * const g_weekdays[] = {
    "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY",
};

clock_period_t clock_model_period_for_hour(int hour)
{
    if (hour >= 5 && hour <= 11) return PERIOD_MORNING;
    if (hour >= 12 && hour <= 16) return PERIOD_AFTERNOON;
    if (hour >= 17 && hour <= 22) return PERIOD_EVENING;
    return PERIOD_NIGHT;
}

/* 两位补零写入 dst[0..1],n 范围 0..99 */
static void write_two_digits(char *dst, int n)
{
    dst[0] = (char)('0' + n / 10);
    dst[1] = (char)('0' + n % 10);
}

void clock_model_build(clock_view_model_t *out)
{
    time_t now;
    struct tm tm_now;
    static char year_buf[8];

    time(&now);
    localtime_r(&now, &tm_now);

    out->month = g_months[tm_now.tm_mon];       /* tm_mon: 0..11 */
    out->weekday = g_weekdays[tm_now.tm_wday];   /* tm_wday: 0..6 */
    out->period = clock_model_period_for_hour(tm_now.tm_hour);
    out->day_of_month = (unsigned)tm_now.tm_mday;

    /* 四位年份字符串,指向静态缓冲 */
    snprintf(year_buf, sizeof(year_buf), "%d", tm_now.tm_year + 1900);
    out->year = year_buf;

    /* 复刻 getTimeParts: H1H2M1M2S1S2 */
    write_two_digits(&out->digits[0], tm_now.tm_hour);
    write_two_digits(&out->digits[2], tm_now.tm_min);
    write_two_digits(&out->digits[4], tm_now.tm_sec);

    out->temp = 25.6f;
    out->humidity = 65.0f;
}

unsigned clock_model_delay_ms_to_next_second(void)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    unsigned ms = (unsigned)(now.tv_usec / 1000U);
    /* ms 属于 [0, 999],结果属于 [1, 1000] */
    return 1000U - ms;
}
