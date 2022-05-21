#include <malloc.h>
#include <stdio.h>
#include "include/token.h"
#include "include/lexer.h"

lexer_state_t* init_lexer(char* src) {
    lexer_state_t* lexer_state = malloc(sizeof(lexer_state_t));
    lexer_state->current_index = 0;
    lexer_state->src = src;
    lexer_state->current_character = src[0];

    return lexer_state;
}

void advance(lexer_state_t* lexer) {
    lexer->current_index += 1;
    lexer->current_character = lexer->src[lexer->current_index];
}

token_t* accept_token(lexer_state_t* lexer, int token_type) {
    advance(lexer);
    return init_token(token_type);
}

void expect_character(lexer_state_t* lexer, char expected_character) {
    advance(lexer);
    if (lexer->current_character != expected_character) {
        fprintf(stderr, "Expected character '%s' token. Found '%s' instead.", expected_character,
                lexer->current_character);
        exit(1);
    }
}

token_t* advance_token(lexer_state_t* lexer) {
    if (lexer->current_character == 0) {
        return accept_token(lexer, TOKEN_EOF);
    }
    if (lexer->current_character == 'h') {
        expect_character(lexer, 'w');
        return accept_token(lexer, TOKEN_HELLO_WORLD);
    }
    if (lexer->current_character == ';') {
        return accept_token(lexer, TOKEN_SEMICOLON);
    }
    fprintf(stderr, "Unknown token %s at %i", lexer->current_character, lexer->current_index);
    exit(1);
}

list_t* lexer_read_tokens(char* src) {
    lexer_state_t* lexer_state = init_lexer(src);
    list_t* tokens = init_list(sizeof(token_t*));

    token_t* token;
    do {
        token = advance_token(lexer_state);
        list_append(tokens, token);
    } while (token->type != TOKEN_EOF);
    return tokens;
}
