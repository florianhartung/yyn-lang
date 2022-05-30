#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "include/token.h"
#include "include/lexer.h"
#include "include/util.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(void*))

static lexer_state_t* init_lexer(char* src) {
    lexer_state_t* lexer_state = malloc(sizeof(lexer_state_t));
    lexer_state->current_index = 0;
    lexer_state->src = src;
    lexer_state->current_character = src[0];

    return lexer_state;
}

static void advance(lexer_state_t* lexer) {
    lexer->current_index += 1;
    lexer->current_character = lexer->src[lexer->current_index];
}

static token_t* parse_identifier_keyword(lexer_state_t* lexer) {
    int identifier_length = 0;
    while (is_alpha(lexer->current_character)) {
        identifier_length += 1;
        advance(lexer);
    }

    char* identifier_name = calloc(identifier_length + 1, sizeof(char));
    strncpy(identifier_name, lexer->src + lexer->current_index - identifier_length, identifier_length);

    // Check if identifier is a valid keyword
    for (int i = 0; i < ARRAY_LEN(keywords); ++i) {
        const char* keyword = keywords[i];
        if (strcmp(keyword, identifier_name) == 0) {
            token_t* token = init_token(TOKEN_KEYWORD);
            token->token_data->keyword = i;
            return token;
        }
    }

    token_t* token = init_token(TOKEN_IDENTIFIER);
    token->token_data->identifier_name = identifier_name;

    return token;
}

static token_t* eat_token(lexer_state_t* lexer, int token_type) {
    advance(lexer);
    return init_token(token_type);
}

static token_t* parse_next_token(lexer_state_t* lexer) { // NOLINT(misc-no-recursion)
    switch (lexer->current_character) {
        case 0:
            return eat_token(lexer, TOKEN_EOF);
        case '(':
            return eat_token(lexer, TOKEN_LPAREN);
        case ')':
            return eat_token(lexer, TOKEN_RPAREN);
        case '{':
            return eat_token(lexer, TOKEN_LBRACE);
        case '}':
            return eat_token(lexer, TOKEN_RBRACE);
        case '\n':
            return eat_token(lexer, TOKEN_NEWLINE);
        case ' ':
        case '\t': {
            advance(lexer);
            return parse_next_token(lexer);
        }
    }

    if (is_alpha(lexer->current_character)) {
        return parse_identifier_keyword(lexer);
    }

    fprintf(stderr, "Unknown token '%c' at %i", lexer->current_character, lexer->current_index);
    exit(1);
}

list_t* lexer_read_tokens(char* src) {
    lexer_state_t* lexer = init_lexer(src);

    list_t* tokens = init_list(sizeof(token_t*));
    tokens->element_free_function = (free_function_t) token_free;

    token_t* token;
    do {
        token = parse_next_token(lexer);
        list_append(tokens, token);
    } while (token->type != TOKEN_EOF);

    free(lexer);

    return tokens;
}