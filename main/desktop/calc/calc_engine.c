/*
 * calc_engine.c — lvgl-free 计算引擎核心
 *
 * 逻辑移植自 design-ui/calc/calc.html 的 JS 实现。
 * 零 lvgl 依赖，可独立 gcc 编译测试。
 *
 * 与 JS 的差异（以测试预期为准）：
 *   1. calculate() 实现 '^'(pow) 分支（JS switch 漏写该 case，走 default 不算）。
 *   2. 初始/clear 态按 SUB 输入负号前缀（JS 总当减法，无法输入负数）。
 *   3. Error 态按数字键替换（JS 会追加成 "Error7"）。
 *   4. sqrt 负数直接判 Error（JS 靠 Math.sqrt 产生 NaN 后统一转 Error，等价）。
 *
 * JS 的 String(parseFloat(x.toPrecision(12))) 用 %.12g 近似（%g 自动去尾零）。
 */
#include "calc_engine.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* 内部小工具                                                          */
/* ------------------------------------------------------------------ */

static bool calc_is_error(const calc_engine_t * e)
{
    return strcmp(e->current, "Error") == 0;
}

static void calc_set_current(calc_engine_t * e, const char * s)
{
    strncpy(e->current, s, sizeof(e->current) - 1);
    e->current[sizeof(e->current) - 1] = '\0';
}

static void calc_append_char(calc_engine_t * e, char c)
{
    size_t len = strlen(e->current);
    if (len < sizeof(e->current) - 1) {
        e->current[len] = c;
        e->current[len + 1] = '\0';
    }
}

/* JS: String(parseFloat(result.toPrecision(12))) — %.12g 近似，自动去尾零 */
static void calc_format_result(calc_engine_t * e, double result)
{
    snprintf(e->current, sizeof(e->current), "%.12g", result);
}

static void calc_clear_expression(calc_engine_t * e)
{
    e->expression[0] = '\0';
}

/* ------------------------------------------------------------------ */
/* handleNumber                                                        */
/* ------------------------------------------------------------------ */
static void calc_handle_number(calc_engine_t * e, const char * digit)
{
    /* Error 态视为 reset：按数字替换（JS 会追加，测试预期替换） */
    if (calc_is_error(e)) {
        calc_set_current(e, digit);
        e->should_reset = false;
        e->is_result = false;
        return;
    }
    /* "0" 或 reset 态：替换；否则追加 */
    if (strcmp(e->current, "0") == 0 || e->should_reset) {
        calc_set_current(e, digit);
        e->should_reset = false;
    } else {
        calc_append_char(e, digit[0]);
    }
    e->is_result = false;
}

/* ------------------------------------------------------------------ */
/* calculate                                                           */
/* ------------------------------------------------------------------ */
static void calc_calculate(calc_engine_t * e)
{
    if (e->op == 0 || e->previous[0] == '\0') {
        return;
    }

    /* JS: isNaN(prev) || isNaN(curr) -> return */
    char * end_prev;
    char * end_curr;
    double prev = strtod(e->previous, &end_prev);
    double curr = strtod(e->current, &end_curr);
    if (end_prev == e->previous || end_curr == e->current) {
        return;
    }

    double result;
    switch (e->op) {
        case '+': result = prev + curr; break;
        case '-': result = prev - curr; break;
        case '*': result = prev * curr; break;
        case '/':
            if (curr == 0) {
                calc_set_current(e, "Error");
                e->op = 0;
                e->previous[0] = '\0';
                e->should_reset = true;
                e->is_result = true;
                calc_clear_expression(e);
                return;
            }
            result = prev / curr;
            break;
        case '^': result = pow(prev, curr); break;
        default: return;
    }

    if (!isfinite(result)) {
        calc_set_current(e, "Error");
        e->op = 0;
        e->previous[0] = '\0';
        e->should_reset = true;
        e->is_result = true;
        calc_clear_expression(e);
        return;
    }

    calc_format_result(e, result);
    e->op = 0;
    e->previous[0] = '\0';
    e->should_reset = true;
    e->is_result = true;
    calc_clear_expression(e);
}

/* ------------------------------------------------------------------ */
/* handleOperator                                                      */
/* ------------------------------------------------------------------ */
static void calc_handle_operator(calc_engine_t * e, char op)
{
    /* 先算挂起的运算（op!=0 且非 reset 态） */
    if (e->op != 0 && !e->should_reset) {
        calc_calculate(e);
        /* calculate 可能除0置 Error 并清空 op，此时不再记录新运算 */
    }

    /* previous = current；记录 op；expression = "previous op"；reset */
    strncpy(e->previous, e->current, sizeof(e->previous) - 1);
    e->previous[sizeof(e->previous) - 1] = '\0';
    e->op = op;
    snprintf(e->expression, sizeof(e->expression), "%s %c", e->previous, op);
    e->should_reset = true;
}

