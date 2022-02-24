#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

#include "common.h"
#include "value.h"

#define ALLOCATE_OBJECT(type, obj_type) \
    (type *)allocate_object(sizeof(type), obj_type)

#define OBJECT_TYPE(object) (AS_OBJECT(object)->type())

#define IS_STRING(value)    (is_object_type(value, O_STRING))

#define AS_STRING(value)    ((String *)AS_OBJECT(value))
#define AS_CSTRING(value)   (((String *)AS_OBJECT(value))->chars())

typedef enum
{
    O_STRING,
} ObjType;

class Object
{
    private:
        ObjType _type;
        Object * _next;

    public:
        ObjType type()  { return _type; }
        void type(ObjType __type)  { _type = __type; }
        Object * next() { return _next; }
        void next(Object * __next) { _next = __next; }

        void free();
};

class String : public Object
{
    private:
        int _length;
        char * _chars;

    public:
        int length()   { return _length; }
        void length(int __length)  { _length = __length; }
        char * chars() { return _chars;  }
        void chars(char * __chars) { _chars = __chars;   }

        void free();
};

Object * allocate_object(size_t, ObjType);
String * take_string(char *, int);
String * copy_string(const char *, int);
bool is_object_type(Value, ObjType);
void print_object(Value);

#endif // OBJECT_H_INCLUDED
