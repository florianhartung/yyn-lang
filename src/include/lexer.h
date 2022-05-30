#pragma once

#include "token.h"
#include "list.h"

typedef struct LEXER_STATE_STRUCT {
    char* src;
    size_t current_index;
    char current_character; // TODO: Convert to index pointing at the current character
} lexer_state_t;

list_t* lexer_read_tokens(char* src);