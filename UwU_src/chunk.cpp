#include "chunk.h"
#include "memory.h"

void Chunk::write(uint8_t byte, int line)
{
    if (_capacity < _count + 1)
    {
        int old_capacity = _capacity;

        _capacity = GROW_CAPACITY(old_capacity);
        _code = GROW_ARRAY(uint8_t, _code, _capacity);
    }

    if (_lines.lcapacity < _lines.lcount + 2)
    {
        int old_capacity = _lines.lcapacity;

        _lines.lcapacity = GROW_CAPACITY(old_capacity);
        _lines.lines = GROW_ARRAY(int *, _lines.lines, _lines.lcapacity);

        for (int i = old_capacity; i < _lines.lcapacity; i++)
        {
            _lines.lines[i] = GROW_ARRAY(int, NULL, 2);
            _lines.lines[i][0] = 1;
            _lines.lines[i][1] = 0;
        }
    }

    if (line == _lines.lines[_lines.lcount][0])
    {
        _lines.lines[_lines.lcount][1]++;
    }
    else
    {
        _lines.lines[++_lines.lcount][0] = line;
        _lines.lines[_lines.lcount][1] = _lines.lines[_lines.lcount - 1][1] + 1;
    }

    _code[_count] = byte;
    _count++;
}

void Chunk::init()
{
    _count = _capacity = 0;
    _lines.lcount = _lines.lcapacity = _lines.lindex = 0;
    _code = NULL;
    _lines.lines = {NULL};
    _constants.init();
}

void Chunk::free()
{
    FREE_ARRAY(uint8_t, _code);
    for (int i = 0; i < _lines.lcapacity; i++)
    {
        FREE_ARRAY(int, _lines.lines[i]);
    }
    FREE_ARRAY(int *, _lines.lines);
    _constants.free();
    init();
}

int Chunk::add_constant(Value value)
{
    _constants.write(value);
    return _constants.vcount() - 1;
}
