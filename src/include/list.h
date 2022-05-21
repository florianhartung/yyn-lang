#pragma once

#include <stddef.h>

typedef struct LIST_STRUCT {
    size_t element_size;
    size_t size;
    void** data;
} list_t;

list_t* init_list(size_t size_per_element);

void list_append(list_t* list, void* element);

void* list_get(list_t* list, size_t index);
