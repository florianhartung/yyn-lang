#pragma once

#include <stddef.h>

typedef struct TOKEN_POSITION_STRUCT {
    int from;
    int to;
} token_position_t;

typedef enum {
    KEYWORD_FUNCTION,
    KEYWORD_HELLO,
    KEYWORD_EXIT,
    KEYWORD_PRINT_CHAR,
    KEYWORD_INT
} keyword_t;

static const char* keywords[] = {
        [KEYWORD_FUNCTION] = "fun",
        [KEYWORD_HELLO] = "hello",
        [KEYWORD_EXIT] = "exit",
        [KEYWORD_PRINT_CHAR] = "printchar",
        [KEYWORD_INT] = "int"
};

typedef union {
    char* identifier_name;
    keyword_t keyword;
    int int_value;
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
        TOKEN_EOF,
        TOKEN_INTEGER,
        TOKEN_EQUALS,
        TOKEN_ASSIGNMENT_ADD
    } type;

    token_data_t* token_data;

    // token_position_t* token_position;
} token_t;

static char* token_types[] = {
        [TOKEN_IDENTIFIER] = "TOKEN_IDENTIFIER",
        [TOKEN_KEYWORD] = "TOKEN_KEYWORD",
        [TOKEN_NEWLINE] = "TOKEN_NEWLINE",
        [TOKEN_LPAREN] = "TOKEN_LPAREN",
        [TOKEN_RPAREN] = "TOKEN_RPAREN",
        [TOKEN_LBRACE] = "TOKEN_LBRACE",
        [TOKEN_RBRACE] = "TOKEN_RBRACE",
        [TOKEN_EOF] = "TOKEN_EOF",
        [TOKEN_INTEGER] = "TOKEN_INTEGER",
        [TOKEN_EQUALS] = "TOKEN_EQUALS",
        [TOKEN_ASSIGNMENT_ADD] = "TOKEN_ASSIGNMENT_ADD"
};

token_t* init_token(int type);

const char* token_type_to_string(int type);

char* token_to_string(token_t* token);

void token_free(token_t* token);
