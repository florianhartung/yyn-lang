#pragma once

#include <stddef.h>

typedef void (*free_function_t)(void*);

typedef struct LIST_STRUCT {
    size_t element_size;
    size_t size;
    void** data;

    free_function_t element_free_function;
} list_t;

list_t* init_list(size_t size_per_element);

void list_append(list_t* list, void* element);

void* list_get(list_t* list, size_t index);

void free_list(list_t* list);