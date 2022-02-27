#ifndef VM_H_INCLUDED
#define VM_H_INCLUDED

#include <unordered_map>

#include "chunk.h"
#include "object.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

typedef struct
{
    Function * _function;
    uint8_t * ip;
    Value * slots;
} CallFrame;

typedef struct
{
    Chunk * chunk;
    uint8_t * ip;
    Value _stack[STACK_MAX];
    Value * stack_top;

    CallFrame frames[FRAMES_MAX];
    int frame_count;

    std::unordered_map<String *, Value> strings;
    std::unordered_map<String *, Value> globals;
    Object * objects;
} VM;

void initVM();
void freeVM();

void reset_frame();

InterpretResult interpret(const char *);

void push(Value);
Value pop();

#endif // VM_H_INCLUDED
