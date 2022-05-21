#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "include/token.h"

token_t* init_token(int type) {
    token_t* token = malloc(sizeof(token_t));
    token->type = type;

    return token;
}


char* token_type_to_string(int type) {
    switch (type) {
        case TOKEN_HELLO_WORLD:
            return "TOKEN_HELLO_WORLD";
        case TOKEN_SEMICOLON:
            return "TOKEN_SEMICOLON";
        case TOKEN_EOF:
            return "TOKEN_EOF";
        default:
            return "undef-token-type";
    }
}

char* token_to_string(token_t* token) {
    char* template = "<TOKEN type='%s', int_type=%d>";
    char* type_str = token_type_to_string(token->type);

    char* str = calloc(strlen(template) + strlen(type_str) + 8, sizeof(char));
    sprintf(str, template, type_str, token->type);

    return str;
}