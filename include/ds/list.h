/*
Singly-linked list implementation in C. 

There are two ways to use this list:

1. The safe way: use list_push, list_get, and list_remove. These functions copy the value of the data onto the heap and manage the memory for you. The caller is responsible for freeing the data if it is a pointer type.
   -> All memory is cleaned up when list_free is called.

2. The unsafe way: use list_push_unsafe, list_get, and list_remove. These functions do not copy the value of the data onto the heap. The caller is responsible for managing the memory of the data stored in the list.
   -> List only manages the memory of the nodes, not the data. The caller must ensure that the data remains valid for the lifetime of the list and must free it if necessary.

Simple usage:

    LinkedList* list = list_init();
    int a = 10, b = 20, c = 30;
    list_push(list, a);
    list_push(list, b);
    list_push(list, c);

*/

#ifndef DS_LIST_H
#define DS_LIST_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#endif

typedef struct ListNode {
    void* data;
    struct ListNode* next;
} ListNode;

typedef struct {
    ListNode* head;
    size_t size;
} LinkedList;

LinkedList* list_init() {
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    if (!list) return NULL;
    list->head = NULL;
    list->size = 0;
    return list;
}

void list_free(LinkedList* list) {
    if (!list) return;
    ListNode* current = list->head;
    while (current) {
        ListNode* next = current->next;
        free(current);
        current = next;
    }
    free(list);
}

size_t list_len(LinkedList* list) {
    return list->size;
}


/*
Push a pointer to data onto the end of the list. The caller is responsible for managing the memory of the data.
*/
void list_push_unsafe_(LinkedList* list, void* data) {
    ListNode* new_node = (ListNode*)malloc(sizeof(ListNode));
    if (!new_node) return;
    new_node->data = data;
    new_node->next = NULL;
    if (list->size == 0) {
        list->head = new_node;
    } else {
        ListNode* current = list->head;
        while (current->next) {
            current = current->next;
        }
        current->next = new_node;
    }
    list->size++;
}

/*
Copies the value of data onto the heap and pushes a pointer to it onto the end of the list. The caller is responsible for managing the memory of the data.
*/
void list_push_(LinkedList* list, void* data, size_t elem_size) {
    void* heap_data = malloc(elem_size);
    if (!heap_data) return;
    memcpy(heap_data, data, elem_size);
    list_push_unsafe_(list, heap_data);
}

#define list_push(list, value) { \
    list_push_((list), &(value), sizeof(value)); \
}

#define list_push_unsafe(list, ptr) list_push_unsafe_((list), (ptr))    

#define list_get(list, index, elem_type) (*(elem_type*)list_get_((list), (index)))

void* list_get_(LinkedList* list, size_t index) {
    if (index >= list->size) {
        fprintf(stderr, "linkedlist: index %zu out of bounds (size=%zu) at %s:%d\n", index, list->size, __FILE__, __LINE__);
        abort();
    }
    ListNode* current = list->head;
    for (size_t i = 0; i < index; i++) {
        current = current->next;
    }
    return current->data;
}

#define list_remove(list, index, elem_type) (*(elem_type*)list_remove_((list), (index)))

void* list_remove_(LinkedList* list, size_t index) {
    if (index >= list->size) {
        fprintf(stderr, "linkedlist: index %zu out of bounds (size=%zu) at %s:%d\n", index, list->size, __FILE__, __LINE__);
        abort();
    }
    ListNode* current = list->head;
    ListNode* prev = NULL;
    for (size_t i = 0; i < index; i++) {
        prev = current;
        current = current->next;
    }
    if (prev) {
        prev->next = current->next;
    } else {
        list->head = current->next;
    }
    void* data = current->data;
    free(current);
    list->size--;
    return data;
}

