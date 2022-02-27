#include <stdarg.h>

#include "vm.h"
#include "compiler.h"
#include "memory.h"

extern int DEBUG_TRACE_EXECUTION;

VM vm;

void reset_frame()
{
    vm.frame_count = 0;
}

void reset_stack()
{
    vm.stack_top = vm._stack;
    reset_frame();
}

void runtime__error(const char * format, ...)
{
    fprintf(stderr, "ewwow: ");

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm.frame_count - 1; i >= 0; i--)
    {
        CallFrame * frame = &vm.frames[i];
        Function * _function = frame->_function;
        Chunk chunk = _function->chunk();

        int current_line;
        int ip_index = (int)(frame->ip - chunk.ccode());
        for (int i = 0; i <= chunk.clines().lcount; i++)
        {
            if (ip_index < chunk.clines().lines[i][1])
            {
                current_line = i;
                break;
            }
        }
        fprintf(stderr, "[line %d] in ", chunk.clines().lines[current_line][0]);

        if (!(_function->name())) fprintf(stderr, "scwipt\n");
        else fprintf(stderr, "<%s>\n", _function->name()->chars());
    }

    reset_stack();
}

void initVM()
{
    reset_stack();
    vm.objects = NULL;
}

void freeVM()
{
    vm.strings.clear();
    vm.globals.clear();
    free_objects();
}

void push(Value value)
{
    *vm.stack_top = value;
    vm.stack_top++;
}

Value pop()
{
    vm.stack_top--;
    return *vm.stack_top;
}

Value peek(int distance)
{
    return vm.stack_top[-1 - distance];
}

static bool call(Function * _function, int arg_count)
{
    if (arg_count != _function->arity())
    {
        runtime__error("expected %d awguments but got %d.", _function->arity(), arg_count);
        return false;
    }

    if (vm.frame_count == FRAMES_MAX)
    {
        runtime__error("stack ovewfwow.");
        return false;
    }

    CallFrame * frame = &vm.frames[vm.frame_count++];
    frame->_function = _function;
    frame->ip = _function->chunk().ccode();
    frame->slots = vm.stack_top - arg_count - 1;

    return true;
}

static bool call_value(Value callee, int arg_count)
{
    if (IS_OBJECT(callee))
    {
        switch (OBJECT_TYPE(callee))
        {
            case O_FUNCTION:
                return call(AS_FUNCTION(callee), arg_count);

            default: break;
        }
    }

    runtime__error("can onwy caww fwunctions.");
    return false;
}

bool is_falsy(Value value)
{
    return (IS_BOOL(value) && !AS_BOOL(value)) || AS_NUMBER(value) == 0;
}

static String * char_to_string(Value value)
{
    if (IS_CHAR(value))
    {
        String * _string = ALLOCATE_OBJECT(String, O_STRING);
        char * chars = ALLOCATE(char, 2);
        chars[0] = value.as.character;
        chars[1] = '\0';

        _string->chars() = chars;
        _string->length() = 1;
        return _string;
    }

    return AS_STRING(value);
}

static void concatenate()
{
    String * b = char_to_string(pop());
    String * a = char_to_string(pop());

    int length = a->length() + b->length();
    char * chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars(), a->length());
    memcpy(chars + a->length(), b->chars(), b->length());
    chars[length] = '\0';

    String * result = take_string(chars, length);
    push(OBJECT_VAL(result));
}

static String * read_string()
{
    int length = -1;
    char * _string, c;

    _string = (char *)malloc(sizeof(char));

    while (scanf("%c", &c) == 1)
    {
        _string = GROW_ARRAY(char, _string, (++length) + 1);

        if (c == '\n') break;

        _string[length] = c;
    }

    fflush(stdin);
    _string[length] = '\0';

    return take_string(_string, length);
}

static double read_number()
{
    int length = -1;
    char * _string, c;

    _string = (char *)malloc(sizeof(char));

    while (scanf("%c", &c) == 1)
    {
        _string = GROW_ARRAY(char, _string, (++length) + 1);

        if (!isdigit(c) && c != '.' && c != '-')
        {
            if (c == '\n') break;
            return 0;
        }

        _string[length] = c;
    }

    fflush(stdin);
    _string[length + 1] = '\0';

    return strtod(_string, NULL);
}

static char read_char()
{
    char c;
    scanf("%c", &c);
    fflush(stdin);

    return c;
}

