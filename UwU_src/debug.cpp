#include "chunk.h"
#include "value.h"

void Chunk::disassemble(const char * name)
{
    printf("===== %s =====\n", name);

    for (int offset = 0; offset < _count;)
    {
        offset = disassemble_instruction(offset);
    }

    _lines.lindex = 0;
}

static int constant_instruction(const char * name, Chunk * chunk, int offset)
{
    uint8_t constant = chunk->ccode()[offset + 1];
    printf("%-16s %4d '", name, constant);
    Value value = chunk->cconstants().vvalues()[constant];
    if (IS_CHAR(value))
    {
        switch (value.as.character)
        {
            case '\n': printf("~n");     break;
            case '\t': printf("~t");     break;
            default: print_value(value); break;
        }
    }
    else
    {
        print_value(value);
    }
    printf("'\n");

    return offset + 2;
}

static int simple_instruction(const char * name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}

static int byte_instruction(const char * name, Chunk * chunk, int offset)
{
    uint8_t slot = chunk->ccode()[offset + 1];
    printf("%-16s %4d\n", name, slot);
    return offset + 2;
}

static int jump_instruction(const char * name, int sign, Chunk * chunk, int offset)
{
    uint16_t jump = (uint16_t)(chunk->ccode()[offset + 1] << 8);
    jump |= chunk->ccode()[offset + 2];
    printf("%-16s %4d -> %d\n", name, offset, (offset + 3) + sign * jump);
    return offset + 3;
}

int Chunk::disassemble_instruction(int offset)
{
    printf("%04d ", offset);

    if (offset > 0 && offset < _lines.lines[_lines.lindex - 1][1])
    {
        printf("   | ");
    }
    else
    {
        printf("%4d ", _lines.lines[_lines.lindex++][0]);
    }

    uint8_t instruction = _code[offset];
    switch (instruction)
    {
        case OP_CONSTANT:
            return constant_instruction("OP_CONSTANT", this, offset);

        case OP_TRUE:
            return simple_instruction("OP_TRUE", offset);
        case OP_FALSE:
            return simple_instruction("OP_FALSE", offset);

        case OP_POP:
            return simple_instruction("OP_POP", offset);

        case OP_GET_GLOBAL:
            return constant_instruction("OP_GET_GLOBAL", this, offset);
        case OP_SET_GLOBAL:
            return constant_instruction("OP_SET_GLOBAL", this, offset);
        case OP_GET_LOCAL:
            return byte_instruction("OP_GET_LOCAL", this, offset);
        case OP_SET_LOCAL:
            return byte_instruction("OP_SET_LOCAL", this, offset);
        case OP_DEFINE_GLOBAL:
            return constant_instruction("OP_DEFINE_GLOBAL", this, offset);

        case OP_EQUAL:
            return simple_instruction("OP_EQUAL", offset);
        case OP_NOT_EQUAL:
            return simple_instruction("OP_NOT_EQUAL", offset);
        case OP_GREATER:
            return simple_instruction("OP_GREATER", offset);
        case OP_GREATER_EQUAL:
            return simple_instruction("OP_GREATER_EQUAL", offset);
        case OP_LESS:
            return simple_instruction("OP_LESS", offset);
        case OP_LESS_EQUAL:
            return simple_instruction("OP_LESS_EQUAL", offset);

        case OP_ADD:
            return simple_instruction("OP_ADD", offset);
        case OP_SUBTRACT:
            return simple_instruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:
            return simple_instruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simple_instruction("OP_DIVIDE", offset);

        case OP_NOT:
            return simple_instruction("OP_NOT", offset);

        case OP_NEGATE:
            return simple_instruction("OP_NEGATE", offset);

        case OP_PRINT:
            return simple_instruction("OP_PRINT", offset);

        case OP_JUMP:
            return jump_instruction("OP_JUMP", 1, this, offset);
        case OP_JUMP_IF_TRUE:
            return jump_instruction("OP_JUMP_IF_TRUE", 1, this, offset);
        case OP_JUMP_IF_FALSE:
            return jump_instruction("OP_JUMP_IF_FALSE", 1, this, offset);
        case OP_LOOP:
            return jump_instruction("OP_LOOP", -1, this, offset);

        case OP_NULL:
            return simple_instruction("OP_NULL", offset);

        case OP_CALL:
            return byte_instruction("OP_CALL", this, offset);

        case OP_OUT:
            return simple_instruction("OP_OUT", offset);

        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
