#pragma once

#include <stddef.h>

typedef struct TOKEN_POSITION_STRUCT {
    int from;
    int to;
} token_position_t;

typedef enum {
    KEYWORD_FUNCTION,
    KEYWORD_HELLO
} keyword_t;

static const char* keywords[] = {
        [KEYWORD_FUNCTION] = "fun",
        [KEYWORD_HELLO] = "hello"
};

typedef union {
    char* identifier_name;
    keyword_t keyword;
} token_data_t;

typedef struct TOKEN_STRUCT {
    enum {
        TOKEN_IDENTIFIER,
        TOKEN_KEYWORD,
        TOKEN_NEWLINE,
        TOKEN_LPAREN,
        TOKEN_RPAREN,
        TOKEN_LBRACE,
        TOKEN_RBRACE,
        TOKEN_EOF
    } type;

    token_data_t* token_data;

    // token_position_t* token_position;
} token_t;

token_t* init_token(int type);

const char* token_type_to_string(int type);

char* token_to_string(token_t* token);

void token_free(token_t* token);
