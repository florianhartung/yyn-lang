#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "include/token.h"

token_t* init_token(int type) {
    token_t* token = malloc(sizeof(token_t));
    token->type = type;

    token->token_data = malloc(sizeof(token_data_t));
    return token;
}


const char* token_type_to_string(int type) {
    switch (type) {
        case TOKEN_IDENTIFIER:
            return "TOKEN_IDENTIFIER";
        case TOKEN_NEWLINE:
            return "TOKEN_NEWLINE";
        case TOKEN_EOF:
            return "TOKEN_EOF";
        case TOKEN_LPAREN:
            return "TOKEN_LPAREN";
        case TOKEN_RPAREN:
            return "TOKEN_RPAREN";
        case TOKEN_LBRACE:
            return "TOKEN_LBRACE";
        case TOKEN_RBRACE:
            return "TOKEN_RBRACE";
        default:
            return "undef-token-type";
    }
}

const char* token_data_to_string(token_t* token) {
    switch (token->type) {
        case TOKEN_IDENTIFIER:
            return token->token_data->identifier_data;
        default:
            return "";
    }
}

char* token_to_string(token_t* token) {
    const char* template = "<TOKEN type='%s', int_type=%d, data='%s'>";
    const char* type_str = token_type_to_string(token->type);
    const char* data_str = token_data_to_string(token);

    char* str = calloc(strlen(template) + strlen(type_str) + strlen(data_str) + 8, sizeof(char));
    sprintf(str, template, type_str, token->type, data_str);

    return str;
}

void token_free(token_t* token) {
    free(token->token_data);
    free(token);
}