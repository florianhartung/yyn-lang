#pragma once

#include <stddef.h>

typedef struct STRING_BUILDER {
    char* data;
    size_t length;
} string_builder_t;

string_builder_t* init_string_builder();

void string_builder_append(string_builder_t* string_builder, char* c);

char* string_builder_build(string_builder_t* string_builder);