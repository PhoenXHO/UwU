#include "natives.h"
#include <math.h>

static double __power__(double b, int e)
{
    double r = 1;
    if (e < 0) { e *= -1; b = 1 / b; }
    while (e > 0)
    {
        if (e % 2 == 1) r *= b;

        e >>= 1; b *= b;
    }

    return r;
}

static const double __H = 1.0e300;

static int __floor__(double d)
{
    uint32_t msw, lsw, exp, x, y;
    GET_WORDS(msw, lsw, d);

    exp = ((msw >> 20) & 0x7ff) - 0x3ff;
    if (exp < 20)
    {
        if (exp < 0)
        {
            if (__H + d > 0)
            {
                if (msw >= 0) { msw = lsw = 0; }
                else if (((msw & 0x7fffffff) | lsw) != 0) { msw = 0xbff00000; lsw = 0; }
            }
        }
        else
        {
            x = (0x000fffff) >> exp;
            if (((msw & x) | lsw) == 0) return d;
            if (__H + d > 0)
            {
                if (msw < 0) { msw += (0x00100000) >> exp; }
                msw &= (~x);
                lsw = 0;
            }
        }
    }
    else if (exp > 51)
    {
        if (exp == 0x400) return d + d;
        return d;
    }
    else
    {
        x = ((uint32_t)(0xffffffff)) >> (exp - 20);
        if ((lsw & x) == 0) return d;
        if (__H + d > 0)
        {
            if (msw > 0)
            {
                if (exp == 0) msw += 1;
                else
                {
                    y = lsw + (1 << (52 - exp));
                    if (y < lsw) msw += 1;
                    lsw = y;
                }
            }
            lsw &= (~x);
        }
    }

    SET_WORDS(d, msw, lsw);
    return d;
}

Value _builtin__abs_(int, Value * arg_list)
{
    if (!IS_NUMBER(arg_list[0])) return NULL_VAL;

    double x = AS_NUMBER(arg_list[0]);
    return NUMBER_VAL(x >= 0 ? x : -x);
}

Value _builtin__pow_(int, Value * arg_list)
{
    if (!IS_NUMBER(arg_list[0]) || !IS_NUMBER(arg_list[1])) return NULL_VAL;

    return NUMBER_VAL(__power__(AS_NUMBER(arg_list[0]), (int)AS_NUMBER(arg_list[1])));
}

Value _builtin__sqrt_(int, Value * arg_list)
{
    if (!IS_NUMBER(arg_list[0])) return NULL_VAL;

    double n = AS_NUMBER(arg_list[0]);

    if (n < 0) return NULL_VAL;

    double x = n;
    double y = 1;
    double e = 1e-12;

    while (x - y > e)
    {
        x = (x + y) / 2;
        y = n / x;
    }

    return NUMBER_VAL(x);
}

Value _builtin__floor_(int, Value * arg_list)
{
    return NUMBER_VAL((double)__floor__(AS_NUMBER(arg_list[0])));
}
