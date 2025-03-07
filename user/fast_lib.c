#include "fast_lib.h"

inline int strcmp_my(const char *str1, const char *str2)
{
    for (; *str1 == *str2; ++str1, ++str2)
    {
        if (*str1 == '\0')
            return (0);
    }
    return 1;
}

uint8_t system_day_count_week(uint8_t year, uint8_t month, uint8_t day)
{
    int buf, buf2;
    if (month < 3)
    {
        buf = month + 12;
        buf2 = year - 1;
        if (buf2 < 0)
            buf2 = 99;
    }
    else
    {
        buf = month;
        buf2 = year;
    }

    return (uint8_t)(day + 2 * month + 3 * (month + 1) / 5 + year + year / 4 - year / 100 + year / 400 + 1) % 7;
}