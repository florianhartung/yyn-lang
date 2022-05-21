#pragma once

#include "token.h"
#include "list.h"

typedef struct LEXER_STRUCT {
    char* src;
    size_t current_index;
    char current_character;
} lexer_state_t;

list_t* lexer_read_tokens(char* src);