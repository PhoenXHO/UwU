#ifndef NATIVES_H_INCLUDED
#define NATIVES_H_INCLUDED

#include "value.h"
#include "timer.h"

typedef union
{
    double value;
    struct
    {
        uint32_t l_s_w;
        uint32_t m_s_w;
    } words;
} IEEE754_DOUBLE_W;

#define GET_WORDS(msw, lsw, x) \
    do \
    { \
        IEEE754_DOUBLE_W _double; \
        _double.value = (x); \
        (msw) = _double.words.m_s_w; \
        (lsw) = _double.words.l_s_w; \
    } while (0)

#define SET_WORDS(x, msw, lsw) \
    do \
    { \
        IEEE754_DOUBLE_W _double; \
        _double.words.m_s_w = (msw); \
        _double.words.l_s_w = (lsw); \
        (x) = _double.value; \
    } while (0)

Value _builtin__abs_(int, Value *);
Value _builtin__pow_(int, Value *);
Value _builtin__sqrt_(int, Value *);
Value _builtin__floor_(int, Value *);

#endif // NATIVES_H_INCLUDED
