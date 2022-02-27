#include "memory.h"
#include "object.h"
#include "vm.h"

extern VM vm;

Object * allocate_object(size_t _size, ObjType type)
{
    Object * object = (Object *)reallocate(NULL, _size);
    object->type() = type;

    object->next() = vm.objects;
    vm.objects = object;

    return object;
}

Function * new_function()
{
    Function * _function = ALLOCATE_OBJECT(Function, O_FUNCTION);
    _function->arity() = 0;
    _function->name() = NULL;
    _function->chunk().init();
    return _function;
}

bool is_object_type(Value value, ObjType type)
{
    return IS_OBJECT(value) && AS_OBJECT(value)->type() == type;
}

String * allocate_string(char * chars, int length)
{
    String * _string = ALLOCATE_OBJECT(String, O_STRING);
    _string->length() = length;
    _string->chars() = chars;

    vm.strings.insert(std::make_pair(_string, OBJECT_VAL(NULL)));

    return _string;
}

void String::free()
{
    FREE_ARRAY(char, _chars);
}

void Object::free()
{
    switch (_type)
    {
        case O_STRING:
        {
            String * _string = (String *)this;
            _string->free();
            FREE(String, this);
            break;
        }

        case O_FUNCTION:
        {
            Function * _function = (Function *)this;
            _function->chunk().free();
            FREE(Function, this);
            break;
        }
    }
}

String * find_key(std::unordered_map<String *, Value> * hash_table, const char * chars, int length)
{
    auto _iterator = hash_table->begin();

    while (_iterator != hash_table->end())
    {
        if (_iterator->first->length() == length && memcmp(_iterator->first->chars(), chars, length) == 0)
        {
            return _iterator->first;
        }

        _iterator++;
    }

    return NULL;
}

String * take_string(char * chars, int length)
{
    String * interned = find_key(&vm.strings, chars, length);

    if (interned != NULL)
    {
        FREE_ARRAY(char, chars);
        return interned;
    }

    return allocate_string(chars, length);
}

String * copy_string(const char * chars, int length)
{
    String * interned = find_key(&vm.strings, chars, length);

    if (interned != NULL) return interned;

    char * heap = ALLOCATE(char, length + 1);
    memcpy(heap, chars, length);
    heap[length] = '\0';

    return allocate_string(heap, length);
}

void print_function(Function * _function)
{
    if (_function->name() == NULL)
    {
        printf("<script>");
        return;
    }
    printf("<fn %s>", _function->name()->chars());
}

void print_object(Value value)
{
    switch (OBJECT_TYPE(value))
    {
        case O_STRING:
            printf("%s", AS_CSTRING(value));
            break;

        case O_FUNCTION:
            print_function(AS_FUNCTION(value));
            break;
    }
}
