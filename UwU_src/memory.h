#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include "common.h"
#include "object.h"

#define ALLOCATE(type, count) \
    (type *)reallocate(NULL, sizeof(type) * (count))

#define FREE(type, pointer) \
    reallocate(pointer, 0)

#define GROW_CAPACITY(capacity) \
    (capacity) < 8 ? 8 : 2 * (capacity)

#define GROW_ARRAY(type, pointer, new_capacity) \
    (type *)reallocate(pointer, sizeof(type) * (new_capacity))

#define FREE_ARRAY(type, pointer) \
    reallocate(pointer, 0)

void * reallocate(void * pointer, size_t new_size);
void free_objects();

#endif // MEMORY_H_INCLUDED
