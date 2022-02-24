#ifndef VALUE_H_INCLUDED
#define VALUE_H_INCLUDED

#include "common.h"

class Object;
class String;

typedef enum
{
    V_BOOL,
    V_NUMBER,
    V_CHAR,
    V_OBJECT,
    V_NULL,
} ValueType;

typedef struct
{
    ValueType type;
    union {
        bool boolean;
        double number;
        char character;
        Object * object;
    } as;
} Value;

#define IS_BOOL(value)     ((value).type == V_BOOL)
#define IS_NUMBER(value)   ((value).type == V_NUMBER)
#define IS_CHAR(value)     ((value).type == V_CHAR)
#define IS_OBJECT(value)   ((value).type == V_OBJECT)

#define AS_BOOL(value)     ((value).as.boolean)
#define AS_NUMBER(value)   ((value).as.number)
#define AS_CHAR(value)     ((value).as.character)
#define AS_OBJECT(value)   ((value).as.object)

#define BOOL_VAL(value)    ((Value){V_BOOL,   {.boolean   = value}})
#define NUMBER_VAL(value)  ((Value){V_NUMBER, {.number    = value}})
#define CHAR_VAL(value)    ((Value){V_CHAR,   {.character = value}})
#define OBJECT_VAL(value)  ((Value){V_OBJECT, {.object    = (Object *)value}})

#define NULL_VAL           ((Value){V_NULL,   {.number    = 0}})

class ValueArray
{
    private:
        int _count;
        int _capacity;
        Value * _values;

    public:
        int vcount()      { return _count;    }
        int vcapacity()   { return _capacity; }
        Value * vvalues() { return _values;   }

        ValueArray() : _count(0), _capacity(0), _values(NULL) {}

        void init();
        void write(Value);
        void free();
};

bool values_equal(Value, Value);
void print_value(Value);

#endif // VALUE_H_INCLUDED
