#include "calc_engine.h"
#include <stdio.h>
#include <string.h>

static int failures = 0;
static void check(const char * label, const char * got, const char * want) {
    if(strcmp(got, want) != 0) {
        fprintf(stderr, "FAIL %s: got '%s' want '%s'\n", label, got, want);
        failures++;
    }
}

int main(void)
{
    calc_engine_t e;
    const char * r;

    calc_engine_init(&e);
    r = calc_engine_press(&e, CALC_BTN_1); check("1", r, "1");
    r = calc_engine_press(&e, CALC_BTN_ADD); check("1+", r, "1");
    r = calc_engine_press(&e, CALC_BTN_2); check("2", r, "2");
    r = calc_engine_press(&e, CALC_BTN_EQ); check("1+2=", r, "3");

    calc_engine_init(&e);
    calc_engine_press(&e, CALC_BTN_5);
    calc_engine_press(&e, CALC_BTN_DIV);
    calc_engine_press(&e, CALC_BTN_0);
    r = calc_engine_press(&e, CALC_BTN_EQ); check("5/0", r, "Error");

    r = calc_engine_press(&e, CALC_BTN_7); check("after error", r, "7");

    calc_engine_init(&e);
    calc_engine_press(&e, CALC_BTN_2);
    calc_engine_press(&e, CALC_BTN_ADD);
    calc_engine_press(&e, CALC_BTN_3);
    calc_engine_press(&e, CALC_BTN_MUL);
    r = calc_engine_press(&e, CALC_BTN_4); check("2+3*4 mid", r, "4");
    r = calc_engine_press(&e, CALC_BTN_EQ); check("2+3*4 end", r, "20");

    calc_engine_init(&e);
    calc_engine_press(&e, CALC_BTN_1);
    calc_engine_press(&e, CALC_BTN_6);
    r = calc_engine_press(&e, CALC_BTN_SQRT); check("sqrt16", r, "4");

    calc_engine_init(&e);
    calc_engine_press(&e, CALC_BTN_5);
    r = calc_engine_press(&e, CALC_BTN_SQUARE); check("5^2", r, "25");

    calc_engine_init(&e);
    calc_engine_press(&e, CALC_BTN_5);
    r = calc_engine_press(&e, CALC_BTN_FACTORIAL); check("5!", r, "120");

    calc_engine_init(&e);
    calc_engine_press(&e, CALC_BTN_SUB);
    calc_engine_press(&e, CALC_BTN_3);
    r = calc_engine_press(&e, CALC_BTN_FACTORIAL); check("-3!", r, "Error");

    calc_engine_init(&e);
    calc_engine_press(&e, CALC_BTN_2);
    calc_engine_press(&e, CALC_BTN_POW);
    calc_engine_press(&e, CALC_BTN_3);
    r = calc_engine_press(&e, CALC_BTN_EQ); check("2^3", r, "8");

    calc_engine_init(&e);
    calc_engine_press(&e, CALC_BTN_5);
    r = calc_engine_press(&e, CALC_BTN_SIGN); check("-5", r, "-5");
    r = calc_engine_press(&e, CALC_BTN_SIGN); check("5 back", r, "5");

    calc_engine_init(&e);
    calc_engine_press(&e, CALC_BTN_5);
    calc_engine_press(&e, CALC_BTN_0);
    r = calc_engine_press(&e, CALC_BTN_PERCENT); check("50%", r, "0.5");

    calc_engine_init(&e);
    calc_engine_press(&e, CALC_BTN_1);
    calc_engine_press(&e, CALC_BTN_2);
    calc_engine_press(&e, CALC_BTN_3);
    r = calc_engine_press(&e, CALC_BTN_DELETE); check("del 123", r, "12");
    r = calc_engine_press(&e, CALC_BTN_DELETE); check("del 12", r, "1");
    r = calc_engine_press(&e, CALC_BTN_DELETE); check("del 1", r, "0");

    calc_engine_init(&e);
    calc_engine_press(&e, CALC_BTN_1);
    calc_engine_press(&e, CALC_BTN_DOT);
    r = calc_engine_press(&e, CALC_BTN_DOT); check("1..", r, "1.");

    if(failures == 0) { printf("test_calc_engine: PASS\n"); return 0; }
    printf("test_calc_engine: %d FAILURES\n", failures);
    return 1;
}
