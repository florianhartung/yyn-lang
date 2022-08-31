#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "include/ast.h"
#include "include/string_builder.h"
#include "include/util.h"


ast_t* init_ast(int type) { // NOLINT(misc-no-recursion)
    ast_t* ast = malloc(sizeof(ast_t));
    ast->type = type;

    switch (type) {
        case AST_COMPOUND:
            ast->compound_data = calloc(1, sizeof(compound_data_t));
            ast->compound_data->children = init_list(sizeof(ast_t*));
            ast->compound_data->children->element_free_function = (free_function_t) free_ast;
            ast->compound_data->variable_context = init_variable_context(NULL);
            break;
        case AST_ROOT_COMPOUND:
            ast->root_compound_data = calloc(1, sizeof(compound_data_t));
            ast->root_compound_data->compound_data.children = init_list(sizeof(ast_t*));
            ast->root_compound_data->compound_data.children->element_free_function = (free_function_t) free_ast;
            ast->root_compound_data->compound_data.variable_context = init_variable_context(NULL);
            break;
        case AST_FUNCTION_DEF:
            ast->function_def_data = calloc(1, sizeof(function_def_data_t));
            ast->function_def_data->child_compound = init_ast(AST_COMPOUND);
            break;
        case AST_ASSIGNMENT:
        case AST_ASSIGNMENT_ADD:
            ast->assignment_data = calloc(1, sizeof(assignment_data_t));
            break;
        case AST_WHILE_LOOP:
            ast->while_loop_data = calloc(1, sizeof(while_loop_data_t));
            break;
        case AST_BOOL_EXPRESSION:
            ast->bool_expression_data = calloc(1, sizeof(bool_expression_data_t));
        default:
            break;
    }

    return ast;
}

variable_context_t* init_variable_context(variable_context_t* parent_variable_context) {
    variable_context_t* variable_context = calloc(1, sizeof(variable_context_t));
    variable_context->variables = init_list(sizeof(variable_t));
    variable_context->parent = parent_variable_context;
    variable_context->children = init_list(sizeof(variable_context_t*));
    if (parent_variable_context) {
        list_append(parent_variable_context->children, variable_context);
    }

    return variable_context;
}

char* ast_print_child(ast_t* ast, char* prefix) { // NOLINT(misc-no-recursion)
    string_builder_t* final_text = init_string_builder();
    switch (ast->type) {
        case AST_HELLO: {
            string_builder_append(final_text, prefix);
            string_builder_append(final_text, "hello\n");
            break;
        }
        case AST_EXIT: {
            char* text = iasprintf("%sexit %d\n", prefix, ast->exit_code);
            string_builder_append(final_text, text);
            free(text);
            break;
        }
        case AST_PRINT_CHAR: {
            char* text = iasprintf("%sprintchar %s\n", prefix, ast_print_child(ast->expression_child, ""));
            string_builder_append(final_text, text);
            free(text);
            break;
        }
        case AST_VARIABLE: {
            char* text = iasprintf("%svariable \"%s\" = %d\n", prefix, ast->variable_data->name,
                                   ast->variable_data->value);
            string_builder_append(final_text, text),
                    free(text);
            break;
        }
        case AST_ROOT_COMPOUND:
        case AST_COMPOUND: {
            char* text_header;
            if (ast->type == AST_COMPOUND) {
                text_header = "child_compound";
            } else {
                text_header = "root";
            }
            char* text_child = " | ";
            char* text_child_prefix = calloc(4 + strlen(prefix), sizeof(char));
            strcat(text_child_prefix, prefix);
            strcat(text_child_prefix, text_child);

            string_builder_append(final_text, prefix);
            string_builder_append(final_text, text_header);

            string_builder_append(final_text, "(variables=[");
            bool first = true;
            LIST_FOREACH(ast->compound_data->variable_context->variables, variable_t* var, {
                if (first) {
                    first = false;
                } else {
                    string_builder_append(final_text, ",");
                }
                string_builder_append(final_text, var->name);
            })
            string_builder_append(final_text, "])\n");

            for (int i = 0; i < ast->compound_data->children->size; ++i) {
                char* child_string = ast_print_child(list_get(ast->compound_data->children, i), text_child_prefix);
                string_builder_append(final_text, child_string);
            }
            break;
        }
        case AST_FUNCTION_DEF: {
            char* function_name = ast->function_def_data->identifier;
            ast_t* child_compound = ast->function_def_data->child_compound;

            char* text_header_template = "function(name=%s):\n";
            char* text_header = calloc(strlen(text_header_template) + strlen(function_name) + 1, sizeof(char));
            sprintf(text_header, text_header_template, function_name);

            string_builder_append(final_text, prefix);
            string_builder_append(final_text, text_header);

            string_builder_append(final_text, ast_print_child(child_compound, prefix));
            break;
        }
        case AST_FUNCTION_CALL: {
            string_builder_append(final_text, iasprintf("%scall %s()\n", prefix, ast->function_identifier));
            break;
        }
        case AST_ASSIGNMENT_ADD: {
            string_builder_append(final_text,
                                  iasprintf("%svariable \"%s\" += %s\n", prefix, ast->assignment_data->variable->name,
                                            ast_print_child(ast->assignment_data->expression, prefix)));
            break;
        }
        case AST_ASSIGNMENT: {
            string_builder_append(final_text,
                                  iasprintf("%svariable \"%s\" = %s\n", prefix, ast->assignment_data->variable->name,
                                            ast_print_child(ast->assignment_data->expression, prefix)));
            break;
        }
        case AST_EXPRESSION: {
            switch (ast->expression_child->type) {
                case AST_INTEGER:
                    string_builder_append(final_text, iasprintf("%d", ast->expression_child->integer_value));
                    break;
                case AST_VARIABLE_ACCESS:
                    string_builder_append(final_text, iasprintf("\"%s\"", ast->expression_child->variable_data->name));
                    break;
                case AST_BOOL_EXPRESSION:
                    string_builder_append(final_text,
                                          iasprintf("%s < %s",
                                                    ast_print_child(ast->expression_child->bool_expression_data->left,
                                                                    ""),
                                                    ast_print_child(ast->expression_child->bool_expression_data->right,
                                                                    "")));
                    break;
                default:
                    string_builder_append(final_text, "---");
                    break;
            }
            break;
        }
        case AST_WHILE_LOOP: {
            string_builder_append(final_text, iasprintf("%swhile(%s):\n", prefix,
                                                        ast_print_child(ast->while_loop_data->expression_child, "")));
            string_builder_append(final_text, ast_print_child(ast->while_loop_data->child_compound, prefix));
            break;
        }
        default:
            string_builder_append(final_text, iasprintf("%s---\n", prefix));
    }
    return string_builder_build(final_text);
}

void ast_print(ast_t* ast) {
    char* empty_prefix = "";
    char* message = ast_print_child(ast, empty_prefix);
    printf(message);
}

void free_ast(ast_t* ast) {
    if ((ast->type == AST_COMPOUND || ast->type == AST_ROOT_COMPOUND) && ast->compound_data->children) {
        free_list(ast->compound_data->children);
    }
    free(ast);
}

char* generate_function_identifier(char* function_name) { //TODO: args
    return iasprintf("f%s_", function_name);
}
