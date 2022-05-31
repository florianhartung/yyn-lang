#include <stdio.h>
#include <malloc.h>
#include "include/yyn.h"
#include "include/lexer.h"
#include "include/ast.h"
#include "include/util.h"
#include "include/parser.h"


static void debug_print_tokens(list_t* tokens) {
    printf("------------- Tokens ------------\n");
    for (int i = 0; i < tokens->size; ++i) {
        token_t* token = list_get(tokens, i);
        char* token_string = token_to_string(token);
        printf("%s\n", token_string);
        free(token_string);
    }
}

static void debug_print_ast(ast_t* ast) {
    printf("------------- AST ------------\n");
    ast_print(ast);
}

void yyn_compile(char* src) {
    list_t* tokens = lexer_read_tokens(src);

    debug_print_tokens(tokens);

    ast_t* root_ast = parser_ast_from_tokens(tokens);

    debug_print_ast(root_ast);

    free(root_ast);
}

void yyn_compile_file(char* filename) {
    char* src = read_file(filename);
    yyn_compile(src);
    free(src);
}
