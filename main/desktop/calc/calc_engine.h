#ifndef CALC_ENGINE_H
#define CALC_ENGINE_H
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CALC_BTN_0, CALC_BTN_1, CALC_BTN_2, CALC_BTN_3, CALC_BTN_4,
    CALC_BTN_5, CALC_BTN_6, CALC_BTN_7, CALC_BTN_8, CALC_BTN_9,
    CALC_BTN_DOT,
    CALC_BTN_ADD, CALC_BTN_SUB, CALC_BTN_MUL, CALC_BTN_DIV, CALC_BTN_POW,
    CALC_BTN_EQ,
    CALC_BTN_CLEAR, CALC_BTN_DELETE, CALC_BTN_PERCENT, CALC_BTN_SIGN,
    CALC_BTN_SIN, CALC_BTN_COS, CALC_BTN_TAN, CALC_BTN_LOG, CALC_BTN_LN,
    CALC_BTN_SQRT, CALC_BTN_SQUARE, CALC_BTN_CUBE,
    CALC_BTN_LPAREN, CALC_BTN_RPAREN, CALC_BTN_FACTORIAL,
    CALC_BTN_COUNT
} calc_btn_t;

typedef struct {
    char current[64];
    char previous[64];
    char expression[80];
    char op;
    bool should_reset;
    bool is_result;
} calc_engine_t;

void calc_engine_init(calc_engine_t * e);
const char * calc_engine_press(calc_engine_t * e, calc_btn_t btn);

#ifdef __cplusplus
}
#endif
#endif
