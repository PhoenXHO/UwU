#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

#include "common.h"
#include "chunk.h"

#define ALLOCATE_OBJECT(type, obj_type) \
    (type *)allocate_object(sizeof(type), obj_type)

#define OBJECT_TYPE(object) (AS_OBJECT(object)->type())

#define IS_STRING(value)    (is_object_type(value, O_STRING))
#define IS_FUNCTION(value)  (is_object_type(value, O_FUNCTION))

#define AS_STRING(value)    ((String *)AS_OBJECT(value))
#define AS_CSTRING(value)   (((String *)AS_OBJECT(value))->chars())
#define AS_FUNCTION(value)  ((Function *)AS_OBJECT(value))

typedef enum
{
    O_STRING,
    O_FUNCTION,
} ObjType;

class Object
{
    private:
        ObjType _type;
        Object * _next;

    public:
        ObjType & type()  { return _type; }
        Object * & next() { return _next; }

        void free();
};

class Function : public Object
{
    private:
        int _arity;
        Chunk _chunk;
        String * _name;

    public:
        int & arity()     { return _arity; }
        Chunk & chunk()   { return _chunk; }
        String * & name() { return _name;  }
};

class String : public Object
{
    private:
        int _length;
        char * _chars;

    public:
        int & length()   { return _length; }
        char * & chars() { return _chars;  }

        void free();
};

Function * new_function();
Object * allocate_object(size_t, ObjType);
String * take_string(char *, int);
String * copy_string(const char *, int);
bool is_object_type(Value, ObjType);
void print_object(Value);

#endif // OBJECT_H_INCLUDED
