/* 
Double-ended queue (deque) implementation in C.
A deque is a doubly-linked list that allows for efficient insertion and removal of elements from both ends.

The caller is responsible for managing the memory of the elements stored in the deque.
*/

#ifndef DS_DEQUE_H
#define DS_DEQUE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#endif

typedef struct DequeNode {
    void* data;
    struct DequeNode* next;
    struct DequeNode* prev;
} DequeNode;

typedef struct {
    DequeNode* head;
    DequeNode* tail;
    size_t size;
} Deque;

Deque* deque_init() {
    Deque* deque = (Deque*)malloc(sizeof(Deque));
    if (!deque) return NULL;
    deque->head = NULL;
    deque->tail = NULL;
    deque->size = 0;
    return deque;
}

void deque_free(Deque* deque) {
    if (!deque) return;
    DequeNode* current = deque->head;
    while (current) {
        DequeNode* next = current->next;
        free(current);
        current = next;
    }
    free(deque);
}

size_t deque_len(Deque* deque) {
    return deque->size;
}

/*
Push a pointer to data onto the end of the list. The caller is responsible for managing the memory of the data.
*/
void deque_push_back_unsafe(Deque* deque, void* data) {
    DequeNode* new_node = (DequeNode*)malloc(sizeof(DequeNode));
    if (!new_node) return;
    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = deque->tail;
    if (deque->size == 0) {
        deque->head = new_node;
        deque->tail = new_node;
    } else {
        deque->tail->next = new_node;
        deque->tail = new_node;
    }
    deque->size++;
}

void deque_push_front_unsafe(Deque* deque, void* data) {
    DequeNode* new_node = (DequeNode*)malloc(sizeof(DequeNode));
    if (!new_node) return;
    new_node->data = data;
    new_node->next = deque->head;
    new_node->prev = NULL;
    if (deque->size == 0) {
        deque->head = new_node;
        deque->tail = new_node;
    } else {
        deque->head->prev = new_node;
        deque->head = new_node;
    }
    deque->size++;
}

void deque_push_back_(Deque* deque, void* data, size_t elem_size) {
    void* heap_data = malloc(elem_size);
    if (!heap_data) return;
    memcpy(heap_data, data, elem_size);
    deque_push_back_unsafe(deque, heap_data);
}

void deque_push_front_(Deque* deque, void* data, size_t elem_size) {
    void* heap_data = malloc(elem_size);
    if (!heap_data) return;
    memcpy(heap_data, data, elem_size);
    deque_push_front_unsafe(deque, heap_data);
}

#define deque_push_back(deque, value) deque_push_back_(deque, &(value), sizeof(value))
#define deque_push_front(deque, value) deque_push_front_(deque, &(value), sizeof(value))


#define deque_get(deque, index, elem_type) (*(elem_type*)deque_get_((deque), (index)))
/*
Get a pointer to the element at the specified index. If index is out of bounds, the program will print an error message and abort.
If index is in the first half of the deque, it will traverse from the head; otherwise, it will traverse from the tail for efficiency.
*/
void* deque_get_(Deque* deque, size_t index) {
    if (index >= deque->size) {
        fprintf(stderr, "deque: index %zu out of bounds (size=%zu) at %s:%d\n", index, deque->size, __FILE__, __LINE__);
        abort();
    }

    DequeNode* current;
    if (index < (size_t)(deque->size / 2)) {
        current = deque->head;
        for (size_t i = 0; i < index; i++) {
            current = current->next;
        }
    } else {
        current = deque->tail;
        for (size_t i = deque->size - 1; i > index; i--) {
            current = current->prev;
        }
    }
    return current->data;
}


#define deque_remove(deque, index, elem_type) (*(elem_type*)deque_remove_((deque), (index)))

void* deque_remove_(Deque* deque, size_t index) {
    if (index >= deque->size) {
        fprintf(stderr, "deque: index %zu out of bounds (size=%zu) at %s:%d\n", index, deque->size, __FILE__, __LINE__);
        abort();
    }
    DequeNode* current;
    if (index < (size_t)(deque->size / 2)) {
        current = deque->head;
        for (size_t i = 0; i < index; i++) {
            current = current->next;
        }
    } else {
        current = deque->tail;
        for (size_t i = deque->size - 1; i > index; i--) {
            current = current->prev;
        }
    }
    if (current->prev) {
        DequeNode* prev = current->prev;
        prev->next = current->next;
    } else {
        deque->head = current->next;
        if (deque->head) {
            deque->head->prev = NULL;
        }
    }
    if (current->next) {
        DequeNode* next = current->next;
        next->prev = current->prev;
    } else {
        deque->tail = current->prev;
        if (deque->tail) {
            deque->tail->next = NULL;
        }
    }
    void* data = current->data;
    free(current);
    deque->size--;
    return data;
}

#define deque_pop_back(deque, elem_type) deque_remove(deque, deque_len(deque) - 1, elem_type)
#define deque_pop_front(deque, elem_type) deque_remove(deque, 0, elem_type)