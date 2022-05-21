#include <stdio.h>
#include <malloc.h>
#include "include/lexer.h"

int main() {
    list_t* tokens = lexer_read_tokens("hw;\0");

    for (int i = 0; i < tokens->size; ++i) {
        token_t* token = list_get(tokens, i);
        printf("%s\n", token_to_string(token));
        free(token);
    }
    free(tokens);

    return 0;
}
