#ifndef CLOCK_MODEL_H
#define CLOCK_MODEL_H

/* 时段枚举，复刻 script.js 的 getPeriodLabel */
typedef enum {
    PERIOD_MORNING,    /*  5:00 - 11:59 */
    PERIOD_AFTERNOON,  /* 12:00 - 16:59 */
    PERIOD_EVENING,    /* 17:00 - 22:59 */
    PERIOD_NIGHT,      /* 23:00 -  4:59 */
} clock_period_t;

/* 视图模型。month/weekday/year 指向静态存储,调用方无需释放 */
typedef struct {
    const char *month;        /* JANUARY..DECEMBER */
    const char *weekday;      /* SUNDAY..SATURDAY */
    clock_period_t period;
    const char *year;         /* 四位数字字符串 */
    unsigned day_of_month;    /* 1..31 */
    char digits[6];           /* '0'..'9' H1H2M1M2S1S2 */
    float temp;               /* 固定 25.6 */
    float humidity;           /* 固定 65.0 */
} clock_view_model_t;

/* 按时段映射小时(0..23 -> period)。抽成可测辅助,供单元测试直接断言 */
clock_period_t clock_model_period_for_hour(int hour);

/* 读取系统时间并填充视图模型。复刻 script.js 的 buildViewModel */
void clock_model_build(clock_view_model_t *out);

/* 到下一整秒的毫秒数,用于对齐刷新边界。复刻 getDelayToNextSecond */
unsigned clock_model_delay_ms_to_next_second(void);

#endif /* CLOCK_MODEL_H */
