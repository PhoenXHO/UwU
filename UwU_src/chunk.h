#ifndef CHUNK_H_INCLUDED
#define CHUNK_H_INCLUDED

#include "common.h"
#include "value.h"

typedef enum
{
    OP_CONSTANT,

    OP_TRUE,
    OP_FALSE,

    OP_POP,

    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_DEFINE_GLOBAL,

    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,

    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,

    OP_NOT,

    OP_NEGATE,

    OP_PRINT,
    OP_READ_STRING,
    OP_READ_NUMBER,
    OP_READ_CHAR,
    OP_NEW_LINE,

    OP_JUMP,
    OP_JUMP_IF_TRUE,
    OP_JUMP_IF_FALSE,
    OP_LOOP,

    OP_NULL,

    OP_RETURN,
} OpCode;

typedef struct
{
    int lcount;
    int lcapacity;
    int lindex;
    int ** lines;
} Lines;

class Chunk
{
    private:
        int _count;
        int _capacity;
        uint8_t * _code;
        Lines _lines;
        ValueArray _constants;

    public:
        int ccount()            { return _count;     }
        int ccapacity()         { return _capacity;  }
        uint8_t * ccode()       { return _code;      }
        Lines clines()          { return _lines;     }
        ValueArray cconstants() { return _constants; }

        Chunk() : _count(0), _capacity(0), _code(NULL)
        {
            _lines.lcapacity = _lines.lcount = _lines.lindex = 0;
            _lines.lines = {NULL};
        }

        void init();
        void write(uint8_t, int);
        void free();
        int add_constant(Value);

        void disassemble(const char *);
        int disassemble_instruction(int);
};

#endif // CHUNK_H_INCLUDED
