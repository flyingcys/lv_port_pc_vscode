#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "lvgl.h"
#include "../desktop/clock/clock_model.h"

/* 合法集合,与 clock_model.c 的静态数组一致 */
static const char * const k_months[] = {
    "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE",
    "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER",
};
static const char * const k_weekdays[] = {
    "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY",
};

static int in_set(const char * needle, const char * const * set, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (strcmp(needle, set[i]) == 0) return 1;
    }
    return 0;
}

static void test_period_mapping(void)
{
    /* 四类主值 */
    assert(clock_model_period_for_hour(8) == PERIOD_MORNING);
    assert(clock_model_period_for_hour(14) == PERIOD_AFTERNOON);
    assert(clock_model_period_for_hour(19) == PERIOD_EVENING);
    assert(clock_model_period_for_hour(23) == PERIOD_NIGHT);

    /* 边界: 5/11 晨起收尾, 12/16 午后, 17/22 傍晚, 4/0 深夜 */
    assert(clock_model_period_for_hour(5) == PERIOD_MORNING);
    assert(clock_model_period_for_hour(11) == PERIOD_MORNING);
    assert(clock_model_period_for_hour(12) == PERIOD_AFTERNOON);
    assert(clock_model_period_for_hour(16) == PERIOD_AFTERNOON);
    assert(clock_model_period_for_hour(17) == PERIOD_EVENING);
    assert(clock_model_period_for_hour(22) == PERIOD_EVENING);
    assert(clock_model_period_for_hour(4) == PERIOD_NIGHT);
    assert(clock_model_period_for_hour(0) == PERIOD_NIGHT);
    assert(clock_model_period_for_hour(23) == PERIOD_NIGHT);
    printf("test_period_mapping: OK\n");
}

static void test_build_labels_in_legal_set(void)
{
    clock_view_model_t vm;
    clock_model_build(&vm);

    assert(in_set(vm.month, k_months, sizeof k_months / sizeof k_months[0]));
    assert(in_set(vm.weekday, k_weekdays, sizeof k_weekdays / sizeof k_weekdays[0]));
    printf("test_build_labels_in_legal_set: OK (month=%s, weekday=%s)\n",
           vm.month, vm.weekday);
}

static void test_build_digits(void)
{
    clock_view_model_t vm;
    clock_model_build(&vm);

    /* 长度恒 6: digits[6] 固定数组,遍历 6 位均为数字 */
    for (int i = 0; i < 6; i++) {
        assert(vm.digits[i] >= '0' && vm.digits[i] <= '9');
    }

    /* 一致性: hour = d0*10+d1, 且 minute/second 在合法范围 */
    int hour = (vm.digits[0] - '0') * 10 + (vm.digits[1] - '0');
    int minute = (vm.digits[2] - '0') * 10 + (vm.digits[3] - '0');
    int second = (vm.digits[4] - '0') * 10 + (vm.digits[5] - '0');
    assert(hour >= 0 && hour <= 23);
    assert(minute >= 0 && minute <= 59);
    assert(second >= 0 && second <= 59);
    printf("test_build_digits: OK (%02d:%02d:%02d)\n", hour, minute, second);
}

static void test_build_temp_humidity(void)
{
    clock_view_model_t vm;
    clock_model_build(&vm);

    assert(vm.temp == 25.6f);
    assert(vm.humidity == 65.0f);
    printf("test_build_temp_humidity: OK (%.1f / %.1f)\n", vm.temp, vm.humidity);
}

static void test_delay_ms_range(void)
{
    unsigned delay = clock_model_delay_ms_to_next_second();
    assert(delay >= 1U && delay <= 1000U);
    printf("test_delay_ms_range: OK (%u ms)\n", delay);
}

int main(void)
{
    /* boilerplate: 最小 LVGL 初始化以链接 lvgl 与校验头文件可用性 */
    lv_init();
    lv_display_t * disp = lv_display_create(800, 480);
    assert(disp != NULL);
    lv_display_delete(disp);

    test_period_mapping();
    test_build_labels_in_legal_set();
    test_build_digits();
    test_build_temp_humidity();
    test_delay_ms_range();

    printf("ALL CLOCK MODEL TESTS PASSED\n");
    return 0;
}
