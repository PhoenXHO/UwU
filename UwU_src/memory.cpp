#include "memory.h"
#include "vm.h"

extern VM vm;

void * reallocate(void * pointer, size_t new_size)
{
    if (new_size == 0)
    {
        free(pointer);
        return NULL;
    }

    void * result = realloc(pointer, new_size);
    if (result == NULL) exit(0);

    return result;
}

void free_objects()
{
    Object * object = vm.objects;
    while (object != NULL)
    {
        Object * next = object->next();
        object->free();
        object = next;
    }
}
