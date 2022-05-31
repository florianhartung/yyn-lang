#pragma once

#include <stdbool.h>

static inline bool is_alpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static inline bool is_whitespace(char c) {
   return c == ' ' || c == '\t' || c == '\r';
}

static inline char* read_file(char* filename) {
    char* contents = 0;
    size_t length;

    FILE* file = fopen(filename, "rb");

    if(file) {
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fseek(file, 0, SEEK_SET);
        contents = calloc(length + 1, sizeof(char));
        fread(contents, sizeof(char), length, file);
        return contents;
    }
    fprintf(stderr, "Could not read file  %s", filename);
    exit(1);
}