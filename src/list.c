#include <malloc.h>
#include "include/list.h"

list_t* init_list(size_t size_per_element) {
    list_t* list = malloc(sizeof(list_t));
    list->size = 0;
    list->element_size = size_per_element;

    return list;
}

void list_append(list_t* list, void* element) {
    list->size += 1;
    if(list->size == 1) {
        list->data = calloc(1, list->element_size);
    } else {
        list->data = realloc(list->data, list->size * list->element_size);
    }
    list->data[list->size - 1ULL] = element;
}

void* list_get(list_t* list, size_t index) {
    return list->data[index];
}

void list_free(list_t* list) {
    for (int i = 0; i < list->size; ++i) {
        list->element_free_function(list_get(list, i));
    }
    free(list);
}