/* ------------------------------------------------------------------ */
/* handleAdvanced                                                      */
/* ------------------------------------------------------------------ */
static void calc_handle_advanced(calc_engine_t * e, calc_btn_t btn)
{
    /* 括号：追加字符（与 JS 一致，不设 reset） */
    if (btn == CALC_BTN_LPAREN) {
        calc_append_char(e, '(');
        return;
    }
    if (btn == CALC_BTN_RPAREN) {
        calc_append_char(e, ')');
        return;
    }
    /* 幂运算走运算符通道 */
    if (btn == CALC_BTN_POW) {
        calc_handle_operator(e, '^');
        return;
    }

    /* 其余需要解析 current 为数字 */
    char * end;
    double num = strtod(e->current, &end);
    if (end == e->current) {
        /* JS: parseFloat 失败 (NaN) -> return */
        return;
    }

    double result;
    switch (btn) {
        case CALC_BTN_SIN: result = sin(num); break;
        case CALC_BTN_COS: result = cos(num); break;
        case CALC_BTN_TAN: result = tan(num); break;
        case CALC_BTN_LOG: result = log10(num); break;
        case CALC_BTN_LN:  result = log(num); break;
        case CALC_BTN_SQRT:
            if (num < 0) {
                calc_set_current(e, "Error");
                e->should_reset = true;
                e->is_result = true;
                return;
            }
            result = sqrt(num);
            break;
        case CALC_BTN_SQUARE: result = pow(num, 2); break;
        case CALC_BTN_CUBE:   result = pow(num, 3); break;
        case CALC_BTN_FACTORIAL:
            if (num < 0 || num != floor(num)) {
                calc_set_current(e, "Error");
                e->should_reset = true;
                e->is_result = true;
                return;
            }
            if (num > 170) {
                calc_set_current(e, "Error");
                e->should_reset = true;
                e->is_result = true;
                return;
            }
            result = 1.0;
            for (int i = 2; i <= (int)num; i++) {
                result *= i;
            }
            break;
        default: return;
    }

    /* JS: if(!isNaN(result) && isFinite(result)) ... else 'Error' */
    if (!isnan(result) && isfinite(result)) {
        calc_format_result(e, result);
    } else {
        calc_set_current(e, "Error");
    }
    e->should_reset = true;
    e->is_result = true;
}

/* ------------------------------------------------------------------ */
/* toggleSign / percent / delete                                       */
/* ------------------------------------------------------------------ */
static void calc_toggle_sign(calc_engine_t * e)
{
    if (strcmp(e->current, "0") == 0) {
        return;
    }
    if (strcmp(e->current, "-") == 0) {
        calc_set_current(e, "0");
        return;
    }
    if (e->current[0] == '-') {
        memmove(e->current, e->current + 1, strlen(e->current));
    } else {
        size_t len = strlen(e->current);
        if (len < sizeof(e->current) - 1) {
            memmove(e->current + 1, e->current, len + 1);
            e->current[0] = '-';
        }
    }
}

static void calc_percent(calc_engine_t * e)
{
    char * end;
    double num = strtod(e->current, &end);
    if (end == e->current) {
        return;
    }
    calc_format_result(e, num / 100.0);
    e->should_reset = true;
    e->is_result = true;
}

static void calc_delete(calc_engine_t * e)
{
    if (strlen(e->current) > 1) {
        e->current[strlen(e->current) - 1] = '\0';
    } else {
        calc_set_current(e, "0");
    }
}

/* ------------------------------------------------------------------ */
/* 公开 API                                                            */
/* ------------------------------------------------------------------ */
void calc_engine_init(calc_engine_t * e)
{
    memset(e, 0, sizeof(*e));
    calc_set_current(e, "0");
    e->previous[0] = '\0';
    e->expression[0] = '\0';
    e->op = 0;
    e->should_reset = false;
    e->is_result = false;
}

const char * calc_engine_press(calc_engine_t * e, calc_btn_t btn)
{
    /* 数字 0-9 */
    if (btn >= CALC_BTN_0 && btn <= CALC_BTN_9) {
        char digit[2] = { (char)('0' + btn), '\0' };
        calc_handle_number(e, digit);
        return e->current;
    }

    /* 小数点：去重 */
    if (btn == CALC_BTN_DOT) {
        if (strchr(e->current, '.') != NULL) {
            return e->current;
        }
        calc_handle_number(e, ".");
        return e->current;
    }

    /* 运算符 */
    if (btn == CALC_BTN_ADD) { calc_handle_operator(e, '+'); return e->current; }
    if (btn == CALC_BTN_MUL) { calc_handle_operator(e, '*'); return e->current; }
    if (btn == CALC_BTN_DIV) { calc_handle_operator(e, '/'); return e->current; }
    if (btn == CALC_BTN_POW) { calc_handle_operator(e, '^'); return e->current; }
    if (btn == CALC_BTN_SUB) {
        /*
         * 初始/clear 态（current=="0" 且无挂起运算且非 reset）按 SUB 输入负号
         * 前缀，使后续数字追加为负数（测试 -3! 期望此行为）。
         * 其余情形 SUB 为减法运算符。
         */
        if (strcmp(e->current, "0") == 0 &&
            e->previous[0] == '\0' &&
            e->op == 0 &&
            !e->should_reset) {
            calc_set_current(e, "-");
            e->should_reset = false;
            return e->current;
        }
        calc_handle_operator(e, '-');
        return e->current;
    }

    /* 等号 */
    if (btn == CALC_BTN_EQ) {
        calc_calculate(e);
        return e->current;
    }

    /* 清除 / 删除 / 百分号 / 正负号 */
    if (btn == CALC_BTN_CLEAR) {
        calc_set_current(e, "0");
        e->previous[0] = '\0';
        e->op = 0;
        e->should_reset = false;
        e->is_result = false;
        calc_clear_expression(e);
        return e->current;
    }
    if (btn == CALC_BTN_DELETE) { calc_delete(e);            return e->current; }
    if (btn == CALC_BTN_PERCENT){ calc_percent(e);           return e->current; }
    if (btn == CALC_BTN_SIGN)   { calc_toggle_sign(e);       return e->current; }

    /* 高级科学键 */
    calc_handle_advanced(e, btn);
    return e->current;
}
