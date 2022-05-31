#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "include/ast.h"
#include "include/string_builder.h"

ast_t* init_ast(int type) {
    ast_t* ast = malloc(sizeof(ast_t));
    ast->type = type;

    if (type == AST_COMPOUND || type == AST_ROOT_COMPOUND) {
        ast->compound_children = init_list(sizeof(ast_t*));
        ast->compound_children->element_free_function = (free_function_t) free_ast;
    } else if (type == AST_FUNCTION_DEF) {
        ast->function_def_data = calloc(1, sizeof(function_def_data_t));
        ast->function_def_data->child_compound = init_ast(AST_COMPOUND);
    }

    return ast;
}

char* ast_print_child(ast_t* ast, char* prefix) { // NOLINT(misc-no-recursion)
    string_builder_t* final_text = init_string_builder();
    switch (ast->type) {
        case AST_HELLO: {
            string_builder_append(final_text, prefix);
            string_builder_append(final_text, "hello\n");
            break;
        }
        case AST_ROOT_COMPOUND:
        case AST_COMPOUND: {
            char* text_header;
            if(ast->type == AST_COMPOUND) {
                text_header = "compound:\n";
            } else {
                text_header = "root:\n";
            }
            char* text_child = " | ";
            char* text_child_prefix = calloc(4 + strlen(prefix), sizeof(char));
            strcat(text_child_prefix, prefix);
            strcat(text_child_prefix, text_child);

            string_builder_append(final_text, prefix);
            string_builder_append(final_text, text_header);

            for (int i = 0; i < ast->compound_children->size; ++i) {
                char* child_string = ast_print_child(list_get(ast->compound_children, i), text_child_prefix);
                string_builder_append(final_text, child_string);
            }
            break;
        }
        case AST_FUNCTION_DEF: {
            char* function_name = ast->function_def_data->function_name;
            ast_t* child_compound = ast->function_def_data->child_compound;

            char* text_header_template = "function(name=%s):\n";
            char* text_header = calloc(strlen(text_header_template) + strlen(function_name) + 1, sizeof(char));
            sprintf(text_header, text_header_template, function_name);

            string_builder_append(final_text, prefix);
            string_builder_append(final_text, text_header);

            string_builder_append(final_text, ast_print_child(child_compound, prefix));
            break;
        }
    }
    return string_builder_build(final_text);
}

void ast_print(ast_t* ast) {
    char* empty_prefix = "";
    char* message = ast_print_child(ast, empty_prefix);
    printf(message);
}

void free_ast(ast_t* ast) {
    if ((ast->type == AST_COMPOUND || ast->type == AST_ROOT_COMPOUND) && ast->compound_children) {
        free_list(ast->compound_children);
    }
    free(ast);
}
