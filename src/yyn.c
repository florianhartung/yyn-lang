#include <stdio.h>
#include <malloc.h>
#include "include/yyn.h"
#include "include/lexer.h"

static void debug_print_token(token_t* token) {
    char* token_string = token_to_string(token);
    printf("%s\n", token_string);
    free(token_string);
}

void yyn_compile(char* src) {
    list_t* tokens = lexer_read_tokens(src);

    for (int i = 0; i < tokens->size; ++i) {
        token_t* token = list_get(tokens, i);
        debug_print_token(token);
    }
    free_list(tokens);
}

void yyn_compile_file(char* filename) {

}
