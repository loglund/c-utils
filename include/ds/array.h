/*
Simple dynamic array implementation in C. 

Provides basic operations like initialization, push, pop, remove, and length retrieval. 
The array automatically resizes when capacity is exceeded. Bounds checking is included for safety. 

The caller is responsible for managing the memory of the elements stored in the array.

*/

#ifndef DS_ARRAY_H
#define DS_ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#endif

#define DS_ARRAY_INITIAL_CAPACITY 256

typedef struct {
    size_t size;
    size_t capacity;
} DynArrHeader;

#define array_bounds_check(arr, index) do { \
    DynArrHeader* header = (DynArrHeader*)(arr) - 1; \
    if ((size_t)(index) >= header->size) { \
        fprintf(stderr, "dynarr: index %zu out of bounds (size=%zu) at %s:%d\n", \
                (size_t)(index), header->size, __FILE__, __LINE__); \
        abort(); \
    } \
} while (0)

static inline void* array_init_(size_t elem_size) {
    DynArrHeader* header = (DynArrHeader*)malloc(DS_ARRAY_INITIAL_CAPACITY * elem_size + sizeof(DynArrHeader));
    if (!header) return NULL;
    header->size = 0;
    header->capacity = DS_ARRAY_INITIAL_CAPACITY;
    return header + 1;
}

#define array_init(elem_type) ((elem_type*)array_init_(sizeof(elem_type)))

#define array_push(arr, value) { \
    DynArrHeader* header = (DynArrHeader*)(arr) - 1; /* Get the header */ \
    if (header->size >= header->capacity) { \
        /* Need to resize */ \
        size_t new_capacity = header->capacity * 2; \
        DynArrHeader* new_header = (DynArrHeader*)realloc(header, new_capacity * sizeof(typeof(*arr)) + sizeof(DynArrHeader)); \
        new_header->capacity = new_capacity; \
        header = new_header;  \
        arr = (typeof(*arr)*)(header + 1); \
    } \
    (arr)[header->size++] = (value); \
}

#define array_free(arr) { \
    if (arr) { \
        free((DynArrHeader*)(arr) - 1); \
    } \
}

#define array_pop(arr) \
    (((DynArrHeader*)(arr) - 1)->size == 0 \
        ? (fprintf(stderr, "dynarr: pop from empty array at %s:%d\n", __FILE__, __LINE__), abort(), (arr)[0]) \
        : (arr)[--(((DynArrHeader*)(arr) - 1)->size)])

#define array_remove(arr, index) { \
    DynArrHeader* header = (DynArrHeader*)(arr) - 1; \
    array_bounds_check((arr), (index)); \
    for (size_t i = (index); i < header->size - 1; i++) { \
        (arr)[i] = (arr)[i + 1]; \
    } \
    header->size--; \
}

#define array_len(arr) (((DynArrHeader*)(arr) - 1)->size)

