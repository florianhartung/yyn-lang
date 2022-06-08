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
    return token_types[type];
}

const char* token_data_to_string(token_t* token) {
    switch (token->type) {
        case TOKEN_IDENTIFIER:
            return token->token_data->identifier_name;
        case TOKEN_KEYWORD:
            return keywords[token->token_data->keyword];
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