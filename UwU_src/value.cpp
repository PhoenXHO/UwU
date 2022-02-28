#include "value.h"
#include "object.h"
#include "memory.h"

void ValueArray::write(Value value)
{
    if (_capacity < _count + 1)
    {
        int old_capacity = _capacity;

        _capacity = GROW_CAPACITY(old_capacity);
        _values = GROW_ARRAY(Value, _values, _capacity);
    }

    _values[_count] = value;
    _count++;
}

void ValueArray::init()
{
    _count = _capacity = 0; _values = NULL;
}

void ValueArray::free()
{
    FREE_ARRAY(Value, _values);
    init();
}

bool values_equal(Value a, Value b)
{
    if (a.type != b.type) return false;

    switch (a.type)
    {
        case V_BOOL:   return AS_BOOL(a)   == AS_BOOL(b);
        case V_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
        case V_CHAR:   return AS_CHAR(a)   == AS_CHAR(b);
        case V_OBJECT: return AS_OBJECT(a) == AS_OBJECT(b);

        default: return false;
    }
}

void print_value(Value value)
{
    switch (value.type)
    {
        case V_BOOL:
            printf(AS_BOOL(value) ? "twue" : "fawse");
            break;
        case V_NUMBER:
            printf("%.15g", AS_NUMBER(value));
            break;
        case V_CHAR:
            printf("%c", AS_CHAR(value));
            break;
        case V_OBJECT:
            print_object(value);
            break;

        default: return;
    }

    fflush(stdout);
}
