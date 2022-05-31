#include "include/string_builder.h"
#include <malloc.h>
#include <stdio.h>

string_builder_t* init_string_builder() {
    string_builder_t* string_builder = calloc(1, sizeof(string_builder_t));
    string_builder->length = 0;
    string_builder->data = calloc(string_builder->length, sizeof(char));

    return string_builder;
}

void string_builder_append(string_builder_t* string_builder, char* string) {
    string_builder->length += strlen(string);
    string_builder->data = realloc(string_builder->data, string_builder->length + 1 );
    strcat(string_builder->data, string);
}


char* string_builder_build(string_builder_t* string_builder) {
    char* data = string_builder->data;
    free(string_builder);
    return data;
}
