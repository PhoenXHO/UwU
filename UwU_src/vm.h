#ifndef VM_H_INCLUDED
#define VM_H_INCLUDED

#include <unordered_map>

#include "chunk.h"
#include "object.h"

#define STACK_MAX 256

typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

typedef struct
{
    Chunk * chunk;
    uint8_t * ip;
    Value _stack[STACK_MAX];
    Value * stack_top;

    std::unordered_map<String *, Value> strings;
    std::unordered_map<String *, Value> globals;
    Object * objects;
} VM;

void initVM();
void freeVM();

InterpretResult interpret(const char *);

void push(Value);
Value pop();

#endif // VM_H_INCLUDED
