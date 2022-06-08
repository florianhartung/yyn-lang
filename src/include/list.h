#pragma once

#include <stddef.h>

#define LIST_INDEX(line) list_index##line
#define LIST_FOREACH(list, element_declaration, action) LIST_FOREACH_INNER(list, element_declaration, action, __COUNTER__)
#define LIST_FOREACH_INNER(list, element_declaration, action, index_suffix) \
    for(int LIST_INDEX(index_suffix) = 0; LIST_INDEX(index_suffix) < list->size; ++LIST_INDEX(index_suffix)){ \
        element_declaration = list_get(list, LIST_INDEX(index_suffix)); \
        action \
    } \


typedef void (* free_function_t)(void*);

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