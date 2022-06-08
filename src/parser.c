#include "include/parser.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct PARSER_STATE_STRUCT {
    list_t* tokens;
    size_t current_token_index;
    token_t* current_token;
} parser_state_t;

void parser_advance(parser_state_t* parser) {
    parser->current_token_index += 1;
    if (parser->current_token_index == parser->tokens->size) {
        fprintf(stderr, "Parser cannot read another token because the last one was already read.");
        exit(1);
    }
    parser->current_token = list_get(parser->tokens, parser->current_token_index);
}

parser_state_t* init_parser(list_t* tokens) {
    parser_state_t* parser = malloc(sizeof(parser_state_t));
    parser->tokens = tokens;
    parser->current_token_index = 0;
    parser->current_token = list_get(parser->tokens, parser->current_token_index);
    return parser;
}

void free_parser(parser_state_t* parser) {
    free_list(parser->tokens);
    free(parser);
}

void expect_token(parser_state_t* parser, int type) {
    if (parser->current_token->type != type) {
        fprintf(stderr, "Unexpected token %s, expected %s", token_to_string(parser->current_token),
                token_type_to_string(type));
        exit(1);
    }
}

void expect_keyword(parser_state_t* parser, int keyword) {
    expect_token(parser, TOKEN_KEYWORD);
    if (parser->current_token->token_data->keyword != keyword) {
        fprintf(stderr, "Unexpected token %s, expected keyword '%s'", token_to_string(parser->current_token),
                keywords[parser->current_token->token_data->keyword]);
        exit(1);
    }
}

void parser_advance_with_token(parser_state_t* parser, int type) {
    expect_token(parser, type);
    parser_advance(parser);
}

void parser_advance_with_keyword(parser_state_t* parser, int keyword) {
    expect_keyword(parser, keyword);
    parser_advance(parser);
}

token_t* parser_peek(parser_state_t* parser) {
    if (parser->current_token->type == TOKEN_EOF) {
        fprintf(stderr, "Cannot peek next token, if end of file is already reached!");
        exit(1);
    }

    return list_get(parser->tokens, parser->current_token_index + 1);
}


static bool
is_variable_name_unique(variable_context_t* variable_context, char* variable_name) { // NOLINT(misc-no-recursion)
    if (!variable_context->parent) {
        return true;
    }

    LIST_FOREACH(variable_context->variables, variable_t* var, {
        if (strcmp(var->name, variable_name) == 0) {
            return false;
        }
    })

    return is_variable_name_unique(variable_context->parent, variable_name);
}

static void register_variable(variable_context_t* variable_context, variable_t* variable) {
    if (!is_variable_name_unique(variable_context, variable->name)) {
        fprintf(stderr, "Variable \"%s\" already exists!", variable->name);
        exit(1);
    }

    list_append(variable_context->variables, variable);
}

static variable_t* get_variable_from_context(variable_context_t* variable_context, char* variable_name) {
    LIST_FOREACH(variable_context->variables, variable_t* var, {
        if (strcmp(var->name, variable_name) == 0) {
            return var;
        }
    })

    if (variable_context->parent == 0) {
        return NULL;
    }

    return get_variable_from_context(variable_context->parent, variable_name);
}

static ast_t* parse_expression(parser_state_t* parser, variable_context_t* variable_context) {
    ast_t* expression = init_ast(AST_EXPRESSION);

    switch (parser->current_token->type) {
        case TOKEN_IDENTIFIER: {
            expression->expression_child = init_ast(AST_VARIABLE_ACCESS);

            char* identifier = parser->current_token->token_data->identifier_name;
            expression->expression_child->variable_data = get_variable_from_context(variable_context, identifier);
            if(!expression->expression_child->variable_data) {
                fprintf(stderr, "Undeclared variable \"%s\" is used in an expresson", identifier);
                exit(1);
            }
            parser_advance(parser);
            break;
        }
        case TOKEN_INTEGER: {
            expression->expression_child = init_ast(AST_INTEGER);
            expression->expression_child->integer_value = parser->current_token->token_data->int_value;
            parser_advance(parser);
            break;
        }
        default:
            fprintf(stderr, "Unknown expression token %s", token_to_string(parser->current_token));
            exit(1);
    }

    return expression;
}

