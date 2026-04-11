#ifndef DYNARR_H
#define DYNARR_H

#include <stdio.h>
#include <stdlib.h>

#define DYNARR_INITIAL_CAPACITY 256

typedef struct {
    size_t size;
    size_t capacity;
} DynArrHeader;

#define arr_bounds_check(arr, index) do { \
    DynArrHeader* header = (DynArrHeader*)(arr) - 1; \
    if ((size_t)(index) >= header->size) { \
        fprintf(stderr, "dynarr: index %zu out of bounds (size=%zu) at %s:%d\n", \
                (size_t)(index), header->size, __FILE__, __LINE__); \
        abort(); \
    } \
} while (0)

#define arr_init(elem_type) ({ \
    DynArrHeader* header = (DynArrHeader*)malloc((DYNARR_INITIAL_CAPACITY) * sizeof(elem_type) + sizeof(DynArrHeader)); \
    header->size = 0; \
    header->capacity = DYNARR_INITIAL_CAPACITY; \
    (elem_type*)(header + 1); \
})

#define arr_push(arr, value) { \
    DynArrHeader* header = (DynArrHeader*)(arr) - 1; /* Get the header */ \
    if (header->size >= header->capacity) { \
        /* Need to resize */ \
        size_t new_capacity = header->capacity * 2; \
        DynArrHeader* new_header = (DynArrHeader*)realloc(header, new_capacity * sizeof(__typeof__(*(arr))) + sizeof(DynArrHeader)); \
        new_header->capacity = new_capacity; \
        header = new_header;  \
        arr = (__typeof__(arr))(header + 1); \
    } \
    (arr)[header->size++] = (value); \
}

#define arr_free(arr) { \
    if (arr) { \
        free((DynArrHeader*)(arr) - 1); \
    } \
}

#define arr_pop(arr) ({ \
    DynArrHeader* header = (DynArrHeader*)(arr) - 1; \
    if (header->size == 0) { \
        fprintf(stderr, "dynarr: pop from empty array at %s:%d\n", __FILE__, __LINE__); \
        abort(); \
    } \
    header->size--; \
    (arr)[header->size]; \
})

#define arr_remove(arr, index) { \
    DynArrHeader* header = (DynArrHeader*)(arr) - 1; \
    arr_bounds_check((arr), (index)); \
    for (size_t i = (index); i < header->size - 1; i++) { \
        (arr)[i] = (arr)[i + 1]; \
    } \
    header->size--; \
}

#define arr_len(arr) ({ \
    ((DynArrHeader*)(arr) - 1)->size; \
})

#endif