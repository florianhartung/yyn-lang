#pragma once

#include <stddef.h>

typedef struct TOKEN_STRUCT {
    enum {
        TOKEN_HELLO_WORLD,
        TOKEN_SEMICOLON,
        TOKEN_EOF
    } type;
} token_t;

token_t* init_token(int type);

char* token_type_to_string(int type);

char* token_to_string(token_t* token);