static ast_t* parse_assignment(parser_state_t* parser, variable_context_t* variable_context) {
    variable_t* variable = get_variable_from_context(variable_context,
                                                     parser->current_token->token_data->identifier_name);
    if(!variable) {
        fprintf(stderr, "Cannot assign value to undeclared variable \"%s\"", parser->current_token->token_data->identifier_name);
        exit(1);
    }
    parser_advance(parser);
    ast_t* assignment;
    switch (parser->current_token->type) {
        case TOKEN_ASSIGNMENT_ADD: {
            assignment = init_ast(AST_ASSIGNMENT_ADD);
            assignment->assignment_data->variable = variable;

            parser_advance(parser);

            assignment->assignment_data->expression = parse_expression(parser, variable_context);
            break;
        }
        default:
            fprintf(stderr, "Unknown assignment token %s", token_to_string(parser->current_token));
            exit(1);
    }
    return assignment;
}

ast_t*
parse_compound(parser_state_t* parser, variable_context_t* parent_variable_context, list_t* undeclared_used_functions);

ast_t*
parse_statement(parser_state_t* parser, variable_context_t* parent_variable_context, int index_in_parent_compound,
                list_t* undeclared_used_functions) { // NOLINT(misc-no-recursion)

    ast_t* statement;
    switch (parser->current_token->type) {
        case TOKEN_KEYWORD: {
            switch (parser->current_token->token_data->keyword) {
                case KEYWORD_HELLO: {
                    parser_advance(parser);
                    statement = init_ast(AST_HELLO);
                    break;
                }
                case KEYWORD_EXIT: {
                    parser_advance(parser);
                    statement = init_ast(AST_EXIT);
                    expect_token(parser, TOKEN_INTEGER);
                    statement->exit_code = parser->current_token->token_data->int_value;
                    parser_advance(parser);
                    break;
                }
                case KEYWORD_PRINT_CHAR: {
                    parser_advance(parser);
                    statement = init_ast(AST_PRINT_CHAR);
                    statement->expression_child = parse_expression(parser, parent_variable_context);
                    break;
                }
                case KEYWORD_INT: {
                    parser_advance(parser);
                    statement = init_ast(AST_VARIABLE);

                    expect_token(parser, TOKEN_IDENTIFIER);
                    statement->variable_data = calloc(1, sizeof(variable_t));
                    statement->variable_data->name = parser->current_token->token_data->identifier_name;
                    statement->variable_data->type = AST_TYPE_INT;
                    parser_advance(parser);

                    parser_advance_with_token(parser, TOKEN_EQUALS);

                    expect_token(parser, TOKEN_INTEGER);

                    statement->variable_data->value = parser->current_token->token_data->int_value;
                    statement->variable_data->declaration_index = index_in_parent_compound;
                    parser_advance(parser);

                    register_variable(parent_variable_context, statement->variable_data);

                    break;
                }
                default:
                    fprintf(stderr, "Unexpected keyword '%s'", keywords[parser->current_token->token_data->keyword]);
                    exit(1);
            }
            break;
        }
        case TOKEN_LBRACE: {
            variable_context_t* variable_context = init_variable_context(parent_variable_context);

            statement = parse_compound(parser, variable_context, undeclared_used_functions);
            break;
        }
        case TOKEN_IDENTIFIER: {
            switch (parser_peek(parser)->type) {
                case TOKEN_ASSIGNMENT_ADD: {
                    statement = parse_assignment(parser, parent_variable_context);
                    break;
                }
                case TOKEN_LPAREN: {
                    char* function_name = parser->current_token->token_data->identifier_name;

                    char* function_identifier = generate_function_identifier(function_name);

                    statement = init_ast(AST_FUNCTION_CALL);
                    statement->function_identifier = function_identifier;

                    parser_advance(parser);
                    parser_advance_with_token(parser, TOKEN_LPAREN);

                    //TODO: parse args

                    parser_advance_with_token(parser, TOKEN_RPAREN);

                    list_append(undeclared_used_functions, function_identifier);
                    break;
                }
                default:
                    fprintf(stderr, "Unexpected token %s, expected an assignment or function call",
                            token_to_string(parser->current_token));
                    exit(1);
            }
            break;
        }
        default:
            fprintf(stderr, "Unknown token %s, expected a statement", token_to_string(parser->current_token));
            exit(1);
    }

    parser_advance_with_token(parser, TOKEN_NEWLINE);
    return statement;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"

ast_t*
parse_compound(parser_state_t* parser, variable_context_t* parent_variable_context, list_t* undeclared_used_functions) {
    parser_advance_with_token(parser, TOKEN_LBRACE);

    ast_t* compound = init_ast(AST_COMPOUND);
    compound->compound_data->variable_context = init_variable_context(parent_variable_context);

    int statement_index = 0;
    while (parser->current_token->type != TOKEN_RBRACE) {
        if (parser->current_token->type == TOKEN_NEWLINE) {
            parser_advance(parser);
            continue;
        }
        ast_t* statement = parse_statement(parser, compound->compound_data->variable_context, statement_index,
                                           undeclared_used_functions);
        list_append(compound->compound_data->children, statement);
        statement_index += 1;
    }

    parser_advance_with_token(parser, TOKEN_RBRACE);

    return compound;
}

#pragma clang diagnostic pop

ast_t*
parse_function(parser_state_t* parser, variable_context_t* parent_variable_context, list_t* undeclared_used_functions) {
    parser_advance_with_keyword(parser, KEYWORD_FUNCTION);

    ast_t* function_definition = init_ast(AST_FUNCTION_DEF);

    expect_token(parser, TOKEN_IDENTIFIER);
    function_definition->function_def_data->identifier = generate_function_identifier(
            parser->current_token->token_data->identifier_name);
    parser_advance(parser);

    parser_advance_with_token(parser, TOKEN_LPAREN);

    variable_context_t* variable_context = init_variable_context(parent_variable_context);
    // TODO: parse function arguments

    parser_advance_with_token(parser, TOKEN_RPAREN);

    ast_t* function_child_compound = parse_compound(parser, variable_context, undeclared_used_functions);
    function_definition->function_def_data->child_compound = function_child_compound;
    return function_definition;
}

ast_t* parse_root_compound(parser_state_t* parser) {
    ast_t* root_compound = init_ast(AST_ROOT_COMPOUND);
    root_compound->root_compound_data->compound_data.variable_context->parent = NULL;

    // lists of string of function names ( <FUNC_NAME>#<ARGS_TYPES_SEPARATED_BY_UNDERSCORES> )
    list_t* declared_functions = init_list(sizeof(char*));
    list_t* undeclared_used_functions = init_list(sizeof(char*));

    while (parser->current_token->type != TOKEN_EOF) {
        if (parser->current_token->type == TOKEN_NEWLINE) {
            parser_advance(parser);
            continue;
        }

        ast_t* function = parse_function(parser, root_compound->root_compound_data->compound_data.variable_context,
                                         undeclared_used_functions);
        list_append(declared_functions, function->function_def_data->identifier);
        list_append(root_compound->root_compound_data->compound_data.children, function);
    }

    LIST_FOREACH(undeclared_used_functions, char* undeclared_function, {
        bool found = false;
        LIST_FOREACH(declared_functions,
        char* declared_function,
        {
            if (strcmp(declared_function, undeclared_function) == 0) {
                found = true;
            }
        })
        if (!found) {
            fprintf(stderr, "Function %s is called, but undeclared", undeclared_function);
            exit(1);
        }
    })

    return root_compound;
}

ast_t* parser_ast_from_tokens(list_t* tokens) {
    parser_state_t* parser = init_parser(tokens);
    ast_t* root = parse_root_compound(parser);
    free_parser(parser);

    return root;
}