static InterpretResult run()
{
    CallFrame * frame = &vm.frames[vm.frame_count - 1];

    #define READ_BYTE()     (*frame->ip++)
    #define READ_SHORT()    (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
    #define READ_CONSTANT() (frame->_function->chunk().cconstants().vvalues()[READ_BYTE()])
    #define READ_STRING()   (AS_STRING(READ_CONSTANT()))
    #define BINARY_OP(value_type, op) \
        do \
        { \
            if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) \
            { \
                runtime__error("Operands must be numbers."); \
                return INTERPRET_RUNTIME_ERROR; \
            } \
            double b = AS_NUMBER(pop()); \
            double a = AS_NUMBER(pop()); \
            push(value_type(a op b)); \
        } while (false)

    while (true)
    {
        if (DEBUG_TRACE_EXECUTION)
        {
            printf("          ");
            for (Value * slot = vm._stack; slot < vm.stack_top; slot++)
            {
                printf("[");
                print_value(*slot);
                printf("]");
            }
            printf("\n");
            frame->_function->chunk().disassemble_instruction((int)(frame->ip - frame->_function->chunk().ccode()));
        }

        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
            case OP_CONSTANT:
            {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }

            case OP_TRUE:  push(BOOL_VAL(true));  break;
            case OP_FALSE: push(BOOL_VAL(false)); break;

            case OP_POP: pop(); break;

            case OP_GET_LOCAL:
            {
                uint8_t slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }

            case OP_SET_LOCAL:
            {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                break;
            }

            case OP_GET_GLOBAL:
            {
                String * name = READ_STRING();
                auto _iterator = vm.globals.find(name);
                if (_iterator == vm.globals.end())
                {
                    runtime__error("unexpected towken '%s'.", name->chars());
                    return INTERPRET_RUNTIME_ERROR;
                }
                Value value = _iterator->second;
                push(value);
                break;
            }

            case OP_SET_GLOBAL:
            {
                String * name = READ_STRING();
                auto _iterator = vm.globals.find(name);
                if (_iterator == vm.globals.end())
                {
                    runtime__error("unexpected towken '%s'.", name->chars());
                    return INTERPRET_RUNTIME_ERROR;
                }
                _iterator->second = peek(0);
                break;
            }

            case OP_DEFINE_GLOBAL:
            {
                String * name = READ_STRING();
                vm.globals.insert(std::make_pair(name, peek(0)));
                pop();
                break;
            }

            case OP_EQUAL:
            {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(values_equal(a, b)));
                break;
            }

            case OP_GREATER:       BINARY_OP(BOOL_VAL, >);   break;
            case OP_GREATER_EQUAL: BINARY_OP(BOOL_VAL, >=);  break;
            case OP_LESS:          BINARY_OP(BOOL_VAL, <);   break;
            case OP_LESS_EQUAL:    BINARY_OP(BOOL_VAL, <=);  break;

            case OP_ADD:
            {
                if ((IS_STRING(peek(0)) || IS_CHAR(peek(0))) && (IS_STRING(peek(1)) || IS_CHAR(peek(1))))
                {
                    concatenate();
                }
                else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)))
                {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                }
                else
                {
                    runtime__error("opewands m-must b-be of same twype.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;

            case OP_NOT:
                push(BOOL_VAL(is_falsy(pop())));
                break;

            case OP_NEGATE:
            {
                if (!IS_NUMBER(peek(0)))
                {
                    runtime__error("operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            }

            case OP_PRINT:
            {
                print_value(pop());
                break;
            }

            case OP_READ_STRING:
            {
                String * _string = read_string();
                push(OBJECT_VAL(_string));
                break;
            }

            case OP_READ_NUMBER:
            {
                Value _number = NUMBER_VAL(read_number());
                push(_number);
                break;
            }

            case OP_READ_CHAR:
            {
                Value _char = CHAR_VAL(read_char());
                push(_char);
                break;
            }

            case OP_JUMP:
            {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }

            case OP_JUMP_IF_TRUE:
            {
                uint16_t offset = READ_SHORT();
                if (!is_falsy(peek(0))) frame->ip += offset;
                break;
            }

            case OP_JUMP_IF_FALSE:
            {
                uint16_t offset = READ_SHORT();
                if (is_falsy(peek(0))) frame->ip += offset;
                break;
            }

            case OP_LOOP:
            {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }

            case OP_NULL: push(NULL_VAL); break;

            case OP_CALL:
            {
                int arg_count = READ_BYTE();
                if (!call_value(peek(arg_count), arg_count))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frame_count - 1];
                break;
            }

            case OP_OUT:
            {
                Value result = pop();
                vm.frame_count--;
                if (vm.frame_count == 0)
                {
                    pop();
                    return INTERPRET_OK;
                }

                vm.stack_top = frame->slots;
                push(result);

                frame = &vm.frames[vm.frame_count - 1];
                break;
            }
        }
    }

    #undef READ_BYTE
    #undef READ_SHORT
    #undef READ_CONSTANT
    #undef READ_STRING
    #undef BINARY_OP
}

InterpretResult interpret(const char * source)
{
    Function * _function = compile(source);
    if (!_function) return INTERPRET_COMPILE_ERROR;

    push(OBJECT_VAL(_function));
    call_value(OBJECT_VAL(_function), 0);

    return run();
}
